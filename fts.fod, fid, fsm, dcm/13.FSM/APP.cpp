
// FSM.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
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


// CFSMApp 생성

CFSMApp::CFSMApp()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CFSMApp 개체입니다.

CFSMApp theApp;


// CFSMApp 초기화

BOOL CFSMApp::InitInstance()
{
	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
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

	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));



	

	// ---------------------------------------------------------------------------
	// P2. 프로세스명 뒤의 Argument List에 "-v"가 있으면 compile 시간 Display
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
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
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



