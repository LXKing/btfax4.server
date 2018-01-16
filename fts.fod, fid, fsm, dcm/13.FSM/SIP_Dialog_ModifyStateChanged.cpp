#include "stdafx.h"
#include "SIP_App.h"
#include "SIP_Dialog_ModifyStateChanged.h"

#include "SIP_Data.h"
#include "SIP_Manager.h"
#include "SIP_CallingTimer.h"
#include "SIP_Utility.h"
#include "TCP_Server.h"
#include "alloc.h"
#include "Utility.h"


const char const_DialogModifyState[BTDialog::NUM_OF_DIALOG_MODIFY_STATE][64] =
{
	"IDLE",
	"REINVITE SENT",
	"REINVITE PROCEEDING",
	"REINVITE CANCELING",
	"REINVITE SUCCESS",
	"REINVITE FAILURE",
	"REINVITE RECEIVED",
	"REINVITE RESPONSE SENT",
	"REINVITE CANCELED"
};

///// Constructor, Destructor ////////////////////////////////
SIP_Dialog_ModifyStateChanged::SIP_Dialog_ModifyStateChanged()
{
}

SIP_Dialog_ModifyStateChanged::~SIP_Dialog_ModifyStateChanged()
{
}

///// Callback ///////////////////////////////////////////////
SIP_Dialog_ModifyStateChanged::DIALOG_MODIFY_SERV 
SIP_Dialog_ModifyStateChanged::s_ServDIALOGMOD[BTDialog::NUM_OF_DIALOG_MODIFY_STATE] =
{
	&SIP_Dialog_ModifyStateChanged::_DialogMIdle,
	&SIP_Dialog_ModifyStateChanged::_DialogMReINVITESent,
	&SIP_Dialog_ModifyStateChanged::_DialogMProceeding,
	&SIP_Dialog_ModifyStateChanged::_DialogMCanceling,
	&SIP_Dialog_ModifyStateChanged::_DialogMSucceeded,
	&SIP_Dialog_ModifyStateChanged::_DialogMFailed,
	&SIP_Dialog_ModifyStateChanged::_DialogMReINVITEReceived,
	&SIP_Dialog_ModifyStateChanged::_DialogMResponseSent,
	&SIP_Dialog_ModifyStateChanged::_DialogMCanceled,
};

int SIP_Dialog_ModifyStateChanged::_Callback( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
	int			 nResult;
	CString		 strSessionInfo;
	SIP_Session* pSession;

	
	pSession = SIP_Manager::inst->FindSession( pDialog );;
	if( pSession ) pSession->getInfo( &strSessionInfo );
	else		   SIP_Session::getEmptyInfo( &strSessionInfo );

	
	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[Dialog-Modify StateChanged] begin. state=%d:%s, reason=%s, Message(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_DialogModifyState[nState], const_DialogReason[nReason], pMessage );

	nResult = s_ServDIALOGMOD[ nState ]( nReason, pDialog, pMessage );

	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[Dialog-Modify StateChanged] end.   state=%d:%s, reason=%s, Message(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_DialogModifyState[nState], const_DialogReason[nReason], pMessage );

	return nResult;
}

int SIP_Dialog_ModifyStateChanged::_DialogMIdle( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
	_DialogMConnected(nReason, pDialog, pMessage);

	return 0;
}

int SIP_Dialog_ModifyStateChanged::_DialogMReINVITESent( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ModifyStateChanged::_DialogMProceeding( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ModifyStateChanged::_DialogMCanceling( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ModifyStateChanged::_DialogMSucceeded( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    //printSIPMessage( pMessage, true );

	SIP_Session*	pSession;

	// Get Session
	pSession = SIP_Manager::inst->FindSession( pDialog );
    if( !pSession )
    {
		APPLOG->Print( DBGLV_ERR, "Dialog(0x%08X) : failed to get session.", pDialog );
		SIP_Session::byeNoSession( pDialog );
        return -1;
    }

	//#######################################################################################
	// Get SDP Message

	BTSDPMessage	aSDPMessage;
	int				remotePort;
	unsigned int	connIndex = 0;
	string			strFrom, strTo, strRemote;

	pMessage->parseSDPWith(aSDPMessage);
	
	// Get From / To 
	BTSIPFromHeader* pFrom	= pMessage->FromHeader();
	BTSIPToHeader*	 pTo	= pMessage->ToHeader();
	
	pFrom->URI()->getURIString( strFrom );
	pTo->URI()->getURIString( strTo );

	// Get Remote IP 
	BTSDPConnectionField* pConnField = (BTSDPConnectionField*)aSDPMessage.firstSessionFieldWithType( BTSDPParser::SESSION_CONNECTION, connIndex );
	if( !pConnField )
	{
		APPLOG->Print( DBGLV_ERR, "Not Exist Connection Field" );
		pSession->bye();
		return -1;
	}
	strRemote = pConnField->connectionAddress();
	
	// Get image(fax) port 
	BTSDPMediaField* pMediaField = (BTSDPMediaField*)aSDPMessage.getMediaWithType( "image" );
	if( !pMediaField )
	{
		APPLOG->Print( DBGLV_ERR, "Not Exist Image Media" );
		pSession->bye();
		return -1;
	}
	remotePort = pMediaField->port();

	// Set DevState Info of Session
	//	ToHeader	: G/W
	//	FromHeader	: FSM
	pSession->setSessionInfoInfo( strTo.c_str(), strFrom.c_str(), strRemote.c_str(), remotePort );
	//#######################################################################################

	_DialogMConnected(nReason, pDialog, pMessage);
	return 0;
}

int SIP_Dialog_ModifyStateChanged::_DialogMFailed( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ModifyStateChanged::_DialogMReINVITEReceived( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    //printSIPMessage( pMessage, true );

	SIP_Session* pSession = SIP_Manager::inst->FindSession( pDialog );
    if( !pSession )
    {
		APPLOG->Print( DBGLV_ERR, "Dialog(0x%08X) : failed to get session.", pDialog );
		SIP_Session::byeNoSession( pDialog );
        return -1;
    }

	
	//#######################################################################################
	// Get SDP Message

	BTSDPMessage	aSDPMessage;
	int				remotePort;
	unsigned int	connIndex = 0;
	string			strFrom, strTo, strRemote;

	pMessage->parseSDPWith(aSDPMessage);
	
	// Get From / To 
	BTSIPFromHeader* pFrom	= pMessage->FromHeader();
	BTSIPToHeader*	 pTo	= pMessage->ToHeader();
	
	pFrom->URI()->getURIString( strFrom );
	pTo->URI()->getURIString( strTo );

	// Get Remote IP 
	BTSDPConnectionField* pConnField = (BTSDPConnectionField*)aSDPMessage.firstSessionFieldWithType( BTSDPParser::SESSION_CONNECTION, connIndex );
	if( !pConnField )
	{
		APPLOG->Print( DBGLV_ERR, "Not Exist Connection Field" );
		pSession->bye();
		return -1;
	}
	strRemote = pConnField->connectionAddress();
	
	// Get image(fax) port 
	BTSDPMediaField* pMediaField = (BTSDPMediaField*)aSDPMessage.getMediaWithType( "image" );
	if( !pMediaField )
	{
		APPLOG->Print( DBGLV_ERR, "Not Exist Image Media" );
		pSession->bye();
		return -1;
	}
	remotePort = pMediaField->port();

	// Set DevState Info of Session
	//	ToHeader	: G/W
	//	FromHeader	: FSM
	pSession->setSessionInfoInfo( strTo.c_str(), strFrom.c_str(), strRemote.c_str(), remotePort );
	//#######################################################################################

    BTSIPMessage* pSendMessage = pDialog->outboundMessage();
	pSendMessage->clear();	
    /*char* strBody =
    {
        "v=0\r\n"
        "o=IPRONv3Softphone 1246451470 0 IN IP4 100.100.105.45\r\n"
        "s=BTIPRON_VOIP_CALL\r\n"
        "c=IN IP4 100.100.105.45\r\n"
        "t=0 0\r\n"
        "m=audio 30000 RTP/AVP 0 8 18 101\r\n"
        "b=AS:64\r\n"
        "a=rtpmap:0 PCMU/8000\r\n"
        "a=rtpmap:8 PCMA/8000\r\n"
        "a=rtpmap:18 G729/8000\r\n"
        "a=rtpmap:101 telephone-event/8000\r\n"
        "a=fmtp:101 0-15\r\n"
        "a=sendrecv\r\n"
    };
    pSendMessage->setBody(strBody, strlen(strBody), "application/sdp");*/
	pSession->setSDPMessage(pSendMessage, true);
	pDialog->accept(200);	

	return 0;
}

int SIP_Dialog_ModifyStateChanged::_DialogMResponseSent( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ModifyStateChanged::_DialogMCanceled( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
    return 0;
}

int SIP_Dialog_ModifyStateChanged::_DialogMConnected( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
	SIP_Session*	pSession;
	dev_state*		d;
	int				chan;

    /*if (nReason == BTDialog::DIALOG_CHANGED_ACK_RECEIVED)
    {
		APPLOG->Print( DBGLV_ERR, "# Reason Error. state=%s reson=%d\n", SIP_Session::const_SessionState[pSession->state()], nReason);
        pSession->setBye();
		return -1;
	}

    
	if ((pSession->state() == SIP_Session::SESSION_STATE_RETRYING) && !pSession->getSDPMessage(pMessage))
	{
        g_pDialogManager->disconnectCall(pDialog);
		APPLOG->Print( DBGLV_ERR, "# SDP not found. state=%s\n", SIP_Session::const_SessionState[pSession->state()]);
        pSession->setBye();
		return -1;
	}*/
	
	
	// Get Session Info
	pSession = SIP_Manager::inst->FindSession( pDialog );
    if( !pSession )
    {
		APPLOG->Print( DBGLV_ERR, "SIP_Dialog_ModifyStateChanged::_DialogMConnected() : failed to get session. Dialog(0x%08X)", pDialog );
		SIP_Session::byeNoSession( pDialog );
        return -1;
    }

	SIP_CallingTimer::Inst()->Unregister( pSession->channel() );
	
	// Send to FID
	d	 = pSession->devState();
	chan = pSession->channel();

	if( pSession->fstate() < 1 ) {
		APPLOG->Print( DBGLV_RPT, "[CHAN_%03d][TCP:%d:S] Fax_Rx_Start. from=%s, to=%s, transfer=%s, fax_addr=%s:%d.", chan, d->reg-1, d->s.from, d->s.to, d->s.transfer, d->s.remote_ip, d->s.remote_port );
		FaxWrite(d->reg-1, Fax_Rx_Start, chan, sizeof(d->s), (char *)&d->s);
	}
	else {
		APPLOG->Print( DBGLV_RPT, "[CHAN_%03d][TCP:%d:S] Fax_Rx_Restart. from=%s, to=%s, transfer=%s, fax_addr=%s:%d.", chan, d->reg-1, d->s.from, d->s.to, d->s.transfer, d->s.remote_ip, d->s.remote_port );
		FaxWrite(d->reg-1, Fax_Rx_Restart, chan, sizeof(d->s), (char *)&d->s);
	}
	
	pSession->incrementFstate();

	return 0;
}

