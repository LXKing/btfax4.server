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
	
	// [DSM ��������]
	static CString				DSM_IP;												// DSM ���� ������
	static int					DSM_PORT;											// DSM ���� ��Ʈ

	// [��� �˶�����]
	static CString				SYSTEM_GROUP;										// �����׷�
    static UINT					IOA_MODULE_ID;										// ��� �˶� ���� IOA ��� ���̵�

	// [�ѽ� ä�θ���͸� ����]
	static CString				FAX_CH_MONI_SHM_KEY;								// ä�λ��� �����޸� �����ּ�
	static int					FAX_CH_MONI_SHM_POLLING_SLEEP;						// ä�θ���͸� �����޸� ���� �ֱ�
	static int					FAX_CH_MONI_TOTAL_CH_CNT;							// ä�ΰ���
	
	// [�ѽ� ť ����͸� ����]
	static int					FAX_QUEUE_MONI_DB_POLLING_SLEEP;					// �ѽ� ��,���� ť ����͸� DB ���� �ֱ�
	
	// [CDR ����͸� ����]
	static CString				FAX_CDR_STG_HOME_PATH;								// �ѽ� ���丮�� Ȩ���
	static CString				FAX_CDR_INBOUND_TIF_PATH;							// �ѽ� ���丮�� �ιٿ�� ���
	static CString				FAX_CDR_FINISHED_TIF_PATH;							// �ѽ� ���丮�� �ƿ��ٿ�� ���
	static int					FAX_CDR_DB_POLLING_SLEEP;							// CDR DB �����ֱ�
	static CString				CDR_PATH;											// CDR ������ġ
	
	// [ LOG ���� ]
	static CString				LOG_PATH;
	static bool					LOG_FILE_SAVE;
	static int					LOG_LEVEL;
	static EnumLogFileSaveUnit	LOG_SAVE_UNIT;

	// [ CDR ���� ]
	

protected:
	

protected:
	
};

#endif // !defined(AFX_SIP_FODINI_H__4837126B_A61D_44EE_B314_A8F77DF7A930__INCLUDED_)
