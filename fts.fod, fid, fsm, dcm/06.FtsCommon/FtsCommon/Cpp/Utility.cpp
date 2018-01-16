#include "StdAfx.h"
#include <windows.h>
#include "Utility.h"
#include <string>
#include <time.h>
#include <vector>

using namespace std;


CUtility::CUtility()
{
}

CUtility::~CUtility()
{
}

void CUtility::ReplaceAll( CString& p_strText, const char* p_szOld, const char* p_szNew )
{
	int cnt;

	do
	{
		cnt = p_strText.Replace( p_szOld, p_szNew );
	}
	while( cnt > 0 );
}

void CUtility::PagesToBase0( CString& p_strPages )
{
	int				begin, find;
	string			strTemp;
	CString			strNumber;
	vector<string>	tokens;

	ReplaceAll( p_strPages, " ", "" );
	ReplaceAll( p_strPages, "all", "1-" );
	ReplaceAll( p_strPages, "--", "-" );
	ReplaceAll( p_strPages, ",,", "," );
	strTemp = p_strPages;

	begin = 0;
	while( true )
	{
		find = strTemp.find_first_of( ",-", begin );
		if( find < 0 )
		{
			if( begin <= (int)strTemp.size() - 1 )
				tokens.push_back( strTemp.substr( begin ) );		// 마지막 숫자
			break;
		}
		
		if( find > begin )
			tokens.push_back( strTemp.substr( begin, find - begin ) ); // 숫자

		tokens.push_back( strTemp.substr( find, 1 ) );			// -,
		begin = find + 1;
	}

	p_strPages = "";
	for( int i = 0 ; i < (int)tokens.size() ; ++i )
	{
		if( tokens[i] == "-" || tokens[i] == "," ) {
			p_strPages += tokens[i].c_str();
			continue;
		}

		strNumber.Format( "%d", atoi(tokens[i].c_str()) - 1 );
		p_strPages += strNumber;
	}
}

void CUtility::GetLocalIp( CString* p_pstrIp )
{
	struct hostent	* host;
	char	hostname[128];
	char	local_ip[50];

	hostname[0]=0;
	gethostname(hostname, sizeof(hostname));
	host=gethostbyname(hostname);
	if(host){
		sprintf(local_ip, "%d.%d.%d.%d",
			((unsigned char*)(**(host->h_addr_list)))	,
			((unsigned char*)(*(*(host->h_addr_list)+1))),
			((unsigned char*)(*(*(host->h_addr_list)+2))),
			((unsigned char*)(*(*(host->h_addr_list)+3))));
	}

	if(!strcmp(local_ip, "127.0.0.1"))
		local_ip[0]=0;
}

bool CUtility::GetNextPages( int p_nStartPage, const char* p_szPages, CString* p_pstrPagesFrom )
{
	int		nPos, nSubPos, nSubFrom, nSubTo;
	CString strPages, strToken;

	strPages = p_szPages;
	nPos = 0;
	strToken = strPages.Tokenize( ",", nPos );

	while( strToken != "" )
	{
		nSubPos = strToken.Find( "-" );
		if( nSubPos < 0 )
		{
			if( nSubPos >= p_nStartPage )
			{
				*p_pstrPagesFrom = strPages.Mid( nPos - strToken.GetLength() - 1 );
				return true;
			}
		}
		else if( nSubPos == strToken.GetLength() - 1 )
		{
			p_pstrPagesFrom->Format( "%d-", p_nStartPage );
			return true;
		}
		else
		{
			nSubFrom	= atoi( strToken.Mid( 0, nSubPos ) );
			nSubTo		= atoi( strToken.Mid( nSubPos + 1 ) );

			if( nSubFrom <= p_nStartPage && p_nStartPage <= nSubTo )
			{
				if( nPos < strPages.GetLength() )
					p_pstrPagesFrom->Format( "%d-%d,%s", p_nStartPage, nSubTo, strPages.Mid(nPos) );
				else
					p_pstrPagesFrom->Format( "%d-%d", p_nStartPage, nSubTo );
				return true;
			}
		}

		strToken = strPages.Tokenize( "-", nPos );
	}

	return false;
}

long CUtility::CurrentTimeToLong()
{	
	time_t currTime; 
	long lTime; 
	currTime = time(0); 
	lTime = static_cast<long> (currTime); 
	return lTime;
}

bool CUtility::CurrentTimeToStr(char *str)
{
	time_t	tTime;
	struct tm* _tm;

	time( &tTime );
	_tm = localtime(&tTime);

	sprintf( str,	"%04d%02d%02d%02d%02d%02d",
					_tm->tm_year+1900, _tm->tm_mon+1, _tm->tm_mday,
					_tm->tm_hour, _tm->tm_min, _tm->tm_sec );

	return true;
}

int CUtility::Base64_decode(char *text, char *dst, int numBytes )
 {
   const char* cp;
   int space_idx = 0, phase;
   int d, prev_d = 0;
   unsigned char c;

    space_idx = 0;
     phase = 0;

    for ( cp = text; *cp != '\0'; ++cp ) {
         d = DecodeMimeBase64[(int) *cp];
         if ( d != -1 ) {
             switch ( phase ) {
                 case 0:
                     ++phase;
                     break;
                 case 1:
                     c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
                     if ( space_idx < numBytes )
                         dst[space_idx++] = c;
                     ++phase;
                     break;
                 case 2:
                     c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
                     if ( space_idx < numBytes )
                         dst[space_idx++] = c;
                     ++phase;
                     break;
                 case 3:
                     c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
                     if ( space_idx < numBytes )
                         dst[space_idx++] = c;
                     phase = 0;
                     break;
             }
             prev_d = d;
         }
     }

    return space_idx;

}

int CUtility::Base64_encode(char *text, int numBytes, char **encodedText)
 {
   unsigned char input[3]  = {0,0,0};
   unsigned char output[4] = {0,0,0,0};
   int   index, i, j, size;
   char *p, *plen;

  plen           = text + numBytes - 1;
   size           = (4 * (numBytes / 3)) + (numBytes % 3? 4 : 0) + 1;
   (*encodedText) = (char*)malloc(size);
   j              = 0;

    for  (i = 0, p = text;p <= plen; i++, p++) {
         index = i % 3;
         input[index] = *p;

        if (index == 2 || p == plen) {
             output[0] = ((input[0] & 0xFC) >> 2);
             output[1] = ((input[0] & 0x3) << 4) | ((input[1] & 0xF0) >> 4);
             output[2] = ((input[1] & 0xF) << 2) | ((input[2] & 0xC0) >> 6);
             output[3] = (input[2] & 0x3F);

            (*encodedText)[j++] = MimeBase64[output[0]];
             (*encodedText)[j++] = MimeBase64[output[1]];
             (*encodedText)[j++] = index == 0? '=' : MimeBase64[output[2]];
             (*encodedText)[j++] = index <  2? '=' : MimeBase64[output[3]];

            input[0] = input[1] = input[2] = 0;
         }
     }

    (*encodedText)[j] = '\0';

    return size;
 }


bool CUtility::DirectoryExists(const char* dirName) 
{
  DWORD attribs = ::GetFileAttributesA(dirName);
  if (attribs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}