
// FID.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
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

	// ������ ���� �� �����ڸ� �����մϴ�.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	return FALSE;
}

bool CFIDApp::Service_Init()
{
	m_bServiceRun = false;

	// P1. BTFAX core �ʱ�ȭ
	int nResult = InitBtFax( CConfig::FAX_CH_CNT);
	if( nResult != 0 )
	{
		APPLOG->Print( DBGLV_ERR, "[FAIL] InitBtFax() in Init_SIP_FID()! ret[%d]\n", nResult );
		return false;
	}

	// P2. �����ѽ� �̹��� ��ȯ �ʱ�ȭ
	InitHmpTiffConvert();

	// P3. SharedMemory ����
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

     // P0. �ѽ���ȣ ��ȣȭ API �ʱ�ȭ
    if(CConfig::ENCRYPT_FIELD_YN == "Y")
    {
        if(!EncryptApi::Inst()->InitApi(CConfig::ENCRYPT_DLL_FILE))
            APPLOG->Print( DBGLV_WRN, "EncryptApi initialize fail.");
        else
            APPLOG->Print( DBGLV_INF, "EncryptApi initialize success.");
    }

	// P1. ä�� ������ ����
	for( int chIdx = 0 ; chIdx < CConfig::FAX_CH_CNT ; chIdx++ )
	{
		g_pFaxChThread[ chIdx ] = new CFaxChThread;

		g_pFaxChThread[ chIdx ]->InitChannel( chIdx, CConfig::FAX_START_CH + chIdx );
		g_pFaxChThread[ chIdx ]->Run();
		Sleep(1);
	}

	::Sleep( 3000 );
    
	// p2. FSM I/F ������ ����
	CFsmIfThread::Inst()->Run();

    ::Sleep( 3000 );
	// p3. Shared memory ������ ����
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

	// P1. ��� ������ ���� ��û
	CThreadBase::ReqStopAll();

	// P2. ä�� ������ ���� ���
	for( int chIdx = 0; chIdx < CConfig::FAX_CH_CNT ; chIdx++ )
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

	// P4. CShmThread ������ ������
	if( CShmThread::Inst()->WaitStop( 2000 ) )
		APPLOG->Print(DBGLV_ERR, "[SUCC] CShmThread stopped" );
	else
		APPLOG->Print(DBGLV_ERR, "[FAIL] CShmThread not stopped" );
	
	// P5. Set FLAG
	m_bServiceRun = false;
}


