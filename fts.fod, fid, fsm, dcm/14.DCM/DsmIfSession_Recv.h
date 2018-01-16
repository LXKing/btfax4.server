#pragma once

#include <list>
#include <string.h>

// iCore..
#include "iLib.h"
#include "iSocket.h"
#include "iThread.h"
#include "iMutex.h"
#include "iType.h"
#include "iTimer.h"
// iCore


class CDsmIfSession_Recv: public iThread
{
public:
	CDsmIfSession_Recv(void);
	~CDsmIfSession_Recv(void);

	NPVOID ThreadRun(NPVOID pArg);
};

