#include "StdAfx.h"
#include <windows.h>
#include <iostream>
#include "DsmIfSession.h"
#include "DsmIfSession_HB.h"


CDsmIfSession_HB::CDsmIfSession_HB(void)
{
}

CDsmIfSession_HB::~CDsmIfSession_HB(void)
{
}

//--------------------------------------------------------
// Title	: ThreadRun
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 세션 시작
//--------------------------------------------------------
NPVOID CDsmIfSession_HB::ThreadRun(NPVOID p_arg)
{
	CDsmIfSession *dsmIfSession = (CDsmIfSession*)p_arg;
	if(dsmIfSession == NULL)
		return 0;

	while(IsOn())
	{
		iSleep(TENSEC);	
		dsmIfSession->SendHeartBit();
	}

	return 0;
}








