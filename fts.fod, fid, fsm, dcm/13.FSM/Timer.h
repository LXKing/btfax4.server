#pragma once

class CTimer
{
public:
	CTimer();
	~CTimer();

	void Start( int p_nTime );
	void Stop();

protected:
	virtual bool _onTimer() = 0;

protected:
	static DWORD WINAPI _ThreadEntry( void* pParam );
	int _TimerThread();

protected:
	int		m_nTime; // [ms]
	bool	m_bRun;
	DWORD	m_thread;
};


