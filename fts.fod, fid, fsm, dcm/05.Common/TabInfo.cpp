// TabInfo.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "afxdialogex.h"

#include "APP.h"
#include "Config.h"
#include "TabInfo.h"
#include "Utility.h"




IMPLEMENT_DYNAMIC(CTabInfo, CDialogEx)



// static
CTabInfo* CTabInfo::s_this = NULL;

void CTabInfo::IncrementError()
{
	++s_this->m_nErrorCnt_total;
	++s_this->m_nErrorCnt_today;
}

void CTabInfo::DBPolling()
{
	++s_this->m_nDBPolling;
	if( s_this->m_nDBPolling > 4 )
		s_this->m_nDBPolling = 1;
}


// method
CTabInfo::CTabInfo(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTabInfo::IDD, pParent)
{
	s_this				= this;

	m_nErrorCnt_total	= 0;
	m_nErrorCnt_today	= 0;
	m_nDBPolling		= 0;
}

CTabInfo::~CTabInfo()
{
}



BOOL CTabInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ctStartTime = CTime::GetCurrentTime();

	_InitList();
	SetTimer(TIMER_UPDATE_UI, 1000, NULL );

	return TRUE;
}


void CTabInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_INFO, m_ctlInfo);
}


BEGIN_MESSAGE_MAP(CTabInfo, CDialogEx)
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP( CTabInfo ) 
  EASYSIZE( IDC_LIST_INFO,	ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0 )
END_EASYSIZE_MAP

void CTabInfo::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	UPDATE_EASYSIZE;
}

void CTabInfo::OnTimer(UINT_PTR nIDEvent)
{
	static int	 s_nDay = 0;
	static CTime ctCurTime;

	// 일자 변경시, 처리
	ctCurTime = CTime::GetCurrentTime();
	if( ctCurTime.GetDay() != s_nDay )
	{
		s_nDay = ctCurTime.GetDay();
		m_nErrorCnt_today = 0;
	}

	// 타이머 처리
	switch( nIDEvent )
	{
	case TIMER_UPDATE_UI:
		_SetInfo( ROW_CURRENT_TIME, CTime::GetCurrentTime().Format(TIME_DISPLAY_FORMAT) );
		_SetInfo( ROW_TODAY_ERROR, m_nErrorCnt_today );
		_SetInfo( ROW_TOTAL_ERROR, m_nErrorCnt_total );
		_SetInfo( ROW_DB_POLLING, m_nDBPolling );
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}



void CTabInfo::_InitList()
{
	CRect		rect;
	
	GetClientRect( &rect );
	m_ctlInfo.SetWindowPos( NULL, 2, 4, rect.Width() - 5, rect.Height() - 4, SWP_SHOWWINDOW | SWP_NOZORDER );
	m_ctlInfo.SetExtendedStyle( LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
 
	m_ctlInfo.InsertColumn( 0, "Item",			LVCFMT_LEFT, 100 );
	m_ctlInfo.InsertColumn( 1, "Information",	LVCFMT_LEFT, 500 );
}

void CTabInfo::_InitInfo()
{
	m_ctlInfo.InsertItem(  ROW_SYSTEM_PROCESS_ID,	"Process ID" );
	m_ctlInfo.SetItemText( ROW_SYSTEM_PROCESS_ID, 1, CConfig::SYSTEM_PROCESS_ID );

	m_ctlInfo.InsertItem(  ROW_COMPILE_TIME,		"Compile" );
	m_ctlInfo.SetItemText( ROW_COMPILE_TIME, 1,		CUtility::GetCompileTime() );

	m_ctlInfo.InsertItem(  ROW_START_TIME,			"Start Time" );
	m_ctlInfo.SetItemText( ROW_START_TIME, 1,		m_ctStartTime.Format(TIME_DISPLAY_FORMAT) );

	m_ctlInfo.InsertItem(  ROW_CURRENT_TIME,		"Current Time" );
	m_ctlInfo.SetItemText( ROW_CURRENT_TIME, 1,		m_ctStartTime.Format(TIME_DISPLAY_FORMAT) );
	
	m_ctlInfo.InsertItem(  ROW_TODAY_ERROR,			"Today Error" );
	m_ctlInfo.SetItemText( ROW_TODAY_ERROR, 1,		"0" );

	m_ctlInfo.InsertItem(  ROW_TOTAL_ERROR,			"Total Error" );
	m_ctlInfo.SetItemText( ROW_TOTAL_ERROR, 1,		"0" );

	m_ctlInfo.InsertItem(  ROW_DB_POLLING,			"DB Polling" );
	m_ctlInfo.SetItemText( ROW_DB_POLLING, 1,		"" );
}

void CTabInfo::_SetInfo( int p_nRow, const char* p_szInfo )
{
	static bool bInit = false;

	if( !bInit )
	{
		bInit = true;
		_InitInfo();
	}

	m_ctlInfo.SetItemText( p_nRow, 1, p_szInfo );
}

void CTabInfo::_SetInfo( int p_nRow, int p_nInfo )
{
	CString strInfo;
	
	if( p_nRow == ROW_DB_POLLING )
	{
		switch(p_nInfo)
		{
			case 1:
				strInfo.Format( "|");
				break;
			case 2:
				strInfo.Format( "/");
				break;
			case 3:
				strInfo.Format( "-");
				break;
			case 4:
				strInfo.Format( "\\");
				break;
		}
	}
	else
	{
		strInfo.Format( "%d", p_nInfo );
	}

	_SetInfo( p_nRow, strInfo );
}







