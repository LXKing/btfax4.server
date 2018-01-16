#include "StdAfx.h"
#include "DbModuleBase.h"

CDbModuleBase* CDbModuleBase::s_pInstanceBase = NULL;

CDbModuleBase* CDbModuleBase::InstBase()
{
	if( s_pInstanceBase == NULL )
		s_pInstanceBase = new CDbModuleBase;

	return s_pInstanceBase;
}

CDbModuleBase::CDbModuleBase(void)
{
}


CDbModuleBase::~CDbModuleBase(void)
{
}

// -------------------------------------------------------------------
// Module ��	: void GetConnInfo()
// -------------------------------------------------------------------
// Descriotion	: ���� �޽��� ���
// -------------------------------------------------------------------
// Argument		: const char* pFuncName;	�Լ���
//				  _com_error &e;			_com_error
// -------------------------------------------------------------------
// Return ��	: void
// -------------------------------------------------------------------
void CDbModuleBase::SetErrorMessage(const char* pFuncName, _com_error &e)
{
	if( e.Description().length() == 0)
		m_ErrorMessage.Format("[0x%x] %s", e.Error(), (LPCSTR)e.ErrorMessage());
	else
		m_ErrorMessage.Format("[0x%x] %s", e.Error(), (LPCSTR)e.Description());

	//g_Debug.Print(DBGLV_ERR, "[FAIL] [%s] ErrMsg[%s]", pFuncName, m_ErrorMessage);
	// Debugging ��
	//AfxMessageBox(m_ErrorMessage);
}

// -------------------------------------------------------------------
// Module ��	: int P_READ_CONFIG()
// -------------------------------------------------------------------
// Descriotion	: ȯ�� ���̺��� ���� �� �б�.
// -------------------------------------------------------------------
// Argument		: char* pProcType;		Process Type (#define FOD)
//				  char* pSysProcId;		System Process ID (FOD_01_FSD_01)
//				  int OccupyReqCnt;     ���� ��û ����
// -------------------------------------------------------------------
// Return ��	:	   1		SUCC
//				  -1			ADI Exception �߻�
//				  -2			��Ÿ Procedure ��� ����
//				  -99		Oracle ���� ���� ����
//				  -98		ȯ���б� ����
//				  -91		DB ���� ����
// -------------------------------------------------------------------
int CDbModuleBase::ReadConfig(const char* pKey, CString* pValue)
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	_ParameterPtr	paramOut	= NULL;

	int				nResult = 1;


	{CDbConnector connector( CDbConnection::Inst() );
	
		try
		{
			pSQLCommand.CreateInstance( __uuidof(Command) );
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		=_bstr_t("PKG_BTFAX_COMMON.USP_SELECT_PROCESS_CONFIG");

			// Output �Ķ���� ����
			paramOut = pSQLCommand->CreateParameter( "P_VALUE",		adVarChar, adParamOutput, 256 );
					   pSQLCommand->Parameters->Append( paramOut );

			// Input �Ķ���� ����
			param = pSQLCommand->CreateParameter( "P_SYSTEM_ID",		adVarChar, adParamInput, strlen(CConfigBase::SYSTEM_ID), (_bstr_t)CConfigBase::SYSTEM_ID );
					pSQLCommand->Parameters->Append( param );
			param = pSQLCommand->CreateParameter( "P_PROCESS_ID",	adVarChar, adParamInput, strlen(CConfigBase::PROCESS_ID), (_bstr_t)CConfigBase::PROCESS_ID );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter( "P_KEY",			adVarChar, adParamInput, strlen(pKey), (_bstr_t)pKey );
					pSQLCommand->Parameters->Append( param );

			// ��ɽ���
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute( &recordsAffected, NULL, adCmdStoredProc );

			// OUTPUT�� �о� ��
			V_STR_TO_STR( paramOut->Value, *pValue );
			if( pValue->GetLength() <= 0 )
				nResult = -98;
		}

		catch ( _com_error &e )
		{
			

			SetErrorMessage( "READ_CONFIG", e );
			nResult = -91;
			CDbConnection::Inst()->Disconnect();
		}

		if( pSQLCommand != NULL )
			pSQLCommand.Release();
	}

	return nResult;
}

int CDbModuleBase::ReadConfig(const char* pKey, int* pValue)
{
	int nRet;
	CString value;

	nRet = ReadConfig(pKey, &value);
	if(nRet > 0)
		*pValue = atoi(value);

	return nRet;
}

int CDbModuleBase::ReadConfig(const char* pKey, bool* pValue)
{
	int nRet;
	CString value;

	nRet = ReadConfig(pKey, &value);
	if(nRet > 0)
		*pValue = (value == "Y") ? true : false;

	return nRet;
}
