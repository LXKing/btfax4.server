#pragma once

class SIP_Dialog;
class SIP_Session;

class SIP_Dialog_StateChanged
{
	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_Dialog_StateChanged();
	~SIP_Dialog_StateChanged();

	///// Callback ///////////////////////////////////////////////
protected:
	typedef int (*DIALOG_SERV)( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static DIALOG_SERV s_ServDIALOG[ BTDialog::NUM_OF_DIALOG_STATE ];

	static int _Callback( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );

	static int _DialogIdle( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogCalling( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogUnauthenticated( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogRedirected( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogProceeding( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogCanceling( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogOffering( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogCanceled( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogAccepted( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogConnected( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogDisconnecting( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogDisconnected( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	static int _DialogTerminated( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage, SIP_Session* pSession );
	
	///// Friend /////////////////////////////////////////////////
friend class SIP_Dialog;
};
