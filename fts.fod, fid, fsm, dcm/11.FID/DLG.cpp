
// FIDDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "APP.h"
#include "DLG.h"
#include "Config.h"
#include "afxdialogex.h"
#include "FsmIfThread.h"

#include "DbConnection.h"
#include "StatusIni.h"
#include "Utility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFIDDlg 대화 상자




CFIDDlg::CFIDDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFIDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_ctlTab);
}

BEGIN_MESSAGE_MAP(CFIDDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CFIDDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFIDDlg::OnBnClickedCancel)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CFIDDlg::m_ctlTab_OnTcnSelchangeTab)
	ON_COMMAND(ID_FILE_EXIT, &CFIDDlg::OnFileExit)
	ON_COMMAND(ID_TASK_START, &CFIDDlg::OnTaskStart)
	ON_COMMAND(ID_TASK_STOP, &CFIDDlg::OnTaskStop)
	ON_COMMAND(ID_TASK_CLEARUILOG, &CFIDDlg::OnTaskClearuilog)
	ON_COMMAND(ID_TASK_LOADCONFIG, &CFIDDlg::OnTaskLoadconfig)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP( CFIDDlg ) 
  EASYSIZE( IDC_TAB,	ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0 )
END_EASYSIZE_MAP


// CFIDDlg 메시지 처리기

BOOL CFIDDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE );
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
	
	CFsmIfThread::Inst()->RcvPacket_Init( CConfig::FAX_START_CH, CConfig::FAX_CH_CNT );

	// P5. 오브젝트 초기화
	CStatusIni::Inst()->Initialize( CConfig::STATUS_FILE, 'I', CConfig::FAX_START_CH, CConfig::FAX_CH_CNT, 0, CConfig::FAX_CH_CNT - 1 );

	
	// P6. Log 초기화
	APPLOG->SetCallback( CTabUiLog::AddLog, CTabInfo::IncrementError);
	APPLOG->SetLogDirectory( CConfig::LOG_PATH );
	APPLOG->SetLogName( CConfig::SYSTEM_PROCESS_ID );
	APPLOG->SetFileSaveUnit( CConfig::LOG_SAVE_UNIT );
	APPLOG->SetDBGLV( CConfig::LOG_LEVEL );

	// P7. FID 시작 로그 기록
	APPLOG->Print( DBGLV_INF, "\n\n ******************** FID START ******************** \n" );
	config.DisplayAllConfig();

	// P8. BTFAX 초기화
	if( !gPAPP->Service_Init() )
	{
		Sleep( 1000 );
		AfxPostQuitMessage( 0 );
		return FALSE;
	}

	// P10. 자동 TASK 시작
	SendMessage( WM_COMMAND, MAKEWPARAM(ID_TASK_START,0), 0 );

	return TRUE;
}

void CFIDDlg::OnPaint()
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

void CFIDDlg::OnSize(UINT nType, int cx, int cy)
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

HCURSOR CFIDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFIDDlg::m_ctlTab_OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
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



void CFIDDlg::OnBnClickedOk()
{
}


void CFIDDlg::OnBnClickedCancel()
{
	/*if( AfxMessageBox( "종료하시겠습니까?", MB_YESNO ) != IDYES )
		return;*/

	CDialogEx::OnCancel();
}

void CFIDDlg::OnFileExit()
{
	CDialogEx::OnCancel();
}

void CFIDDlg::OnTaskStart()
{
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem( ID_TASK_START, MF_GRAYED  );
	pMenu->EnableMenuItem( ID_TASK_STOP,  MF_ENABLED );

	if( !gPAPP->Service_Start() )
		AfxPostQuitMessage( 0 );
}


void CFIDDlg::OnTaskStop()
{
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem( ID_TASK_START, MF_ENABLED );
	pMenu->EnableMenuItem( ID_TASK_STOP,  MF_GRAYED  );
}


void CFIDDlg::OnTaskClearuilog()
{	
}


void CFIDDlg::OnTaskLoadconfig()
{
}


void CFIDDlg::_InitTab()
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





void CFIDDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	// dialog hide	
	lpwndpos->flags &= ~SWP_SHOWWINDOW; 
	CDialogEx::OnWindowPosChanging(lpwndpos);
}
