#pragma once


class SIP_Dialog;

class SIP_Dialog_ReferStateChanged
{
	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_Dialog_ReferStateChanged();
	~SIP_Dialog_ReferStateChanged();

	///// Callback ///////////////////////////////////////////////
protected:
	typedef int (*DIALOG_REFER_SERV)( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static DIALOG_REFER_SERV s_ServDIALOGREF[ BTDialog::NUM_OF_DIALOG_REFER_STATE ];

	static int _Callback( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogRIdle( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogRSent( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogRCalling( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogRUnauthenticated( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogRRedirected( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogRReceived( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );

	///// Friend /////////////////////////////////////////////////
friend class SIP_Dialog;
};

