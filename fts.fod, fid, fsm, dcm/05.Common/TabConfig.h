#pragma once
#include "afxcmn.h"
#include "EasySize.h"


class CTabConfig : public CDialogEx
{
	DECLARE_DYNAMIC(CTabConfig)
	DECLARE_EASYSIZE


	// Definition
public:
	enum { IDD = IDD_TAB_CONFIG };


	// Callback
public:
	static CTabConfig* s_This;
	static void DisplayConfig( const char* p_szKey, const char* p_szValue );


	// Constructor, Destructor
public:
	CTabConfig(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CTabConfig();


	// Method
public:
	void AddItem( const char* p_szKey, const char* p_szValue );


	// Override
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);


	// Message Handler
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);


	// Implementation
public:
	void _InitList();


	// Fields
public:
	CListCtrl m_ctlConfig;
};
