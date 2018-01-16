#include "stdafx.h"
#include "SIP_Utility.h"



void printSIPMessage( BTSIPMessage* pMessage, bool bReceived, bool bDisplayLHOPE )
{
	if( !bDisplayLHOPE )
		return;
    //if( !g_pConfig->nTracMode ) return;
    if( !pMessage ) return;

    int     nBufferLength   = 0;
    int     nReturnLength   = 0;
    char*   pBuffer = NULL;
    int     nResult = 0;
    int     nRange  = 0;

    nRange = g_pResourceMgr->getMemoryManager()->getPage( &pBuffer );
    if( nRange==BTPagePool::rtNOT_AVAILABLE )
    {
        char aBuffer[4096];
        nBufferLength = sizeof( aBuffer );
        nResult = pMessage->getSipMessageWithBuffer( aBuffer,nBufferLength,nReturnLength,bReceived );
        pBuffer = &aBuffer[0];
    }
    else
    {
        nBufferLength = g_pResourceMgr->getMemoryManager()->pageSize();
        nResult = pMessage->getSipMessageWithBuffer( pBuffer,nBufferLength,nReturnLength,bReceived );
    }
    if( nResult != rtSUCCESS )
    {
		APPLOG->Print( DBGLV_INF, "[%s] getSipMessageWithBuffer Error. result=%d\n", __FUNCTION__, nResult );
        g_pResourceMgr->getMemoryManager()->releasePage( nRange );
        return;
    }

	BTDateTime	currDateTime;
	char		strTime[32] = {0, };
	char		strDir[64] = {0, };
	time_t		currTime;
	currDateTime.getCTime( &currTime );
	if( !currDateTime.getDateTimeString(strTime, sizeof(strTime), "%Y/%m/%d-%H:%M:%S") )
		sprintf( strTime, "0000-00-00 00:00:00" );
	if( bReceived )
		sprintf( strDir, "%s:%d <--- %s:%d", 
			pMessage->destIP(), pMessage->destPort(), pMessage->sourceIP(), pMessage->sourcePort() );
	else
		sprintf( strDir, "%s:%d ---> %s:%d", 
			pMessage->sourceIP(), pMessage->sourcePort(), pMessage->destIP(), pMessage->destPort() );

	APPLOG->Print( DBGLV_INF, "\n\n%s\n %s %s %s %dbytes..\n%s\n\n%s",
    		"===============================================================================",
    		(pMessage->transport()!=Transport_TCP)?"UDP":"TCP", strTime, strDir, nReturnLength,
    		"===============================================================================",
    		pBuffer );
	
    g_pResourceMgr->getMemoryManager()->releasePage( nRange );
}


void printSDPMessage( BTSDPMessage* pSDPMessage, bool bDisplayLHOPE )
{
	if( !bDisplayLHOPE )
		return;

    char strBuffer[1024] = {0, };
    char *pStrBuffer = &strBuffer[0];
    int nBufferLength = sizeof( strBuffer );
    int nReturnLength = 0;

    pSDPMessage->getSDPWithBuffer( pStrBuffer, nBufferLength, nReturnLength );
    APPLOG->Print( DBGLV_ERR, "SDP message %dbytes = \n%s", nReturnLength, pStrBuffer );
}


/** @brief	SIP 헤더 설정 */
// Location : "Name" <sip:User@IP:Port>
void setLocation( CString& p_strLocation, const char* p_szIp, int p_nPort, const char* p_szUser, const char* p_szName )
{
	if( p_szName && p_szName[0] )
        p_strLocation.Format( "\"%s\" <sip:%s@%s:%d>", p_szName, p_szUser, p_szIp, p_nPort );
	else    
		p_strLocation.Format( "<sip:%s@%s:%d>", p_szUser, p_szIp, p_nPort );
}

void setUri( CString& p_strUri, const char* p_szIp, int p_nPort, const char* p_szUser )
{
	p_strUri.Format( "sip:%s@%s:%d", p_szUser, p_szIp, p_nPort );
}


void setFrom( char* strFrom, const char* strURI, const char* strName )
{
    if (strName && strName[0])
    {
        sprintf(strFrom, "\"%s\" <%s>", strName, strURI);
    }
    else
    {
        strcpy(strFrom, strURI);
    }

    APPLOG->Print( DBGLV_INF, "From : %s", strFrom );
}

void setTo( char* strTo, const char* strURI, const char* strName )
{
    if (strName && strName[0])
    {
        sprintf(strTo, "\"%s\" <%s>", strName, strURI);
    }
    else
    {
        strcpy(strTo, strURI);
    }

    APPLOG->Print( DBGLV_INF, "To : %s", strTo );
}

void setContact( BTSIPMessage* pMessage, const char* strURI, int nPort )
{
    if (pMessage)
    {
        char    strTemp[64] = {0, };

        nPort ? sprintf(strTemp, "<%s:%d>", strURI, nPort) : sprintf(strTemp, "<%s>", strURI);

        pMessage->parseContactHeaderWithString(strTemp);
    }
}

void setTimestamp( BTSIPMessage* pMessage, int nTimestamp )
{
    if (nTimestamp && pMessage)
    {
        APPLOG->Print( DBGLV_INF, "Timestamp header : %d", nTimestamp );
        pMessage->parseOtherHeaderWithInt("Timestamp", nTimestamp);
    }
}

void setExpires( BTSIPMessage* pMessage, int nExpires )
{
    if (nExpires && pMessage)
    {
        APPLOG->Print( DBGLV_INF, "Expires header : %d", nExpires );
        pMessage->parseOtherHeaderWithInt("Expires", nExpires);
    }
}

void setSupported( BTSIPMessage* pMessage, const char* strSupported )
{
    if (strSupported && strlen(strSupported) && pMessage)
    {
        APPLOG->Print( DBGLV_INF, "Supported header : %s", strSupported );
        pMessage->parseOtherHeaderWithString( "Supported", (char*)strSupported );
    }
}

void setAllowed( BTSIPMessage* pMessage )
{
    if (pMessage)
    {
        char    strTemp[1024] = {0, };
        int     n   = 0;
        int     nloop;

        n = sprintf(strTemp, "%s", BTSIPMessage::const_SIPMethod[SIPMethod_INVITE]);
        for( nloop = SIPMethod_ACK; nloop < SIPMethod_OTHER; nloop++ )
            n += sprintf( strTemp+n, ", %s", BTSIPMessage::const_SIPMethod[nloop] );
        for( nloop = CConfig::SERVICE_SERVE_METHOD.GetCount()-1; nloop >= 0; nloop-- )
            n += sprintf( strTemp+n, ", %s", (LPCSTR)CConfig::SERVICE_SERVE_METHOD[nloop] );

        APPLOG->Print( DBGLV_INF, "Allowed Headers : %s", strTemp );
        pMessage->parseOtherHeaderWithString("Allowed", strTemp);
    }
}

bool checkMethod( BTSIPMessage* pMessage )
{
    APPLOG->Print( DBGLV_INF, "[%s] begin ---->", __FUNCTION__ );
    if (!pMessage || !pMessage->isRequest())
    {
        APPLOG->Print( DBGLV_ERR, "# pMessage is %s.", (!pMessage ? "Null" : "Not request") );
        return false;
    }

    BTSIPRequestLine* pReqLine 	= pMessage->requestLine();
    BTSIPCSeqHeader*  pCSeq 	= pMessage->CSeqHeader();

    APPLOG->Print( DBGLV_INF, "Request's Method type = %d.", pReqLine->requestMethodType() );
    if( CConfig::SERVICE_CHECK_METHOD && pReqLine->requestMethodType() == SIPMethod_OTHER )
    {
        bool bResult = false;
        for( int nloop = 0 ; nloop < CConfig::SERVICE_SERVE_METHOD.GetCount() ; nloop++ )
        {
            if( !strcmp( CConfig::SERVICE_SERVE_METHOD[nloop], pCSeq->CSeqMethod() ) )
            {
                bResult = true;
                break;
            }
        }
        if (bResult==false)
        {
            APPLOG->Print( DBGLV_INF, "%s is unsupported Method type.", pCSeq->CSeqMethod() );
            return false;
        }
    }

    APPLOG->Print( DBGLV_INF, "pMessage(0x%08X)'s Method : %s", pMessage, pCSeq->CSeqMethod() );
    APPLOG->Print( DBGLV_INF, "[%s] <---- end.", __FUNCTION__ );

    return true;
}



