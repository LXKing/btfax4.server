
// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // �Ϻ� CString �����ڴ� ��������� ����˴ϴ�.

// MFC�� ���� �κа� ���� ������ ��� �޽����� ���� ����⸦ �����մϴ�.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC �ٽ� �� ǥ�� ���� ����Դϴ�.
#include <afxext.h>         // MFC Ȯ���Դϴ�.


#include <afxdisp.h>        // MFC �ڵ�ȭ Ŭ�����Դϴ�.


#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC�� ���� �� ��Ʈ�� ���� ����


#include <afxsock.h>            // MFC ���� Ȯ��

#include "AppLog.h"
#include "FaxChThread.h"
#include "ShmCtrl.h"

#define MAX_CHANNEL	480

struct FAX_CH_INFO
{
	int					chnum;							// 0 Based CH NO. (�ʱⰪ -1)
														// CFaxChThread �⵿ �� �����Ǹ� ������ ����

	EN_IO_FLAG			IO_flag;						// FAX IN/OUT ���� (See enum EN_IO_FLAG Define) (�ʱⰪ 0 : FAX_NONE_CH)
														// CFaxChThread �⵿ �� �����Ǹ� ������ ����

	bool				bBroadCast;						// ���� �߼� ����
	int					ch_state;						// FAX CH ���� �ڵ尪 (�ʱⰪ -1 : FAX_INIT)

	// SIP Call Leg : OutBound �Ϸ�� ����
	char				remoteIP[20];					// SDP ���� IP (G/W) (�ʱⰪ NULL)
	int					remotePORT;						// SDP ���� PORT (G/W) (�ʱⰪ 0)

	// ��� ���� : �ѽ� �߼� �Ϸ� �� ����
	int					TryCnt;							// ��õ� ȸ�� (�ʱⰪ 0)
	RESULT				Fax_result_cd;					// �߼� ��� (See enum EN_RESULT Define) (�ʱⰪ 99 : FAX_RESULT_ETC_ERROR)

	HANDLE				sipEvent;						// SIP ó�� Notify Event (�ʱⰪ NULL)
	HANDLE				btfaxEvent;						// FAX ó�� Notify Event

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




