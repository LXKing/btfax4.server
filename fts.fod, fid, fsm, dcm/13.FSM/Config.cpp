// SIP_FIDIni.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "APP.h"
#include "Config.h"
#include "Utility.h"
#include "DbModule.h"

#include "SIP_Mgws.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// [ 기본 정보 ]
CString			CConfig::EXEC_PATH						= "";
CString			CConfig::CONFIG_FILE					= "";

// [ 공통 정보 ]
CString			CConfig::APP_VER						= "v4.0";
CString			CConfig::SYSTEM_TYPE					= "";
long			CConfig::SYSTEM_NO						= -1;
CString			CConfig::SYSTEM_ID						= "";
CString			CConfig::SYSTEM_IP						= "";
P_TYPE			CConfig::PROCESS_TYPE					= P_FSM;
CString			CConfig::PROCESS_TYPE_STR				= ProcessType( CConfig::PROCESS_TYPE );
long			CConfig::PROCESS_NO						= -1;
CString			CConfig::PROCESS_ID						= "";
CString			CConfig::SYSTEM_PROCESS_ID				= "";
CString			CConfig::DB_CONNECTION_STRING			= "";

int				CConfig::SYSTEM_MAX_DIALOGS				= 0;
int				CConfig::SYSTEM_MAX_REGISTERS			= 0;
int				CConfig::SYSTEM_MAX_TRANSCS				= 0;
int				CConfig::SYSTEM_TIMER_POOL_CNT			= 0;
int				CConfig::SYSTEM_DISPATCHER_CNT			= 0;
int				CConfig::SYSTEM_TIMERMGR_CNT			= 0;
int				CConfig::SYSTEM_EVENTMGR_CNT			= 0;
CString			CConfig::SYSTEM_LOG_PATH				= "";
int				CConfig::SYSTEM_LOG_LEVEL				= 0;
int				CConfig::SYSTEM_BTSIP_LOG_LEVEL			= 0;

bool			CConfig::ADDRESS_FSM_CROSS				= false;
CString			CConfig::ADDRESS_LOCAL_IP				= "";
int				CConfig::ADDRESS_LOCAL_PORT				= 0;
int				CConfig::ADDRESS_MGW_ALIVE_SND_TIME		= 0;
int				CConfig::ADDRESS_MGW_ALIVE_CHK_TIME		= 0;
int				CConfig::ADDRESS_RTP_PORT_BASE			= 0;
int				CConfig::ADDRESS_CLIENT_LISTEN_PORT		= 0;
int				CConfig::ADDRESS_CLIENT_SESSION_BEGIN	= 0;
int				CConfig::ADDRESS_CLIENT_SESSION_END		= 0;
int				CConfig::ADDRESS_CLIENT_ALIVE_CHK_TIME	= 0;

CString			CConfig::USER_LOCAL_URI					= "";
CString			CConfig::USER_LOCAL_NAME				= "";
CStringArray	CConfig::USER_REDIRECT_URI;

int				CConfig::TIMER_1						= 0;
int				CConfig::TIMER_2						= 0;
int				CConfig::TIMER_4						= 0;
int				CConfig::TIMER_D						= 0;
int				CConfig::TIMER_P						= 0;

CString			CConfig::SERVICE_REGIST_YN				= "";
int				CConfig::SERVICE_PERF_CDT				= 0;
int				CConfig::SERVICE_ALERT_EXPIRES			= 0;
int				CConfig::SERVICE_SESSION_EXPIRES		= 0;
int				CConfig::SERVICE_MIN_SE					= 0;
int				CConfig::SERVICE_EXPIRES				= 0;
int				CConfig::SERVICE_STATUS					= 0;
int				CConfig::SERVICE_DELAY					= 0;
int				CConfig::SERVICE_TCP_WAIT_TIME			= 0;
int				CConfig::SERVICE_UDP_MTU_SIZE			= 0;
CString			CConfig::SERVICE_TRANSPORT_TYPE			= "";
int				CConfig::SERVICE_TRANSPORT				= 0;
int				CConfig::SERVICE_CALLING_TIMEOUT		= 0;
int				CConfig::SERVICE_CHECK_METHOD			= 0;
CStringArray	CConfig::SERVICE_SERVE_METHOD;
int				CConfig::SERVICE_CHECK_SUPPORTED		= 0;
CStringArray	CConfig::SERVICE_SERVE_SUPPORTED;
bool			CConfig::SERVICE_AUTO_RESPONSE			= false;
bool			CConfig::SERVICE_AUTO_ACK				= false;
bool			CConfig::SERVICE_AUTO_PRACK				= false;
bool			CConfig::SERVICE_AUTO_TRYING			= false;
bool			CConfig::SERVICE_AUTO_RINGING			= false;
bool			CConfig::SERVICE_AUTO_OFFER				= false;
bool			CConfig::SERVICE_AUTO_ANSWER			= false;

// ADD - KIMCG : 20150914
// LOOP(순환) 팩스번호 추가 
CString		    CConfig::SERVICE_LOOP_FAXNO             = "";
// ADD - END


// [장애 알람정보]
CString			CConfig::SYSTEM_GROUP					= "";
UINT			CConfig::IOA_MODULE_ID					= 0;  

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{
	//char szVal[200];

	////m_pfnDisplayFn = NULL;
	//다른프로세스에서 FOD프로세스를 기동할때 문제가 발생(GetCurrentDirectory())
	//GetCurrentDirectory(sizeof(szVal), szVal);
	//EXEC_PATH = CString(szVal) + "\\";
	
	CString strPath;
	
	strPath.GetBufferSetLength(1024);

	GetModuleFileName(NULL,(LPTSTR)(LPCTSTR)strPath,1024);

	int nCurPos = 0,nPos = 0; 

	while(1)
	{
		nCurPos = strPath.Find(_T("\\"),nCurPos+1);
		if(nCurPos == -1)
		{
			strPath = strPath.Left(nPos);
			break;
		}
		nPos = nCurPos; 
	} 

	EXEC_PATH = strPath + "/";
		
	CONFIG_FILE = EXEC_PATH + "../Btfax.ini";
}

CConfig::~CConfig()
{
}



//void CConfig::SetCallback( Config_Display p_pfnCallback )
//{
//	m_pfnDisplayFn = p_pfnCallback;
//}

// -------------------------------------------------------------------
// Module 명	: bool LoadIni()
// -------------------------------------------------------------------
// Descriotion	: Load Config File
// -------------------------------------------------------------------
// Argument		: char* pIniFile;		INI File 명
// -------------------------------------------------------------------
// Return 값	: true		SUCC
//				  false		FAIL
// -------------------------------------------------------------------
bool CConfig::LoadConfig_file()
{
	int		nRet;
	char	szVal[200];

	
	// -------------------------------------------------------------------
	// [SYSTEM] : FOD 시스템 (장비번호, 장비ID, 프로세스ID, 장비/프로세스 ID, 장비IP)
	// -------------------------------------------------------------------
	if( !_ReadConfig( "SYSTEM", "SYSTEM_TYPE", &SYSTEM_TYPE ) )
		return false;
	if( !_ReadConfig( "SYSTEM", "SYSTEM_NO", &SYSTEM_NO ) )
		return false;
	SYSTEM_ID.Format( "%s_%d", (LPCSTR)SYSTEM_TYPE, SYSTEM_NO );

	PROCESS_ID.Format( "%s_%d", ProcessType(PROCESS_TYPE), PROCESS_NO );
	SYSTEM_PROCESS_ID = SYSTEM_ID + "_" + PROCESS_ID;
		
	if( !_ReadConfig( "SYSTEM", "SYSTEM_IP", &SYSTEM_IP ) )
		return false;
	
	CString temp= "";	
	int		len = 0;
	char*	b64encoded_text = NULL;
	char	dbDataSource[30];
	char	dbUserId[30];
	char	dbUserPw[30];

	memset(dbDataSource	, 0x00, sizeof(dbDataSource));
	memset(dbUserId		, 0x00, sizeof(dbDataSource));
	memset(dbUserPw		, 0x00, sizeof(dbDataSource));

	//// 데이터소스 ////
	if( !_ReadConfig( "SYSTEM", "DB_CONN_DATA_SOURCE", &temp ) )
		return false;

	if(temp.GetLength() <= 0)
	{	
		_ReadConfig( "SYSTEM", "DB_CONN_DATA_SOURCE_ENK", &temp );		
		len = CUtility::Base64_decode((LPSTR)(LPCSTR)temp, dbDataSource, sizeof(dbDataSource));
		if(len <=0)
			return false;
	}
	else
	{	
		b64encoded_text = NULL;
		len = CUtility::Base64_encode((LPSTR)(LPCSTR)temp, temp.GetLength() + 1, &b64encoded_text);
		
		_WriteConfig("SYSTEM", "DB_CONN_DATA_SOURCE_ENK", b64encoded_text);
		_WriteConfig("SYSTEM", "DB_CONN_DATA_SOURCE", "");
		
		strcpy(dbDataSource, temp);
		if(b64encoded_text != NULL) 
		{
			free(b64encoded_text);
			b64encoded_text = NULL;
		}
	}

	temp = "";
	len = 0;

	//// 사용자아이디 ////
	if( !_ReadConfig( "SYSTEM", "DB_CONN_USER_ID", &temp ) )
		return false;

	if(temp.GetLength() <= 0)
	{	
		_ReadConfig( "SYSTEM", "DB_CONN_USER_ID_ENK", &temp );		
		len = CUtility::Base64_decode((LPSTR)(LPCSTR)temp, dbUserId, sizeof(dbUserId));
		if(len <=0)
			return false;
	}
	else
	{	
		b64encoded_text = NULL;
		len = CUtility::Base64_encode((LPSTR)(LPCSTR)temp, temp.GetLength() + 1, &b64encoded_text);
		
		_WriteConfig("SYSTEM", "DB_CONN_USER_ID_ENK", b64encoded_text);
		_WriteConfig("SYSTEM", "DB_CONN_USER_ID", "");
		
		strcpy(dbUserId, temp);
		if(b64encoded_text != NULL) 
		{
			free(b64encoded_text);
			b64encoded_text = NULL;
		}
	}

	temp = "";
	len = 0;

	//// 사용자 암호 ////
	if( !_ReadConfig( "SYSTEM", "DB_CONN_USER_PW", &temp ) )
		return false;

	if(temp.GetLength() <= 0)
	{	
		_ReadConfig( "SYSTEM", "DB_CONN_USER_PW_ENK", &temp );		
		len = CUtility::Base64_decode((LPSTR)(LPCSTR)temp, dbUserPw, sizeof(dbUserPw));
		if(len <=0)
			return false;
	}
	else
	{	
		b64encoded_text = NULL;
		len = CUtility::Base64_encode((LPSTR)(LPCSTR)temp, temp.GetLength() + 1, &b64encoded_text);
		
		_WriteConfig("SYSTEM", "DB_CONN_USER_PW_ENK", b64encoded_text);
		_WriteConfig("SYSTEM", "DB_CONN_USER_PW", "");
		
		strcpy(dbUserPw, temp);
		if(b64encoded_text != NULL) 
		{
			free(b64encoded_text);
			b64encoded_text = NULL;
		}
	}

	DB_CONNECTION_STRING.Format("Provider=OraOLEDB.Oracle; PLSQLRSet=2; Data Source=%s; User ID=%s; Password=%s",dbDataSource, dbUserId, dbUserPw);
	
	return true;
}

bool CConfig::LoadConfig_db()
{
	int			i, mgwCnt, port;
	bool		bPrimary;
	CString		connStr, id, pwd, temp, strKey, strIp;

	if( SYSTEM_IP == "SYSTEM_IP" )
	{
		if( !_ReadDbConfig( "SYSTEM_IP",		&SYSTEM_IP) )			return false;
	}

    // -------------------------------------------------------------------
	// [LOG] : LOG 정보 (LOG 위치, LOG Level. Log File 저장 여부, Log File 저장 단위)
	// -------------------------------------------------------------------    
    if( !_ReadDbConfig( "LOG_HOME_PATH",			&SYSTEM_LOG_PATH) )
        SYSTEM_LOG_PATH = "X";

    if(SYSTEM_LOG_PATH.GetLength() <= 0 || SYSTEM_LOG_PATH == "X")
        SYSTEM_LOG_PATH = EXEC_PATH + "log/";
    else
        SYSTEM_LOG_PATH.Format("%s\\%s\\", SYSTEM_LOG_PATH, PROCESS_TYPE_STR);

    // directory exists check
    if(!CUtility::DirectoryExists(SYSTEM_LOG_PATH))
    {
        if(!CreateDirectory(SYSTEM_LOG_PATH, NULL))
            SYSTEM_LOG_PATH = EXEC_PATH + "log\\";
    }
		
	if( !_ReadDbConfig( "SYSTEM.MAX_DIALOGS",				&SYSTEM_MAX_DIALOGS ) )				return false;
	if( !_ReadDbConfig( "SYSTEM.MAX_REGISTERS",				&SYSTEM_MAX_REGISTERS ) )			return false;
	if( !_ReadDbConfig( "SYSTEM.MAX_TRANSACTIONS",			&SYSTEM_MAX_TRANSCS ) )				return false;
	if( !_ReadDbConfig( "SYSTEM.TIMER_POOL_CNT",			&SYSTEM_TIMER_POOL_CNT ) )			return false;
	if( !_ReadDbConfig( "SYSTEM.DISPATCHER_CNT",			&SYSTEM_DISPATCHER_CNT ) )			return false;
	if( !_ReadDbConfig( "SYSTEM.TIMERMGR_CNT",				&SYSTEM_TIMERMGR_CNT ) )			return false;
	if( !_ReadDbConfig( "SYSTEM.EVENTMGR_CNT",				&SYSTEM_EVENTMGR_CNT ) )			return false;
	

	if( !_ReadDbConfig( "SYSTEM.LOG_LEVEL",					&SYSTEM_LOG_LEVEL ) )				return false;
	if( !_ReadDbConfig( "SYSTEM.BTSIP_LOG_LEVEL",			&temp ) )							return false;
	SYSTEM_BTSIP_LOG_LEVEL = BTSIP_LOGLEVEL( (LPCSTR)temp );
	
	if( !_ReadDbConfig( "ADDRESS.FSM_CROSS",				&ADDRESS_FSM_CROSS ) )				return false;
	if( !_ReadDbConfig( "ADDRESS.LOCAL.IP",					&ADDRESS_LOCAL_IP ) )				return false;
	if( ADDRESS_LOCAL_IP == "SYSTEM_IP" )
		ADDRESS_LOCAL_IP = SYSTEM_IP;
	if( !_ReadDbConfig( "ADDRESS.LOCAL.PORT",				&ADDRESS_LOCAL_PORT ) )				return false;
	if( !_ReadDbConfig( "ADDRESS.MGW.CNT",					&mgwCnt ) )							return false;
	for( i = 1 ; i <= mgwCnt ; ++i )
	{
		strKey.Format( "ADDRESS.MGW.%d.IP", i );
		if( !_ReadDbConfig( strKey,				&strIp ) )
			return false;

		strKey.Format( "ADDRESS.MGW.%d.PORT", i );
		if( !_ReadDbConfig( strKey,			&port ) )
			return false;

		strKey.Format( "ADDRESS.MGW.%d.PRIMARY", i );
		if( !_ReadDbConfig( strKey,	&bPrimary ) )
			return false;

		SIP_Mgws::Inst()->RegistMgw( bPrimary, strIp, port );
	}
	if( !_ReadDbConfig( "ADDRESS.MGW.ALIVE.SND_TIME",		&ADDRESS_MGW_ALIVE_SND_TIME ) )		return false;
	if( !_ReadDbConfig( "ADDRESS.MGW.ALIVE.CHK_TIME",		&ADDRESS_MGW_ALIVE_CHK_TIME ) )		return false;
	if( !_ReadDbConfig( "ADDRESS.RTP.PORT_BASE",			&ADDRESS_RTP_PORT_BASE ) )			return false;
	if( !_ReadDbConfig( "ADDRESS.CLIENT.LISTEN_PORT",		&ADDRESS_CLIENT_LISTEN_PORT ) )		return false;
	if( !_ReadDbConfig( "ADDRESS.CLIENT.SESSION_BEGIN",		&ADDRESS_CLIENT_SESSION_BEGIN ) )	return false;
	if( !_ReadDbConfig( "ADDRESS.CLIENT.SESSION_END",		&ADDRESS_CLIENT_SESSION_END ) )		return false;
	if( !_ReadDbConfig( "ADDRESS.CLIENT.ALIVE.CHK_TIME",	&ADDRESS_CLIENT_ALIVE_CHK_TIME ) )	return false;

	if( !_ReadDbConfig( "USER.LOCAL_NAME",					&USER_LOCAL_NAME ) )				return false;
	USER_LOCAL_URI.Format( "sip:%s@%s:%d", (LPCSTR)USER_LOCAL_NAME, (LPCSTR)ADDRESS_LOCAL_IP, ADDRESS_LOCAL_PORT );
	if( !_ReadDbConfig( "USER.REDIRECT_URI",				&USER_REDIRECT_URI ) )				return false;
	
	if( !_ReadDbConfig( "TIMER.TIMER_1",					&TIMER_1 ) )						return false;
	if( !_ReadDbConfig( "TIMER.TIMER_2",					&TIMER_2 ) )						return false;
	if( !_ReadDbConfig( "TIMER.TIMER_4",					&TIMER_4 ) )						return false;
	if( !_ReadDbConfig( "TIMER.TIMER_D",					&TIMER_D ) )						return false;
	if( !_ReadDbConfig( "TIMER.TIMER_P",					&TIMER_P ) )						return false;
	
	if( !_ReadDbConfig( "SERVICE.REGIST_YN",				&SERVICE_REGIST_YN ) )				return false;
	if( !_ReadDbConfig( "SERVICE.PERF_CDT",					&SERVICE_PERF_CDT ) )				return false;
	if( !_ReadDbConfig( "SERVICE.ALERT_EXPIRES",			&SERVICE_ALERT_EXPIRES ) )			return false;
	SERVICE_SESSION_EXPIRES = 0;
	SERVICE_MIN_SE			= 0;
	if( !_ReadDbConfig( "SERVICE.EXPIRES",					&SERVICE_EXPIRES ) )				return false;
	if( !_ReadDbConfig( "SERVICE.STATUS",					&SERVICE_STATUS ) )					return false;
	if( !_ReadDbConfig( "SERVICE.DELAY",					&SERVICE_DELAY ) )					return false;
	if( !_ReadDbConfig( "SERVICE.TCP_WAIT_TIME",			&SERVICE_TCP_WAIT_TIME ) )			return false;
	if( !_ReadDbConfig( "SERVICE.UDP_MTU_SIZE",				&SERVICE_UDP_MTU_SIZE ) )			return false;
	if( !_ReadDbConfig( "SERVICE.TRANSPORT_TYPE",			&SERVICE_TRANSPORT_TYPE ) )			return false;
	SERVICE_TRANSPORT = (SERVICE_TRANSPORT_TYPE == "udp") ? 1:2;

	if( !_ReadDbConfig( "SERVICE.CALLING_TIMEOUT",			&SERVICE_CALLING_TIMEOUT ) )		return false;
	

	if( !_ReadDbConfig( "SERVICE.CHECK_METHOD",				&SERVICE_CHECK_METHOD ) )			return false;
	if( !_ReadDbConfig( "SERVICE.SERVE_METHOD",				&SERVICE_SERVE_METHOD ) )			return false;
	if( !_ReadDbConfig( "SERVICE.CHECK_SUPPORTED",			&SERVICE_CHECK_SUPPORTED ) )		return false;
	if( !_ReadDbConfig( "SERVICE.SERVE_SUPPORTED",			&SERVICE_SERVE_SUPPORTED ) )		return false;

	if( !_ReadDbConfig( "SERVICE.AUTO_RESPONSE",			&SERVICE_AUTO_RESPONSE ) )			return false;
	if( !_ReadDbConfig( "SERVICE.AUTO_ACK",					&SERVICE_AUTO_ACK ) )				return false;
	if( !_ReadDbConfig( "SERVICE.AUTO_PRACK",				&SERVICE_AUTO_PRACK ) )				return false;
	if( !_ReadDbConfig( "SERVICE.AUTO_TRYING",				&SERVICE_AUTO_TRYING ) )			return false;
	if( !_ReadDbConfig( "SERVICE.AUTO_RINGING",				&SERVICE_AUTO_RINGING ) )			return false;
    if( !_ReadDbConfig( "SERVICE.AUTO_OFFER",				&SERVICE_AUTO_OFFER ) )				return false;
	if( !_ReadDbConfig( "SERVICE.AUTO_ANSWER",				&SERVICE_AUTO_ANSWER ) )			return false;

    // ADD - KIMCG : 20150914
    if( !_ReadDbConfig( "LOOP_FAXNO",				        &SERVICE_LOOP_FAXNO ) )
        SERVICE_LOOP_FAXNO = "";
    // ADD - END

	// [장애알람 정보]	
	if( !_ReadDbConfig( "SYSTEM_GROUP",						&SYSTEM_GROUP) )					SYSTEM_GROUP		= "90000";
	if( !_ReadDbConfig( "IOA_MODULE_ID",					&IOA_MODULE_ID) )					IOA_MODULE_ID		= 589825;

	return true;
}







//
//
//bool CConfig::_ReadConfig( const char* p_szSection, const char* p_szKey, CString* p_pstrValue )
//{
//	char szValue[1024];
//
//	::GetPrivateProfileString( p_szSection, p_szKey, "__none__", szValue, sizeof(szValue), CConfig::CONFIG_FILE );
//	if( !strcmp( szValue, "__none__") ) 
//	{
//		CString strMsg;
//
//		strMsg.Format( "'%s' 환경파일에서, [%s]를 읽지 못하였습니다.", (LPCSTR)CConfig::CONFIG_FILE, p_szKey );
//		AfxMessageBox( strMsg );
//		return false;
//	}
//
//	*p_pstrValue = szValue;
//	return true;
//}
//
//bool CConfig::_ReadConfig( const char* p_szSection, const char* p_szKey, int* p_pnValue )
//{
//	CString strValue = "";
//
//	if( !_ReadConfig( p_szSection, p_szKey, &strValue ) )
//		return false;
//
//	*p_pnValue = atoi( strValue );
//	return true;
//}
//
//bool CConfig::_ReadConfig( const char* p_szSection, const char* p_szKey, bool* p_pbValue )
//{
//	CString strValue = "";
//
//	if( !_ReadConfig( p_szSection, p_szKey, &strValue ) )
//		return false;
//
//	if( strValue == "Y" )
//		*p_pbValue = true;
//	else
//		*p_pbValue = false;
//
//	return true;
//}
//
//bool CConfig::_ReadDbConfig( CDbModule& p_db, const char* p_szKey, CString* p_pstrValue )
//{
//	if( p_db.ReadConfig( p_szKey, p_pstrValue ) <= 0 )
//	{
//		CString strMsg;
//
//		strMsg.Format( "PROCESS_CONFIG 테이블에서, [%s]를 읽지 못하였습니다.", p_szKey );
//		AfxMessageBox( strMsg );
//		return false;
//	}
//
//	if( m_pfnDisplayFn != NULL )
//		m_pfnDisplayFn( p_szKey, *p_pstrValue );
//
//	return true;
//}
//
//bool CConfig::_ReadDbConfig( CDbModule& p_db, const char* p_szKey, int* p_pnValue )
//{
//	CString strValue;
//
//	if( !_ReadDbConfig( p_db, p_szKey, &strValue ) )
//		return false;
//
//	*p_pnValue = atoi( strValue );
//
//	return true;
//}
//
//bool CConfig::_ReadDbConfig( CDbModule& p_db, const char* p_szKey, bool* p_pbValue )
//{
//	CString strValue = "";
//
//	if( !_ReadDbConfig( p_db, p_szKey, &strValue ) )
//		return false;
//
//	if( strValue == "Y" )
//		*p_pbValue = true;
//	else
//		*p_pbValue = false;
//
//	return true;
//}
//
//bool CConfig::_ReadDbConfig( CDbModule& p_db, const char* p_szKey, CStringArray* p_psaValues )
//{
//	CString strToken;
//	CString strValue = "";
//
//	if( !_ReadDbConfig( p_db, p_szKey, &strValue ) )
//		return false;
//
//	for( int i = 0 ; ; ++i )
//	{
//		if( !AfxExtractSubString(strToken, strValue, i, ',') )
//			break;
//		strToken.Trim();
//		p_psaValues->Add( strToken );
//	}
//
//	return true;
//}

void CConfig::Display()
{

}