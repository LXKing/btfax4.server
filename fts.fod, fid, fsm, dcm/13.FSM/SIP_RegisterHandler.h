#pragma once


class SIP_Manager;


class SIP_RegisterHandler
{
	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_RegisterHandler();
	~SIP_RegisterHandler();

	///// Method /////////////////////////////////////////////////
public:

	///// Callback ///////////////////////////////////////////////
protected:
	typedef int (*REGISTER_SERV)(unsigned int nReason, BTRegister* pRegister);
	static REGISTER_SERV s_ServREGISTER[BTRegister::NUM_OF_REGISTER_STATE];

	static void _RegisterStateChanged( unsigned int nState, unsigned int nReason, BTRegister* pRegister );
	static int  _RegiIdle( unsigned int nReason, BTRegister* pRegister );
	static int  _RegiRegistering( unsigned int nReason, BTRegister* pRegister );
	static int  _RegiUnregistering( unsigned int nReason, BTRegister* pRegister );
	static int  _RegiUnauthenticated( unsigned int nReason, BTRegister* pRegister );
	static int  _RegiRedirected( unsigned int nReason, BTRegister* pRegister );
	static int  _RegiRegisterReceived( unsigned int nReason, BTRegister* pRegister );
	static int  _RegiRegistered( unsigned int nReason, BTRegister* pRegister );
	static int  _RegiFailed( unsigned int nReason, BTRegister* pRegister );
	static int  _RegiTerminated( unsigned int nReason, BTRegister* pRegister );

	///// Implementaion //////////////////////////////////////////
protected:

	///// Field //////////////////////////////////////////////////
protected:
	
	///// Friend /////////////////////////////////////////////////
friend class SIP_Manager;
};



