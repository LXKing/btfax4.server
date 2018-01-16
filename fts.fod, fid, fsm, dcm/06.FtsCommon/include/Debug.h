// Debug.h: interface for the CDebug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUG_H__67640E23_A24E_4A76_ADBC_41340D9CFA3A__INCLUDED_)
#define AFX_DEBUG_H__67640E23_A24E_4A76_ADBC_41340D9CFA3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// -------------------------------------------------------------------
// System Header
// -------------------------------------------------------------------
#include <string>
using namespace std;

#include "afxmt.h"

	
// -------------------------------------------------------------------
// #define
// -------------------------------------------------------------------
#define	DBGLV_NO_PRT	-1
#define	DBGLV_0			0			// HIGH
#define	DBGLV_1			1
#define	DBGLV_2			2
#define	DBGLV_3			3
#define	DBGLV_4			4
#define	DBGLV_5			5
#define	DBGLV_6			6
#define	DBGLV_7			7
#define	DBGLV_8			8
#define	DBGLV_9			9			// LOW
#define	DBGLV_10		10
#define	DBGLV_ALL_PRT	DBGLV_10

#define	DBGLV_MJR0		DBGLV_0		// HIGH
#define	DBGLV_ERR0		DBGLV_1
#define	DBGLV_ERR1		DBGLV_2
#define	DBGLV_ERR2		DBGLV_3
#define	DBGLV_WRN0		DBGLV_4
#define	DBGLV_WRN1		DBGLV_5
#define	DBGLV_WRN2		DBGLV_6
#define	DBGLV_RPT0		DBGLV_7
#define	DBGLV_RPT1		DBGLV_8
#define	DBGLV_RPT2		DBGLV_9		// LOW
#define	DBGLV_INF0		DBGLV_10

#define	DBGLV_INF		DBGLV_INF0
#define	DBGLV_RPT		DBGLV_RPT0
#define	DBGLV_WRN		DBGLV_WRN0
#define	DBGLV_ERR		DBGLV_ERR0
#define	DBGLV_MAJOR		DBGLV_MJR0

// -------------------------------------------------------------------
// typedef
// -------------------------------------------------------------------
typedef enum
{
	LOG_FILE_SAVE_UNIT_DAY = 1,	
	LOG_FILE_SAVE_UNIT_HOUR = 2
} EnumLogFileSaveUnit;


// -------------------------------------------------------------------
// CLASS DECLARATION
// -------------------------------------------------------------------
class CDebug
{
	// TYPE
public:
	typedef void (*PFN_DISPLAY_LOG)( const char* p_szLog );
	typedef void (*PFN_INCREMENT_ERROR)();

// PUBLIC MEMBER
public:
    // Constructor & Destructor
    CDebug();
    virtual ~CDebug();

    // Variable Declaration
	unsigned int	m_StaticKey;
	FILE*			m_WriteFd;
	
    // Method Declaration
	void SetCallback( PFN_DISPLAY_LOG pfnDisplayLog, PFN_INCREMENT_ERROR p_pfnIncrementError );
	bool SetLogDirectory( const char* LogDirectory );
	bool SetProgramName( const char* ProgramName );
	bool SetPrintConsole( bool bPrint = false );
	bool SetPrintFile( bool bPrint = false );
	bool SetStaticKey( unsigned int Key );

	// -1~9, m_DBGLV 보다 적거나 같은 Level만  Print된다. -1이면 전부 No Print
	bool SetDBGLV( int DBGLV );
	bool SetFileSaveUnit( EnumLogFileSaveUnit SaveUnit );
	bool Print( int DebugLevel, char* Format, ...  );
	bool Print( int DebugLevel, unsigned int Key, char* Format, ... );

	bool Open(void);
	bool Close(void);

	void LogSip( char p_chType, int p_chan, int p_msgid, int p_nLen, int p_result = -99999999 );
	void LogSip( char p_chType, int p_chan, int p_msgid, int p_nLen, const char* p_szMsg );
	void LogSip( int p_logLevel, const char* p_szMsg );

	static void PrintHexaData( void* pData, int DataSize, string &PrintData );

    // Virtual Method Declaration

    // Static Variable Declaration

    // Static Method Declaration

    // Operator Redefinition

// PRIVATE MEMBER
private:
    // Variable Declaration
	string		m_LogDir;
	string		m_ProgramName;
	bool		m_bPrintConsol;
	bool	    m_bPrintFile;
	char		m_DBGLV;
	CTime		m_FileOpenDate;
	EnumLogFileSaveUnit	m_FileSaveUnit;
	PFN_DISPLAY_LOG		m_pfnDisplayLog;
	PFN_INCREMENT_ERROR m_pfnIncrementError;

	CCriticalSection m_Sync;
	CString		m_strDisplayLog;
	
	static char	m_DBGLVStr[20][5];

    // Method Declaration

// PROTECTED MEMBER
protected:
    // Variable Declaration
};

#endif // !defined(AFX_DEBUG_H__67640E23_A24E_4A76_ADBC_41340D9CFA3A__INCLUDED_)
