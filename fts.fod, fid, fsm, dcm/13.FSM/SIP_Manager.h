#pragma once

#include "SIP_Data.h"
#include "SIP_ChannelHunter.h"
#include <map>
using namespace std;




class SIP_Manager
{
	///// Static /////////////////////////////////////////////////
public:
	static SIP_Manager*		inst;
	static BTStackManager*	s_pStackManager;

	static void InitSingleton();

	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_Manager();
	~SIP_Manager();

	///// Method /////////////////////////////////////////////////
public:
	bool Start();

	// 채널 관련 처리 함수
	bool			RegistChannels( char p_chDirection, int p_chanBegin, int p_chanEnd );
	void			UnregistChannels( int p_chanBegin, int p_chanEnd );
	SIP_Session*	AssignChann( BTDialog* p_pDialog, SIP_Client& p_Client, int* p_pnConnectedCnt );
	void			ReleaseChann( SIP_Session* pSession );
	void			DisconnectChann( int chan );
	bool			CallChann( int p_chan, const char* p_szTarget, const char* p_szFrom, const char* p_szClientCallId);

	void			CallingTimeout( vector< int >& Chans );
	

	// 세션 관련 처리 함수
	int				newSessionId();
	SIP_Session*	GetSession( int chan );
	SIP_Session*	FindSession( BTDialog* pDialog );
	SIP_Session*	newSession( int chan, BTDialog* pDialog = NULL );
	
	// 다이얼로그 처리 함수
	void			newDialog( SIP_Session* pSession, BTDialog* pDialog );
	void			deleteDialog( SIP_Session* pSession, BTDialog* pDialog );
	void			showSession();

	inline int		referCount()		
	{
		return ++m_cntRefer;
	}

	/* 	유저 관련 처리 함수 */
	unsigned int newUserId();
	SIP_User* newUser( BTRegister* pRegister = NULL );
	SIP_User* findUser( BTRegister* pRegister );
	SIP_User* getUser( unsigned int nUser );
	void deleteUser( SIP_User* pUser );
	void showUser();

	/**
	* @brief	명령어 처리 함수 (메시지 전송)
	*/
	int nInvite( const char* strTarget = NULL );
    int nReinvite( int nSession );
	int nReject( int nSession, int nCode );
    int nCancel( int nSession );
    int nBye( int nSession );
    //int nOptions( int nSession );
    int nPerfOptions( const char* strTarget );
    int nRefer( const char* strTarget );
    int nRegister( int nExpires );
    
	/**
	* @brief	메시지 처리용 함수 
	*/
	/*int getServHeader( BTSIPMessage* pMessage );
	int setServHeader( BTSIPMessage* pMessage );
	int addServHeader( const char* strName, const char* strValue );
	int delServHeader( const char* strName );
	int clrServHeader();*/

	static int OutdialogOptions( const char* p_szIp, int p_nPort, const char* p_szTargetUser, const char* p_szTargetName = NULL );

	///// Implementaion //////////////////////////////////////////
protected:

	///// Field //////////////////////////////////////////////////
protected:
	CCriticalSection		m_ChanLock;
	CHANNEL_LIST			m_ChanList;
	CHANNEL_DIALOG_MAP		m_ChanDialogMap;
	int						m_connectedCnt;

	int						m_sessionId;
	unsigned int			m_cntRefer;

	CCriticalSection		m_UserLock;
	USER_REGISTER_MAP		m_UserRegisterMap;
	BTPool< SIP_User >		m_UserPool;
	USER_LIST				m_UserList;
	unsigned int			m_newUserId;
	
	char					m_szBuffer[2048];

	time_t					m_tLastRecv_1;
	time_t					m_tLastRecv_2;
};


