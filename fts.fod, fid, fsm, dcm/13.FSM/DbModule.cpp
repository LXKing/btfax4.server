// OraADODB.cpp: implementation of the CDbModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "APP.h"
#include "DbModule.h"
#include "Config.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


inline void TESTHR(HRESULT x) { if FAILED(x)  _com_issue_error(x); };


CDbModule* CDbModule::s_pInstance = NULL;

CDbModule* CDbModule::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new CDbModule;

	return s_pInstance;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDbModule::CDbModule()
{
	CoInitialize(NULL);

	//m_bConnected	= false;
	//m_pConn			= NULL;
}

CDbModule::~CDbModule()
{
	CoUninitialize();
}




int CDbModule::InsertAlarmEvent( const char* p_szEventType, const char* p_szEvent, const char* p_szEventStatus, const char* p_szEventDetial, const char* p_szDescription )
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	_ParameterPtr	paramOut	= NULL;
	
	int				nRow = -1;
	int				effectCnt = -2;
	

	{CDbConnector connector( CDbConnection::Inst() );
	
	
		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_BTFAX_COMMON.USP_ADD_HISTORY";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_Event_Type",	adVarChar, adParamInput, strlen(p_szEventType),					(_bstr_t)p_szEventType );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_Event",			adVarChar, adParamInput, strlen(p_szEvent),						(_bstr_t)p_szEvent );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_Event_Status",	adVarChar, adParamInput, strlen(p_szEventStatus),				(_bstr_t)p_szEventStatus );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_Event_Detail",	adVarChar, adParamInput, strlen(p_szEventDetial),				(_bstr_t)p_szEventDetial );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_System",		adVarChar, adParamInput, g_Config.SYSTEM_ID.GetLength(),		(_bstr_t)g_Config.SYSTEM_ID );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_Process",		adVarChar, adParamInput, g_Config.SYSTEM_PROCESS_ID.GetLength(), (_bstr_t)g_Config.SYSTEM_PROCESS_ID );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_Reg_System",	adVarChar, adParamInput, g_Config.SYSTEM_ID.GetLength(),		(_bstr_t)g_Config.SYSTEM_ID );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_DESCRIPTION",	adVarChar, adParamInput, strlen(p_szDescription),				(_bstr_t)p_szDescription );
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute( &recordsAffected, NULL, adCmdStoredProc );
		
			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage( pSQLCommand->CommandText, e );
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}


	return nRow;
}

