
// FODDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "APP.h"
#include "DLG.h"
#include "Config.h"
#include "afxdialogex.h"

#include "DbConnection.h"
#include "StatusIni.h"
#include "Utility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFODDlg 대화 상자




CFODDlg::CFODDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFODDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFODDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_ctlTab);
}

BEGIN_MESSAGE_MAP(CFODDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CFODDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFODDlg::OnBnClickedCancel)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CFODDlg::m_ctlTab_OnTcnSelchangeTab)
	ON_COMMAND(ID_FILE_EXIT, &CFODDlg::OnFileExit)
	ON_COMMAND(ID_TASK_START, &CFODDlg::OnTaskStart)
	ON_COMMAND(ID_TASK_STOP, &CFODDlg::OnTaskStop)
	ON_COMMAND(ID_TASK_CLEARUILOG, &CFODDlg::OnTaskClearuilog)
	ON_COMMAND(ID_TASK_LOADCONFIG, &CFODDlg::OnTaskLoadconfig)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP( CFODDlg ) 
  EASYSIZE( IDC_TAB,	ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0 )
END_EASYSIZE_MAP



// CFODDlg 메시지 처리기

BOOL CFODDlg::OnInitDialog()
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

	// P6. Log 초기화
	APPLOG->SetCallback( CTabUiLog::AddLog, CTabInfo::IncrementError);
	APPLOG->SetLogDirectory( CConfig::LOG_PATH );
	APPLOG->SetLogName( CConfig::SYSTEM_PROCESS_ID );
	APPLOG->SetFileSaveUnit( CConfig::LOG_SAVE_UNIT );
	APPLOG->SetDBGLV( CConfig::LOG_LEVEL );
		
	// P7. DCM 시작 로그 기록
	APPLOG->Print( DBGLV_RPT, "\n\n ******************** DCM START ******************** \n" );
	
	config.DisplayAllConfig();
	
	
	// P9.자동 TASK 시작
	SendMessage( WM_COMMAND, MAKEWPARAM(ID_TASK_START,0), 0 );

	return TRUE;
}

void CFODDlg::OnPaint()
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

void CFODDlg::OnSize(UINT nType, int cx, int cy)
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

HCURSOR CFODDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CFODDlg::m_ctlTab_OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
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




void CFODDlg::OnBnClickedOk()
{	
}


void CFODDlg::OnBnClickedCancel()
{
	/*if( AfxMessageBox( "종료하시겠습니까?", MB_YESNO ) != IDYES )
		return;*/

	iSocket::WSCleanup();

	gPAPP->Service_Stop();
	CDialogEx::OnCancel();
}


void CFODDlg::OnFileExit()
{
	CDialogEx::OnCancel();
}


void CFODDlg::OnTaskStart()
{
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem( ID_TASK_START, MF_GRAYED  );
	pMenu->EnableMenuItem( ID_TASK_STOP,  MF_ENABLED );

	if( !gPAPP->Service_Start() )
		AfxPostQuitMessage( 0 );
}


void CFODDlg::OnTaskStop()
{
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem( ID_TASK_START, MF_ENABLED );
	pMenu->EnableMenuItem( ID_TASK_STOP,  MF_GRAYED  );
}


void CFODDlg::OnTaskClearuilog()
{	
}


void CFODDlg::OnTaskLoadconfig()
{
}


void CFODDlg::_InitTab()
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




void CFODDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	// dialog hide	
	lpwndpos->flags &= ~SWP_SHOWWINDOW; 
	CDialogEx::OnWindowPosChanging(lpwndpos);
}
