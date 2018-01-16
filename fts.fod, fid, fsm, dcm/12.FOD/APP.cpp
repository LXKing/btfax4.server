
// FOD.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "APP.h"
#include "DLG.h"
#include "Config.h"
#include "DbThread.h"
#include "FsmIfThread.h"
#include "Utility.h"

#include "btfax.h"
#include "btflib.h"

#include "ShmCtrl.h"
#include "ShmThread.h"

#include "EncryptApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFODApp

BEGIN_MESSAGE_MAP(CFODApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CFODApp theApp;


CFODApp::CFODApp()
{
	m_bServiceRun = false;
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
	CShellManager *pShellManager = new CShellManager;

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
		strMsg.Format( "FOD %s  [ compiled at %s ]", (LPCSTR)CConfig::APP_VER, (LPCSTR)CUtility::GetCompileTime() );
		AfxMessageBox( strMsg );
		return FALSE;
	}
	else
	{
		CConfig::PROCESS_NO = atoi( m_lpCmdLine );
		if( CConfig::PROCESS_NO <= 0 )
		{
			AfxMessageBox( "Usage : FOD.exe {[process no] or \"-v\"" );
			return FALSE;
		}
	}

	CFODDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{	
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고  응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	return FALSE;
}

bool CFODApp::IsStart()
{
	return m_bServiceRun;
}

bool CFODApp::Service_Init()
{
	m_bServiceRun = false;

    // P0. 팩스번호 암호화 API 초기화
    if(CConfig::ENCRYPT_FIELD_YN == "Y")
    {
        if(!EncryptApi::Inst()->InitApi(CConfig::ENCRYPT_DLL_FILE))
            APPLOG->Print( DBGLV_WRN, "EncryptApi initialize fail.");
        else
            APPLOG->Print( DBGLV_INF, "EncryptApi initialize success.");
    }

	// P1. 채널 구조체 초기화
	for( int idx = 0 ; idx < MAX_CHANNEL ; idx++ )
		g_FaxChInfo[ idx ].clear();
	
	// P2. BTFAX core 초기화
	int nResult = InitBtFax( CConfig::FAX_TOTAL_CH_CNT );
	if( nResult != 0 )
	{
		APPLOG->Print( DBGLV_ERR, "[FAIL] InitBtFax() in Init_SIP_FOD()! ret[%d]", nResult );
		return false;
	}
	
	// P3. SharedMemory 생성
	if(!g_shmCtrl.OpenOrCreateShm(CConfig::FAX_CH_MONI_SHM_KEY))
	{
		APPLOG->Print(DBGLV_ERR, "[FAIL] %s", g_shmCtrl.GetErrMsg());
		return false;
	}

	return true;
}

bool CFODApp::Service_Start()
{
	int		chIdx;

	// P0. Check FLAG
	if( m_bServiceRun )
		return false;

	// P1. FSM I/F Thread 구동
	CFsmIfThread::Inst()->Run();

	// P2. FaxChThread를 Channel 개수만큼 기동
	for( chIdx = 0 ; chIdx < CConfig::FAX_TOTAL_CH_CNT ; chIdx++ )
	{
		g_pFaxChThread[chIdx] = new CFaxChThread;
		g_pFaxChThread[chIdx]->FaxChInit( chIdx, CConfig::FAX_START_CH + chIdx );
		g_pFaxChThread[chIdx]->Run();

		Sleep( 1 );
	}
	::Sleep(1000);

	// P3. DB 처리 Thread 구동
	CDbPollingThread::Inst()->Run();

	// P4. Shared memory Thread 구동
	CShmThread::Inst()->Run();

	// P5. Set FLAG
	m_bServiceRun = true;

	return true;
}

void CFODApp::Service_Stop()
{
	int		chIdx;
	DWORD	dwResult;

	// P0. Check FLAG
	if( !m_bServiceRun )
		return;

	// P1. 모든 쓰레드 종료 요청
	CThreadBase::ReqStopAll();

	// P1. CDbPollingThread 종료
	if( CDbPollingThread::Inst()->WaitStop( 2000 ) )
		APPLOG->Print(DBGLV_ERR, "[SUCC] CDbPollingThread stopped" );
	else
		APPLOG->Print(DBGLV_ERR, "[FAIL] CDbPollingThread not stopped" );

	// P2. CFaxChThread 종료
	for( chIdx = 0 ; chIdx < CConfig::FAX_TOTAL_CH_CNT ; chIdx++ )
	{
		if(g_pFaxChThread[chIdx] == NULL)
			continue;

		if( g_pFaxChThread[ chIdx ]->WaitStop( 2000 ) )
			APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] [SUCC] CChThread stopped", chIdx + CConfig::FAX_START_CH );
		else
			APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] [FAIL] CChThread not stopped", chIdx + CConfig::FAX_START_CH );
	}

	// P3. FSM I/F 쓰레드 종료대기
	if( CFsmIfThread::Inst()->WaitStop( 2000 ) )
		APPLOG->Print(DBGLV_ERR, "[SUCC] CFsmIfThread stopped" );
	else
		APPLOG->Print(DBGLV_ERR, "[FAIL] CFsmIfThread not stopped" );
	

	// P4. Shared Memory 쓰레드 종료대기.
	if(CShmThread::Inst()->WaitStop(2000))
		APPLOG->Print(DBGLV_ERR, "[SUCC] CShmThread stopped" );
	else
		APPLOG->Print(DBGLV_ERR, "[FAIL] CShmThread not stopped" );

	// P5. Set FLAG
	m_bServiceRun = false;
}

