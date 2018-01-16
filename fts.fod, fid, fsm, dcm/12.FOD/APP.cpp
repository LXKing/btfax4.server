
// FOD.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
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

	// ��ȭ ���ڿ� �� Ʈ�� �� �Ǵ�
	// �� ��� �� ��Ʈ���� ���ԵǾ� �ִ� ��� �� �����ڸ� ����ϴ�.
	CShellManager *pShellManager = new CShellManager;

	// ǥ�� �ʱ�ȭ
	// �̵� ����� ������� �ʰ� ���� ���� ������ ũ�⸦ ���̷���
	// �Ʒ����� �ʿ� ���� Ư�� �ʱ�ȭ
	// ��ƾ�� �����ؾ� �մϴ�.
	// �ش� ������ ����� ������Ʈ�� Ű�� �����Ͻʽÿ�.
	// TODO: �� ���ڿ��� ȸ�� �Ǵ� ������ �̸��� ����
	// ������ �������� �����ؾ� �մϴ�.
	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));

	// ---------------------------------------------------------------------------
	// P2. ���μ����� ���� Argument List�� "-v"�� ������ compile �ð� Display
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

	// ������ ���� �� �����ڸ� �����մϴ�.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ��ȭ ���ڰ� �������Ƿ� ���� ���α׷��� �޽��� ������ �������� �ʰ�  ���� ���α׷��� ���� �� �ֵ��� FALSE��
	// ��ȯ�մϴ�.
	return FALSE;
}

bool CFODApp::IsStart()
{
	return m_bServiceRun;
}

bool CFODApp::Service_Init()
{
	m_bServiceRun = false;

    // P0. �ѽ���ȣ ��ȣȭ API �ʱ�ȭ
    if(CConfig::ENCRYPT_FIELD_YN == "Y")
    {
        if(!EncryptApi::Inst()->InitApi(CConfig::ENCRYPT_DLL_FILE))
            APPLOG->Print( DBGLV_WRN, "EncryptApi initialize fail.");
        else
            APPLOG->Print( DBGLV_INF, "EncryptApi initialize success.");
    }

	// P1. ä�� ����ü �ʱ�ȭ
	for( int idx = 0 ; idx < MAX_CHANNEL ; idx++ )
		g_FaxChInfo[ idx ].clear();
	
	// P2. BTFAX core �ʱ�ȭ
	int nResult = InitBtFax( CConfig::FAX_TOTAL_CH_CNT );
	if( nResult != 0 )
	{
		APPLOG->Print( DBGLV_ERR, "[FAIL] InitBtFax() in Init_SIP_FOD()! ret[%d]", nResult );
		return false;
	}
	
	// P3. SharedMemory ����
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

	// P1. FSM I/F Thread ����
	CFsmIfThread::Inst()->Run();

	// P2. FaxChThread�� Channel ������ŭ �⵿
	for( chIdx = 0 ; chIdx < CConfig::FAX_TOTAL_CH_CNT ; chIdx++ )
	{
		g_pFaxChThread[chIdx] = new CFaxChThread;
		g_pFaxChThread[chIdx]->FaxChInit( chIdx, CConfig::FAX_START_CH + chIdx );
		g_pFaxChThread[chIdx]->Run();

		Sleep( 1 );
	}
	::Sleep(1000);

	// P3. DB ó�� Thread ����
	CDbPollingThread::Inst()->Run();

	// P4. Shared memory Thread ����
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

	// P1. ��� ������ ���� ��û
	CThreadBase::ReqStopAll();

	// P1. CDbPollingThread ����
	if( CDbPollingThread::Inst()->WaitStop( 2000 ) )
		APPLOG->Print(DBGLV_ERR, "[SUCC] CDbPollingThread stopped" );
	else
		APPLOG->Print(DBGLV_ERR, "[FAIL] CDbPollingThread not stopped" );

	// P2. CFaxChThread ����
	for( chIdx = 0 ; chIdx < CConfig::FAX_TOTAL_CH_CNT ; chIdx++ )
	{
		if(g_pFaxChThread[chIdx] == NULL)
			continue;

		if( g_pFaxChThread[ chIdx ]->WaitStop( 2000 ) )
			APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] [SUCC] CChThread stopped", chIdx + CConfig::FAX_START_CH );
		else
			APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] [FAIL] CChThread not stopped", chIdx + CConfig::FAX_START_CH );
	}

	// P3. FSM I/F ������ ������
	if( CFsmIfThread::Inst()->WaitStop( 2000 ) )
		APPLOG->Print(DBGLV_ERR, "[SUCC] CFsmIfThread stopped" );
	else
		APPLOG->Print(DBGLV_ERR, "[FAIL] CFsmIfThread not stopped" );
	

	// P4. Shared Memory ������ ������.
	if(CShmThread::Inst()->WaitStop(2000))
		APPLOG->Print(DBGLV_ERR, "[SUCC] CShmThread stopped" );
	else
		APPLOG->Print(DBGLV_ERR, "[FAIL] CShmThread not stopped" );

	// P5. Set FLAG
	m_bServiceRun = false;
}

