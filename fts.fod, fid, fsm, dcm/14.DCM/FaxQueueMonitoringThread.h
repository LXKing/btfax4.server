#pragma once

#define		SEND_CDR  0X01
#define		RECV_CDR  0X02

#include "DbModule.h"

// iCore..
#include "iLib.h"
#include "iSocket.h"
#include "iThread.h"
#include "iMutex.h"
#include "iType.h"
// iCore
#include "DsmIfSession.h"

#include <list>
using namespace std;

class CFaxQueueMonitoringThread : public iThread
{

public:
	CFaxQueueMonitoringThread(void);
	~CFaxQueueMonitoringThread(void);
	
	bool StartThread();
    bool StopThread();

	//void  MakeSendCdrData(CString& p_cdrData, CDbModule::FAX_SEND_CDR p_faxCdr);
	//void  MakeRecvCdrData(CString& p_cdrData, CDbModule::FAX_RECV_CDR p_faxCdr);
	
	void LogInfo(char* p_msg);
	void LogWrn(char* p_msg);
	void LogErr(char* p_msg);

protected:
	virtual			NPVOID ThreadRun(NPVOID pArg);
	CString			m_logPrefix;
};

