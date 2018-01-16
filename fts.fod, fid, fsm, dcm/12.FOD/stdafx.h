
// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 일부 CString 생성자는 명시적으로 선언됩니다.

// MFC의 공통 부분과 무시 가능한 경고 메시지에 대한 숨기기를 해제합니다.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.


#include <afxdisp.h>        // MFC 자동화 클래스입니다.


#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC의 리본 및 컨트롤 막대 지원


#include <afxsock.h>            // MFC 소켓 확장

#include "AppLog.h"
#include "FaxChThread.h"
#include "ShmCtrl.h"

#define MAX_CHANNEL	480

struct FAX_CH_INFO
{
	int					chnum;							// 0 Based CH NO. (초기값 -1)
														// CFaxChThread 기동 후 설정되면 변하지 않음

	EN_IO_FLAG			IO_flag;						// FAX IN/OUT 구분 (See enum EN_IO_FLAG Define) (초기값 0 : FAX_NONE_CH)
														// CFaxChThread 기동 후 설정되면 변하지 않음

	bool				bBroadCast;						// 동보 발송 여부
	int					ch_state;						// FAX CH 상태 코드값 (초기값 -1 : FAX_INIT)

	// SIP Call Leg : OutBound 완료시 설정
	char				remoteIP[20];					// SDP 연동 IP (G/W) (초기값 NULL)
	int					remotePORT;						// SDP 연동 PORT (G/W) (초기값 0)

	// 결과 정보 : 팩스 발송 완료 후 설정
	int					TryCnt;							// 재시도 회수 (초기값 0)
	RESULT				Fax_result_cd;					// 발송 결과 (See enum EN_RESULT Define) (초기값 99 : FAX_RESULT_ETC_ERROR)

	HANDLE				sipEvent;						// SIP 처리 Notify Event (초기값 NULL)
	HANDLE				btfaxEvent;						// FAX 처리 Notify Event

	CDbModule::SEND_REQ	sendReq;
	
	void clear()
	{
		chnum		= 0;
		IO_flag		= IO_NONE;
		bBroadCast	= false;
		ch_state	= FAX_INIT;

		idle();
	}
	void idle()
	{
		memset(remoteIP, 0x00, sizeof(remoteIP));
		remotePORT		= 0;

		TryCnt			= 0;
		Fax_result_cd	= EMPTY;

		sipEvent		= NULL;
		btfaxEvent		= NULL;

		sendReq.clear();
	}

	void reqSend( CDbModule::SEND_REQ& p_sendReq )
	{
		sendReq  = p_sendReq;
		ch_state = FAX_OCCUPY;
	}
};



#define gPAPP			( (CFODApp*) AfxGetApp() )
#define APPLOG			CAppLog::Inst()

extern CFaxChThread*	g_pFaxChThread[ MAX_CHANNEL ];
extern FAX_CH_INFO		g_FaxChInfo[ MAX_CHANNEL ];
extern CShmCtrl         g_shmCtrl;




