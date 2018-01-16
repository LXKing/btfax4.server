#include <windows.h> 
#include "StdAfx.h"
#include "APP.h"
#include "FaxQueueMonitoringThread.h"
#include "Config.h"
#include "enum.h"
#include "DcmDataSet.h"
#include "DcmDataTable.h"

using namespace COMMON_LIB;

CFaxQueueMonitoringThread::CFaxQueueMonitoringThread(void)
{	
}


CFaxQueueMonitoringThread::~CFaxQueueMonitoringThread(void)
{	
}

// implementation
bool CFaxQueueMonitoringThread::StartThread()
{	
	Start();
	return true;
}

bool CFaxQueueMonitoringThread::StopThread()
{	
	Stop(true);
	return true;
}

// override
NPVOID CFaxQueueMonitoringThread::ThreadRun(NPVOID pArg)
{
	LogInfo("Fax queue monitoring thread start.");

	int				dbRet;							// DB 접속 결과
	CString			connStr, id, pwd;				// DB connection 정보
	CString			msg;
	char			strCmd[35];
	CDbModule::FAX_Q_ITEM faxQueueItem;
	CDcmDataTable* table_send	= NULL;
	CDcmDataTable* table_recv	= NULL;
	CDcmDataRow *row			= NULL;
	CDcmDataRow *row2			= NULL;
	while(IsOn())
	{
		iSleep(CConfig::FAX_QUEUE_MONI_DB_POLLING_SLEEP);
		if(!IsRunning())
			break;
		
		// Clear memory
		faxQueueItem.clear();
	
		// DSM 연결상태 확인.
		if(!CDsmIfSession::Inst()->IsConnected())
			continue;

		// 세션키 확인
		if(CDsmIfSession::Inst()->GetSessionKey() <= 0)
			continue;

		// 실시간 데이터 전송상태 일때만 데이터 추가.
		PacketCommand cmd = CDsmIfSession::Inst()->GetPacketCommand();
		if(cmd != CMD_EMS_DCM_RT_MONIDATA)
			continue;
		
		faxQueueItem.send_queue.SYSTEM_ID	= CConfig::SYSTEM_NO;
		faxQueueItem.send_queue.SYSTEM_NAME = CConfig::SYSTEM_TYPE;
		faxQueueItem.send_queue.SENDQ_ID	= "BTF_FAX_SEND_MSTR";
		faxQueueItem.send_queue.SENDQ_NAME	= "팩스송신큐";

		faxQueueItem.recv_queue.SYSTEM_ID	= CConfig::SYSTEM_NO;
		faxQueueItem.recv_queue.SYSTEM_NAME = CConfig::SYSTEM_TYPE;
		faxQueueItem.recv_queue.RECVQ_ID	= "BTF_FAX_RECV_MSTR";
		faxQueueItem.recv_queue.RECVQ_NAME	= "팩스수신큐";
		
		// Get Send queue record from Db
		dbRet = CDbModule::Inst()->SelectFaxQueue(faxQueueItem);
		if(dbRet < 0 ) 
		{	
			APPLOG->Print( DBGLV_ERR, "PKG_PRC_DCM.USP_SELECT_SEND_RECV_QUEUE! F_DB_ERROR" );
			continue;
		}

		table_send = CDsmIfSession::Inst()->GetDataTable("DS_FX_SEND_Q_MONITOR");
		if(table_send != NULL && table_send->m_registed)
		{	
			row = table_send->NewRow();
			row->m_isBulkRow = false;
			row->SetValueInt("SYSTEM_ID"			, CConfig::SYSTEM_NO);
			row->SetValueStr("SENDQ_ID"				, LPSTR(LPCTSTR(faxQueueItem.send_queue.SENDQ_ID)));
			row->SetValueStr("SYSTEM_NAME"			, LPSTR(LPCTSTR(CConfig::SYSTEM_TYPE)));
			row->SetValueStr("SENDQ_NAME"			, LPSTR(LPCTSTR(faxQueueItem.send_queue.SENDQ_NAME)));
			row->SetValueInt("SUM_COMPLETE_CNT"		, faxQueueItem.send_queue.completed_send_total);
			row->SetValueInt("SUM_SUCC_CNT"			, faxQueueItem.send_queue.send_sucess_total);
			row->SetValueInt("SUM_FAIL_CNT"			, faxQueueItem.send_queue.send_fail_total);
			row->SetValueInt("RTS_PROCESSING_CNT"	, faxQueueItem.send_queue.wait_processing_total);
			row->SetValueInt("RTS_TIFF_WAITCNT"		, faxQueueItem.send_queue.wait_make_tiff_total);
			row->SetValueInt("RTS_SEND_WAITCNT"		, faxQueueItem.send_queue.wait_send_total);
			row->SetValueInt("RTS_SENDING_CNT"		, faxQueueItem.send_queue.sending_total);
			row->SetValueInt("RTS_SENDING_PAGECNT"	, faxQueueItem.send_queue.sending_tiff_page_count_total);
			row->SetValueInt("MAX_SEND_TRYCNT"		, faxQueueItem.send_queue.MAX_SEND_TRYCNT);
			row->SetValueInt("MAX_TIFF_PAGECNT"		, faxQueueItem.send_queue.MAX_TIFF_PAGECNT);
			row->SetValueInt("MAX_TIFF_FILESIZE"	, faxQueueItem.send_queue.MAX_TIFF_FILESIZE);
			row->SetValueInt("MAX_TIFF_MAKETIME"	, faxQueueItem.send_queue.MAX_TIFF_MAKETIME);
			row->SetValueInt("MAX_WAIT_TIME"		, faxQueueItem.send_queue.MAX_WAIT_TIME);
			row->SetValueInt("MAX_SENDING_TIME"		, faxQueueItem.send_queue.MAX_SENDING_TIME);
			row->SetValueInt("AVG_SEND_TRYCNT"		, faxQueueItem.send_queue.AVG_SEND_TRYCNT);
			row->SetValueInt("AVG_TIFF_PAGECNT"		, faxQueueItem.send_queue.AVG_TIFF_PAGECNT);
			row->SetValueInt("AVG_TIFF_FILESIZE"	, faxQueueItem.send_queue.AVG_TIFF_FILESIZE);
			row->SetValueInt("AVG_TIFF_MAKETIME"	, faxQueueItem.send_queue.AVG_TIFF_MAKETIME);
			row->SetValueInt("AVG_WAIT_TIME"		, faxQueueItem.send_queue.AVG_WAIT_TIME);
			row->SetValueInt("AVG_SENDING_TIME"		, faxQueueItem.send_queue.AVG_SENDING_TIME);
			table_send->AddRow(row);
		}
		
		table_recv = CDsmIfSession::Inst()->GetDataTable("DS_FX_RECV_Q_MONITOR");
		if(table_recv != NULL && table_recv->m_registed)
		{	
			row2 = table_recv->NewRow();
			row2->m_isBulkRow = false;
			row2->SetValueInt("SYSTEM_ID"			, CConfig::SYSTEM_NO);
			row2->SetValueStr("RECVQ_ID"			, LPSTR(LPCTSTR(faxQueueItem.recv_queue.RECVQ_ID)));
			row2->SetValueStr("SYSTEM_NAME"			, LPSTR(LPCTSTR(CConfig::SYSTEM_TYPE)));
			row2->SetValueStr("RECVQ_NAME"			, LPSTR(LPCTSTR(faxQueueItem.recv_queue.RECVQ_NAME)));
			row2->SetValueInt("SUM_COMPLETE_CNT"	, faxQueueItem.recv_queue.completed_receive_total);
			row2->SetValueInt("SUM_SUCC_CNT"		, faxQueueItem.recv_queue.receive_sucess_total);
			row2->SetValueInt("SUM_FAIL_CNT"		, faxQueueItem.recv_queue.receive_fail_total);
			row2->SetValueInt("RTS_RECVING_CNT"		, faxQueueItem.recv_queue.receving_total);
			row2->SetValueInt("RTS_RECVING_PAGECNT"	, faxQueueItem.recv_queue.receive_tiff_page_count_total);
			row2->SetValueInt("MAX_TIFF_PAGECNT"	, faxQueueItem.recv_queue.MAX_TIFF_PAGECNT);
			row2->SetValueInt("MAX_TIFF_FILESIZE"	, faxQueueItem.recv_queue.MAX_TIFF_FILESIZE);
			row2->SetValueInt("MAX_RECVING_TIME"	, faxQueueItem.recv_queue.MAX_RECVING_TIME);
			row2->SetValueInt("AVG_TIFF_PAGECNT"	, faxQueueItem.recv_queue.AVG_TIFF_PAGECNT);
			row2->SetValueInt("AVG_TIFF_FILESIZE"	, faxQueueItem.recv_queue.AVG_TIFF_FILESIZE);
			row2->SetValueInt("AVG_RECVING_TIME"	, faxQueueItem.recv_queue.AVG_RECVING_TIME);			
			//row->SetValueStr("DB_UPDATE_TIME",		"");
			table_recv->AddRow(row2);
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
void CFaxQueueMonitoringThread::LogInfo(char* p_msg)
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
void CFaxQueueMonitoringThread::LogWrn(char* p_msg)
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
void CFaxQueueMonitoringThread::LogErr(char* p_msg)
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
