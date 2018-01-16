#include "stdafx.h"


CFaxChThread*	g_pFaxChThread[ MAX_CHANNEL ];
FAX_CH_INFO		g_FaxChInfo[ MAX_CHANNEL ];

// ADD - KIMCG : 2013.11.19
CShmCtrl g_shmCtrl;
// ADD - END