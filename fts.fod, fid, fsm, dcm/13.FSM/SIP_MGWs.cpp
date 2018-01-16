#include "stdafx.h"
#include "SIP_MGWs.h"
#include "SIP_Manager.h"
#include "Debug.h"


SINGLETON_IMPLEMENT( SIP_Mgws );


SIP_Mgws::SIP_Mgws()
{
}

SIP_Mgws::~SIP_Mgws()
{
}


void SIP_Mgws::RegistMgw( bool p_bPrimary, const char* p_szIp, int p_port )
{
	{CSingleLock Locker( &m_Lock, TRUE );

		_RegistMgw( p_bPrimary, p_szIp, p_port );
	}
}
void SIP_Mgws::UnregistMgw( const char* p_szIp, int p_port )
{
	MGW_KEY			MgwKey( p_szIp, p_port );
	MGWS::iterator	pos;

	{CSingleLock Locker( &m_Lock, TRUE );

		_UnregistMgw( true, MgwKey );
		_UnregistMgw( false, MgwKey );
	}
}

bool SIP_Mgws::HuntMgw( bool* p_pbPrimary, CString* p_pstrIp, int* p_pnPort )
{
	MGW Mgw;

	if( !HuntMgw( &Mgw ) )
		return false;

	*p_pbPrimary = Mgw.bPrimary;
	*p_pstrIp	 = Mgw.strIp.c_str();
	*p_pnPort	 = Mgw.nPort;

	return true;
}

bool SIP_Mgws::HuntMgw( MGW* p_pMgw )
{
	{CSingleLock Locker( &m_Lock, TRUE );

		if( _HuntMgw( true, p_pMgw ) )
			return true;

		if( _HuntMgw( false, p_pMgw ) )
			return true;
	}

	return false;
}

bool SIP_Mgws::GetMgw( const char* p_szIp, int p_port, bool* p_pbPrimary )
{
	MGW Mgw;

	if( !GetMgw( p_szIp, p_port, &Mgw ) )
		return false;

	*p_pbPrimary = Mgw.bPrimary;

	return true;
}


bool SIP_Mgws::GetMgw( const char* p_szIp, int p_port, MGW* p_pMgw )
{
	MGW_KEY	MgwKey( p_szIp, p_port );
	
	{CSingleLock Locker( &m_Lock, TRUE );

		if( !SIP_Mgws::_GetMgw( true, MgwKey, p_pMgw ) )
		{
			if( !SIP_Mgws::_GetMgw( false, MgwKey, p_pMgw ) )
				return false;
		}
	}

	return true;
}

void SIP_Mgws::SendAlive()
{
	if( CConfig::ADDRESS_MGW_ALIVE_SND_TIME <= 0 )
		return;

	{CSingleLock Locker( &m_Lock, TRUE );

		_SendAlive( true );
		_SendAlive( false );
	}
}

void SIP_Mgws::CheckAlvie()
{
	if( CConfig::ADDRESS_MGW_ALIVE_CHK_TIME <= 0 )
		return;

	{CSingleLock Locker( &m_Lock, TRUE );

		_CheckAlive( true );
		_CheckAlive( false );
	}
}

void SIP_Mgws::UpdateSendTime( const char* p_szIp, int p_port )
{
	MGW_KEY			MgwKey( p_szIp, p_port );
	MGWS::iterator	pos;

	{CSingleLock Locker( &m_Lock, TRUE );
		
		if( !_UpdateTime( false, false, MgwKey ) )
			_UpdateTime( true, false, MgwKey );
	}

}

void SIP_Mgws::UpdateRecvTime( const char* p_szIp, int p_port )
{
	MGW_KEY			MgwKey( p_szIp, p_port );
	MGWS::iterator	pos;

	{CSingleLock Locker( &m_Lock, TRUE );

		if( !_UpdateTime( true, true, MgwKey ) )
			_UpdateTime( false, true, MgwKey );
	}

}

inline void SIP_Mgws::_RegistMgw( bool p_bPrimary, const char* p_szIp, int p_port )
{
	MGWS&	MgwsMap  = (p_bPrimary) ? m_Primarys : m_Backups;
	MGW_KEY	MgwKey( p_szIp, p_port );
	MGW		Mgw;
	
	Mgw.bPrimary	= p_bPrimary;
	Mgw.strIp		= MgwKey.strIp;
	Mgw.nPort		= p_port;
	Mgw.tRecv		= time( NULL );
	Mgw.tSend		= Mgw.tRecv;
	
	Mgw.bAliveCheked = false;
	if( CConfig::ADDRESS_MGW_ALIVE_CHK_TIME > 0 ) 
		Mgw.bAlive = false;
	else
		Mgw.bAlive = true;
	

	MgwsMap[ MgwKey ] = Mgw;
	_RebuildMgwsList( p_bPrimary );
}

inline void SIP_Mgws::_UnregistMgw( bool p_bPrimary, const MGW_KEY& p_MgwKey )
{
	MGWS&			Mgws = (p_bPrimary) ? m_Primarys : m_Backups;
	MGWS::iterator	pos;

	pos = Mgws.find( p_MgwKey );
	if( pos != Mgws.end() )
		Mgws.erase( pos );

	_RebuildMgwsList( p_bPrimary );
}

inline void SIP_Mgws::_RebuildMgwsList( bool p_bPrimary )
{
	MGWS&			Mgws		= (p_bPrimary) ? m_Primarys : m_Backups;
	MGWKEY_LIST&	MgwKeyList	= (p_bPrimary) ? m_PrimaryList : m_BackupList;
	int&			huntIdx		= (p_bPrimary) ? m_huntPrimary : m_huntBackup;

	MgwKeyList.clear();
	huntIdx = -1;

	MGWS::iterator pos		= Mgws.begin();
	MGWS::iterator posEnd	= Mgws.end();
	for( ; pos != posEnd ; ++pos )
	{
		MgwKeyList.push_back( pos->first );
	}
}

inline bool SIP_Mgws::_GetMgw( bool p_bPrimary, const MGW_KEY& p_MgwKey, MGW* p_pMgw )
{
	MGWS&			Mgws = (p_bPrimary) ? m_Primarys : m_Backups;
	MGWS::iterator	pos;
	
	pos = Mgws.find( p_MgwKey );
	if( pos == Mgws.end() )
		return false;
	
	MGW& Mgw = pos->second;
	*p_pMgw = Mgw;
	return true;
}

// p_bRecv : true  - 수신 시각 업데이트
//           false - 송신 시각 업데이트
inline bool SIP_Mgws::_UpdateTime( bool p_bPrimary, bool p_bRecv, const MGW_KEY& p_MgwKey )
{
	MGWS&			Mgws = (p_bPrimary) ? m_Primarys : m_Backups;
	MGWS::iterator	pos;

	pos = Mgws.find( p_MgwKey );
	if( pos == Mgws.end() )
		return false;

	MGW& Mgw = Mgws[ p_MgwKey ];
	
	if( !p_bRecv ) // 송신
	{
		Mgw.tSend = time( NULL );
		return true;
	}
	else // 수신
	{
		Mgw.tRecv  = time( NULL );

		/// MGW 업 설정
		if( !Mgw.bAlive )
		{
			Mgw.bAliveCheked = true; // ALIVE 체크함
			Mgw.bAlive = true;		 // MGW UP됨

			struct tm tmDate;
			CString strLastRecvTime;
			
			// 로그 기록
			tmDate = *localtime( &Mgw.tRecv );
			strLastRecvTime.Format( "%d/%d/%d %d:%d:%d", tmDate.tm_year + 1900, tmDate.tm_mon + 1, tmDate.tm_mday, tmDate.tm_hour, tmDate.tm_min, tmDate.tm_sec );
			
			APPLOG->Print( DBGLV_MAJOR, "[MGW ALIVE CHECK] MGW UP. IP(%s) Port(%d) Primary(%d) LastRecvTime(%s)", 
							Mgw.strIp.c_str(), Mgw.nPort, p_bPrimary, (LPCSTR)strLastRecvTime );

			// Event 테이블 등록
			CString strEvent, strEventDetail;
			strEvent.Format( "%s-%s:%d", MGW_EVENT, Mgw.strIp.c_str(), Mgw.nPort );
			if( Mgw.bPrimary )
				strEventDetail.Format( "%s:%d:PRIMARY", Mgw.strIp.c_str(), Mgw.nPort );
			else
				strEventDetail.Format( "%s:%d:BACKUP", Mgw.strIp.c_str(), Mgw.nPort );
			CDbModule::Inst()->InsertAlarmEvent( MGW_EVENTTYPE, strEvent, MGW_UP, strEventDetail );
		}
	}

	return true;
}

inline bool SIP_Mgws::_HuntMgw( bool p_bPrimary, MGW* p_pMgw )
{
	MGWS&			Mgws		= (p_bPrimary) ? m_Primarys : m_Backups;
	MGWKEY_LIST&	MgwKeyList	= (p_bPrimary) ? m_PrimaryList : m_BackupList;
	int&			huntIdx		= (p_bPrimary) ? m_huntPrimary : m_huntBackup;
	int				cnt;
	
	cnt = MgwKeyList.size();
	for( int i = 0 ; i < cnt ; ++i )
	{
		huntIdx	= ( huntIdx >= cnt - 1 ) ? 0 : huntIdx + 1;

		MGW_KEY&		MgwKey	= MgwKeyList[ huntIdx ];
		MGWS::iterator	pos		= Mgws.find( MgwKey );
		if( pos == Mgws.end() )
			continue;

		if( !pos->second.bAlive )
			continue;

		*p_pMgw = pos->second;
		APPLOG->Print( DBGLV_INF, "Hunt MGW. Primary=%d IP=%s, Port=%d", p_pMgw->bPrimary, p_pMgw->strIp.c_str(), p_pMgw->nPort );
		return true;
	}

	return false;
}

inline void SIP_Mgws::_SendAlive( bool p_bPrimary )
{
	MGWS&			Mgws = (p_bPrimary) ? m_Primarys : m_Backups;
	MGWS::iterator	pos, posEnd;
	time_t				tCurrent;

	tCurrent = time( NULL );
	pos		= Mgws.begin();
	posEnd	= Mgws.end();
	for( ; pos != posEnd ; ++pos )
	{
		MGW&  Mgw = pos->second;

		if( CConfig::ADDRESS_MGW_ALIVE_SND_TIME > 0 &&
			tCurrent > Mgw.tSend + CConfig::ADDRESS_MGW_ALIVE_SND_TIME &&
			tCurrent > Mgw.tRecv + CConfig::ADDRESS_MGW_ALIVE_SND_TIME    )
		{
			if( SIP_Manager::inst->OutdialogOptions( Mgw.strIp.c_str(), Mgw.nPort, "111" ) == 0 )
				pos->second.tSend = tCurrent;
		}
	}
}

inline void SIP_Mgws::_CheckAlive( bool p_bPrimary )
{
	MGWS&			Mgws = (p_bPrimary) ? m_Primarys : m_Backups;
	MGWS::iterator	pos, posEnd;
	time_t				tCurrent;

	//APPLOG->Print( DBGLV_INF, "[ALIVE CHECK] Start" );

	tCurrent = time( NULL );
	pos		= Mgws.begin();
	posEnd	= Mgws.end();
	for( ; pos != posEnd ; ++pos )
	{
		MGW&  Mgw = pos->second;

		/// MGW 다운 체크
		if( ( !Mgw.bAliveCheked || Mgw.bAlive ) &&
			CConfig::ADDRESS_MGW_ALIVE_CHK_TIME > 0 &&
			tCurrent > Mgw.tRecv + CConfig::ADDRESS_MGW_ALIVE_CHK_TIME )
		{
			Mgw.bAliveCheked = true;  // ALIVE 체크함
			Mgw.bAlive		 = false; // MGW DOWN 됨

			struct tm tmDate;
			CString strLastRecvTime;
			CString strCurrentTime;
			
			// 로그 기록
			tmDate = *localtime( &Mgw.tRecv );
			strLastRecvTime.Format( "%d/%d/%d %d:%d:%d", tmDate.tm_year + 1900, tmDate.tm_mon + 1, tmDate.tm_mday, tmDate.tm_hour, tmDate.tm_min, tmDate.tm_sec );

			tmDate = *localtime( &tCurrent );
			strCurrentTime.Format( "%d/%d/%d %d:%d:%d", tmDate.tm_year + 1900, tmDate.tm_mon + 1, tmDate.tm_mday, tmDate.tm_hour, tmDate.tm_min, tmDate.tm_sec );

			APPLOG->Print( DBGLV_MAJOR, "[MGW ALIVE CHECK] MGW DOWN. IP(%s) Port(%d) Primary(%d) LastRecvTime(%s) CurrentTime(%s) AliveCheckTime(%d)", 
							Mgw.strIp.c_str(), Mgw.nPort, p_bPrimary, 
							(LPCSTR)strLastRecvTime, (LPCSTR)strCurrentTime, CConfig::ADDRESS_MGW_ALIVE_CHK_TIME );

			// 이벤트 DB 등록
			CString	strEvent, strEventDetail;
			strEvent.Format( "%s-%s:%d", MGW_EVENT, Mgw.strIp.c_str(), Mgw.nPort );
			if( Mgw.bPrimary )
				strEventDetail.Format( "%s:%d:PRIMARY", Mgw.strIp.c_str(), Mgw.nPort );
			else
				strEventDetail.Format( "%s:%d:BACKUP", Mgw.strIp.c_str(), Mgw.nPort );
			CDbModule::Inst()->InsertAlarmEvent( MGW_EVENTTYPE, strEvent, MGW_DOWN, strEventDetail );
		}
	}

	//APPLOG->Print( DBGLV_INF, "[ALIVE CHECK] End" );
}




