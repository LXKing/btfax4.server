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

typedef void (*Config_Display) ( const char* p_szKey, const char* p_szValue ); 

class CConfig  : public CConfigBase
{
public:
	CConfig();
	virtual ~CConfig();

	//static void	SetCallback( Config_Display p_pfnCallback );
	bool	LoadConfig_file();
	bool	LoadConfig_db();
	void	DisplayAllConfig();

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

	

	// [ FSM 연동 정보 ]
	static CString				FSM_IP;						// FSM 연동 IP
	static int					FSM_PORT;					// FSM 연동 Port
	static int					FSM_SND_ALIVE_TIME;			// FSM 연동 Keepalive를 송신하는 시간
	static int					FSM_CHK_ALIVE_TIME;			// FSM 연동 Keepalive를 수신하는 시간

	// [장애 알람정보]
	static CString				SYSTEM_GROUP;				// 서버그룹
    static UINT					IOA_MODULE_ID;				// 장애 알람 보낼 IOA 모듈 아이디

	// [ FAX 회선 정보 ]
	static int					FAX_START_CH;				// 시작 CH 번호
	static int					FAX_CH_CNT;					// CH 개수
	static CString				FAX_CH_MONI_SHM_KEY;				// 채널상태 공유메모리 주소			

	// [ RTP 정보 ]
	static int					RTP_BASE_PORT;
	static int					RTP_PORT_SPACE;

	// [ 경로 정보 ]
	static CString				LOCAL_TIF_PATH;
	static CString				STG_HOME_PATH;
	static CString				INBOUND_TIF_PATH;
	static CString				INBOUND_TIF_FULL_PATH;

    // [팩스번호 암호화 정보]
    static CString				ENCRYPT_FIELD_YN;					// 팩스번호가 암호화 된지에 대한 유무
    static CString				ENCRYPT_DLL_FILE;					// 외부 dll 위치 : 절대경로

	// [LOG] : LOG 정보
	static CString				LOG_PATH;
	static bool					LOG_FILE_SAVE;
	static int					LOG_LEVEL;
	static EnumLogFileSaveUnit	LOG_SAVE_UNIT;

protected:
	//bool _ReadConfig( const char* p_szSection, const char* p_szKey, CString* p_pstrValue );
	//bool _ReadConfig( const char* p_szSection, const char* p_szKey, int* p_pnValue );
	//bool _ReadConfig( const char* p_szSection, const char* p_szKey, bool* p_pbValue );

	//bool _ReadDbConfig( CDbModule& p_db, const char* p_szKey, CString* p_pstrValue );
	//bool _ReadDbConfig( CDbModule& p_db, const char* p_szKey, int* p_pnValue );
	//bool _ReadDbConfig( CDbModule& p_db, const char* p_szKey, bool* p_pbValue );
	//bool _ReadDbConfig( CDbModule& p_db, const char* p_szKey, CStringArray* p_psaValues );

protected:
	//static Config_Display	s_pfnDisplayFn;
};

#endif // !defined(AFX_SIP_FIDINI_H__6AB08EE3_2C7A_4DF3_B155_0A87085A4CED__INCLUDED_)
