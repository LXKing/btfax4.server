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

class CDsmIfSession_HB: public iThread
{
public:
	CDsmIfSession_HB(void);
	~CDsmIfSession_HB(void);

	NPVOID ThreadRun(NPVOID pArg);

protected:
	


};

