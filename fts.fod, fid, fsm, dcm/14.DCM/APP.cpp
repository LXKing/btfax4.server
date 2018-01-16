
// FOD.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "APP.h"
#include "DLG.h"
#include "Config.h"
#include "Utility.h"

// iCore..
#include "iLib.h"
#include "iSocket.h"
#include "iThread.h"
#include "iMutex.h"
#include "iType.h"
#include "iTimer.h"
// iCore

#include "FaxChanelMonitoringThread.h";
#include "FaxQueueMonitoringThread.h";
#include "FaxCdrThread.h";

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFODApp

BEGIN_MESSAGE_MAP(CFODApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CFODApp theApp;
CFaxChanelMonitoringThread	m_faxChMoniThread;
CFaxCdrThread				m_faxCdrThread;
CFaxQueueMonitoringThread	m_faxQueueMoniThread;

CFODApp::CFODApp()
{
	m_bServiceRun = false;
	
	m_DataSet	= new CDcmDataSet("RT_MONI");
	m_table1	= new CDcmDataTable("DS_FX_CHNL_MONITOR");
	m_table2	= new CDcmDataTable("DS_FX_SEND_Q_MONITOR");
	m_table3	= new CDcmDataTable("DS_FX_RECV_Q_MONITOR");
}


BOOL CFODApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	// 대화 상자에 셸 트리 뷰 또는
	// 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	
	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));

	// ---------------------------------------------------------------------------
	// P2. 프로세스명 뒤의 Argument List에 "-v"가 있으면 compile 시간 Display
	// ---------------------------------------------------------------------------
	if( !strcmp(m_lpCmdLine, "-v") )
	{
		CString strMsg;
		strMsg.Format( "DCM %s  [ compiled at %s ]", (LPCSTR)CConfig::APP_VER, (LPCSTR)CUtility::GetCompileTime() );
		AfxMessageBox( strMsg );
		return FALSE;
	}
	else
	{
		CConfig::PROCESS_NO = atoi( m_lpCmdLine );
		if( CConfig::PROCESS_NO <= 0 )
		{
			AfxMessageBox( "Usage : DCM.exe {[process no] or \"-v\"" );
			return FALSE;
		}
	}

	
	iSocket::WSStartup();

	CFODDlg dlg;
	m_pMainWnd = &dlg;
	
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{	
	}
	else if (nResponse == IDCANCEL)
	{
	}
		

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고  응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	iSocket::WSCleanup();

	return FALSE;
}

bool CFODApp::IsStart()
{
	return m_bServiceRun;
}

bool CFODApp::Service_Init()
{		
	return true;
}

bool CFODApp::Service_Start()
{
	//// P1. Check FLAG
	if( m_bServiceRun )
		return false;


	//// 데이터 테이블 생성.
	MakeDataTables();

	//// 벌크데이터 생성.
	MakeBulkDatas();

	//// DSM 인터페이스 세션 SET
	CDsmIfSession::Inst()->SetInfo(LPSTR(LPCTSTR(CConfig::SYSTEM_TYPE))
									, LPSTR(LPCTSTR(CConfig::SYSTEM_TYPE))
									, CConfig::SYSTEM_NO
									, LPSTR(LPCTSTR(CConfig::PROCESS_TYPE_STR))
									, CConfig::PROCESS_NO);

	CDsmIfSession::Inst()->SetAddress(LPSTR(LPCTSTR(CConfig::DSM_IP)), CConfig::DSM_PORT);
	CDsmIfSession::Inst()->SetDataSet(m_DataSet);
	CDsmIfSession::Inst()->Run();
	
	// 채널모니터링 스레드
	m_faxChMoniThread.StartThread();

	// 팩스 cdr 스레드
	m_faxCdrThread.StartThread();
	
	// 팩스 큐 모니터링 스레드
	m_faxQueueMoniThread.StartThread();
		
	// P4. Set FLAG
	m_bServiceRun = true;

	return true;
}

void CFODApp::Service_Stop()
{
	// Check FLAG
	if( !m_bServiceRun )
		return;

	CDsmIfSession::Inst()->StopAll();

	delete m_DataSet;
	delete m_table1;
	delete m_table2;
	delete m_table3;

	// 팩스 채널 모니터링 스레드
	m_faxChMoniThread.StopThread();

	// 팩스 cdr 스레드
	m_faxCdrThread.StopThread();
	
	// 팩스 큐 모니터링 스레드
	m_faxQueueMoniThread.StopThread();

	// Set FLAG
	m_bServiceRun = false;
}


void CFODApp::MakeDataTables()
{	
	// 채널모니터링
	m_table1->AddColumn("SYSTEM_ID"		, DcmColumnType_Integer);
	m_table1->AddColumn("CHNL_NO"		, DcmColumnType_Integer);	
	m_table1->AddColumn("FOD_ID"		, DcmColumnType_Integer);
	m_table1->AddColumn("DIRECTION"		, DcmColumnType_Integer);	
	m_table1->AddColumn("CHNL_STATUS"	, DcmColumnType_Integer);
	m_table1->AddColumn("ANI"			, DcmColumnType_String);
	m_table1->AddColumn("DNIS"			, DcmColumnType_String);	
	m_table1->AddColumn("REQST_TIME"	, DcmColumnType_String);
	m_table1->AddColumn("START_TIME"	, DcmColumnType_String);
	m_table1->AddColumn("FINSH_TIME"	, DcmColumnType_String);
	m_table1->AddColumn("SERVICE_ID"	, DcmColumnType_String);
	m_table1->AddColumn("SERVICE_NAME"	, DcmColumnType_String);
	m_table1->AddColumn("SUM_SEND_CNT"	, DcmColumnType_Integer);
	m_table1->AddColumn("SUM_RECV_CNT"	, DcmColumnType_Integer);
	m_table1->AddColumn("CHANGE_FLAG"	, DcmColumnType_Integer);
	m_DataSet->AddTable(m_table1);

	// 팩스 송신큐 테이블	
	m_table2->AddColumn("SYSTEM_ID"			, DcmColumnType_Integer);
	m_table2->AddColumn("SENDQ_ID"			, DcmColumnType_String);
	m_table2->AddColumn("SYSTEM_NAME"		, DcmColumnType_String);
	m_table2->AddColumn("SENDQ_NAME"		, DcmColumnType_String);
	m_table2->AddColumn("SUM_COMPLETE_CNT"	, DcmColumnType_Integer);
	m_table2->AddColumn("SUM_SUCC_CNT"		, DcmColumnType_Integer);
	m_table2->AddColumn("SUM_FAIL_CNT"		, DcmColumnType_Integer);
	m_table2->AddColumn("RTS_PROCESSING_CNT", DcmColumnType_Integer);
	m_table2->AddColumn("RTS_TIFF_WAITCNT"	, DcmColumnType_Integer);
	m_table2->AddColumn("RTS_SEND_WAITCNT"	, DcmColumnType_Integer);
	m_table2->AddColumn("RTS_SENDING_CNT"	, DcmColumnType_Integer);
	m_table2->AddColumn("RTS_SENDING_PAGECNT", DcmColumnType_Integer);
	m_table2->AddColumn("MAX_SEND_TRYCNT"	, DcmColumnType_Integer);
	m_table2->AddColumn("MAX_TIFF_PAGECNT"	, DcmColumnType_Integer);
	m_table2->AddColumn("MAX_TIFF_FILESIZE"	, DcmColumnType_Integer);
	m_table2->AddColumn("MAX_TIFF_MAKETIME"	, DcmColumnType_String);
	m_table2->AddColumn("MAX_WAIT_TIME"		, DcmColumnType_String);
	m_table2->AddColumn("MAX_SENDING_TIME"	, DcmColumnType_String);
	m_table2->AddColumn("AVG_SEND_TRYCNT"	, DcmColumnType_Integer);
	m_table2->AddColumn("AVG_TIFF_PAGECNT"	, DcmColumnType_Integer);
	m_table2->AddColumn("AVG_TIFF_FILESIZE"	, DcmColumnType_Integer);
	m_table2->AddColumn("AVG_TIFF_MAKETIME"	, DcmColumnType_String);
	m_table2->AddColumn("AVG_WAIT_TIME"		, DcmColumnType_Integer);
	m_table2->AddColumn("AVG_SENDING_TIME"	, DcmColumnType_Integer);
	m_DataSet->AddTable(m_table2);

	// 팩스 수신큐 테이블
	m_table3->AddColumn("SYSTEM_ID"			,	DcmColumnType_Integer);
	m_table3->AddColumn("RECVQ_ID"			,	DcmColumnType_String);
	m_table3->AddColumn("SYSTEM_NAME"		,	DcmColumnType_String);
	m_table3->AddColumn("RECVQ_NAME"		,	DcmColumnType_String);
	m_table3->AddColumn("SUM_COMPLETE_CNT"	,	DcmColumnType_Integer);
	m_table3->AddColumn("SUM_SUCC_CNT"		,	DcmColumnType_Integer);
	m_table3->AddColumn("SUM_FAIL_CNT"		,	DcmColumnType_Integer);
	m_table3->AddColumn("RTS_RECVING_CNT"	,	DcmColumnType_Integer);
	m_table3->AddColumn("RTS_RECVING_PAGECNT",	DcmColumnType_Integer);
	m_table3->AddColumn("MAX_TIFF_PAGECNT"	,	DcmColumnType_Integer);
	m_table3->AddColumn("MAX_TIFF_FILESIZE"	,	DcmColumnType_Integer);
	m_table3->AddColumn("MAX_RECVING_TIME"	,	DcmColumnType_String);
	m_table3->AddColumn("AVG_TIFF_PAGECNT"	,	DcmColumnType_Integer);
	m_table3->AddColumn("AVG_TIFF_FILESIZE"	,	DcmColumnType_Integer);
	m_table3->AddColumn("AVG_RECVING_TIME"	,	DcmColumnType_Integer);
	//table_total->AddColumn("DB_UPDATE_TIME"		,	DcmColumnType_Integer);

	m_DataSet->AddTable(m_table3);
}


void CFODApp::MakeBulkDatas()
{
	CShmCtrl		shmCtrl;
	
	// shared memory create.
	if(shmCtrl.OpenOrCreateShm(CConfig::FAX_CH_MONI_SHM_KEY))
	{	
		SHM_CH_MONI_DATA* shmData = shmCtrl.GetShmData(0);
		if(shmData != NULL)
		{	
			for(int i = 0; i < CConfig::FAX_CH_MONI_TOTAL_CH_CNT; i++)
			{		
				try
				{
					CDcmDataRow* row = m_table1->NewRow();		
					row->m_isBulkRow = true;
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
					m_table1->AddRow(row);

					shmData++;
				}
				catch(DWORD ex)
				{
					break;
				}
			}
		}
	}

	CDbModule::FAX_Q_ITEM faxQueueItem;
	faxQueueItem.clear();
	faxQueueItem.send_queue.SYSTEM_ID	= CConfig::SYSTEM_NO;
	faxQueueItem.send_queue.SYSTEM_NAME = CConfig::SYSTEM_TYPE;
	faxQueueItem.send_queue.SENDQ_ID	= "BTF_FAX_SEND_MSTR";
	faxQueueItem.send_queue.SENDQ_NAME	= "팩스송신큐";

	faxQueueItem.recv_queue.SYSTEM_ID	= CConfig::SYSTEM_NO;
	faxQueueItem.recv_queue.SYSTEM_NAME = CConfig::SYSTEM_TYPE;
	faxQueueItem.recv_queue.RECVQ_ID	= "BTF_FAX_RECV_MSTR";
	faxQueueItem.recv_queue.RECVQ_NAME	= "팩스수신큐";

	// make bulk data..
	CDcmDataRow *row2 = m_table2->NewRow();
	row2->m_isBulkRow = true;
	row2->SetValueInt("SYSTEM_ID"			, CConfig::SYSTEM_NO);
	row2->SetValueStr("SENDQ_ID"			, LPSTR(LPCTSTR(faxQueueItem.send_queue.SENDQ_ID)));
	row2->SetValueStr("SYSTEM_NAME"			, LPSTR(LPCTSTR(CConfig::SYSTEM_TYPE)));
	row2->SetValueStr("SENDQ_NAME"			, LPSTR(LPCTSTR(faxQueueItem.send_queue.SENDQ_NAME)));
	row2->SetValueInt("SUM_COMPLETE_CNT"	, faxQueueItem.send_queue.completed_send_total);
	row2->SetValueInt("SUM_SUCC_CNT"		, faxQueueItem.send_queue.send_sucess_total);
	row2->SetValueInt("SUM_FAIL_CNT"		, faxQueueItem.send_queue.send_fail_total);
	row2->SetValueInt("RTS_PROCESSING_CNT"	, faxQueueItem.send_queue.wait_processing_total);
	row2->SetValueInt("RTS_TIFF_WAITCNT"	, faxQueueItem.send_queue.wait_make_tiff_total);
	row2->SetValueInt("RTS_SEND_WAITCNT"	, faxQueueItem.send_queue.wait_send_total);
	row2->SetValueInt("RTS_SENDING_CNT"		, faxQueueItem.send_queue.sending_total);
	row2->SetValueInt("RTS_SENDING_PAGECNT"	, faxQueueItem.send_queue.sending_tiff_page_count_total);
	row2->SetValueInt("MAX_SEND_TRYCNT"		, faxQueueItem.send_queue.MAX_SEND_TRYCNT);
	row2->SetValueInt("MAX_TIFF_PAGECNT"	, faxQueueItem.send_queue.MAX_TIFF_PAGECNT);
	row2->SetValueInt("MAX_TIFF_FILESIZE"	, faxQueueItem.send_queue.MAX_TIFF_FILESIZE);
	row2->SetValueInt("MAX_TIFF_MAKETIME"	, faxQueueItem.send_queue.MAX_TIFF_MAKETIME);
	row2->SetValueInt("MAX_WAIT_TIME"		, faxQueueItem.send_queue.MAX_WAIT_TIME);
	row2->SetValueInt("MAX_SENDING_TIME"	, faxQueueItem.send_queue.MAX_SENDING_TIME);
	row2->SetValueInt("AVG_SEND_TRYCNT"		, faxQueueItem.send_queue.AVG_SEND_TRYCNT);
	row2->SetValueInt("AVG_TIFF_PAGECNT"	, faxQueueItem.send_queue.AVG_TIFF_PAGECNT);
	row2->SetValueInt("AVG_TIFF_FILESIZE"	, faxQueueItem.send_queue.AVG_TIFF_FILESIZE);
	row2->SetValueInt("AVG_TIFF_MAKETIME"	, faxQueueItem.send_queue.AVG_TIFF_MAKETIME);
	row2->SetValueInt("AVG_WAIT_TIME"		, faxQueueItem.send_queue.AVG_WAIT_TIME);
	row2->SetValueInt("AVG_SENDING_TIME"	, faxQueueItem.send_queue.AVG_SENDING_TIME);
	m_table2->AddRow(row2);

	CDcmDataRow *row3 = m_table3->NewRow();
	row3->m_isBulkRow = true;
	row3->SetValueInt("SYSTEM_ID"			, CConfig::SYSTEM_NO);
	row3->SetValueStr("RECVQ_ID"			, LPSTR(LPCTSTR(faxQueueItem.recv_queue.RECVQ_ID)));
	row3->SetValueStr("SYSTEM_NAME"			, LPSTR(LPCTSTR(CConfig::SYSTEM_TYPE)));
	row3->SetValueStr("RECVQ_NAME"			, LPSTR(LPCTSTR(faxQueueItem.recv_queue.RECVQ_NAME)));
	row3->SetValueInt("SUM_COMPLETE_CNT"	, faxQueueItem.recv_queue.completed_receive_total);
	row3->SetValueInt("SUM_SUCC_CNT"		, faxQueueItem.recv_queue.receive_sucess_total);
	row3->SetValueInt("SUM_FAIL_CNT"		, faxQueueItem.recv_queue.receive_fail_total);
	row3->SetValueInt("RTS_RECVING_CNT"		, faxQueueItem.recv_queue.receving_total);
	row3->SetValueInt("RTS_RECVING_PAGECNT"	, faxQueueItem.recv_queue.receive_tiff_page_count_total);
	row3->SetValueInt("MAX_TIFF_PAGECNT"	, faxQueueItem.recv_queue.MAX_TIFF_PAGECNT);
	row3->SetValueInt("MAX_TIFF_FILESIZE"	, faxQueueItem.recv_queue.MAX_TIFF_FILESIZE);
	row3->SetValueInt("MAX_RECVING_TIME"	, faxQueueItem.recv_queue.MAX_RECVING_TIME);
	row3->SetValueInt("AVG_TIFF_PAGECNT"	, faxQueueItem.recv_queue.AVG_TIFF_PAGECNT);
	row3->SetValueInt("AVG_TIFF_FILESIZE"	, faxQueueItem.recv_queue.AVG_TIFF_FILESIZE);
	row3->SetValueInt("AVG_RECVING_TIME"	, faxQueueItem.recv_queue.AVG_RECVING_TIME);			
	m_table3->AddRow(row3);
}