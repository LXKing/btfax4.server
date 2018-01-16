
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


#include "FaxChThread.h"
#include "AppLog.h"
#include "ShmCtrl.h";

#define MAX_CHANNEL	480


struct FAX_CH_INFO
{
	int					chnum;						// 0 Based CH NO. (�ʱⰪ -1)
													// CFaxChThread �⵿ �� �����Ǹ� ������ ����

	int					IO_flag;					// FAX IN/OUT ���� (See enum EN_IO_FLAG Define) (�ʱⰪ 0 : FAX_NONE_CH)
													// CFaxChThread �⵿ �� �����Ǹ� ������ ����

	int					ch_state;					// FAX CH ���� �ڵ尪 (�ʱⰪ -1 : FAX_INIT)
	char				ch_msg[256];				// FAX CH ���� Msg (�ʱⰪ  NULL)


	char				Ani[50];					// �߽Ź�ȣ
	char				Dnis[50];					// ���Ź�ȣ

	char				recvDateTime[20];			// ���� �ð�
	char				srcFile[256];				// ���� FAX File�� (Source)
	char				destFile[256];				// ���� FAX File�� (Destination)
} ;


#define gPAPP			( (CFIDApp*) AfxGetApp() )
#define APPLOG			CAppLog::Inst()

extern CFaxChThread*	g_pFaxChThread[ MAX_CHANNEL ];
extern FAX_CH_INFO		g_FaxChInfo[ MAX_CHANNEL ];
extern CShmCtrl         g_shmCtrl;



