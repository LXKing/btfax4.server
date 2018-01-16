#include "stdafx.h"
#include "SIP_TransportHandler.h"




///// Constructor, Destructor ////////////////////////////////

SIP_TransportHandler::SIP_TransportHandler()
{
}

SIP_TransportHandler::~SIP_TransportHandler()
{
}




///// Method /////////////////////////////////////////////////





///// Callback ///////////////////////////////////////////////
// 수신시 SYNTAX 오류
int SIP_TransportHandler::_ErrorCallback(char* pBuffer, int nLength, char* strSrcIP, int nSrcPort, char* strDestIP, int nDestPort, int nTr)
{
    BTDateTime  currDateTime;
    char        strTime[32] = {0, };
    char        strDir[64] = {0, };
    time_t      currTime;
    currDateTime.getCTime(&currTime);
    sprintf(strDir, "%s:%d <--- %s:%d", strDestIP, nDestPort, strSrcIP, nSrcPort);
    if (currDateTime.getDateTimeString(strTime, sizeof(strTime), "%Y/%m/%d-%H:%M:%S") == false)
        sprintf(strTime, "0000-00-00 00:00:00");

	APPLOG->Print( DBGLV_ERR, "\n\n%s\n %s %s %s %dbytes..\n%s\n\n%s\n",
            "_ErrorCallback==================================================================================",
			nTr ? "TCP":"UDP",
            strTime, strDir, nLength,
            "==================================================================================",
            pBuffer);

	return	0;
}



///// Implementaion //////////////////////////////////////////




///// Field //////////////////////////////////////////////////




