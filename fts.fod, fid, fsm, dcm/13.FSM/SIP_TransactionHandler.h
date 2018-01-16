#pragma once


class SIP_Manager;


class SIP_TransactionHandler
{
	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_TransactionHandler();
	~SIP_TransactionHandler();

	///// Method /////////////////////////////////////////////////
public:

	///// Callback ///////////////////////////////////////////////
protected:
	typedef int (*TRANSC_SERV)(unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage);
	static TRANSC_SERV s_ServTRANSC[BTTransaction::NUM_OF_TRANSACTION_STATE];

	static void _TranscStateChanged( unsigned int nState, unsigned int nReason, BTTransaction* pTransc, BTSIPMessage* pMessage );
	static void _TranscMessageReceived( BTSIPMessage* pMessage );
	static void _TranscMessageSending( BTSIPMessage* pMessage );
	static int  _TranscAuthCredentialFound( void * pobj, int objtype, bool support );
	static int  _TranscAuthComplete( void * pobj, int objtype, bool authresult );

	static int  _TSC_TranscIdle( unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int  _TSC_TranscUnexpected( unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int  _TSC_TranscGCRequestSent( unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int  _TSC_TranscGCProceeding( unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int  _TSC_TranscGCCompleted( unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int  _TSC_TranscGSRequestReceived( unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int  _TSC_TranscGSCompleted( unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	static int  _TSC_TranscTerminated( unsigned int nReason, BTTransaction* pTransaction, BTSIPMessage* pMessage );
	

	///// Implementaion //////////////////////////////////////////
protected:

	///// Field //////////////////////////////////////////////////
protected:


friend class SIP_Manager;
};



