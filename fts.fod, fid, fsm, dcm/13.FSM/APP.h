
// FSM.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"


class CFSMApp : public CWinApp
{
public:
	CFSMApp();

	void	DisplayState(int chIdx, EN_STATUS status, CString szMsg );
	void	DisplayLog(char* Format, ...);

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CFSMApp theApp;