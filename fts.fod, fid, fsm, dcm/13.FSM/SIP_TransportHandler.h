#pragma once

class SIP_Manager;


class SIP_TransportHandler
{
	///// Constructor, Destructor ////////////////////////////////
public:
	SIP_TransportHandler();
	~SIP_TransportHandler();

	///// Method /////////////////////////////////////////////////
public:

	///// Callback ///////////////////////////////////////////////
protected:
	static int _ErrorCallback(char* pBuffer, int nLength, char* strSrcIP, int nSrcPort, char* strDestIP, int nDestPort, int nTr);

	///// Field //////////////////////////////////////////////////
protected:


friend class SIP_Manager;
};



