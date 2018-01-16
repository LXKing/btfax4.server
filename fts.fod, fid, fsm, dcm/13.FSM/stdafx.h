
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

#include "EasySize.h"


// -------------------------------------------------------------------
// Application Header File (Global ������ extern ���𶧹�)
// -------------------------------------------------------------------
//#include "SIP_FID_DEF.h"
#include "fasip.h"

//#include "FaxChThread.h"
//#include "FASIPThread.h"
#include "Config.h"
#include "AppLog.h"
// --------------------------------------------------------------------------
// BT-SIP
// --------------------------------------------------------------------------
#include "Winperf.h"
#pragma comment(lib, "Ws2_32.lib")
//
//#ifdef _WIN64
//	#ifdef _DEBUG
//		#pragma comment(lib, "Apg_x64_d.lib")
//		#pragma comment(lib, "BTCore1.2_x64_d.lib")
//		#pragma comment(lib, "BTSIP1.2_x64_d.lib")
//	#else
//		#pragma comment(lib, "Apg_x64_r.lib")
//		#pragma comment(lib, "BTCore1.2_x64_r.lib")
//		#pragma comment(lib, "BTSIP1.2_x64_r.lib")
//	#endif
//#else
//	#ifdef _DEBUG
//// aaa		#pragma comment(lib, "Apg_d.lib")
//		#pragma comment(lib, "apg5.1_d.lib")
//// aaa		#pragma comment(lib, "BTCore1.2_d.lib")
//		#pragma comment(lib, "BTCore1.3_d.lib")
//// aaa		#pragma comment(lib, "BTSIP1.2_d.lib")
//		#pragma comment(lib, "BTSIP1.3_d.lib")
//	#else
//		#pragma comment(lib, "Apg_r.lib")
//		#pragma comment(lib, "BTCore1.2_r.lib")
//		#pragma comment(lib, "BTSIP1.2_r.lib")
//	#endif
//#endif

#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "apg5.1_64d.lib")
		#pragma comment(lib, "BTCore1.3_64d.lib")
		#pragma comment(lib, "BTSIP1.3_64d.lib")
	#else
		#pragma comment(lib, "apg5.1_64r.lib")
		#pragma comment(lib, "BTCore1.3_64r.lib")
		#pragma comment(lib, "BTSIP1.3_64r.lib")
	#endif
#else
	#ifdef _DEBUG
// aaa		#pragma comment(lib, "Apg_d.lib")
		#pragma comment(lib, "apg5.1_d.lib")
// aaa		#pragma comment(lib, "BTCore1.2_d.lib")
		#pragma comment(lib, "BTCore1.3_d.lib")
// aaa		#pragma comment(lib, "BTSIP1.2_d.lib")
		#pragma comment(lib, "BTSIP1.3_d.lib")
	#else
		#pragma comment(lib, "apg5.1_r.lib")
		#pragma comment(lib, "BTCore1.3_r.lib")
		#pragma comment(lib, "BTSIP1.3_r.lib")
	#endif
#endif

#include "BTCORE/BTSystem.h"
#include "BTCORE/BTIniFile.h"
#include "BTCORE/BTLogFile.h"
#include "Stack/BTStackGlobal.h"
#include "Stack/BTStackManager.h"
#include "Parser/BTSDPParser.h"
#include "Parser/BTSIPParser.h"
#include "Message/BTSDPMessage.h"
#include "Message/BTSIPMessage.h"
#include "Transport/BTTransportManager.h"
#include "Transaction/BTTransactionManager.h"
#include "Transaction/BTTransaction.h"
#include "Dialog/BTDialogManager.h"
#include "Dialog/BTDialog.h"
#include "Register/BTRegisterManager.h"
#include "Register/BTRegister.h"


#define gPAPP		((CFSMApp*)AfxGetApp())
#define APPLOG		CAppLog::Inst()


// --------------------------------------------------------------------------
// Config
// --------------------------------------------------------------------------
extern CConfig		g_Config;


// -------------------------------------------------------------
// Default INI File ��
// -------------------------------------------------------------
#define CONFIG_INI		""
#define STATUS_INI		"cfg\\Fax_Channel.ini"




#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


