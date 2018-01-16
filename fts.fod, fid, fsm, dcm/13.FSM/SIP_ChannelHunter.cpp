#include "stdafx.h"
#include "SIP_ChannelHunter.h"

#include "SIP_Manager.h"



SINGLETON_IMPLEMENT( SIP_ChannelHunter );


///// Constructor, Destructor ////////////////////////////////

SIP_ChannelHunter::SIP_ChannelHunter()
{
	m_chanCntOUT = 0;
	m_chanCntIN	 = 0;
}

SIP_ChannelHunter::~SIP_ChannelHunter()
{
}



///// Method /////////////////////////////////////////////////

int SIP_ChannelHunter::RegistClient( int p_tcpSession, char p_chDirection, int p_chanBegin, int p_chanEnd )
{
	{CSingleLock Locker( &m_Lock, TRUE );

		//// P1.기등록 여부 확인
		if( _IsRegistedClient( m_ClientsOUT, p_tcpSession ) )
			return -1;	// 에러 : 세션 중복 (Outbound)

		if( _IsRegistedClient( m_ClientsIN, p_tcpSession ) )
			return -2;  // 에러 : 세션 중복 (Inbound )

		if( _IsRegistedChannel( p_chDirection, p_chanBegin, p_chanEnd ) )
			return -3;  // 에러 : 채널 중복 (Hunter)


		//// P2. SIP_Manaer 에 등록
		if( !SIP_Manager::inst->RegistChannels( p_chDirection, p_chanBegin, p_chanEnd ) )
			return -4;  // 에러 : 채널 중복 (SIP_Manager)


		//// P3. Hunter 에 등록
		SIP_Client Client;
		Client.Set( p_tcpSession, p_chDirection, p_chanBegin, p_chanEnd );
		if( p_chDirection == 'O' )
		{
			m_chanCntOUT += Client.m_chanCnt;
			m_ClientsOUT.push_back( Client );
		}
		else 
		{
			m_chanCntIN += Client.m_chanCnt;
			m_ClientsIN.push_back( Client );
		}
	}

	return 0;
}

bool SIP_ChannelHunter::UnregistClient( int p_tcpSession )
{
	CLIENTS*			pClients = NULL;
	CLIENTS::iterator	pos, posEnd;
	
	{CSingleLock Locker( &m_Lock, TRUE );

		pClients = &m_ClientsOUT;
		pos		 = pClients->begin();
		posEnd	 = pClients->end();
		for( ; pos != posEnd ; ++pos )
		{
			if( pos->TcpSession() == p_tcpSession ) 
			{
				m_chanCntOUT -= pos->m_chanCnt;

				SIP_Manager::inst->UnregistChannels( pos->m_chanBegin, pos->m_chanEnd );
				pClients->erase( pos );
				return true;
			}
		}

		pClients = &m_ClientsIN;
		pos		 = pClients->begin();
		posEnd	 = pClients->end();
		for( ; pos != posEnd ; ++pos )
		{
			if( pos->TcpSession() == p_tcpSession ) 
			{
				m_chanCntIN -= pos->m_chanCnt;

				SIP_Manager::inst->UnregistChannels( pos->m_chanBegin, pos->m_chanEnd );
				pClients->erase( pos );
				return true;
			}
		}
		
	}

	return false;
}

SIP_Session* SIP_ChannelHunter::HuntChannel( char p_chDirection, BTDialog* pDialog )
{
	SIP_Session*	pSession	= NULL;
	CLIENTS*		pClients	= NULL;
	SIP_Client*		pClient	= NULL;
	int				cnt, cntConnected;


	if( p_chDirection == 'O' )	pClients = &m_ClientsOUT;
	else						pClients = &m_ClientsIN;

	{CSingleLock Locker( &m_Lock, TRUE );

		cnt = pClients->size();
		if( cnt <= 0 )
			return false;

		CLIENTS::iterator	pos		= pClients->begin();
		CLIENTS::iterator	posEnd	= pClients->end();
		for( ; pos != posEnd ; ++pos )
		{
			pClient = &(*pos);
			pSession = SIP_Manager::inst->AssignChann( pDialog, *pos, &cntConnected );
			if( pSession ) 
			{
				APPLOG->Print( DBGLV_INF, "Hunt Channel : %d [Client:%d]. Running/Total = %d/%d", pos->m_chanHunt, pos->TcpSession(), cntConnected, m_chanCntIN );
				return pSession;
			}
		}

		APPLOG->Print( DBGLV_ERR, "Hunt Channel : Channel Full. Running/Total = %d/%d", cntConnected, m_chanCntIN );
	}

	return NULL;
}

bool SIP_ChannelHunter::_IsRegistedClient( CLIENTS& p_Clients, int p_tcpSession )
{
	CLIENTS::iterator pos	 = p_Clients.begin();
	CLIENTS::iterator posEnd = p_Clients.end();
	for( ; pos != posEnd ; ++pos )
	{
		if( pos->TcpSession() == p_tcpSession )
			return true;
	}

	return false;
}

bool SIP_ChannelHunter::_IsRegistedChannel( char p_chDirection, int p_chanBegin, int p_chanEnd )
{
	CLIENTS* pClients	= NULL;

	if( p_chDirection == 'O' )
		pClients	= &m_ClientsOUT;
	else
		pClients = &m_ClientsIN;

	CLIENTS::iterator pos	 = pClients->begin();
	CLIENTS::iterator posEnd = pClients->end();
	for( ; pos != posEnd ; ++pos )
	{
		if( pos->IsCross( p_chanBegin, p_chanEnd ) )
			return true;
	}

	return false;
}



///// Constructor, Destructor ////////////////////////////////

SIP_Client::SIP_Client()
{
	m_tcpSession  = -1;
	m_chDirection = ' ';
	m_chanBegin	= -1;
	m_chanEnd	= -1;
	m_chanCnt	= -1;
	m_chanHunt	= -1;
}

SIP_Client::~SIP_Client()
{
}



///// Method /////////////////////////////////////////////////

int	 SIP_Client::TcpSession()
{
	return m_tcpSession;
}

char SIP_Client::Direction()
{
	return m_chDirection;
}

void SIP_Client::Set( int p_tcpSession, char p_chDirection, int p_chanBegin, int p_chanEnd )
{
	m_tcpSession  = p_tcpSession;
	m_chDirection = p_chDirection;

	m_chanBegin	= p_chanBegin;
	m_chanEnd	= p_chanEnd;

	m_chanCnt	= m_chanEnd - m_chanBegin + 1;
	m_chanHunt	= m_chanBegin - 1;
}

bool SIP_Client::IsCross( int p_chanBegin, int p_chanEnd )
{
	if( m_chanBegin <= p_chanBegin && p_chanBegin <= m_chanEnd )
		return true;

	if( m_chanBegin <= p_chanEnd && p_chanEnd <= m_chanEnd )
		return true;

	return false;
}

//int	SIP_Client::HuntIdle()
//{
//	int chanCnt = m_chanEnd - m_chanBegin + 1;
//
//	for( int i = 0 ; i < chanCnt ; ++i )
//	{
//
//	}
//}

