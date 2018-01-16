#pragma once

#include "Timer.h"
#include "ClassUtility.h"

class SIP_AliveTimer : public CTimer
{
	SINGLETON_DECLARE( SIP_AliveTimer );

public:
	SIP_AliveTimer();
	~SIP_AliveTimer();

protected:
	virtual bool _onTimer();
};
