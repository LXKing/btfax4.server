// FaxChThread.cpp : implementation file
//

#include "stdafx.h"
#include "APP.h"
#include "ShmCtrl.h";
#include "FaxChThread.h"
#include "FsmIfThread.h"
#include "Config.h"

#include "StatusIni.h"
#include "Utility.h"

#include "btfax.h"
#include "btflib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFaxChThread

CFaxChThread::CFaxChThread()
{
	m_chidx	= -1;
	m_chnum	= -1;
	m_pSendReq = NULL;
}

CFaxChThread::~CFaxChThread()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaxChThread message handlers


// -------------------------------------------------------------------
// Module ��	: void GetFaxChNum()
// -------------------------------------------------------------------
// Descriotion	: CFaxChThread ��ü�� 0 Based FAX CH ��ȣ�� ��´�.
// -------------------------------------------------------------------
// Argument		: int &chidx;		g_FaxChInfo Array�� Index
//				  int &chnum;		0 Based FAX CH ��ȣ
// -------------------------------------------------------------------
// Return ��	: 0 ~ MAX_FODCH-1		SUCC
//				  -1					FAIL
// -------------------------------------------------------------------
void CFaxChThread::GetFaxChNum(int &chidx, int &chnum)
{
	if(m_chidx < 0 || m_chidx >= MAX_CHANNEL || m_chnum < 0)
	{
		chidx = -1;
		chnum = -1;
	}
	else
	{
		chidx = m_chidx;
		chnum = m_chnum;
	}
	return;
}

void CFaxChThread::RequestOutbound( CDbModule::SEND_REQ* p_pSendReq )
{
	;
}

// -------------------------------------------------------------------
// Module ��	: void SetShmState()
// -------------------------------------------------------------------
// Descriotion	: SharedMemory ���� ������Ʈ
// -------------------------------------------------------------------
// Argument		: EN_STATUS ch_state;		FAX CH ���� (EN_STATUS)
// -------------------------------------------------------------------
// Return ��	: void
// -------------------------------------------------------------------
void CFaxChThread::SetShmState( EN_STATUS ChState)
{
	char currTime[20];
	CUtility::CurrentTimeToStr(currTime);

	SHM_CH_MONI_DATA* shmData = g_shmCtrl.GetShmData(m_chnum);
	if(shmData == NULL)
		return;
	
	shmData->channel_state	= ChState;
	shmData->chg_flag		= 1;			
	switch(ChState)
	{
		case FAX_INIT : 
			shmData->Clear();
			shmData->use_flag				= 1;
			shmData->channel				= m_chnum;
			shmData->call_diretion			= IO_OUTBOUND;
			shmData->system_id				= CConfig::SYSTEM_NO;
			shmData->module_id				= CConfig::PROCESS_NO;			
			break;

		case FAX_IDLE : 			
			memset(shmData->ani,			0x00, sizeof(shmData->ani));
			memset(shmData->dnis,			0x00, sizeof(shmData->dnis));
			memset(shmData->service_id,		0x00, sizeof(shmData->service_id));
			memset(shmData->service_name,	0x00, sizeof(shmData->service_name));

			memset(shmData->call_connected_time		,	0x00, sizeof(shmData->call_connected_time));
			memset(shmData->call_incoming_time		,	0x00, sizeof(shmData->call_incoming_time));
			memset(shmData->call_disconnected_time	,	0x00, sizeof(shmData->call_disconnected_time));
			break;

		case FAX_OCCUPY : 
			if( g_FaxChInfo[m_chidx].sendReq.master.REQ_USER_TEL_NO.GetLength() > 0 )
				strcpy(shmData->ani, g_FaxChInfo[m_chidx].sendReq.master.REQ_USER_TEL_NO);
			else
				strcpy(shmData->ani, CConfig::FAX_DEFAULT_ANI);	

			strcpy(shmData->dnis		, g_FaxChInfo[m_chidx].sendReq.detail.FAX_NO);
			strcpy(shmData->service_id	, g_FaxChInfo[m_chidx].sendReq.master.TR_NO);
			strcpy(shmData->service_name, g_FaxChInfo[m_chidx].sendReq.master.SVC_NAME);
			break;

		case FAX_DIAL : 
			strcpy(shmData->call_connected_time, currTime);
			break;

		case FAX_SEND : 
			strcpy(shmData->call_incoming_time, currTime);
			shmData->outbound_cnt++;
			break;

		case FAX_RECV : 
		case FAX_SUCC_SEND : 			
		case FAX_FAIL_SEND : 			
		case FAX_SUCC_RECV : 
		case FAX_FAIL_RECV : 
			strcpy(shmData->call_disconnected_time, currTime);
			break;
		
		case FAX_ABORT : break;
		default : break;
	}

	APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] Channel Info (channel:%d, channel_state:%d, chg_flag:%d, outbound_cnt:%d, system_id:%d, module_id:%d, ani:%s, dnis:%s)"
		, m_chnum
		, shmData->channel
		, shmData->channel_state
		, shmData->chg_flag
		, shmData->outbound_cnt		
		, shmData->system_id
		, shmData->module_id
		, shmData->ani
		, shmData->dnis									
		);
}


// -------------------------------------------------------------------
// Module ��	: void SetFaxChState()
// -------------------------------------------------------------------
// Descriotion	: FAX CH�� ���¸� ����
// -------------------------------------------------------------------
// Argument		: EN_STATUS ch_state;		FAX CH ���� (EN_STATUS)
//				  char* ch_msg;				CH ���� Message
// -------------------------------------------------------------------
// Return ��	: void
// -------------------------------------------------------------------
void CFaxChThread::SetFaxChState( EN_STATUS ChState, const char* szStatusMsg, const char* szLogMsg )
{
	CString strLogLine;
	
	if(m_chidx < 0 || m_chidx >= MAX_CHANNEL || m_chnum < 0)
		return;

	// P0. ���� ����
	g_FaxChInfo[m_chidx].ch_state	= ChState;
	
	// P1. ä�� ����  INI ����
	CStatusIni::Inst()->Update( m_chidx, ChState, szStatusMsg, &strLogLine );

	// P2. ���� DISPLAY
	if( szLogMsg ) {
		strLogLine += ":";
		strLogLine += szLogMsg;
	}
	//gPAPP->DisplayState(m_chidx, ChState, strLogLine );
	APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] %s", m_chnum, (LPCSTR)strLogLine );

	// P3. Shared Memory ����
	SetShmState(ChState);
}

void CFaxChThread::SetFaxChState( EN_STATUS enStatus, LPCSTR szFaxId, LPCSTR szSeq, LPCSTR szFaxNo, int nTryCnt, RESULT result )
{
	CString strStatusMsg, strLogMsg;

	switch( enStatus )
	{
	case FAX_SUCC_SEND:
	case FAX_FAIL_SEND:
		strStatusMsg.Format("FAX[%s]TRY[%d]RSSULT[%d]", szFaxNo, nTryCnt, result );
		strLogMsg.Format("[%s_%s] [%15s] Try[%d] [%03d]", szFaxId, szSeq, szFaxNo, nTryCnt, result );
		break;
	default:
		strStatusMsg.Format("FAX[%s]TRY[%d]", szFaxNo, nTryCnt );
		strLogMsg.Format("[%s_%s] [%15s] Try[%d]", szFaxId, szSeq, szFaxNo, nTryCnt );
		break;
	}
	SetFaxChState( enStatus, strStatusMsg, strLogMsg );
}



bool CFaxChThread::FaxChInit( int chidx, int chnum )
{
	int		nBTFAXRet;

	// p0. WM_FAXCH_INIT�� �޾��� �� Argument�� �Ѱܹ��� 0 Based FAX CH ��ȣ�� ����
	m_chidx = chidx;
	m_chnum = chnum;
	
	// p1. WM_FAXCH_INIT�� �޾��� �� HMP FAX CH Open (qid�� m_chidx��...)
	nBTFAXRet = HmpFaxOpenSync( m_chnum, m_chidx );
	if(nBTFAXRet)
	{
		APPLOG->Print(DBGLV_MAJOR, "[CHAN_%03d] HmpFaxOpenSync...FAIL reason[%s]", m_chnum, GetFaxError(nBTFAXRet));
		return false;
	}

	// P2. �ڽ��� ������ ���� g_FaxChInfo ����ü�� chnum�� IO_flag �ʱ�ȭ
	InitFaxChInfo();
	APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] HmpFaxOpenSync...SUCC! qid[%d]", m_chnum, m_chidx);

	// shared memory init
	SetShmState(FAX_INIT);

	// p3. FAX �߼��� �� �� �ִ� ���·� FAX CH ���� ����
	IdleFaxChInfo();

	return true;
}

// -------------------------------------------------------------------
// Module ��	: void SetFaxSendInfo()
// -------------------------------------------------------------------
// Descriotion	: FAX �߼� ���� ����
// -------------------------------------------------------------------
// Argument		: FAX_SEND_INFO* pFaxSendInfo;		�ѽ��߼�����
// -------------------------------------------------------------------
// Return ��	: void
// -------------------------------------------------------------------
void CFaxChThread::SetFaxSendInfo(CDbModule::SEND_REQ* pSendReq)
{
	if(m_chidx < 0 || m_chidx >= MAX_CHANNEL || m_chnum < 0)
		return;

	g_FaxChInfo[m_chidx].sendReq	= *pSendReq;
}


// -------------------------------------------------------------------
// Module ��	: void SetFaxResult()
// -------------------------------------------------------------------
// Descriotion	: FAX �߼۰�� ����
// -------------------------------------------------------------------
// Argument		: EN_RESULT FaxResult;		�߼� ���
// -------------------------------------------------------------------
// Return ��	: void
// -------------------------------------------------------------------
void CFaxChThread::SetFaxResult(RESULT result)
{
	if(m_chidx < 0 || m_chidx >= MAX_CHANNEL || m_chnum < 0)
		return;

	g_FaxChInfo[m_chidx].TryCnt++;
	g_FaxChInfo[m_chidx].Fax_result_cd	= result;

/*
	APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] FAX SEND! ID_REQ[%d_%d] FAX_NO[%s] TIFF[%s] B_DATE[%s] TRY[%d] RET[%d]",
								m_chnum,
								(LPCSTR) g_FaxChInfo[m_chidx].sendReq.detail.FAX_ID,
								(LPCSTR) g_FaxChInfo[m_chidx].sendReq.detail.SEQ,
								(LPCSTR) g_FaxChInfo[m_chidx].sendReq.detail.FAX_NO,
								(LPCSTR) g_FaxChInfo[m_chidx].sendReq.detail.TIF_FILE,
								(LPCSTR) g_FaxChInfo[m_chidx].sendReq.detail.beginTime,
								g_FaxChInfo[m_chidx].sendReq.detail.TRY_CNT,
								g_FaxChInfo[m_chidx].Fax_result_cd );*/
}

// -------------------------------------------------------------------
// Module ��	: bool InitFaxChInfo();
// -------------------------------------------------------------------
// Descriotion	: g_FaxChInfo[m_chidx]�� ��밡�� ���·� �����Ѵ�. shm�� �Բ� �ʱ�ȭ�Ѵ�.
//				: SharedMemory ������ �Բ� �ʱ�ȭ�Ѵ�.
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return ��	: void
// -------------------------------------------------------------------
void CFaxChThread::InitFaxChInfo()
{
	if( m_chidx >= CConfig::FAX_TOTAL_CH_CNT - CConfig::FAX_BROAD_CH_CNT)
		g_FaxChInfo[m_chidx].bBroadCast = true;

	g_FaxChInfo[m_chidx].chnum		= m_chnum;
	g_FaxChInfo[m_chidx].IO_flag	= IO_OUTBOUND;
}

// -------------------------------------------------------------------
// Module ��	: bool IdleFaxChInfo();
// -------------------------------------------------------------------
// Descriotion	: ���� �ѽ� �۽��� ���� �ڽ��� g_FaxChInfo[m_chidx]�� �ʱ�ȭ �Ѵ�.
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return ��	: void
// -------------------------------------------------------------------
void CFaxChThread::IdleFaxChInfo()
{
	g_FaxChInfo[m_chidx].idle();

	// Shared Memory SET 

	SetFaxChState( FAX_IDLE, NULL, NULL );
}

// -------------------------------------------------------------------
// Module ��	: long OnFaxSendStart();
// -------------------------------------------------------------------
// Descriotion	: WM_FAXSEND_START Message�� �޾��� �� ó�� (FAX �߼۽ø��� ����)
// -------------------------------------------------------------------
// Argument		: WPARAM wParam;
//				  LPARAM lParam;
// -------------------------------------------------------------------
// Return ��	: 
// -------------------------------------------------------------------
void CFaxChThread::onThreadEntry()
{
	RESULT	result;
	bool	bWaitAbort;
	
	while( !IsReqStop() )
	{
		// P1. ������ ���
		if( g_FaxChInfo[ m_chidx ].ch_state != FAX_OCCUPY )
		{
			Sleep( 500 );
			continue;
		}

		CDbModule::SEND_REQ& sendReq = g_FaxChInfo[m_chidx].sendReq;
		
		// �߼����� �ѽ���ȣ ó�� 
		CDbModule::Inst()->DbLock();
		if(CDbModule::Inst()->IsSendOffFaxNo(sendReq.detail.FAX_NO))
		{	 
			
			result = F_BLOCKED_FAX_NO;
			SetFaxResult( result );
			UpdateResult( sendReq, result );
			DeleteFile( sendReq.detail.tifFile_send );
			IdleFaxChInfo();
			continue;
		}
		CDbModule::Inst()->DbUnlock();
		
		SetFaxChState( FAX_OCCUPY, NULL, NULL);
		Sleep( 500 );

		// P2. �ѽ� ����
		result = Fax_DialAndSend( sendReq, &bWaitAbort );
		if( result == SUCCESS )
		{	
			SetFaxChState( FAX_SUCC_SEND, 
							sendReq.detail.FAX_ID,
							sendReq.detail.SEQ,
							sendReq.detail.FAX_NO,
							sendReq.detail.TRY_CNT, 
							result );
		}
		else
		{
			SetFaxChState( FAX_FAIL_SEND, 
							sendReq.detail.FAX_ID,
							sendReq.detail.SEQ,
							sendReq.detail.FAX_NO,
							sendReq.detail.TRY_CNT, 
							result );
		}
		SetFaxResult( result );
		Sleep( 1000 );

		// P3. ��� ������Ʈ
		UpdateResult( sendReq, result );

		if( bWaitAbort )
		{
			FAX_RECV_PACKET RcvPacket;
			CFsmIfThread::Inst()->RcvPacket_Wait( m_chnum, Fax_Abort, 60 * 1000, &RcvPacket );
		}
		
		// P4. ȸ�� ���� �ʱ�ȭ
		Sleep(3000);
		DeleteFile( sendReq.detail.tifFile_send );
		IdleFaxChInfo();
	}
}


RESULT CFaxChThread::Fax_DialAndSend( CDbModule::SEND_REQ& sendReq, bool* p_pbWaitAbort )
{
	RESULT			result = EMPTY;
	CString			strFromNo;
	CString			strPagesToSend, strNextPages;
	FAX_SEND_PACKET	SndPacket;
	FAX_RECV_PACKET	RcvPacket;
	

	strPagesToSend = sendReq.detail.PAGES_TO_SEND;

	//// P1. Set page to send

	// P1-2. Convert 1-base => 0-base, and "all" => "0-".
	CUtility::PagesToBase0( strPagesToSend );

	// P1-1. Set next pages
	if( sendReq.detail.LAST_PAGE_SENT <= 0 )
	{
		strNextPages = strPagesToSend;
	}
	else
	{	
		int nextPage = sendReq.detail.LAST_PAGE_SENT + 1;

		if( !CUtility::GetNextPages( nextPage - 1,	// 0-base
									strPagesToSend, &strNextPages ) )
			return SUCCESS;

		APPLOG->Print( DBGLV_RPT, "#### LAST:%d, NEXT:%d, NEXTS:%s", sendReq.detail.LAST_PAGE_SENT, nextPage, (LPCSTR)strNextPages );
	}
	
	SetFaxChState( FAX_DIAL, NULL, NULL);

	//// P2. Send 'Fax_ReqCalling'
	// P2-1. Make Packet
	
	SndPacket.h.msgid	= Fax_ReqCalling;
	SndPacket.h.len		= sizeof( SndPacket.b.call );
	
	strcpy_s( SndPacket.b.call.client_callid,	sendReq.detail.SEQ );

	if( CConfig::FAX_OUTBOUND_PREFIX == "X" || CConfig::FAX_OUTBOUND_PREFIX == "x" )
		strcpy_s( SndPacket.b.call.fax_target,	sendReq.detail.FAX_NO );
	else
		strcpy_s( SndPacket.b.call.fax_target,	CConfig::FAX_OUTBOUND_PREFIX + sendReq.detail.FAX_NO );

	if( sendReq.master.REQ_USER_TEL_NO.GetLength() > 0 )
		strFromNo = sendReq.master.REQ_USER_TEL_NO;
	else
		strFromNo = CConfig::FAX_DEFAULT_ANI;

	strcpy_s( SndPacket.b.call.fax_from, strFromNo );

	// P2-2. Send Packet
	CFsmIfThread::Inst()->SndPacket_Push( m_chidx, SndPacket );

	//// P3. Wait 'Fax_ReqCalling_Ack', 'Fax_Rx_Start'
	*p_pbWaitAbort = false;
	while( true )
	{
		// P2. Wait 'Fax_ReqCalling_Ack'
		if( !CFsmIfThread::Inst()->RcvPacket_Wait( m_chnum, 30000*1000, &RcvPacket ) )
			return F_SYSTEM_ERROR;

		switch( RcvPacket.h.msgid )
		{
		case Fax_ReqCalling_Ack:
			if( RcvPacket.b.call_ack.result != 0 )
				return F_SYSTEM_ERROR;
			continue;

		case Fax_Cancelled:
            APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] Fax_Cancelled msgid:[%d], ack_result:[%d], sip_reason:[%d]", m_chnum, RcvPacket.h.msgid, RcvPacket.b.call_ack.result, RcvPacket.b.cancelled.sip_reason );			
			return GetSIPResult(RcvPacket.b.cancelled.sip_reason);
            
		case Fax_Rx_Start:
            break;

		case Fax_Rx_Restart:
			break;
		
		case Fax_Abort:
			return F_FAX_ABORT;

		default:
            APPLOG->Print( DBGLV_RPT, "[CHAN_%03d]Invalid msg header! msgid:[%d], ack_result:[%d], sip_reason:[%d]", m_chnum, RcvPacket.h.msgid, RcvPacket.b.call_ack.result, RcvPacket.b.cancelled.sip_reason );
			return F_FAX_ABORT;
		}

		break;
	}
	*p_pbWaitAbort = true;

	strcpy_s( g_FaxChInfo[m_chidx].remoteIP, RcvPacket.b.start.gw_ip );
	g_FaxChInfo[m_chidx].remotePORT = RcvPacket.b.start.gw_port;

	SetFaxChState( FAX_SEND, NULL, NULL);

	//// P4. HMP(T.38) : Send FAX
	APPLOG->Print( DBGLV_RPT, 
					"[CHAN_%03d] Send Info [%s_%s] [%11s] Try[%d] To[%s:%d] Pages[%s] [%s]",
					m_chnum, 
					(LPCSTR) sendReq.detail.FAX_ID, 
					(LPCSTR) sendReq.detail.SEQ, 
					(LPCSTR) sendReq.detail.FAX_NO, 
					sendReq.detail.TRY_CNT,
					(LPCSTR) g_FaxChInfo[m_chidx].remoteIP, 
					g_FaxChInfo[m_chidx].remotePORT, 
					(LPCSTR) strNextPages,
					(LPCSTR) sendReq.detail.tifFile_send );
	
	result = SendT38( g_FaxChInfo[m_chidx].remoteIP, 
						g_FaxChInfo[m_chidx].remotePORT, 
						sendReq,
						(LPCSTR) strNextPages );

	//// P5. Send 'Fax_Complete'
	SndPacket.h.msgid	= Fax_Complete;
	SndPacket.h.len		= 0;
	CFsmIfThread::Inst()->SndPacket_Push( m_chidx, SndPacket );

	return result;
}

RESULT CFaxChThread::SendT38( const char* szRemoteIp, int nRemotePort, CDbModule::SEND_REQ& sendReq, const char* p_szPages )
{
	int				nResult;
	CString			strFrom, strTo;
	send_tiff_info	sndTif;
	ipc_fax_state	faxState;

	// P13. FAX ���� ����
	memset(&sndTif, 0x00 , sizeof(send_tiff_info));

	// FAX �߼� Page�� ��ܿ� Display�� �ð�, FROM, TO, Page ������ �����Ѵ�.
	sndTif.h.ovl.overwrite	= 1;							// Append Mode : 0, Overwrite Mode : 1
	sndTif.h.ovl.align		= CConfig::OVERLAY_ALIGN;		// Center : 0, Left : 1, Right : 2
	sndTif.h.ovl.xoff		= CConfig::OVERLAY_X_OFFSET;	// center align������ �ǹ� ����. Left�� ��� ���ʿ������� offset, right�� ��� �����ʿ������� offset
	sndTif.h.ovl.yoff		= CConfig::OVERLAY_Y_OFFSET;	// high resolution ���� ���������� offset

	// From : �߽����̸�[�ִ�20 Byte] + ( �߽��� ��ȣ �Ǵ� �⺻�� )
	strFrom = "";
	
	if( sendReq.master.REQ_USER_NAME.GetLength() > 0 )
		strFrom = sendReq.master.REQ_USER_NAME.Left( 20 ) + " ";

	if( sendReq.master.REQ_USER_TEL_NO.GetLength() > 0 )
		strFrom += sendReq.master.REQ_USER_TEL_NO;
	else
		strFrom += CConfig::FAX_DEFAULT_ANI;

	//To : �������̸�[�ִ�20 Byte] + ������ ��ȣ
	strTo = "";
	
	if( sendReq.detail.RECIPIENT_NAME.GetLength() > 0 )
		strTo = sendReq.detail.RECIPIENT_NAME.Left( 20 ) + " ";
	
	if( sendReq.detail.FAX_NO.GetLength() > 0 )
		strTo += sendReq.detail.FAX_NO;

	// TIF ��� ���� : �������� ����� ����Ҷ��� ó��
	if(CConfig::OVERLAY_YN == "Y")
	{
		sprintf_s( sndTif.h.ovl.text, 
					" $T(YYYY/MM/DD hh:mm:ss APM WWW)    FROM: %s    TO: %s    PAGE: $p/$P ",
					(LPCSTR)strFrom, (LPCSTR)strTo );
	}
	
	// �ѽ� �߼�
	strcpy_s( sndTif.h.ip, szRemoteIp );
	sndTif.h.port = nRemotePort;
	sndTif.h.redundancy_level = -1;
	strcpy_s( sndTif.f[0].path , sendReq.detail.tifFile_send );
	sndTif.cnt = 1;
	strcpy_s( sndTif.f[0].page, p_szPages );

	nResult = HmpFaxSendSync(m_chnum, &sndTif, &faxState);							
	if( faxState.page > 0 )
		sendReq.detail.LAST_PAGE_SENT += faxState.page;

	// P14. �������� ����
	if( nResult != 0 )
	{	
		APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] HmpFaxSendSync() Fail! (fax_id:%s_%s, try:%d, result:%d msg:%s)",
									m_chnum,									
									(LPCSTR) sendReq.detail.FAX_ID,
									(LPCSTR) sendReq.detail.SEQ,
									g_FaxChInfo[m_chidx].TryCnt,
									nResult,
									GetFaxError(nResult)
									);

		switch(nResult)
		{
			case Fax_Error_Not_Opened					: return F_FAX_HMP_NOT_OPENED;
			case Fax_Error_License						: return F_FAX_LICENSE;				
			case Fax_Error_Invalid_Channel				: return F_FAX_INVALID_CHANNEL;
			case Fax_Error_Fax_Reset					: return F_FAX_RESET;
			case Fax_Error_Fax_Already_Opened			: return F_FAX_ALREADY_OPENED;
			case Fax_Error_Fax_Already_Stopped			: return F_FAX_ALREADY_STOPPED;
			case Fax_Error_Device_Busy					: return F_FAX_DEVICE_BUSY;
			case Fax_Error_Socket_Fail					: return F_FAX_SOCKET_FAIL;
			case Fax_Error_Bad_File						: return F_FAX_BAD_FILE_FORMAT;
			case Fax_Error_Abort						: return F_FAX_ABORT;
			case Fax_Error_Open_To_Stop_Active_Channel	: return F_FAX_OPEN_TO_STOP_ACTIVE_CHANNEL;
			case Fax_Error_Close_To_Stop_Active_Channel	: return F_FAX_CLOSE_TO_STOP_ACTIVE_CHANNEL;
			default										: return F_FAX_TRANS_ERROR;
		}
	}
	
	//P15. GW�κ��� state ����
	if( faxState.error != 0 )
	{
		APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] faxState.error! (fax_id:%s_%s, try:%d page_cnt:%d result:%d, faxState.error:%d, FaxGetTxStateName:%s",
									m_chnum,									
									(LPCSTR) sendReq.detail.FAX_ID,
									(LPCSTR) sendReq.detail.SEQ,
									g_FaxChInfo[m_chidx].TryCnt,
									faxState.page,
									nResult,
                                    faxState.error,
									HmpFaxGetTxStateName(faxState.state)
									);

		// �κ� ����
		if(faxState.page >= 1)
        {
            APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] Partial Page Sent! (fax_id:%s_%s, try:%d page_cnt:%d result:%d msg:%s",
									m_chnum,									
									(LPCSTR) sendReq.detail.FAX_ID,
									(LPCSTR) sendReq.detail.SEQ,
									g_FaxChInfo[m_chidx].TryCnt,
									faxState.page,
									faxState.error,
									HmpFaxGetTxStateName(faxState.state)
									);

			return F_FAX_PARTIAL_PAGE_SENT;
        }

		switch(faxState.error)
		{
			case Fax_Tx_Fail_Dis_Timeover		: return F_FAX_TX_FAIL_DIS_TIMEOVER;
			case Fax_Tx_Fail_Train              : return F_FAX_TX_FAIL_TRAIN;
			case Fax_Tx_Fail_Rtn                : return F_FAX_TX_FAIL_RTN;
			case Fax_Tx_Fail_Ctc_Error          : return F_FAX_TX_FAIL_CTC_ERROR;
			case Fax_Tx_Fail_Cfr_Timeout        : return F_FAX_TX_FAIL_CFR_TIMEOUT;
			case Fax_Tx_Fail_Mcf_Timeout        : return F_FAX_TX_FAIL_MCF_TIMEOUT;
			case Fax_Tx_Fail_Mcf_Last_Timeout   : return F_FAX_TX_FAIL_MCF_LAST_TIMEOUT;
			case Fax_Tx_Fail_Ctr_Timeout        : return F_FAX_TX_FAIL_CTR_TIMEOUT;
			case Fax_Tx_Fail_Dis_In_Image       : return F_FAX_TX_FAIL_DIS_IN_IMAGE;
			case Fax_Tx_Fail_Dis_In_Waiting_Mcf : return F_FAX_TX_FAIL_DIS_IN_WAITING_MCF;
			case Fax_Tx_Fail_Dcn_Recv         	: return F_FAX_TX_FAIL_DCN_RECV;
			case Fax_Tx_Fail_Local_Interrupt    : return F_FAX_TX_FAIL_LOCAL_INTERRUPT;
			case Fax_Tx_Fail_Pin_Recv           : return F_FAX_TX_FAIL_PIN_RECV;
			case Fax_Tx_Fail_Pip_Recv           : return F_FAX_TX_FAIL_PIP_RECV;
			case Fax_Tx_Fail_Flow_Control       : return F_FAX_TX_FAIL_FLOW_CONTROL;
			case Fax_Tx_Fail_Stop             	: return F_FAX_TX_FAIL_STOP;
			default								: return F_FAX_TRANS_ERROR;
		}
	}

	return SUCCESS;
}

void CFaxChThread::UpdateResult(CDbModule::SEND_REQ& sendReq, RESULT result)
{
	int				dbRet;									// DB ���� ���
	int				effectCnt;

	CString			connStr, id, pwd;

	bool bRetry = false;

    int retryCnt = 0;
    int	 delayTime = 0; // [sec]
    if(CConfig::TRY_CNT_BUSY >= CConfig::TRY_CNT_NOANSWER)
    {
        retryCnt = CConfig::TRY_CNT_BUSY;
        delayTime = CConfig::TRY_DELAY_BUSY;
    }
    else
    {
        retryCnt = CConfig::TRY_CNT_NOANSWER;
        delayTime = CConfig::TRY_DELAY_NOANSWER;
    }

	switch(result)
	{
        case SUCCESS:                       // ����        
        case F_FAX_TX_FAIL_STOP:            // ����ڿ� ���� ����
        case F_BLOCKED_FAX_NO:              // �۽����ܹ�ȣ
            //sendReq.detail.TRY_CNT++;
			bRetry = false;
			break;

        // �̵�����Ʈ���� ������ ��õ� ���� ����.
        case  F_FAX_SIP_SERVER_INTERNAL_ERROR:
        case  F_FAX_SIP_NOT_IMPLEMENTED:
        case  F_FAX_SIP_BAD_GATEWAY:
        case  F_FAX_SIP_SERVICE_UNAVAILABLE:
        case  F_FAX_SIP_SERVER_TIME_OUT:
        case  F_FAX_SIP_VERSION_NOT_SUPPORTED:
        case  F_FAX_SIP_MESSAGE_TOO_LARGE:
        case  F_FAX_SIP_PRECONDITION_FAILURE:            
            if(sendReq.detail.TRY_CNT >= retryCnt)
                --sendReq.detail.TRY_CNT;

            bRetry = true;
            break;

        // ��Ÿ ���н� ��õ�
        default:            
            if(sendReq.detail.TRY_CNT < retryCnt)
            {
                //sendReq.detail.TRY_CNT++;
				bRetry = true;
            }
            break;
	}

	CDbModule::Inst()->DbLock();
	if( bRetry )
	{
		effectCnt = CDbModule::Inst()->RetrySendReqEx( sendReq, m_chnum, result, delayTime );
	}
	else
	{	
		effectCnt = CDbModule::Inst()->FinishSendReqEx( sendReq
														, g_FaxChInfo[m_chidx].remoteIP
														, g_FaxChInfo[m_chidx].remotePORT
														, m_chnum
                                                        , result );
	}
	CDbModule::Inst()->DbUnlock();

	if(effectCnt <= 0)
	{
		APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] P_FINISH_SEND_REQ() Fail[%d]! FAX_ID[%s] FAX_ID_DTL[%s] TRY[%d] RESULT[%d]",
									m_chnum,
									effectCnt,
									(LPCSTR) sendReq.detail.FAX_ID,
									(LPCSTR) sendReq.detail.SEQ,
									sendReq.detail.TRY_CNT,
									g_FaxChInfo[m_chidx].Fax_result_cd);
	}
	else
	{
        if(bRetry)
        {
            APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] PKG_PRC_FOD.USP_RETRY_SEND_REQ_EX() SUCC[%d]! FAX_ID[%s] FAX_ID_DTL[%s] TRY[%d] RESULT[%d]",
									    m_chnum,
									    effectCnt,
									    (LPCSTR) sendReq.detail.FAX_ID,
									    (LPCSTR) sendReq.detail.SEQ,
									    sendReq.detail.TRY_CNT,
									    g_FaxChInfo[m_chidx].Fax_result_cd);
        }
        else
        {
		    APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] P_FINISH_SEND_REQ() SUCC[%d]! FAX_ID[%s] FAX_ID_DTL[%s] TRY[%d] RESULT[%d]",
									    m_chnum,
									    effectCnt,
									    (LPCSTR) sendReq.detail.FAX_ID,
									    (LPCSTR) sendReq.detail.SEQ,
									    sendReq.detail.TRY_CNT,
									    g_FaxChInfo[m_chidx].Fax_result_cd);
        }
	}
}


RESULT CFaxChThread:: GetSIPResult(int nSipReasonCode)
{
    switch(nSipReasonCode)
    {
        case 400 : return F_FAX_SIP_BAD_REQUEST;
        case 401 : return F_FAX_SIP_UNAUTHORIZED;
        case 402 : return F_FAX_SIP_PAYMENT_REQUIRED;
        case 403 : return F_FAX_SIP_FORBIDDEN;
        case 404 : return F_FAX_SIP_NOT_FOUND;
        case 405 : return F_FAX_SIP_METHOD_NOT_ALLOWED;
        case 406 : return F_FAX_SIP_NOT_ACCEPTABLE;
        case 407 : return F_FAX_SIP_PROXY_AUTHENTICATION_REQUIRED;
        case 408 : return F_FAX_SIP_REQUEST_TIMEOUT;
        case 409 : return F_FAX_SIP_CONFLICT;
        case 410 : return F_FAX_SIP_GONE;
        case 411 : return F_FAX_SIP_LENGTH_REQUIRED;
        case 412 : return F_FAX_SIP_CONDITIONAL_REQUEST_FAILED;
        case 413 : return F_FAX_SIP_REQUEST_ENTITY_TOO_LARGE;
        case 414 : return F_FAX_SIP_REQUEST_URI_TOO_LONG;
        case 415 : return F_FAX_SIP_UNSUPPORTED_MEDIA_TYPE;
        case 416 : return F_FAX_SIP_UNSUPPORTED_URI_SCHEME;
        case 417 : return F_FAX_SIP_UNKNOWN_RESOURCE_PRIORITY;
        case 420 : return F_FAX_SIP_BAD_EXTENSION;
        case 421 : return F_FAX_SIP_EXTENSION_REQUIRED;
        case 422 : return F_FAX_SIP_SESSION_INTERVAL_TOO_SMALL;
        case 423 : return F_FAX_SIP_INTERVAL_TOO_BRIEF;
        case 424 : return F_FAX_SIP_BAD_LOCATION_INFORMATION;
        case 428 : return F_FAX_SIP_USE_IDENTITY_HEADER;
        case 429 : return F_FAX_SIP_PROVIDE_REFERRER_IDENTITY;
        case 430 : return F_FAX_SIP_FLOW_FAILED;
        case 433 : return F_FAX_SIP_ANONYMITY_DISALLOWED;
        case 436 : return F_FAX_SIP_BAD_IDENTITY_INFO;
        case 437 : return F_FAX_SIP_UNSUPPORTED_CERTIFICATE;
        case 438 : return F_FAX_SIP_INVALID_IDENTITY_HEADER;
        case 439 : return F_FAX_SIP_FIRST_HOP_LACKS_OUTBOUND_SUPPORT;
        case 470 : return F_FAX_SIP_CONSENT_NEEDED;
        case 480 : return F_FAX_SIP_TEMPORARILY_UNAVAILABLE;
        case 481 : return F_FAX_SIP_CALL_TRANSACTION_DOES_NOT_EXIST;
        case 482 : return F_FAX_SIP_LOOP_DETECTED;
        case 483 : return F_FAX_SIP_TOO_MANY_HOPS;
        case 484 : return F_FAX_SIP_ADDRESS_INCOMPLETE;
        case 485 : return F_FAX_SIP_AMBIGUOUS;
        case 486 : return F_FAX_SIP_BUSY_HERE;
        case 487 : return F_FAX_SIP_REQUEST_TERMINATED;
        case 488 : return F_FAX_SIP_NOT_ACCEPTABLE_HERE;
        case 489 : return F_FAX_SIP_BAD_EVENT;
        case 491 : return F_FAX_SIP_REQUEST_PENDING;
        case 493 : return F_FAX_SIP_UNDECIPHERABLE;
        case 494 : return F_FAX_SIP_SECURITY_AGREEMENT_REQUIRED;
        case 500 : return F_FAX_SIP_SERVER_INTERNAL_ERROR;
        case 501 : return F_FAX_SIP_NOT_IMPLEMENTED;
        case 502 : return F_FAX_SIP_BAD_GATEWAY;
        case 503 : return F_FAX_SIP_SERVICE_UNAVAILABLE;
        case 504 : return F_FAX_SIP_SERVER_TIME_OUT;
        case 505 : return F_FAX_SIP_VERSION_NOT_SUPPORTED;
        case 513 : return F_FAX_SIP_MESSAGE_TOO_LARGE;
        case 580 : return F_FAX_SIP_PRECONDITION_FAILURE;
        default: return F_FAX_TRANS_ERROR;
    }
}






