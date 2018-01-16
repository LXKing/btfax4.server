#include <windows.h> 
#include <winsock.h> 
#include "StdAfx.h"
#include "Config.h"
#include "APP.h"
#include "DcmDataSet.h"
#include "DcmDataTable.h"

#include "FaxChanelMonitoringThread.h"

// constructor && destructor
CFaxChanelMonitoringThread::CFaxChanelMonitoringThread(void)
{ 
}
CFaxChanelMonitoringThread::~CFaxChanelMonitoringThread(void)
{ 
}


// implementation
bool CFaxChanelMonitoringThread::StartThread()
{	
	Start();

	return true;
}

bool CFaxChanelMonitoringThread::StopThread()
{	
	return false;
}

// override
NPVOID CFaxChanelMonitoringThread::ThreadRun(NPVOID pArg)
{	
	LogInfo("Fax channel monitoring thread start.");
	CString			msg;
	SHM_CH_MONI_DATA* shmData	= NULL;
	CDcmDataTable* chTable		= NULL;
	CDcmDataRow* row			= NULL;
	while(IsOn())
	{
		iSleep(CConfig::FAX_CH_MONI_SHM_POLLING_SLEEP);
		if(!IsRunning())
			break;
		
		if(!CDsmIfSession::Inst()->IsRunning())
		{	
			LogErr("DsmIfSession is NULL or not running.");
			continue;
		}

		// DSM 연결상태 확인.
		if(!CDsmIfSession::Inst()->IsConnected())
			continue;

		// 세션키 확인
		if(CDsmIfSession::Inst()->GetSessionKey() <= 0)
			continue;

		// 실시간 데이터 전송상태 확인.
		PacketCommand cmd = CDsmIfSession::Inst()->GetPacketCommand();
		if(cmd != CMD_EMS_DCM_RT_MONIDATA)
			continue;

		// 공유메모리 Open
		if(!m_shmCtrl.OpenOrCreateShm(CConfig::FAX_CH_MONI_SHM_KEY))
		{	
			LogErr("Open Or Create Shared memory fail.");
			continue;
		}
		
		shmData = m_shmCtrl.GetShmData(0);
		if(shmData == NULL)
		{
			LogErr("Shared memory data is null.");			
			continue;
		}

		chTable = CDsmIfSession::Inst()->GetDataTable("DS_FX_CHNL_MONITOR");
		if(chTable == NULL)
		{	
			LogErr("Invalid data table. name:DS_FX_CHNL_MONITOR");						
			continue;
		}

		if(!chTable->m_registed)
			LogWrn("Data table is not registed. name:DS_FX_CHNL_MONITOR");			

		for(int i = 0; i < CConfig::FAX_CH_MONI_TOTAL_CH_CNT; i++)
		{		
			try
			{
				/*if(shmData->use_flag <= 0 || shmData->chg_flag <= 0 )
					continue;*/

				if(shmData->use_flag <= 0 )
					continue;

				row = chTable->NewRow();
				row->m_isBulkRow = false;
				
				//row->SetValueInt("use_flag",		shmData[i].use_flag);
				row->SetValueInt("SYSTEM_ID"	, shmData->system_id				);
				row->SetValueInt("CHNL_NO"		, shmData->channel					);
				row->SetValueInt("FOD_ID"		, shmData->module_id				);
				row->SetValueInt("DIRECTION"	, shmData->call_diretion			);
				row->SetValueInt("CHNL_STATUS"	, shmData->channel_state			);
				row->SetValueStr("ANI"			, shmData->ani						);
				row->SetValueStr("DNIS"			, shmData->dnis						);
				row->SetValueStr("REQST_TIME"	, shmData->call_connected_time		);
				row->SetValueStr("START_TIME"	, shmData->call_incoming_time		);
				row->SetValueStr("FINSH_TIME"	, shmData->call_disconnected_time	);
				row->SetValueStr("SERVICE_ID"	, shmData->service_id				);
				row->SetValueStr("SERVICE_NAME"	, shmData->service_name				);
				row->SetValueInt("SUM_SEND_CNT"	, shmData->outbound_cnt				);
				row->SetValueInt("SUM_RECV_CNT"	, shmData->inbound_cnt				);
				row->SetValueInt("CHANGE_FLAG"	, shmData->chg_flag					);				
				chTable->AddRow(row);

				/*APPLOG->Print(DBGLV_INF0, "channel:%d, call_direction:%d, channel_state:%d, service_id:%s(%s), use_flag:%d, chg_flag:%d"
										, shmData->channel
										, shmData->call_diretion
										, shmData->channel_state
										, shmData->service_id
										, shmData->service_name
										, shmData->use_flag
										, shmData->chg_flag
										);*/
				shmData->chg_flag = 0;
				shmData++;
			}
			catch(DWORD ex)
			{	
				break;
			}
		}
	}

	return 0;
}



//--------------------------------------------------------
// Title	: LogInfo
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 메시지 로그
//--------------------------------------------------------
void CFaxChanelMonitoringThread::LogInfo(char* p_msg)
{	
	try
	{	
		APPLOG->Print(DBGLV_INF0, "[%s:%d(%d)] %s"						
						, CDsmIfSession::Inst()->GetIpAddress().c_str()
						, CDsmIfSession::Inst()->GetPort()
						, CDsmIfSession::Inst()->GetSessionKey()						
						, p_msg
						);
	}
	catch(DWORD e)
	{
		return;
	}
}


//--------------------------------------------------------
// Title	: LogWrn
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 오류 로그
//--------------------------------------------------------
void CFaxChanelMonitoringThread::LogWrn(char* p_msg)
{	
	try
	{
		APPLOG->Print(DBGLV_WRN, "[%s:%d(%d)] %s"						
						, CDsmIfSession::Inst()->GetIpAddress().c_str()
						, CDsmIfSession::Inst()->GetPort()
						, CDsmIfSession::Inst()->GetSessionKey()						
						, p_msg
						);		
	}
	catch(DWORD e)
	{
		return;
	}
}

//--------------------------------------------------------
// Title	: LogErr
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 오류 로그
//--------------------------------------------------------
void CFaxChanelMonitoringThread::LogErr(char* p_msg)
{
	try
	{
		APPLOG->Print(DBGLV_ERR, "[%s:%d(%d)] %s"						
						, CDsmIfSession::Inst()->GetIpAddress().c_str()
						, CDsmIfSession::Inst()->GetPort()
						, CDsmIfSession::Inst()->GetSessionKey()
						, p_msg
						);
	}
	catch(DWORD e)
	{
		return;
	}
}
