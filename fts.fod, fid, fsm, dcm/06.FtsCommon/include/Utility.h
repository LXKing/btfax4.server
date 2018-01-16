#pragma once

#include <stdlib.h>

/*------ Base64 Encoding Table ------*/
 static const char MimeBase64[] = {
     'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
     'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
     'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
     'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
     'w', 'x', 'y', 'z', '0', '1', '2', '3',
     '4', '5', '6', '7', '8', '9', '+', '/'
 };

/*------ Base64 Decoding Table ------*/
 static int DecodeMimeBase64[256] = {
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
     52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
     -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
     15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
     -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
     41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
     -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
     };


class CUtility
{
public:
	CUtility();
	virtual ~CUtility();

public:
	inline static CString GetCompileTime()
	{
		char	month[8];
		int		mon, date, year, hour, minute, second;
		CString strCompileTime;

		sscanf(__DATE__, "%s%d%d", month, &date, &year);
		sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);

		mon=0;
		if(!strcmp(month, "Jan"))	mon=1;
		else if(!strcmp(month, "Feb"))	mon=2;
		else if(!strcmp(month, "Mar"))	mon=3;
		else if(!strcmp(month, "Apr"))	mon=4;
		else if(!strcmp(month, "May"))	mon=5;
		else if(!strcmp(month, "Jun"))	mon=6;
		else if(!strcmp(month, "Jul"))	mon=7;
		else if(!strcmp(month, "Aug"))	mon=8;
		else if(!strcmp(month, "Sep"))	mon=9;
		else if(!strcmp(month, "Oct"))	mon=10;
		else if(!strcmp(month, "Nov"))	mon=11;
		else if(!strcmp(month, "Dec"))	mon=12;

		strCompileTime.Format( "%4d/%02d/%02d %02d:%02d:%02d", year, mon, date, hour, minute, second );

		return strCompileTime;
	}

	static void ReplaceAll( CString& p_strText, const char* p_szOld, const char* p_szNew );
	static void GetLocalIp( CString* p_pstrIp );
	static void PagesToBase0( CString& p_strPages );
	static bool GetNextPages( int p_nStartPage, const char* p_szPages, CString* p_pstrPagesFrom );
	static long CurrentTimeToLong();
	static bool CurrentTimeToStr(char *str);
	static int	Base64_decode(char *text, char *dst, int numBytes );
	static int	Base64_encode(char *text, int numBytes, char **encodedText);

    static bool DirectoryExists(const char* dirName);
};


#define STRCPY( __szDst, __szSrc ) \
	{ \
		int __nSrcLen__ = strlen( __szSrc ); \
		memset( __szDst, 0x00, sizeof(__szDst) ); \
		if( sizeof(__szDst) > __nSrcLen__) \
			strncpy_s( __szDst, __szSrc, __nSrcLen__ ); \
		else \
			strncpy_s( __szDst, __szSrc, sizeof(__szDst)-1 ); \
	}

