#include "stdafx.h"
#include <Windows.h>
#include <Mmsystem.h>
#include "TCP_Server.h"
#include "Utility.h"
#include "SIP_Manager.h"
#include "SIP_Utility.h"
#include "Config.h"
#include "timer.h"


unsigned int	mili, last;

int		maxfd;											// MAX socket 개수
fd_set	readfds, allset;								// 연동할 socket에 대한 fd_set

int		sock_ipc;										// 
int		sock_agent;										// SIP_FID와 연동할 socket

int		TcpServerInit(int localport)
{
	struct sockaddr_in bindaddr;
	struct linger	linger;
	int		i, sock;

	// ---------------------------------------------------------------------------
	// P0. 초기화
	// ---------------------------------------------------------------------------
	FD_ZERO(&allset);

	for( i = CConfig::ADDRESS_CLIENT_SESSION_BEGIN ; i <= CConfig::ADDRESS_CLIENT_SESSION_END ; i++ )
		memset( &shmem->r[i], 0, sizeof(shmem->r[0]) );


	// ---------------------------------------------------------------------------
	// P1. SIP_FID와 연동할 TCP Server Socket 생성
	// ---------------------------------------------------------------------------
	sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( sock < 0 )
	{
		APPLOG->Print( DBGLV_ERR, "CHAN[   ][TCP: : ] can't create socket." );
		return	-1;
	}

	// ---------------------------------------------------------------------------
	// P2. linger 설정
	// ---------------------------------------------------------------------------
	linger.l_onoff=1;
	linger.l_linger=0;
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&linger, sizeof(linger));

	// ---------------------------------------------------------------------------
	// P3. 송/수신 buffer를 128KBytes로 설정
	// ---------------------------------------------------------------------------
	i=0x20000;
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&i, sizeof(i));
	i=0x20000;
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&i, sizeof(i));

	// ---------------------------------------------------------------------------
	// P4. bind 처리
	// ---------------------------------------------------------------------------
	memset( (char *)&bindaddr, 0, sizeof(bindaddr) );
	bindaddr.sin_family		 = AF_INET;
	bindaddr.sin_addr.s_addr = INADDR_ANY;
	bindaddr.sin_port		 = htons(localport);
	i = bind( sock, (struct sockaddr *)&bindaddr, sizeof(bindaddr) );
	if( i < 0 )
	{
		APPLOG->Print( DBGLV_ERR, "CHAN[   ][TCP: : ] fail to bind. localport(%d) socket(%d) result = %d", localport, sock, i );
		closesocket(sock);
		return	-1;
	}
	// ---------------------------------------------------------------------------
	// P5. listen 처리
	// ---------------------------------------------------------------------------
	listen( sock, 5 );
	APPLOG->Print( DBGLV_RPT, "CHAN[   ][TCP: : ] Listening. port(%d)", localport );

	return	sock;
}

int	FaxAccept()
{
	struct sockaddr_in peer;
	int		len=sizeof(peer);
	uchar	* ip;
	int		i, sock;

	// ---------------------------------------------------------------------------
	// P1. accept 처리
	// ---------------------------------------------------------------------------
	sock = accept( sock_agent, (struct sockaddr *)&peer, &len );
	if( sock < 0 )
	{
		APPLOG->Print( DBGLV_ERR, "CHAN[   ][TCP: : ] fail to accept. result(%d)", sock );
		return	-1;
	}

	// ---------------------------------------------------------------------------
	// P2. fasip_in.exe에게 접속한 SIP_FID의 정보를 저장할 공간 확보
	// ---------------------------------------------------------------------------
	for( i = CConfig::ADDRESS_CLIENT_SESSION_BEGIN ; ; i++ )
	{
		if(!shmem->r[i].start_time)
			break;

		if( i >= CConfig::ADDRESS_CLIENT_SESSION_END )
		{
			APPLOG->Print( DBGLV_ERR, "CHAN[   ][TCP: : ] connection full. begin(%d) end(%d)", 
							CConfig::ADDRESS_CLIENT_SESSION_BEGIN, CConfig::ADDRESS_CLIENT_SESSION_END );
			closesocket(sock);
			return	-1;
		}
	}
	
	// ---------------------------------------------------------------------------
	// P3. 주어진 위체에 SIP_FID 정보 설정
	// ---------------------------------------------------------------------------
	shmem->r[i].accept = sock;

	ip=(uchar *)(&peer.sin_addr.s_addr);
	APPLOG->Print( DBGLV_RPT, "CHAN[   ][TCP:%d:A] accepted client(%d.%d.%d.%d:%d)", i, ip[0], ip[1], ip[2], ip[3], htons(peer.sin_port) );

	shmem->r[i].ip			= peer.sin_addr.s_addr;
	shmem->r[i].port		= htons(peer.sin_port);
	shmem->r[i].start_time	= time(0);
	shmem->r[i].last		= time(0);

	return	i;
}

void	CheckAlive()
{
	time_t	tCurrent = time( NULL );

	for( int i = CConfig::ADDRESS_CLIENT_SESSION_BEGIN ; i <= CConfig::ADDRESS_CLIENT_SESSION_END ; i++ )
	{
		if( shmem->r[i].start_time == 0 )
			continue;

		if( CConfig::ADDRESS_CLIENT_ALIVE_CHK_TIME > 0 
			&&  tCurrent - shmem->r[i].last >= CConfig::ADDRESS_CLIENT_ALIVE_CHK_TIME  )
		{
			ResetDevRegistered( i, &shmem->r[i].f );
			FaxDisconnect( i );
		}
	}
}

int		FaxRead(int i)
{
	fax_sip_control * m;
	int		* state, * ptr, len, reqlen;
	uchar	* ip;
	char	* p;

	m=&shmem->r[i].m;
	state=&shmem->r[i].state;
	ptr=&shmem->r[i].ptr;

	if( *state == 0 )								// Header 수신 처리
	{
		reqlen	= (int)(((fax_sip_control *)0)->para) - *ptr;
		p		= (char *)m + *ptr;
	}
	else											// Body 수신 처리
	{
		// ---------------------------------------------------------------------------
		// fax_sip_control 구조체의 msgid부터 len까지 길이
		//		(((fax_sip_control *)0)->para)
		//			-> fax_sip_control 구조체의 시작위치가 0일 경우 para 시작위치는 12
		//			-> *ptr의 초기 값은 0이므로 12에서 0빼면 12
		//		즉 초기 reqlen의 길이는 para를 뺀 header부 길이가 됨
		// ---------------------------------------------------------------------------
		reqlen	= m->len  - *ptr;
		p		= m->para + *ptr;
	}
	len = recv(shmem->r[i].accept, p, reqlen, 0);
	
	ip=(uchar *)&shmem->r[i].ip;

	//APPLOG->Print( DBGLV_INF, "CHAN[   ][TCP:%d:R] . Receive, ip=%d.%d.%d.%d, len=%d, packet=%s", i, ip[0], ip[1], ip[2], ip[3], len , p);
    
	/*if( CConfig::SYSTEM_LOG_LEVEL >= 5 )
		HexaDump(-1, len, (uchar *)p);*/

	if( len < 0 )
		return len;
	*ptr += len;

	// ---------------------------------------------------------------------------
	// 마지막 전문 받은 시간 설정
	// ---------------------------------------------------------------------------
	shmem->r[i].last = time( NULL );

	// ---------------------------------------------------------------------------
	// Header나 Body Data를 reqlen만큼 다 못 읽었으면
	// ---------------------------------------------------------------------------
	if( len < reqlen )
		return len;

	(*state)++;

	// ---------------------------------------------------------------------------
	// 처음 전문 받음
	// ---------------------------------------------------------------------------
	if( *state == 1 )
	{
		// ---------------------------------------------------------------------------
		// 전문의 para길이 field값 오류 처리
		// ---------------------------------------------------------------------------
		if( m->len < 0 || m->len > sizeof(m->para) )
		{
			APPLOG->Print( DBGLV_ERR, "CHAN[   ][TCP:%d:R] received wrong message length, len=%d", i, m->len );
			return -2;
		}
		*ptr = 0;
		// ---------------------------------------------------------------------------
		// 전문의 para길이 field값이 0인 경우 (para data가 없는 경우)
		// ---------------------------------------------------------------------------
		if( m->len == 0 )
			(*state)++;
	}

	// ---------------------------------------------------------------------------
	// 나머지 Data를 받았으므로 SIP_FID로부터 수신받은 Message 처리
	// ---------------------------------------------------------------------------
	if( *state == 2 )
	{
		DecodeFaxMsg( i );
		*state	= 0;
		*ptr	= 0;
	}

	return	len;
}

// ---------------------------------------------------------------------------
// Module 명		: void FaxDisconnected()
// ---------------------------------------------------------------------------
// Description	: SIP_FID와의 연결 해제
// ---------------------------------------------------------------------------
// Argument		: int i;		Index
//				  int cause;	연결해제 사유
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
void	FaxDisconnected( int i, int cause )
{
	uchar	* ip;

	ip = (uchar *)&shmem->r[i].ip;
	APPLOG->Print( DBGLV_RPT, "CHAN[   ][TCP:%d:C] disconnected. addr=%d.%d.%d.%d:%d, cause=%d.", i, ip[0], ip[1], ip[2], ip[3], shmem->r[i].port, cause );

	// ---------------------------------------------------------------------------
	// 해당 socket을 fd_set에서 clear
	// maxfd값 재설정
	// socket close
	// ---------------------------------------------------------------------------
	FD_CLR(shmem->r[i].accept, &allset);
	LowerMaxFd(shmem->r[i].accept);
	closesocket(shmem->r[i].accept);

	// ---------------------------------------------------------------------------
	// Regist 정보 clear
	// ---------------------------------------------------------------------------
	ResetDevRegistered( i, &shmem->r[i].f );

	// ---------------------------------------------------------------------------
	// Shared Memory에서 정보 삭제
	// ---------------------------------------------------------------------------
	memset( &(shmem->r[i]), 0, sizeof(shmem->r[0]) );
}

void	FaxDisconnect( int i )
{
	uchar	* ip;

	ip = (uchar *)&shmem->r[i].ip;
	APPLOG->Print( DBGLV_RPT, "CHAN[   ][TCP:%d:C] disconnect. addr=%d.%d.%d.%d:%d.", i, ip[0], ip[1], ip[2], ip[3], shmem->r[i].port );

	// ---------------------------------------------------------------------------
	// 해당 socket을 fd_set에서 clear
	// maxfd값 재설정
	// socket close
	// ---------------------------------------------------------------------------
	FD_CLR(shmem->r[i].accept, &allset);
	LowerMaxFd(shmem->r[i].accept);
	closesocket(shmem->r[i].accept);

	// ---------------------------------------------------------------------------
	// Regist 정보 clear
	// ---------------------------------------------------------------------------
	ResetDevRegistered( i, &shmem->r[i].f );

	// ---------------------------------------------------------------------------
	// Shared Memory에서 정보 삭제
	// ---------------------------------------------------------------------------
	memset( &(shmem->r[i]), 0, sizeof(shmem->r[0]) );
}

// ---------------------------------------------------------------------------
// Module 명		: void DecodeFaxMsg()
// ---------------------------------------------------------------------------
// Description	: SIP_FID로부터 받은 전문 정보 저장 및 Display
// ---------------------------------------------------------------------------
// Argument		: int i;		접속한 SIP_FID 정보가 저장되어 있는 index
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
void	DecodeFaxMsg(int i)
{
	switch( shmem->r[i].m.msgid )
	{
	/////// TCP Session
	case Fax_Keep_Alive_Fax:
		FaxWrite(i, Fax_Keep_Alive_Sip, -1, 0, 0);
		break;

	case Fax_Register:
		{
			fax_register * f;
			fax_register_ack a;

			f = (fax_register *)shmem->r[i].m.para;
			APPLOG->Print( DBGLV_RPT, "CHAN[   ][TCP:%d:R] Fax_Register. dir=%c, ip=%s, port=%d, spacing=%d, dev=%d(+%d).", i, f->fax_direction, f->fax_ip, f->fax_base_port, f->fax_port_spacing, f->fax_device_start, f->fax_device_cnt );
			// ---------------------------------------------------------------------------
			// Fax_Register Data 저장 (FAX IP, Base Port, Port Spacing, 시작 채널, 채널 개수)
			// ---------------------------------------------------------------------------
			shmem->r[i].f=*f;

			a.result = SetDevRegistered( f, i );
			APPLOG->Print( DBGLV_RPT, "CHAN[   ][TCP:%d:S] Fax_Register_Ack. result=%d.", i, a.result );
			FaxWrite( i, Fax_Register_Ack, -1, sizeof(a), (char *)&a );
		}

		break;

	/////// INBOUND
	case Fax_Complete:
		APPLOG->Print( DBGLV_RPT, "CHAN[%03d][TCP:%d:R] Fax_Complete.", shmem->r[i].m.chan, i );
		shmem->d[shmem->r[i].m.chan].fstate = 0;
		SIP_Manager::inst->DisconnectChann( shmem->r[i].m.chan );
		break;

	/////// OUTBOUND
	case Fax_ReqCalling:
		{
			fax_call*		f;
			fax_call_ack	a;

			f = (fax_call *)shmem->r[i].m.para;
            APPLOG->Print( DBGLV_RPT, "CHAN[%03d][TCP:%d:R] Fax_ReqCalling. target=%s, from=%s", shmem->r[i].m.chan, i, f->fax_target, f->fax_from);
			shmem->d[shmem->r[i].m.chan].fstate = 0;

			if( SIP_Manager::inst->CallChann( shmem->r[i].m.chan, f->fax_target, f->fax_from, f->client_callid) )
				a.result = 0;
			else
				a.result = -1;

			APPLOG->Print( DBGLV_RPT, "[CHAN_%03d][TCP:%d:S] Fax_ReqCalling_Ack. result=%d.", shmem->r[i].m.chan, i, a.result );
			FaxWrite(i, Fax_ReqCalling_Ack, shmem->r[i].m.chan, sizeof(a), (char *)&a);
		}
		break;
	}
}

// ---------------------------------------------------------------------------
// Module 명		: void FaxWrite()
// ---------------------------------------------------------------------------
// Description	: SIP_FID로 전문 송신
// ---------------------------------------------------------------------------
// Argument		: int i;		접속한 SIP_FID 정보가 저장되어 있는 index
//				  int msgid;	Msg ID
//				  int chan;		FAX CH
//				  int len;		para 길이
//				  char * para;	para Data
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
void	FaxWrite(int i, int msgid, int chan, int len, char * para)
{
	fax_sip_control m;

	if(i<0)
		return;

	m.msgid=msgid;
	m.chan=chan;
	m.len=len;
	if(len)
		memcpy(m.para, para, len);

	//send(shmem->r[i].accept, (char *)&m, len+12, 0);
	//APPLOG->Print( DBGLV_RPT, "FaxWrite[%d]", sizeof(fax_sip_control) - sizeof(m.para));
	send(shmem->r[i].accept, (char *)&m, len + sizeof(fax_sip_control) - sizeof(m.para), 0);
}

// ---------------------------------------------------------------------------
// Module 명		: void SetDevRegistered()
// ---------------------------------------------------------------------------
// Description	: SIP_FID로터 Fax_Register 전문을 받아 처리한 후 Register 상태로 등록
// ---------------------------------------------------------------------------
// Argument		: fax_register * r;		Fax_Register  전문의 Data
//				  int id;				접속한 SIP_FID 정보가 저장되어 있는 index
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
int	SetDevRegistered( fax_register * r, int id )
{
	int	s, e, result;

	// ---------------------------------------------------------------------------
	// Fax_Register 결과 설정 및 송신
	// ---------------------------------------------------------------------------

	s = r->fax_device_start;
	e = r->fax_device_start + r->fax_device_cnt - 1;
	if( e + 1 > MAXDEVICE )
		e = MAXDEVICE - 1;

	// ---------------------------------------------------------------------------
	// 채널값 확인
	// ---------------------------------------------------------------------------
	if( s < 0 || s >= MAXDEVICE )
		return -1;

	else if( e <= s )
		return -2;

	// ---------------------------------------------------------------------------
	// FAX CH 정보에 SIP_FID Register 정보가 있는 Array의 index+1을 설정
	// ---------------------------------------------------------------------------
	for( int i = s ; i <= e ; i++ )
	{
		shmem->d[i].reg = id + 1;
		shmem->d[i].call_direction = r->fax_direction;
	}

	// ---------------------------------------------------------------------------
	// SIP ChannelHunter 에 채널 등록
	// ---------------------------------------------------------------------------
	result = SIP_ChannelHunter::Inst()->RegistClient( id, r->fax_direction, s, e );
	if( result != 0 )
		result += -10;

	return result;
}

// ---------------------------------------------------------------------------
// Module 명		: void ResetDevRegistered()
// ---------------------------------------------------------------------------
// Description	: SIP_FID와 접속이 끊어졌을 때 UnRegister 처리
// ---------------------------------------------------------------------------
// Argument		: fax_register * r;		Fax_Register  전문의 Data
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
void	ResetDevRegistered( int id, fax_register * r )
{
	int		i, s, e;

	SIP_ChannelHunter::Inst()->UnregistClient( id );

	// ---------------------------------------------------------------------------
	// 시작 채널 확인
	// ---------------------------------------------------------------------------
	if( r->fax_device_start<0 || r->fax_device_start>=MAXDEVICE )
		return;

	// ---------------------------------------------------------------------------
	// 시작 채널이 종료 CH보다 작은지 확인
	// ---------------------------------------------------------------------------
	s=r->fax_device_start;
	e=r->fax_device_start+r->fax_device_cnt;
	if( e <= s )
		return;

	// ---------------------------------------------------------------------------
	// 종료채널이 MAXDEVICE(960)보다 크면 MAXDEVICE로 재설정
	// ---------------------------------------------------------------------------
	if( e > MAXDEVICE )
		e = MAXDEVICE;

	// ---------------------------------------------------------------------------
	// FAX CH 정보에 SIP_FID Register 정보가 있는 Array index 정보를 reset
	// ---------------------------------------------------------------------------
	for( i=s ; i < e ; i++ )
	{
		shmem->d[i].reg=0;

		/*if(shmem->d[i].leg)
			SipDisc(i);*/
	}
}

void	LowerMaxFd(int fd)
{
	int		i, max;

	// ---------------------------------------------------------------------------
	// fd가 maxfd가 아니면 return
	// ---------------------------------------------------------------------------
	if(fd!=maxfd)
		return;

	max=0;

	// ---------------------------------------------------------------------------
	// fd가 famon.exe와 연동하는 socket이 아니고 famon.exe와 연동중이면
	// ---------------------------------------------------------------------------
	if(sock_ipc!=fd && sock_ipc>max)
		max=sock_ipc;

	// ---------------------------------------------------------------------------
	// fd가 SIP_FID와 연동하는 listen socket이 아니고 listen socket이 famon.exe와 연동하는 socket보다 크면
	// ---------------------------------------------------------------------------
	if(sock_agent!=fd && sock_agent>max)
		max=sock_agent;

	// ---------------------------------------------------------------------------
	// 현재 fasip_in.exe와 연결된 socket들 중 fd를 제외하고 가장 큰 socket 값을 찾는다
	// ---------------------------------------------------------------------------
	for( i = CConfig::ADDRESS_CLIENT_SESSION_BEGIN ; i <= CConfig::ADDRESS_CLIENT_SESSION_END ; i++ )
	{
		if( shmem->r[i].accept!=fd && shmem->r[i].accept > max )
			max = shmem->r[i].accept;
	}

	maxfd=max;
}

void	GetMaxFd(int fd)
{
	if(fd>maxfd)
		maxfd=fd;
}


unsigned __stdcall ProcessAppEvent(LPVOID arg)
{
	struct timeval	tv;
	int				i, result, cnt, len;

	// ---------------------------------------------------------------------------
	// OS가 부팅된 시간부터 현재까지의 시간을 millisecond 단위로 return
	// ---------------------------------------------------------------------------
	last=mili=timeGetTime();							// millisecond로 system 시간 얻어오기

	// ---------------------------------------------------------------------------
	// timer에 현재 시간 설정 (끝의 milli second는 올림 처리)
	// ---------------------------------------------------------------------------
	//GetTimer(-1, Wait_Sec, 1000-(mili%1000), 0, 0);

	while( 1 )
	{
		// ---------------------------------------------------------------------------
		// SIP_FID로부터 Data 수신 wait
		// ---------------------------------------------------------------------------
		readfds = allset;
		tv.tv_sec = 0;
		tv.tv_usec = 2000;

		result = select( maxfd+1, &readfds, 0, 0, &tv );
		if( result < 0 )
		{
			APPLOG->Print( DBGLV_ERR, "CHAN[   ][TCP: : ] select fail, errno=%d, %s.", errno, strerror(errno) );
			Sleep(1000);
			continue;
		}
		else if( result == 0 )
		{
			CheckAlive();
			continue;
		}

		//// ---------------------------------------------------------------------------
		//// famon.exe로부터 data 받았을 때 처리
		//// ---------------------------------------------------------------------------
		//if(FD_ISSET(sock_ipc, &readfds))
		//	RxIpcMsg();
		// ---------------------------------------------------------------------------
		// SIP_FID가 접속했을 때 처리
		// ---------------------------------------------------------------------------
		if( FD_ISSET(sock_agent, &readfds) )
		{
			i = FaxAccept();
			if( i >= 0 )
			{
				FD_SET(shmem->r[i].accept, &allset);
				GetMaxFd(shmem->r[i].accept);
			}
		}

		// ---------------------------------------------------------------------------
		// 기접속한 SIP_FID로부터 Data를 수신했을때 처리
		// ---------------------------------------------------------------------------
		for( i = CConfig::ADDRESS_CLIENT_SESSION_BEGIN ; i <= CConfig::ADDRESS_CLIENT_SESSION_END ; ++i )
		{
			if( !shmem->r[i].start_time )
				continue;
			
			if( FD_ISSET( shmem->r[i].accept, &readfds ) )
			{
				len = FaxRead( i );
				if( len <= 0 )
					FaxDisconnected( i, len );
			}
		}

		// ---------------------------------------------------------------------------
		// Data를 수신하거나 select timeout일 경우 시간 재설정
		// ---------------------------------------------------------------------------
//		mili=timeGetTime();							// OS가 부팅된 후 현재까지 지난 시간 (millisecond 단위)
//		cnt=(mili-last)/10;
//		if(cnt){									// 0.02초가 지났으면 (0.002초가 10번 지날때마다)
////			dbg(-1, 3, "Call TimerEvent(): cnt=%d mili=%ld last=%ld.\n", cnt, mili, last);
//			TimerEvent();
//			last=mili;
//		}
	}
}

void	WriteSip(int msgid, int chan, int len, char * para)
{
	fax_sip_control m;

	m.msgid=msgid;
	m.chan=chan;
	m.len=len;
	if(len)
		memcpy(m.para, para, len);

	send(sock_agent, (char *)&m, len+12, 0);
}


