#pragma once

#include "ShmCtrl.h"
#include "DsmIfSession.h"

// iCore..
#include "iLib.h"
#include "iSocket.h"
#include "iThread.h"
#include "iMutex.h"
#include "iType.h"
// iCore

#define MAXCNT_RETRAY			(3)


#include <list>

using namespace std;
using namespace COMMON_LIB;

class CFaxChanelMonitoringThread : public iThread
{
public:
	CFaxChanelMonitoringThread(void);
	~CFaxChanelMonitoringThread(void);

	bool StartThread();
    bool StopThread();

	void LogInfo(char* p_msg);
	void LogWrn(char* p_msg);
	void LogErr(char* p_msg);
	
protected:
	virtual			NPVOID ThreadRun(NPVOID pArg);
	
// field
protected:
	CShmCtrl		m_shmCtrl;
	CString			m_logPrefix;

private :
	
};

