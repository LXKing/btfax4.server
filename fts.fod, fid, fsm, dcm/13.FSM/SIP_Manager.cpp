#include "stdafx.h"
#include "SIP_Manager.h"
#include "Config.h"

#include "SIP_App.h"
#include "SIP_MGWs.h"
#include "SIP_TransportHandler.h"
#include "SIP_TransactionHandler.h"
#include "SIP_DialogHandler.h"
#include "SIP_RegisterHandler.h"
#include "SIP_AliveTimer.h"
#include "SIP_CallingTimer.h"
#include "SIP_Utility.h"






///// Static /////////////////////////////////////////////////
SIP_Manager*		SIP_Manager::inst			= NULL;
BTStackManager*	SIP_Manager::s_pStackManager	= NULL;

void SIP_Manager::InitSingleton()
{
	SIP_Manager::inst = new SIP_Manager();
}



///// Constructor, Destructor ////////////////////////////////
SIP_Manager::SIP_Manager()
{
	SIP_Manager::inst = this;
}

SIP_Manager::~SIP_Manager()
{
}




///// Method /////////////////////////////////////////////////
bool SIP_Manager::Start()
{
	BTStackConfigItems	stackConfigItems;

//#ifdef BTCORE_WIN32
	BTUDPSocket::_WSAStartup();
//#endif

// aaa	_snprintf(stackConfigItems.strLogPrefix, sizeof(stackConfigItems.strLogPrefix),
//				"BTSIP_%02d_", CConfig::PROCESS_NO );
	strcpy(stackConfigItems.strLogPath  , CConfig::SYSTEM_LOG_PATH );
	stackConfigItems.nLogFlags			= CConfig::SYSTEM_BTSIP_LOG_LEVEL;
	stackConfigItems.bAuto100Send		= CConfig::SERVICE_AUTO_TRYING;
	stackConfigItems.bManualACK			= 0;
	stackConfigItems.bManualPRACK		= 0;
	strcpy(stackConfigItems.strUserAgent,	"BTSIP");
	strcpy(stackConfigItems.strLocalIP, CConfig::ADDRESS_LOCAL_IP);
	stackConfigItems.nLocalPort			= CConfig::ADDRESS_LOCAL_PORT;
	stackConfigItems.nTimer1			= CConfig::TIMER_1;
	stackConfigItems.nTimer2			= CConfig::TIMER_2;
	stackConfigItems.nTimer4			= CConfig::TIMER_4;
	stackConfigItems.nTimerD			= CConfig::TIMER_D;
	stackConfigItems.nTimerP			= 120000;	// out invite timeout	// aaa

	stackConfigItems.nMTUSize			= CConfig::SERVICE_UDP_MTU_SIZE;

	if (CConfig::SERVICE_SESSION_EXPIRES)
	{
		stackConfigItems.bSessionTimer	= true;
		stackConfigItems.nSessionExpires= CConfig::SERVICE_SESSION_EXPIRES;
		stackConfigItems.nMinSE			= CConfig::SERVICE_MIN_SE;
	}
	else
	{
		stackConfigItems.bSessionTimer	= false;
		stackConfigItems.nSessionExpires= 0;
		stackConfigItems.nMinSE			= 0;
	}

	stackConfigItems.nMaxDialogs		= CConfig::SYSTEM_MAX_DIALOGS;
	stackConfigItems.nMaxTransactions	= CConfig::SYSTEM_MAX_TRANSCS;
	stackConfigItems.nMaxRegisters		= CConfig::SYSTEM_MAX_REGISTERS;
	stackConfigItems.nTimerPoolCnt		= CConfig::SYSTEM_TIMER_POOL_CNT;
	stackConfigItems.nMemoryPageSize	= 4096;
	stackConfigItems.nMemoryPoolCnt		= CConfig::SYSTEM_TIMER_POOL_CNT;
	stackConfigItems.nDispatcherCnt		= 1;
	stackConfigItems.nTimerMgrCnt		= 1;
	stackConfigItems.nEventMgrCnt		= CConfig::SYSTEM_EVENTMGR_CNT;;

	strcpy(stackConfigItems.strAllow, "INVITE,ACK,BYE,UPDATE,CANCEL,OPTIONS,INFO");

	int nRes  = 0;
	if( CConfig::SERVICE_SERVE_SUPPORTED.GetCount() > 0 )
	{
		nRes = sprintf( stackConfigItems.strSupported, "%s", (LPCSTR)CConfig::SERVICE_SERVE_SUPPORTED[0] );
		for (int i = 1; i < CConfig::SERVICE_SERVE_SUPPORTED.GetCount() ; ++i )
			nRes += sprintf( stackConfigItems.strSupported + nRes, ", %s", (LPCSTR)CConfig::SERVICE_SERVE_SUPPORTED[i] );
	}
			
	s_pStackManager	= new BTStackManager();
	nRes = s_pStackManager->initialize( &stackConfigItems );
	if( nRes != rtSUCCESS )
	{
		APPLOG->Print( DBGLV_ERR, "BTSIP Stack initialization failed.. (nRes=%d)", nRes );
		return false;
	}

	APPLOG->Print( DBGLV_RPT, "BTSIP Stack Initialized" );

	g_pTrptMgr->setCallbackFunction( 
												SIP_TransportHandler::_ErrorCallback );
	g_pTrxMgr->setCallbackFunction( 
												SIP_TransactionHandler::_TranscStateChanged, 
												SIP_TransactionHandler::_TranscMessageReceived, 
												SIP_TransactionHandler::_TranscMessageSending,
												SIP_TransactionHandler::_TranscAuthCredentialFound,
												SIP_TransactionHandler::_TranscAuthComplete	);
	g_pDlgMgr->setCallbackFunction(
												SIP_Dialog::_DialogStateChanged, 
												SIP_Dialog::_DialogModifyStateChanged,
												SIP_Dialog::_DialogPrackStateChanged,
												SIP_Dialog::_DialogTranscStateChanged,
												SIP_Dialog::_DialogSessionTimerEvent,
												SIP_Dialog::_DialogSessionTimerRefreshAlert,
												SIP_Dialog::_DialogMessageReceived,
												SIP_Dialog::_DialogMessageSending );
	g_pRegiMgr->setCallbackFunction(
												SIP_RegisterHandler::_RegisterStateChanged );
	
	g_pAuthMgr->setCallbackFunction(AuthGetSharedSecret, AuthGetMD5);
	g_pAppMgr->setCallbackFunction(AppEventCallback);
	
	m_cntRefer = 0;
	m_connectedCnt	= 0;
	
	m_UserPool.initPoolWithNullObject( CConfig::SYSTEM_MAX_REGISTERS, 0 );
	m_newUserId	= MIN_USER_ID - 1;

	memset( m_szBuffer, 0x00, sizeof(m_szBuffer) );

	SIP_AliveTimer::Inst()->Start( 1000 );
	SIP_CallingTimer::Inst()->Start( 1000 );

	if( CConfig::SERVICE_REGIST_YN == "Y" )
	{
		if( nRegister( CConfig::SERVICE_EXPIRES ) < 0 )
			return false;
	}

	m_tLastRecv_1 = time( NULL );
	m_tLastRecv_2 = time( NULL );

	return true;
}


/* 	세션 관련 처리 함수 */

bool SIP_Manager::RegistChannels( char p_chDirection, int p_chanBegin, int p_chanEnd )
{
	int				nSessionId;
	SIP_Session*	pSession;

	{CSingleLock Locker( &m_ChanLock, TRUE );
	
		for( int chan = p_chanBegin ; chan <= p_chanEnd ; ++chan )
		{
			CHANNEL_LIST::iterator pos = m_ChanList.find( chan );
			if( pos != m_ChanList.end() ) 
				return false;

			nSessionId = newSessionId();

			pSession = new SIP_Session( NULL, chan );
			pSession->setSessionId( nSessionId );
			pSession->setDirection( p_chDirection );
	
			m_ChanList.insert( make_pair( chan, pSession ) );
		}
	}

	return true;
}

void SIP_Manager::UnregistChannels( int p_chanBegin, int p_chanEnd )
{
	CHANNEL_DIALOG_MAP::iterator	pos, posEnd;
	vector< BTDialog* >				Dialogs;

	{CSingleLock Locker( &m_ChanLock, TRUE );
	
		//  채널 리스트에서 제거
		for( int chan = p_chanBegin ; chan <= p_chanEnd ; ++chan )
			m_ChanList.erase( chan );

		// 다이얼로그 맵 에서 제거
		for( int chan = p_chanBegin ; chan <= p_chanEnd ; ++chan )
		{
			pos		= m_ChanDialogMap.begin();
			posEnd	= m_ChanDialogMap.end();
			for( ; pos != posEnd ; ++pos )
			{
				if( pos->second == chan )
					Dialogs.push_back( pos->first );
			}
		}
		int cnt = Dialogs.size();
		for( int i = 0 ; i < cnt ; ++i )
			m_ChanDialogMap.erase( Dialogs[i] );

	} // LOCK
}

SIP_Session* SIP_Manager::AssignChann( BTDialog* p_pDialog, SIP_Client& p_Client, int* p_pnConnectedCnt )
{
	SIP_Session* pSession = NULL;

	{CSingleLock Locker( &m_ChanLock, TRUE );
		
		for( int i = 0 ; i < p_Client.m_chanCnt ; ++i )
		{
			p_Client.m_chanHunt = ( p_Client.m_chanHunt >= p_Client.m_chanEnd ) ? p_Client.m_chanBegin : p_Client.m_chanHunt + 1;

			APPLOG->Print( DBGLV_INF, "AssignChann(). p_Client.m_chanHunt[%d], m_ChanList[ p_Client.m_chanHunt ]->state()[%d]", p_Client.m_chanHunt, m_ChanList[ p_Client.m_chanHunt ]->state());

			if( m_ChanList[ p_Client.m_chanHunt ]->state() == SIP_Session::SESSION_STATE_IDLE )
			{
				pSession = m_ChanList[ p_Client.m_chanHunt ];
				
				pSession->setDialog( p_pDialog );
				m_ChanDialogMap[ p_pDialog ] = p_Client.m_chanHunt;

				break;
			}
		}

		if( pSession )
			++m_connectedCnt;

		*p_pnConnectedCnt = m_connectedCnt;
	}

	return pSession;
}

void SIP_Manager::ReleaseChann( SIP_Session* pSession )
{
	{CSingleLock Locker( &m_ChanLock, TRUE );

		BTDialog* pDialog = pSession->dialog();
		if( pDialog )
			m_ChanDialogMap.erase( pDialog );
	
		pSession->setIdle();
		--m_connectedCnt;

		APPLOG->Print( DBGLV_INF, "Rel  Channel : %d [        ]. Running = %d", pSession->channel(), m_connectedCnt );
	}
}

void SIP_Manager::DisconnectChann( int chan )
{
	SIP_Session* pSession = GetSession( chan );
	if( !pSession ) {
		APPLOG->Print( DBGLV_WRN, "[SIP_Manager] Call Channel : Fail. Channel(%d) not exist", chan );
		return;
	}

	pSession->byeInUserContext();
}

bool SIP_Manager::CallChann( int p_chan, const char* p_szTarget, const char* p_szFrom, const char* p_szClientCallId)
{
	SIP_Session* pSession = GetSession( p_chan );

	if( !pSession ) {
		APPLOG->Print( DBGLV_WRN, "[SIP_Manager] Disconnect Channel : Fail. Channel(%d) not exist", p_chan );
		return false;
	}

	{CSingleLock Locker( &m_ChanLock, TRUE );

		//BTDialog* pDialog = pSession->inviteInUserContext( p_szTarget, p_szFrom );
        BTDialog* pDialog = pSession->inviteInUserContext( pSession, p_szTarget, p_szFrom);
		if( pDialog == NULL )
			return false;

		/*pSession->setDialog( pDialog );
		pSession->setClientCallId( p_szClientCallId );
		m_ChanDialogMap[ pDialog ] = p_chan;
		SIP_CallingTimer::Inst()->Register( p_chan );*/

        pSession->setClientCallId( p_szClientCallId );
		m_ChanDialogMap[ pDialog ] = p_chan;
		SIP_CallingTimer::Inst()->Register( p_chan );
	}

	return true;
}

void SIP_Manager::CallingTimeout( vector< int >& Chans )
{
	int cnt = Chans.size();

	for( int i = 0 ; i < cnt ; ++i )
	{
		SIP_Session* pSession = GetSession( Chans[i] );

		//APPLOG->Print( DBGLV_WRN, "[SIP_Manager] CallingTimeout Channel : Fail. Channel(%d) not exist", Chans[i] );
        APPLOG->Print( DBGLV_WRN, "[SIP_Manager] CallingTimeout Channel : [%d]", Chans[i] );
		if( !pSession ) {
			APPLOG->Print( DBGLV_ERR, "[CHAN_%03d] Session(        ) Dialog(        ), DialogID(        ). CallingTimeout Channel : Fail. Channel not exist", Chans[i] );
			continue;
		}

		/*APPLOG->Print( DBGLV_WRN, "[CHAN_%03d] Session(0x%08X) Dialog(        ). fail to send BYE. not in dialog", 
						Chans[i], this, pSession->dialog() );*/

		pSession->setStatusCode( 487 ); //Request Terminated : Request has terminated by bye or cancel		
        pSession->byeNoSession(pSession->dialog());
	}
}

int SIP_Manager::newSessionId()
{
	m_sessionId = ( m_sessionId >= MAX_SESSION_ID ) ? MIN_SESSION_ID : m_sessionId + 1;
	return m_sessionId;
}

SIP_Session* SIP_Manager::GetSession( int chan )
{
	SIP_Session*			pSession = NULL;
	CHANNEL_LIST::iterator	sessionPos;
	
	{CSingleLock Locker( &m_ChanLock, TRUE );
	
		sessionPos = m_ChanList.find( chan );
		if( sessionPos != m_ChanList.end() )
			pSession = sessionPos->second;
		else
			APPLOG->Print( DBGLV_INF, "[GetSession] Session not found" );
	}

	return pSession;
}

SIP_Session* SIP_Manager::FindSession( BTDialog* pDialog )
{
	SIP_Session*					pSession = NULL;
	CHANNEL_DIALOG_MAP::iterator	dialogPos;
	CHANNEL_LIST::iterator			sessionPos;
	
	{CSingleLock Locker( &m_ChanLock, TRUE );

		dialogPos = m_ChanDialogMap.find( pDialog );
		if( dialogPos == m_ChanDialogMap.end() )
		{
			APPLOG->Print( DBGLV_INF, "[FindSession] Dialog not found. Dialog(0x%08X)", pDialog );
			return NULL;
		}

		sessionPos = m_ChanList.find( dialogPos->second );
		if( sessionPos != m_ChanList.end() )
			pSession = sessionPos->second;
		else
			APPLOG->Print( DBGLV_INF, "[FindSession] Session not found. Dialog(0x%08X)", pDialog );
	}
		
	return pSession;
}

SIP_Session* SIP_Manager::newSession( int chan, BTDialog* pDialog )
{
	int				nSessionId;
	SIP_Session*	pSession;

	{CSingleLock Locker( &m_ChanLock, TRUE );
	
		CHANNEL_LIST::iterator pos = m_ChanList.find( chan );
		if( pos != m_ChanList.end() ) 
			return pos->second;

		nSessionId = newSessionId();

		pSession = new SIP_Session( pDialog, chan );
		pSession->setSessionId( nSessionId );
	
		m_ChanList.insert(make_pair(chan, pSession));
		if( pDialog )
			m_ChanDialogMap.insert(make_pair(pDialog, chan));
	}

	return pSession;
}

void SIP_Manager::newDialog( SIP_Session* pSession, BTDialog* pDialog )
{
	pSession->setDialog(pDialog);

	{CSingleLock Locker( &m_ChanLock, TRUE );

		m_ChanDialogMap.insert( make_pair(pDialog, pSession->channel()) );
	}
}

void SIP_Manager::deleteDialog( SIP_Session* pSession, BTDialog* pDialog )
{
	{CSingleLock Locker( &m_ChanLock, TRUE );
		m_ChanDialogMap.erase(pDialog);
	}
}

void SIP_Manager::showSession()
{
    /*BTStackResourceStatus 	aStat;
    SIP_Session* pSession 	= NULL;
    BTDialog*   pDialog 	= NULL;
    BTDialog*   pRefer 		= NULL;
    BTDateTime  eventDateTime;
    BTDateTime  currDateTime;
    time_t      currTime;
    char    strTime[32] = {0, };
    char    strAddr[32] = {0, };
    int     nTmpLength  = 0;
    int     nShowCnt    = 0;

    currDateTime.getCTime(&currTime);
    printf("\n      ***** Session Informations (%s) *****\n",
        currDateTime.getDateTimeString(strTime, sizeof(strTime), "%Y%m%d-%H%M%S") ? strTime : "-");

    printf(" ------------------------------------------------------------------------------\n");
    printf(" * INDEX DIALOG   REFER    STATE        EVENT-TIME      REMOTE-ADDR           *\n");
    printf(" ------------------------------------------------------------------------------\n");
	SESSION_LIST___ITERATOR sPos = m_SessionList__.begin();
	for ( ; sPos!=m_SessionList__.end(); sPos++)
	{
        pSession = sPos->second;
		if (pSession) pDialog = pSession->dialog();
        if (pSession && pDialog)
        {
			pRefer = pSession->referDialog();	
            eventDateTime.setDateTimeWithCTime(&pSession->nEventTime);
            APPLOG->Print( DBGLV_INF, "   %05d %08X %08X %-12s %-14s %s", 
						sPos->first, pDialog, pRefer ? pRefer:NULL,
						SIP_Session::const_SessionState[pSession->state()], 
						eventDateTime.getDateTimeString(strTime, sizeof(strTime), "%Y%m%d-%H%M%S") ? strTime : "-", 
						pDialog->getRemoteAddressWithBuffer(strAddr, sizeof(strAddr), nTmpLength)!=rtSUCCESS ? "-" : strAddr);
                nShowCnt++;
        }
    }
    printf(" ------------------------------------------------------------------------------\n");
    s_pStackManager->getResourceStatus(&aStat);
    printf(" #> Dialog=%d/%d(%d%%), Transc=%d/%d(%d%%), \n    Timer=%d/%d(%d%%), Page=%d/%d(%d%%)\n",
        aStat.nDialogUsage, aStat.nDialogMax, aStat.nDialogUsage*100/aStat.nDialogMax,
        aStat.nTransactionUsage, aStat.nTransactionMax, aStat.nTransactionUsage*100/aStat.nTransactionMax,
        aStat.nTimerPoolUsage, aStat.nTimerPoolMax, aStat.nTimerPoolUsage*100/aStat.nTimerPoolMax,
        aStat.nPagePoolUsage, aStat.nPagePoolMax, aStat.nPagePoolUsage*100/aStat.nPagePoolMax);

    APPLOG->Print( DBGLV_INF, " COUNT=%d/%d", nShowCnt, m_SessionList__.size() );*/
}


/* 	유저 관련 처리 함수 */
unsigned int SIP_Manager::newUserId()
{
	if( (int)m_UserList.size() >= CConfig::SYSTEM_MAX_REGISTERS - 1 ) 
	{
		APPLOG->Print( DBGLV_ERR, "[SIP_Manager] New User : User Full. List Size/Max Size = %d/%d", m_UserList.size(), CConfig::SYSTEM_MAX_REGISTERS );
		return 0;
	}

	m_newUserId = ( m_newUserId >= MAX_USER_ID ) ? MIN_USER_ID : m_newUserId + 1;
	APPLOG->Print( DBGLV_INF, "[SIP_Manager] New User : %d", m_newUserId );

	return m_newUserId;
}

SIP_User* SIP_Manager::newUser( BTRegister* pRegister )
{
	SIP_User*	pUser = NULL;

	if( !m_UserPool.isPoolAvailable() )
	{
		APPLOG->Print( DBGLV_ERR, "[newUser] User-Pool not available" );
		return NULL;
	}

	void *pPoolId=NULL;
	pPoolId = m_UserPool.getPool(&pUser);
	if( pPoolId == NULL )
	{
		APPLOG->Print( DBGLV_ERR, "[newUser] User-Pool error" );
		return NULL;
	}

	int nUserId = newUserId();
	if( nUserId == 0 )
	{
		APPLOG->Print( DBGLV_ERR, "[newUser] User-Pool overmax" ); 
		return NULL;
	}

	pUser->initUser();
	if (!pRegister) 
		g_pRegiMgr->createRegister(&pRegister);
	if (!pRegister)
	{
		APPLOG->Print( DBGLV_ERR, "[newUser] Register-Pool not available" );
		deleteUser(pUser);
		return NULL;
	}
	pUser->setPoolId(pPoolId);
	pUser->setRegister(pRegister);

	{CSingleLock Locker( &m_UserLock, TRUE );
		
		m_UserList.insert(make_pair(nUserId, pUser));
		m_UserRegisterMap.insert(make_pair(pRegister, pUser));
	}

	APPLOG->Print( DBGLV_INF, "[newUser] User(0x%08X-%d) created. Register(0x%08X)", pUser, nUserId, pRegister );

	return pUser;
}
	
void SIP_Manager::deleteUser( SIP_User* pUser )
{
	BTRegister* pRegister = pUser->regi();

	{CSingleLock Locker( &m_UserLock, TRUE );

		m_UserRegisterMap.erase(pRegister);
		m_UserPool.releasePool(pUser->getPoolId());
		m_UserList.erase(pUser->getUserId());
	}

	APPLOG->Print( DBGLV_INF, "[deleteUser] User released. count(%d)", m_UserList.size() );
}

SIP_User* SIP_Manager::findUser( BTRegister* pRegister )
{
	USER_REGISTER_MAP_ITERATOR regiPos;

	{CSingleLock Locker( &m_UserLock, TRUE );

		regiPos = m_UserRegisterMap.find(pRegister);
		if (regiPos==m_UserRegisterMap.end())
		{
			APPLOG->Print( DBGLV_INF, "[findUser] Register not found. Register(0x%08X)", pRegister );
			return NULL;
		}
	}
	
	return regiPos->second;
}

SIP_User* SIP_Manager::getUser( unsigned int nUser )
{
	USER_LIST_ITERATOR userPos;
	
	{CSingleLock Locker( &m_UserLock, TRUE );

		userPos = nUser ? m_UserList.find(nUser):m_UserList.begin();
		if (userPos==m_UserList.end())
		{
			APPLOG->Print( DBGLV_INF, "[getUser] failed to get User. User(%d)", nUser );
			return NULL;
		}
	}
	
	return userPos->second;
}

void SIP_Manager::showUser()
{
    BTStackResourceStatus 	aStat;
    SIP_User* pUser 	= NULL;
    BTRegister* pRegister 	= NULL;
    BTDateTime  eventDateTime;
    BTDateTime  currDateTime;
    time_t      currTime;
    char    strTime[32] = {0, };
    int     nShowCnt    = 0;

    currDateTime.getCTime(&currTime);
    printf("\n      ***** User Informations (%s) *****\n",
        currDateTime.getDateTimeString(strTime, sizeof(strTime), "%Y%m%d-%H%M%S") ? strTime : "-");
    s_pStackManager->getResourceStatus(&aStat);

    printf(" ------------------------------------------------------------------------------\n");
    printf(" * INDEX REGISTER  STATE        EVENT-TIME      EXPIRES AOR                   *\n");
    printf(" ------------------------------------------------------------------------------\n");
	USER_LIST_ITERATOR sPos = m_UserList.begin();
	for ( ; sPos!=m_UserList.end(); sPos++)
	{
        pUser = sPos->second;
		if (pUser) pRegister = pUser->regi();
        if (pUser && pRegister)
        {
            eventDateTime.setDateTimeWithCTime(&pUser->nEventTime);
            APPLOG->Print( DBGLV_INF, "   %05d %08X  %-12s %-14s %7d %s", sPos->first, pRegister, SIP_User::const_UserState[pUser->state()], 
						eventDateTime.getDateTimeString(strTime, sizeof(strTime), "%Y%m%d-%H%M%S") ? strTime : "-",
						pUser->getExpires(), pUser->getTo() );
                nShowCnt++;
        }
    }
    APPLOG->Print( DBGLV_INF, " ------------------------------------------------------------------------------");
    APPLOG->Print( DBGLV_INF, " #> Register=%d/%d(%d%%), Transc=%d/%d(%d%%), \n    Timer=%d/%d(%d%%), Page=%d/%d(%d%%)",
        aStat.nRegisterUsage, aStat.nRegisterMax, aStat.nRegisterUsage*100/aStat.nRegisterMax,
        aStat.nTransactionUsage, aStat.nTransactionMax, aStat.nTransactionUsage*100/aStat.nTransactionMax,
        aStat.nTimerPoolUsage, aStat.nTimerPoolMax, aStat.nTimerPoolUsage*100/aStat.nTimerPoolMax,
        aStat.nPagePoolUsage, aStat.nPagePoolMax, aStat.nPagePoolUsage*100/aStat.nPagePoolMax );
    APPLOG->Print( DBGLV_INF, " COUNT=%d/%d", nShowCnt, m_UserList.size() );
}




//
///** @brief	REJECT 전송 */
//int SIP_Manager::nReject( int nSession, int nCode )
//{
//	SIP_Session* pSession = NULL;
//
//    //SIP_Session* pSession = getSession__(nSession);
//    if( !pSession )
//    {
//        APPLOG->Print( DBGLV_ERR, "# invalid session index(%d)", nSession );
//        return -1;
//    }
//    
//    BTDialog* pDialog = pSession->dialog();
//    if( !pDialog )
//    {
//        APPLOG->Print( DBGLV_ERR, "# not valid dialog." );
//        return -1;
//    }
//
//	if( g_pDlgMgr->reject(pDialog, nCode) != rtSUCCESS ) 
//		return -1;
//
//	return 0;
//}
//
///** @brief	CANCEL 전송 */
//int SIP_Manager::nCancel( int nSession )
//{
//	SIP_Session* pSession = NULL;
//
//    //SIP_Session* pSession = getSession__(nSession);
//    if (!pSession)
//    {
//        APPLOG->Print( DBGLV_ERR, "# invalid session index(%d)", nSession );
//        return -1;
//    }
//    
//    BTDialog* pDialog = pSession->dialog();
//    if (!pDialog)
//    {
//        APPLOG->Print( DBGLV_ERR, "# not valid dialog." );
//        return -1;
//    }
//
//	if (g_pDlgMgr->cancel(pDialog)!=rtSUCCESS) return -1;
//
//	return 0;
//}
//
///** @brief	BYE 전송 */
//int SIP_Manager::nBye( int nSession )
//{
//	SIP_Session* pSession = NULL;
//    //SIP_Session* pSession = getSession__(nSession);
//    if (!pSession)
//    {
//        APPLOG->Print( DBGLV_ERR, "# invalid session index(%d)", nSession );
//        return -1;
//    }
//    
//    BTDialog* pDialog = pSession->dialog();
//    if (!pDialog)
//    {
//        APPLOG->Print( DBGLV_ERR, "# not valid dialog." );
//        return -1;
//    }
//
//	pSession->setBye();
//
//	if (g_pDlgMgr->disconnectCall(pDialog)!=rtSUCCESS) return -1;
//
//	return 0;
//}
//
///** @brief	OPTIONS 전송 */
//int SIP_Manager::nPerfOptions( const char* strTarget )
//{
//	BTTransaction* pTransaction = NULL;
//	g_pTrxMgr->createTransaction(&pTransaction);
//	if (!pTransaction)
//	{
//		APPLOG->Print( DBGLV_ERR, "# fail to create transaction." );
//		return -1;
//	}
//
//    char strFrom[256] = {0, };
//    setFrom( strFrom, CConfig::USER_LOCAL_URI, CConfig::USER_LOCAL_NAME );
//
//	BTSIPMessage* pMessage = pTransaction->outboundMessage();
//    if (!pMessage)
//    {
//        APPLOG->Print( DBGLV_ERR, "# failed to get the message" );
//        return -1;
//    }
//
//    pMessage->makeRequestMessage( "OPTIONS", strFrom, (char*)strTarget, (char*)strTarget );
//    setContact( pMessage, CConfig::USER_LOCAL_URI, CConfig::ADDRESS_LOCAL_PORT );
//
//	APPLOG->Print( DBGLV_ERR, "sending Perf-OPTIONS request." );
//
//	if (g_pTrxMgr->sendRequest(pTransaction, pMessage)!=rtSUCCESS) return -1;
//
//	return 0;
//}

/** @brief	OPTIONS 전송 */
int SIP_Manager::OutdialogOptions( const char* p_szIp, int p_nPort, const char* p_szTargetUser, const char* p_szTargetName)
{
	int		result;
	CString	strToURI, strTo, strFrom, strContact;

	BTTransaction* pTransaction = NULL;
	g_pTrxMgr->createTransaction(&pTransaction);
	if (!pTransaction)
	{
		APPLOG->Print( DBGLV_ERR, "# fail to create transaction." );
		return -1;
	}

    BTSIPMessage* pMessage = pTransaction->outboundMessage();
    if (!pMessage)
    {
        APPLOG->Print( DBGLV_ERR, "# failed to get the message" );
        return -1;
    }

	setUri(		 strToURI,	 p_szIp, p_nPort, p_szTargetUser );
	setLocation( strTo,		 p_szIp, p_nPort, p_szTargetUser, p_szTargetName );
	setLocation( strFrom,	 CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT, CConfig::USER_LOCAL_NAME );
	setLocation( strContact, CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT, CConfig::USER_LOCAL_NAME );
	
	pMessage->makeRequestMessage( "OPTIONS", (char*)(LPCSTR)strFrom, (char*)(LPCSTR)strTo, (char*)(LPCSTR)strToURI );
	pMessage->parseContactHeaderWithString( (char*)(LPCSTR)strContact );

	result = pTransaction->sendRequest( pMessage );
	if( result != rtSUCCESS ) 
	{
		APPLOG->Print( DBGLV_ERR, "sending out-dialog OPTIONS request." );
		return -1;
	}

	APPLOG->Print( DBGLV_RPT, "sending out-dialog OPTIONS request." );

	return 0;
}

static int nIndialogOptions( SIP_Session* pSession )
{
/*
	BTTransaction* pTransaction = NULL;
	BTDialog* pDialog = pSession->dialog();
	g_pDlgMgr->createTransaction(pDialog, &pTransaction);
	if (!pTransaction)
	{
		APPLOG->Print( DBGLV_ERR, "# failed to create transaction." );
		return -1;
	}

	APPLOG->Print( DBGLV_ERR, "sending in-dialog OPTIONS request." );

	if (g_pDlgMgr->sendTransactionRequest(pDialog, pTransaction, "OPTIONS")!=rtSUCCESS) return -1;
*/
	BTDialog* pDialog = pSession->dialog();
	if(!pDialog)
		return	0;

	g_pAppMgr->sendNotification(APPEVT_OPTION_CALL, StackObject_Dialog, pDialog, 0);
	
	return 0;
}

//int SIP_Manager::nOptions( int nSession )
//{
//	if (!nSession) return nOutdialogOptions();
//
//	SIP_Session* pSession = NULL;
//    //SIP_Session* pSession = getSession__(nSession);
//    if (!pSession)
//    {
//    	APPLOG->Print( DBGLV_ERR, "# invalid session index(%d)", nSession );
//    	return -1;
//    }
//	return nIndialogOptions(pSession);
//}


/** @brief	REFER 전송 */
int SIP_Manager::nRefer( const char* strTarget )
{
/* aaa
	SIP_Session* pSession = NULL;
    //SIP_Session* pSession = getSession__(0);
    if (!pSession)
    {
        APPLOG->Print( DBGLV_ERR, "# can't get any session." );
        return -1;
    }
    
    BTDialog* pDialog = pSession->dialog();
    if (!pDialog)
    {
        APPLOG->Print( DBGLV_ERR, "# not valid dialog." );
        return -1;
    }

	if (g_pDlgMgr->refer(pDialog, (char*)strTarget)!=rtSUCCESS) return -1;
	pSession->setTransfer();
*/
	return 0;
}

/** @brief	REGISTER 전송 */
int SIP_Manager::nRegister( int nExpires )
{
/* aaa
	SIP_User* pUser = newUser();
	if( !pUser )
    {
        APPLOG->Print( DBGLV_ERR, "# failed to get the new user" );
        return -1;
    }

    BTRegister* pRegister = pUser->regi();
	if( !pRegister )
		g_pRegiMgr->createRegister(&pRegister);
	if( !pRegister )
	{
        APPLOG->Print( DBGLV_ERR, "# failed to get the register" );
        return -1;
	}

    BTTransaction* pTransaction = pRegister->getTransaction();
    BTSIPMessage* pMessage = pTransaction->outboundMessage();
    if (!pMessage)
    {
		BTSIPMessage dummyMessage;
        pTransaction->setOutboundMessage(&dummyMessage);
        pMessage = pTransaction->outboundMessage();
	}
	if (!pMessage)
	{
        APPLOG->Print( DBGLV_ERR, "# failed to get the SIP message" );
        return -1;
    }

	pMessage->clearMessage();
    pUser->setRegister(pRegister);

    char    strRegistra[64] = {0, };
    char    strContact[64]  = {0, };
	CString strMgwIp;
	int		mgwPort;
	bool	bPrimary;

    sprintf( strContact,  "<%s>", CConfig::USER_LOCAL_URI );
    //g_pTrxMgr->setOutboundProxy(g_pConfig->strServerIP, g_pConfig->nServerPort);

	if( SIP_Mgws::Inst()->HuntMgw( &bPrimary, &strMgwIp, &mgwPort ) )
		return -1;

    sprintf( strRegistra, "sip:%s:%d", strMgwIp, mgwPort );
    //sprintf(strRegistra, "sip:registra.bridgetec.co.kr");

    if (g_pRegiMgr->registerUser(pRegister, (char*)(LPCSTR)CConfig::USER_LOCAL_URI, (char*)(LPCSTR)CConfig::USER_LOCAL_URI,
                strRegistra, strContact, nExpires)!=rtSUCCESS)
        return -1;
*/

	return 0;
}




/** @brief 	메시지 처리용 함수 */
//int SIP_Manager::clrServHeader()
//{
//	m_SIPMessage.deleteMessage();
//	return 0;
//}
//
//int SIP_Manager::addServHeader( const char* strName, const char* strValue )
//{
//	BTSIPParser	aParser;
//	unsigned int nRule;
//	int	nResult	= -1;
//
//	char strHeader[256] = {0, };
//	sprintf(strHeader, "%s: %s", strName, strValue);
//
//    APPLOG->Print( DBGLV_INF, "try to add header \"%s\"", strHeader);
//
//	nRule = aParser.getRuleNameWithHeaderName( (char*)strName );
//	switch (nRule)
//	{
//       	case BTSIPParser::ID_FROM:
//			nResult = m_SIPMessage.parseFromHeaderWithString( (char*)strValue );
//			break;
//       	case BTSIPParser::ID_TO:
//			nResult = m_SIPMessage.parseToHeaderWithString( (char*)strValue );
//			break;
//       	case BTSIPParser::ID_CONTACT:
//			nResult = m_SIPMessage.parseContactHeaderWithString( (char*)strValue );
//			break;
//       	case BTSIPParser::ID_ROUTE:
//			nResult = m_SIPMessage.parseRouteHeaderWithString( (char*)strValue );
//			break;
//       	case BTSIPParser::ID_RECORD_ROUTE:
//			nResult = m_SIPMessage.parseRecordRouteHeaderWithString( (char*)strValue );
//			break;
//       	case BTSIPParser::ID_REFER_TO:
//			nResult = m_SIPMessage.parseReferToHeaderWithString( (char*)strValue );
//			break;
//    	case BTSIPParser::ID_REFERRED_BY:
//			nResult = m_SIPMessage.parseReferredByHeaderWithString( (char*)strValue );
//			break;
//    	case BTSIPParser::ID_P_ASSERTED_IDENTITY:
//			nResult = m_SIPMessage.parseAssertedIdentityHeaderWithString( (char*)strValue );
//			break;
//		default:
//			nResult = m_SIPMessage.parseOtherHeaderWithString( (char*)strName, (char*)strValue );
//			break;
//	}
//
//	if (nResult!=rtSUCCESS) 	
//	{
//        APPLOG->Print( DBGLV_ERR, "# Failed to add header \"%s\"", strHeader );
//		return -1;
//	}
//
//	return nResult;
//}
//
//int SIP_Manager::delServHeader( const char* strName )
//{
//	BTSIPHeaderList::BTSIPHeaderListIterator iGotPos;
//
//    APPLOG->Print( DBGLV_INF, "try to delete header \"%s\"", strName );
//
//	if( m_SIPMessage.headerList()->firstHeaderWithName((char*)strName, iGotPos) )
//	{
//		m_SIPMessage.deleteHeaderFromHeaderList(iGotPos.value());
//		return 0;
//	}
//	else
//	{
//        APPLOG->Print( DBGLV_INF, "# Failed to delete header \"%s\"", strName );
//		return -1;
//	}
//}
//
//int SIP_Manager::setServHeader( BTSIPMessage* pMessage )
//{
//    bool bResult = pMessage->headerList()->copyHeaderList(m_SIPMessage.headerList());
//    if(!bResult)
//    {
//        g_pStackLog->writeLog(BTLogFile::LogLevelError, "# Failed to set header list\n");
//		return -1;
//    }
//
//	int nResult = delServHeader("Route");
//	while (!nResult)
//	{
//		nResult = delServHeader("Route");
//	}
//
//	return 0;
//}
//
//int SIP_Manager::getServHeader( BTSIPMessage* pMessage )
//{
//	char strBuffer[128] = {0, };
//	int nReturnLength 	= 0;
//	
//	g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "# INCOMMING SIP MESSAGE From %s:%d(%s)\n", pMessage->sourceIP(), pMessage->sourcePort(), (pMessage->transport()==BTSIPMessage::Transport_TCP) ? "TCP":"UDP");  
//
//	BTSIPAddressHeader* pAssertedIdentity = pMessage->AssertedIdentityHeader();
//	if (pAssertedIdentity && pAssertedIdentity->URI())
//		g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "ANI(Asserted)", pAssertedIdentity->URI()->userInfo());
//	BTSIPContactHeader* pContactHeader = pMessage->ContactHeader();
//	if (pContactHeader && pContactHeader->URI())
//		g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "ANI(Contact)", pContactHeader->URI()->userInfo());
//	BTSIPFromHeader* pFromHeader = pMessage->FromHeader();
//	if (pFromHeader && pFromHeader->URI())
//		g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "ANI(From)", pFromHeader->URI()->userInfo());
//	
//	BTSIPRequestLine* pRequestLine = pMessage->requestLine();
//	if (pRequestLine && pRequestLine->requestURI())
//		g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "DN(RURI)", pRequestLine->requestURI()->userInfo());
//	BTSIPToHeader* pToHeader = pMessage->ToHeader();
//	if (pToHeader && pToHeader->URI())
//		g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "DN(To)", pToHeader->URI()->userInfo());
//
//	BTSIPOtherHeader* pSessionExpires = pMessage->OtherHeader("Session-Expires");
//	if (pSessionExpires)
//	{
//		pSessionExpires->getOtherHeaderWithBuffer(strBuffer, sizeof(strBuffer), nReturnLength);
//		g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "Session-Expires", strBuffer);
//	}
//
//	BTSIPReferredByHeader* pReferredByHeader = pMessage->ReferredByHeader();
//	if (pReferredByHeader)
//	{
//		string strRefererURI;
//		pReferredByHeader->URI()->getURIString(strRefererURI);
//		if (strRefererURI.length())
//		{
//			g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "Referred-By", strRefererURI.c_str());
//			g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "cid",
//				pReferredByHeader->referredById() ? pReferredByHeader->referredById():"null");
//			g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "user",
//				pReferredByHeader->referredUser() ? pReferredByHeader->referredUser():"null");
//			g_pStackLog->writeLog(BTLogFile::LogLevelDebug, "+ %16s : %s\n", "addr",
//				pReferredByHeader->referredAddress() ? pReferredByHeader->referredAddress():"null");
//		}
//	}
//	
//	return 0;
//}





///// Implementaion //////////////////////////////////////////





///// Field //////////////////////////////////////////////////




