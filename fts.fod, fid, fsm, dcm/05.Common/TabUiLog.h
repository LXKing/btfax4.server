#pragma once
#include "afxwin.h"
#include "EasySize.h"

#include <vector>
using namespace std;


class CTabUiLog : public CDialogEx
{
	DECLARE_DYNAMIC(CTabUiLog)
	DECLARE_EASYSIZE

	// definition
public:
	enum 
	{ 
		IDD = IDD_TAB_UI_LOG 
	};
	enum 
	{ 
		TIMER_UPDATE_UI = 101 
	};


	// static
protected:
	static CTabUiLog* s_this;
public:
	static void AddLog( const char* p_szLog );


	// method
public:
	CTabUiLog( CWnd* pParent = NULL );
	virtual ~CTabUiLog();

	
	// override
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);


	// message handler
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	// implement
	void _InitList();
	void _addLog( const char* p_szLog );


	// field
public:
	CListBox m_ctlLog;
	
	vector<string>		m_vecLog;
	CCriticalSection	m_Lock;
};
