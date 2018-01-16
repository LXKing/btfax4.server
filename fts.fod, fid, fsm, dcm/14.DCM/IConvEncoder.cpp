#include "StdAfx.h"
#include "IConvEncoder.h"
#include "iconv.h"

CIConvEncoder::CIConvEncoder(void)
{
}


CIConvEncoder::~CIConvEncoder(void)
{
}


bool CIConvEncoder::ConvertToUtf8(NPSTR strChar, int32 nSrcBuffSize)
{	
	if(!strChar || nSrcBuffSize <= 0)
		return false;	

	int32 nSrcLen = strlen(strChar);
	char inBuf[CONV_BUF];
	memset(inBuf, 0, sizeof(inBuf));
	strncpy(inBuf, strChar, sizeof(inBuf));

	char outBuf[CONV_BUF];
	memset(outBuf, 0, sizeof(outBuf));

	size_t sizsSrc = nSrcLen;
	size_t sizeDest = sizeof(outBuf);

	NPCSTR inConv = inBuf;
	NPSTR outConv = outBuf;

	iconv_t cv = iconv_open(UTF8, EUC_KR);	
	if (cv < 0)
		return false; 

	size_t res = iconv(cv, &inConv, &sizsSrc, &outConv, &sizeDest);
	if (res < 0)
	{
		iconv_close(cv);		
		return false;
	}	

	iconv_close(cv);

	memset(strChar, 0x00, nSrcBuffSize);
	strcpy(strChar, outBuf);
	
	return true;
}


bool CIConvEncoder::ConvertToEucKr(NPSTR strChar, int32 nSrcBuffSize)
{	
	if(!strChar || nSrcBuffSize <= 0)
		return false;	

	int32 nSrcLen = strlen(strChar);
	char inBuf[CONV_BUF];
	memset(inBuf, 0, sizeof(inBuf));
	strncpy(inBuf, strChar, sizeof(inBuf));

	char outBuf[CONV_BUF];
	memset(outBuf, 0, sizeof(outBuf));

	size_t sizsSrc = nSrcLen;
	size_t sizeDest = sizeof(outBuf);

	NPCSTR inConv = inBuf;
	NPSTR outConv = outBuf;

	iconv_t cv = iconv_open(EUC_KR, UTF8);	
	if (cv < 0)
		return false; 

	size_t res = iconv(cv, &inConv, &sizsSrc, &outConv, &sizeDest);
	if (res < 0)
	{
		iconv_close(cv);		
		return false;
	}	

	iconv_close(cv);
	
	memset(strChar, 0x00, nSrcBuffSize);
	strcpy(strChar, outBuf);
	return true;
}