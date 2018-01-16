#pragma once
#include "afxcmn.h"
#include "EasySize.h"



class CTabInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CTabInfo)
	DECLARE_EASYSIZE

// definition
public:
	enum 
	{ 
		IDD = IDD_TAB_INFO 
	};
	enum 
	{ 
		TIMER_UPDATE_UI = 101 
	};
	enum 
	{ 
		ROW_SYSTEM_PROCESS_ID = 0,
		ROW_COMPILE_TIME,
		ROW_START_TIME,
		ROW_CURRENT_TIME,
		ROW_TODAY_ERROR,
		ROW_TOTAL_ERROR, 
		ROW_DB_POLLING
	};
	#define TIME_DISPLAY_FORMAT "%Y/%m/%d %H:%M:%S"

	// static
protected:
	static CTabInfo* s_this;
public:
	static void IncrementError();
	static void DBPolling();

// method
public:
	CTabInfo( CWnd* pParent = NULL );
	virtual ~CTabInfo();
	
// ovveride
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

// message handler
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

// Implement
protected:
	void _InitList();
	void _InitInfo();
	void _SetInfo( int p_nRow, const char* p_szInfo );
	void _SetInfo( int p_nRow, int p_nInfo );

// field
public:
	CListCtrl	m_ctlInfo;

	CString		m_strCompileTime;
	CTime		m_ctStartTime;
	int			m_nErrorCnt_total;
	int			m_nErrorCnt_today;
	int			m_nDBPolling;
};
