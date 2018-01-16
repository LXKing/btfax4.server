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


//--------------------------------------------------------
// Title	: OccupySendCdr
// Writer	: KIMCG
// Date		: 2013.12.17
// Content  : 팩스 송신 CDR 점유
//--------------------------------------------------------
int	CDbModule::OccupySendCdr(int OccupyCdrCnt, vector<FAX_CDR>& vecCdrs)
{
	_CommandPtr		pSQLCommand			= NULL;
	_RecordsetPtr	recordset			= NULL;
	_ParameterPtr	param				= NULL;
	_variant_t		vTemp;

	int				nCnt, i, j;
	int				nRow = -1;
	int				getCnt = -2;
	long			lTemp = 0;

	CString			strPrint, strInfo;
	FAX_CDR			faxCdr;

	char			szSubFolder[256];
	char			szTifName[256];
	char			szTifExt[256];
	
	{CDbConnector connector( CDbConnection::Inst() );
		try
		{
			// P1. Procedure 실행
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();
			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_DCM.USP_OCCUPY_SEND_CDR";
		
			// Input 파라미터 지정
			param = pSQLCommand->CreateParameter("P_PROCESS_TYPE",			adVarChar, adParamInput, strlen(CConfig::PROCESS_TYPE_STR), (_bstr_t)CConfig::PROCESS_TYPE_STR);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_FETCH_PROCESS",			adVarChar, adParamInput, strlen(CConfig::SYSTEM_PROCESS_ID), (_bstr_t)CConfig::SYSTEM_PROCESS_ID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_OCCUPY_CNT",			adInteger, adParamInput, sizeof(int), OccupyCdrCnt);
					pSQLCommand->Parameters->Append(param);			

			// Command 실행
			recordset = pSQLCommand->Execute(NULL, NULL, adCmdStoredProc);
			if( recordset == NULL )
				return 0;
			CTime timeCurrent = CTime::GetCurrentTime();
		
			nCnt = recordset->RecordCount;
			for(i = 0 ; i < nCnt ; ++i)
			{	
				faxCdr.clear();
				faxCdr.CDR_TYPE = 1;
				
				R_GET_DEC(recordset, vTemp, "FAX_ID"					, faxCdr.send_cdr.FAX_ID					);
				R_GET_DEC(recordset, vTemp, "FAX_DTL_ID"				, faxCdr.send_cdr.FAX_DTL_ID               );
				R_GET_STR(recordset, vTemp, "INDEX_NO"					, faxCdr.send_cdr.INDEX_NO                 );
				R_GET_STR(recordset, vTemp, "TR_NO"						, faxCdr.send_cdr.TR_NO                    );
				R_GET_STR(recordset, vTemp, "STATE"						, faxCdr.send_cdr.STATE                    );
				R_GET_STR(recordset, vTemp, "PRIORITY"					, faxCdr.send_cdr.PRIORITY                 );
				R_GET_STR(recordset, vTemp, "TITLE"						, faxCdr.send_cdr.TITLE                    );
				R_GET_STR(recordset, vTemp, "MEMO"						, faxCdr.send_cdr.MEMO                     );
				R_GET_STR(recordset, vTemp, "PRE_PROCESS_REQ"			, faxCdr.send_cdr.PRE_PROCESS_REQ          );
				R_GET_STR(recordset, vTemp, "REQ_TYPE"					, faxCdr.send_cdr.REQ_TYPE                 );
				R_GET_STR(recordset, vTemp, "REQUESTER_TYPE"			, faxCdr.send_cdr.REQUESTER_TYPE           );
				R_GET_STR(recordset, vTemp, "REQ_USER_ID"				, faxCdr.send_cdr.REQ_USER_ID              );
				R_GET_STR(recordset, vTemp, "REQ_USER_NAME"				, faxCdr.send_cdr.REQ_USER_NAME            );
				R_GET_STR(recordset, vTemp, "REQ_USER_TELNO"			, faxCdr.send_cdr.REQ_USER_TELNO           );
				R_GET_STR(recordset, vTemp, "REQ_DATE"					, faxCdr.send_cdr.REQ_DATE                 );
				R_GET_STR(recordset, vTemp, "PREVIEW_REQ"				, faxCdr.send_cdr.PREVIEW_REQ              );
				R_GET_STR(recordset, vTemp, "PREVIEWED_ID"				, faxCdr.send_cdr.PREVIEWED_ID             );
				R_GET_STR(recordset, vTemp, "PREVIEWED_DATE"			, faxCdr.send_cdr.PREVIEWED_DATE           );
				R_GET_STR(recordset, vTemp, "APPROVE_REQ"				, faxCdr.send_cdr.APPROVE_REQ              );
				R_GET_STR(recordset, vTemp, "APPROVED_YN"				, faxCdr.send_cdr.APPROVED_YN              );
				R_GET_STR(recordset, vTemp, "APPROVED_ID"				, faxCdr.send_cdr.APPROVED_ID              );
				R_GET_STR(recordset, vTemp, "APPROVED_DATE"				, faxCdr.send_cdr.APPROVED_DATE            );
				R_GET_STR(recordset, vTemp, "APPROVED_COMMENT"			, faxCdr.send_cdr.APPROVED_COMMENT         );
				R_GET_STR(recordset, vTemp, "RESERVED_YN"				, faxCdr.send_cdr.RESERVED_YN              );
				R_GET_STR(recordset, vTemp, "RESERVED_DATE"				, faxCdr.send_cdr.RESERVED_DATE            );
				R_GET_STR(recordset, vTemp, "PROCESS_FETCH"				, faxCdr.send_cdr.PROCESS_FETCH            );
				R_GET_STR(recordset, vTemp, "SMS_CONTENTS"				, faxCdr.send_cdr.SMS_CONTENTS             );
				R_GET_STR(recordset, vTemp, "SMS_SEND_YN"				, faxCdr.send_cdr.SMS_SEND_YN              );
				R_GET_STR(recordset, vTemp, "RESULT_FORWARD"			, faxCdr.send_cdr.RESULT_FORWARD           );
				R_GET_STR(recordset, vTemp, "TEST_TYPE"					, faxCdr.send_cdr.TEST_TYPE                );
				R_GET_STR(recordset, vTemp, "BROADCAST_YN"				, faxCdr.send_cdr.BROADCAST_YN             );
				R_GET_STR(recordset, vTemp, "PROCESS_FETCH_DATE"		, faxCdr.send_cdr.PROCESS_FETCH_DATE       );
				R_GET_DEC(recordset, vTemp, "BROADCAST_CNT"				, faxCdr.send_cdr.BROADCAST_CNT            );
				R_GET_STR(recordset, vTemp, "STATE_EACH"				, faxCdr.send_cdr.STATE_EACH               );
				R_GET_STR(recordset, vTemp, "FAX_NO"					, faxCdr.send_cdr.FAX_NO                   );
				R_GET_STR(recordset, vTemp, "RECIPIENT_NAME"			, faxCdr.send_cdr.RECIPIENT_NAME           );
				R_GET_STR(recordset, vTemp, "TIF_FILE"					, faxCdr.send_cdr.TIF_FILE                 );
				R_GET_DEC(recordset, vTemp, "TF_FILE_SIZE"				, faxCdr.send_cdr.TF_FILE_SIZE             );
				R_GET_DEC(recordset, vTemp, "TIF_PAGE_CNT"				, faxCdr.send_cdr.TIF_PAGE_CNT             );
				R_GET_STR(recordset, vTemp, "PAGES_TO_SEND"				, faxCdr.send_cdr.PAGES_TO_SEND            );
				R_GET_DEC(recordset, vTemp, "LAST_PAGE_SENT"			, faxCdr.send_cdr.LAST_PAGE_SENT           );
				R_GET_STR(recordset, vTemp, "TITLE_DTL"					, faxCdr.send_cdr.TITLE_DTL                );
				R_GET_STR(recordset, vTemp, "MEMO_DTL"					, faxCdr.send_cdr.MEMO_DTL                 );
				R_GET_STR(recordset, vTemp, "RESULT"					, faxCdr.send_cdr.RESULT                   );
				R_GET_STR(recordset, vTemp, "REASON"					, faxCdr.send_cdr.REASON                   );
				R_GET_DEC(recordset, vTemp, "TRY_CNT"					, faxCdr.send_cdr.TRY_CNT                  );
				R_GET_STR(recordset, vTemp, "SMSNO"						, faxCdr.send_cdr.SMSNO                    );
				R_GET_STR(recordset, vTemp, "PROCESS_FETCH_EACH"		, faxCdr.send_cdr.PROCESS_FETCH_EACH       );
				R_GET_STR(recordset, vTemp, "DATE_TO_SEND"				, faxCdr.send_cdr.DATE_TO_SEND             );
				R_GET_STR(recordset, vTemp, "RESULT_DATE"				, faxCdr.send_cdr.RESULT_DATE              );
				R_GET_STR(recordset, vTemp, "PROCESS_FETCH_DATE_EACH"	, faxCdr.send_cdr.PROCESS_FETCH_DATE_EACH  );
				R_GET_STR(recordset, vTemp, "POST_SND_PROC_YN"			, faxCdr.send_cdr.POST_SND_PROC_YN         );
				R_GET_STR(recordset, vTemp, "TIF_MAKE_B_DATE"			, faxCdr.send_cdr.TIF_MAKE_B_DATE          );
				R_GET_STR(recordset, vTemp, "TIF_MAKE_E_DATE"			, faxCdr.send_cdr.TIF_MAKE_E_DATE          );
				R_GET_STR(recordset, vTemp, "SYSTEM_ID"					, faxCdr.send_cdr.SYSTEM_ID                );
				R_GET_STR(recordset, vTemp, "MGW_IP"					, faxCdr.send_cdr.MGW_IP                   );
				R_GET_DEC(recordset, vTemp, "MGW_PORT"					, faxCdr.send_cdr.MGW_PORT                 );
				R_GET_STR(recordset, vTemp, "DEPT_CD"					, faxCdr.send_cdr.DEPT_CD                  );
				R_GET_STR(recordset, vTemp, "DEPT_NAME"					, faxCdr.send_cdr.DEPT_NAME                );
				R_GET_STR(recordset, vTemp, "TASK_ID"					, faxCdr.send_cdr.TASK_ID                  );
                //R_GET_DEC(recordset, vTemp, "TASK_ID"					, faxCdr.send_cdr.TASK_ID                  );
				R_GET_STR(recordset, vTemp, "TASK_NAME"					, faxCdr.send_cdr.TASK_NAME                );
				R_GET_STR(recordset, vTemp, "DCM_STATE"					, faxCdr.send_cdr.DCM_STATE                );
				R_GET_STR(recordset, vTemp, "SVC_NAME"					, faxCdr.send_cdr.SVC_NAME                 );
				R_GET_DEC(recordset, vTemp, "CHANNEL_NO"				, faxCdr.send_cdr.CHANNEL_NO               );
				
				vecCdrs.push_back(faxCdr);

				recordset->MoveNext();
			}

			nRow = vecCdrs.size();
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_DCM.USP_OCCUPY_SEND_CDR", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}



//--------------------------------------------------------
// Title	: FinishSendCdr
// Writer	: KIMCG
// Date		: 2013.12.17
// Content  : 팩스 송신 cdr 결과 업데이트
//--------------------------------------------------------
int	CDbModule::FinishSendCdr(FAX_CDR& p_faxCdr)
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	
	int				nRow = -1;
	DECIMAL			faxID;
	
	{CDbConnector connector( CDbConnection::Inst() );
		
	STR_TO_DEC( p_faxCdr.send_cdr.FAX_ID, faxID);

		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_DCM.USP_SEND_CDR_DONE";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_FAX_ID",	adDecimal, adParamInput, sizeof(DECIMAL), faxID);
					pSQLCommand->Parameters->Append(param);

			param = pSQLCommand->CreateParameter("P_STATE",		adVarChar, adParamInput, strlen(p_faxCdr.DCM_STATE), (_bstr_t)p_faxCdr.DCM_STATE);
					pSQLCommand->Parameters->Append(param);

			param = pSQLCommand->CreateParameter("P_RESULT",	adVarChar, adParamInput, strlen(p_faxCdr.RESULT), (_bstr_t)p_faxCdr.RESULT);
					pSQLCommand->Parameters->Append(param);

			param = pSQLCommand->CreateParameter("P_REASON",	adVarChar, adParamInput, strlen(p_faxCdr.REASON), (_bstr_t)p_faxCdr.REASON);
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);

			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_DCM.USP_SEND_CDR_DONE", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}


//--------------------------------------------------------
// Title	: OccupyRecvCdr
// Writer	: KIMCG
// Date		: 2013.12.17
// Content  : 팩스 수신 CDR 점유
//--------------------------------------------------------
int	CDbModule::OccupyRecvCdr(int OccupyCdrCnt, vector<FAX_CDR>& vecCdrs)
{
	_CommandPtr		pSQLCommand			= NULL;
	_RecordsetPtr	recordset			= NULL;
	_ParameterPtr	param				= NULL;
	_variant_t		vTemp;

	int				nCnt, i, j;
	int				nRow = -1;
	int				getCnt = -2;
	long			lTemp = 0;

	CString			strPrint, strInfo;
	FAX_CDR			faxCdr;
	//vector<FAX_CDR> vecTemp_sendCdrs;

	char			szSubFolder[256];
	char			szTifName[256];
	char			szTifExt[256];
	
	{CDbConnector connector( CDbConnection::Inst() );
		try
		{
			// P1. Procedure 실행
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();
			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_DCM.USP_OCCUPY_RECV_CDR";
		
			// Input 파라미터 지정
			param = pSQLCommand->CreateParameter("P_PROCESS_TYPE",			adVarChar, adParamInput, strlen(CConfig::PROCESS_TYPE_STR), (_bstr_t)CConfig::PROCESS_TYPE_STR);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_FETCH_PROCESS",			adVarChar, adParamInput, strlen(CConfig::SYSTEM_PROCESS_ID), (_bstr_t)CConfig::SYSTEM_PROCESS_ID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_OCCUPY_CNT",			adInteger, adParamInput, sizeof(int), OccupyCdrCnt);
					pSQLCommand->Parameters->Append(param);			

			// Command 실행
			recordset = pSQLCommand->Execute(NULL, NULL, adCmdStoredProc);
			if( recordset == NULL )
				return 0;
			CTime timeCurrent = CTime::GetCurrentTime();
			
			nCnt = recordset->RecordCount;
			for(i = 0 ; i < nCnt ; ++i)
			{	
				faxCdr.clear();
				faxCdr.CDR_TYPE = 2;

				R_GET_DEC( recordset, vTemp, "FAX_ID"			  , faxCdr.recv_cdr.FAX_ID            ); 
				R_GET_STR( recordset, vTemp, "RECEIVE_TYPE"       , faxCdr.recv_cdr.RECEIVE_TYPE      );
				R_GET_STR( recordset, vTemp, "SERVICE_FAXNO"      , faxCdr.recv_cdr.SERVICE_FAXNO     );
				R_GET_STR( recordset, vTemp, "DID"                , faxCdr.recv_cdr.DID               );
				R_GET_STR( recordset, vTemp, "CID"                , faxCdr.recv_cdr.CID               );
				R_GET_STR( recordset, vTemp, "CID_NAME"           , faxCdr.recv_cdr.CID_NAME          );
				R_GET_STR( recordset, vTemp, "TIF_FILE"           , faxCdr.recv_cdr.TIF_FILE          );
				R_GET_DEC( recordset, vTemp, "TIF_FILE_SIZE"      , faxCdr.recv_cdr.TIF_FILE_SIZE     );
				R_GET_DEC( recordset, vTemp, "TIF_PAGE_CNT"       , faxCdr.recv_cdr.TIF_PAGE_CNT      );
				R_GET_STR( recordset, vTemp, "TITLE"              , faxCdr.recv_cdr.TITLE             );
				R_GET_STR( recordset, vTemp, "MEMO"               , faxCdr.recv_cdr.MEMO              );
				R_GET_STR( recordset, vTemp, "SPLIT_YN"           , faxCdr.recv_cdr.SPLIT_YN          );
				R_GET_STR( recordset, vTemp, "RESULT"             , faxCdr.recv_cdr.RESULT            );
				R_GET_STR( recordset, vTemp, "REASON"             , faxCdr.recv_cdr.REASON            );
				R_GET_STR( recordset, vTemp, "RECV_DATE"          , faxCdr.recv_cdr.RECV_DATE         );
				R_GET_STR( recordset, vTemp, "WORK_STATE"         , faxCdr.recv_cdr.WORK_STATE        );
				R_GET_STR( recordset, vTemp, "PROCESS_FETCH"      , faxCdr.recv_cdr.PROCESS_FETCH     );
				R_GET_STR( recordset, vTemp, "FORWARD_YN"         , faxCdr.recv_cdr.FORWARD_YN        );
				R_GET_STR( recordset, vTemp, "FORWARD_USER_ID"    , faxCdr.recv_cdr.FORWARD_USER_ID   );
				R_GET_STR( recordset, vTemp, "ORG_SERVICE_FAXNO"  , faxCdr.recv_cdr.ORG_SERVICE_FAXNO );
				R_GET_STR( recordset, vTemp, "ORG_CID"            , faxCdr.recv_cdr.ORG_CID           );
				R_GET_STR( recordset, vTemp, "ORG_CID_NAME"       , faxCdr.recv_cdr.ORG_CID_NAME      );
				R_GET_STR( recordset, vTemp, "ORG_RECV_DATE"      , faxCdr.recv_cdr.ORG_RECV_DATE     );
				R_GET_STR( recordset, vTemp, "SYSTEM_ID"          , faxCdr.recv_cdr.SYSTEM_ID         );
				R_GET_DEC( recordset, vTemp, "CHANNEL_NO"         , faxCdr.recv_cdr.CHANNEL_NO        );
				R_GET_STR( recordset, vTemp, "FORWARD_B_DATE"     , faxCdr.recv_cdr.FORWARD_B_DATE    );
				R_GET_STR( recordset, vTemp, "FORWARD_E_DATE"     , faxCdr.recv_cdr.FORWARD_E_DATE    );
				R_GET_STR( recordset, vTemp, "RECV_RESULT"        , faxCdr.recv_cdr.RECV_RESULT       );
				R_GET_STR( recordset, vTemp, "RECV_REASON"        , faxCdr.recv_cdr.RECV_REASON       );
				R_GET_STR( recordset, vTemp, "MGW_IP"             , faxCdr.recv_cdr.MGW_IP            );
				R_GET_DEC( recordset, vTemp, "MGW_PORT"           , faxCdr.recv_cdr.MGW_PORT          );
				R_GET_STR( recordset, vTemp, "RECV_B_DATE"        , faxCdr.recv_cdr.RECV_B_DATE       );
				R_GET_STR( recordset, vTemp, "RECV_E_DATE"        , faxCdr.recv_cdr.RECV_E_DATE       );
				R_GET_STR( recordset, vTemp, "STATE"              , faxCdr.recv_cdr.STATE             );
				R_GET_STR( recordset, vTemp, "DEPT_CD"            , faxCdr.recv_cdr.DEPT_CD           );
				R_GET_STR( recordset, vTemp, "DEPT_NAME"          , faxCdr.recv_cdr.DEPT_NAME         );
				R_GET_STR( recordset, vTemp, "TASK_ID"            , faxCdr.recv_cdr.TASK_ID           );
                //R_GET_DEC( recordset, vTemp, "TASK_ID"            , faxCdr.recv_cdr.TASK_ID           );
				R_GET_STR( recordset, vTemp, "TASK_NAME"          , faxCdr.recv_cdr.TASK_NAME         );
				R_GET_STR( recordset, vTemp, "DCM_STATE"          , faxCdr.recv_cdr.DCM_STATE         );
                R_GET_STR( recordset, vTemp, "USER_ID"            , faxCdr.recv_cdr.USER_ID           );
                R_GET_STR( recordset, vTemp, "USER_NAME"          , faxCdr.recv_cdr.USER_NAME         );
				
				vecCdrs.push_back(faxCdr);

				recordset->MoveNext();
			}

			nRow = vecCdrs.size();
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_DCM.USP_OCCUPY_RECV_CDR", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}



//--------------------------------------------------------
// Title	: FinishRecvCdr
// Writer	: KIMCG
// Date		: 2013.12.17
// Content  : 팩스 수신 cdr 결과 업데이트
//--------------------------------------------------------
int	CDbModule::FinishRecvCdr(FAX_CDR& p_faxCdr)
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	
	int				nRow = -1;
	DECIMAL			faxID;
	
	{CDbConnector connector( CDbConnection::Inst() );
		
	STR_TO_DEC( p_faxCdr.recv_cdr.FAX_ID, faxID);

		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_DCM.USP_RECV_CDR_DONE";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_FAX_ID",			adDecimal, adParamInput, sizeof(DECIMAL), faxID);
					pSQLCommand->Parameters->Append(param);

			param = pSQLCommand->CreateParameter("P_STATE",				adVarChar, adParamInput, strlen(p_faxCdr.DCM_STATE), (_bstr_t)p_faxCdr.DCM_STATE);
					pSQLCommand->Parameters->Append(param);

			param = pSQLCommand->CreateParameter("P_RESULT",			adVarChar, adParamInput, strlen(p_faxCdr.RESULT), (_bstr_t)p_faxCdr.RESULT);
					pSQLCommand->Parameters->Append(param);

			param = pSQLCommand->CreateParameter("P_REASON",			adVarChar, adParamInput, strlen(p_faxCdr.REASON), (_bstr_t)p_faxCdr.REASON);
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);

			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_DCM.USP_RECV_CDR_DONE", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}




//--------------------------------------------------------
// Title	: SelectFaxQueue
// Writer	: KIMCG
// Date		: 2013.12.23
// Content  : 팩스 송,수신 큐 조회
//--------------------------------------------------------
int	CDbModule::SelectFaxQueue(FAX_Q_ITEM& p_faxQueueItem)
{	
	_CommandPtr		pSQLCommand			= NULL;
	_RecordsetPtr	recordset			= NULL;	
	_variant_t		vTemp;

	int				nRow = -1;
	int				nCnt = 0;
	long			lTemp = 0;
	
	{CDbConnector connector( CDbConnection::Inst() );
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_DCM.USP_SELECT_SEND_RECV_QUEUE";

			// Command 실행
			recordset = pSQLCommand->Execute(NULL, NULL, adCmdStoredProc);
			if( recordset == NULL )
				return 0;
			CTime timeCurrent = CTime::GetCurrentTime();

			// send part
			CString SEND_STATE;
			int SEND_CNT, SEND_TIF_PAGE_CNT;
			nCnt = recordset->RecordCount;
			for(int i = 0 ; i < nCnt ; ++i)
			{	
				R_GET_STR( recordset, vTemp, "SEND_STATE",			SEND_STATE			);
				R_GET_NUM( recordset, vTemp, "SEND_CNT",			SEND_CNT			);
				R_GET_NUM( recordset, vTemp, "SEND_TIF_PAGE_CNT",	SEND_TIF_PAGE_CNT	);
				
				// DCV 생성대기
				if(SEND_STATE == "11")
				{
					p_faxQueueItem.send_queue.wait_processing_total += SEND_CNT;
					p_faxQueueItem.send_queue.wait_make_tiff_total += SEND_CNT;
				}

				// TMK 생성대기
				if(SEND_STATE == "21")
				{
					p_faxQueueItem.send_queue.wait_processing_total += SEND_CNT;
					p_faxQueueItem.send_queue.wait_make_tiff_total += SEND_CNT;
				}

				// TPC 생성대기
				if(SEND_STATE == "31")
				{
					p_faxQueueItem.send_queue.wait_processing_total += SEND_CNT;
					p_faxQueueItem.send_queue.wait_make_tiff_total += SEND_CNT;
				}

				// FOD 발송대기
				if(SEND_STATE == "41")
				{
					p_faxQueueItem.send_queue.wait_processing_total += SEND_CNT;
					p_faxQueueItem.send_queue.wait_send_total += SEND_CNT;					
				}
				
				// FOD 발송중
				if(SEND_STATE == "42")
				{
					p_faxQueueItem.send_queue.wait_processing_total += SEND_CNT;
					p_faxQueueItem.send_queue.sending_total += SEND_CNT;
					p_faxQueueItem.send_queue.sending_tiff_page_count_total += SEND_TIF_PAGE_CNT;
				}

				// FOD 발송성공
				if(SEND_STATE == "91")
				{
					p_faxQueueItem.send_queue.completed_send_total += SEND_CNT;
					p_faxQueueItem.send_queue.send_sucess_total += SEND_CNT;
				}

				// FOD 발송실패
				if(SEND_STATE == "99")
				{
					p_faxQueueItem.send_queue.completed_send_total += SEND_CNT;
					p_faxQueueItem.send_queue.send_fail_total += SEND_CNT;
				}

				recordset->MoveNext();
			}
   
			recordset = recordset->NextRecordset((VARIANT*)lTemp);
			if(recordset == NULL)
				return -1;
			nCnt = recordset->RecordCount;
			if(nCnt >0)
			{
				R_GET_NUM( recordset, vTemp, "MAX_SEND_TRYCNT",		p_faxQueueItem.send_queue.MAX_SEND_TRYCNT		);
				R_GET_NUM( recordset, vTemp, "MAX_TIFF_PAGECNT",	p_faxQueueItem.send_queue.MAX_TIFF_PAGECNT		);
				R_GET_NUM( recordset, vTemp, "MAX_TIFF_FILESIZE",	p_faxQueueItem.send_queue.MAX_TIFF_FILESIZE		);
				R_GET_NUM( recordset, vTemp, "MAX_TIFF_MAKETIME",	p_faxQueueItem.send_queue.MAX_TIFF_MAKETIME		);
				R_GET_NUM( recordset, vTemp, "MAX_WAIT_TIME",		p_faxQueueItem.send_queue.MAX_WAIT_TIME			);
				R_GET_NUM( recordset, vTemp, "MAX_SENDING_TIME",	p_faxQueueItem.send_queue.MAX_SENDING_TIME		);
			
				R_GET_NUM( recordset, vTemp, "AVG_SEND_TRYCNT",		p_faxQueueItem.send_queue.AVG_SEND_TRYCNT		);
				R_GET_NUM( recordset, vTemp, "AVG_TIFF_PAGECNT",	p_faxQueueItem.send_queue.AVG_TIFF_PAGECNT		);
				R_GET_NUM( recordset, vTemp, "AVG_TIFF_FILESIZE",	p_faxQueueItem.send_queue.AVG_TIFF_FILESIZE		);
				R_GET_NUM( recordset, vTemp, "AVG_TIFF_MAKETIME",	p_faxQueueItem.send_queue.AVG_TIFF_MAKETIME		);
				R_GET_NUM( recordset, vTemp, "AVG_WAIT_TIME",		p_faxQueueItem.send_queue.AVG_WAIT_TIME			);
				R_GET_NUM( recordset, vTemp, "AVG_SENDING_TIME",	p_faxQueueItem.send_queue.AVG_SENDING_TIME		);
			}
			

			// receive part
			CString RECV_STATE;
			int RECV_CNT, RECV_TIF_PAGE_SUM, RECV_RESULT_CNT;
			recordset = recordset->NextRecordset((VARIANT*)lTemp);		
			if(recordset == NULL)
				return -1;
			nCnt = recordset->RecordCount;
			for(int i = 0 ; i < nCnt ; ++i)
			{	
				R_GET_STR( recordset, vTemp, "RECV_STATE",			RECV_STATE			);
				R_GET_NUM( recordset, vTemp, "RECV_CNT",			RECV_CNT			);
				R_GET_NUM( recordset, vTemp, "RECV_TIF_PAGE_SUM",	RECV_TIF_PAGE_SUM	);
				R_GET_NUM( recordset, vTemp, "RECV_RESULT_CNT",		RECV_RESULT_CNT		);
				
				// FID 수신대기
				if(RECV_STATE == "41")
				{
					//p_faxQueueItem.recv_queue.receive_sucess_total += RECV_CNT;
				}

				// FID 수신중
				if(RECV_STATE == "42")
				{
					p_faxQueueItem.recv_queue.receving_total += RECV_CNT;
					p_faxQueueItem.recv_queue.receive_tiff_page_count_total += RECV_CNT;
				}

				// FID 수신실패
				if(RECV_STATE == "49")
				{
					p_faxQueueItem.recv_queue.completed_receive_total += RECV_CNT;
					p_faxQueueItem.recv_queue.receive_fail_total += RECV_CNT;
				}

				// PSI 배달실패
				if(RECV_STATE == "89")
				{
					p_faxQueueItem.recv_queue.completed_receive_total += RECV_CNT;
					p_faxQueueItem.recv_queue.receive_fail_total += RECV_CNT;
				}

				// PSI 배달성공
				if(RECV_STATE == "91")
				{
					p_faxQueueItem.recv_queue.receive_sucess_total += RECV_CNT;
					p_faxQueueItem.recv_queue.completed_receive_total += RECV_CNT;
				}

				recordset->MoveNext();
			}

			recordset = recordset->NextRecordset((VARIANT*)lTemp);
			if(recordset == NULL)
				return -1;
			nCnt = recordset->RecordCount;
			if(nCnt > 0)
			{
				R_GET_NUM( recordset, vTemp, "MAX_TIFF_PAGECNT",	p_faxQueueItem.recv_queue.MAX_TIFF_PAGECNT		);
				R_GET_NUM( recordset, vTemp, "MAX_TIFF_FILESIZE",	p_faxQueueItem.recv_queue.MAX_TIFF_FILESIZE		);
				R_GET_NUM( recordset, vTemp, "MAX_RECVING_TIME",	p_faxQueueItem.recv_queue.MAX_RECVING_TIME		);
			
				R_GET_NUM( recordset, vTemp, "AVG_TIFF_PAGECNT",	p_faxQueueItem.recv_queue.AVG_TIFF_PAGECNT		);
				R_GET_NUM( recordset, vTemp, "AVG_TIFF_FILESIZE",	p_faxQueueItem.recv_queue.AVG_TIFF_FILESIZE		);
				R_GET_NUM( recordset, vTemp, "AVG_RECVING_TIME",	p_faxQueueItem.recv_queue.AVG_RECVING_TIME		);
			}
			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_DCM.USP_SELECT_SEND_RECV_QUEUE", e);
			APPLOG->Print(DBGLV_ERR, "%s", e.ErrorMessage());
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}


