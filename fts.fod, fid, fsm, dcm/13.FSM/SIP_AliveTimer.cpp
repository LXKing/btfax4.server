#include "stdafx.h"
#include "SIP_AliveTimer.h"
#include "SIP_Mgws.h"


SINGLETON_IMPLEMENT( SIP_AliveTimer );

SIP_AliveTimer::SIP_AliveTimer()
{
}

SIP_AliveTimer::~SIP_AliveTimer()
{
}


bool SIP_AliveTimer::_onTimer()
{
	SIP_Mgws::Inst()->CheckAlvie();
	SIP_Mgws::Inst()->SendAlive();

	return true;
}