// OraADODB.cpp: implementation of the CDbModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "APP.h"
#include "DbModule.h"
#include "Config.h"
#include "DbConnector.h"
#include "DbConnection.h"
#include "EncryptApi.h"


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



// -------------------------------------------------------------------
// Module 명	: int P_FOD_GET_OCCUPIED_SEND_REQS()
// -------------------------------------------------------------------
// Descriotion	: 점유된 발송 대기건이 있으면 필요한 정보를 가져온다.
// -------------------------------------------------------------------
// Argument		: char* pProcType;			Process Type (#define FOD)
//				  char* pSysProcType;		System Process Type (FOD_01_FSD_01)
//				  int OccupyReqCnt;			점유 요청 개수
//				  int maxFODCh;				MAX FOD CH 개수
//				  int maxFODCh;				MAX FOD CH 개수
//				  int &ID_REQ;				점유 Seq
//				  string &FAX_NO;			발신 AX번호
//				  string &LAST_TIFF_FILE;	발송할 TIFF File 명
//				  string &SEND_B_DT;		발송시작시간
// -------------------------------------------------------------------
// Return 값	: >=0		발송 대기건 점유 개수
//				-1		ADI Exception 발생
//				-2		기타 Procedure 결과 오류
//				-99		Oracle 접속 해제 상태
//				-91		DB 예외 오류
// -------------------------------------------------------------------
int CDbModule::OccupySendReq(int totalCnt, int maxBroadcastCnt, vector<CDbModule::SEND_REQ>& vecSendReqs, vector<CDbModule::SEND_REQ>& vecSendReqs_broad)
{
	_CommandPtr		pSQLCommand			= NULL;
	_RecordsetPtr	recordset			= NULL;
	_ParameterPtr	param				= NULL;
	_variant_t		vTemp;

	int				nCnt, i, j;
	int				nRow = -1;
	int				getCnt = -2;
	long			lTemp = 0;

	CString			strPrint, strInfo, strTemp;
	SEND_REQ		sendReq;
	SEND_REQ_MSTR	sendReq_m;
	vector<SEND_REQ_MSTR> vecTemp_sendReqs_m;

	char			szSubFolder[256];
	char			szTifName[256];
	char			szTifExt[256];
	
	vecSendReqs.clear();
	vecSendReqs_broad.clear();


	{CDbConnector connector( CDbConnection::Inst() );
	

		try
		{
			//// P1. Procedure 실행
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_FOD.USP_OCCUPY_SEND_REQ";
		
			// Input 파라미터 지정						
			param = pSQLCommand->CreateParameter("P_SYSTEM_NO",		adInteger, adParamInput, sizeof(int), CConfig::SYSTEM_NO);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_PROCESS_TYPE",			adVarChar, adParamInput, strlen(CConfig::PROCESS_TYPE_STR), (_bstr_t)CConfig::PROCESS_TYPE_STR);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_FETCH_PROCESS",			adVarChar, adParamInput, strlen(CConfig::SYSTEM_PROCESS_ID), (_bstr_t)CConfig::SYSTEM_PROCESS_ID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_OCCUPY_TOTAL_CNT",		adInteger, adParamInput, sizeof(int), totalCnt);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_OCCUPY_BROADCAST_CNT",	adInteger, adParamInput, sizeof(int), maxBroadcastCnt);
					pSQLCommand->Parameters->Append(param);

			// Command 실행
			recordset = pSQLCommand->Execute(NULL, NULL, adCmdStoredProc);
			if( recordset == NULL )
				return 0;
			CTime timeCurrent = CTime::GetCurrentTime();
		

			//// P2. 일반건-Master Row(s) 얻기
			nCnt = recordset->RecordCount;
			for(i = 0 ; i < nCnt ; ++i)
			{	
				sendReq_m.clear();

				R_GET_DEC( recordset, vTemp, "FAX_ID",			sendReq_m.FAX_ID			);
				R_GET_STR( recordset, vTemp, "TR_NO",			sendReq_m.TR_NO				);
				
				// ADD - KIMCG : 20131212
				R_GET_STR( recordset, vTemp, "SVC_NAME",		sendReq_m.SVC_NAME			);
				// ADD - END
				
				R_GET_STR( recordset, vTemp, "STATE",			sendReq_m.STATE				);
				R_GET_STR( recordset, vTemp, "PRIORITY",		sendReq_m.PRIORITY			);
				R_GET_STR( recordset, vTemp, "TITLE",			sendReq_m.TITLE				);
				R_GET_STR( recordset, vTemp, "REQ_TYPE",		sendReq_m.REQ_TYPE			);
				R_GET_STR( recordset, vTemp, "REQUESTER_TYPE",	sendReq_m.REQUESTER_TYPE	);
				R_GET_STR( recordset, vTemp, "REQ_USER_ID",		sendReq_m.REQ_USER_ID		);
				R_GET_STR( recordset, vTemp, "REQ_USER_NAME",	sendReq_m.REQ_USER_NAME		);
				R_GET_STR( recordset, vTemp, "REQ_USER_TELNO",	sendReq_m.REQ_USER_TEL_NO	);
				R_GET_DAT( recordset, vTemp, "REQ_DATE",		sendReq_m.REQ_DATE			);
				R_GET_STR( recordset, vTemp, "SMS_CONTENTS",	sendReq_m.SMS_CONTENTS		);
				R_GET_STR( recordset, vTemp, "SMS_SEND_YN",		sendReq_m.SMS_SEND_YN		);
				R_GET_STR( recordset, vTemp, "TEST_TYPE",		sendReq_m.TEST_TYPE			);
				R_GET_STR( recordset, vTemp, "BROADCAST_YN",	sendReq_m.BROADCAST_YN		);

				vecTemp_sendReqs_m.push_back(sendReq_m);

				recordset->MoveNext();
			}


			//// P3. 일반건-Detail Row(s) 얻기
			recordset = recordset->NextRecordset((VARIANT*)lTemp);
			if(recordset == 0)
				return 0;

			nCnt = recordset->RecordCount;
            

			for(i = 0 ; i < nCnt ; ++i)
			{
				sendReq.clear();
			
				R_GET_DEC( recordset, vTemp, "SEQ",				sendReq.detail.SEQ				);
				R_GET_DEC( recordset, vTemp, "FAX_ID",			sendReq.detail.FAX_ID			);
				R_GET_STR( recordset, vTemp, "STATE_EACH",		sendReq.detail.STATE_EACH		);
				R_GET_STR( recordset, vTemp, "FAX_NO",			sendReq.detail.FAX_NO			);

                // ADD - KIMCG : 팩스번호 복호화 처리
                if(CConfig::ENCRYPT_FIELD_YN == "Y")
                {
                    if(EncryptApi::Inst()->TryDecrypt(sendReq.detail.FAX_NO, strTemp))
                        sendReq.detail.FAX_NO = strTemp;
                }
                // ADD - END

				R_GET_STR( recordset, vTemp, "RECIPIENT_NAME",	sendReq.detail.RECIPIENT_NAME	);
				R_GET_STR( recordset, vTemp, "TIF_FILE",		sendReq.detail.TIF_FILE			);
				R_GET_NUM( recordset, vTemp, "TIF_PAGE_CNT",	sendReq.detail.TIF_PAGE_CNT		);
				R_GET_STR( recordset, vTemp, "PAGES_TO_SEND",	sendReq.detail.PAGES_TO_SEND	);
				R_GET_NUM( recordset, vTemp, "LAST_PAGE_SENT",	sendReq.detail.LAST_PAGE_SENT	);
				R_GET_STR( recordset, vTemp, "TITLE",			sendReq.detail.TITLE			);
				R_GET_STR( recordset, vTemp, "RESULT",			sendReq.detail.RESULT			);
				R_GET_STR( recordset, vTemp, "REASON",			sendReq.detail.REASON			);
				R_GET_NUM( recordset, vTemp, "TRY_CNT",			sendReq.detail.TRY_CNT			);
				R_GET_STR( recordset, vTemp, "SMSNO",			sendReq.detail.SMSNO			);
				sendReq.detail.beginTime = timeCurrent.Format("%Y/%m/%d %H:%M:%S");

				#pragma warning ( push )
				#pragma warning( disable : 4996 )
				_splitpath( sendReq.detail.TIF_FILE, NULL, szSubFolder, szTifName, szTifExt );
				#pragma warning( pop )
				sendReq.detail.tifFile_send.Format("%s%s%s", (LPCSTR)CConfig::LOCAL_TIF_PATH, szTifName, szTifExt);
				sendReq.detail.tifFile_storage	= CConfig::FINISHED_TIF_FULL_PATH + "\\" + sendReq.detail.TIF_FILE;

				for(j=0 ;  ; ++j)
				{
					if( j >= (int)vecTemp_sendReqs_m.size())
						return -2;

					if(sendReq.detail.FAX_ID == vecTemp_sendReqs_m[j].FAX_ID) {
						sendReq.master = vecTemp_sendReqs_m[j];
						break;
					}
				}

				vecSendReqs.push_back(sendReq);

				recordset->MoveNext();
			}
			vecTemp_sendReqs_m.clear();

			//// P4. 동보건-Master Row(s) 얻기
			recordset = recordset->NextRecordset((VARIANT*)lTemp);
			if(recordset == 0)
				return 0;

			nCnt = recordset->RecordCount;
			for(i = 0 ; i < nCnt ; ++i)
			{	
				sendReq_m.clear();

				R_GET_DEC( recordset, vTemp, "FAX_ID",			sendReq_m.FAX_ID			);
				R_GET_STR( recordset, vTemp, "TR_NO",			sendReq_m.TR_NO				);
				R_GET_STR( recordset, vTemp, "STATE",			sendReq_m.STATE				);
				R_GET_STR( recordset, vTemp, "PRIORITY",		sendReq_m.PRIORITY			);
				R_GET_STR( recordset, vTemp, "TITLE",			sendReq_m.TITLE				);
				R_GET_STR( recordset, vTemp, "REQ_TYPE",		sendReq_m.REQ_TYPE			);
				R_GET_STR( recordset, vTemp, "REQUESTER_TYPE",	sendReq_m.REQUESTER_TYPE	);
				R_GET_STR( recordset, vTemp, "REQ_USER_ID",		sendReq_m.REQ_USER_ID		);
				R_GET_STR( recordset, vTemp, "REQ_USER_NAME",	sendReq_m.REQ_USER_NAME		);
				R_GET_STR( recordset, vTemp, "REQ_USER_TELNO",	sendReq_m.REQ_USER_TEL_NO	);
				R_GET_DAT( recordset, vTemp, "REQ_DATE",		sendReq_m.REQ_DATE			);
				R_GET_STR( recordset, vTemp, "SMS_CONTENTS",	sendReq_m.SMS_CONTENTS		);
				R_GET_STR( recordset, vTemp, "SMS_SEND_YN",		sendReq_m.SMS_SEND_YN		);
				R_GET_STR( recordset, vTemp, "TEST_TYPE",		sendReq_m.TEST_TYPE			);
				R_GET_STR( recordset, vTemp, "BROADCAST_YN",	sendReq_m.BROADCAST_YN		);

				vecTemp_sendReqs_m.push_back(sendReq_m);

				recordset->MoveNext();
			}


			//// P5. 동보건-Detail Row(s) 얻기
			recordset = recordset->NextRecordset((VARIANT*)lTemp);
			if(recordset == 0)
				return 0;

			nCnt = recordset->RecordCount;
			for(i = 0 ; i < nCnt ; ++i)
			{
				sendReq.clear();
			
				R_GET_DEC( recordset, vTemp, "SEQ",				sendReq.detail.SEQ				);
				R_GET_DEC( recordset, vTemp, "FAX_ID",			sendReq.detail.FAX_ID			);
				R_GET_STR( recordset, vTemp, "STATE_EACH",		sendReq.detail.STATE_EACH		);
				R_GET_STR( recordset, vTemp, "FAX_NO",			sendReq.detail.FAX_NO			);
				R_GET_STR( recordset, vTemp, "RECIPIENT_NAME",	sendReq.detail.RECIPIENT_NAME	);
				R_GET_STR( recordset, vTemp, "TIF_FILE",		sendReq.detail.TIF_FILE			);
				R_GET_NUM( recordset, vTemp, "TIF_PAGE_CNT",	sendReq.detail.TIF_PAGE_CNT		);
				R_GET_STR( recordset, vTemp, "PAGES_TO_SEND",	sendReq.detail.PAGES_TO_SEND	);
				R_GET_NUM( recordset, vTemp, "LAST_PAGE_SENT",	sendReq.detail.LAST_PAGE_SENT	);
				R_GET_STR( recordset, vTemp, "TITLE",			sendReq.detail.TITLE			);
				R_GET_STR( recordset, vTemp, "RESULT",			sendReq.detail.RESULT			);
				R_GET_STR( recordset, vTemp, "REASON",			sendReq.detail.REASON			);
				R_GET_NUM( recordset, vTemp, "TRY_CNT",			sendReq.detail.TRY_CNT			);
				R_GET_STR( recordset, vTemp, "SMSNO",			sendReq.detail.SMSNO			);
				sendReq.detail.beginTime = timeCurrent.Format("%Y/%m/%d %H:%M:%S");

				#pragma warning ( push )
				#pragma warning( disable : 4996 )
				_splitpath( sendReq.detail.TIF_FILE, NULL, szSubFolder, szTifName, szTifExt );
				#pragma warning ( pop )
				sendReq.detail.tifFile_send.Format( "%s\\%s%s", (LPCSTR)CConfig::LOCAL_TIF_PATH, szTifName, szTifExt );
				sendReq.detail.tifFile_storage	= CConfig::FINISHED_TIF_FULL_PATH + "\\" + sendReq.detail.TIF_FILE;

				for(j=0 ;  ; ++j)
				{
					if( j >= (int)vecTemp_sendReqs_m.size())
						return -2;

					if(sendReq.detail.FAX_ID == vecTemp_sendReqs_m[j].FAX_ID) {
						sendReq.master = vecTemp_sendReqs_m[j];
						break;
					}
				}

				vecSendReqs_broad.push_back(sendReq);

				recordset->MoveNext();
			}

			nRow = vecSendReqs.size() + vecSendReqs_broad.size();
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_FOD.USP_OCCUPY_SEND_REQ", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}

int CDbModule::RetrySendReq( CDbModule::SEND_REQ& p_sendReq, int p_fodChannel, int p_result, int p_delayTime )
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	
	int				nRow = -1;
	int				effectCnt = -2;
	char			szResult[10];
	DECIMAL			faxID, faxID_d;


	{CDbConnector connector( CDbConnection::Inst() );

		memset( szResult, 0x00, sizeof(szResult) );
		sprintf_s( szResult, "%03d", p_result );
		STR_TO_DEC( p_sendReq.detail.FAX_ID,	faxID	);
		STR_TO_DEC( p_sendReq.detail.SEQ,		faxID_d	);

		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_FOD.USP_RETRY_SEND_REQ";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_FAX_DTL_ID",		adDecimal, adParamInput, sizeof(DECIMAL), faxID_d);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_FAX_ID",			adDecimal, adParamInput, sizeof(DECIMAL), faxID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_fod_channel",		adInteger, adParamInput, sizeof(int), p_fodChannel);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_system_process_id", adVarChar, adParamInput, strlen(CConfig::SYSTEM_PROCESS_ID), (_bstr_t)CConfig::SYSTEM_PROCESS_ID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_result_code_3",		adVarChar, adParamInput, strlen(szResult), (_bstr_t)szResult);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_DELAY_SECOND",		adInteger, adParamInput, sizeof(int), p_delayTime );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_LAST_PAGE_SENT",	adInteger, adParamInput, sizeof(int), p_sendReq.detail.LAST_PAGE_SENT);
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);

			nRow  = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_FOD.USP_RETRY_SEND_REQ", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}


int CDbModule::RetrySendReqEx( CDbModule::SEND_REQ& p_sendReq, int p_fodChannel, int p_result, int p_delayTime)
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	
	int				nRow = -1;
	int				effectCnt = -2;
	char			szResult[10];
	DECIMAL			faxID, faxID_d;


	{CDbConnector connector( CDbConnection::Inst() );

		memset( szResult, 0x00, sizeof(szResult) );
		sprintf_s( szResult, "%03d", p_result );
		STR_TO_DEC( p_sendReq.detail.FAX_ID,	faxID	);
		STR_TO_DEC( p_sendReq.detail.SEQ,		faxID_d	);

		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_FOD.USP_RETRY_SEND_REQ_EX";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_FAX_DTL_ID",		adDecimal, adParamInput, sizeof(DECIMAL), faxID_d);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_FAX_ID",			adDecimal, adParamInput, sizeof(DECIMAL), faxID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_fod_channel",		adInteger, adParamInput, sizeof(int), p_fodChannel);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_system_process_id", adVarChar, adParamInput, strlen(CConfig::SYSTEM_PROCESS_ID), (_bstr_t)CConfig::SYSTEM_PROCESS_ID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_result_code_3",		adVarChar, adParamInput, strlen(szResult), (_bstr_t)szResult);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_DELAY_SECOND",		adInteger, adParamInput, sizeof(int), p_delayTime );
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_LAST_PAGE_SENT",	adInteger, adParamInput, sizeof(int), p_sendReq.detail.LAST_PAGE_SENT);
					pSQLCommand->Parameters->Append(param);
            param = pSQLCommand->CreateParameter("P_RETRY_CNT",	        adInteger, adParamInput, sizeof(int), p_sendReq.detail.TRY_CNT);
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);

			nRow  = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
            APPLOG->Print(DBGLV_ERR, "PKG_PRC_FOD.USP_RETRY_SEND_REQ_EX %s", e.ErrorMessage());
			SetErrorMessage("PKG_PRC_FOD.USP_RETRY_SEND_REQ_EX", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}

// -------------------------------------------------------------
// Module 명	: int P_FINISH_SEND_REQ()
// -------------------------------------------------------------
// Description	: FAX 발송 결과를 설정한다.
// -------------------------------------------------------------
// Argument		: int ID_REQ;			ID_REQ
//				  char* pSysProcType;	System Process Type (FOD_01_FSD_01)
//				  int fod_channel;		발송 Channel
//				  int try_cnt;			재시도 회수
//				  int iResult;			실패 사유
// -------------------------------------------------------------
// Return		: 1			SUCC
//				  0			Update 실패
//				 -1			SQLException 발생
//				 -2			기타 Procedure 결과 오류
//				 -99			Oracle DB Query 준비 상태가 아님
//				 -91			DB 예외 오류
// -------------------------------------------------------------
int CDbModule::FinishSendReq( CDbModule::SEND_REQ& p_sendReq, int p_fodChannel, int p_result )
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	
	int				nRow = -1;
	int				effectCnt = -2;
	char			szResult[10];
	DECIMAL			faxID, faxID_d;


	{CDbConnector connector( CDbConnection::Inst() );
	

		memset( szResult, 0x00, sizeof(szResult) );
		sprintf_s( szResult, "%03d", p_result );
		STR_TO_DEC( p_sendReq.detail.FAX_ID,	faxID	);
		STR_TO_DEC( p_sendReq.detail.SEQ,		faxID_d	);

		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_FOD.USP_FINISH_SEND_DTL";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_FAX_ID",			adDecimal, adParamInput, sizeof(DECIMAL), faxID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_FAX_DTL_ID",		adDecimal, adParamInput, sizeof(DECIMAL), faxID_d);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_PROCESS_TYPE",		adVarChar, adParamInput, strlen(CConfig::PROCESS_TYPE_STR), (_bstr_t)CConfig::PROCESS_TYPE_STR);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_LAST_PAGE_SENT",	adInteger, adParamInput, sizeof(int), CConfig::SYSTEM_NO);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_system_process_id", adVarChar, adParamInput, strlen(CConfig::SYSTEM_PROCESS_ID), (_bstr_t)CConfig::SYSTEM_PROCESS_ID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_fod_channel",		adInteger, adParamInput, sizeof(int), (long)p_fodChannel);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_result_code_3",		adVarChar, adParamInput, strlen(szResult), (_bstr_t)szResult);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_BROADCAST_YN",		adVarChar, adParamInput, 1, (_bstr_t)p_sendReq.master.BROADCAST_YN);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_LAST_PAGE_SENT",	adInteger, adParamInput, sizeof(int), p_sendReq.detail.LAST_PAGE_SENT);
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);

			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_FOD.USP_FINISH_SEND_DTL", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}



// -------------------------------------------------------------
// Module 명	: int FinishSendReqEx()
// -------------------------------------------------------------
// Description	: FAX 발송 결과를 설정한다.
// -------------------------------------------------------------
// Argument		: int ID_REQ;			ID_REQ
//				  const char* p_mgwIp 리모트 아이피
//				  int p_mgwPort 리모트 포트
//				  int fod_channel;		발송 Channel
//				  int try_cnt;			재시도 회수
//				  int iResult;			실패 사유
// -------------------------------------------------------------
// Return		: 1			SUCC
//				  0			Update 실패
//				 -1			SQLException 발생
//				 -2			기타 Procedure 결과 오류
//				 -99			Oracle DB Query 준비 상태가 아님
//				 -91			DB 예외 오류
// -------------------------------------------------------------
int CDbModule::FinishSendReqEx( SEND_REQ& p_sendReq, const char* p_mgwIp, int p_mgwPort, int p_fodChannel, int p_result )
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;

	int				nRow = -1;
	int				effectCnt = -2;
	char			szResult[10];		
	char			szSystemId[15];
	DECIMAL			faxID, faxID_d;

	{CDbConnector connector( CDbConnection::Inst() );
		memset( szResult, 0x00, sizeof(szResult) );
		sprintf_s( szResult, "%03d", p_result );

		memset( szSystemId, 0x00, sizeof(szSystemId) );
		sprintf_s( szSystemId, "%ld", CConfig::SYSTEM_NO );		

		STR_TO_DEC( p_sendReq.detail.FAX_ID,	faxID	);
		STR_TO_DEC( p_sendReq.detail.SEQ,		faxID_d	);

		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_FOD.USP_FINISH_SEND_DTL_EX";

    
			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_FAX_ID",			adDecimal, adParamInput, sizeof(DECIMAL), faxID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_FAX_DTL_ID",		adDecimal, adParamInput, sizeof(DECIMAL), faxID_d);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_PROCESS_TYPE",		adVarChar, adParamInput, strlen(CConfig::PROCESS_TYPE_STR), (_bstr_t)CConfig::PROCESS_TYPE_STR);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_SYSTEM_ID",			adVarChar, adParamInput, strlen(szSystemId), (_bstr_t)szSystemId);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_system_process_id", adVarChar, adParamInput, strlen(CConfig::SYSTEM_PROCESS_ID), (_bstr_t)CConfig::SYSTEM_PROCESS_ID);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_remote_ip",			adVarChar, adParamInput, sizeof(_variant_t), (_bstr_t)p_mgwIp);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_remote_port",		adInteger, adParamInput, sizeof(int), (long)p_mgwPort);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_fod_channel",		adInteger, adParamInput, sizeof(int), (long)p_fodChannel);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_result_code_3",		adVarChar, adParamInput, strlen(szResult), (_bstr_t)szResult);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_BROADCAST_YN",		adVarChar, adParamInput, 1, (_bstr_t)p_sendReq.master.BROADCAST_YN);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("P_LAST_PAGE_SENT",	adInteger, adParamInput, sizeof(int), p_sendReq.detail.LAST_PAGE_SENT);
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);

			nRow = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
            APPLOG->Print(DBGLV_ERR, "PKG_PRC_FOD.USP_FINISH_SEND_DTL_EX %s", e.ErrorMessage());
			SetErrorMessage("PKG_PRC_FOD.USP_FINISH_SEND_DTL_EX", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
			APPLOG->Print(DBGLV_ERR, "%s", e.ErrorMessage());
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}

int CDbModule::ReadySendReq()
{
	_CommandPtr		pSQLCommand	= NULL;
	_ParameterPtr	param		= NULL;
	
	int				nRow = -1;
	int				effectCnt = -2;
	char			szResult[10];
	DECIMAL			faxID, faxID_d;


	{CDbConnector connector( CDbConnection::Inst() );
	

		// P1. Oracle Stored Procedure Command 생성
		try
		{
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_FOD.USP_READY_SEND_DTL";

			// 저장프로시져의 파라메터(Input) 을 지정함.
			param = pSQLCommand->CreateParameter("P_PROCESS_TYPE",		adVarChar, adParamInput, strlen(CConfig::PROCESS_TYPE_STR), (_bstr_t)CConfig::PROCESS_TYPE_STR);
					pSQLCommand->Parameters->Append(param);
			param = pSQLCommand->CreateParameter("p_system_process_id", adVarChar, adParamInput, strlen(CConfig::SYSTEM_PROCESS_ID), (_bstr_t)CConfig::SYSTEM_PROCESS_ID);
					pSQLCommand->Parameters->Append(param);

			// 명령실행
			_variant_t	recordsAffected = (long)0;
			pSQLCommand->Execute(&recordsAffected, NULL, adCmdStoredProc);

			nRow  = 1;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			SetErrorMessage("PKG_PRC_FOD.USP_READY_SEND_DTL", e);
			nRow = -91;
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return nRow;
}

bool CDbModule::IsSendOffFaxNo(const char* p_faxNo)
{
	_CommandPtr		pSQLCommand			= NULL;
	_RecordsetPtr	recordset			= NULL;
	_ParameterPtr	param				= NULL;
	_variant_t		vTemp;

	int nCnt = 0;	
	bool ret = false;

	{CDbConnector connector( CDbConnection::Inst() );

		try
		{
			//// P1. Procedure 실행
			pSQLCommand.CreateInstance(__uuidof(Command));
			//pSQLCommand->ActiveConnection	= m_pConn;
			pSQLCommand->ActiveConnection	= CDbConnection::Inst()->Connection();

			pSQLCommand->CommandType		= adCmdStoredProc;
			pSQLCommand->CommandText		= "PKG_PRC_FOD.USP_SELECT_SEND_OFF_FAX_NUMBER";
		
			// Input 파라미터 지정									
			param = pSQLCommand->CreateParameter("P_FAX_NO",	adVarChar, adParamInput, strlen(p_faxNo), (_bstr_t)p_faxNo);
					pSQLCommand->Parameters->Append(param);

			// Command 실행
			recordset = pSQLCommand->Execute(NULL, NULL, adCmdStoredProc);
			if( recordset == NULL )
				return false;

			CTime timeCurrent = CTime::GetCurrentTime();
		

			//// P2. 일반건-Master Row(s) 얻기
			nCnt = recordset->RecordCount;
			if(nCnt >= 1)
				ret = true;
		}

		// P2. Oracle Delete Command 생성 실패 처리
		catch (_com_error &e)
		{
			ret = false;
			SetErrorMessage("PKG_PRC_FOD.USP_SELECT_SEND_OFF_FAX_NUMBER", e);
			APPLOG->Print(DBGLV_ERR, "%s", e.ErrorMessage());
			CDbConnection::Inst()->Disconnect();
		}
	
		if(pSQLCommand != NULL)
			pSQLCommand.Release();
	}

	return ret;
}

void CDbModule::DbLock()
{
	m_dbLock.Lock();
}

void CDbModule::DbUnlock()
{
	m_dbLock.Unlock();
}



