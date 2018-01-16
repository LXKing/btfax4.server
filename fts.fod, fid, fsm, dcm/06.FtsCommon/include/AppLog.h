#include "Log.h"

#pragma once
class CAppLog : public CLog
{
public:
	static CAppLog* Inst();
protected:
	static CAppLog* s_pInstance;

public:
	typedef void (*PFN_DISPLAY_LOG)( const char* p_szLog );
	typedef void (*PFN_INCREMENT_ERROR)();

private:
	CAppLog(void);
	virtual ~CAppLog(void);

public:
	void SetCallback( PFN_DISPLAY_LOG pfnDisplayLog, PFN_INCREMENT_ERROR p_pfnIncrementError );

	bool Print( int DebugLevel, char* Format, ...  );

protected:
	PFN_DISPLAY_LOG		m_pfnDisplayLog;
	PFN_INCREMENT_ERROR m_pfnIncrementError;

};

