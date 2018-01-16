
// FODDlg.h : ��� ����
//

#pragma once

#include "afxcmn.h"
#include "EasySize.h"
#include "TabConfig.h"
#include "TabUiLog.h"
#include "TabInfo.h"

// CFODDlg ��ȭ ����
class CFODDlg : public CDialogEx
{
	DECLARE_EASYSIZE

// �����Դϴ�.
public:
	CFODDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_FOD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	// ������ �޽��� �� �Լ�
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
