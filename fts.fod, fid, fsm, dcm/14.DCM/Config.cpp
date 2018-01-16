// SIP_FODIni.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "APP.h"
#include "Config.h"
#include "Utility.h"
#include "DbModule.h"
#include "enum.h"

// -------------------------------------------------------------------
// System Header File
// -------------------------------------------------------------------
#include <string.h>


using namespace COMMON_LIB;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// [ DCM 기본 정보 ]
CString			CConfig::EXEC_PATH							= "";
CString			CConfig::CONFIG_FILE						= "";
CString			CConfig::STATUS_FILE						= "";

// 공통 
CString			CConfig::APP_VER							= "v4.0.2.2";
CString			CConfig::SYSTEM_TYPE						= "";
long			CConfig::SYSTEM_NO							= -1;
CString			CConfig::SYSTEM_ID							= "";
P_TYPE			CConfig::PROCESS_TYPE						= P_DCM;
CString			CConfig::PROCESS_TYPE_STR					= ProcessType(CConfig::PROCESS_TYPE);
long			CConfig::PROCESS_NO							= -1;
CString			CConfig::PROCESS_ID							= "";
CString			CConfig::SYSTEM_PROCESS_ID					= "";
CString			CConfig::DB_CONNECTION_STRING				= "";

// [DSM 접속정보]
CString			CConfig::DSM_IP								= "";				
int				CConfig::DSM_PORT							= 0;				

// [장애 알람정보]
CString			CConfig::SYSTEM_GROUP						= "";
UINT			CConfig::IOA_MODULE_ID						= 0;      

// [팩스 채널모니터링 정보]
CString			CConfig::FAX_CH_MONI_SHM_KEY				= "";				
int				CConfig::FAX_CH_MONI_SHM_POLLING_SLEEP		= 0;				
int				CConfig::FAX_CH_MONI_TOTAL_CH_CNT			= 0;				

// [팩스 송수신큐 정보]
int				CConfig::FAX_QUEUE_MONI_DB_POLLING_SLEEP	= 0;				

// [CDR 정보]
CString			CConfig::FAX_CDR_STG_HOME_PATH				= "";								
CString			CConfig::FAX_CDR_INBOUND_TIF_PATH			= "";															
CString			CConfig::FAX_CDR_FINISHED_TIF_PATH			= "";								
int				CConfig::FAX_CDR_DB_POLLING_SLEEP			= 0;								

// [LOG] : LOG 정보
CString			CConfig::LOG_PATH					= "";
bool			CConfig::LOG_FILE_SAVE				= true;
int				CConfig::LOG_LEVEL					= 0;
EnumLogFileSaveUnit	CConfig::LOG_SAVE_UNIT			= LOG_FILE_SAVE_UNIT_DAY;

// [LOG] : CDR 정보
CString			CConfig::CDR_PATH					= "";



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{
	//다른프로세스에서 FOD프로세스를 기동할때 문제가 발생(GetCurrentDirectory())
	/*char szVal[200];
	GetCurrentDirectory(sizeof(szVal), szVal);
	EXEC_PATH = CString(szVal) + "/";*/

	//SetCurrentDirectory(_T("D:\\Break") );

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

	CConfig::PROCESS_TYPE_STR = "IF" + CConfig::PROCESS_TYPE_STR;
	EXEC_PATH	= strPath	+ "/";	
	CDR_PATH	= EXEC_PATH + "Data/cdr/";
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
	// [SYSTEM] : DCM 시스템 (장비번호, 장비ID, 프로세스ID, 장비/프로세스 ID, 장비IP)
	// -------------------------------------------------------------------
	if( !_ReadConfig( "SYSTEM", "SYSTEM_TYPE", &SYSTEM_TYPE ) )
		return false;
	if( !_ReadConfig( "SYSTEM", "SYSTEM_NO", &SYSTEM_NO ) )
		return false;
	SYSTEM_ID.Format( "%s_%d", (LPCSTR)SYSTEM_TYPE, SYSTEM_NO );

	PROCESS_ID.Format( "%s_%d", ProcessType(PROCESS_TYPE), PROCESS_NO );
	SYSTEM_PROCESS_ID = SYSTEM_ID + "_" + PROCESS_ID;


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

	// [DSM 접속정보]
	if( !_ReadDbConfig( "DSM_IP",							&DSM_IP) )							return false;
	if( !_ReadDbConfig( "DSM_PORT",							&DSM_PORT) )						return false;

	// [장애알람 정보]	
	if( !_ReadDbConfig( "SYSTEM_GROUP",						&SYSTEM_GROUP) )					SYSTEM_GROUP		= "90000";
	if( !_ReadDbConfig( "IOA_MODULE_ID",					&IOA_MODULE_ID) )					IOA_MODULE_ID		= 589825;

	// [팩스 채널모니터링 정보]
	if( !_ReadDbConfig( "FAX_CH_MONI_TOTAL_CH_CNT",			&FAX_CH_MONI_TOTAL_CH_CNT) )		return false;
	if( !_ReadDbConfig( "FAX_CH_MONI_SHM_KEY",				&FAX_CH_MONI_SHM_KEY) )				return false;
	if( !_ReadDbConfig( "FAX_CH_MONI_SHM_POLLING_SLEEP",	&FAX_CH_MONI_SHM_POLLING_SLEEP))	return false;

	// [팩스 송수신큐 정보]
	if( !_ReadDbConfig( "FAX_QUEUE_MONI_DB_POLLING_SLEEP",	&FAX_QUEUE_MONI_DB_POLLING_SLEEP) )	return false;
	
	// [CDR 정보]
	if( !_ReadDbConfig( "STG_HOME_PATH",					&FAX_CDR_STG_HOME_PATH))			return false;
	if( !_ReadDbConfig( "INBOUND_TIF_PATH",					&FAX_CDR_INBOUND_TIF_PATH))			return false;
	if( !_ReadDbConfig( "FINISHED_TIF_PATH",				&FAX_CDR_FINISHED_TIF_PATH))		return false;
	if( !_ReadDbConfig( "FAX_CDR_DB_POLLING_SLEEP",			&FAX_CDR_DB_POLLING_SLEEP))			return false;
	if(FAX_CDR_DB_POLLING_SLEEP < 1) FAX_CDR_DB_POLLING_SLEEP = 10000;


	// -------------------------------------------------------------------
	// [LOG] : LOG 정보 (LOG 위치, LOG Level. Log File 저장 여부, Log File 저장 단위)
	// -------------------------------------------------------------------
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
        
	if( !_ReadDbConfig( "LOG_LEVEL",				&LOG_LEVEL) )				return false;
	if( !_ReadDbConfig( "LOG_FILE_SAVE",			&LOG_FILE_SAVE) )			return false;
	if( !_ReadDbConfig( "LOG_SAVE_UNIT",			&strTemp) )					return false;
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
	APPLOG->Print(DBGLV_INF0, " []           EXEC_PATH						[%s]", (LPCSTR) EXEC_PATH);
	APPLOG->Print(DBGLV_INF0, " []           CONFIG_FILE						[%s]", (LPCSTR) CONFIG_FILE);
	APPLOG->Print(DBGLV_INF0, " []           STATUS_FILE						[%s]", (LPCSTR) STATUS_FILE);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     SYSTEM_NO						[%d]", SYSTEM_NO);
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     SYSTEM_ID						[%s]", (LPCSTR) SYSTEM_ID);	
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     PROCESS_ID						[%s]", (LPCSTR) PROCESS_ID);
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     SYSPROCID						[%s]", (LPCSTR) SYSTEM_PROCESS_ID);
	APPLOG->Print(DBGLV_INF0, " [SYSTEM]     DB_CONNECTION_STRING			[%s]", (LPCSTR) DB_CONNECTION_STRING);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [FAX]        DSM_IP							[%s]", DSM_IP);
	APPLOG->Print(DBGLV_INF0, " [FAX]        DSM_PORT						[%d]", DSM_PORT);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_CH_MONI_TOTAL_CH_CNT		[%d]", FAX_CH_MONI_TOTAL_CH_CNT);
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_CH_MONI_SHM_KEY				[%s]", FAX_CH_MONI_SHM_KEY);
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_CH_MONI_SHM_POLLING_SLEEP	[%d]", FAX_CH_MONI_SHM_POLLING_SLEEP);	
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_QUEUE_MONI_DB_POLLING_SLEEP	[%d]", FAX_QUEUE_MONI_DB_POLLING_SLEEP);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_CDR_DB_POLLING_SLEEP		[%d]", FAX_CDR_DB_POLLING_SLEEP);
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_CDR_STG_HOME_PATH			[%s]", FAX_CDR_STG_HOME_PATH);
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_CDR_INBOUND_TIF_PATH		[%s]", FAX_CDR_INBOUND_TIF_PATH);
	APPLOG->Print(DBGLV_INF0, " [FAX]        FAX_CDR_FINISHED_TIF_PATH		[%s]", FAX_CDR_FINISHED_TIF_PATH);
	APPLOG->Print(DBGLV_INF0, " ------------------------------------------------------------------------------- ");
	APPLOG->Print(DBGLV_INF0, " [LOG]        LOG_PATH						[%s]", (LPCSTR) LOG_PATH);
	APPLOG->Print(DBGLV_INF0, " [LOG]        LOG_LEVEL						[%d]", LOG_LEVEL);
	APPLOG->Print(DBGLV_INF0, " [LOG]        LOG_FILE_SAVE					[%c]", (LOG_FILE_SAVE) ? 'Y' : 'N');
	APPLOG->Print(DBGLV_INF0, " [LOG]        LOG_SAVE_UNIT					[%d]", (int) LOG_SAVE_UNIT);
	APPLOG->Print(DBGLV_INF0, " =============================================================================== ");
}


