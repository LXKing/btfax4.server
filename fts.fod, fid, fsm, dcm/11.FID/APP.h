
// FID.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CFIDApp:
// �� Ŭ������ ������ ���ؼ��� FID.cpp�� �����Ͻʽÿ�.
//

class CFIDApp : public CWinApp
{
// method
public:
	CFIDApp();
	bool IsStart();
	bool Service_Init();
	bool Service_Start();
	void Service_Stop();

// ovveride
public:
	virtual BOOL InitInstance();

// implement
	DECLARE_MESSAGE_MAP()

// field
private:
	bool m_bServiceRun;
};

extern CFIDApp theApp;