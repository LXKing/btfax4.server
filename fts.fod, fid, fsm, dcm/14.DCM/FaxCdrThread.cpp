#include <windows.h> 
#include "StdAfx.h"
#include "APP.h"
#include "FaxCdrThread.h"
#include "Config.h"
#include "enum.h"
#include "iBulkFile.h"
#include "IConvEncoder.h"

using namespace COMMON_LIB;

CFaxCdrThread::CFaxCdrThread(void)
{
	m_logPrefix = "IF_CDR";
}


CFaxCdrThread::~CFaxCdrThread(void)
{
}

// implementation
bool CFaxCdrThread::StartThread()
{	
	iBulkFile::Inst()->Init(CConfig::LOG_PATH, CConfig::CDR_PATH, "ifcdr");
	iBulkFile::Inst()->SetLogLvl(1);
	iBulkFile::Inst()->SetOldFileChkDur(3600);
	
	char msg[128];
	memset(msg, 0x00, sizeof(msg));
	sprintf(msg, "iBulk Init(%s)", CConfig::CDR_PATH);
	LogInfo(msg);
	
	Start();
	return true;
}

bool CFaxCdrThread::StopThread()
{	
	Stop(true);
	return true;
}

// override
NPVOID CFaxCdrThread::ThreadRun(NPVOID pArg)
{
	LogInfo("Fax CDR thread start.");

	int				dbRet;					// DB 접속 결과
	CString			connStr, id, pwd;		// DB connection 정보
	CString			msg;
	CString			cdrData;
	char utfCdrData[4028];

	vector<CDbModule::FAX_CDR> faxCdrs;		// 송, 수신 CDR 정보
	while(IsOn())
	{	
		iSleep(CConfig::FAX_CDR_DB_POLLING_SLEEP);
		if(!IsRunning())
			break;
		
		// Clear memory
		faxCdrs.clear();
		
		// Get Send CDR from Db
		dbRet = CDbModule::Inst()->OccupySendCdr(10 , faxCdrs);
		if(dbRet < 0 ) 
		{	
			APPLOG->Print(DBGLV_ERR, "PKG_PRC_DCM.USP_OCCUPY_SEND_CDR!! F_DB_ERROR");
			continue;
		}
				
		// Get Receive CDR from Db
		dbRet = CDbModule::Inst()->OccupyRecvCdr(10 , faxCdrs);
		if(dbRet < 0 ) 
		{	
			APPLOG->Print(DBGLV_ERR, "PKG_PRC_DCM.USP_OCCUPY_SEND_CDR!! F_DB_ERROR");
			continue;
		}
		
		if(faxCdrs.size() <= 0)
			continue;
			
		// Write CDR 		
		for( int i = 0 ; i < (int)faxCdrs.size() ; i++ )
		{	
			cdrData = "";
			switch(faxCdrs[i].CDR_TYPE)
			{
				case SEND_CDR:
					MakeSendCdrData(cdrData, faxCdrs[i].send_cdr);
					break;

				case RECV_CDR:
					MakeRecvCdrData(cdrData, faxCdrs[i].recv_cdr);
					break;

				default:
					continue;
			}

			memset(utfCdrData, 0x00, sizeof(utfCdrData));
			strcpy(utfCdrData, cdrData);
			CIConvEncoder::ConvertToUtf8(utfCdrData, sizeof(utfCdrData));

            // for debug
            msg.Format("data size:%d, data=%s", strlen(utfCdrData), utfCdrData);
			LogInfo(LPSTR(LPCTSTR(msg)));

			if(iBulkFile::Inst()->Write(0, utfCdrData, strlen(utfCdrData)) == false)		
			{
				faxCdrs[i].DCM_STATE	= "N";
				faxCdrs[i].RESULT		= "F";
				faxCdrs[i].REASON		= "811";

				msg.Format("iBulkFile Write error. [%d]", iBulkFile::Inst()->GetError());
				LogErr(LPSTR(LPCTSTR(msg)));
				
				continue;
			}

			faxCdrs[i].DCM_STATE	= "Y";
			faxCdrs[i].RESULT		= "S";
			faxCdrs[i].REASON		= "0001";
		}
		
		// Finished Update CDR		
		int result, reason;
		for( int i = 0 ; i < (int)faxCdrs.size() ; i++ )
		{	
			switch(faxCdrs[i].CDR_TYPE)
			{
				case SEND_CDR:
					dbRet = CDbModule::Inst()->FinishSendCdr(faxCdrs[i]);
					if(dbRet < 0 )
					{
						LogErr("PKG_PRC_DCM.USP_SEND_CDR_DONE!");
						continue;
					}
					break;

				case RECV_CDR:
					dbRet = CDbModule::Inst()->FinishRecvCdr(faxCdrs[i]);
					if(dbRet < 0 )
					{
						LogErr("PKG_PRC_DCM.USP_RECV_CDR_DONE!");
						continue;
					}
					break;

				default:
					msg.Format("Invalid CDR_TYPE:%d", faxCdrs[i].CDR_TYPE);
					LogErr(LPSTR(LPCTSTR(msg)));
					continue;
			}

			msg.Format("Fax CDR result success. CDR_TYPE:%d", faxCdrs[i].CDR_TYPE);
			LogInfo(LPSTR(LPCTSTR(msg)));
		}
	}
	
	return 0;
}

void  CFaxCdrThread::MakeSendCdrData(CString& p_cdrData, CDbModule::FAX_SEND_CDR p_faxCdr)
{
	CString cdrTemp;
	cdrTemp.Format("CDR_TYPE := \"%s\""			, "1"						);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("FAX_SEND_ID := \"%s\""		, p_faxCdr.FAX_ID			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("FAX_DETL_ID := \"%s\""		, p_faxCdr.FAX_DTL_ID		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SERVICE_CODE := \"%s\""		, p_faxCdr.TR_NO			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SERVICE_NAME := \"%s\""		, p_faxCdr.SVC_NAME			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("PRIORITY := \"%s\""			, p_faxCdr.PRIORITY			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("REQ_TYPE := \"%s\""			, p_faxCdr.REQ_TYPE			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SENDER_TYPE := \"%s\""		, p_faxCdr.REQUESTER_TYPE	);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SENDER_ID := \"%s\""		, p_faxCdr.REQ_USER_ID		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SENDER_NAME := \"%s\""		, p_faxCdr.REQ_USER_NAME	);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("DEPT_CDOE := \"%s\""		, p_faxCdr.DEPT_CD			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("DEPT_NAME := \"%s\""		, p_faxCdr.DEPT_NAME		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("BIZ_ID := \"%s\""			, p_faxCdr.TASK_ID			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("BIZ_NAME := \"%s\""			, p_faxCdr.TASK_NAME		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("REQ_DATE := \"%s\""			, p_faxCdr.REQ_DATE			);  p_cdrData += cdrTemp + "\t";
	
	p_faxCdr.APPROVE_REQ = (p_faxCdr.APPROVE_REQ == "Y")? "1":"0"; 
	cdrTemp.Format("APPROVE_TYPE := \"%s\""		, p_faxCdr.APPROVE_REQ		);  p_cdrData += cdrTemp + "\t";

	p_faxCdr.APPROVED_YN = (p_faxCdr.APPROVED_YN == "Y")? "1":"0"; 
	cdrTemp.Format("APPROVE_YN := \"%s\""		, p_faxCdr.APPROVED_YN		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("APPROVE_ID := \"%s\""		, p_faxCdr.APPROVED_ID		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("APPROVE_NAME := \"%s\""		, ""						);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("APPROVE_TIME := \"%s\""		, p_faxCdr.APPROVED_DATE	);  p_cdrData += cdrTemp + "\t";

	p_faxCdr.RESERVED_YN = (p_faxCdr.RESERVED_YN == "Y")? "1":"0"; 
	cdrTemp.Format("RESERVE_YN := \"%s\""		, p_faxCdr.RESERVED_YN		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("RESERVE_SENDTIME := \"%s\""	, p_faxCdr.RESERVED_DATE	);  p_cdrData += cdrTemp + "\t";
	
	p_faxCdr.BROADCAST_YN = (p_faxCdr.BROADCAST_YN == "Y")? "1":"0"; 
	cdrTemp.Format("MULTISEND_YN := \"%s\""		, p_faxCdr.BROADCAST_YN		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("MULTISEND_CNT := \"%s\""	, p_faxCdr.BROADCAST_CNT	);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("RECVER_FAXNO := \"%s\""		, p_faxCdr.FAX_NO			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("RECVER_NAME := \"%s\""		, p_faxCdr.RECIPIENT_NAME	);  p_cdrData += cdrTemp + "\t";
	
	//cdrTemp.Format("TIFF_FILENAME := \"%s\""	, p_faxCdr.TIF_FILE			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("TIFF_FILENAME := \"%s\\%s\\%s\"", CConfig::FAX_CDR_STG_HOME_PATH, CConfig::FAX_CDR_FINISHED_TIF_PATH, p_faxCdr.TIF_FILE);
	p_cdrData += cdrTemp + "\t";

	cdrTemp.Format("TIFF_FILESIZE := \"%s\""	, p_faxCdr.TF_FILE_SIZE		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("TIFF_START_TIME := \"%s\""	, p_faxCdr.TIF_MAKE_B_DATE	);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("TIFF_FINSH_TIME := \"%s\""	, p_faxCdr.TIF_MAKE_E_DATE	);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SYSTEM_ID := \"%s\""		, p_faxCdr.SYSTEM_ID		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("CHNL_NO := \"%s\""			, p_faxCdr.CHANNEL_NO		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("MGW_SYSTEM_IP := \"%s\""	, p_faxCdr.MGW_IP			);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SEND_PAGECNT := \"%s\""		, p_faxCdr.TIF_PAGE_CNT		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SEND_SCHEDULE_TIME := \"%s\"", p_faxCdr.DATE_TO_SEND		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SEND_START_TIME := \"%s\""	, p_faxCdr.DATE_TO_SEND		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SEND_FINSH_TIME := \"%s\""	, p_faxCdr.RESULT_DATE		);  p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SEND_TRYCNT := \"%s\""		, p_faxCdr.TRY_CNT			);  p_cdrData += cdrTemp + "\t";

	p_faxCdr.RESULT = (p_faxCdr.RESULT == "S")? "1":"0"; 
	cdrTemp.Format("SEND_RESULT_CD := \"%s\""	, p_faxCdr.RESULT	        );  p_cdrData += cdrTemp + "\t";
	
	p_faxCdr.REASON = p_faxCdr.REASON.Mid(1);
	cdrTemp.Format("SEND_REASON_CD := \"%d\""	, atoi(p_faxCdr.REASON)	    );  p_cdrData += cdrTemp + "\t";
	p_cdrData += "\r\n";

	// 추가 사용가능한 데이터 : 필요시 주석해제
	//cdrTemp.Format("STATE                   = \"%s\" ", p_faxCdr.STATE                    );  p_cdrData += cdrTemp;
	//cdrTemp.Format("INDEX_NO                = \"%s\" ", p_faxCdr.INDEX_NO                 );  p_cdrData += cdrTemp;
	//cdrTemp.Format("REQ_USER_TELNO          = \"%s\" ", p_faxCdr.REQ_USER_TELNO           );  p_cdrData += cdrTemp;
	//cdrTemp.Format("TITLE                   = \"%s\" ", p_faxCdr.TITLE                    );  p_cdrData += cdrTemp;
	//cdrTemp.Format("MEMO                    = \"%s\" ", p_faxCdr.MEMO                     );  p_cdrData += cdrTemp;
	//cdrTemp.Format("PRE_PROCESS_REQ         = \"%s\" ", p_faxCdr.PRE_PROCESS_REQ          );  p_cdrData += cdrTemp;
	//cdrTemp.Format("PREVIEW_REQ             = \"%s\" ", p_faxCdr.PREVIEW_REQ              );  p_cdrData += cdrTemp;
	//cdrTemp.Format("PREVIEWED_ID            = \"%s\" ", p_faxCdr.PREVIEWED_ID             );  p_cdrData += cdrTemp;
	//cdrTemp.Format("PREVIEWED_DATE          = \"%s\" ", p_faxCdr.PREVIEWED_DATE           );  p_cdrData += cdrTemp;
	//cdrTemp.Format("APPROVED_COMMENT        = \"%s\" ", p_faxCdr.APPROVED_COMMENT         );  p_cdrData += cdrTemp;
	//cdrTemp.Format("PROCESS_FETCH           = \"%s\" ", p_faxCdr.PROCESS_FETCH            );  p_cdrData += cdrTemp;
	//cdrTemp.Format("SMS_CONTENTS            = \"%s\" ", p_faxCdr.SMS_CONTENTS             );  p_cdrData += cdrTemp;
	//cdrTemp.Format("SMS_SEND_YN             = \"%s\" ", p_faxCdr.SMS_SEND_YN              );  p_cdrData += cdrTemp;
	//cdrTemp.Format("RESULT_FORWARD          = \"%s\" ", p_faxCdr.RESULT_FORWARD           );  p_cdrData += cdrTemp;
	//cdrTemp.Format("TEST_TYPE               = \"%s\" ", p_faxCdr.TEST_TYPE                );  p_cdrData += cdrTemp;
	//cdrTemp.Format("PROCESS_FETCH_DATE      = \"%s\" ", p_faxCdr.PROCESS_FETCH_DATE       );  p_cdrData += cdrTemp;	
	//cdrTemp.Format("STATE_EACH              = \"%s\" ", p_faxCdr.STATE_EACH               );  p_cdrData += cdrTemp;
	//cdrTemp.Format("PAGES_TO_SEND           = \"%s\" ", p_faxCdr.PAGES_TO_SEND            );  p_cdrData += cdrTemp;
	//cdrTemp.Format("LAST_PAGE_SENT          = \"%s\" ", p_faxCdr.LAST_PAGE_SENT           );  p_cdrData += cdrTemp;
	//cdrTemp.Format("TITLE_DTL               = \"%s\" ", p_faxCdr.TITLE_DTL                );  p_cdrData += cdrTemp;
	//cdrTemp.Format("MEMO_DTL                = \"%s\" ", p_faxCdr.MEMO_DTL                 );  p_cdrData += cdrTemp;
	//cdrTemp.Format("SMSNO                   = \"%s\" ", p_faxCdr.SMSNO                    );  p_cdrData += cdrTemp;
	//cdrTemp.Format("PROCESS_FETCH_EACH      = \"%s\" ", p_faxCdr.PROCESS_FETCH_EACH       );  p_cdrData += cdrTemp;
	//cdrTemp.Format("PROCESS_FETCH_DATE_EACH = \"%s\" ", p_faxCdr.PROCESS_FETCH_DATE_EACH  );  p_cdrData += cdrTemp;
	//cdrTemp.Format("POST_SND_PROC_YN        = \"%s\" ", p_faxCdr.POST_SND_PROC_YN         );  p_cdrData += cdrTemp;	
	//cdrTemp.Format("MGW_PORT                = \"%s\" ", p_faxCdr.MGW_PORT                 );  p_cdrData += cdrTemp;
	//cdrTemp.Format("DCM_STATE               = \"%s\" ", p_faxCdr.DCM_STATE                );  p_cdrData += cdrTemp;
}

void  CFaxCdrThread::MakeRecvCdrData(CString& p_cdrData, CDbModule::FAX_RECV_CDR p_faxCdr)
{
	CString cdrTemp;
	cdrTemp.Format("CDR_TYPE := \"%s\""			, "2"						); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("FAX_RECV_ID := \"%s\""		, p_faxCdr.FAX_ID           ); p_cdrData += cdrTemp + "\t";
	
	p_faxCdr.RECEIVE_TYPE = (p_faxCdr.RECEIVE_TYPE == "F")? "0":"1";
	cdrTemp.Format("RECV_TYPE := \"%s\""		, p_faxCdr.RECEIVE_TYPE     ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SENDER_FAXNO := \"%s\""		, p_faxCdr.CID              ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("SENDER_ID := \"%s\""		, ""						); p_cdrData += cdrTemp + "\t";	
	cdrTemp.Format("SENDER_NAME := \"%s\""		, p_faxCdr.CID_NAME         ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("RECVER_SVC_NO := \"%s\""	, p_faxCdr.SERVICE_FAXNO    ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("RECVER_DID_NO := \"%s\""	, p_faxCdr.DID              ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("DEPT_CODE := \"%s\""		, p_faxCdr.DEPT_CD          ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("DEPT_NAME := \"%s\""		, p_faxCdr.DEPT_NAME        ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("BIZ_ID := \"%s\""			, p_faxCdr.TASK_ID          ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("BIZ_NAME := \"%s\""			, p_faxCdr.TASK_NAME        ); p_cdrData += cdrTemp + "\t";
    
	cdrTemp.Format("SYSTEM_ID := \"%s\""		, p_faxCdr.SYSTEM_ID        ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("CHNL_NO := \"%s\""			, p_faxCdr.CHANNEL_NO       ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("MGW_SYSTEM_IP := \"%s\""	, p_faxCdr.MGW_IP			); p_cdrData += cdrTemp + "\t";
	
	p_faxCdr.RESULT = (p_faxCdr.RESULT == "S")? "1":"0";
	cdrTemp.Format("RECV_STATUS := \"%s\""		, p_faxCdr.RESULT           ); p_cdrData += cdrTemp + "\t";
	
	//cdrTemp.Format("TIFF_FILENAME := \"%s\""	, p_faxCdr.TIF_FILE			);  p_cdrData += cdrTemp + "\t";	
	cdrTemp.Format("TIFF_FILENAME := \"%s\\%s\\%s\"", CConfig::FAX_CDR_STG_HOME_PATH, CConfig::FAX_CDR_INBOUND_TIF_PATH, p_faxCdr.TIF_FILE);		
	p_cdrData += cdrTemp + "\t";

	cdrTemp.Format("TIFF_FILESIZE := \"%s\""	, p_faxCdr.TIF_FILE_SIZE    ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("TIFF_PAGECNT := \"%s\""		, p_faxCdr.TIF_PAGE_CNT     ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("RECV_START_TIME := \"%s\""	, p_faxCdr.RECV_B_DATE      ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("RECV_FINSH_TIME := \"%s\""	, p_faxCdr.RECV_E_DATE      ); p_cdrData += cdrTemp + "\t";
	
	p_faxCdr.RECV_RESULT = (p_faxCdr.RECV_RESULT == "S")? "1":"0";
	cdrTemp.Format("RECV_RESULT_CD := \"%s\""	, p_faxCdr.RECV_RESULT      ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("RECV_REASON_CD := \"%d\""	, atoi(p_faxCdr.RECV_REASON)); p_cdrData += cdrTemp + "\t";
	
	cdrTemp.Format("ROUTE_START_TIME := \"%s\""	, p_faxCdr.FORWARD_B_DATE   ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("ROUTE_FINSH_TIME := \"%s\""	, p_faxCdr.FORWARD_E_DATE   ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("ROUTE_RESULT_CD := \"%s\""	, p_faxCdr.RESULT           ); p_cdrData += cdrTemp + "\t";
	cdrTemp.Format("ROUTE_REASON_CD := \"%d\""	, atoi(p_faxCdr.REASON)     ); p_cdrData += cdrTemp + "\t";
	p_cdrData += "\r\n";
	
	// 추가 사용가능한 데이터 : 필요시 주석해제
    //cdrTemp.Format("USER_ID := \"%s\""			, p_faxCdr.USER_ID          ); p_cdrData += cdrTemp + "\t";
    //cdrTemp.Format("USER_NAME := \"%s\""		, p_faxCdr.USER_NAME        ); p_cdrData += cdrTemp + "\t";
	//cdrTemp.Format("TITLE               = \"%s\" ", p_faxCdr.TITLE              ); p_cdrData += cdrTemp;
	//cdrTemp.Format("MEMO                = \"%s\" ", p_faxCdr.MEMO               ); p_cdrData += cdrTemp;
	//cdrTemp.Format("SPLIT_YN            = \"%s\" ", p_faxCdr.SPLIT_YN           ); p_cdrData += cdrTemp;
	//cdrTemp.Format("REASON              = \"%s\" ", p_faxCdr.REASON             ); p_cdrData += cdrTemp;
	//cdrTemp.Format("RECV_DATE           = \"%s\" ", p_faxCdr.RECV_DATE          ); p_cdrData += cdrTemp;
	//cdrTemp.Format("WORK_STATE          = \"%s\" ", p_faxCdr.WORK_STATE         ); p_cdrData += cdrTemp;
	//cdrTemp.Format("PROCESS_FETCH       = \"%s\" ", p_faxCdr.PROCESS_FETCH      ); p_cdrData += cdrTemp;
	//cdrTemp.Format("FORWARD_YN          = \"%s\" ", p_faxCdr.FORWARD_YN         ); p_cdrData += cdrTemp;
	//cdrTemp.Format("FORWARD_USER_ID     = \"%s\" ", p_faxCdr.FORWARD_USER_ID    ); p_cdrData += cdrTemp;
	//cdrTemp.Format("ORG_SERVICE_FAXNO   = \"%s\" ", p_faxCdr.ORG_SERVICE_FAXNO  ); p_cdrData += cdrTemp;
	//cdrTemp.Format("ORG_CID             = \"%s\" ", p_faxCdr.ORG_CID            ); p_cdrData += cdrTemp;
	//cdrTemp.Format("ORG_CID_NAME        = \"%s\" ", p_faxCdr.ORG_CID_NAME       ); p_cdrData += cdrTemp;
	//cdrTemp.Format("ORG_RECV_DATE       = \"%s\" ", p_faxCdr.ORG_RECV_DATE      ); p_cdrData += cdrTemp;
	//cdrTemp.Format("MGW_PORT            = \"%s\" ", p_faxCdr.MGW_PORT           ); p_cdrData += cdrTemp;
	//cdrTemp.Format("STATE               = \"%s\" ", p_faxCdr.STATE              ); p_cdrData += cdrTemp;
	//cdrTemp.Format("DCM_STATE			= \"%s\" ", p_faxCdr.DCM_STATE			); p_cdrData += cdrTemp;
}


//--------------------------------------------------------
// Title	: LogInfo
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 메시지 로그
//--------------------------------------------------------
void CFaxCdrThread::LogInfo(char* p_msg)
{	
	if(!IsRunning())
		return;

	APPLOG->Print(DBGLV_INF0, "[%-10s] %s"
					, m_logPrefix
					, p_msg
					);
}


//--------------------------------------------------------
// Title	: LogErr
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 오류 로그
//--------------------------------------------------------
void CFaxCdrThread::LogErr(char* p_msg)
{
	if(!IsRunning())
		return;

	APPLOG->Print(DBGLV_ERR, "[%-10s] %s"
					, m_logPrefix
					, p_msg
					);
}
