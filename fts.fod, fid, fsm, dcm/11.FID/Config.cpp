// SIP_FIDIni.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "APP.h"
#include "Config.h"
#include "Utility.h"
#include "DbModule.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// [ 기본 정보 ]
CString			CConfig::EXEC_PATH					= "";
CString			CConfig::CONFIG_FILE				= "";
CString			CConfig::STATUS_FILE				= "";

// 공통 
CString			CConfig::APP_VER					= "v4.0.2.2";
CString			CConfig::SYSTEM_TYPE				= "";
long			CConfig::SYSTEM_NO					= -1;
CString			CConfig::SYSTEM_ID					= "";
CString			CConfig::SYSTEM_IP					= "";
P_TYPE			CConfig::PROCESS_TYPE				= P_FID;
CString			CConfig::PROCESS_TYPE_STR			= ProcessType(CConfig::PROCESS_TYPE);
long			CConfig::PROCESS_NO					= -1;
CString			CConfig::PROCESS_ID					= "";
CString			CConfig::SYSTEM_PROCESS_ID			= "";
CString			CConfig::DB_CONNECTION_STRING		= "";

// [ FSM 정보 ]
CString			CConfig::FSM_IP						= "";
int				CConfig::FSM_PORT					= 0;
int				CConfig::FSM_CHK_ALIVE_TIME			= 0;
int				CConfig::FSM_SND_ALIVE_TIME			= 0;

// [장애 알람정보]
CString			CConfig::SYSTEM_GROUP						= "";
UINT			CConfig::IOA_MODULE_ID						= 0;    

// [ FAX 회선 정보 ]
CString			CConfig::FAX_CH_MONI_SHM_KEY		= "";
int				CConfig::FAX_START_CH				= 0;
int				CConfig::FAX_CH_CNT					= 0;

// [ RTP 정보 ]
int				CConfig::RTP_BASE_PORT				= 0;
int				CConfig::RTP_PORT_SPACE				= 0;

// [ 경로 정보 ]
CString			CConfig::LOCAL_TIF_PATH				= "";
CString			CConfig::STG_HOME_PATH				= "";
CString			CConfig::INBOUND_TIF_PATH			= "";
CString			CConfig::INBOUND_TIF_FULL_PATH		= "";

// [팩스번호 암호화 정보]
CString			CConfig::ENCRYPT_FIELD_YN           = "N";		
CString			CConfig::ENCRYPT_DLL_FILE			= "";


// [LOG] : LOG 정보
CString			CConfig::LOG_PATH					= "";
bool			CConfig::LOG_FILE_SAVE				= true;
int				CConfig::LOG_LEVEL					= 0;
EnumLogFileSaveUnit	CConfig::LOG_SAVE_UNIT			= LOG_FILE_SAVE_UNIT_DAY;



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{
	/*char szVal[200];
	GetCurrentDirectory(sizeof(szVal), szVal);
	EXEC_PATH = CString(szVal) + "/";*/

	
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
	STATUS_FILE = EXEC_PATH + "cfg/Fax_Channel.ini";
}

CConfig::~CConfig()
{
}

//void CConfig::SetCallback( Config_Display p_pfnCallback )
//{
//	s_pfnDisplayFn = p_pfnCallback;
//}

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
	CString strTemp;

	if( SYSTEM_IP == "SYSTEM_IP" )
	{
		if( !_ReadDbConfig( "SYSTEM_IP",		&SYSTEM_IP) )			return false;
	}

	// -------------------------------------------------------------------
	// [FASIP] : FASIP 연동 정보
	// -------------------------------------------------------------------
	if( !_ReadDbConfig( "FSM_IP",				&FSM_IP) )				return false;
	if( !_ReadDbConfig( "FSM_PORT",				&FSM_PORT ) )			return false;
	if( !_ReadDbConfig( "FSM_SND_ALIVE_TIME",	&FSM_SND_ALIVE_TIME) )	return false;
	if( !_ReadDbConfig( "FSM_CHK_ALIVE_TIME",	&FSM_CHK_ALIVE_TIME) )	return false;

	// -------------------------------------------------------------------
	// [장애알람 정보]	
	// -------------------------------------------------------------------
	if( !_ReadDbConfig( "SYSTEM_GROUP",						&SYSTEM_GROUP) )					SYSTEM_GROUP		= "90000";	
	if( !_ReadDbConfig( "IOA_MODULE_ID",					&IOA_MODULE_ID) )					IOA_MODULE_ID		= 589825;

	// -------------------------------------------------------------------
	// [FAX] : FAX 회선정보 (시작 CH번호, CH 개수, 다음 점유회선 Index, 발신번호)
	// -------------------------------------------------------------------
	if( !_ReadDbConfig( "FAX_CH_MONI_SHM_KEY",	&FAX_CH_MONI_SHM_KEY) )		return false;
	if( !_ReadDbConfig( "FAX_START_CH",			&FAX_START_CH ) )		return false;
	if( !_ReadDbConfig( "FAX_CH_CNT",			&FAX_CH_CNT) )			return false;

	// CH 개수는 MAX_FIDCH을 넘을 수 없다...
	if(FAX_CH_CNT > MAX_CHANNEL)
		return false;

	// -------------------------------------------------------------------
	// [RTP] : RTP 정보
	// -------------------------------------------------------------------
	if( !_ReadDbConfig( "RTP_BASE_PORT",			&RTP_BASE_PORT) )	return false;
	if( !_ReadDbConfig( "RTP_PORT_SPACE",		&RTP_PORT_SPACE) )		return false;

	// -------------------------------------------------------------------
	// [PATH 정보]
	// -------------------------------------------------------------------
	LOCAL_TIF_PATH = EXEC_PATH + "tif/";
	if( !_ReadDbConfig( "STG_HOME_PATH",			&STG_HOME_PATH) )	return false;
	if( !_ReadDbConfig( "INBOUND_TIF_PATH",		&INBOUND_TIF_PATH) )	return false;
	INBOUND_TIF_FULL_PATH = STG_HOME_PATH + INBOUND_TIF_PATH;
	
    if( !_ReadDbConfig( "LOG_HOME_PATH",			&LOG_PATH) )
        LOG_PATH = "X";

    if(LOG_PATH.GetLength() <= 0 || LOG_PATH == "X")
        LOG_PATH = EXEC_PATH + "log/";
    else
        LOG_PATH.Format("%s\\%s\\", LOG_PATH, PROCESS_TYPE_STR);

    // directory exists check
    if(!CUtility::DirectoryExists(LOG_PATH))
    {
        if(!CreateDirectory(LOG_PATH, NULL))
            LOG_PATH = EXEC_PATH + "log\\";
    }

    // -------------------------------------------------------------------
	// [팩스번호 암호화 정보]	
	// -------------------------------------------------------------------
	if( !_ReadDbConfig( "ENCRYPT_FIELD_YN",			&ENCRYPT_FIELD_YN) )	ENCRYPT_FIELD_YN	= "N";
    if( !_ReadDbConfig( "ENCRYPT_DLL_FILE",			&ENCRYPT_DLL_FILE) )	ENCRYPT_DLL_FILE	= "";


	// -------------------------------------------------------------------
	// [LOG] : LOG 정보 (LOG 위치, LOG Level. Log File 저장 여부, Log File 저장 단위)
	// -------------------------------------------------------------------
	
	if( !_ReadDbConfig( "LOG_LEVEL",			&LOG_LEVEL) )			return false;
	if( !_ReadDbConfig( "LOG_FILE_SAVE",		&LOG_FILE_SAVE) )		return false;
	if( !_ReadDbConfig( "LOG_SAVE_UNIT",		&strTemp) )				return false;
	LOG_SAVE_UNIT	= ( strTemp == "DAY" ) ? LOG_FILE_SAVE_UNIT_DAY : LOG_FILE_SAVE_UNIT_HOUR;

	return true;
}







// -------------------------------------------------------------------
// Module 명	: void DisplayAllConfig()
// -------------------------------------------------------------------
// Descriotion	: Config File에서 특정 Section의 특정 Key 값을 가져오기 (Integer일 경우)
// -------------------------------------------------------------------
// Argument		: char* pSection;		Section
//				  char* pKey;			Key
//				  char* pIniFile;		INI File 명
//				  int &Value;			Section과 Key에 해당하는 값
// -------------------------------------------------------------------
// Return 값	: true		SUCC
//				  false		FAIL
// -------------------------------------------------------------------
void CConfig::DisplayAllConfig()
{
	APPLOG->Print(DBGLV_INF0, " =============================================================================== ");
	APPLOG->Print(DBGLV_INF0, " []           EXEC_PATH                  [%s]", (LPCSTR) EXEC_PATH);
	APPLOG->Print(DBGLV_INF0, " []           LOG_PATH                   [%s]", (LPCSTR) LOG_PATH);
	APPLOG->Print(DBGLV_INF0, " []           LOCAL_TIF_PATH             [%s]", (LPCSTR) LOCAL_TIF_PATH);
	APPLOG->Print(DBGLV_INF0, " []           STG_HOME_PATH              [%s]", (LPCSTR) STG_HOME_PATH);
	APPLOG->Print(DBGLV_INF0, " []           INBOUND_TIF_FULL_PATH      [%s]", (LPCSTR) INBOUND_TIF_FULL_PATH);
	APPLOG->Print(DBGLV_INF0, " []           CONFIG_FILE                [%s]", (LPCSTR) CONFIG_FILE);
	APPLOG->Print(DBGLV_INF0, " []           STATUS_FILE                [%s]", (LPCSTR) STATUS_FILE);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     SYSTEM_NO                  [%d]", SYSTEM_NO);
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     SYSTEM_ID                  [%s]", (LPCSTR) SYSTEM_ID);
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     SYSTEM_IP                  [%s]", (LPCSTR) SYSTEM_IP);
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     PROCESS_ID                 [%s]", (LPCSTR) PROCESS_ID);
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     SYSPROCID                  [%s]", (LPCSTR) SYSTEM_PROCESS_ID);
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     DB_CONNECTION_STRING       [%s]", (LPCSTR) DB_CONNECTION_STRING);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [FASIP]      FSM_IP                     [%s]", (LPCSTR) FSM_IP);
	APPLOG->Print(DBGLV_INF0, " [FASIP]      FSM_PORT			       [%d]", FSM_PORT);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_START_CH               [%d]", FAX_START_CH);
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_CH_CNT                 [%d]", FAX_CH_CNT);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [RTP]        RTP_BASE_PORT               [%d]", RTP_BASE_PORT);
	APPLOG->Print(DBGLV_INF0, " [RTP]        RTP_PORT_SPACE              [%d]", RTP_PORT_SPACE);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
    APPLOG->Print(DBGLV_INF0, " [ENCRYPT]   ENCRYPT_FIELD_YN            [%s]", ENCRYPT_FIELD_YN );
    APPLOG->Print(DBGLV_INF0, " [ENCRYPT]   ENCRYPT_DLL_FILE            [%s]", ENCRYPT_DLL_FILE );
    APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [LOG]        LOG_LEVEL                  [%d]", LOG_LEVEL);
	APPLOG->Print(DBGLV_INF0, " [LOG]        LOG_FILE_SAVE              [%c]", (LOG_FILE_SAVE) ? 'Y' : 'N');
	APPLOG->Print(DBGLV_INF0, " [LOG]        LOG_SAVE_UNIT              [%d]", (int) LOG_SAVE_UNIT);
	APPLOG->Print(DBGLV_INF0, " =============================================================================== ");
}

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
//	if( s_pfnDisplayFn != NULL )
//		s_pfnDisplayFn( p_szKey, *p_pstrValue );
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

