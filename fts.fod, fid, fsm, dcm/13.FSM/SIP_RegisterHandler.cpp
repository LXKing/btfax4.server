#include "stdafx.h"
#include "SIP_Manager.h"
#include "SIP_RegisterHandler.h"
#include "SIP_Utility.h"


const char const_RegisterState[BTRegister::NUM_OF_REGISTER_STATE][64] =
{
	"IDLE",
	"REGISTERING",
	"UNREGISTERING",
	"UNAUTHENTICATED",
	"REDIRECTED",
	"REGISTER_RECEIVED",
	"UNREGISTER_RECEIVED",
	"REGISTERED",
	"FAILED",
	"TERMINATED",
};

const char const_RegisterReason[BTRegister::NUM_OF_REGISTER_REASON][64] =
{
	"Unknown",
	"REGISTER Received",
	"UNREGISTER Received",
	"SUCCESS(2xx) Response Received",
	"REDIRECT(3xx) Response Received",
	"UNAUTHENTICATED(401or407) Response Received",
	"ERROR Response Received",
	"REGISTER Sent",
	"UNREGISTER Sent",
	"SUCCESS Response Sent",
	"ERROR Response Sent",
	"Timer Expired",
	"Local Terminated",
	"Forced Terminated",
};



///// Constructor, Destructor ////////////////////////////////

SIP_RegisterHandler::SIP_RegisterHandler()
{
}

SIP_RegisterHandler::~SIP_RegisterHandler()
{
}




///// Method /////////////////////////////////////////////////




///// Callback ///////////////////////////////////////////////
SIP_RegisterHandler::REGISTER_SERV 
SIP_RegisterHandler::s_ServREGISTER[BTRegister::NUM_OF_REGISTER_STATE] =
{
	&SIP_RegisterHandler::_RegiIdle,
	&SIP_RegisterHandler::_RegiRegistering,
	&SIP_RegisterHandler::_RegiUnregistering,
	&SIP_RegisterHandler::_RegiUnauthenticated,
	&SIP_RegisterHandler::_RegiRedirected,
	&SIP_RegisterHandler::_RegiRegisterReceived,
	&SIP_RegisterHandler::_RegiRegisterReceived,
	&SIP_RegisterHandler::_RegiRegistered,
	&SIP_RegisterHandler::_RegiFailed,
	&SIP_RegisterHandler::_RegiTerminated
};

void SIP_RegisterHandler::_RegisterStateChanged( unsigned int nState, unsigned int nReason, BTRegister* pRegister )
{
	CString	strSessionInfo;

	SIP_Session::getEmptyInfo( &strSessionInfo );

	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[Dialog registerStateChanged] begin. state=%d:%s, reason=%s, Register(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_RegisterState[nState], const_RegisterReason[nReason], pRegister );

	if( s_ServREGISTER[ nState ] )
		s_ServREGISTER[ nState ]( nReason, pRegister );

	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[Dialog registerStateChanged] end.   state=%d:%s, reason=%s, Register(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_RegisterState[nState], const_RegisterReason[nReason], pRegister );
}

int SIP_RegisterHandler::_RegiIdle( unsigned int nReason, BTRegister* pRegister )
{
    return 0;
}

int SIP_RegisterHandler::_RegiRegistering( unsigned int nReason, BTRegister* pRegister )
{
    return 0;
}

int SIP_RegisterHandler::_RegiUnregistering( unsigned int nReason, BTRegister* pRegister )
{
    return 0;
}

int SIP_RegisterHandler::_RegiUnauthenticated( unsigned int nReason, BTRegister* pRegister )
{
    return 0;
}

int SIP_RegisterHandler::_RegiRedirected( unsigned int nReason, BTRegister* pRegister )
{
    return 0;
}

int SIP_RegisterHandler::_RegiRegisterReceived( unsigned int nReason, BTRegister* pRegister )
{
    SIP_User* pUser = SIP_Manager::inst->findUser(pRegister);
    if (!pUser)
    	pUser = SIP_Manager::inst->newUser(pRegister);
    if (!pUser)
    {
        APPLOG->Print( DBGLV_ERR, "# failed to get user" );
        return -1;
    }

	BTTransaction* pTransaction = pRegister->getTransaction();
	BTSIPMessage* pMessage = pTransaction->receivedMessage();

	char strHeader[256] = {0, };
	int nHeaderLength = 0;
	pMessage->getToHeaderWithBuffer(strHeader, sizeof(strHeader), nHeaderLength, false); 
    APPLOG->Print( DBGLV_INF, "*---- USER INFO ----*", pRegister );
    APPLOG->Print( DBGLV_INF, "* AOR(To) : %s", strHeader );
	
	BTSIPContactHeader* pContactHeader = pMessage->ContactHeader();
	pContactHeader->getContactHeaderWithBuffer(strHeader, sizeof(strHeader), nHeaderLength, false);

    APPLOG->Print( DBGLV_INF, "* Contact : %s\n", strHeader );
	int nExpires = pContactHeader->paramExpires();
	if (nExpires < 0)
	{
		int nResult = pMessage->getOtherHeaderWithBuffer("Expires", strHeader, sizeof(strHeader), nHeaderLength, false);
		if (nResult!=rtSUCCESS)
        	APPLOG->Print( DBGLV_ERR, "# Expires not found" );
		else
			nExpires = atoi(strHeader);
	}
    APPLOG->Print( DBGLV_INF, "* Expires : %d", nExpires );
    APPLOG->Print( DBGLV_INF, "* R-Addr  : %s:%d", pMessage->sourceIP(), pMessage->sourcePort() );
	
    if( CConfig::SERVICE_DELAY ) 
		BTSystem::sleepEx( CConfig::SERVICE_DELAY, 0 );

    BTSIPMessage* pSendMessage = pTransaction->outboundMessage();
   	if (!pSendMessage)
    {   
        BTSIPMessage dummyMessage;
        pTransaction->setOutboundMessage(&dummyMessage);
        pSendMessage = pTransaction->outboundMessage();
    } 
	/*if (g_pCMDHandler->nStatus==200)
    {
		setContact( pSendMessage, CConfig::USER_LOCAL_URI, CConfig::ADDRESS_LOCAL_PORT );
        setExpires( pSendMessage, CConfig::SERVICE_SESSION_EXPIRES );
        g_pRegisterManager->accept(pRegister, 200);
		pUser->setTrying();
    }
    else
    {
        g_pRegisterManager->reject(pRegister, g_pCMDHandler->nStatus);
		pUser->setReject();
    }*/

	return 0;
}

int SIP_RegisterHandler::_RegiRegistered( unsigned int nReason, BTRegister* pRegister )
{
    /*SIMUser* pUser = g_pSIMServ->findUser(pRegister);
    if (!pUser)
    {
        g_pLog->writeLog(BTLogFile::LogLevelError, "# failed to get user.\n");
        return -1;
    }	

	char	strTmp[128] = {0, };
	char	strTo[128] 	= {0, };
	int		nTmpLength 	= 0, i, j;
	BTSIPMessage* pMessage = pRegister->getTransaction()->receivedMessage();
	if (pMessage->getToHeaderWithBuffer(strTmp, sizeof(strTmp), nTmpLength, false))
	{
		APPLOG->Print( DBGLV_ERR, "failed to get address of record." );
		return -1;
	}
	for (i=0, j=0; strTmp[i]!='\0'; i++)
	{
		if (strTmp[i]=='>' || strTmp[i]==';') break;
		if (strTmp[i]=='<') continue;
		strTo[j++] = strTmp[i];
	}
	strTo[j] = '\0';
	APPLOG->Print( DBGLV_INF, "Address of recored : %s", strTo ); 
	pUser->setTo(strTo);

	if (pMessage->getOtherHeaderWithBuffer("Expires", strTmp, sizeof(strTmp), nTmpLength, false))
	{
		APPLOG->Print( DBGLV_ERR, "failed to get expires." );
		return -1;
	}
	APPLOG->Print( DBGLV_INF, "Expires : %s", strTmp ); 
	int 	nExpires	= atoi(strTmp);
	APPLOG->Print( DBGLV_INF, "Registered completed. (expires=%d)", nExpires ); 
	pUser->setExpires(nExpires);
	if (nExpires)
	{
    	BTDateTime  currentDateTime;
    	currentDateTime.getCTime(&pUser->nEventTime);
		pUser->setRegistered();
	}
	else
	{
		pUser->setIdle();
		g_pSIMServ->deleteUser(pUser);
	}*/

	return 0;
}

int SIP_RegisterHandler::_RegiFailed( unsigned int nReason, BTRegister* pRegister )
{
    return 0;
}

int SIP_RegisterHandler::_RegiTerminated( unsigned int nReason, BTRegister* pRegister )
{
    return 0;
}



///// Implementaion //////////////////////////////////////////





///// Field //////////////////////////////////////////////////




