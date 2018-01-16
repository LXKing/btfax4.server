#include "stdafx.h"
#include "SIP_DialogHandler.h"

#include "SIP_Manager.h"
#include "SIP_Dialog_StateChanged.h"
#include "SIP_Dialog_TranscStateChanged.h"
#include "SIP_Dialog_ModifyStateChanged.h"
//#include "SIP_Dialog_ReferStateChanged.h"
//#include "SIP_Dialog_ReferNotifyEvent.h"
#include "SIP_Utility.h"
#include "alloc.h"
#include "TCP_Server.h"


const char const_DialogSessionTimerReason[BTDialog::NUM_OF_DIALOG_SESSION_TIMER_REASON][64] =
{
	"Unknown",
	"Session Expired",
	"422 Received"
};


///// Constructor, Destructor ////////////////////////////////

SIP_Dialog::SIP_Dialog()
{
}

SIP_Dialog::~SIP_Dialog()
{
}




///// Method /////////////////////////////////////////////////




///// Callback ///////////////////////////////////////////////
void SIP_Dialog::_DialogStateChanged( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
	SIP_Dialog_StateChanged::_Callback(nState, nReason, pDialog, pMessage);
}

void SIP_Dialog::_DialogTranscStateChanged( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransc, BTSIPMessage* pMessage )
{
	SIP_Dialog_TranscStateChanged::_Callback( nState, nReason, pDialog, pTransc, pMessage );
}

void SIP_Dialog::_DialogModifyStateChanged( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
	SIP_Dialog_ModifyStateChanged::_Callback( nState, nReason, pDialog, pMessage );
}

void SIP_Dialog::_DialogPrackStateChanged( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTTransaction * pTransc, BTSIPMessage* pMessage )
{
}

void SIP_Dialog::_DialogSessionTimerEvent( unsigned int nReason, BTDialog* pDialog )
{
	CString		 strSessionInfo;
	SIP_Session* pSession;

	pSession = SIP_Manager::inst->FindSession( pDialog );;
	if( pSession ) pSession->getInfo( &strSessionInfo );
	else		   SIP_Session::getEmptyInfo( &strSessionInfo );

	APPLOG->Print( DBGLV_INF, "#----> %s CALLBACK[DIALOG_SESSION_TIMER_EVENT] Reason: %s", (LPCSTR)strSessionInfo, const_DialogSessionTimerReason[nReason] );

	if(!pSession)
		return;

	pSession->bye();
}

void SIP_Dialog::_DialogSessionTimerRefreshAlert(BTDialog* pDialog)
{
	CString		 strSessionInfo;
	SIP_Session* pSession;

	pSession = SIP_Manager::inst->FindSession( pDialog );;
	if( pSession ) pSession->getInfo( &strSessionInfo );
	else		   SIP_Session::getEmptyInfo( &strSessionInfo );

	APPLOG->Print( DBGLV_INF, "#----> %s CALLBACK[DIALOG_SESSION_TIMER_ALERT]", (LPCSTR)strSessionInfo);

	if(!pSession)
		return;

	pSession->dialog()->outboundMessage()->clear();
	pSession->dialog()->refresh();
}

void SIP_Dialog::_DialogMessageReceived(BTSIPMessage * pMessage)
{
	printSIPMessage(pMessage, true, true);
}

void SIP_Dialog::_DialogMessageSending(BTSIPMessage * pMessage)
{
	printSIPMessage(pMessage, false, true);
}
