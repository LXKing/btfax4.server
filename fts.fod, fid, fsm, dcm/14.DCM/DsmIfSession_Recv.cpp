#include "StdAfx.h"
#include <windows.h>
#include <iostream>
#include "DsmIfSession_Recv.h"
#include "DsmIfSession.h"


CDsmIfSession_Recv::CDsmIfSession_Recv(void)
{
}


CDsmIfSession_Recv::~CDsmIfSession_Recv(void)
{
}

//--------------------------------------------------------
// Title	: ThreadRun
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 세션 시작
//--------------------------------------------------------
NPVOID CDsmIfSession_Recv::ThreadRun(NPVOID p_arg)
{	
	CDsmIfSession *dsmIfSession = (CDsmIfSession*)p_arg;
	if(dsmIfSession == NULL)
		return 0;

	while(IsOn())
	{
		iSleep(ONESEC);

		dsmIfSession->Receive();
	}

	return 0;
}



