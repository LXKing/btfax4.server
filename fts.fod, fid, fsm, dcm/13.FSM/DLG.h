
// FSMDlg.h : 헤더 파일
//

#pragma once
#include "afxcmn.h"
#include "TabConfig.h"
#include "TabUiLog.h"
#include "TabInfo.h"



class CFSMDlg : public CDialogEx
{
	DECLARE_EASYSIZE

public:
	// Definition
	enum { IDD = IDD_FSM_DIALOG };


	// Constructor, Destructor
public:
	CFSMDlg(CWnd* pParent = NULL);


	// Override
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
	
	
	// Message Handler
protected:
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void m_ctlTab_OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileExit();
	afx_msg void OnTaskStart();
	afx_msg void OnTaskStop();
	afx_msg void OnTaskClearuilog();
	afx_msg void OnTaskLoadconfig();
	DECLARE_MESSAGE_MAP()


	// Implementation
protected:
	void _InitTab();


	// Field
public:
	HICON		m_hIcon;

	CTabCtrl	m_ctlTab;
	CTabConfig	m_tabConfig;
	CTabUiLog	m_tabUiLog;
	CTabInfo	m_tabInfo;
	CWnd*		m_pwndShow;
	
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
