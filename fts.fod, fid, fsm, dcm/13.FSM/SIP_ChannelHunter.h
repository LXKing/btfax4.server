#pragma once

#include "SIP_Data.h"
#include "ClassUtility.h"
#include <list>

using namespace std;





struct SIP_Client;

class SIP_ChannelHunter
{
	SINGLETON_DECLARE( SIP_ChannelHunter );

	///// Type ///////////////////////////////////////////////////
private:
	typedef list< SIP_Client >	CLIENTS;


	///// Field //////////////////////////////////////////////////
protected:
	CCriticalSection	m_Lock;
	CLIENTS				m_ClientsOUT;
	CLIENTS				m_ClientsIN;
	int					m_chanCntOUT;
	int					m_chanCntIN;
	

	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_ChannelHunter();
	virtual ~SIP_ChannelHunter();


	///// Method /////////////////////////////////////////////////
public:
	int				RegistClient( int p_tcpSession, char p_chDirection, int p_m_chanBegin, int p_m_chanEnd );
	bool			UnregistClient( int p_tcpSession );
	SIP_Session*	HuntChannel( char p_chDirection, BTDialog* pDialog );


	///// Implementaion //////////////////////////////////////////
protected:
	bool			_IsRegistedClient( CLIENTS& p_Clients, int p_tcpSession );
	bool			_IsRegistedChannel( char p_chDirection, int p_chanBegin, int p_chanEnd );

	
};


struct SIP_Client
{
	///// Field //////////////////////////////////////////////////
protected:
	int		m_tcpSession;
	char	m_chDirection;
public:
	int		m_chanBegin;
	int		m_chanEnd;
	int		m_chanCnt;
	int		m_chanHunt;


	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_Client();
	virtual ~SIP_Client();


	///// Method /////////////////////////////////////////////////
public:
	int		TcpSession();
	char	Direction();
	void	Set( int p_tcpSession, char p_chDirection, int p_chanBegin, int p_chanEnd );
	bool	IsCross( int p_chanBegin, int p_chanEnd );
};
