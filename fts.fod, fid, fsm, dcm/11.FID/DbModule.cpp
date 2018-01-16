// OraADODB.cpp: implementation of the CDbModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "APP.h"
#include "DbModule.h"
#include "Config.h"
#include "DbConnector.h"
#include "DbConnection.h"

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



int CDbModule::InsertRecvInfo(int channel, CDbModule::RECV_INFO& recvInfo, CString* pstrFaxId)
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
			pSQLCommand->CommandText		= "PKG_PRC_FID.USP_INSERT_RECV_MSTR";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			CString recvType = "I";
			param = pSQLCommand->CreateParameter("P_RECV_TYPE",			adVarChar, adParamInput, recvType.GetLength(), (_bstr_t)recvType );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_SYSTEM_ID",			adInteger, adParamInput, sizeof(int), CConfig::SYSTEM_NO );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_PROCESS_ID",		adInteger, adParamInput, sizeof(int), CConfig::PROCESS_NO );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_CHANNEL",			adInteger, adParamInput, sizeof(int), channel );
					pSQLCommand->Parameters->Append(param);					
			param = pSQLCommand->CreateParameter("P_CID",				adVarChar, adParamInput, recvInfo.CID.GetLength(),	(_bstr_t)recvInfo.CID );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_DID",				adVarChar, adParamInput, recvInfo.DID.GetLength(),	(_bstr_t)recvInfo.DID );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_TIF_FILE",			adVarChar, adParamInput, recvInfo.TIF_FILE.GetLength(),	(_bstr_t)recvInfo.TIF_FILE );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_TIF_FILE_SIZE",		adInteger, adParamInput, sizeof(int), recvInfo.TIF_FILE_SIZE );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_TIF_PAGE_CNT",		adInteger, adParamInput, sizeof(int), recvInfo.TIF_PAGE_CNT );
					pSQLCommand->Parameters->Append(param);

			paramOut = pSQLCommand->CreateParameter("P_OUT_FAX_ID",		adNumeric, adParamOutput, 20);										
					   pSQLCommand->Parameters->Append(paramOut);
		

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);

			if( pstrFaxId != NULL ) {
				_variant_t vTemp = paramOut->Value;
				V_DEC_TO_STR(vTemp, *pstrFaxId);
			}
		
			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_FID.USP_INSERT_RECV_MSTR", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}


int	CDbModule::InsertRecvInfoEx(int channel, CString pstrMgwIp, int mgwPort, CDbModule::RECV_INFO& recvInfo, CString* pstrFaxId)
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
			pSQLCommand->CommandText		= "PKG_PRC_FID.USP_INSERT_RECV_MSTR_EX";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			CString recvType = "I";
			param = pSQLCommand->CreateParameter("P_RECV_TYPE",			adVarChar, adParamInput, recvType.GetLength(), (_bstr_t)recvType );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_SYSTEM_ID",			adInteger, adParamInput, sizeof(int), CConfig::SYSTEM_NO );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_PROCESS_ID",		adInteger, adParamInput, sizeof(int), CConfig::PROCESS_NO );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_MGW_IP",			adVarChar, adParamInput, pstrMgwIp.GetLength(), (_bstr_t)pstrMgwIp );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_MGW_PORT",			adInteger, adParamInput, sizeof(int), mgwPort );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_CHANNEL",			adInteger, adParamInput, sizeof(int), channel );
					pSQLCommand->Parameters->Append(param);					
			param = pSQLCommand->CreateParameter("P_CID",				adVarChar, adParamInput, recvInfo.CID.GetLength(),	(_bstr_t)recvInfo.CID );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_DID",				adVarChar, adParamInput, recvInfo.DID.GetLength(),	(_bstr_t)recvInfo.DID );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_TIF_FILE",			adVarChar, adParamInput, recvInfo.TIF_FILE.GetLength(),	(_bstr_t)recvInfo.TIF_FILE );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_TIF_FILE_SIZE",		adInteger, adParamInput, sizeof(int), recvInfo.TIF_FILE_SIZE );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_TIF_PAGE_CNT",		adInteger, adParamInput, sizeof(int), recvInfo.TIF_PAGE_CNT );
					pSQLCommand->Parameters->Append(param);

			paramOut = pSQLCommand->CreateParameter("P_OUT_FAX_ID",		adNumeric, adParamOutput, 20);										
					   pSQLCommand->Parameters->Append(paramOut);
		

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);

			if( pstrFaxId != NULL ) {
				_variant_t vTemp = paramOut->Value;
				V_DEC_TO_STR(vTemp, *pstrFaxId);
			}
		
			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{	
			SetErrorMessage("PKG_PRC_FID.USP_INSERT_RECV_MSTR_EX", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}

int	CDbModule::UpdateRecvRunningState(CString pFaxId)
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	_ParameterPtr	paramOut	= NULL;
	
	int				nRow = -1;
	int				effectCnt = -2;
	
	DECIMAL			faxID;	
	{CDbConnector connector( CDbConnection::Inst() );
	
		STR_TO_DEC( pFaxId,	faxID	);

		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_FID.USP_UPDATE_RECV_STATE_RUNNING";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_FAX_ID",			adDecimal, adParamInput, sizeof(DECIMAL), faxID);
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);
		
			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_FID.USP_UPDATE_RECV_STATE_RUNNING", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
			
			APPLOG->Print(DBGLV_ERR, "msg[%s]", m_ErrorMessage);			
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}



int	CDbModule::UpdateRecvFinished(CString pstrFaxId, int pResult, int pReason, CString pTifFile, int pTifFileSize, int pTifPageCnt)
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	_ParameterPtr	paramOut	= NULL;
	
	int				nRow = -1;
	int				effectCnt = -2;
	char			strReason[20];
	
	DECIMAL			faxID;	
	{CDbConnector connector( CDbConnection::Inst() );
	
		STR_TO_DEC( pstrFaxId,	faxID	);
		memset(strReason, 0x00, sizeof(strReason));
		sprintf(strReason, "%04d", pReason);

		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_FID.USP_UPDATE_RECV_FINISHED";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_FAX_ID",			adDecimal, adParamInput, sizeof(DECIMAL), faxID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_RESULT",			adInteger, adParamInput, sizeof(int), pResult );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_REASON",			adVarChar, adParamInput, strlen(strReason), (_bstr_t)strReason );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_TIF_FILE",			adVarChar, adParamInput, pTifFile.GetLength(),	(_bstr_t)pTifFile );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_TIF_FILE_SIZE",		adInteger, adParamInput, sizeof(int), pTifFileSize );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_TIF_PAGE_CNT",		adInteger, adParamInput, sizeof(int), pTifPageCnt );
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);
		
			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_FID.USP_UPDATE_RECV_FINISHED", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
			
			APPLOG->Print(DBGLV_ERR, "msg[%s]", m_ErrorMessage);			
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;

	return 0;
}
