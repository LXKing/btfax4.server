#include "stdafx.h"
#include "SIP_App.h"
#include "SIP_Dialog_StateChanged.h"
#include "SIP_Manager.h"
#include "SIP_CallingTimer.h"
#include "SIP_Utility.h"
#include "TCP_Server.h"
#include "alloc.h"
#include "Utility.h"


const char const_DialogState[BTDialog::NUM_OF_DIALOG_STATE][64] =
{
	"IDLE",
	"CALLING",
	"UNAUTHENTICATED",
	"REDIRECTED",
	"PROCEEDING",
	"CANCELING",
	"OFFERING",
	"CANCELED",
	"ACCEPTED",
	"CONNECTED",
	"DISCONNECTING",
	"DISCONNECTED",
	"TERMINATED"
};

const char const_DialogReason[BTDialog::NUM_OF_DIALOG_REASON][64] =
{
	"Unknown",
	"INVITE Received",
	"RELIABLE PROVISIONAL(1xx) Response Received",
	"PROVISIONAL(1xx) Response Received",
	"RINGING Response Received",
	"SUCCESS(2xx) Response Received",
	"ERROR(3xx-6xx) Response Received",
	"ACK Received",
	"PRACK Received",
	"CANCEL Received",
	"BYE Received",
	"re-INVITE Received",
	"Transc Request Received",
	"Transc Response Received",
	"INVITE Sent",
	"RELIABLE Response Sent",
	"PROVISIONAL(1xx) Response Sent",
	"SUCCESS(2xx) Response Sent",
	"ERROR(3xx-6xx) Response Sent",
	"ACK Sent",
	"CANCEL Sent",
	"BYE Sent",
	"re-INVITE Sent",
	"Transc Request Sent",
	"Transc Response Sent",
	"Transc Terminated",
	"Local Terminated",
	"Forced Terminated"
};

///// Constructor, Destructor ////////////////////////////////
SIP_Dialog_StateChanged::SIP_Dialog_StateChanged()
{
}

SIP_Dialog_StateChanged::~SIP_Dialog_StateChanged()
{
}

///// Callback ///////////////////////////////////////////////
SIP_Dialog_StateChanged::DIALOG_SERV 
SIP_Dialog_StateChanged::s_ServDIALOG[ BTDialog::NUM_OF_DIALOG_STATE ] =
{
	&SIP_Dialog_StateChanged::_DialogIdle,
	&SIP_Dialog_StateChanged::_DialogCalling,
	&SIP_Dialog_StateChanged::_DialogUnauthenticated,
	&SIP_Dialog_StateChanged::_DialogRedirected,
	&SIP_Dialog_StateChanged::_DialogProceeding,
	&SIP_Dialog_StateChanged::_DialogCanceling,
	&SIP_Dialog_StateChanged::_DialogOffering,
	&SIP_Dialog_StateChanged::_DialogCanceled,
	&SIP_Dialog_StateChanged::_DialogAccepted,
	&SIP_Dialog_StateChanged::_DialogConnected,
	&SIP_Dialog_StateChanged::_DialogDisconnecting,
	&SIP_Dialog_StateChanged::_DialogDisconnected,
	&SIP_Dialog_StateChanged::_DialogTerminated
};
int SIP_Dialog_StateChanged::_Callback( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage )
{
	int			 nResult;
	CString		 strSessionInfo;
	SIP_Session* pSession;

	
	pSession = SIP_Manager::inst->FindSession( pDialog );;
	if( pSession ) pSession->getInfo( &strSessionInfo );
	else		   SIP_Session::getEmptyInfo( &strSessionInfo );


	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[Dialog stateChanged] begin. state=%d:%s, reason=%s, Message(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_DialogState[nState], const_DialogReason[nReason], pMessage );

	if( s_ServDIALOG[ nState ] )
		nResult = s_ServDIALOG[ nState ]( nState, nReason, pDialog, pMessage, pSession );

	if( pSession ) 
		pSession->getInfo( &strSessionInfo );
	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[Dialog stateChanged] end.   state=%d:%s, reason=%s, Message(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_DialogState[nState], const_DialogReason[nReason], pMessage );

	return nResult;
}

int SIP_Dialog_StateChanged::_DialogIdle( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
    return 0;
}

int SIP_Dialog_StateChanged::_DialogCalling( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
 	return 0;
}

int SIP_Dialog_StateChanged::_DialogUnauthenticated( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
    return 0;
}

int SIP_Dialog_StateChanged::_DialogRedirected( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{ 
    return 0;
}

int SIP_Dialog_StateChanged::_DialogProceeding( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
	return 0;
}

int SIP_Dialog_StateChanged::_DialogCanceling( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
    return 0;
}

int SIP_Dialog_StateChanged::_DialogOffering( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
	pSession = SIP_ChannelHunter::Inst()->HuntChannel( 'I', pDialog );
	if( pSession == NULL )
	{
		APPLOG->Print( DBGLV_ERR, "_DialogOffering() : Hunt Channel failed" );
		SIP_Session::rejectNoSession( pDialog, 503 );
		return -1;
	}

	CString strMgwIp = pMessage->sourceIP();
	int		mgwPort	 = pMessage->sourcePort();
	if( !pSession->SetMgw( strMgwIp, mgwPort ) )
	{
		APPLOG->Print( DBGLV_ERR, "_DialogOffering() : Fail to set MGW IP. not registered MGW" );
		SIP_Session::rejectNoSession( pDialog, 503 );
		return -1;
	}

	BTSIPURI * rqh;
	const char * p;
	char	cont[256], user[256];

	rqh=pMessage->requestLine()->requestURI();
	p=rqh->userInfo();
	if(p)
		sprintf(user, "%s@", p);
	else
		user[0]=0;
	if(CConfig::ADDRESS_LOCAL_PORT)
		sprintf(cont, "<sip:%s%s:%d>", user, CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT);
	else
		sprintf(cont, "<sip:%s%s>", user, CConfig::ADDRESS_LOCAL_IP);
	pDialog->setLocalContactHeader(cont);
	
	pSession->resetFstate();
	BTSIPCallIDHeader* pCallId = pMessage->CallIDHeader();
	if( pCallId )
		pSession->setCallId( pCallId->callID() );

	APPLOG->Print( DBGLV_RPT, "Call allocated." );

	if( CConfig::SERVICE_AUTO_RINGING ) 
		pSession->proceed( 180 );

	Sleep(200);
	
	pSession->accept( 200 );

	return 0;
}

int SIP_Dialog_StateChanged::_DialogCanceled( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
    return 0;
}

int SIP_Dialog_StateChanged::_DialogAccepted( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
    return 0;
}

int SIP_Dialog_StateChanged::_DialogConnected( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
    if (!pSession)
    {
        APPLOG->Print( DBGLV_ERR, "SIP_Dialog_StateChanged::_DialogConnected() : not session. Dialog(0x%08X)", pDialog );
        return -1;
    }
        
	if( pSession->direction() == 'I' ) // 수신
	{
        Sleep(5);
		pSession->reinviteInUserContext();
	}
	else // 발신
	{   
        // ADD - KIMCG : 20150914
        BTSIPToHeader* pToHeader = NULL;
        BTSIPParam* pParam = NULL;

        pToHeader = pDialog->getToHeader();
        if(pToHeader != NULL)
        {
            pParam = pToHeader->URI()->URIParamList()->paramWithPName("loop_faxno");
            if(pParam != NULL)
            {   
                if(strcmp((const char*)CConfig::SERVICE_LOOP_FAXNO, (const char*)pParam->pvalue()) == 0)
                {
                    APPLOG->Print( DBGLV_INF, "SIP_Dialog_StateChanged::_DialogConnected() : LOOP FAX. fax_no=%s, Dialog(0x%08X)", pParam->pvalue(), pDialog );
                    return 0;
                }
            }
        }
        // ADD - END

        if(!CConfig::ADDRESS_FSM_CROSS )
		{
			Sleep(1);            
            APPLOG->Print( DBGLV_INF, "SIP_Dialog_StateChanged::_DialogConnected() : Address fsm cross :%d. Dialog(0x%08X)", CConfig::ADDRESS_FSM_CROSS,  pDialog );
			pSession->reinviteInUserContext();
		}
	}

	return 0;
}

int SIP_Dialog_StateChanged::_DialogDisconnecting( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
	return 0;
}

int SIP_Dialog_StateChanged::_DialogDisconnected( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
	BTSIPResponseLine* pResponse = NULL;

	if( pSession )
		SIP_CallingTimer::Inst()->Unregister( pSession->channel() );

	if( pMessage )
		pResponse = pMessage->responseLine();

	if( pResponse )
	{
		//// 최종 Status Code SET
		APPLOG->Print( DBGLV_RPT, "#----> [DIALOG_STATE_DISCONNECTED : %d]", pResponse->statusCode() );
		// Session에 이전에 오류가 발생한 status 값이 세팅되어있을 수 있으므로, DialogDisconnected 가 200 OK 일때는 제외.
		if( pResponse->statusCode() != 200 )
		{
			// Session에 RequestTimeout 이 발생하여 Timer Thread가 status Code를 408로 세팅한 경우는 제외.
			if( pSession && pSession->statusCode() != 487 )
				pSession->setStatusCode( pResponse->statusCode() );
		}
	}
	
	return 0;
}

int SIP_Dialog_StateChanged::_DialogTerminated( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession )
{
    if( !pSession )
    {
        APPLOG->Print( DBGLV_RPT, "SIP_Dialog_StateChanged::_DialogTerminated() : failed to get session." );
        return -1;
    }

    if( pSession )
		SIP_CallingTimer::Inst()->Unregister( pSession->channel() );

	if( pDialog == pSession->referDialog() )
	{
/*
		APPLOG->Print( DBGLV_RPT, "## REFER" );

		g_pDialogManager->referEnd( pDialog );
		pSession->setReferDialog( NULL );
*/
	}
	else
	{
		//WriteCdr( pSession->channel() );

		if( pSession->direction() == 'O' )
		{
			dev_state*	d		= pSession->devState();
			int			chan	= pSession->channel();

			if( pSession->fstate() >= 1 || pSession->statusCode() < 0 )
			{
				APPLOG->Print( DBGLV_RPT, "[CHAN_%03d][TCP:%d:S] Fax_Abort.", chan, d->reg-1 );
				FaxWrite(d->reg-1, Fax_Abort, chan, 0, 0);
			}
			else
			{
				fax_cancelled body;
				body.sip_reason = pSession->statusCode();

				APPLOG->Print( DBGLV_RPT, "[CHAN_%03d][TCP:%d:S] Fax_Cancelled. reason=%d", chan, d->reg-1, body.sip_reason );
				FaxWrite( d->reg-1, Fax_Cancelled, chan, sizeof(body), (char*)&body );
			}

			pSession->ResetDevState();
		}
		else 
		{
			dev_state*	d		= pSession->devState();
			int			chan	= pSession->channel();

			APPLOG->Print( DBGLV_RPT, "[CHAN_%03d][TCP:%d:S] Fax_Abort.", chan, d->reg-1 );
			FaxWrite( d->reg-1, Fax_Abort, chan, 0, 0 );
			
			pSession->ResetDevState();
		}

		SIP_Manager::inst->ReleaseChann( pSession );
	}
	
	return 0;
}


int	AuthGetSharedSecret(void * pobj, int objtype, char * realm, char * user, char * passwd)
{
	return	0;
}

int	AuthGetMD5(char * md5in, int len, char * md5out)
{
	return	0;
}