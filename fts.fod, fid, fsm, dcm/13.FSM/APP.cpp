
// FSM.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
//

#include "stdafx.h"
#include "APP.h"
#include "DLG.h"
#include "Config.h"
#include "Utility.h"
#include "SIP_Manager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFSMApp

BEGIN_MESSAGE_MAP(CFSMApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CFSMApp ����

CFSMApp::CFSMApp()
{
	// �ٽ� ���� ������ ����
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
}


// ������ CFSMApp ��ü�Դϴ�.

CFSMApp theApp;


// CFSMApp �ʱ�ȭ

BOOL CFSMApp::InitInstance()
{
	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�.
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	CShellManager *pShellManager = new CShellManager;

	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));



	

	// ---------------------------------------------------------------------------
	// P2. ���μ����� ���� Argument List�� "-v"�� ������ compile �ð� Display
	// ---------------------------------------------------------------------------
	if( !strcmp(m_lpCmdLine, "-v") )
	{
		CString strMsg;
		strMsg.Format( "FSM %s  [ compiled at %s ]", (LPCSTR)CConfig::APP_VER, (LPCSTR)CUtility::GetCompileTime() );
		AfxMessageBox( strMsg );
		return FALSE;
	}
	else
	{
		CConfig::PROCESS_NO = atol( m_lpCmdLine );
		//double aaaa = atof( m_lpCmdLine );
		if( CConfig::PROCESS_NO <= 0 )
		{
			AfxMessageBox( "Usage : FSM.exe {[process no] or \"-v\"" );
			return FALSE;
		}
	}

	// ---------------------------------------------------------------------------
	// P3. Open Dialog 
	// ---------------------------------------------------------------------------
	SIP_Manager::InitSingleton();

	// ---------------------------------------------------------------------------
	// P4. Open Dialog 
	// ---------------------------------------------------------------------------
	CFSMDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ���⿡ [Ȯ��]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ���⿡ [���]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
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


void CFSMApp::DisplayState(int chIdx, EN_STATUS status, CString szMsg )
{
	/*
	if(m_hViewWnd != NULL)
	{
		FAX_CH_STATE	*pChStatus	= new FAX_CH_STATE;

		memset(pChStatus, 0x00, sizeof(FAX_CH_STATE));
		pChStatus->chIdx	= chIdx;
		pChStatus->status	= status;
		sprintf(pChStatus->szMsg, "%s", (LPCSTR)szMsg);

		::PostMessage( m_hViewWnd, WM_USER_DISPLAY_STATE, (WPARAM) pChStatus, (LPARAM) 0 );	
	}
	*/
}

void CFSMApp::DisplayLog(char* Format, ...)
{
	/*
	va_list	Args;	
	char	Buff[2048];	

	if( m_hViewWnd != NULL )
	{
		memset(Buff, 0x00, sizeof(Buff));

		va_start(Args, Format);
		vsprintf(Buff, Format, Args);
		va_end(Args);

		CString* pString = new CString();
		pString->Format(_T("%s"), Buff);
		::PostMessage( m_hViewWnd, WM_USER_DISPLAY_LOG, (WPARAM)pString, 0 );	
	}
	*/
}



