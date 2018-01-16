// TabConfig.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "APP.h"
#include "TabConfig.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(CTabConfig, CDialogEx)


// Callback
CTabConfig* CTabConfig::s_This = NULL;
void CTabConfig::DisplayConfig( const char* p_szKey, const char* p_szValue )
{
	int nRow;

	
	nRow = s_This->m_ctlConfig.InsertItem( s_This->m_ctlConfig.GetItemCount(), 
										   p_szKey );
	s_This->m_ctlConfig.SetItemText( nRow, 1, p_szValue );
}




CTabConfig::CTabConfig(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTabConfig::IDD, pParent)
{
	s_This = this;
}

CTabConfig::~CTabConfig()
{
}

void CTabConfig::AddItem( const char* p_szKey, const char* p_szValue )
{
	int nRow;

	nRow = m_ctlConfig.InsertItem( 0, p_szKey );
	m_ctlConfig.SetItemText( nRow, 1, p_szValue );
}


void CTabConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONFIG, m_ctlConfig);
}


BEGIN_MESSAGE_MAP(CTabConfig, CDialogEx)
	ON_WM_SIZE()
	ON_WM_SIZING()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP( CTabConfig ) 
  EASYSIZE( IDC_CONFIG,	ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0 )
END_EASYSIZE_MAP

void CTabConfig::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	UPDATE_EASYSIZE;
}


BOOL CTabConfig::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	_InitList();

	return TRUE;
}




void CTabConfig::_InitList()
{
	CRect		rect;
	
	GetClientRect( &rect );
	m_ctlConfig.SetWindowPos( NULL, 2, 4, rect.Width() - 5, rect.Height() - 4, SWP_SHOWWINDOW | SWP_NOZORDER );
	m_ctlConfig.SetExtendedStyle( LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
 
	m_ctlConfig.InsertColumn( 0, "Key",		LVCFMT_LEFT, 200 );
	m_ctlConfig.InsertColumn( 1, "Value",	LVCFMT_LEFT, 400 );
}



