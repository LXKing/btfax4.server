#include "StdAfx.h"
#include "AlarmAPI.h"
#include "Config.h"

typedef void    (*LPF_SBUS_INIT)(UINT nSvrID, UINT nIOAModuleID, UINT nModuleID, LPCSTR lpszProcName);
typedef void    (*LPF_SBUS_INITLOG)(int nLogLevel, LPCSTR lpszLogPath);
typedef int     (*LPF_SBUS_START)(UINT nPort, RECV_CALLBACK rc);
typedef void    (*LPF_SBUS_STOP)();
typedef const char* (*LPF_SBUS_SENDALARM)(unsigned short nErrMoId, unsigned int nAlarmID, int AlarmType, int AlarmLevel, const char* szAlarmKey, const char* szAlarmMsg);
typedef BOOL    (*LPF_SBUS_ISALARM)(const char* szAlarmKey);
typedef BOOL    (*LPF_SBUS_ISALARMUUID)(const char* szAlarmUUID);

CAlarmAPI* CAlarmAPI::s_pInstance = NULL;
CAlarmAPI* CAlarmAPI::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new CAlarmAPI;

	return s_pInstance;
}

CAlarmAPI::CAlarmAPI(void)
{	
	m_hinstLib			= NULL;
	m_systemNo			= 0;
    m_systemGroupId		= 0;
    m_ioaModuleId		= 0;
    m_procId			= 0;
    m_ioaAlarmPort		= 0;
    m_ioaAlarmBaseCode	= 0;
    m_procName			= "";
    m_logLevel			= 0;
	m_logPath			= "";
}


CAlarmAPI::~CAlarmAPI(void)
{
	UnlinkLibrary();
}

//--------------------------------------------------------
// Title	: LinkLibrary
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : 라이브러리 링크
//--------------------------------------------------------
bool CAlarmAPI::LinkLibrary( LPCSTR lpszDllName )
{
    CString strPath;
    char    szCurrentPath[1024];

    //DWORD dwResult = GetCurrentDirectory(1024, szCurrentPath);
    //strPath.Format("%s\\%s", szCurrentPath, lpszDllName);
    //strPath.Format("%s\\%s", CConfig::EXEC_PATH, lpszDllName);
    strPath.Format(".\\%s", lpszDllName);
    
	m_hinstLib = LoadLibrary(strPath);
	if(m_hinstLib == NULL)
		return FALSE;
	
	return TRUE;
}

//--------------------------------------------------------
// Title	: UnlinkLibrary
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : 라이브러리 해제
//--------------------------------------------------------
void CAlarmAPI::UnlinkLibrary()
{
    FreeLibrary(m_hinstLib);
}

//--------------------------------------------------------
// Title	: Init
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : 알람API 초기화
//--------------------------------------------------------
bool CAlarmAPI::Init(UINT nSvrID
					, LPCSTR lpszServerGroup
					, UINT nIoaModuleId
					, UINT nIoaAlarmPort
					, UINT nIoaAlarmBaseCode
					, UINT nProcId
					, LPCSTR lpszProcName
					, int nLogLevel
					, LPCSTR lpszLogPath)
{
	if(!LinkLibrary("SBus64.dll"))
		return false;

	m_systemNo				= nSvrID;
    sscanf(lpszServerGroup, "%x", &m_systemGroupId);
    m_ioaModuleId			= 0x0000FFFF & nIoaModuleId;
    m_ioaAlarmPort			= nIoaAlarmPort;
	m_procId				= 0x0000FFFF & nProcId;    
    m_ioaAlarmBaseCode		= nIoaAlarmBaseCode;
    m_procName				= lpszProcName;
    m_logLevel				= nLogLevel;
	m_logPath				= lpszLogPath;
	   
    Init(m_systemGroupId, m_ioaModuleId, m_procId, m_procName);

    if (m_logLevel >= 0)
        InitLog(m_logLevel, m_logPath);

    return Start(m_ioaAlarmPort, NULL);

	//m_uuidMap.insert(pair<UINT,string>(1923, "David D"));
}


//--------------------------------------------------------
// Title	: SendFaultAlarm
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : 장애 알람 통보
//--------------------------------------------------------
void CAlarmAPI::SendFaultAlarm(UINT p_alarmId)
{
	char msg[1024];
	memset(msg, 0x00, sizeof(msg));
	sprintf(msg, "%d,%s",m_systemNo, m_procName);

    UINT swatErrCode = 0;
    ConvertToSwatErrCode(p_alarmId, swatErrCode);	
	
    SendAlarm(m_procId, swatErrCode, Fault, Minor, NULL, msg);
}

//--------------------------------------------------------
// Title	: SendFaultAlarm
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : 장애 알람 통보 (Critical)
//--------------------------------------------------------
void CAlarmAPI::SendCriticalAlarm(UINT p_alarmId, string szMsg)
{
	char paramMsg[1024];
	memset(paramMsg, 0x00, sizeof(paramMsg));
	if(szMsg.length() <= 0)
		sprintf(paramMsg, "%d,%s",m_systemNo, m_procName);
	else
		sprintf(paramMsg, "%d,%s,%s",m_systemNo, m_procName, szMsg.c_str());

	UINT swatErrCode = 0;
	ConvertToSwatErrCode(p_alarmId, swatErrCode);	
	if(ExistsUUID(swatErrCode))
		return;

	string uuid = GetUUID(p_alarmId);
	if (IsAlarmUUID(uuid.c_str()))
		return;
	
    uuid = SendAlarm(m_procId, swatErrCode, Alarm, Critical, NULL, paramMsg);

	if(strlen(uuid.c_str()) <= 0)
		return;

	m_uuidMap.insert(pair<UINT,string>(swatErrCode, uuid));
}


//--------------------------------------------------------
// Title	: SendMajorAlarm
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : 장애 알람 통보 (Major)
//--------------------------------------------------------
void CAlarmAPI::SendMajorAlarm(UINT p_alarmId, string szMsg)
{
	char paramMsg[1024];
	memset(paramMsg, 0x00, sizeof(paramMsg));
	if(szMsg.length() <= 0)
		sprintf(paramMsg, "%d,%s",m_systemNo, m_procName);
	else
		sprintf(paramMsg, "%d,%s,%s",m_systemNo, m_procName, szMsg.c_str());

	UINT swatErrCode = 0;
	ConvertToSwatErrCode(p_alarmId, swatErrCode);	
	if(ExistsUUID(swatErrCode))
		return;

	string uuid = GetUUID(p_alarmId);
	if (IsAlarmUUID(uuid.c_str()))
		return;
	
    uuid = SendAlarm(m_procId, swatErrCode, Alarm, Major, NULL, paramMsg);

	if(strlen(uuid.c_str()) <= 0)
		return;

	m_uuidMap.insert(pair<UINT,string>(swatErrCode, uuid));
}


//--------------------------------------------------------
// Title	: SendRecoveryAlarm
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : 장애 알람 통보 (Major)
//--------------------------------------------------------
void CAlarmAPI::SendRecoveryAlarm(UINT p_alarmId, string szMsg)
{
	char paramMsg[1024];
	memset(paramMsg, 0x00, sizeof(paramMsg));
	if(szMsg.length() <= 0)
		sprintf(paramMsg, "%d,%s",m_systemNo, m_procName);
	else
		sprintf(paramMsg, "%d,%s,%s",m_systemNo, m_procName, szMsg.c_str());

	UINT swatErrCode = 0;
	ConvertToSwatErrCode(p_alarmId, swatErrCode);	
	
    SendAlarm(m_procId, swatErrCode, Alarm, Normal, NULL, paramMsg);

	if(ExistsUUID(p_alarmId))
		return;

	m_uuidMap.erase(swatErrCode);
}


//--------------------------------------------------------
// Title	: ConvertToSwatErrCode
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : 알람코드를 SWAT 코드 체계로 변환
//--------------------------------------------------------
void CAlarmAPI::ConvertToSwatErrCode(int nAramId, UINT &nSwatErrCode)
{
	nSwatErrCode = m_ioaAlarmBaseCode + nAramId;
}

//--------------------------------------------------------
// Title	: ExistsUUID
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : UUID 존재 여부 체크
//--------------------------------------------------------
bool CAlarmAPI::ExistsUUID(UINT nAlarmId)
{
	UINT swatErrCode = 0;
	ConvertToSwatErrCode(nAlarmId, swatErrCode);

	map<UINT,string>::iterator itor;	
	if (m_uuidMap.find(swatErrCode) == m_uuidMap.end() )
		return false;
	else
		return true;
}

//--------------------------------------------------------
// Title	: GetUUID
// Writer	: KIMCG
// Date		: 2014.01.23
// Content  : UUID 얻음.
//--------------------------------------------------------
string CAlarmAPI::GetUUID(UINT nAlarmId)
{
	UINT swatErrCode = 0;
	ConvertToSwatErrCode(nAlarmId, swatErrCode);	
	map<UINT,string>::iterator itor;	
	if (m_uuidMap.find(swatErrCode) == m_uuidMap.end() )
		return "";
	
	string uuid = m_uuidMap[swatErrCode];
	return uuid;
}


bool CAlarmAPI::Init(UINT nSvrID, UINT nIOAModuleID, UINT nModuleID, LPCSTR lpszProcName)
{
	if(m_hinstLib == NULL)
		return false;

    LPF_SBUS_INIT   lpfnInit = (LPF_SBUS_INIT)GetProcAddress(m_hinstLib, "SBus_Init");
    if (!lpfnInit)
		return false;
    
    lpfnInit(nSvrID, nIOAModuleID, nModuleID, lpszProcName);
	return true;
}

void CAlarmAPI::InitLog( int nLogLevel, LPCSTR lpszLogPath )
{
    LPF_SBUS_INITLOG   lpfnInitLog = (LPF_SBUS_INITLOG)GetProcAddress(m_hinstLib, "SBus_InitLog");
    if (lpfnInitLog)
        return lpfnInitLog(nLogLevel, lpszLogPath);
}


int CAlarmAPI::Start(UINT nPort, RECV_CALLBACK rc)
{
    LPF_SBUS_START   lpfnStart = (LPF_SBUS_START)GetProcAddress(m_hinstLib, "SBus_Start");
    if (!lpfnStart)
        return 0;
    else
        return lpfnStart(nPort, rc);
}

void CAlarmAPI::Stop()
{
    LPF_SBUS_STOP   lpfnStop = (LPF_SBUS_STOP)GetProcAddress(m_hinstLib, "SBus_Stop");
    if (!lpfnStop)
		return;
    
    return lpfnStop();
}

const char* CAlarmAPI::SendAlarm( unsigned short nErrMoId, unsigned int nAlarmID, int AlarmType
                         , int AlarmLevel, const char* szAlarmKey, const char* szAlarmMsg )
{
	if(m_hinstLib == NULL)
		return 0;

    LPF_SBUS_SENDALARM   lpfnSendAlarm = (LPF_SBUS_SENDALARM)GetProcAddress(m_hinstLib, "SBus_SendAlarm");
    if (!lpfnSendAlarm)
        return 0;
    else
        return lpfnSendAlarm(nErrMoId, nAlarmID, AlarmType, AlarmLevel, szAlarmKey, szAlarmMsg);
}

bool CAlarmAPI::IsAlarm( const char* szAlarmKey )
{
	if(m_hinstLib == NULL)
		return false;

    LPF_SBUS_ISALARM   lpfnIsAlarm = (LPF_SBUS_ISALARM)GetProcAddress(m_hinstLib, "SBus_IsAlarm");
    if (!lpfnIsAlarm)
        return 0;    
    else
        return lpfnIsAlarm(szAlarmKey);
}

bool CAlarmAPI::IsAlarmUUID( const char* szAlarmUUID )
{
	if(m_hinstLib == NULL)
		return false;

    LPF_SBUS_ISALARMUUID   lpfnIsAlarmUUID = (LPF_SBUS_ISALARMUUID)GetProcAddress(m_hinstLib, "SBus_IsAlarmUUID");
    if (!lpfnIsAlarmUUID)
        return 0;
    else
        return lpfnIsAlarmUUID(szAlarmUUID);
}
