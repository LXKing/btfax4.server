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
	
public:
	
	// [DSM 접속정보]
	static CString				DSM_IP;												// DSM 접속 아이피
	static int					DSM_PORT;											// DSM 접속 포트

	// [장애 알람정보]
	static CString				SYSTEM_GROUP;										// 서버그룹
    static UINT					IOA_MODULE_ID;										// 장애 알람 보낼 IOA 모듈 아이디

	// [팩스 채널모니터링 정보]
	static CString				FAX_CH_MONI_SHM_KEY;								// 채널상태 공유메모리 접근주소
	static int					FAX_CH_MONI_SHM_POLLING_SLEEP;						// 채널모니터링 공유메모리 폴링 주기
	static int					FAX_CH_MONI_TOTAL_CH_CNT;							// 채널개수
	
	// [팩스 큐 모니터링 정보]
	static int					FAX_QUEUE_MONI_DB_POLLING_SLEEP;					// 팩스 송,수신 큐 모니터링 DB 폴링 주기
	
	// [CDR 모니터링 정보]
	static CString				FAX_CDR_STG_HOME_PATH;								// 팩스 스토리지 홈경로
	static CString				FAX_CDR_INBOUND_TIF_PATH;							// 팩스 스토리지 인바운드 경로
	static CString				FAX_CDR_FINISHED_TIF_PATH;							// 팩스 스토리지 아웃바운드 경로
	static int					FAX_CDR_DB_POLLING_SLEEP;							// CDR DB 폴링주기
	static CString				CDR_PATH;											// CDR 생성위치
	
	// [ LOG 정보 ]
	static CString				LOG_PATH;
	static bool					LOG_FILE_SAVE;
	static int					LOG_LEVEL;
	static EnumLogFileSaveUnit	LOG_SAVE_UNIT;

	// [ CDR 정보 ]
	

protected:
	

protected:
	
};

#endif // !defined(AFX_SIP_FODINI_H__4837126B_A61D_44EE_B314_A8F77DF7A930__INCLUDED_)
