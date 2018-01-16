#pragma once

// iCore..
#include "iLib.h"
#include "iSocket.h"
#include "iThread.h"
#include "iMutex.h"
#include "iType.h"
// iCore

#define CONV_BUF	4028
#define UTF8		"UTF-8"
#define EUC_KR		"EUC-KR"

class CIConvEncoder
{
public:
	CIConvEncoder(void);
	~CIConvEncoder(void);

public :
	static bool ConvertToUtf8(NPSTR strChar, int32 nSrcBuffSize);
	static bool ConvertToEucKr(NPSTR strChar, int32 nSrcBuffSize);
};

