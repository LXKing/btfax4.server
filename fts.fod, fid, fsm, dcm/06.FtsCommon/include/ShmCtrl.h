#pragma once

#include <afxmt.h>
#include<string.h>
#include "enum.h"
using namespace COMMON_LIB;

#define MAX_SIZE	480

struct SHM_CH_MONI_DATA
{
	int use_flag;
	int channel;
	int call_diretion;
	int channel_state;
	int outbound_cnt;
	int inbound_cnt;
	int chg_flag;
	int system_id;
	int module_id;

	char ani[50];
	char dnis[50];
	char service_id[20];
	char service_name[50];

	char call_connected_time[20];
	char call_incoming_time[20];
	char call_disconnected_time[20];
		
	void Clear()
	{
		use_flag				= -1;
		channel					= -1;
		call_diretion			= 0;
		channel_state			= -1;
		
		chg_flag				= 0;
		system_id				= 0;
		module_id				= 0;
	
		memset(ani,				0x00, sizeof(ani));
		memset(dnis,			0x00, sizeof(dnis));		
		memset(service_id,		0x00, sizeof(service_id));
		memset(service_name,	0x00, sizeof(service_name));
		memset(call_connected_time		,	0x00, sizeof(call_connected_time));
		memset(call_incoming_time		,	0x00, sizeof(call_incoming_time));
		memset(call_disconnected_time	,	0x00, sizeof(call_disconnected_time));
	
		outbound_cnt = 0;
		inbound_cnt = 0;
	}
};

class CShmCtrl
{
public:
	CShmCtrl(void);
	~CShmCtrl(void);

	bool OpenOrCreateShm(const char* p_pKey);
	bool CreateShm(const char* p_pKey);	
	bool OpenShm(const char* p_pKey);
	void SetErrMsg(const char* pFuncName, const char* pErrMsg);
    void ReleaseShm();
    void Clear();
    SHM_CH_MONI_DATA* GetShmData(int p_nIdx);
	void GetShmData(int p_nIdx, SHM_CH_MONI_DATA* p_pShmData);
	CString GetErrMsg();
	

private:
	CString				m_strKey;
	HANDLE				m_hMapFile;
	PVOID				m_pView;
	CString				m_strErrMsg;
	SHM_CH_MONI_DATA	m_pShmDatas[MAX_SIZE];
};

