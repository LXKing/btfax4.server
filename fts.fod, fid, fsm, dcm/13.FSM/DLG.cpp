
// FSMDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "APP.h"
#include "DLG.h"
#include "afxdialogex.h"
#include "TCP_Server.h"
#include "Config.h"
#include "TabConfig.h"
#include "TabUiLog.h"
#include "TabInfo.h"

#include "SIP_Manager.h"
#include "SIP_CallingTimer.h"

#include "Utility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFSMDlg 대화 상자




CFSMDlg::CFSMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFSMDlg::IDD, pParent)
{
	m_hIcon		= AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pwndShow	= NULL;
}

void CFSMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_ctlTab);
}

BEGIN_MESSAGE_MAP( CFSMDlg, CDialogEx )
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CFSMDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFSMDlg::OnBnClickedCancel)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CFSMDlg::m_ctlTab_OnTcnSelchangeTab)
	ON_COMMAND(ID_FILE_EXIT, &CFSMDlg::OnFileExit)
	ON_COMMAND(ID_TASK_START, &CFSMDlg::OnTaskStart)
	ON_COMMAND(ID_TASK_STOP, &CFSMDlg::OnTaskStop)
	ON_COMMAND(ID_TASK_CLEARUILOG, &CFSMDlg::OnTaskClearuilog)
	ON_COMMAND(ID_TASK_LOADCONFIG, &CFSMDlg::OnTaskLoadconfig)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP( CFSMDlg ) 
  EASYSIZE( IDC_TAB,	ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0 )
END_EASYSIZE_MAP





BOOL CFSMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// P1. UI 초기화
	_InitTab();
	INIT_EASYSIZE;

	// P1. 클래스 초기화
	CConfig::SetCallback( &CTabConfig::DisplayConfig );


	// P2. 환경 파일 읽기
	CConfig config;
	if( !config.LoadConfig_file())
		return false;

	// P3. DB 연결 문자열 SET
	CDbConnection::Inst()->Initialize( CConfig::DB_CONNECTION_STRING, true );

	// P4. 환경 DB 읽기
	if( !config.LoadConfig_db() )
		return false;

	//SetWindowText( CConfig::PROCESS_ID + "    " + CConfig::APP_VER );

	// P5. 로그 초기화
	APPLOG->SetCallback( CTabUiLog::AddLog, CTabInfo::IncrementError);
	APPLOG->SetLogDirectory( CConfig::SYSTEM_LOG_PATH );
	APPLOG->SetLogName( (char*)(LPCSTR)CConfig::SYSTEM_PROCESS_ID );
	APPLOG->SetFileSaveUnit( LOG_FILE_SAVE_UNIT_DAY );
	APPLOG->SetDBGLV( CConfig::SYSTEM_LOG_LEVEL );
	

	APPLOG->Print( DBGLV_INF, "\n\n ******************** FSM START ******************** \n" );
    
	// P6. 자동 TASK 시작
	SendMessage( WM_COMMAND, MAKEWPARAM(ID_TASK_START,0), 0 );

	return TRUE;
}

void CFSMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

void CFSMDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	
	UPDATE_EASYSIZE;

	if( m_ctlTab.GetSafeHwnd() )
	{
		CRect Rect;

		m_ctlTab.GetClientRect( &Rect );
		m_tabConfig.SetWindowPos(NULL, 5, 25, Rect.Width() - 12, Rect.Height() - 33, SWP_NOZORDER);
		m_tabUiLog.SetWindowPos(NULL, 5, 25, Rect.Width() - 12, Rect.Height() - 33, SWP_NOZORDER);
		m_tabInfo.SetWindowPos(NULL, 5, 25, Rect.Width() - 12, Rect.Height() - 33, SWP_NOZORDER);
	}
}


HCURSOR CFSMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFSMDlg::m_ctlTab_OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{	
	if(m_pwndShow != NULL)
	{
		m_pwndShow->ShowWindow(SW_HIDE);
		m_pwndShow = NULL;
	}

	switch( m_ctlTab.GetCurSel() )
	{
	case 0:
		m_tabConfig.ShowWindow( SW_SHOW );
		m_pwndShow = &m_tabConfig;
		break;
	case 1:
		m_tabUiLog.ShowWindow( SW_SHOW );
		m_pwndShow = &m_tabUiLog;
		break;
	case 2:
		m_tabInfo.ShowWindow( SW_SHOW );
		m_pwndShow = &m_tabInfo;
		break;
	}

	*pResult = 0;
}


void CFSMDlg::OnBnClickedOk()
{	
}


void CFSMDlg::OnBnClickedCancel()
{
	/*if( AfxMessageBox( "종료하시겠습니까?", MB_YESNO ) != IDYES )
		return;*/

	CDialogEx::OnCancel();
}

void CFSMDlg::OnFileExit()
{
	AfxPostQuitMessage( 0 );
}


void CFSMDlg::OnTaskStart()
{
	// ---------------------------------------------------------------------------
	// P1. Shared Memory 생성
	// ---------------------------------------------------------------------------
	CString strFsmShm;
    
    while(1)
    {
	    shmem = (sip_share *)CSharedMemory::OpenSharedMemory( FSM_SHM, sizeof(sip_share), true );
	    if( shmem == NULL ) {
            APPLOG->Print( DBGLV_ERR, "Create Shared Memory Fail." );
            Sleep(2000);
            continue;
	    }

        break;
    }

	SHMID_FSM = CConfig::PROCESS_NO;
		
	shmem->pin[SHMID_FSM]		=_getpid();
	shmem->start_time[SHMID_FSM]=time(0);
	

	// ---------------------------------------------------------------------------
	// P2. SIP_FID와 연동을 위해 TCP Server socket 초기화
	// ---------------------------------------------------------------------------
    while(1)
    {
	    sock_agent=TcpServerInit( CConfig::ADDRESS_CLIENT_LISTEN_PORT );
	    if( sock_agent < 0 )
	    {
            APPLOG->Print( DBGLV_ERR, "TCP Port(%d) Open Fail.", CConfig::ADDRESS_CLIENT_LISTEN_PORT );
            Sleep(2000);
            continue;
	    }

        break;
    }

	FD_SET(sock_agent, &allset);
	GetMaxFd(sock_agent);

	// ---------------------------------------------------------------------------
	// P3. SIP_FID로 부터 받은 Msg 처리 Thread 기동
	// ---------------------------------------------------------------------------
	unsigned int	tid;
	_beginthreadex(0, 0, ProcessAppEvent, 0, 0, &tid);

	// ---------------------------------------------------------------------------
	// P4. SIP 시작
	// ---------------------------------------------------------------------------
	while(1)
    {
        if( !SIP_Manager::inst->Start() )
	    {
            APPLOG->Print( DBGLV_ERR, "SIP Start Fail.");
            Sleep(2000);
            continue;
	    }

        break;
    }


	// ---------------------------------------------------------------------------
	// P5. 메뉴 활성화/비활성화
	// ---------------------------------------------------------------------------
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem( ID_TASK_START, MF_GRAYED  );
	pMenu->EnableMenuItem( ID_TASK_STOP,  MF_ENABLED );
}


void CFSMDlg::OnTaskStop()
{
	// ---------------------------------------------------------------------------
	// P1. 메뉴 활성화/비활성화
	// ---------------------------------------------------------------------------
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem( ID_TASK_START, MF_ENABLED );
	pMenu->EnableMenuItem( ID_TASK_STOP,  MF_GRAYED  );
}


void CFSMDlg::OnTaskClearuilog()
{
}


void CFSMDlg::OnTaskLoadconfig()
{
}




void CFSMDlg::_InitTab()
{
	m_ctlTab.InsertItem( 1, " CONFIG " );
	m_ctlTab.InsertItem( 2, " LOG " );
	m_ctlTab.InsertItem( 3, " INFO " );

	CRect Rect;
	m_ctlTab.GetClientRect( &Rect );
 
	m_tabConfig.Create(CTabConfig::IDD, &m_ctlTab);
	m_tabConfig.SetWindowPos(NULL, 5, 25, Rect.Width() - 12, Rect.Height() - 33, SWP_SHOWWINDOW | SWP_NOZORDER);
	m_pwndShow = &m_tabConfig;

	m_tabUiLog.Create(CTabUiLog::IDD, &m_ctlTab);
	m_tabUiLog.SetWindowPos(NULL, 5, 25, Rect.Width() - 12, Rect.Height() - 33, SWP_NOZORDER);

	m_tabInfo.Create(CTabInfo::IDD, &m_ctlTab);
	m_tabInfo.SetWindowPos(NULL, 5, 25, Rect.Width() - 12, Rect.Height() - 33, SWP_NOZORDER);
}

void CFSMDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	// dialog hide	
	lpwndpos->flags &= ~SWP_SHOWWINDOW; 
	CDialogEx::OnWindowPosChanging(lpwndpos);
}


void CFSMDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	//m_pwndShow->ShowWindow(SW_HIDE);
}
