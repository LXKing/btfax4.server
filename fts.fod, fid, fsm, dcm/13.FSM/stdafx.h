
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

#include "EasySize.h"


// -------------------------------------------------------------------
// Application Header File (Global 변수의 extern 선언때문)
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
// Default INI File 명
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


