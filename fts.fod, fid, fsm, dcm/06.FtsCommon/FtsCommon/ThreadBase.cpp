#include "stdafx.h"
#include "ThreadBase.h"


//// Static /////////////////////////////////////
bool CThreadBase::s_bReqStop = false;
void CThreadBase::ReqStopAll()
{
	s_bReqStop = true;
}


unsigned int CThreadBase::threadEntryTemp( void* p_pArg )
{
	CThreadBase* pThread = ( CThreadBase* ) p_pArg;
	
	pThread->init();

	pThread->m_bRun = true;
	pThread->onThreadEntry();
	pThread->m_bRun = false;

	return 0;
}



//// Constructor, Destructor ///////////////////

CThreadBase::CThreadBase()
{
	m_bRun		= false;
	m_bReqStop	= false;
}

CThreadBase::~CThreadBase()
{
}




//// Method /////////////////////////////////////

bool CThreadBase::Run()
{
	if( IsRun() )
		return false;

	AfxBeginThread( CThreadBase::threadEntryTemp, this );
	return true;
}

bool CThreadBase::IsRun()
{
	return m_bRun;
}

void CThreadBase::ReqStop()
{
	m_bReqStop = true;
}

bool CThreadBase::IsReqStop()
{
	return m_bReqStop;
}

bool CThreadBase::WaitStop( int p_nTimeout )
{
	int i;

	for( i = 0 ; i < p_nTimeout / 100 ; ++i )
	{
		if( !IsRun() )
			return true;

		Sleep( 100 );
	}

	return false;
}

void CThreadBase::init()
{
	s_bReqStop = false;
	m_bReqStop = false;
}





