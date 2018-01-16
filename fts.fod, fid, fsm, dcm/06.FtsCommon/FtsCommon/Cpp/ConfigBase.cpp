#include "StdAfx.h"
#include "ConfigBase.h"

Config_Display	CConfigBase::s_pfnDisplayFn = NULL;

CConfigBase::CConfigBase(void)
{
}


CConfigBase::~CConfigBase(void)
{
}

void CConfigBase::SetCallback( Config_Display p_pfnCallback )
{
	s_pfnDisplayFn = p_pfnCallback;
}

bool CConfigBase::_ReadConfig( const char* p_szSection, const char* p_szKey, CString* p_pstrValue )
{
	char szValue[1024];

	::GetPrivateProfileString( p_szSection, p_szKey, "__none__", szValue, sizeof(szValue), CConfigBase::CONFIG_FILE );
	if( !strcmp( szValue, "__none__") ) 
	{
		CString strMsg;

		//strMsg.Format( "'%s' 환경파일에서, [%s]를 읽지 못하였습니다.", (LPCSTR)CConfigBase::CONFIG_FILE, p_szKey );
		//AfxMessageBox( strMsg );
		return false;
	}

	*p_pstrValue = szValue;
	return true;
}

bool CConfigBase::_ReadConfig( const char* p_szSection, const char* p_szKey, int* p_pnValue )
{
	CString strValue = "";

	if( !_ReadConfig( p_szSection, p_szKey, &strValue ) )
		return false;

	*p_pnValue = atoi( strValue );
	return true;
}


bool CConfigBase::_ReadConfig( const char* p_szSection, const char* p_szKey, long* p_pnValue )
{
	CString strValue = "";

	if( !_ReadConfig( p_szSection, p_szKey, &strValue ) )
		return false;

	*p_pnValue = atoi( strValue );
	return true;
}

bool CConfigBase::_ReadConfig( const char* p_szSection, const char* p_szKey, bool* p_pbValue )
{
	CString strValue = "";

	if( !_ReadConfig( p_szSection, p_szKey, &strValue ) )
		return false;

	if( strValue == "Y" )
		*p_pbValue = true;
	else
		*p_pbValue = false;

	return true;
}


bool CConfigBase::_ReadDbConfig( const char* p_szKey, CString* p_pstrValue )
{
	if( CDbModuleBase::InstBase()->ReadConfig( p_szKey, p_pstrValue ) <= 0 )
	{
		CString strMsg;

		strMsg.Format( "SYSTEM_ID[%s],PROCESS_ID[%s].PROCESS_CONFIG 테이블에서, [%s]를 읽지 못하였습니다. -> [%s]", CConfigBase::SYSTEM_ID, CConfigBase::PROCESS_ID, p_szKey, CDbModuleBase::InstBase()->m_ErrorMessage );
		AfxMessageBox( strMsg );
		return false;
	}

	if( s_pfnDisplayFn != NULL )
		s_pfnDisplayFn( p_szKey, *p_pstrValue );

	return true;
}

bool CConfigBase::_ReadDbConfig( const char* p_szKey, int* p_pnValue )
{
	CString strValue;

	if( !_ReadDbConfig( p_szKey, &strValue ) )
		return false;

	*p_pnValue = atoi( strValue );

	return true;
}

bool CConfigBase::_ReadDbConfig( const char* p_szKey, bool* p_pbValue )
{
	CString strValue = "";

	if( !_ReadDbConfig( p_szKey, &strValue ) )
		return false;

	if( strValue == "Y" )
		*p_pbValue = true;
	else
		*p_pbValue = false;

	return true;
}

bool CConfigBase::_ReadDbConfig( const char* p_szKey, CStringArray* p_psaValues )
{
	CString strToken;
	CString strValue = "";

	if( !_ReadDbConfig( p_szKey, &strValue ) )
		return false;

	for( int i = 0 ; ; ++i )
	{
		if( !AfxExtractSubString(strToken, strValue, i, ',') )
			break;
		strToken.Trim();
		p_psaValues->Add( strToken );
	}

	return true;
}


bool CConfigBase::_ReadDbConfig( const char* p_szKey, UINT* p_pnValue )
{
	CString strValue = "";

	if( !_ReadDbConfig( p_szKey, &strValue ) )
		return false;

	*p_pnValue = atol( strValue );
	return true;
}


bool CConfigBase::_WriteConfig( const char* p_szSection, const char* p_szKey, CString* p_pstrValue )
{	
	::WritePrivateProfileString( p_szSection, p_szKey, *p_pstrValue, CConfigBase::CONFIG_FILE );
	return true;
}

bool CConfigBase::_WriteConfig( const char* p_szSection, const char* p_szKey, const char* p_pstrValue )
{	
	::WritePrivateProfileString( p_szSection, p_szKey, p_pstrValue, CConfigBase::CONFIG_FILE );
	return true;
}

