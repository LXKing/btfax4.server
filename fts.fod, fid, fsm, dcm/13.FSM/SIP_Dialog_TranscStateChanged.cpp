#include "stdafx.h"
#include "SIP_App.h"
#include "SIP_Dialog_TranscStateChanged.h"
#include "SIP_Manager.h"
#include "SIP_Data.h"


const char const_DialogTranscState[BTDialog::NUM_OF_DIALOG_TRANSC_STATE][64] =
{
	"IDLE",
	"REQUEST SENT",
	"PROCEEDING",
	"RESPONSE RECEIVED",
	"REQUEST RECEIVED",
	"RESPONSE SENT",
	"TERMINATED"
};


///// Constructor, Destructor ////////////////////////////////
SIP_Dialog_TranscStateChanged::SIP_Dialog_TranscStateChanged()
{
}

SIP_Dialog_TranscStateChanged::~SIP_Dialog_TranscStateChanged()
{
}

///// Callback ///////////////////////////////////////////////
SIP_Dialog_TranscStateChanged::DIALOG_TRANSC_SERV 
SIP_Dialog_TranscStateChanged::s_ServDIALOGTR[ BTDialog::NUM_OF_DIALOG_TRANSC_STATE ] =
{
	&SIP_Dialog_TranscStateChanged::_DialogTIdle,
	&SIP_Dialog_TranscStateChanged::_DialogTRequestSent,
	&SIP_Dialog_TranscStateChanged::_DialogTProceeding,
	&SIP_Dialog_TranscStateChanged::_DialogTResponseReceived,
	&SIP_Dialog_TranscStateChanged::_DialogTRequestReceived,
	&SIP_Dialog_TranscStateChanged::_DialogTResponseSent,
	&SIP_Dialog_TranscStateChanged::_DialogTTerminated
};

int SIP_Dialog_TranscStateChanged::_Callback(unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
	int			 nResult;
	CString		 strSessionInfo;
	SIP_Session* pSession;

	
	pSession = SIP_Manager::inst->FindSession( pDialog );;
	if( pSession ) pSession->getInfo( &strSessionInfo );
	else		   SIP_Session::getEmptyInfo( &strSessionInfo );


	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[Dialog transcStateChanged] begin. state=%d:%s, reason=%s, Message(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_DialogTranscState[nState], const_DialogReason[nReason], pMessage );
	
	nResult = s_ServDIALOGTR[ nState ]( nReason, pDialog, pTransaction, pMessage );

	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[Dialog transcStateChanged] end.   state=%d:%s, reason=%s, Message(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_DialogTranscState[nState], const_DialogReason[nReason], pMessage );

	return nResult;
}

int SIP_Dialog_TranscStateChanged::_DialogTIdle(unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}

int SIP_Dialog_TranscStateChanged::_DialogTRequestSent(unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}

int SIP_Dialog_TranscStateChanged::_DialogTProceeding(unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}

int SIP_Dialog_TranscStateChanged::_DialogTResponseReceived(unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}


int SIP_Dialog_TranscStateChanged::_DialogTRequestReceived(unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
	BTSIPRequestLine * rqline=pMessage->requestLine();
	SIPMethod_t method=rqline->requestMethodType();
	BTSIPCSeqHeader * cseq=pMessage->CSeqHeader();
	const char * mstr=cseq->CSeqMethod();
	int     code=SIPResponse_MethodNotAllowed;

	if(method==SIPMethod_UPDATE || method==SIPMethod_OPTIONS)
		code=200;

	pDialog->outboundMessage()->clear();
	pDialog->sendResponse(code, pTransaction);

	return 0;
}

int SIP_Dialog_TranscStateChanged::_DialogTResponseSent(unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}

int SIP_Dialog_TranscStateChanged::_DialogTTerminated(unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}

