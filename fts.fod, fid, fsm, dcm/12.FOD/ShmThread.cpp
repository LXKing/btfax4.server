#include "stdafx.h"
#include "APP.h"
#include "ShmThread.h"
#include "Config.h"
#include "enum.h"

using namespace COMMON_LIB;

CShmThread* CShmThread::s_pInstance = NULL;

CShmThread* CShmThread::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new CShmThread;

	return s_pInstance;
}

CShmThread::CShmThread(void)
{
	CoInitialize(NULL);
}


CShmThread::~CShmThread(void)
{
	CoUninitialize();
}


void CShmThread::onThreadEntry()
{
	// �����Ͻ� ����
	time_t timer;
	timer = time(NULL) + (24 * 60 * 60);
	m_currTime = localtime(&timer);

	while( !IsReqStop() )
	{	
		// ���ں���� �� ä�κ� �߼�Ƚ�� �ʱ�ȭ.
		if(IsChangedDate())
		{
			if(!g_shmCtrl.OpenOrCreateShm(CConfig::FAX_CH_MONI_SHM_KEY))
			{
				APPLOG->Print(DBGLV_ERR, "[FAIL] %s", g_shmCtrl.GetErrMsg());
				continue;
			}
			SHM_CH_MONI_DATA* shmData = g_shmCtrl.GetShmData(CConfig::FAX_START_CH);
			if(shmData == NULL)
			{
				APPLOG->Print(DBGLV_ERR, "[FAIL] %s", g_shmCtrl.GetErrMsg());
				continue;
			}

			for(int i = 0; i < CConfig::FAX_TOTAL_CH_CNT; i++)
			{	
				shmData->outbound_cnt	= 0;
				shmData->chg_flag		= 1;
				
				shmData++;
			}
		}

		Sleep(1000 * 60);
	}
}

bool CShmThread::IsChangedDate()
{
	time_t timer;
	struct tm *t;
	int curDay = m_currTime->tm_mday;

	timer = time(NULL) + (24 * 60 * 60);
	t = localtime(&timer);

	if(t->tm_mday != curDay)
	{
		m_currTime = t;
		return true;
	}

	return false;
}