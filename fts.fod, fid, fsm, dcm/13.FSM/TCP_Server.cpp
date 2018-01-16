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

int		maxfd;											// MAX socket ����
fd_set	readfds, allset;								// ������ socket�� ���� fd_set

int		sock_ipc;										// 
int		sock_agent;										// SIP_FID�� ������ socket

int		TcpServerInit(int localport)
{
	struct sockaddr_in bindaddr;
	struct linger	linger;
	int		i, sock;

	// ---------------------------------------------------------------------------
	// P0. �ʱ�ȭ
	// ---------------------------------------------------------------------------
	FD_ZERO(&allset);

	for( i = CConfig::ADDRESS_CLIENT_SESSION_BEGIN ; i <= CConfig::ADDRESS_CLIENT_SESSION_END ; i++ )
		memset( &shmem->r[i], 0, sizeof(shmem->r[0]) );


	// ---------------------------------------------------------------------------
	// P1. SIP_FID�� ������ TCP Server Socket ����
	// ---------------------------------------------------------------------------
	sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( sock < 0 )
	{
		APPLOG->Print( DBGLV_ERR, "CHAN[   ][TCP: : ] can't create socket." );
		return	-1;
	}

	// ---------------------------------------------------------------------------
	// P2. linger ����
	// ---------------------------------------------------------------------------
	linger.l_onoff=1;
	linger.l_linger=0;
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&linger, sizeof(linger));

	// ---------------------------------------------------------------------------
	// P3. ��/���� buffer�� 128KBytes�� ����
	// ---------------------------------------------------------------------------
	i=0x20000;
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&i, sizeof(i));
	i=0x20000;
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&i, sizeof(i));

	// ---------------------------------------------------------------------------
	// P4. bind ó��
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
	// P5. listen ó��
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
	// P1. accept ó��
	// ---------------------------------------------------------------------------
	sock = accept( sock_agent, (struct sockaddr *)&peer, &len );
	if( sock < 0 )
	{
		APPLOG->Print( DBGLV_ERR, "CHAN[   ][TCP: : ] fail to accept. result(%d)", sock );
		return	-1;
	}

	// ---------------------------------------------------------------------------
	// P2. fasip_in.exe���� ������ SIP_FID�� ������ ������ ���� Ȯ��
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
	// P3. �־��� ��ü�� SIP_FID ���� ����
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

	if( *state == 0 )								// Header ���� ó��
	{
		reqlen	= (int)(((fax_sip_control *)0)->para) - *ptr;
		p		= (char *)m + *ptr;
	}
	else											// Body ���� ó��
	{
		// ---------------------------------------------------------------------------
		// fax_sip_control ����ü�� msgid���� len���� ����
		//		(((fax_sip_control *)0)->para)
		//			-> fax_sip_control ����ü�� ������ġ�� 0�� ��� para ������ġ�� 12
		//			-> *ptr�� �ʱ� ���� 0�̹Ƿ� 12���� 0���� 12
		//		�� �ʱ� reqlen�� ���̴� para�� �� header�� ���̰� ��
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
	// ������ ���� ���� �ð� ����
	// ---------------------------------------------------------------------------
	shmem->r[i].last = time( NULL );

	// ---------------------------------------------------------------------------
	// Header�� Body Data�� reqlen��ŭ �� �� �о�����
	// ---------------------------------------------------------------------------
	if( len < reqlen )
		return len;

	(*state)++;

	// ---------------------------------------------------------------------------
	// ó�� ���� ����
	// ---------------------------------------------------------------------------
	if( *state == 1 )
	{
		// ---------------------------------------------------------------------------
		// ������ para���� field�� ���� ó��
		// ---------------------------------------------------------------------------
		if( m->len < 0 || m->len > sizeof(m->para) )
		{
			APPLOG->Print( DBGLV_ERR, "CHAN[   ][TCP:%d:R] received wrong message length, len=%d", i, m->len );
			return -2;
		}
		*ptr = 0;
		// ---------------------------------------------------------------------------
		// ������ para���� field���� 0�� ��� (para data�� ���� ���)
		// ---------------------------------------------------------------------------
		if( m->len == 0 )
			(*state)++;
	}

	// ---------------------------------------------------------------------------
	// ������ Data�� �޾����Ƿ� SIP_FID�κ��� ���Ź��� Message ó��
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
// Module ��		: void FaxDisconnected()
// ---------------------------------------------------------------------------
// Description	: SIP_FID���� ���� ����
// ---------------------------------------------------------------------------
// Argument		: int i;		Index
//				  int cause;	�������� ����
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
void	FaxDisconnected( int i, int cause )
{
	uchar	* ip;

	ip = (uchar *)&shmem->r[i].ip;
	APPLOG->Print( DBGLV_RPT, "CHAN[   ][TCP:%d:C] disconnected. addr=%d.%d.%d.%d:%d, cause=%d.", i, ip[0], ip[1], ip[2], ip[3], shmem->r[i].port, cause );

	// ---------------------------------------------------------------------------
	// �ش� socket�� fd_set���� clear
	// maxfd�� �缳��
	// socket close
	// ---------------------------------------------------------------------------
	FD_CLR(shmem->r[i].accept, &allset);
	LowerMaxFd(shmem->r[i].accept);
	closesocket(shmem->r[i].accept);

	// ---------------------------------------------------------------------------
	// Regist ���� clear
	// ---------------------------------------------------------------------------
	ResetDevRegistered( i, &shmem->r[i].f );

	// ---------------------------------------------------------------------------
	// Shared Memory���� ���� ����
	// ---------------------------------------------------------------------------
	memset( &(shmem->r[i]), 0, sizeof(shmem->r[0]) );
}

void	FaxDisconnect( int i )
{
	uchar	* ip;

	ip = (uchar *)&shmem->r[i].ip;
	APPLOG->Print( DBGLV_RPT, "CHAN[   ][TCP:%d:C] disconnect. addr=%d.%d.%d.%d:%d.", i, ip[0], ip[1], ip[2], ip[3], shmem->r[i].port );

	// ---------------------------------------------------------------------------
	// �ش� socket�� fd_set���� clear
	// maxfd�� �缳��
	// socket close
	// ---------------------------------------------------------------------------
	FD_CLR(shmem->r[i].accept, &allset);
	LowerMaxFd(shmem->r[i].accept);
	closesocket(shmem->r[i].accept);

	// ---------------------------------------------------------------------------
	// Regist ���� clear
	// ---------------------------------------------------------------------------
	ResetDevRegistered( i, &shmem->r[i].f );

	// ---------------------------------------------------------------------------
	// Shared Memory���� ���� ����
	// ---------------------------------------------------------------------------
	memset( &(shmem->r[i]), 0, sizeof(shmem->r[0]) );
}

// ---------------------------------------------------------------------------
// Module ��		: void DecodeFaxMsg()
// ---------------------------------------------------------------------------
// Description	: SIP_FID�κ��� ���� ���� ���� ���� �� Display
// ---------------------------------------------------------------------------
// Argument		: int i;		������ SIP_FID ������ ����Ǿ� �ִ� index
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
			// Fax_Register Data ���� (FAX IP, Base Port, Port Spacing, ���� ä��, ä�� ����)
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
// Module ��		: void FaxWrite()
// ---------------------------------------------------------------------------
// Description	: SIP_FID�� ���� �۽�
// ---------------------------------------------------------------------------
// Argument		: int i;		������ SIP_FID ������ ����Ǿ� �ִ� index
//				  int msgid;	Msg ID
//				  int chan;		FAX CH
//				  int len;		para ����
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
// Module ��		: void SetDevRegistered()
// ---------------------------------------------------------------------------
// Description	: SIP_FID���� Fax_Register ������ �޾� ó���� �� Register ���·� ���
// ---------------------------------------------------------------------------
// Argument		: fax_register * r;		Fax_Register  ������ Data
//				  int id;				������ SIP_FID ������ ����Ǿ� �ִ� index
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
int	SetDevRegistered( fax_register * r, int id )
{
	int	s, e, result;

	// ---------------------------------------------------------------------------
	// Fax_Register ��� ���� �� �۽�
	// ---------------------------------------------------------------------------

	s = r->fax_device_start;
	e = r->fax_device_start + r->fax_device_cnt - 1;
	if( e + 1 > MAXDEVICE )
		e = MAXDEVICE - 1;

	// ---------------------------------------------------------------------------
	// ä�ΰ� Ȯ��
	// ---------------------------------------------------------------------------
	if( s < 0 || s >= MAXDEVICE )
		return -1;

	else if( e <= s )
		return -2;

	// ---------------------------------------------------------------------------
	// FAX CH ������ SIP_FID Register ������ �ִ� Array�� index+1�� ����
	// ---------------------------------------------------------------------------
	for( int i = s ; i <= e ; i++ )
	{
		shmem->d[i].reg = id + 1;
		shmem->d[i].call_direction = r->fax_direction;
	}

	// ---------------------------------------------------------------------------
	// SIP ChannelHunter �� ä�� ���
	// ---------------------------------------------------------------------------
	result = SIP_ChannelHunter::Inst()->RegistClient( id, r->fax_direction, s, e );
	if( result != 0 )
		result += -10;

	return result;
}

// ---------------------------------------------------------------------------
// Module ��		: void ResetDevRegistered()
// ---------------------------------------------------------------------------
// Description	: SIP_FID�� ������ �������� �� UnRegister ó��
// ---------------------------------------------------------------------------
// Argument		: fax_register * r;		Fax_Register  ������ Data
// ---------------------------------------------------------------------------
// Return		: void
// ---------------------------------------------------------------------------
void	ResetDevRegistered( int id, fax_register * r )
{
	int		i, s, e;

	SIP_ChannelHunter::Inst()->UnregistClient( id );

	// ---------------------------------------------------------------------------
	// ���� ä�� Ȯ��
	// ---------------------------------------------------------------------------
	if( r->fax_device_start<0 || r->fax_device_start>=MAXDEVICE )
		return;

	// ---------------------------------------------------------------------------
	// ���� ä���� ���� CH���� ������ Ȯ��
	// ---------------------------------------------------------------------------
	s=r->fax_device_start;
	e=r->fax_device_start+r->fax_device_cnt;
	if( e <= s )
		return;

	// ---------------------------------------------------------------------------
	// ����ä���� MAXDEVICE(960)���� ũ�� MAXDEVICE�� �缳��
	// ---------------------------------------------------------------------------
	if( e > MAXDEVICE )
		e = MAXDEVICE;

	// ---------------------------------------------------------------------------
	// FAX CH ������ SIP_FID Register ������ �ִ� Array index ������ reset
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
	// fd�� maxfd�� �ƴϸ� return
	// ---------------------------------------------------------------------------
	if(fd!=maxfd)
		return;

	max=0;

	// ---------------------------------------------------------------------------
	// fd�� famon.exe�� �����ϴ� socket�� �ƴϰ� famon.exe�� �������̸�
	// ---------------------------------------------------------------------------
	if(sock_ipc!=fd && sock_ipc>max)
		max=sock_ipc;

	// ---------------------------------------------------------------------------
	// fd�� SIP_FID�� �����ϴ� listen socket�� �ƴϰ� listen socket�� famon.exe�� �����ϴ� socket���� ũ��
	// ---------------------------------------------------------------------------
	if(sock_agent!=fd && sock_agent>max)
		max=sock_agent;

	// ---------------------------------------------------------------------------
	// ���� fasip_in.exe�� ����� socket�� �� fd�� �����ϰ� ���� ū socket ���� ã�´�
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
	// OS�� ���õ� �ð����� ��������� �ð��� millisecond ������ return
	// ---------------------------------------------------------------------------
	last=mili=timeGetTime();							// millisecond�� system �ð� ������

	// ---------------------------------------------------------------------------
	// timer�� ���� �ð� ���� (���� milli second�� �ø� ó��)
	// ---------------------------------------------------------------------------
	//GetTimer(-1, Wait_Sec, 1000-(mili%1000), 0, 0);

	while( 1 )
	{
		// ---------------------------------------------------------------------------
		// SIP_FID�κ��� Data ���� wait
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
		//// famon.exe�κ��� data �޾��� �� ó��
		//// ---------------------------------------------------------------------------
		//if(FD_ISSET(sock_ipc, &readfds))
		//	RxIpcMsg();
		// ---------------------------------------------------------------------------
		// SIP_FID�� �������� �� ó��
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
		// �������� SIP_FID�κ��� Data�� ���������� ó��
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
		// Data�� �����ϰų� select timeout�� ��� �ð� �缳��
		// ---------------------------------------------------------------------------
//		mili=timeGetTime();							// OS�� ���õ� �� ������� ���� �ð� (millisecond ����)
//		cnt=(mili-last)/10;
//		if(cnt){									// 0.02�ʰ� �������� (0.002�ʰ� 10�� ����������)
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


