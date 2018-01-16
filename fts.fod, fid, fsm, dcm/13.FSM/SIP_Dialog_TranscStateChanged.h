#pragma once

class SIP_Dialog;

class SIP_Dialog_TranscStateChanged
{
	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_Dialog_TranscStateChanged();
	~SIP_Dialog_TranscStateChanged();

	///// Callback ///////////////////////////////////////////////
protected:
	typedef int (*DIALOG_TRANSC_SERV)(unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage);
	static DIALOG_TRANSC_SERV s_ServDIALOGTR[ BTDialog::NUM_OF_DIALOG_TRANSC_STATE ];

	static int _Callback( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int _DialogTIdle( unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int _DialogTRequestSent( unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int _DialogTProceeding( unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int _DialogTResponseReceived( unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int _DialogTRequestReceived( unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int _DialogTResponseSent( unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int _DialogTTerminated( unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransaction, BTSIPMessage* pMessage );

	///// Friend /////////////////////////////////////////////////
friend class SIP_Dialog;
};

