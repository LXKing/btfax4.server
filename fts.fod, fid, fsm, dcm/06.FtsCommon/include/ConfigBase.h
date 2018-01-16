
#pragma once

#include "enum.h"
#include "DbModuleBase.h"

using namespace COMMON_LIB;

typedef void (*Config_Display) ( const char* p_szKey, const char* p_szValue ); 

class CConfigBase
{
public:
	CConfigBase(void);
	virtual ~CConfigBase(void);

	static void	SetCallback( Config_Display p_pfnCallback );

public:
	// [ 기본 정보 ]
	static CString				EXEC_PATH;
	static CString				CONFIG_FILE;
	static CString				STATUS_FILE;

public:
	// [ 공통 정보 ]
	static CString				APP_VER;
    static CString				SYSTEM_TYPE;
	static long					SYSTEM_NO;
    static CString				SYSTEM_ID;
	static CString				SYSTEM_IP;
    static P_TYPE				PROCESS_TYPE;
	static CString				PROCESS_TYPE_STR;
    static long					PROCESS_NO;
    static CString				PROCESS_ID;
    static CString				SYSTEM_PROCESS_ID;
    static CString				DB_CONNECTION_STRING;

protected:
	bool _ReadConfig( const char* p_szSection, const char* p_szKey, CString* p_pstrValue );
	bool _ReadConfig( const char* p_szSection, const char* p_szKey, int* p_pnValue );
	bool _ReadConfig( const char* p_szSection, const char* p_szKey, bool* p_pbValue );
	bool _ReadConfig( const char* p_szSection, const char* p_szKey, long* p_pnValue );

	bool _ReadDbConfig( const char* p_szKey, CString* p_pstrValue );
	bool _ReadDbConfig( const char* p_szKey, int* p_pnValue );
	bool _ReadDbConfig( const char* p_szKey, bool* p_pbValue );
	bool _ReadDbConfig( const char* p_szKey, CStringArray* p_psaValues );
	bool _ReadDbConfig( const char* p_szKey, UINT* p_pnValue );

	bool _WriteConfig( const char* p_szSection, const char* p_szKey, CString* p_pstrValue );
	bool _WriteConfig( const char* p_szSection, const char* p_szKey, const char* p_pstrValue );

protected:
	static Config_Display	s_pfnDisplayFn;
};

