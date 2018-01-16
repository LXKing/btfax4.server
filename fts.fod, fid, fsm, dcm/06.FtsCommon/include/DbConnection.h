#ifndef __DB_CONNECTION_H__
#define __DB_CONNECTION_H__

#pragma once

#include <afxmt.h>

#if defined( WINDOWS7 )
	// Windows7, 2008 server 
	#if defined( _WIN64 )
			#import "../../01.ado_win7/msado60_Backcompat_64.tlb" no_namespace rename("EOF", "EndOfFile")
	#else 
			#import "../../01.ado_win7/msado60_Backcompat_32.tlb" no_namespace rename("EOF", "EndOfFile")
	#endif
#else
	// WindowsXP, 2003 server
	#import "../../02.ado_xp/msado15.dll" no_namespace rename("EOF", "EndOfFile")
#endif




class CDbConnection
{
public:
	static CDbConnection* Inst();
protected:
	static CDbConnection* s_pInstance;

public:
	CDbConnection(void);
	virtual ~CDbConnection();

	void Initialize( const char* p_szConnString, bool m_bKeepSession = true );

	bool Open();
	void Close();
	void Disconnect();
	_ConnectionPtr Connection();

private:
	bool _open();
	void _disconnect();

private:
	bool				m_bKeepSession;
	bool				m_bConnected;		// Oracle 立加 咯何 (true/false)
	CString				m_ConnStr;			// Oracle 立加 String

	_ConnectionPtr		m_pConn;
	CCriticalSection	m_csLock;
};

#endif