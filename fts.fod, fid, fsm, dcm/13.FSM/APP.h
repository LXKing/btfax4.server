
// FSM.h : PROJECT_NAME 응용 프로그램에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"


class CFSMApp : public CWinApp
{
public:
	CFSMApp();

	void	DisplayState(int chIdx, EN_STATUS status, CString szMsg );
	void	DisplayLog(char* Format, ...);

// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.

	DECLARE_MESSAGE_MAP()
};

extern CFSMApp theApp;