
// FODDlg.h : 헤더 파일
//

#pragma once

#include "afxcmn.h"
#include "EasySize.h"
#include "TabConfig.h"
#include "TabUiLog.h"
#include "TabInfo.h"

// CFODDlg 대화 상자
class CFODDlg : public CDialogEx
{
	DECLARE_EASYSIZE

// 생성입니다.
public:
	CFODDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_FOD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void m_ctlTab_OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileExit();
	afx_msg void OnTaskStart();
	afx_msg void OnTaskStop();
	afx_msg void OnTaskClearuilog();
	afx_msg void OnTaskLoadconfig();


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
};
