// Config.h: interface for the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIP_FIDINI_H__6AB08EE3_2C7A_4DF3_B155_0A87085A4CED__INCLUDED_)
#define AFX_SIP_FIDINI_H__6AB08EE3_2C7A_4DF3_B155_0A87085A4CED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "enum.h"
#include "DbModule.h"
#include "ConfigBase.h"

using namespace COMMON_LIB;


#define MGW_EVENTTYPE	"INTERFACE"
#define MGW_EVENT		"MGW"
#define MGW_UP			"UP"
#define MGW_DOWN		"DOWN"



typedef void (*Config_Display) ( const char* p_szKey, const char* p_szValue ); 

#define BTSIP_LOGLEVEL(x)		\
	!(stricmp(x, "OFF")) ? BTLogFile::LogLevelNone : 	\
	!(stricmp(x, "INF")) ? BTLogFile::LogLevelInfo : 	\
	!(stricmp(x, "DBG")) ? BTLogFile::LogLevelDebug:	\
	!(stricmp(x, "TRC")) ? BTLogFile::LogLevelTrace:    \
    !(stricmp(x, "ERR")) ? BTLogFile::LogLevelError:    \
    !(stricmp(x, "WRN")) ? BTLogFile::LogLevelWarning:  \
    !(stricmp(x, "TEST")) ? BTLogFile::LogLevelTest:    \
    !(stricmp(x, "UD1")) ? BTLogFile::LogLevelUserDef1: \
    !(stricmp(x, "UD2")) ? BTLogFile::LogLevelUserDef2: \
    !(stricmp(x, "UD3")) ? BTLogFile::LogLevelUserDef3: \
    !(stricmp(x, "UD4")) ? BTLogFile::LogLevelUserDef4: \
    !(stricmp(x, "ALL")) ? BTLogFile::LogLevelAll:      \
    !(stricmp(x, "LV1")) ? (BTLogFile::LogLevelInfo  | BTLogFile::LogLevelDebug | BTLogFile::LogLevelTrace | BTLogFile::LogLevelError | BTLogFile::LogLevelWarning) : \
    !(stricmp(x, "LV2")) ? (BTLogFile::LogLevelTrace | BTLogFile::LogLevelError | BTLogFile::LogLevelWarning) : \
    !(stricmp(x, "LV3")) ? (BTLogFile::LogLevelError | BTLogFile::LogLevelWarning) : \
    BTLogFile::LogLevelAll



class CConfig  : public CConfigBase
{
public:
	CConfig();
	virtual ~CConfig();

	//void		SetCallback( Config_Display p_pfnCallback );

	bool		LoadConfig_file();
	bool		LoadConfig_db();
	

public:
	// [ 기본 정보 ]
	//static CString				EXEC_PATH;
	//static CString				CONFIG_FILE;
	//static CString				STATUS_FILE;

public:
	// [ 공통 정보 ]
	//static CString				APP_VER;
 //   static CString				SYSTEM_TYPE;
	//static int					SYSTEM_NO;
 //   static CString				SYSTEM_ID;
	//static CString				SYSTEM_IP;
 //   static P_TYPE				PROCESS_TYPE;
	//static CString				PROCESS_TYPE_STR;
 //   static int					PROCESS_NO;
 //   static CString				PROCESS_ID;
 //   static CString				SYSTEM_PROCESS_ID;
 //   static CString				DB_CONNECTION_STRING;


	static int			SYSTEM_MAX_DIALOGS;
	static int			SYSTEM_MAX_REGISTERS;
	static int			SYSTEM_MAX_TRANSCS;
	static int			SYSTEM_TIMER_POOL_CNT;
	static int			SYSTEM_DISPATCHER_CNT;
	static int			SYSTEM_TIMERMGR_CNT;
	static int			SYSTEM_EVENTMGR_CNT;

	static CString		SYSTEM_LOG_PATH;
	static int			SYSTEM_LOG_LEVEL;
	static int			SYSTEM_BTSIP_LOG_LEVEL;
	

	static bool			ADDRESS_FSM_CROSS;
	static CString		ADDRESS_LOCAL_IP;
	static int			ADDRESS_LOCAL_PORT;
	static int			ADDRESS_MGW_ALIVE_SND_TIME;
	static int			ADDRESS_MGW_ALIVE_CHK_TIME;
	static int			ADDRESS_RTP_PORT_BASE;
	static int			ADDRESS_CLIENT_LISTEN_PORT;
	static int			ADDRESS_CLIENT_SESSION_BEGIN;
	static int			ADDRESS_CLIENT_SESSION_END;
	static int			ADDRESS_CLIENT_ALIVE_CHK_TIME;
	
	static CString		USER_LOCAL_URI;
	static CString		USER_LOCAL_NAME;
	static CStringArray	USER_REDIRECT_URI;
	
	static int			TIMER_1;
	static int			TIMER_2;
	static int			TIMER_4;
	static int			TIMER_D;
	static int			TIMER_P;

	static CString		SERVICE_REGIST_YN;
	static int			SERVICE_PERF_CDT;
	static int			SERVICE_ALERT_EXPIRES;
	static int			SERVICE_SESSION_EXPIRES;
	static int			SERVICE_MIN_SE;
	static int			SERVICE_EXPIRES;
	static int			SERVICE_STATUS;
	static int			SERVICE_DELAY;
	static int			SERVICE_TCP_WAIT_TIME;
	static int			SERVICE_UDP_MTU_SIZE;
	static CString		SERVICE_TRANSPORT_TYPE;
	static int			SERVICE_TRANSPORT;
	static int			SERVICE_CALLING_TIMEOUT;
	

	static int			SERVICE_CHECK_METHOD;
	static CStringArray	SERVICE_SERVE_METHOD;
	static int			SERVICE_CHECK_SUPPORTED;
	static CStringArray	SERVICE_SERVE_SUPPORTED;

	static bool			SERVICE_AUTO_RESPONSE;
	static bool			SERVICE_AUTO_ACK;
	static bool			SERVICE_AUTO_PRACK;
	static bool			SERVICE_AUTO_TRYING;
	static bool			SERVICE_AUTO_RINGING;
	static bool			SERVICE_AUTO_OFFER;
	static bool			SERVICE_AUTO_ANSWER;

    // ADD - KIMCG : 20150904
    // LOOOP(내부순환) 팩스 번호 추가
    static CString		SERVICE_LOOP_FAXNO;
    // ADD - END


	// [장애 알람정보]
	static CString				SYSTEM_GROUP;										// 서버그룹
    static UINT					IOA_MODULE_ID;										// 장애 알람 보낼 IOA 모듈 아이디

protected:
	//bool _ReadConfig( const char* p_szSection, const char* p_szKey, CString* p_pstrValue );
	//bool _ReadConfig( const char* p_szSection, const char* p_szKey, int* p_pnValue );
	//bool _ReadConfig( const char* p_szSection, const char* p_szKey, bool* p_pbValue );

	//bool _ReadDbConfig( CDbModule& p_db, const char* p_szKey, CString* p_pstrValue );
	//bool _ReadDbConfig( CDbModule& p_db, const char* p_szKey, int* p_pnValue );
	//bool _ReadDbConfig( CDbModule& p_db, const char* p_szKey, bool* p_pbValue );
	//bool _ReadDbConfig( CDbModule& p_db, const char* p_szKey, CStringArray* p_psaValues );

protected:
	//Config_Display		m_pfnDisplayFn;

};

#endif // !defined(AFX_SIP_FIDINI_H__6AB08EE3_2C7A_4DF3_B155_0A87085A4CED__INCLUDED_)
