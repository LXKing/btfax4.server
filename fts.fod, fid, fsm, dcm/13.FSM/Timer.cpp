#include "stdafx.h"
#include "Timer.h"


CTimer::CTimer()
{
	m_bRun = false;
}

CTimer::~CTimer()
{
	;
}

void CTimer::Start( int p_nTime )
{
	m_nTime = p_nTime;

	m_bRun = true;
	::CreateThread( NULL, 0, _ThreadEntry, (void*)this, 0, &m_thread );
}

void CTimer::Stop()
{
	m_bRun = false;
}

DWORD CTimer::_ThreadEntry( void* pParam )
{
	CTimer* pThis = (CTimer*)pParam;
	return (DWORD)pThis->_TimerThread();
}

int CTimer::_TimerThread()
{
	while( m_bRun )
	{
		Sleep( m_nTime );
		m_bRun = _onTimer();
	}

	return NULL;
}

