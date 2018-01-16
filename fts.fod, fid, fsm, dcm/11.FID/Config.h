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
	// [ �⺻ ���� ]
	//static CString				EXEC_PATH;
	//static CString				CONFIG_FILE;
	//static CString				STATUS_FILE;

public:
	// [ ���� ���� ]
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

	

	// [ FSM ���� ���� ]
	static CString				FSM_IP;						// FSM ���� IP
	static int					FSM_PORT;					// FSM ���� Port
	static int					FSM_SND_ALIVE_TIME;			// FSM ���� Keepalive�� �۽��ϴ� �ð�
	static int					FSM_CHK_ALIVE_TIME;			// FSM ���� Keepalive�� �����ϴ� �ð�

	// [��� �˶�����]
	static CString				SYSTEM_GROUP;				// �����׷�
    static UINT					IOA_MODULE_ID;				// ��� �˶� ���� IOA ��� ���̵�

	// [ FAX ȸ�� ���� ]
	static int					FAX_START_CH;				// ���� CH ��ȣ
	static int					FAX_CH_CNT;					// CH ����
	static CString				FAX_CH_MONI_SHM_KEY;				// ä�λ��� �����޸� �ּ�			

	// [ RTP ���� ]
	static int					RTP_BASE_PORT;
	static int					RTP_PORT_SPACE;

	// [ ��� ���� ]
	static CString				LOCAL_TIF_PATH;
	static CString				STG_HOME_PATH;
	static CString				INBOUND_TIF_PATH;
	static CString				INBOUND_TIF_FULL_PATH;

    // [�ѽ���ȣ ��ȣȭ ����]
    static CString				ENCRYPT_FIELD_YN;					// �ѽ���ȣ�� ��ȣȭ ������ ���� ����
    static CString				ENCRYPT_DLL_FILE;					// �ܺ� dll ��ġ : ������

	// [LOG] : LOG ����
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
