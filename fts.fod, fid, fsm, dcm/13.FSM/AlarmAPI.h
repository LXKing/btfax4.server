#pragma once
#include <string.h>
#include <iostream>
#include <map>
#include <utility>

using namespace std;

typedef int 	(*RECV_CALLBACK)(unsigned char *, int);

typedef enum AlamType
{
	None        = 0,
    Information = 1,
    Fault       = 2,
    Alarm       = 3
};

typedef enum AlamLevel
{
	Normal      = 0,
    Minor       = 1,
    Major       = 2,
    Critical    = 3,
};

class CAlarmAPI
{
public:
	static CAlarmAPI* Inst();
protected:
	static CAlarmAPI* s_pInstance;

public:
	CAlarmAPI(void);
	~CAlarmAPI(void);

protected:
	bool LinkLibrary(LPCSTR lpszDllName);
	void UnlinkLibrary();
	bool Init(UINT nSvrID, UINT nIOAModuleID, UINT nModuleID, LPCSTR lpszProcName);
	void ConvertToSwatErrCode(int nAramId, UINT &nSwatErrCode);
    void InitLog(int nLogLevel, LPCSTR lpszLogPath);
    int Start(UINT nPort, RECV_CALLBACK rc);
    void Stop();
    bool IsAlarm(const char* szAlarmKey);
    bool IsAlarmUUID(const char* szAlarmUUID);
	const char* SendAlarm(unsigned short nErrMoId
						, unsigned int nAlarmID
						, int AlarmType
						, int AlarmLevel
						, const char* szAlarmKey
						, const char* szAlarmMsg);
	
public :
	bool Init(UINT nSvrID
				, LPCSTR lpszServerGroup
				, UINT nIoaModuleId
				, UINT nIoaAlarmPort
				, UINT nIoaAlarmBaseCode
				, UINT nProcId
				, LPCSTR lpszProcName
				, int nLogLevel
				, LPCSTR lpszLogPath);


	void SendFaultAlarm(UINT p_alarmId);
	void SendCriticalAlarm(UINT p_alarmId, string szMsg);
	void SendMajorAlarm(UINT p_alarmId, string szMsg);
	void SendRecoveryAlarm(UINT p_alarmId, string szMsg);

	bool ExistsUUID(UINT nAlarmId);
	string GetUUID(UINT nSwatAlarmId);

private :
	HMODULE		m_hinstLib;
	UINT		m_systemNo;
    UINT		m_systemGroupId;
    UINT		m_ioaModuleId;
    UINT		m_procId;
    UINT		m_ioaAlarmPort;
    UINT		m_ioaAlarmBaseCode;
    LPCSTR		m_procName;
    int			m_logLevel;
	LPCSTR		m_logPath;
	
	map<UINT, string> m_uuidMap;
};

