#include "StdAfx.h"
#include "AppLog.h"

CAppLog* CAppLog::s_pInstance = NULL;

CAppLog* CAppLog::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new CAppLog;

	return s_pInstance;
}



CAppLog::CAppLog(void)
{
	m_pfnDisplayLog		= NULL;
	m_pfnIncrementError = NULL;
}


CAppLog::~CAppLog(void)
{
}



void CAppLog::SetCallback( PFN_DISPLAY_LOG pfnDisplayLog, PFN_INCREMENT_ERROR p_pfnIncrementError )
{
	m_pfnDisplayLog		=  pfnDisplayLog;
	m_pfnIncrementError = p_pfnIncrementError;
}

bool CAppLog::Print(int DebugLevel, char* Format, ...)
{	
	if( DebugLevel <= DBGLV_ERR2 )
	{
		if( m_pfnIncrementError )
			m_pfnIncrementError();
	}
	
	char	Buff[MAX_LINE_CHAR_SIZE];	
	va_list	Args;

	memset(Buff, 0x00, MAX_LINE_CHAR_SIZE);
	
	va_start( Args, Format );
	vsprintf_s( Buff, Format, Args );
	va_end( Args );

	CString strLogLine;
	if( !_print( &strLogLine, DebugLevel, Buff ) )
		return false;

	/*if( m_pfnDisplayLog )
		m_pfnDisplayLog( strLogLine );*/

	return true;
}

