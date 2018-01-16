#include "stdafx.h"
#include "SIP_Dialog_ReferStateChanged.h"
#include "SIP_Data.h"
#include "SIP_Manager.h"
#include "SIP_Utility.h"



///// Constructor, Destructor ////////////////////////////////
SIP_Dialog_ReferStateChanged::SIP_Dialog_ReferStateChanged()
{
}

SIP_Dialog_ReferStateChanged::~SIP_Dialog_ReferStateChanged()
{
}

///// Callback ///////////////////////////////////////////////
SIP_Dialog_ReferStateChanged::DIALOG_REFER_SERV 
SIP_Dialog_ReferStateChanged::s_ServDIALOGREF[BTDialog::NUM_OF_DIALOG_REFER_STATE] =
{
	&SIP_Dialog_ReferStateChanged::_DialogRIdle,
	&SIP_Dialog_ReferStateChanged::_DialogRSent,
	&SIP_Dialog_ReferStateChanged::_DialogRCalling,
	&SIP_Dialog_ReferStateChanged::_DialogRUnauthenticated,
	&SIP_Dialog_ReferStateChanged::_DialogRRedirected,
	&SIP_Dialog_ReferStateChanged::_DialogRReceived
};

int SIP_Dialog_ReferStateChanged::_Callback( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
	int			 nResult;
	CString		 strSessionInfo;
	SIP_Session* pSession;

	
	pSession = SIP_Manager::inst->FindSession( pDialog );;
	if( pSession ) pSession->getInfo( &strSessionInfo );
	else		   SIP_Session::getEmptyInfo( &strSessionInfo );


	APPLOG->Print( DBGLV_INF, 
					"%s CALLBACK[Dialog referStateChanged] begin. state=%d:%s, reason=%s, Message(0x%08X)", 
					(LPCSTR)strSessionInfo, nState, BTDialog::const_DialogReferState[nState], BTDialog::const_DialogReason[nReason], pMessage );

	if( s_ServDIALOGREF[ nState ] )
		nResult = s_ServDIALOGREF[ nState ]( nReason, pDialog, pMessage );

	APPLOG->Print( DBGLV_INF, 
					"%s CALLBACK[Dialog referStateChanged] end.   state=%d:%s, reason=%s, Message(0x%08X)", 
					(LPCSTR)strSessionInfo, nState, BTDialog::const_DialogReferState[nState], BTDialog::const_DialogReason[nReason], pMessage );

	return nResult;
}

int SIP_Dialog_ReferStateChanged::_DialogRIdle( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ReferStateChanged::_DialogRSent( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ReferStateChanged::_DialogRCalling( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ReferStateChanged::_DialogRUnauthenticated( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ReferStateChanged::_DialogRRedirected( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ReferStateChanged::_DialogRReceived( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    SIP_Session* pSession = SIP_Manager::inst->FindSession( pDialog );
	if (!pSession)
	{
		APPLOG->Print( DBGLV_ERR, "# failed to get session." );
		g_pDialogManager->referReject(pDialog, 503);
        return -1;
    }

	char 	aBuffer[256]	= {0, };	/* ReferTo 를 저장하기 위한 Buffer */
	int 	nBufferLength	= sizeof(aBuffer);
	int 	nReturnLength 	= 0;
	int		nResult			= 0;
	nResult = pMessage->getReferToHeaderWithBuffer(aBuffer, nBufferLength, nReturnLength, false);
	if (nResult!=rtSUCCESS)
	{
		APPLOG->Print( DBGLV_ERR, "# failed to get Refer-To header." );
		g_pDialogManager->referReject(pDialog, 400);
	}

	BTDialog* pNewDialog 	= NULL;
	pSession->setReferDialog(pDialog);
	pSession->setReferCSeq( SIP_Manager::inst->referCount() );
	nResult = g_pDialogManager->referAccept(pDialog, true, &pNewDialog);
	if (nResult!=rtSUCCESS)
	{
		APPLOG->Print( DBGLV_ERR, "# failed to accept REFER request." );
		return -1;
	}
	
	SIP_Manager::inst->newDialog(pSession, pNewDialog);
	BTSIPMessage* pSendMessage = pNewDialog->outboundMessage();
	if (!pSendMessage)
	{
        APPLOG->Print( DBGLV_ERR, "[%s] failed to get SIP message", __FUNCTION__ );
		g_pDialogManager->referNotify(pDialog, 500, pSession->referCSeqStep());
        return -1;
    }

    char strFrom[256] = {0, };
    setFrom(strFrom, (char*)(LPCSTR)CConfig::USER_LOCAL_URI, CConfig::USER_LOCAL_NAME);
    pSendMessage->clearMessage();
    setContact( pSendMessage, CConfig::USER_LOCAL_URI );
    pSession->setSDPMessage(pSendMessage);
    pSession->setTrying();

    if( g_pDialogManager->makeCall(pNewDialog, strFrom, aBuffer) != rtSUCCESS ) 
		return -1;

	return 0;
}

