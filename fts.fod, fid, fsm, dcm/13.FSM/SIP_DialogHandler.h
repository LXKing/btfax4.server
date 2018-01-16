#pragma once

class SIP_Manager;

class SIP_Dialog
{
	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_Dialog();
	~SIP_Dialog();

	///// Method /////////////////////////////////////////////////
public:

	///// Callback ///////////////////////////////////////////////
protected:
	static void _DialogStateChanged( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static void _DialogModifyStateChanged( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTSIPMessage* pMessage );
	static void _DialogPrackStateChanged( unsigned int nState, unsigned int nReason, BTDialog * pDialog, BTTransaction * pTransc, BTSIPMessage * pMessage );
	static void _DialogTranscStateChanged( unsigned int nState, unsigned int nReason, BTDialog* pDialog, BTTransaction* pTransc, BTSIPMessage* pMessage );
	static void _DialogSessionTimerEvent( unsigned int nReason, BTDialog* pDialog );
	static void _DialogSessionTimerRefreshAlert(BTDialog* pDialog);
	static void _DialogMessageReceived(BTSIPMessage * pMessage);
	static void _DialogMessageSending(BTSIPMessage * pMessage);

	///// Implementaion //////////////////////////////////////////
protected:

	///// Field //////////////////////////////////////////////////
protected:

	///// Friend /////////////////////////////////////////////////
friend class SIP_Manager;
};


int		AuthGetSharedSecret(void * pobj, int objtype, char * realm, char * user, char * passwd);
int		AuthGetMD5(char * md5in, int len, char * md5out);

void	AppEventCallback(uint event, uint objtype, void * pobj, void * pappdata);