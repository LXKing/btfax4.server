#pragma once

#include "SIP_Data.h"

class SIP_Dialog;

class SIP_Dialog_ModifyStateChanged
{
	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_Dialog_ModifyStateChanged();
	~SIP_Dialog_ModifyStateChanged();

	///// Callback ///////////////////////////////////////////////
protected:
	typedef int (*DIALOG_MODIFY_SERV)(unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static DIALOG_MODIFY_SERV s_ServDIALOGMOD[ BTDialog::NUM_OF_DIALOG_MODIFY_STATE ];

	static int _Callback( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogMIdle( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogMReINVITESent( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogMProceeding( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogMCanceling( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogMSucceeded( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogMFailed( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogMReINVITEReceived( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogMResponseSent( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static int _DialogMCanceled( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );

	static int _DialogMConnected( unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );

	///// Friend /////////////////////////////////////////////////
friend class SIP_Dialog;
};

