#include "stdafx.h"
#include "SIP_App.h"
#include "Config.h"
#include "SIP_Data.h"
#include "SIP_Manager.h"
#include "SIP_MGWs.h"
#include "SIP_Utility.h"

const char SIP_User::const_UserState[ SIP_User::NUM_OF_USER_STATE ][64] = 
{
	"IDLE",
	"TRYING",
	"REGISTERED",
	"RETRYING",
	"REJECT",
};

const char SIP_Session::const_SessionState[ SIP_Session::NUM_OF_SESSION_STATE ][64] = 
{
	"IDLE",
	"TRYING",
	"ESTABLISHED",
	"RETRYING",
	"REJECT",
	"TRANSFER",
	"BYE",
};

bool SIP_Session::HuntMgw( MGW_INFO* p_pMgwInfo )
{
	{CSingleLock Locker( &m_Lock, TRUE );

		m_MgwInfo.Clear();
		if( !SIP_Mgws::Inst()->HuntMgw( &m_MgwInfo.bPrimary, &m_MgwInfo.strIp, &m_MgwInfo.nPort ) )
			return false;

		m_MgwInfo.bAllocated = true;

		*p_pMgwInfo = m_MgwInfo;
	}

	return true;
}

bool SIP_Session::SetMgw( const char* p_szIp, int p_port )
{
	bool bPrimary;

	{CSingleLock Locker( &m_Lock, TRUE );

		m_MgwInfo.Clear();

        // ADD - KIMCG : 20150910 내부 루프 시험처리
        if(strcmp(p_szIp, CConfig::ADDRESS_LOCAL_IP) == 0 &&
            p_port == CConfig::ADDRESS_LOCAL_PORT)
        {
            m_MgwInfo.strIp		= p_szIp;
		    m_MgwInfo.nPort		= p_port;
		    m_MgwInfo.bPrimary	= bPrimary;
		    m_MgwInfo.bAllocated = true;
            return true;            
        }
        // ADD - END
        
		if( !SIP_Mgws::Inst()->GetMgw( p_szIp, p_port, &bPrimary ) )
			return false;

		m_MgwInfo.strIp		= p_szIp;
		m_MgwInfo.nPort		= p_port;
		m_MgwInfo.bPrimary	= bPrimary;
		m_MgwInfo.bAllocated = true;
	}

	return true;
}

bool SIP_Session::GetMgw( MGW_INFO* p_pMgwInfo )
{
	{CSingleLock Locker( &m_Lock, TRUE );

		if( !m_MgwInfo.bAllocated )
			return false;

		*p_pMgwInfo = m_MgwInfo;
	}

	return true;
}

void SIP_Session::ClearMgw()
{
	{CSingleLock Locker( &m_Lock, TRUE );

		m_MgwInfo.Clear();	
	}
}


/** @brief	SDP 메시지 저장 */
bool SIP_Session::getSDPMessage(BTSIPMessage* pSIPMessage)
{
	BTSDPMessage aSDPMessage;

	bool bResult = pSIPMessage->parseSDPWith(aSDPMessage);
	//if( bResult )
		//printSDPMessage(&aSDPMessage);

	return bResult;
}

bool SIP_Session::setSDPMessage(BTSIPMessage* pMessage, bool bFax)
{
    char    strBody[1024] 		= {0, };
    char    strTempLine[256] 	= {0, };
    unsigned int    nMediaIndex	= 0;
	int 	nBody = 0;

    BTSDPMessage* pSDPMessage = new BTSDPMessage();

	pSDPMessage->setSDPVersionField(0);
	pSDPMessage->setSDPOriginField( "Bridgetec", time(NULL), time(NULL), "IN", "IP4", (char*)(LPCSTR)CConfig::ADDRESS_LOCAL_IP );
	//pSDPMessage->setSDPSessionNameField(g_pConfig->getTestName());
	pSDPMessage->setSDPSessionNameField( "lhope_test_name" );
	pSDPMessage->setSDPConnectionField( "IN", "IP4", (char*)(LPCSTR)CConfig::ADDRESS_LOCAL_IP );
	pSDPMessage->setSDPTimeField(0, 0);
	if( !bFax )
	{
		//pSDPMessage->setSDPMediaField("audio", CConfig::ADDRESS_RTP_PORT_BASE/10+2*m_chan, 1, "RTP/AVP", "0 8 101", nMediaIndex);
		pSDPMessage->setSDPMediaField("audio", CConfig::ADDRESS_RTP_PORT_BASE+2*m_chan+1, 1, "RTP/AVP", "0 8 101", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("rtpmap", "0 PCMU/8000", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("rtpmap", "8 PCMA/8000", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("rtpmap", "18 G729/8000", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("rtpmap", "101 telephone-event/8000", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("fmp", "101 0-15", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("sendrecv", NULL, nMediaIndex);
	}

	//if ((strstr(strlwr("lhope_test_name"), "fax")) &&
 //       (m_nState == SIP_Session::SESSION_STATE_RETRYING))    // for fax test.
	else
	{
		//pSDPMessage->setSDPMediaField("image", CConfig::ADDRESS_RTP_PORT_BASE+2*m_chan, 1, "udptl", "t38", nMediaIndex);
		pSDPMessage->setSDPMediaField("image", CConfig::ADDRESS_RTP_PORT_BASE+2*m_chan, 1, "udptl", "t38", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("T38FaxVersion", "0", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("T38MaxBitRate", "14400", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("T38FaxFillBitRemoval", "0", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("T38FaxTranscodingMMR", "0", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("T38FaxTranscodingJBIG", "0", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("T38FaxRateManagement", "transferredTCF", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("T38FaxMaxBuffer", "400", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("T38FaxMaxDatagram", "400", nMediaIndex);
		pSDPMessage->setSDPMediaAttributeField("T38FaxUdpEC", "t38UDPRedundancy", nMediaIndex);
	}

	pSDPMessage->getSDPWithBuffer(strBody, sizeof(strBody), nBody);
    pMessage->setBody(strBody, nBody, "application/SDP");
	//printSDPMessage(pSDPMessage);

    delete pSDPMessage;
	return true;
}

/** @brief	SIP Response 전송 */
int SIP_Session::proceed(int nStatus)
{
    APPLOG->Print( DBGLV_INF, "[0x%08X] INVITE request in progress(%d).", m_pDialog, nStatus );
	
	BTSIPMessage* pMessage = m_pDialog->outboundMessage();
	pMessage->clear();

	//SIP_Manager::inst->setServHeader(pMessage);
//    setContact( pMessage, CConfig::USER_LOCAL_URI );

	m_pDialog->proceed(nStatus);

	return 0;
}

int SIP_Session::accept(int nStatus)
{
    APPLOG->Print( DBGLV_INF, "[0x%08X] INVITE request accepted(%d).", __FUNCTION__, m_pDialog, nStatus );

    BTSIPMessage* pMessage = m_pDialog->outboundMessage();
	pMessage->clear();

//	setContact( pMessage, CConfig::USER_LOCAL_URI );
	if( ((m_nState == SESSION_STATE_IDLE) && CConfig::SERVICE_AUTO_OFFER) ||
		((m_nState == SESSION_STATE_TRYING) && CConfig::SERVICE_AUTO_ANSWER))
	{
		setSDPMessage(pMessage);
		m_nState++;	
	}

    m_pDialog->accept(nStatus);

	return 0;
}

int SIP_Session::reject(int nStatus)
{
    APPLOG->Print( DBGLV_INF, "[0x%08X] INVITE request rejected(%d).", m_pDialog, nStatus );

    BTSIPMessage* pMessage = m_pDialog->outboundMessage();
	pMessage->clear();

    /*if (nStatus < 400 || nStatus == 485)
    {
        for( int i=0; i < CConfig::USER_REDIRECT_URI.GetCount() ; i++ )
            setContact( pMessage, CConfig::USER_REDIRECT_URI[i] );
    }*/

	char strReason[128] = {0, };
	sprintf(strReason, "SIP;cause=%d;text=\"%s\"", nStatus, BTSIPMessage::const_SIPResponseReason[nStatus]);
	pMessage->parseOtherHeaderWithString("Reason", strReason);
    m_pDialog->reject(nStatus);

	return 0;
}

int SIP_Session::rejectNoSession( BTDialog* pDialog, int nStatus )
{
    APPLOG->Print( DBGLV_INF, "[0x%08X] INVITE request rejected(%d).", pDialog, nStatus );

    BTSIPMessage* pMessage = pDialog->outboundMessage();
	pMessage->clear();

    /*if (nStatus < 400 || nStatus == 485)
    {
        for( int i=0; i < CConfig::USER_REDIRECT_URI.GetCount() ; i++ )
            setContact( pMessage, CConfig::USER_REDIRECT_URI[i] );
    }*/

	char strReason[128] = {0, };
	sprintf(strReason, "SIP;cause=%d;text=\"%s\"", nStatus, BTSIPMessage::const_SIPResponseReason[nStatus]);
	pMessage->parseOtherHeaderWithString("Reason", strReason);
    pDialog->reject( nStatus );

	return 0;
}

int SIP_Session::bye()
{
	int nResult;

	if( !m_pDialog ) 
	{
		APPLOG->Print( DBGLV_ERR, "[CHAN_%03d] Session(0x%08X) Dialog(        ), DialogID(        ). fail to send BYE. not in dialog", 
						channel(), this, m_pDialog );
		return -1;
	}

    setBye();

	APPLOG->Print( DBGLV_INF, "[CHAN_%03d] Session(0x%08X) Dialog(0x%08X), DialogID(0x%08X). send BYE", 
					channel(), this, m_pDialog, m_pDialog->dialogID() );

	m_pDialog->outboundMessage()->clear();
	nResult = m_pDialog->disconnect();
	if( nResult != rtSUCCESS ) 
	{
		APPLOG->Print( DBGLV_ERR, "[CHAN_%03d] Session(0x%08X) Dialog(0x%08X), DialogID(0x%08X). fail to send BYE. result(%d)", 
					channel(), this, m_pDialog, m_pDialog->dialogID(), nResult );
		return -1;
	}

	return 0;
}

int SIP_Session::byeInUserContext()
{
	int nResult;

	if( !m_pDialog ) 
	{
		APPLOG->Print( DBGLV_ERR, "[CHAN_%03d] Session(0x%08X) Dialog(        ), DialogID(        ). fail to send BYE. not in dialog", 
						channel(), this, m_pDialog );
		return -1;
	}

	
	if( m_nState != SESSION_STATE_IDLE)
	{
		setBye();

		APPLOG->Print( DBGLV_INF, "[CHAN_%03d] Session(0x%08X) Dialog(0x%08X), DialogID(0x%08X). send BYE", 
						channel(), this, m_pDialog, m_pDialog->dialogID() );

		g_pAppMgr->sendNotification(APPEVT_DISCONNECT_CALL, StackObject_Dialog, m_pDialog, 0);
	}

	return 0;
}

int SIP_Session::byeNoSession( BTDialog* pDialog )
{
	int nResult;

	APPLOG->Print( DBGLV_INF, "[0x%08X] BYE send", pDialog );

	if( !pDialog ) 
	{
		APPLOG->Print( DBGLV_ERR, "[CHAN_   ] Session(        ) Dialog(0x%08X), DialogID(        ). fail to send BYE. not in dialog", pDialog );
		return -1;
	}

	APPLOG->Print( DBGLV_ERR, "[CHAN_   ] Session(        ) Dialog(0x%08X), DialogID(0x%08X). send BYE", pDialog, pDialog->dialogID() );
	
	pDialog->outboundMessage()->clear();
	nResult = pDialog->disconnect();
    if( nResult != rtSUCCESS ) 
	{
		APPLOG->Print( DBGLV_ERR, "[CHAN_   ] Session(        ) Dialog(0x%08X), DialogID(0x%08X). send BYE. result(%d)", pDialog, pDialog->dialogID(), nResult );
		return -1;
	}

	return 0;
}

/** @brief	INVITE 전송 */
BTDialog* SIP_Session::inviteInUserContext( const char* szTarget, const char* szFrom )
{
	MGW_INFO	MgwInfo;
	BTDialog*	pDialog;
	CString		strTargetURI, strFromURI, strContact;

	// Hunt MGW
	if( !HuntMgw( &MgwInfo ) )
	{
		APPLOG->Print( DBGLV_ERR, "[inviteInUserContext] MGW not available" );
		return NULL;
	}

	// Create Dialog
	g_pDlgMgr->createDialog(&pDialog);
	if (!pDialog)
	{
		APPLOG->Print( DBGLV_ERR, "[inviteInUserContext] Dialog-Pool not available" );
		return NULL;
	}
	pDialog->ready2call();
	
	if( szFrom && strlen(szFrom) > 0 )
		strFromURI.Format( "<sip:%s@%s:%d>", szFrom, (LPCSTR)CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT );
	else
		strFromURI.Format( "<sip:%s@%s:%d>", (LPCSTR)CConfig::USER_LOCAL_NAME, (LPCSTR)CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT );
	strContact.Format( "<sip:%s@%s:%d>", (LPCSTR)CConfig::USER_LOCAL_NAME, (LPCSTR)CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT );
    
	pDialog->setFromHeader((char*)(LPCSTR)strFromURI);
	pDialog->setToHeader((char*)(LPCSTR)strTargetURI);
	pDialog->setLocalContactHeader((char*)(LPCSTR)strContact);
    
	g_pAppMgr->sendNotification(APPEVT_MAKE_CALL, StackObject_Dialog, pDialog, 0);
/*
	BTSIPMessage* pMessage = pDialog->outboundMessage();
	if( CConfig::SERVICE_SESSION_EXPIRES > 0 )
		g_pDlgMgr->setSessionTimerParameters( pDialog, CConfig::SERVICE_SESSION_EXPIRES, CConfig::SERVICE_MIN_SE, 0 );
	pMessage->parseContactHeaderWithString( (char*)(LPCSTR)strContact );

	if( CConfig::SERVICE_AUTO_OFFER ) 
	{
		setSDPMessage( pMessage );
		setTrying();
	}

	pMessage->setTransport( (BTSIPMessage::transport_type)CConfig::SERVICE_TRANSPORT );
	
	if( g_pDlgMgr->makeCall(pDialog, (char*)(LPCSTR)strFromURI, (char*)(LPCSTR)strTargetURI) != rtSUCCESS )
    {
        APPLOG->Print( DBGLV_ERR, "[inviteInUserContext] failed to make call. strTarget=%s", szTarget );
		return NULL;
	}
*/

	return pDialog;
}

/** @brief	INVITE 전송 */
BTDialog* SIP_Session::inviteInUserContext(SIP_Session* pSession, const char* szTarget, const char* szFrom)
{
	MGW_INFO	MgwInfo;
	BTDialog*	pDialog;
	CString		strTargetURI, strFromURI, strContact;


	// Hunt MGW
	if( !HuntMgw( &MgwInfo ) )
	{
		APPLOG->Print( DBGLV_ERR, "[inviteInUserContext] MGW not available" );
		return NULL;
	}

	// Create Dialog
	g_pDlgMgr->createDialog(&pDialog);
	if (!pDialog)
	{
		APPLOG->Print( DBGLV_ERR, "[inviteInUserContext] Dialog-Pool not available" );
		return NULL;
	}
	pDialog->ready2call();

    pSession->setDialog( pDialog );
	
    // ADD - KIMCG : 20150910
    APPLOG->Print( DBGLV_INF, "target:%s, loop_fax:%s", szTarget, (const char*)CConfig::SERVICE_LOOP_FAXNO );
    if(strcmp(szTarget, (const char*)CConfig::SERVICE_LOOP_FAXNO) == 0)
    {
        strTargetURI.Format( "<sip:%s@%s:%d>", szTarget, (LPCSTR)CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT );
    }
    else
    {
        strTargetURI.Format( "<sip:%s@%s:%d>", szTarget, (LPCSTR)MgwInfo.strIp, MgwInfo.nPort );
    }
    // ADD - END

	if( szFrom && strlen(szFrom) > 0 )
		strFromURI.Format( "<sip:%s@%s:%d>", szFrom, (LPCSTR)CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT );
	else
		strFromURI.Format( "<sip:%s@%s:%d>", (LPCSTR)CConfig::USER_LOCAL_NAME, (LPCSTR)CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT );
	strContact.Format( "<sip:%s@%s:%d>", (LPCSTR)CConfig::USER_LOCAL_NAME, (LPCSTR)CConfig::ADDRESS_LOCAL_IP, CConfig::ADDRESS_LOCAL_PORT );
    
	pDialog->setFromHeader((char*)(LPCSTR)strFromURI);
	pDialog->setToHeader((char*)(LPCSTR)strTargetURI);
	pDialog->setLocalContactHeader((char*)(LPCSTR)strContact);
    
    // ADD - KIMCG : 20150914
    if(strcmp(szTarget, (const char*)CConfig::SERVICE_LOOP_FAXNO) == 0)
    {
        pDialog->getToHeader()->URI()->addOtherParam("loop_faxno", (char*)szTarget);
    }
    // ADD - END
    
	g_pAppMgr->sendNotification(APPEVT_MAKE_CALL, StackObject_Dialog, pDialog, 0);

    
/*
	BTSIPMessage* pMessage = pDialog->outboundMessage();
	if( CConfig::SERVICE_SESSION_EXPIRES > 0 )
		g_pDlgMgr->setSessionTimerParameters( pDialog, CConfig::SERVICE_SESSION_EXPIRES, CConfig::SERVICE_MIN_SE, 0 );
	pMessage->parseContactHeaderWithString( (char*)(LPCSTR)strContact );

	if( CConfig::SERVICE_AUTO_OFFER ) 
	{
		setSDPMessage( pMessage );
		setTrying();
	}

	pMessage->setTransport( (BTSIPMessage::transport_type)CConfig::SERVICE_TRANSPORT );
	
	if( g_pDlgMgr->makeCall(pDialog, (char*)(LPCSTR)strFromURI, (char*)(LPCSTR)strTargetURI) != rtSUCCESS )
    {
        APPLOG->Print( DBGLV_ERR, "[inviteInUserContext] failed to make call. strTarget=%s", szTarget );
		return NULL;
	}
*/

	return pDialog;
}

int SIP_Session::reinvite()
{
	MGW_INFO	MgwInfo;
	BTSIPMessage * pmsg;

	// Get MGW
	if( !GetMgw( &MgwInfo ) )
	{
		APPLOG->Print( DBGLV_ERR, "[reinviteInUserContext] MGW not set" );
		return NULL;
	}

	if (!m_pDialog || m_pDialog->state()!=BTDialog::DIALOG_STATE_CONNECTED)
    {
        APPLOG->Print( DBGLV_ERR, "# not valid dialog or dialog state." );
        return -1;
    }

	m_nState = SIP_Session::SESSION_STATE_RETRYING;

	pmsg=m_pDialog->outboundMessage();
	pmsg->clear();

	setSDPMessage( pmsg, true );
	
	m_pDialog->modify();
}

/** @brief	reINVITE 전송 */
int SIP_Session::reinviteInUserContext()
{
	MGW_INFO	MgwInfo;

	// Get MGW
	if( !GetMgw( &MgwInfo ) )
	{
		APPLOG->Print( DBGLV_ERR, "[reinviteInUserContext] MGW not set" );
		return NULL;
	}

	if (!m_pDialog || m_pDialog->state()!=BTDialog::DIALOG_STATE_CONNECTED)
    {
        APPLOG->Print( DBGLV_ERR, "# not valid dialog or dialog state." );
        return -1;
    }

	m_nState = SIP_Session::SESSION_STATE_RETRYING;

	g_pAppMgr->sendNotification(APPEVT_MODIFY_CALL, StackObject_Dialog, m_pDialog, 0);
/*
    BTSIPMessage* pMessage = m_pDialog->outboundMessage();

	pMessage->clearMessage();

	CString strContact;
	strContact.Format( "<sip:%s@%s:%d>", (LPCSTR)CConfig::USER_LOCAL_NAME, (LPCSTR)MgwInfo.strIp, MgwInfo.nPort );
	//pMessage->parseContactHeaderWithString( (char*)(LPCSTR)strContact );
    
	if( CConfig::SERVICE_AUTO_OFFER ) 
	{
		setSDPMessage(pMessage, true);
    	setRetrying();
	}

	//printSIPMessage( pMessage, true );

    if (g_pDlgMgr->modify(m_pDialog)!=rtSUCCESS) return -1;
*/

	return 0;
}
