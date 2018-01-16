#include "stdafx.h"
#include "SIP_App.h"
#include "SIP_Dialog_StateChanged.h"
#include "SIP_Manager.h"
#include "SIP_CallingTimer.h"
#include "SIP_Utility.h"
#include "TCP_Server.h"
#include "alloc.h"
#include "Utility.h"


char	* aename[]={
	"MakeCall",
	"ProceedCall",
	"AcceptCall",
	"RejectCall",
	"DiscCall",
	"ModifyCall",
};


void	AppMakeCall(uint objtype, void * pobj, void * pappdata)
{
	BTSIPMessage * pmsg;
	BTDialog * pdia;
	SIP_Session* pSession;

	pdia=(BTDialog *)pobj;
	pSession = SIP_Manager::inst->FindSession( pdia );
	if(!pSession)
		return;

	pmsg=pdia->outboundMessage();
	pmsg->clear();

	pSession->setSDPMessage( pmsg );

	pdia->prepareCall();

	pSession->resetFstate();
	BTSIPCallIDHeader* pCallId = pmsg->CallIDHeader();
	if( pCallId )
		pSession->setCallId( pCallId->callID() );

	pdia->make();
}

void	AppProceedCall(uint objtype, void * pobj, void * pappdata)
{
	BTDialog * pdia;
	SIP_Session* pSession;

	pdia=(BTDialog *)pobj;
	pSession = SIP_Manager::inst->FindSession( pdia );
	if(!pSession)
		return;

	pdia->outboundMessage()->clear();
	pdia->proceed((int)pappdata);
}

void	AppAcceptCall(uint objtype, void * pobj, void * pappdata)
{
	BTDialog * pdia;
	SIP_Session* pSession;

	pdia=(BTDialog *)pobj;
	pSession = SIP_Manager::inst->FindSession( pdia );
	if(!pSession)
		return;

	pdia=(BTDialog *)pobj;
	pdia->outboundMessage()->clear();
	pdia->accept((int)pappdata);
}

void	AppRejectCall(uint objtype, void * pobj, void * pappdata)
{
	BTDialog * pdia;
	SIP_Session* pSession;

	pdia=(BTDialog *)pobj;
	pSession = SIP_Manager::inst->FindSession( pdia );
	if(!pSession)
		return;

	pdia->outboundMessage()->clear();
	pdia->reject((int)pappdata);
}

void	AppDiscCall(uint objtype, void * pobj, void * pappdata)
{
	BTDialog * pdia;
	SIP_Session* pSession;

	pdia=(BTDialog *)pobj;
	pSession = SIP_Manager::inst->FindSession( pdia );
	if(!pSession)
		return;

	pdia->outboundMessage()->clear();
	pdia->disconnect();
}

void	AppModifyCall(uint objtype, void * pobj, void * pappdata)
{
	BTSIPMessage * pmsg;
	BTDialog * pdia;
	SIP_Session* pSession;

	pdia=(BTDialog *)pobj;
	pSession = SIP_Manager::inst->FindSession( pdia );
	if(!pSession)
		return;
	
	pmsg=pdia->outboundMessage();
	pmsg->clear();

	pSession->setSDPMessage( pmsg, true );
	
	pdia->modify();
}

void	AppInfoCall(uint objtype, void * pobj, void * pappdata)
{
	BTDialog * pdia;
	SIP_Session* pSession;

	pdia=(BTDialog *)pobj;
	pSession = SIP_Manager::inst->FindSession( pdia );
	if(!pSession)
		return;
}

void	AppOptionCall(uint objtype, void * pobj, void * pappdata)
{
	BTTransaction * ptran;
	BTDialog * pdia;
	SIP_Session* pSession;

	pdia=(BTDialog *)pobj;
	pSession = SIP_Manager::inst->FindSession( pdia );
	if(!pSession)
		return;

	pdia->createTransaction(&ptran);
	pdia->outboundMessage()->clear();
	pdia->sendRequest("OPTIONS", ptran);
}

void	AppEventCallback(uint event, uint objtype, void * pobj, void * pappdata)
{
	APPLOG->Print( DBGLV_INF, "AppEventCallback begin (%08X), event=%d_%s, objtype=%d.\n", pobj, event, aename[event], objtype);

	if(objtype!=StackObject_Dialog)
		return;

	switch(event){
	case APPEVT_MAKE_CALL:
		AppMakeCall(objtype, pobj, pappdata);
		break;
	case APPEVT_PROCEED_CALL:
		AppProceedCall(objtype, pobj, pappdata);
		break;
	case APPEVT_ACCEPT_CALL:
		AppAcceptCall(objtype, pobj, pappdata);
		break;
	case APPEVT_REJECT_CALL:
		AppRejectCall(objtype, pobj, pappdata);
		break;
	case APPEVT_DISCONNECT_CALL:
		AppDiscCall(objtype, pobj, pappdata);
		break;
	case APPEVT_MODIFY_CALL:
		AppModifyCall(objtype, pobj, pappdata);
		break;
	case APPEVT_INFO_CALL:
		AppInfoCall(objtype, pobj, pappdata);
		break;
	case APPEVT_OPTION_CALL:
		AppOptionCall(objtype, pobj, pappdata);
		break;
	default:
		break;
	}

	APPLOG->Print( DBGLV_INF, "AppEventCallback end (%08X), event=%d_%s, objtype=%d.\n", pobj, event, aename[event], objtype);
}
