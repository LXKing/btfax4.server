// TabUiLog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "afxdialogex.h"
#include <afxmt.h>

#include "APP.h"
#include "TabUiLog.h"




IMPLEMENT_DYNAMIC(CTabUiLog, CDialogEx)

// static
CTabUiLog* CTabUiLog::s_this = NULL;
void CTabUiLog::AddLog( const char* p_szLog )
{
	if( !s_this ) return;

	{ CSingleLock Locker( &s_this->m_Lock, TRUE );
			
		s_this->m_vecLog.push_back( p_szLog );
	}
}

CTabUiLog::CTabUiLog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTabUiLog::IDD, pParent)
{
	s_this = this;
}

CTabUiLog::~CTabUiLog()
{
}


BOOL CTabUiLog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetTimer( TIMER_UPDATE_UI, 500, NULL );
	_InitList();

	return TRUE;
}


void CTabUiLog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_ctlLog);
}


BEGIN_MESSAGE_MAP( CTabUiLog, CDialogEx )
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP( CTabUiLog ) 
  EASYSIZE( IDC_LIST_LOG,	ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0 )
END_EASYSIZE_MAP

void CTabUiLog::OnSize( UINT nType, int cx, int cy )
{
	CDialogEx::OnSize(nType, cx, cy);

	UPDATE_EASYSIZE;
}

void CTabUiLog::OnTimer(UINT_PTR nIDEvent)
{
	switch( nIDEvent )
	{
	case TIMER_UPDATE_UI:
		{ CSingleLock Locker( &m_Lock, TRUE );
			
			for( int i = 0 ; i < m_vecLog.size() ; ++i )
				_addLog( m_vecLog[i].c_str() );
			m_vecLog.clear();
		}
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CTabUiLog::_InitList()
{
	CRect		rect;
	
	GetClientRect( &rect );
	m_ctlLog.SetWindowPos( NULL, 2, 4, rect.Width() - 5, rect.Height() - 4, SWP_SHOWWINDOW | SWP_NOZORDER );
}

void CTabUiLog::_addLog( const char* p_szLog )
{
	if( m_ctlLog.GetCount() >= 100 )
		m_ctlLog.DeleteString( 0 );

	m_ctlLog.AddString( p_szLog );
	m_ctlLog.SetCurSel( m_ctlLog.GetCount() - 1 );
}


