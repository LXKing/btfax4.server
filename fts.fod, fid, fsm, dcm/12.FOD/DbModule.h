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
	// type
	struct SEND_REQ_MSTR
	{
		CString	FAX_ID;
		CString TR_NO;
		CString SVC_NAME;
		CString STATE;
		CString PRIORITY;
		CString TITLE;
		CString REQ_TYPE;
		CString REQUESTER_TYPE;
		CString REQ_USER_ID;
		CString REQ_USER_NAME;
		CString REQ_USER_TEL_NO;
		CString	REQ_DATE;
		CString SMS_CONTENTS;
		CString SMS_SEND_YN;
		CString TEST_TYPE;
		CString	BROADCAST_YN;

		void clear()
		{
			FAX_ID			= "";
			TR_NO			= "";
			SVC_NAME		= "";
			STATE			= "";
			PRIORITY		= "";
			TITLE			= "";
			REQ_TYPE		= "";
			REQUESTER_TYPE	= "";
			REQ_USER_ID		= "";
			REQ_USER_NAME	= "";
			REQ_USER_TEL_NO = "";
			REQ_DATE		= "";
			SMS_CONTENTS	= "";
			SMS_SEND_YN		= "";
			TEST_TYPE		= "";
			BROADCAST_YN	= "";
		}
	};

	struct SEND_REQ_DTL
	{
		CString SEQ;
        CString FAX_ID;
        CString STATE_EACH;
        CString FAX_NO;
        CString RECIPIENT_NAME;
        CString TIF_FILE;
        int		TIF_PAGE_CNT;
        CString	PAGES_TO_SEND;
        int		LAST_PAGE_SENT;
        CString TITLE;
        CString RESULT;
        CString REASON;
        int		TRY_CNT;
        CString SMSNO;

		CString	beginTime;
		CString	tifFile_storage;
		CString	tifFile_send;

		void clear()
		{
			SEQ				= "";
			FAX_ID			= "";
			STATE_EACH		= "";
			FAX_NO			= "";
			RECIPIENT_NAME	= "";
			TIF_FILE		= "";
			TIF_PAGE_CNT	= 0;
			PAGES_TO_SEND	= "";
			LAST_PAGE_SENT	= 0;
			TITLE			= "";
			RESULT			= "";
			REASON			= "";
			TRY_CNT			= 0;
			SMSNO			= "";

			beginTime		= "";
			tifFile_storage	= "";
			tifFile_send	= "";
		}
	};

	struct SEND_REQ
	{
		SEND_REQ_MSTR	master;
		SEND_REQ_DTL	detail;

		void clear()
		{
			master.clear();
			detail.clear();
		}
	};

	
		
private:
	CDbModule();
	virtual ~CDbModule();

public:
	int	OccupySendReq(int OccupyReqCnt, int nBroadcastCnt, vector<SEND_REQ>& vecSendReqs, vector<SEND_REQ>& vecSendReqs_broad);
	int	RetrySendReq( SEND_REQ& p_sendReq, int p_fodChannel, int p_result, int p_delayTime );
    int	RetrySendReqEx( SEND_REQ& p_sendReq, int p_fodChannel, int p_result, int p_delayTime );
	int	FinishSendReq( SEND_REQ& p_sendReq, int p_fodChannel, int p_result );
	int	FinishSendReqEx( SEND_REQ& p_sendReq, const char* p_mgwIp, int p_mgwPort, int p_fodChannel, int p_result );
	int	ReadySendReq();
	bool IsSendOffFaxNo( const char* p_faxNo);


	void DbLock();
	void DbUnlock();

	CCriticalSection	 m_dbLock;
};

#endif // !defined(AFX_ORAADODB_H__3DEB4F64_23D1_498F_B9C0_9898597073BD__INCLUDED_)
