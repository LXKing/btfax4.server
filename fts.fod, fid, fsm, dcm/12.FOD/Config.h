// Config.h: interface for the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIP_FODINI_H__4837126B_A61D_44EE_B314_A8F77DF7A930__INCLUDED_)
#define AFX_SIP_FODINI_H__4837126B_A61D_44EE_B314_A8F77DF7A930__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "enum.h"
#include "Debug.h"
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
//		static CString				APP_VER;
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

	// [ DB 폴링 정보 ]
	static int					DB_POLLING_SLEEP;
	static int					DB_FETCH_CNT;

	// [ FSM 정보 ]
	static CString				FSM_IP;
	static int					FSM_PORT;
	static int					FSM_SND_ALIVE_TIME;
	static int					FSM_CHK_ALIVE_TIME;

	// [ 회선 정보 ]
	static int					FAX_START_CH;				// 시작 CH 번호
	static int					FAX_TOTAL_CH_CNT;			// 전체 채널 개수
	static int					FAX_BROAD_CH_CNT;			// 동보 채널 개수
	static int					FAX_CH_beg;					// 일반 채널 시작
	static int					FAX_CH_end;					// 일반 채널 끝
	static int					FAX_BCH_beg;				// 동보 채널 시작
	static int					FAX_BCH_end;				// 동보 채널 끝
	static CString				FAX_DEFAULT_ANI;
	static CString				FAX_OUTBOUND_PREFIX;
	static CString				FAX_CH_MONI_SHM_KEY;		// 채널상태 공유메모리 주소				

	// [ 재시도 정보 ]
	static int					TRY_CNT_DOWNLOAD;
	static int					TRY_DELAY_DOWNLOAD;
	static int					TRY_CNT_NOANSWER;
	static int					TRY_DELAY_NOANSWER;
	static int					TRY_CNT_BUSY;
	static int					TRY_DELAY_BUSY;

	// [ RTP 정보 ]
	static int					RTP_BASE_PORT;
	static int					RTP_PORT_SPACE;

	// [ 경로 정보 ]
	static CString				LOCAL_TIF_PATH;
	static CString				STG_HOME_PATH;
	static CString				FINISHED_TIF_PATH;
	static CString				FINISHED_TIF_FULL_PATH;

	// [장애 알람정보]
	static CString				SYSTEM_GROUP;				// 서버그룹
    static UINT					IOA_MODULE_ID;				// 장애 알람 보낼 IOA 모듈 아이디
	
	// [오버레이정보]
	static CString				OVERLAY_YN;					// 오버레이사용유무
	static UINT					OVERLAY_ALIGN;				// 오버레이 정렬 ( Center : 0, Left : 1, Right : 2)
	static UINT					OVERLAY_X_OFFSET;			// 오버레이 x좌표 
	static UINT					OVERLAY_Y_OFFSET;			// 오버레이 y좌표
        
    // [팩스번호 암호화 정보]
    static CString				ENCRYPT_FIELD_YN;					// 팩스번호가 암호화 된지에 대한 유무
    static CString				ENCRYPT_DLL_FILE;					// 외부 dll 위치 : 절대경로
    
	// [ LOG 정보 ]
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

#endif // !defined(AFX_SIP_FODINI_H__4837126B_A61D_44EE_B314_A8F77DF7A930__INCLUDED_)
