
// FOD.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CFODApp:
// �� Ŭ������ ������ ���ؼ��� FOD.cpp�� �����Ͻʽÿ�.
//

class CFODApp : public CWinApp
{
// method
public:
	CFODApp();
	bool IsStart();
	bool Service_Init();
	bool Service_Start();
	void Service_Stop();

// override.
public:
	virtual BOOL InitInstance();

// implement
	DECLARE_MESSAGE_MAP()

// field
private:
	bool m_bServiceRun;
};

extern CFODApp theApp;