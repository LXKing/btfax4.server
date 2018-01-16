// DbModule.h: interface for the CDbModule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ORAADODB_H__3DEB4F64_23D1_498F_B9C0_9898597073BD__INCLUDED_)
#define AFX_ORAADODB_H__3DEB4F64_23D1_498F_B9C0_9898597073BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <string>
#include <vector>
using namespace std;

#include "enum.h"
using namespace COMMON_LIB;

#include "DbModuleBase.h"


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

#include "WTypes.h"


class CDbModule : public CDbModuleBase
{
public:
	static CDbModule* Inst();
protected:
	static CDbModule* s_pInstance;


public:

	struct FAX_SEND_CDR
	{
		CString FAX_ID					,FAX_DTL_ID   		,INDEX_NO 			,TR_NO 				,STATE;
		CString PRIORITY 				,TITLE 				,MEMO 				,PRE_PROCESS_REQ   	,REQ_TYPE;
		CString REQUESTER_TYPE			,REQ_USER_ID   		,REQ_USER_NAME 		,REQ_USER_TELNO   	,REQ_DATE;
		CString PREVIEW_REQ   			,PREVIEWED_ID 		,PREVIEWED_DATE   	,APPROVE_REQ   		,APPROVED_YN;
		CString APPROVED_ID   			,APPROVED_DATE 		,APPROVED_COMMENT  	,RESERVED_YN   		,RESERVED_DATE;
		CString PROCESS_FETCH 			,SMS_CONTENTS 		,SMS_SEND_YN   		,RESULT_FORWARD   	,TEST_TYPE;
		CString BROADCAST_YN 			,PROCESS_FETCH_DATE ,BROADCAST_CNT 		,STATE_EACH   		,FAX_NO;
		CString RECIPIENT_NAME  		,TIF_FILE 			,TF_FILE_SIZE 		,TIF_PAGE_CNT 		,PAGES_TO_SEND;
		CString LAST_PAGE_SENT  		,TITLE_DTL 			,MEMO_DTL 			,RESULT  			,REASON;
		CString TRY_CNT  				,SMSNO 				,PROCESS_FETCH_EACH ,DATE_TO_SEND 		,RESULT_DATE;
		CString PROCESS_FETCH_DATE_EACH ,POST_SND_PROC_YN  	,TIF_MAKE_B_DATE   	,TIF_MAKE_E_DATE   	,SYSTEM_ID;
		CString MGW_IP  				,MGW_PORT 			,DEPT_CD  			,DEPT_NAME 			,TASK_ID;
		CString TASK_NAME 				,DCM_STATE 			,SVC_NAME 			,CHANNEL_NO;
		
		void clear()
		{
			FAX_ID					= "";			FAX_DTL_ID              = "";			INDEX_NO                = "";			
			TR_NO                   = "";			STATE                   = "";			PRIORITY                = "";			
			TITLE                   = "";			MEMO                    = "";			PRE_PROCESS_REQ         = "";			
			REQ_TYPE                = "";			REQUESTER_TYPE          = "";			REQ_USER_ID             = "";			
			REQ_USER_NAME           = "";			REQ_USER_TELNO          = "";			REQ_DATE                = "";
			PREVIEW_REQ             = "";			PREVIEWED_ID            = "";			PREVIEWED_DATE          = "";			
			APPROVE_REQ             = "";			APPROVED_YN             = "";			APPROVED_ID             = "";			
			APPROVED_DATE           = "";			APPROVED_COMMENT        = "";			RESERVED_YN             = "";			
			RESERVED_DATE           = "";			PROCESS_FETCH           = "";			SMS_CONTENTS            = "";			
			SMS_SEND_YN             = "";			RESULT_FORWARD          = "";			TEST_TYPE               = "";
			BROADCAST_YN            = "";			PROCESS_FETCH_DATE      = "";			BROADCAST_CNT           = "";			
			STATE_EACH              = "";			FAX_NO                  = "";			RECIPIENT_NAME          = "";			
			TIF_FILE                = "";			TF_FILE_SIZE            = "";			TIF_PAGE_CNT            = "";			
			PAGES_TO_SEND           = "";			LAST_PAGE_SENT          = "";			TITLE_DTL               = "";			
			MEMO_DTL                = "";			RESULT                  = "";			REASON                  = "";
			TRY_CNT                 = "";			SMSNO                   = "";			PROCESS_FETCH_EACH      = "";			
			DATE_TO_SEND            = "";			RESULT_DATE             = "";			PROCESS_FETCH_DATE_EACH = "";			
			POST_SND_PROC_YN        = "";			TIF_MAKE_B_DATE         = "";			TIF_MAKE_E_DATE         = "";			
			SYSTEM_ID               = "";			MGW_IP                  = "";			MGW_PORT                = "";			
			DEPT_CD                 = "";			DEPT_NAME               = "";			TASK_ID                 = "";
			TASK_NAME               = "";			DCM_STATE               = "";			SVC_NAME                = "";			
			CHANNEL_NO              = "";
		}

	};

	struct FAX_RECV_CDR
	{
		CString FAX_ID		  ,	RECEIVE_TYPE   , SERVICE_FAXNO     ,DID               ,CID;
		CString CID_NAME      ,	TIF_FILE       , TIF_FILE_SIZE     ,TIF_PAGE_CNT      ,TITLE;
		CString MEMO          ,	SPLIT_YN       , RESULT            ,REASON            ,RECV_DATE;
		CString WORK_STATE    ,	PROCESS_FETCH  , FORWARD_YN        ,FORWARD_USER_ID   ,ORG_SERVICE_FAXNO;
		CString ORG_CID       ,	ORG_CID_NAME   , ORG_RECV_DATE     ,SYSTEM_ID         ,CHANNEL_NO;
		CString FORWARD_B_DATE,	FORWARD_E_DATE , RECV_RESULT       ,RECV_REASON       ,MGW_IP;
		CString MGW_PORT      ,	RECV_B_DATE    , RECV_E_DATE       ,STATE             ,DEPT_CD;
		CString DEPT_NAME     ,	TASK_ID        , TASK_NAME         ,DCM_STATE         ,USER_ID;
        CString USER_NAME;


		void clear()
		{
			FAX_ID              = "";		RECEIVE_TYPE        = "";		SERVICE_FAXNO       = "";		
			DID                 = "";		CID                 = "";		CID_NAME            = "";		
			TIF_FILE            = "";		TIF_FILE_SIZE       = "";		TIF_PAGE_CNT        = "";		
			TITLE               = "";		MEMO                = "";		SPLIT_YN            = "";		
			RESULT              = "";		REASON              = "";		RECV_DATE           = "";
			WORK_STATE          = "";		PROCESS_FETCH       = "";		FORWARD_YN          = "";		
			FORWARD_USER_ID     = "";		ORG_SERVICE_FAXNO   = "";		ORG_CID             = "";		
			ORG_CID_NAME        = "";		ORG_RECV_DATE       = "";		SYSTEM_ID           = "";		
			CHANNEL_NO          = "";		FORWARD_B_DATE      = "";		FORWARD_E_DATE      = "";		
			RECV_RESULT         = "";		RECV_REASON         = "";		MGW_IP              = "";
			MGW_PORT            = "";		RECV_B_DATE         = "";		RECV_E_DATE         = "";		
			STATE               = "";		DEPT_CD             = "";		DEPT_NAME           = "";		
			TASK_ID             = "";		TASK_NAME           = "";		DCM_STATE			= "";					
            USER_ID             = "";       USER_NAME           = "";
		}
	};

	// type
	struct FAX_CDR
	{
		int		CDR_TYPE;	// 송,수신cdr 구분 - 1:송신 2:수신
		CString	RESULT;
		CString	REASON;
		CString	DCM_STATE;
		FAX_SEND_CDR	send_cdr;
		FAX_RECV_CDR	recv_cdr;
		void clear()
		{
			send_cdr.clear();
			recv_cdr.clear();

			CDR_TYPE	= 0;
			RESULT		= "";
			REASON		= "";
			DCM_STATE	= "";
		}
	};
	
	// fax send queue item 
	struct FAX_SEND_Q_ITEM
	{
		int SYSTEM_ID;
		CString	SENDQ_ID;
		CString	SYSTEM_NAME;
		CString	SENDQ_NAME;

		int completed_send_total;
		int send_sucess_total;
		int send_fail_total;
		int wait_processing_total;
		int wait_make_tiff_total;
		int wait_send_total;
		int sending_total;
		int sending_tiff_page_count_total;

		int  MAX_SEND_TRYCNT;
		int  MAX_TIFF_PAGECNT;
		int  MAX_TIFF_FILESIZE;
		int  MAX_TIFF_MAKETIME;
		int MAX_WAIT_TIME;
		int MAX_SENDING_TIME;
		int AVG_SEND_TRYCNT;
		int AVG_TIFF_PAGECNT;
		int AVG_TIFF_FILESIZE;
		int AVG_TIFF_MAKETIME;
		int AVG_WAIT_TIME;
		int AVG_SENDING_TIME;

		void clear()
		{
			SYSTEM_ID			= 0;
			SENDQ_ID			= "";
			SYSTEM_NAME			= "";
			SENDQ_NAME			= "";

			completed_send_total	= 0;
			send_sucess_total		= 0;
			send_fail_total			= 0;
			wait_processing_total	= 0;
			wait_make_tiff_total	= 0;
			wait_send_total			= 0;
			sending_total			= 0;
			sending_tiff_page_count_total = 0;

			MAX_SEND_TRYCNT		= 0;
			MAX_TIFF_PAGECNT    = 0;
			MAX_TIFF_FILESIZE   = 0;
			MAX_TIFF_MAKETIME   = 0;
			MAX_WAIT_TIME       = 0;
			MAX_SENDING_TIME    = 0;
			AVG_SEND_TRYCNT     = 0;
			AVG_TIFF_PAGECNT    = 0;
			AVG_TIFF_FILESIZE   = 0;
			AVG_TIFF_MAKETIME   = 0;
			AVG_WAIT_TIME       = 0;			
			AVG_SENDING_TIME    = 0;
		}
	};

	// fax recv queue item 
	struct FAX_RECV_Q_ITEM
	{
		int SYSTEM_ID;
		CString RECVQ_ID;
		CString SYSTEM_NAME;
		CString RECVQ_NAME;

		int completed_receive_total;
		int receving_total;
		int receive_sucess_total;
		int receive_fail_total;
		int receive_tiff_page_count_total;

		int MAX_TIFF_PAGECNT ;
		int MAX_TIFF_FILESIZE;
		int MAX_RECVING_TIME ;
		int AVG_TIFF_PAGECNT ;
		int AVG_TIFF_FILESIZE;
		int AVG_RECVING_TIME ;

		void clear()
		{
			SYSTEM_ID			= 0;
			RECVQ_ID			= "";
			SYSTEM_NAME			= "";
			RECVQ_NAME			= "";

			completed_receive_total = 0;
			receving_total			= 0;
			receive_sucess_total	= 0;
			receive_fail_total		= 0;
			receive_tiff_page_count_total = 0;

			MAX_TIFF_PAGECNT 	= 0;
			MAX_TIFF_FILESIZE	= 0;
			MAX_RECVING_TIME	= 0;
			AVG_TIFF_PAGECNT	= 0;
			AVG_TIFF_FILESIZE	= 0;
			AVG_RECVING_TIME	= 0;
		}
	};
		
	struct FAX_Q_ITEM
	{
		int queue_type;				// send_queue : 1, recv_queue : 2
		FAX_SEND_Q_ITEM send_queue;
		FAX_RECV_Q_ITEM recv_queue;

		void clear()
		{
			queue_type = 0;
			send_queue.clear();
			recv_queue.clear();
		}
	};

private:
	CDbModule();
	virtual ~CDbModule();

public:
	int	OccupySendCdr(int OccupyCdrCnt, vector<FAX_CDR>& vecSendCdrs);
	int	FinishSendCdr(FAX_CDR& p_faxCdr);

	int	OccupyRecvCdr(int OccupyCdrCnt, vector<FAX_CDR>& vecSendCdrs);
	int	FinishRecvCdr(FAX_CDR& p_faxCdr);
	int	SelectFaxQueue(FAX_Q_ITEM& p_faxQueueItem);

};

#endif // !defined(AFX_ORAADODB_H__3DEB4F64_23D1_498F_B9C0_9898597073BD__INCLUDED_)
