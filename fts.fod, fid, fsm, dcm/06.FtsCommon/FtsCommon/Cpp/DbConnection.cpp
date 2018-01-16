#include "StdAfx.h"
#include "DbConnection.h"


CDbConnection* CDbConnection::s_pInstance = NULL;

CDbConnection* CDbConnection::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new CDbConnection;

	return s_pInstance;
}



CDbConnection::CDbConnection()
{
	m_bConnected = false;
}


CDbConnection::~CDbConnection()
{
}

void CDbConnection::Initialize( const char* p_szConnString, bool m_bKeepSession )
{
	//p_szConnString = p_szConnString;
	m_ConnStr = p_szConnString;
	m_bKeepSession = m_bKeepSession;
}

bool CDbConnection::Open()
{
	m_csLock.Lock();

	if( !_open() )
		m_csLock.Unlock();

	return true;
}

void CDbConnection::Close()
{
	if( !m_bKeepSession )
		_disconnect();
	
	m_csLock.Unlock();
}

void CDbConnection::Disconnect()
{
	_disconnect();
	m_csLock.Unlock();
}


bool CDbConnection::_open()
{
	if( m_bConnected )
		return true;

	try
	{	
		//// KIMCG ////
		CoInitialize(NULL);
		m_pConn.CreateInstance( "ADODB.Connection" );
		m_pConn->ConnectionString = _bstr_t( m_ConnStr );

		//HRESULT hr = m_pConn->Open( "", "", "", adConnectUnspecified );
		HRESULT hr = m_pConn->Open( "", "", "", adConnectUnspecified );
		if( !SUCCEEDED(hr) )
		{
			// 확인필요
			if( m_pConn != NULL )
				m_pConn.Release();
			m_pConn = NULL;
			return false;
		}

		m_bConnected = true;
		m_pConn->CursorLocation = adUseClient;
	}

	catch( _com_error &e )
	{
		m_pConn = NULL;
		return false;
	}

	return true;
}

void CDbConnection::_disconnect()
{
	if( !m_bConnected )
		return;

	try
	{
		if( m_pConn != NULL )
		{
			m_pConn->Close();
			m_pConn.Release();
		}
	}
	catch( _com_error &e ) {}

	m_pConn = NULL;
	m_bConnected = false;
}

_ConnectionPtr CDbConnection::Connection()
{
	return m_pConn;
}
