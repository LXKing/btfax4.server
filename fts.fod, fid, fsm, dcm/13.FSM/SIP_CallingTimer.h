#pragma once

#include "Timer.h"
#include "ClassUtility.h"

#include <list>
#include <map>

class SIP_CallingTimer : public CTimer
{
	SINGLETON_DECLARE( SIP_CallingTimer );

protected:
	struct ITEM
	{
		int		chan;
		time_t	tCallStartTime;
	};

	typedef list< int >			CHAN_QUEUE;
	typedef map< int, ITEM >	ITEM_LIST;

public:
	SIP_CallingTimer();
	~SIP_CallingTimer();

	void Register( int p_chan );
	void Unregister( int p_chan );
	void PopTimeoutChannels( vector<int>* p_pChans );

protected:
	void _Register( int p_chan );
	void _Unregister( int p_chan );
	
	virtual bool _onTimer();
	
protected:
	bool				m_bRun;
	CCriticalSection	m_Lock;
	CHAN_QUEUE			m_Queue;
	ITEM_LIST			m_List;
	
	vector< int >		m_TimeoutChans;
};


