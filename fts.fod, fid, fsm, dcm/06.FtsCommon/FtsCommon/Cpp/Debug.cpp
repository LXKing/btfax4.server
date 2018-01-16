// Debug.cpp: implementation of the CDebug class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Debug.h"
#include "PacketDef.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// -------------------------------------------------------------------
// System Header
// -------------------------------------------------------------------
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

// -------------------------------------------------------------------
// Application Header
// -------------------------------------------------------------------
#include "Debug.h"

// -------------------------------------------------------------------
// #define (used in this file)
// -------------------------------------------------------------------
#define MAX_LINE_CHAR_SIZE				151*1024
#define SPACE_STRING	"                                                                                      "

// -------------------------------------------------------------------
// STATIC MEMBER  DEFINITION
// -------------------------------------------------------------------
char	CDebug::m_DBGLVStr[20][5] =
{
	"MJR0",
	"ERR0", "ERR1", "ERR2",
	"WRN0", "WRN1", "WRN2",
	"RPT0", "RPT1", "RPT2",
	"INFO",
	""
};

// -------------------------------------------------------------------
// STATIC FUNCTION DECLARATION
// -------------------------------------------------------------------
static string TrimRight(const string & s);
static string TrimLeft(const string & s);
static string Trim(const string & s);


// -------------------------------------------------------------------
// CLASS  DEFINITION
// -------------------------------------------------------------------
// -------------------------------------------------------------------
// Module 명	: CDebug()
// -------------------------------------------------------------------
// Descriotion	: CDebug Class 생성자
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return 값	: void
// -------------------------------------------------------------------
CDebug::CDebug()
{
	m_LogDir		= "";
	m_ProgramName	= "";

	m_pfnDisplayLog		= NULL;
	m_pfnIncrementError = NULL;

	m_bPrintConsol	= false;							// Console Display : false
	m_bPrintFile	= false;							// File Log : false
	m_DBGLV			= DBGLV_NO_PRT;						// Debug Level : 남기지 않음

	m_StaticKey		= 0;
	m_WriteFd		= NULL;
	m_FileSaveUnit	= LOG_FILE_SAVE_UNIT_DAY;			// 1일 단위로 로그파일 생성

}

// -------------------------------------------------------------------
// Module 명	: ~CDebug()
// -------------------------------------------------------------------
// Descriotion	: CDebug Class 소멸자
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return 값	: void
// -------------------------------------------------------------------
CDebug::~CDebug()
{
	if(m_WriteFd != NULL)
		fclose(m_WriteFd);
}

/*----------------------------------------------------------------------------
# Name   : SetLogDirectory
# Input  :
# Output :
# Return :
# Description:
----------------------------------------------------------------------------*/


void CDebug::SetCallback( PFN_DISPLAY_LOG pfnDisplayLog, PFN_INCREMENT_ERROR p_pfnIncrementError )
{
	m_pfnDisplayLog		=  pfnDisplayLog;
	m_pfnIncrementError = p_pfnIncrementError;
}


// -------------------------------------------------------------------
// Module 명	: bool SetLogDirectory()
// -------------------------------------------------------------------
// Descriotion	: Log Folder 설정
// -------------------------------------------------------------------
// Argument		: char* LogDirectory;		Log Folder
// -------------------------------------------------------------------
// Return 값	: true		Log Folder 설정 성공
//				  false		Log Folder 설정 실패
// -------------------------------------------------------------------
bool CDebug::SetLogDirectory(const char* LogDirectory)
{
	// Log Folder 설정
	m_LogDir = LogDirectory;
	m_LogDir = Trim(m_LogDir);

	if( m_LogDir.length() <= 0 )
		return false;

	// Log Folder 끝에 '/'sk '\\'가 없을 경우 추가
	if( m_LogDir[m_LogDir.length()-1] != '/' && m_LogDir[m_LogDir.length()-1] != '\\' )
		m_LogDir += "/";

	return true;
}

// -------------------------------------------------------------------
// Module 명	: bool SetProgramName()
// -------------------------------------------------------------------
// Descriotion	: 프로그램 명 설정 (Log File Prefix)
// -------------------------------------------------------------------
// Argument		: char* ProgramName;		Log File Prefix
// -------------------------------------------------------------------
// Return 값	: true	Always
// -------------------------------------------------------------------
bool CDebug::SetProgramName(const char* ProgramName)
{
	m_ProgramName = ProgramName;
	return true;
}

// -------------------------------------------------------------------
// Module 명	: bool SetPrintConsole()
// -------------------------------------------------------------------
// Descriotion	: Console로 Display할지 여부 설정
// -------------------------------------------------------------------
// Argument		: bool bPrint;		Console로 Display할지 여부 (tue/false)
// -------------------------------------------------------------------
// Return 값	: bOldPrint		이전 설정값
// -------------------------------------------------------------------
bool CDebug::SetPrintConsole(bool bPrint)
{
	bool	bOldPrint = m_bPrintConsol;

	m_bPrintConsol = bPrint;
	return bOldPrint;
}

// -------------------------------------------------------------------
// Module 명	: bool SetPrintFile()
// -------------------------------------------------------------------
// Descriotion	: File로 Display할지 여부 설정
// -------------------------------------------------------------------
// Argument		: bool bPrint;		File로 Display할지 여부 (tue/false)
// -------------------------------------------------------------------
// Return 값	: bOldPrint		이전 설정값
// -------------------------------------------------------------------
bool CDebug::SetPrintFile(bool bPrint)
{
	bool	bOldPrint = m_bPrintFile;
	
	m_bPrintFile = bPrint;
	return bOldPrint;
}

// -------------------------------------------------------------------
// Module 명	: bool SetDBGLV()
// -------------------------------------------------------------------
// Descriotion	: Debug Level 설정
// -------------------------------------------------------------------
// Argument		: int DBGLV;		Debug Level
// -------------------------------------------------------------------
// Return 값	: true		SUCC
//				  false		FAIL
// -------------------------------------------------------------------
bool CDebug::SetDBGLV(int DBGLV)
{
	if(DBGLV < DBGLV_NO_PRT || DBGLV > DBGLV_10)					// Debug Level이 Invalid할 경우
		return false;
	else
	{
		m_DBGLV = DBGLV;
		return true;
	}
}

// -------------------------------------------------------------------
// Module 명	: bool SetStaticKey()
// -------------------------------------------------------------------
// Descriotion	: Static Key 설정
// -------------------------------------------------------------------
// Argument		: unsigned int key;		static key
// -------------------------------------------------------------------
// Return 값	: true		Always
// -------------------------------------------------------------------
bool CDebug::SetStaticKey(unsigned int Key)
{
	m_StaticKey = Key;
	return true;	
}

// -------------------------------------------------------------------
// Module 명	: bool SetFileSaveUnit()
// -------------------------------------------------------------------
// Descriotion	: Log File 저장 단위 설정
// -------------------------------------------------------------------
// Argument		: EnumLogFileSaveUnit SaveUnit;		Log File 저장 단위
// -------------------------------------------------------------------
// Return 값	: true		Always
// -------------------------------------------------------------------
bool CDebug::SetFileSaveUnit(EnumLogFileSaveUnit SaveUnit)
{
	m_FileSaveUnit = SaveUnit;
	return true;
}

// -------------------------------------------------------------------
// Module 명	: bool Open()
// -------------------------------------------------------------------
// Descriotion	: Log File Open
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return 값	: true		SUCC
//				  false		FAIL
// -------------------------------------------------------------------
bool CDebug::Open()
{
	// P0. Log File이 이미 Open되어 있는지 여부 확인
	if(m_WriteFd != NULL)
		return true;
	
	// P1. 현재 시간
	CTime CurTime = CTime::GetCurrentTime();
	
	char	FileName[512];
	memset(FileName, 0x00, 512);
	
	// P2. Log File 저장단위별 File명 구하기
	switch(m_FileSaveUnit)
	{
	case LOG_FILE_SAVE_UNIT_HOUR:							// 시간별 저장
		sprintf_s( FileName, "%s%s_%04d%02d%02d_%02d.log", 
								m_LogDir.c_str(),
								m_ProgramName.c_str(),
								CurTime.GetYear(),
								CurTime.GetMonth(),
								CurTime.GetDay(),
								CurTime.GetHour() );
		break;

	case LOG_FILE_SAVE_UNIT_DAY:							// 일자별 저장
	default:				
		sprintf_s( FileName, "%s%s_%04d%02d%02d.log",
								m_LogDir.c_str(),
								m_ProgramName.c_str(),
								CurTime.GetYear(),
								CurTime.GetMonth(),
								CurTime.GetDay() );
		break;
	}

	// P3. Log File Open
	/*errno_t err = fopen_s( &m_WriteFd, FileName, "a+" );
	if( err != 0 )
		return false;*/

	m_WriteFd = fopen( FileName, "a+" );
	if( m_WriteFd == NULL )
		return false;

	m_FileOpenDate = CurTime;
	return true;
}

// -------------------------------------------------------------------
// Module 명	: bool Close()
// -------------------------------------------------------------------
// Descriotion	: Log File Close
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return 값	: true		Always
// -------------------------------------------------------------------
bool CDebug::Close( void )
{
	if(m_WriteFd != NULL)
	{
		fclose(m_WriteFd);
		m_WriteFd = NULL;
	}			
	return true;
}

// -------------------------------------------------------------------
// Module 명	: bool Print()
// -------------------------------------------------------------------
// Descriotion	: Display여부나 Debug Level에 따라 Log를 남긴다.
// -------------------------------------------------------------------
// Argument		: int DebugLevel;		Debug Level
//				  char* Format;			Log String Format
//				  ...;					Log String Argument
// -------------------------------------------------------------------
// Return 값	: true		Always
// -------------------------------------------------------------------
bool CDebug::Print(int DebugLevel, char* Format, ...)
{
	
	if( DebugLevel <= DBGLV_ERR2 )
	{
		if( m_pfnIncrementError )
			m_pfnIncrementError();
	}
	// P0. Console 또는 File로 Log를 쌓지 않도록 설정되어 있으면
	if(!m_bPrintConsol && !m_bPrintFile)
		return true;

	// P1. Debug Level이 현재 Debug Level보다 크거나 Invalid한 경우 Log를 쌓지 않는다.
	if( DebugLevel > m_DBGLV || DebugLevel < DBGLV_NO_PRT)
		return true;

	// P2. 현재 시간
	CTime	CurTime = CTime::GetCurrentTime();

	char	Buff[MAX_LINE_CHAR_SIZE];	
	char	TimeBuff[64];
	va_list	Args;

	memset(Buff, 0x00, MAX_LINE_CHAR_SIZE);
	memset(TimeBuff, 0x00, 64);

	// P3. Log String 만들기
	va_start( Args, Format );
	vsprintf_s( Buff, Format, Args );
	va_end( Args );

	// P4. Lock
	m_Sync.Lock();	

	// P5. Console Log
	if(m_bPrintConsol)
	{
		sprintf_s( TimeBuff, "%02d:%02d:%02d", CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond() );
		if(DebugLevel > DBGLV_NO_PRT && DebugLevel <= DBGLV_RPT)
		{
			if( m_pfnDisplayLog )
			{
				m_strDisplayLog.Format( "[%s][%s] : %s", m_DBGLVStr[DebugLevel], TimeBuff, Buff );
				m_pfnDisplayLog( m_strDisplayLog );
			}
		}
	}

	// P6. File Log
	if(m_bPrintFile)
	{
		FILE*	Fw;

		// p6-1. Log File이 Open되어 있지 않은 상태면 Open (Log File 생성 단위에 따라)
		if(m_WriteFd == NULL)
		{
			char	FileName[512];
			memset(FileName, 0x00, 512);

			switch(m_FileSaveUnit)
			{
			case LOG_FILE_SAVE_UNIT_HOUR:
				sprintf_s( FileName, "%s%s_%04d%02d%02d_%02d.log", 
								m_LogDir.c_str(),
								m_ProgramName.c_str(),
								CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour() );
				break;

			case LOG_FILE_SAVE_UNIT_DAY:
			default:				
				sprintf_s( FileName, "%s%s_%04d%02d%02d.log",
								m_LogDir.c_str(),
								m_ProgramName.c_str(),
								CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay() );
				break;
			}

			// -------------------------------------------------------------------
			// Update By SSY (2011.03.19)
			// -------------------------------------------------------------------
			// Fw = fopen(FileName, "a+");
			// -------------------------------------------------------------------
			// 한번도 Log File이 Open된 적이 없을 경우에 위와 같은 방법을 사용하면
			//		아래의 P7을 타게되므로 계속해서 Log File을 Open/Close하게 된다.
			// -------------------------------------------------------------------
			Open();
			Fw = m_WriteFd;
		}
		// p6-2. Log File이 Open되어 있는 상태면 시간대 또는 일자가 바뀌었으면 재 Open
		else
		{
			switch(m_FileSaveUnit)
			{
			case LOG_FILE_SAVE_UNIT_HOUR:
				if( m_FileOpenDate.GetHour() != CurTime.GetHour())
				{
					Close();
					Open();
				}
				break;

			case LOG_FILE_SAVE_UNIT_DAY:
			default:
				if(m_FileOpenDate.GetDay() != CurTime.GetDay())
				{
					Close();
					Open();
				}
				break;
			}
			Fw = m_WriteFd;
		}

		// P7. Log File 쌓기
		if(Fw != NULL)
		{
			sprintf_s( TimeBuff, "%04d-%02d-%02d %02d:%02d:%02d", 
							CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(),
							CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond() );
			fprintf_s( Fw, "[%s][%s] : %s\n", m_DBGLVStr[DebugLevel], TimeBuff, Buff );			
			fflush(Fw);
		}

		// P8. Log File Open을 Local에서만 한 경우 m_FileOpenDate이 설정되지 않으므로 close한다.
		if(m_WriteFd == NULL && Fw != NULL)
			fclose(Fw);
	}

	// P9. UnLock
	m_Sync.Unlock();

	return true;
}

// -------------------------------------------------------------------
// Module 명	: bool Print()
// -------------------------------------------------------------------
// Descriotion	: Display여부나 Debug Level에 따라 Log를 남긴다. (Key 값은 Argument 처리)
// -------------------------------------------------------------------
// Argument		: int DebugLevel;		Debug Level
//				  unsigned int Key;		static Key
//				  char* Format;			Log String Format
//				  ...;					Log String Argument
// -------------------------------------------------------------------
// Return 값	: true		Always
// -------------------------------------------------------------------
bool CDebug::Print(int DebugLevel, unsigned int Key, char* Format, ... )
{
	// P0. Console 또는 File로 Log를 쌓지 않도록 설정되어 있으면
	if( !m_bPrintConsol && !m_bPrintFile )
		return true;

	// P1. Debug Level이 현재 Debug Level보다 크거나 Invalid한 경우 Log를 쌓지 않는다.
	if( DebugLevel > m_DBGLV || DebugLevel < DBGLV_NO_PRT )
		return true;

	// P2. 현재 시간
	CTime	CurTime = CTime::GetCurrentTime();

	char	Buff[MAX_LINE_CHAR_SIZE];
	char	FileBuff[MAX_LINE_CHAR_SIZE];	
	char	TimeBuff[64];
	va_list	Args;

	memset( Buff, 0x00, MAX_LINE_CHAR_SIZE );
	memset( FileBuff, 0x00, MAX_LINE_CHAR_SIZE );
	memset( TimeBuff, 0x00, 64 );

	// P3. Log String 만들기
	va_start( Args, Format );
	vsprintf_s( Buff, Format, Args );
	va_end( Args );	
	
	// P4. Lock
	m_Sync.Lock();
	
	// P5. Console Log
	if(m_bPrintConsol)
	{
		sprintf_s( TimeBuff, "%02d:%02d:%02d", CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond() );
		printf_s( "[%02d][%010u][%s] : %s\n", DebugLevel, Key, TimeBuff, Buff );		
	}

	// P6. File Log
	if(m_bPrintFile)
	{
		FILE*	Fw;

		// p6-1. Log File이 Open되어 있지 않은 상태면 Open (Log File 생성 단위에 따라)
		if(m_WriteFd == NULL)
		{
			char	FileName[512];
			memset( FileName, 0x00, 512 );

			switch( m_FileSaveUnit )
			{
			case LOG_FILE_SAVE_UNIT_HOUR:
				sprintf_s( FileName, "%s%s_%04d%02d%02d_%02d.log", 
							m_LogDir.c_str(), m_ProgramName.c_str(),
							CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour() );
				break;

			case LOG_FILE_SAVE_UNIT_DAY:
			default:
				sprintf_s( FileName, "%s%s_%04d%02d%02d.log",
							m_LogDir.c_str(), m_ProgramName.c_str(),
							CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay() );
				break;
			}

			// -------------------------------------------------------------------
			// Update By SSY (2011.03.19)
			// -------------------------------------------------------------------
			// Fw = fopen(FileName, "a+");
			// -------------------------------------------------------------------
			// 한번도 Log File이 Open된 적이 없을 경우에 위와 같은 방법을 사용하면
			//		아래의 P7을 타게되므로 계속해서 Log File을 Open/Close하게 된다.
			// -------------------------------------------------------------------
			Open();
			Fw = m_WriteFd;
		}
		// p6-2. Log File이 Open되어 있는 상태면 시간대 또는 일자가 바뀌었으면 재 Open
		else
		{
			switch( m_FileSaveUnit )
			{
			case LOG_FILE_SAVE_UNIT_HOUR:
				if( m_FileOpenDate.GetHour() != CurTime.GetHour() )
				{
					Close();
					Open();
				}
				break;

			case LOG_FILE_SAVE_UNIT_DAY:
			default:
				if( m_FileOpenDate.GetDay() != CurTime.GetDay() )
				{
					Close();
					Open();
				}
				break;
			}

			Fw = m_WriteFd;
		}
		
		// P7. Log File 쌓기
		if(Fw != NULL)
		{
			sprintf_s( TimeBuff, "%04d-%02d-%02d %02d:%02d:%02d", 
								CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(),
								CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond() );									
			fprintf_s( Fw, "[%02d][%010u][%s][%d] : %s\n", DebugLevel, Key, TimeBuff, errno, Buff );			
			fflush(Fw);
		}
		
		// P8. Log File Open을 Local에서만 한 경우 m_FileOpenDate이 설정되지 않으므로 close한다.
		if( m_WriteFd == NULL && Fw != NULL )
			fclose(Fw);
	}
	
	// P9. UnLock
	m_Sync.Unlock();	

	return true;
}

void CDebug::LogSip( char p_chType, int p_chan, int p_msgid, int p_nLen, int p_result )
{
	if( p_result == -99999999 )
	{
		LogSip( p_chType, p_chan, p_msgid, p_nLen, "" );
	}
	else
	{
		CString strMsg;
		strMsg.Format( "Result[%d]", p_result );

		LogSip( p_chType, p_chan, p_msgid, p_nLen, strMsg );
	}
}

void CDebug::LogSip( char p_chType, int p_chan, int p_msgid, int p_nLen, const char* p_szMsg )
{
	int		logLevel;
	CString	strChannel;

	if( p_nLen > 0 )	logLevel = DBGLV_RPT;
	else				logLevel = DBGLV_ERR;

	if( p_chan >= 0 )
		strChannel.Format( "CHAN_%03d", p_chan );
	else
		strChannel = "        ";
	
	if( p_chType == 'S' )
	{
		Print( logLevel, "[%s][SIP][%c:%03d] %s. PacketLen[%03d]. %s", 
				(LPCSTR)strChannel, p_chType, p_nLen, MsgName_FromClient(p_msgid), MsgLen_FromClient(p_msgid), p_szMsg );
	}
	else
	{
		Print( logLevel, "[%s][SIP][%c:%03d] %s. PacketLen[%03d]. %s", 
				(LPCSTR)strChannel, p_chType, p_nLen, MsgName_FromFsm(p_msgid), MsgLen_FromFsm(p_msgid), p_szMsg );
	}
}

void CDebug::LogSip( int p_logLevel, const char* p_szMsg )
{
	Print( p_logLevel, "[        ][SIP][     ] %s", p_szMsg );
}

// -------------------------------------------------------------------
// Module 명	: void PrintHexaData()
// -------------------------------------------------------------------
// Descriotion	: 
// -------------------------------------------------------------------
// Argument		: void* pData;			Log로 남길 전문 Data
//				  int DataSize;			Data 길이
//				  string &PrintData;	OFFSET, HEXA, ASCII로 표현된 전문 Data	
// -------------------------------------------------------------------
// Return 값	: true		Always
// -------------------------------------------------------------------
void CDebug::PrintHexaData(void* pData, int DataSize, string &PrintData )
{

	#define OUT_DATA_MAX_SIZE	512*1024
	
	char			TempBuff[256];
	char			OutData[OUT_DATA_MAX_SIZE];
	char			AsciiStr[256];	
	int				Offset =0;
		
	unsigned char*	pMsg = (unsigned char*)pData;

	try
	{

		memset( TempBuff, 0x00, 256 );
		memset( OutData, 0x00, OUT_DATA_MAX_SIZE );

		// P0. HEXA로 남길 Log의 Header를 만든다.
		sprintf_s( TempBuff , "[OFFSET]                           [HEXA]                           [ASCII]     \n" );
		strcat_s( OutData, TempBuff );
		sprintf_s( TempBuff , "========   =================================================   =================\n" );
		strcat_s( OutData, TempBuff );
		//                   0000000a   0A 0A 0A 0A 0A 0A 0A 0A | 0B 0B 0B 0B 0B 0B 0B 0B   
				
		memset( AsciiStr, 0x00, 256 );
		memset( TempBuff, 0x00, 256 );
		
		// P1. HEXA 및 ASCII로 남길 Log String을 만든다.
		for(int i=0; i<DataSize; i++)
		{
			// P1-1. 16Bytes 마다 OFFSET 값을 Hexa로 찍는다.
			if(i%16 == 0)
			{
				sprintf_s( TempBuff, "%08X : ", i );
				strcat_s( OutData, TempBuff );
			}
	
			// P1-2. 전문 Data를 1Bytes씩 Hexa로 저장한다.
			sprintf_s( TempBuff, "%02X ", pMsg[i] );		
			strcat_s( OutData, TempBuff );
	

			// P1-3. ASCII 문자가 아닌 경우 Blank로
			if( pMsg[i] > 128 || pMsg[i] < 32 )
				sprintf_s( TempBuff, " ");
			else
				sprintf_s( TempBuff, "%c", pMsg[i] );
			strcat_s( AsciiStr, TempBuff );

			// P1-4.8Bytes 마다 구분자를 준다
			if(i%8 == 7)
			{
				strcat_s( OutData, "| " );
				strcat_s( AsciiStr, " " );
			}
	
			// P1-5.16Bytes 마다 Ascii String을 붙이고 Line을 바꿔준다.
			if(i%16 == 15)
			{
				strcat_s( OutData, AsciiStr );
				strcat_s( OutData, "\n" );								
				memset( AsciiStr, 0x00, 256 );
			}
		}
	
		// P2. 전문 끝의 Data가 16Bytes가 아닌 경우 찍히지 않는 뒷부분을 Blank로 채운다
		int	Cnt = DataSize % 16;
		if(Cnt < 15)
		{
			memset( TempBuff, 0x00, 64 );
			// P2-1. 8Bytes 이하인 경우 : 
			if(Cnt <= 7)
				memcpy( TempBuff, SPACE_STRING, (16-Cnt)*3 + 4 );	// 4 = "| " + Hexa와 Ascii 사이 "  "
			// P2-1. 8Bytes 이상인 경우 : 
			else
				memcpy( TempBuff, SPACE_STRING, (16-Cnt)*3 + 2 );	// 2 = Hexa와 Ascii 사이 "  "

			strcat_s( OutData, TempBuff );
			strcat_s( OutData, AsciiStr );
			strcat_s( OutData, "\n" );			
		}
	
		sprintf_s( TempBuff , "================================================================================\n" );
		strcat_s( OutData, TempBuff );
	}
	catch(exception e)
	{
		// PrintData = "";
	}
	
	PrintData = OutData;
}

// -------------------------------------------------------------------
// Module 명	: void TrimRight()
// -------------------------------------------------------------------
// Descriotion	: 오른쪽 Trim
// -------------------------------------------------------------------
// Argument		: const string& s;		오른쪽 Trim할 string
// -------------------------------------------------------------------
// Return 값	: string		Trim 결과
// -------------------------------------------------------------------
static string TrimRight(const string& s)
{
	unsigned int	e;	

	if (s.length() == 0)
		return s;

	e = s.find_last_not_of(" \t\r\n");

	if (e == string::npos)
		return "";

	return string(s, 0, e+1);
}

// -------------------------------------------------------------------
// Module 명	: void TrimLeft()
// -------------------------------------------------------------------
// Descriotion	: 왼쪽 Trim
// -------------------------------------------------------------------
// Argument		: const string& s;		왼쪽 Trim할 string
// -------------------------------------------------------------------
// Return 값	: string		Trim 결과
// -------------------------------------------------------------------
static string TrimLeft(const string& s)
{
	unsigned int	f;

	if (s.length() == 0)
		return s;

	f = s.find_first_not_of(" \t\r\n");

	if (f == string::npos)
		return "";

	return string(s, f, s.length()-f);
}

// -------------------------------------------------------------------
// Module 명	: void Trim()
// -------------------------------------------------------------------
// Descriotion	: 왼쪽 && 오른쪽 Trim
// -------------------------------------------------------------------
// Argument		: const string& s;		왼쪽 && 오른쪽 Trim할 string
// -------------------------------------------------------------------
// Return 값	: string		Trim 결과
// -------------------------------------------------------------------
static string Trim(const string& s)
{
	return TrimLeft(TrimRight(s));
}

