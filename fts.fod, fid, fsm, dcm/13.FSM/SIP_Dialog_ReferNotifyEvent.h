#pragma once


class SIP_Dialog;

class SIP_Dialog_ReferNotifyEvent
{
	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_Dialog_ReferNotifyEvent();
	~SIP_Dialog_ReferNotifyEvent();

	///// Callback ///////////////////////////////////////////////
protected:
	typedef int (*DIALOG_REFER_NOTIFY_SERV)(unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog);
	static DIALOG_REFER_NOTIFY_SERV s_ServDIALOGREFN[ BTDialog::NUM_OF_DIALOG_REFER_NOTIFY_EVENT ];

	static int _Callback( unsigned int nEvent, unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog );
	static int _DialogRNReady( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog );
	static int _DialogRNSent( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog );
	static int _DialogRNUnauthenticated( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog );
	static int _DialogRNRedirected( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog );
	static int _DialogRNResponseReceived( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog );
	static int _DialogRNReceived( unsigned int nReason, unsigned int nRespCode, unsigned int nCSeqStep, BTDialog* pDialog );

	///// Friend /////////////////////////////////////////////////
friend class SIP_Dialog;
};

