
// FID.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "APP.h"
#include "DLG.h"
#include "ShmCtrl.h"
#include "ShmThread.h"
#include "FaxChThread.h"
#include "FsmIfThread.h"
#include "Config.h"
#include "btfconvert.h"
#include "Utility.h"
#include "AppLog.h"

#include "btfax.h"
#include "btflib.h"

#include "EncryptApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFIDApp

BEGIN_MESSAGE_MAP(CFIDApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CFIDApp theApp;

CFIDApp::CFIDApp()
{
	m_bServiceRun = false;
}


BOOL CFIDApp::InitInstance()
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
		strMsg.Format( "FID %s  [ compiled at %s ]", (LPCSTR)CConfig::APP_VER, (LPCSTR)CUtility::GetCompileTime() );
		AfxMessageBox( strMsg );
		return FALSE;
	}
	else
	{
		CConfig::PROCESS_NO = atoi( m_lpCmdLine );
		if( CConfig::PROCESS_NO <= 0 )
		{
			AfxMessageBox( "Usage : FID.exe {[process no] or \"-v\"" );
			return FALSE;
		}
	}

	CFIDDlg dlg;
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

	return FALSE;
}

bool CFIDApp::Service_Init()
{
	m_bServiceRun = false;

	// P1. BTFAX core 초기화
	int nResult = InitBtFax( CConfig::FAX_CH_CNT);
	if( nResult != 0 )
	{
		APPLOG->Print( DBGLV_ERR, "[FAIL] InitBtFax() in Init_SIP_FID()! ret[%d]\n", nResult );
		return false;
	}

	// P2. 수신팩스 이미지 변환 초기화
	InitHmpTiffConvert();

	// P3. SharedMemory 생성
	if(!g_shmCtrl.OpenOrCreateShm(CConfig::FAX_CH_MONI_SHM_KEY))
	{
		APPLOG->Print(DBGLV_ERR, "[FAIL] %s", g_shmCtrl.GetErrMsg());
		return false;
	}

	return true;
}

bool CFIDApp::Service_Start()
{
	// P0. Check FLAG
	if( m_bServiceRun )
		return false;

     // P0. 팩스번호 암호화 API 초기화
    if(CConfig::ENCRYPT_FIELD_YN == "Y")
    {
        if(!EncryptApi::Inst()->InitApi(CConfig::ENCRYPT_DLL_FILE))
            APPLOG->Print( DBGLV_WRN, "EncryptApi initialize fail.");
        else
            APPLOG->Print( DBGLV_INF, "EncryptApi initialize success.");
    }

	// P1. 채널 쓰레드 실행
	for( int chIdx = 0 ; chIdx < CConfig::FAX_CH_CNT ; chIdx++ )
	{
		g_pFaxChThread[ chIdx ] = new CFaxChThread;

		g_pFaxChThread[ chIdx ]->InitChannel( chIdx, CConfig::FAX_START_CH + chIdx );
		g_pFaxChThread[ chIdx ]->Run();
		Sleep(1);
	}

	::Sleep( 3000 );
    
	// p2. FSM I/F 쓰레드 실행
	CFsmIfThread::Inst()->Run();

    ::Sleep( 3000 );
	// p3. Shared memory 쓰레드 실행
	CShmThread::Inst()->Run();

	// p4. Set FLAG
	m_bServiceRun = true;

	return true;
}

void CFIDApp::Service_Stop()
{
	// P0. Check FLAG
	if( !m_bServiceRun )
		return;

	// P1. 모든 쓰레드 종료 요청
	CThreadBase::ReqStopAll();

	// P2. 채널 쓰레드 종료 대기
	for( int chIdx = 0; chIdx < CConfig::FAX_CH_CNT ; chIdx++ )
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

	// P4. CShmThread 쓰레드 종료대기
	if( CShmThread::Inst()->WaitStop( 2000 ) )
		APPLOG->Print(DBGLV_ERR, "[SUCC] CShmThread stopped" );
	else
		APPLOG->Print(DBGLV_ERR, "[FAIL] CShmThread not stopped" );
	
	// P5. Set FLAG
	m_bServiceRun = false;
}


