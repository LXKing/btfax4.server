#include "stdafx.h"
#include "SIP_CallingTimer.h"

#include "SIP_Manager.h"



SINGLETON_IMPLEMENT( SIP_CallingTimer );

SIP_CallingTimer::SIP_CallingTimer()
{
	m_thread	= 0;
	m_bRun		= false;
}

SIP_CallingTimer::~SIP_CallingTimer()
{
}

void SIP_CallingTimer::Register( int p_chan )
{
	{CSingleLock Locker( &m_Lock, TRUE );
		
		_Unregister( p_chan );
		_Register( p_chan );
	}
}

void SIP_CallingTimer::Unregister( int p_chan )
{
	{CSingleLock Locker( &m_Lock, TRUE );

		_Unregister( p_chan );
	}
}

void SIP_CallingTimer::PopTimeoutChannels( vector<int>* p_pChans )
{
	int					chan;
	time_t				tCurrent;
	ITEM_LIST::iterator	pos;

	p_pChans->clear();

	{CSingleLock Locker( &m_Lock, TRUE );

		tCurrent = time( NULL );

		while( true )
		{
			if( m_Queue.empty() )
				return;

			chan = m_Queue.front();
			pos = m_List.find( chan );
			if( pos == m_List.end() )
			{
				m_Queue.pop_front();
				continue;
			}

			ITEM& Item = pos->second;
			if( tCurrent < Item.tCallStartTime + CConfig::SERVICE_CALLING_TIMEOUT )
				break;

			p_pChans->push_back( chan );
			m_Queue.pop_front();
			m_List.erase( pos );
		}
	}
}

void SIP_CallingTimer::_Register( int p_chan )
{
	ITEM Item;
	
	Item.chan			= p_chan;
	Item.tCallStartTime	= time( NULL );
	
	m_List[p_chan] = Item;
	m_Queue.push_back( p_chan );
}

void SIP_CallingTimer::_Unregister( int p_chan )
{
	ITEM_LIST::iterator pos;

	pos = m_List.find( p_chan );
	if( pos == m_List.end() )
		return;
	
	m_Queue.remove( p_chan );
	m_List.erase( pos );

}

bool SIP_CallingTimer::_onTimer()
{
	PopTimeoutChannels( &m_TimeoutChans );
	if( m_TimeoutChans.size() > 0 )
	{
		SIP_Manager::inst->CallingTimeout( m_TimeoutChans );
		m_TimeoutChans.clear();
	}

	return true;
}







