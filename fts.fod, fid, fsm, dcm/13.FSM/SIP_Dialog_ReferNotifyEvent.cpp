#include "stdafx.h"
#include "SIP_Dialog_ReferNotifyEvent.h"

#include "SIP_Data.h"
#include "SIP_Manager.h"



///// Constructor, Destructor ////////////////////////////////
SIP_Dialog_ReferNotifyEvent::SIP_Dialog_ReferNotifyEvent()
{
}

SIP_Dialog_ReferNotifyEvent::~SIP_Dialog_ReferNotifyEvent()
{
}

///// Callback ///////////////////////////////////////////////
SIP_Dialog_ReferNotifyEvent::DIALOG_REFER_NOTIFY_SERV 
SIP_Dialog_ReferNotifyEvent::s_ServDIALOGREFN[ BTDialog::NUM_OF_DIALOG_REFER_NOTIFY_EVENT ] =
{
	&SIP_Dialog_ReferNotifyEvent::_DialogRNReady,
	&SIP_Dialog_ReferNotifyEvent::_DialogRNSent,
	&SIP_Dialog_ReferNotifyEvent::_DialogRNUnauthenticated,
	&SIP_Dialog_ReferNotifyEvent::_DialogRNRedirected,
	&SIP_Dialog_ReferNotifyEvent::_DialogRNResponseReceived,
	&SIP_Dialog_ReferNotifyEvent::_DialogRNReceived
};

int SIP_Dialog_ReferNotifyEvent::_Callback( unsigned int nEvent, unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog )
{
	int			 nResult;
	CString		 strSessionInfo;
	SIP_Session* pSession;

	
	pSession = SIP_Manager::inst->FindSession( pDialog );;
	if( pSession ) pSession->getInfo( &strSessionInfo );
	else		   SIP_Session::getEmptyInfo( &strSessionInfo );


	APPLOG->Print( DBGLV_INF, 
					"%s CALLBACK[Dialog referNotifyEvent] begin. event=%d:%s, reason=%s, seqStep=%d", 
					(LPCSTR)strSessionInfo, nEvent, BTDialog::const_DialogReferNotifyEvent[nEvent], BTDialog::const_DialogReason[nReason], nCSeqStep );
		
	nResult = s_ServDIALOGREFN[nEvent](nReason, nRespCode, nCSeqStep, pDialog);

	APPLOG->Print( DBGLV_INF, 
					"%s CALLBACK[Dialog referNotifyEvent] end.   event=%d:%s, reason=%s, seqStep=%d", 
					(LPCSTR)strSessionInfo, nEvent, BTDialog::const_DialogReferNotifyEvent[nEvent], BTDialog::const_DialogReason[nReason], nCSeqStep );

	return nResult;
}

int SIP_Dialog_ReferNotifyEvent::_DialogRNReady( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog )
{
    return 0;
}

int SIP_Dialog_ReferNotifyEvent::_DialogRNSent( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog )
{
    return 0;
}

int SIP_Dialog_ReferNotifyEvent::_DialogRNUnauthenticated( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog )
{
    return 0;
}

int SIP_Dialog_ReferNotifyEvent::_DialogRNRedirected( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog )
{
    return 0;
}

int SIP_Dialog_ReferNotifyEvent::_DialogRNResponseReceived( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog )
{
    return 0;
}

int SIP_Dialog_ReferNotifyEvent::_DialogRNReceived( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog )
{
    SIP_Session* pSession = SIP_Manager::inst->FindSession( pDialog );
	if (!pSession)
	{
		APPLOG->Print( DBGLV_ERR, "# failed to get session." );
		g_pDialogManager->disconnectCall(pDialog);
        return -1;
    }
	
	if (nRespCode > 199) 
	{
		APPLOG->Print( DBGLV_ERR, "Call-Transfer completed successfully." );
		g_pDialogManager->disconnectCall(pDialog);
		pSession->setBye();
	}

	return 0;
}


