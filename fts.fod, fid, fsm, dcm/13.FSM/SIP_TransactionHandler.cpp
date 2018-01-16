#include "stdafx.h"
#include "SIP_TransactionHandler.h"
#include "SIP_Mgws.h"
#include "SIP_Data.h"
#include "SIP_Utility.h"


const char const_TransactionState[BTTransaction::NUM_OF_TRANSACTION_STATE][64] =
{
	"IDLE",
	//INVITE Client Transaction StateS
	"INVITE Client CALLING",
	"INVITE Client PROCEEDING",
	"INVITE Client CANCELLING",
	"INVITE Client SUCCESS RESPONSE RECEIVED",
	"INVITE Client FINAL RESPONSE RECEIVED",
	//INVITE Server Transaction StateS
	"INVITE Server INVITE RECEIVED",
	"INVITE Server RELIABLE PROVISIONAL RESPONSE SENT",
	"INVITE Server PRACK COMPLETED",
	"INVITE Server COMPLETED",
	"INVITE Server CONFIRMED",
	//GENERAL Client Transaction StateS
	"GENERAL Client TRYING",
	"GENERAL Client PROCEEDING",
	"GENERAL Client FINAL RESPONSE RECEIVED",
	//GENERAL Server Transaction StateS
	"GENERAL Server REQUEST RECEIVED",
	"GENERAL Server COMPLETED",
	"TERMINATED",
	"BACK TO POOL"
};

const char const_TransactionReason[BTTransaction::NUM_OF_TRANSACTION_REASON][64] =
{
	"Unknown",
	"INVITE Received",
	"CANCEL Received",
	"ACK Received",
	"PRACK Received",
	"GENERAL Request Received",
	"100TRYING Received",
	"Reliable Provisional Response Received",
	"Provisional Response Received",
	"Received Response for CANCEL",
	"Success Response Received",
	"Error Response Received",
	"Final Response Received",
	"INVITE Sent",
	"CANCEL Sent",
	"ACK Sent",
	"GENERAL Request Sent",
	"100TRYING Sent",
	"Reliable Provisional Response Sent",
	"Provisional Response Sent",
	"Success Response Sent",
	"Error Response Sent",
	"Final Response Sent",
	"T1 Timeout",
	"T2 Timeout",
	"T4 Timeout",
	"TimerA Timeout",
	"TimerB Timeout",
	"TimerD Timeout",
	"TimerE Timeout",
	"TimerF Timeout",
	"TimerG Timeout",
	"TimerH Timeout",
	"TimerI Timeout",
	"TimerJ Timeout",
	"TimerK Timeout",
	"TimerR Timeout",
	"TimerP Timeout",
	"Pool Not Available",
	"Forced Terminated"
};

///// Constructor, Destructor ////////////////////////////////

SIP_TransactionHandler::SIP_TransactionHandler()
{
}

SIP_TransactionHandler::~SIP_TransactionHandler()
{
}




///// Method /////////////////////////////////////////////////




///// Callback ///////////////////////////////////////////////

SIP_TransactionHandler::TRANSC_SERV SIP_TransactionHandler::s_ServTRANSC[ BTTransaction::NUM_OF_TRANSACTION_STATE ] =
{
    &SIP_TransactionHandler::_TSC_TranscIdle,
    &SIP_TransactionHandler::_TSC_TranscUnexpected,
    &SIP_TransactionHandler::_TSC_TranscUnexpected,
    &SIP_TransactionHandler::_TSC_TranscUnexpected,
    &SIP_TransactionHandler::_TSC_TranscUnexpected,
    &SIP_TransactionHandler::_TSC_TranscUnexpected,       
    &SIP_TransactionHandler::_TSC_TranscUnexpected,
    &SIP_TransactionHandler::_TSC_TranscUnexpected,
    &SIP_TransactionHandler::_TSC_TranscUnexpected,
    &SIP_TransactionHandler::_TSC_TranscUnexpected,
    &SIP_TransactionHandler::_TSC_TranscUnexpected,
    &SIP_TransactionHandler::_TSC_TranscGCRequestSent,
    &SIP_TransactionHandler::_TSC_TranscGCProceeding,
    &SIP_TransactionHandler::_TSC_TranscGCCompleted,
    &SIP_TransactionHandler::_TSC_TranscGSRequestReceived,
    &SIP_TransactionHandler::_TSC_TranscGSCompleted,
    &SIP_TransactionHandler::_TSC_TranscTerminated,
	0,
};

void SIP_TransactionHandler::_TranscStateChanged( unsigned int nState, unsigned int nReason, BTTransaction* pTransc, BTSIPMessage* pMessage )
{
	CString	strSessionInfo;

	SIP_Session::getEmptyInfo( &strSessionInfo );

	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[General transcStateChanged] begin. state=%d:%s, reason=%s, Transc(0x%08X), Message(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_TransactionState[nState], const_TransactionReason[nReason], pTransc, pMessage );

	if( s_ServTRANSC[ nState ] )
		s_ServTRANSC[ nState ]( nReason, pTransc, pMessage );

	APPLOG->Print( DBGLV_INF, 
				   "%s CALLBACK[General transcStateChanged] end.   state=%d:%s, reason=%s, Transc(0x%08X), Message(0x%08X)", 
				   (LPCSTR)strSessionInfo, nState, const_TransactionState[nState], const_TransactionReason[nReason], pTransc, pMessage );
}

void SIP_TransactionHandler::_TranscMessageReceived( BTSIPMessage* pMessage )
{
	if( pMessage && pMessage->CSeqHeader() )
	{
		//if( strcmp(pMessage->CSeqHeader()->CSeqMethod(), "ACK") )
			printSIPMessage( pMessage, true, true );
	}

	if( pMessage )
	{	
		CString strIp = pMessage->sourceIP();
		int		nPort = pMessage->sourcePort();

		SIP_Mgws::Inst()->UpdateRecvTime( strIp, nPort );
	}
}

void SIP_TransactionHandler::_TranscMessageSending( BTSIPMessage* pMessage )
{
	if( pMessage && pMessage->CSeqHeader() )
	{
		//if( strcmp(pMessage->CSeqHeader()->CSeqMethod(), "ACK") )
			printSIPMessage( pMessage, false, true );
	}
}

int SIP_TransactionHandler::_TranscAuthCredentialFound( void * pobj, int objtype, bool support )
{
	return	-1;
}

int SIP_TransactionHandler::_TranscAuthComplete( void * pobj, int objtype, bool support )
{
	return	0;
}

int SIP_TransactionHandler::_TSC_TranscIdle(unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}

int SIP_TransactionHandler::_TSC_TranscUnexpected(unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
	return 0;
}

int SIP_TransactionHandler::_TSC_TranscGCRequestSent(unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}

int SIP_TransactionHandler::_TSC_TranscGCProceeding(unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}

int SIP_TransactionHandler::_TSC_TranscGCCompleted(unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
	CString strIp = pMessage->sourceIP();
	int		nPort = pMessage->sourcePort();

	SIP_Mgws::Inst()->UpdateRecvTime( strIp, nPort );
	
	return 0;
}

int SIP_TransactionHandler::_TSC_TranscGSRequestReceived(unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
/*
    if (nReason != BTTransaction::TRANSACTION_CHANGED_ACK_RECEIVED)
    {
		g_pMemAllocMutex->lock();
        BTSIPMessage *pNewMessage = new BTSIPMessage;
		g_pMemAllocMutex->unlock();

        pNewMessage->makeResponseWithRequestMessage(200, 2, 0, pMessage);
        g_pTransactionManager->sendResponse(pTransaction, pNewMessage);
        delete pNewMessage;
    }
*/
	BTSIPRequestLine * rqline=pMessage->requestLine();
	SIPMethod_t method=rqline->requestMethodType();
	BTSIPCSeqHeader * cseq=pMessage->CSeqHeader();
	const char * mstr=cseq->CSeqMethod();
	int     resp=SIPResponse_MethodNotAllowed;

	APPLOG->Print( DBGLV_INF, "method=%d_%s", method, mstr);

	if(method==SIPMethod_OPTIONS)
		resp=200;

	BTSIPMessage rmsg;
	rmsg.makeResponseWithReqMsg(resp, 2, 0, pMessage);
	pTransaction->sendResponse(&rmsg);

	return 0;
}

int SIP_TransactionHandler::_TSC_TranscGSCompleted(unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}

int SIP_TransactionHandler::_TSC_TranscTerminated(unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage)
{
    return 0;
}



///// Implementaion //////////////////////////////////////////





///// Field //////////////////////////////////////////////////




