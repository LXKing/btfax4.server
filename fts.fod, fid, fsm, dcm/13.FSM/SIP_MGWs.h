#pragma once

#include "ClassUtility.h"
#include <string>
#include <map>



class SIP_Mgws
{
	SINGLETON_DECLARE( SIP_Mgws );

	//// TYPE ////
public:
	struct MGW_KEY
	{
		string	strIp;
		int		port;
		
		inline MGW_KEY( const char* p_szIp, int p_port )
		{
			strIp = p_szIp;
			port  = p_port;
		}

		inline bool operator<( const MGW_KEY& p_Other ) const
		{
			if( strIp == p_Other.strIp )
				return ( port < p_Other.port );

			return ( strIp < p_Other.strIp );
		}
	};
	struct MGW
	{
		bool	bPrimary;
		string	strIp;
		int		nPort;

		time_t	tRecv;
		time_t	tSend;
		bool	bAlive;
		bool	bAliveCheked; // false - ALIVE 체크해 보지 않음. true - ALIVE 체크해 봄
	};
	typedef map< MGW_KEY, MGW >	MGWS;
	typedef vector< MGW_KEY >	MGWKEY_LIST;

	//// Variable ////
protected:
	MGWS				m_Primarys;
	MGWKEY_LIST			m_PrimaryList;
	int					m_huntPrimary;
	MGWS				m_Backups;
	MGWKEY_LIST			m_BackupList;
	int					m_huntBackup;
	CCriticalSection	m_Lock;
	

	//// Method ////
public:
	SIP_Mgws();
	virtual ~SIP_Mgws();

	void RegistMgw( bool p_bPrimary, const char* p_szIp, int p_port );
	void UnregistMgw( const char* p_szIp, int p_port );

	bool HuntMgw( bool* p_pbPrimary, CString* p_pstrIp, int* p_pnPort );
	bool HuntMgw( MGW* p_pMgw );

	bool GetMgw( const char* p_szIp, int p_port, bool* p_pbPrimary );
	bool GetMgw( const char* p_szIp, int p_port, MGW* p_pMgw );
	
	void SendAlive();
	void CheckAlvie();
	void UpdateSendTime( const char* p_szIp, int p_port );
	void UpdateRecvTime( const char* p_szIp, int p_port );


	//// Implementation ////
protected:
	inline void _RegistMgw( bool p_bPrimary, const char* p_szMgwIp, int p_nPort );
	inline void _UnregistMgw( bool p_bPrimary, const MGW_KEY& p_MgwKey );
	inline void _RebuildMgwsList( bool p_bPrimary );
	inline bool _GetMgw( bool p_bPrimary, const MGW_KEY& p_MgwKey, MGW* p_pMgw );
	inline bool _UpdateTime( bool p_bPrimary, bool p_bRecv, const MGW_KEY& p_MgwKey );
	inline bool _HuntMgw( bool p_bPrimary, MGW* p_pMgw );
	inline void _SendAlive( bool p_bPrimary );
	inline void _CheckAlive( bool p_bPrimary );
};
