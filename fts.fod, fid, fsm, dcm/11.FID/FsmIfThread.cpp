// FASIPThread.cpp : implementation file
//

#include "stdafx.h"
#include <mmsystem.h>

#include "APP.h"
#include "FsmIfThread.h"
#include "Config.h"

#include "AppLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFsmIfThread

CFsmIfThread* CFsmIfThread::s_pInstance = NULL;

CFsmIfThread* CFsmIfThread::Inst()
{   
	if( s_pInstance == NULL )
		s_pInstance = new CFsmIfThread;
	return s_pInstance;
}

CFsmIfThread::CFsmIfThread()
{
	m_hClient		= INVALID_SOCKET;
	m_sip_status	= SIP_Uninit;

	m_tLastSend		= 0;
	m_tLastRecv		= 0;
}

CFsmIfThread::~CFsmIfThread()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFsmIfThread message handlers

void CFsmIfThread::onThreadEntry()
{   
	SOCKET				hSockArray[WSA_MAXIMUM_WAIT_EVENTS];	
	WSAEVENT			hEventArray[WSA_MAXIMUM_WAIT_EVENTS];

	WSAEVENT			newEvent;
	WSANETWORKEVENTS	netEvents;

    int					index, i, len;
	time_t				tCurrent;
	CString				strLog;

	FAX_SIP_HEADER*		pHead;

	char				recvBuf[512];

	m_sip_status = SIP_Init;

	// P3. 종료될 때까지 Socket 이벤트 또는 IO 이벤트를 기다린다.
	while( !IsReqStop() )
	{   
		// P4. 접속이 끊어졌을 때 재접속 시도
		if( m_sip_status < SIP_Connect )
		{   
			::Sleep( 5000 );
			if( !Connect() )
				continue;

			Sleep( 1000 );

			// P4-1. FD_READ | FD_CLOSE 이벤트 등록
			newEvent = WSACreateEvent();
			WSAEventSelect(m_hClient, newEvent, FD_READ | FD_CLOSE);

			hEventArray[0] = newEvent;
			hSockArray[0] = m_hClient;

			// P4-2. Register Packet 송신
			len = FASIPRegister();
			if( len <= 0 )
			{
				ReleaseResource(hEventArray[0]);
				continue;
			}
            
		}

        Sleep( 1000 );

		// P5. Register 상태면 Keep Alive 전송
		if( m_sip_status == SIP_Register )
		{   
			tCurrent = time( NULL );
			if( CConfig::FSM_SND_ALIVE_TIME > 0 
				&& tCurrent - m_tLastSend >= CConfig::FSM_SND_ALIVE_TIME )
			{   
				len = FASIPKeepalive();
				if( len <= 0 )
				{   
					ReleaseResource(hEventArray[0]);
					continue;
				}
			}
		}

		index = WSAWaitForMultipleEvents(1, hEventArray, FALSE, 200 , FALSE);
        index = index - WSA_WAIT_EVENT_0;
		// P6-1. Socket 이벤트 Timeout
		if(index == WSA_WAIT_TIMEOUT)											// 0.2초 동안 이벤트가 없을 경우
		{
			// Alive 체크
			time_t tCurrent = time( NULL );

			if( CConfig::FSM_CHK_ALIVE_TIME > 0 
				&& tCurrent - m_tLastRecv >= CConfig::FSM_CHK_ALIVE_TIME )
			{
				strLog.Format( "Check Alive. CLOSE. current[%d], last[%d], elapsed[%d], FSM_CHK_ALIVE_TIME[%d]", 
								tCurrent, m_tLastRecv, tCurrent-m_tLastRecv, CConfig::FSM_CHK_ALIVE_TIME );
				APPLOG->LogSip( DBGLV_ERR, strLog );

				ReleaseResource(hEventArray[0]);
				continue;
			}

			// FaxChThread Send 체크
			FAX_SEND_PACKET  SndPacket;
			while( SndPacket_Pop( &SndPacket ) )
			{	
				len = Send( &SndPacket, sizeof(SndPacket.h) + SndPacket.h.len );
				APPLOG->LogSip( 'S', SndPacket.h.chan, SndPacket.h.msgid, len );
			}
			continue;
		}


		// P6-2. Socket 이벤트 발생
		for( i = index ; i < 1 ; i++ ) 
		{
			index = WSAWaitForMultipleEvents(1, &hEventArray[i], TRUE, 5, FALSE);
			if ((index == WSA_WAIT_FAILED) || (index == WSA_WAIT_TIMEOUT))		// 이벤트가 발생한 Socket이 아니면 다음 Socket 검색
				continue;


			// 이벤트가 발생한 Socket이면
			index = i;
			WSAEnumNetworkEvents(hSockArray[index], hEventArray[index], &netEvents);

			// p6-3. Server Data 수신
			if( netEvents.lNetworkEvents & FD_READ )
			{
				if (netEvents.iErrorCode[FD_READ_BIT] != 0)
				{
					strLog.Format( "Read Socket Error. error[%d]", netEvents.iErrorCode[FD_READ_BIT] );
					APPLOG->LogSip( DBGLV_ERR, strLog );
					break;
				}

				// p6-3-1. SIP 전문 수신
				ZeroMemory(recvBuf,sizeof(recvBuf));
				pHead = (FAX_SIP_HEADER *) recvBuf;

				int rcvLen = ReadFASIP(hSockArray[index - WSA_WAIT_EVENT_0], recvBuf);
				if( rcvLen < 0 )
				{
					ReleaseResource(hEventArray[index]);
					break;
				}

				// p6-3-2. SIP 전문 처리
				if( !HandleRcvPacket( recvBuf, rcvLen ) )
				{
					ReleaseResource(hEventArray[index]);
					break;
				}

			} // if(netEvents.lNetworkEvents & FD_READ) end

			// p6-3. Server Socket에서 발생한 CLOSE Event면 (Client의 접속종료)
			if (netEvents.lNetworkEvents & FD_CLOSE)               
			{
				if (netEvents.iErrorCode[FD_READ_BIT] != 0)
				{
					strLog.Format( "Closed Error. error[%d]", netEvents.iErrorCode[FD_READ_BIT] );
					APPLOG->LogSip( DBGLV_ERR, strLog );
				}

				APPLOG->LogSip( DBGLV_RPT, "Closed" );
				ReleaseResource(hEventArray[index]);
				break;
			} // if(netEvents.lNetworkEvents & FD_CLOSE) end

		} // for(i=index; i<sockTotal; i++) end
	}

	APPLOG->LogSip( DBGLV_ERR, "Thread Stop" );
}

// -------------------------------------------------------------------
// Module 명	: void FASIPConnect()
// -------------------------------------------------------------------
// Descriotion	: FASIP Connect 처리
// -------------------------------------------------------------------
// Argument		: bool	bReconn;
// -------------------------------------------------------------------
// Return 값	: SOCKET			SUCC
//				  INVALID_SOCKET	FAIL
// -------------------------------------------------------------------
bool CFsmIfThread::Connect()
{   
	int					addrLen, result;
	struct sockaddr_in	addrSrv;
	CString				strLog;

	// P1. Socket 생성 (Client Socket 1개)
	m_hClient = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);	
	if( m_hClient == INVALID_SOCKET )
	{
		strLog.Format( "WSASocket Fail. error[%d]", GetLastError() );
		APPLOG->LogSip( DBGLV_ERR, strLog );
		
		m_hClient = INVALID_SOCKET;
		m_sip_status = SIP_Init;

		return false;
	}

	// P2. 접속할 Server Address 설정
	addrLen = sizeof(addrSrv);
	memset( &addrSrv,0x00, addrLen );
	addrSrv.sin_family		= AF_INET;
	addrSrv.sin_addr.s_addr = inet_addr(CConfig::FSM_IP.GetBuffer(0));
	addrSrv.sin_port		= htons(CConfig::FSM_PORT);


	// P3. connect
	result = connect( m_hClient, (struct sockaddr *)&addrSrv, sizeof(struct sockaddr_in) );
	if( result < 0 )
	{
		closesocket( m_hClient );

		m_hClient = INVALID_SOCKET;
		m_sip_status = SIP_Init;
				
		strLog.Format( "Connect Fail. ip[%s] port[%d] error[%d]", (LPCSTR)CConfig::FSM_IP, CConfig::FSM_PORT, GetLastError() );
		APPLOG->LogSip( DBGLV_ERR, strLog );
		
		return false;
	}
	
	m_sip_status = SIP_Connect;
	m_tLastSend = m_tLastRecv = time( NULL );

	strLog.Format( "Connected. ip[%s] port[%d]", (LPCSTR)CConfig::FSM_IP, CConfig::FSM_PORT );
	APPLOG->LogSip( DBGLV_RPT, strLog );

	return true;
}

// -------------------------------------------------------------------
// Module 명	: int FASIPRegister()
// -------------------------------------------------------------------
// Descriotion	: FASIP Register 처리
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return 값	: nsLen			SUCC
//				  <=			FAIL
// -------------------------------------------------------------------
int CFsmIfThread::FASIPRegister()
{
	int					len;
	FAX_REGISTER*		pRegister;

	FAX_SIP_CONTROL		faxRegister;

	// P1. Register Packet Clear
	memset(&faxRegister, 0x00, sizeof(FAX_SIP_CONTROL));

	// P3. Register Packet Set
	faxRegister.header.msgid	= Fax_Register;
	faxRegister.header.chan		= -1;
	faxRegister.header.len		= sizeof(FAX_REGISTER);
	pRegister = (FAX_REGISTER*)faxRegister.body;
	pRegister->fax_direction	= 'I';
	memcpy(pRegister->fax_ip, CConfig::SYSTEM_IP, CConfig::SYSTEM_IP.GetLength());
	pRegister->fax_base_port	= CConfig::RTP_BASE_PORT;
	pRegister->fax_port_spacing	= CConfig::RTP_PORT_SPACE;
	pRegister->fax_device_start	= CConfig::FAX_START_CH;
	pRegister->fax_device_cnt	= CConfig::FAX_CH_CNT;

	// P2. Register Packet Send
	len = Send( &faxRegister, sizeof(FAX_SIP_HEADER) + sizeof(FAX_REGISTER) );
	if( len <= 0 )
	{
		APPLOG->LogSip(DBGLV_ERR, "Fax_Register send FAIL");
		return len;
	}
	
	// P3. Log Register Packet
	APPLOG->LogSip( 'S', -1, Fax_Register, len );

	return len;
}


// -------------------------------------------------------------------
// Module 명	: int FASIPKeepalive()
// -------------------------------------------------------------------
// Descriotion	: FASIP Keepalive 처리
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return 값	: nsLen			SUCC
//				  <=			FAIL
// -------------------------------------------------------------------
int CFsmIfThread::FASIPKeepalive()
{
	int					len;
	FAX_SIP_HEADER		faxKeepAlive;
	
	memset(&faxKeepAlive, 0x00, sizeof(FAX_SIP_HEADER));

	faxKeepAlive.msgid	= Fax_Keep_Alive_Fax;
	faxKeepAlive.chan	= -1;
	faxKeepAlive.len	= 0;

	len = Send( &faxKeepAlive, sizeof(FAX_SIP_HEADER) );
	//if( len <= 0 )
		APPLOG->LogSip( 'S', -1, Fax_Keep_Alive_Fax, len );

	return len;
}

int CFsmIfThread::Send( const void* p_sPacket, int p_len )
{
	int len;

	if( m_hClient == INVALID_SOCKET )
		return -1;

	len = send( m_hClient, (const char*)p_sPacket, p_len, 0 );
	if( len < 0 )
	{
		CString strLog;

		strLog.Format( "fail to send packet. (%d)|", p_len );
		strLog.Append( (const char*)p_sPacket, p_len );
		strLog+= "|";
		APPLOG->LogSip( DBGLV_ERR, strLog );

		return len;
	}

	m_tLastSend = time( NULL);

	return len;
}

// -------------------------------------------------------------------
// Module 명	: int ReadPacket()
// -------------------------------------------------------------------
// Descriotion	: 특정 SOCKET으로부터 전문을 수신한다.
// -------------------------------------------------------------------
// Argument		: SOCKET hInSocket;		SOCKET handle
//				  char *pszBuf4Recv;	수신 Buffer
//				  int nSize;			수신 Size
//				  int nTimeOut;			TimeOUT (second)
// -------------------------------------------------------------------
// Return 값	: nRC			SUCC (recv() 길이
//				  0				FAIL (Remote Connection Closed)
//				  -1			FAIL (invalid hInSocket)
//				  -2			FAIL (select() 오류)
//				  -3			FAIL (FD_ISSET() 오류)
//				  -4			FAIL (recv() 오류)
// -------------------------------------------------------------------
int CFsmIfThread::ReadPacket(SOCKET hInSocket, char *pszBuf4Recv, int nSize, int nTimeOut)
{
	int nRC;							/* function return code or received bytes count */
	int nEC;							/* error code */
	fd_set rfds, efds;					/* read file descriptor set, except descriptor set */
	static struct timeval tv;

	/* p0. if socket handle is invalid, return */
	if(NULL == hInSocket)
	{
		APPLOG->Print(DBGLV_ERR, "[ReadPacket] hInSocket is invalid");
		return -1;
	}
	
	nRC = 1;
	nEC = 0;
	
	FD_ZERO(&rfds);
	FD_ZERO(&efds);
	FD_SET(hInSocket, &rfds);
	FD_SET(hInSocket, &efds);
	tv.tv_sec = nTimeOut;
	tv.tv_usec = 0;
	

	/* p1. select() */
	nRC = select(0, &rfds, NULL, &efds, &tv);
	if(SOCKET_ERROR == nRC)
	{
		nEC = WSAGetLastError();
		APPLOG->Print(DBGLV_ERR, "[ReadPacket] select() error...rc[%d]", nEC);
		return -2;
	}
	else if(!(FD_ISSET(hInSocket, &rfds)))
	{
		APPLOG->Print(DBGLV_ERR, "[ReadPacket] select() time out ...[%d] seconds", nTimeOut);
		return -3;
	} 
	
	/* p2. recv */
	nRC = recv(hInSocket, pszBuf4Recv, nSize, 0);
	if(nRC == SOCKET_ERROR)
	{
		nEC = WSAGetLastError();
		APPLOG->Print(DBGLV_ERR, "[ReadPacket] recv() socket error...rc[%d]", nEC);
		return -4;
		
	}
	else if(0 == nRC)
	{
		return 0;
	}
	
	return nRC;
}


// -------------------------------------------------------------------
// Module 명	: int ReadFASIP()
// -------------------------------------------------------------------
// Descriotion	: FASIP로부터 날라온 전문을 수신한다.
// -------------------------------------------------------------------
// Argument		: SOCKET hInSocket;		SOCKET handle
//				  char *pszBuf4Recv;	수신 Buffer
// -------------------------------------------------------------------
// Return 값	: 0			SUCC (FASIP 전문 수신 성공)
//				  -1		FAIL (Header Recv 실패)
//				  -2		FAIL (Body Recv 실패)
// -------------------------------------------------------------------
int CFsmIfThread::ReadFASIP(SOCKET hInSocket, char *pszBuf4Recv)
{
	int				len1, len2;
	CString			strLog;
	FAX_SIP_HEADER*	pHead;

	pHead = (FAX_SIP_HEADER*) pszBuf4Recv;

	len1 = len2 = 0;
	len1 = ReadPacket(hInSocket, pszBuf4Recv, sizeof(FAX_SIP_HEADER), 1);
	if( len1 <= 0 )
	{
		strLog.Format( "Receive Fail. error[%d]", len1 );
		APPLOG->LogSip( DBGLV_ERR,  strLog );
		return -1;
	}

	m_tLastRecv = time( NULL );

	if( pHead->len > 0 )
	{
		len2 = ReadPacket(hInSocket, pszBuf4Recv + sizeof(FAX_SIP_HEADER), pHead->len, 1);
		if( len2 <= 0 )
		{
			APPLOG->LogSip( 'R', pHead->chan, pHead->msgid, len2, "Not Received Body" );
			return -2;
		}

		if( len2 != pHead->len )
		{
			APPLOG->LogSip( 'R', pHead->chan, pHead->msgid, len1 + len2, "Not Mismatch Packet Len" );
			return -3;
		}
	}

	return len1 + len2;
}


bool CFsmIfThread::HandleRcvPacket( const char* p_pBuffer, int p_len )
{
	CString			strLog;
	FAX_SIP_HEADER* pHead		= (FAX_SIP_HEADER*) p_pBuffer;
	FAX_RECV_PACKET* pPacket	= (FAX_RECV_PACKET*) p_pBuffer;
	

	switch( pHead->msgid )
	{
	case Fax_Keep_Alive_Sip:
		{
			APPLOG->LogSip( 'R', -1, pHead->msgid, p_len );
		}
		break;

	case Fax_Register_Ack:
		{
			APPLOG->LogSip( 'R', -1, pHead->msgid, p_len, pPacket->b.regi.result );

			if( pPacket->b.regi.result != 0)
				return false;
					
			m_sip_status = SIP_Register;
		}
		break;

	case Fax_Rx_Start:
	case Fax_Rx_Restart:
		{
			strLog.Format( "from[%s] to[%s] transfer[%s] gw_ip[%s] gw_port[%d]", 
							pPacket->b.start.from, 
							pPacket->b.start.to,
							pPacket->b.start.transfer,
							pPacket->b.start.gw_ip,
							pPacket->b.start.gw_port );
			APPLOG->LogSip( 'R', pHead->chan, pHead->msgid, p_len, strLog );

			RcvPacket_Push( *pPacket );
		}
		break;

	case Fax_Abort:
		{
			APPLOG->LogSip( 'R', pHead->chan, pHead->msgid, p_len );
			RcvPacket_Push( *pPacket );
		}
		break;

	case Fax_ReqCalling_Ack:
		{
			APPLOG->LogSip( 'R', pHead->chan, pHead->msgid, p_len, pPacket->b.call_ack.result );
			RcvPacket_Push( *pPacket );
		}
		break;

	case Fax_Cancelled:
		{
			APPLOG->LogSip( 'R', pHead->chan, pHead->msgid, p_len, pPacket->b.cancelled.sip_reason );
			RcvPacket_Push( *pPacket );
		}
		break;
		
	default:
		{
			APPLOG->LogSip( 'R', pHead->chan, pHead->msgid, p_len, "not defined packet" );
		}
		break;
	}

	return true;
}


// -------------------------------------------------------------------
// Module 명	: void ReleaseResource()
// -------------------------------------------------------------------
// Descriotion	: 오류 전문을 받았거나 전문 수신 실패시 처리
// -------------------------------------------------------------------
// Argument		: WSAEVENT hWasEvent;		WSAEVENT handle
// -------------------------------------------------------------------
// Return 값	: void
// -------------------------------------------------------------------
void CFsmIfThread::ReleaseResource(WSAEVENT hWasEvent)
{
	WSACloseEvent(hWasEvent);
	closesocket(m_hClient);
	APPLOG->LogSip( DBGLV_RPT, "Close" );

	m_hClient	 = INVALID_SOCKET;
	m_sip_status = SIP_Init;
}


bool CFsmIfThread::SndPacket_Push(  int chan, FAX_SEND_PACKET& SndPacket )
{
	SndPacket.h.chan = chan + m_chanBegin;
	
	{CSingleLock locker( &m_csLock, TRUE );

		m_SndPackets.push_back( SndPacket );
	}

	return true;
}

bool CFsmIfThread::SndPacket_Pop( FAX_SEND_PACKET* pSndPacket )
{
	{CSingleLock locker( &m_csLock, TRUE );

		if( m_SndPackets.empty() )
			return false;

		*pSndPacket = m_SndPackets.front();
		m_SndPackets.pop_front();
	}

	return true;
}

bool CFsmIfThread::RcvPacket_Init( int chanBegin, int chanCnt )
{
	CHAN_RECV_PACKET rcvPacket;

	rcvPacket.bReceived = false;
	memset( rcvPacket.packet.dummy, 0x00, sizeof(rcvPacket.packet.dummy) );

	{CSingleLock locker( &m_csLock, TRUE );

		m_chanBegin = chanBegin;
		for( int i = 0 ; i < chanCnt ; ++i )
			m_RcvPackets.push_back( rcvPacket );
	}

	return true;
}

bool CFsmIfThread::RcvPacket_Push( FAX_RECV_PACKET& RcvPacket )
{
	int chan = RcvPacket.h.chan - m_chanBegin;

	{CSingleLock locker( &m_csLock, TRUE );

		if( chan >= (int)m_RcvPackets.size() )
			return false;

		m_RcvPackets[ chan ].packet	= RcvPacket;
		m_RcvPackets[ chan ].bReceived = true;
	}

	return true;
}

bool CFsmIfThread::RcvPacket_Pop( int chanNum, FAX_RECV_PACKET* pRcvPacket )
{
	int chan = chanNum - m_chanBegin;

	{CSingleLock locker( &m_csLock, TRUE );

		if( chan >= (int)m_RcvPackets.size() )
			return false;

		if( !m_RcvPackets[ chan ].bReceived )
			return false;

		*pRcvPacket = m_RcvPackets[ chan ].packet;
		m_RcvPackets[ chan ].bReceived = false;
	}

	return true;
}

bool CFsmIfThread::RcvPacket_Wait( int chanNum, int nWaitMilli, FAX_RECV_PACKET* pRcvPacket )
{
	DWORD dwStart, dwCurrent;
	
	dwStart = timeGetTime();
	while( true )
	{
		Sleep( 100 );

		if( RcvPacket_Pop( chanNum, pRcvPacket ) )
			break;

		dwCurrent = timeGetTime();
		if( dwCurrent - dwStart > (DWORD)nWaitMilli )
			return false;
	}

	return true;
}

bool CFsmIfThread::RcvPacket_Wait( int chanNum, int nMsgId, int nWaitMilli, FAX_RECV_PACKET* pRcvPacket )
{
	DWORD dwStart, dwCurrent;
	
	dwStart = timeGetTime();
	while( true )
	{
		Sleep( 100 );

		if( RcvPacket_Pop( chanNum, pRcvPacket ) )
		{
			if( pRcvPacket->h.msgid == nMsgId )
				return true;
		}

		dwCurrent = timeGetTime();
		if( dwCurrent - dwStart > (DWORD)nWaitMilli )
			break;
	}

	return false;
}


