// OraThread.cpp : implementation file
//

#include "stdafx.h"
#include "APP.h"
#include "DbThread.h"
#include "FaxChThread.h"
#include "FsmIfThread.h"
#include "Config.h"
#include "enum.h"
#include "TabInfo.h"

using namespace COMMON_LIB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDbPollingThread


CDbPollingThread* CDbPollingThread::s_pInstance = NULL;

CDbPollingThread* CDbPollingThread::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new CDbPollingThread;

	return s_pInstance;
}


CDbPollingThread::CDbPollingThread()
{
	CoInitialize(NULL);
}

CDbPollingThread::~CDbPollingThread()
{
	CoUninitialize();
}


/////////////////////////////////////////////////////////////////////////////

void CDbPollingThread::onThreadEntry()
{
	int				dbRet;									// DB 접속 결과
	
	CString			connStr, id, pwd;
	
	
	int				i;
	bool			bGeneral;
	list<int>		vecChIdxes, vecChIdxes_broad;
	vector<CDbModule::SEND_REQ> sendReqs;					// 발송 요청 정보 - 일반건
	vector<CDbModule::SEND_REQ> sendReqs_broad;				// 발송 요청 정보 - 동보건
	
	// P1-2. 기 발송 진행중이던 건, 대기상태로 초기화
	CDbModule::Inst()->DbLock();
	CDbModule::Inst()->ReadySendReq();
	CDbModule::Inst()->DbUnlock();

	bGeneral = false;
	while( !IsReqStop() )
	{
		Sleep( CConfig::DB_POLLING_SLEEP );
		
		if( !CFsmIfThread::Inst()->IsRegistered() )
			continue;

		// -------------------------------------------------------------
		// P2. IDLE 채널 검색
		// -------------------------------------------------------------
		bGeneral = !bGeneral;
		if( bGeneral )
		{
			Get_Idle_ChIdx( false, CConfig::DB_FETCH_CNT, vecChIdxes );

			if( vecChIdxes.size() < CConfig::DB_FETCH_CNT )
				Get_Idle_ChIdx( true, CConfig::DB_FETCH_CNT - vecChIdxes.size(), vecChIdxes_broad );
		}
		else
		{
			Get_Idle_ChIdx(true, CConfig::DB_FETCH_CNT, vecChIdxes_broad);

			if( vecChIdxes_broad.size() < CConfig::DB_FETCH_CNT )
				Get_Idle_ChIdx( false, CConfig::DB_FETCH_CNT - vecChIdxes_broad.size(), vecChIdxes );
		}

		if( vecChIdxes.size() + vecChIdxes_broad.size() <= 0 )
			continue;
		
		CTabInfo::DBPolling();
		
		// -------------------------------------------------------------
		// P3. 발송 요청건 GET
		// -------------------------------------------------------------
		CDbModule::Inst()->DbLock();
		dbRet = CDbModule::Inst()->OccupySendReq( vecChIdxes.size() + vecChIdxes_broad.size(),
													vecChIdxes_broad.size(), 
													sendReqs, 
													sendReqs_broad);
		CDbModule::Inst()->DbUnlock();

		APPLOG->Print(DBGLV_RPT, "[SUCC] DbPolling Check..");

		if(dbRet < 0 ) 
		{
			APPLOG->Print(DBGLV_ERR, "[FAIL] P_OCCUPY_SEND_REQS_FOD! dbRet[%d]", dbRet);
			continue;
		}

		if( sendReqs.size() + sendReqs_broad.size() <= 0 )
			continue;
		APPLOG->Print(DBGLV_INF, "[SUCC] P_OCCUPY_SEND_REQS_FOD! OCCUPY[G:%d,B:%d]", sendReqs.size(), sendReqs_broad.size());

		// -------------------------------------------------------------
		// P4. 발송 시작
		//		r : 요청건 INDEX
		// -------------------------------------------------------------
		for( i = 0 ; i < (int)sendReqs.size() ; ++i )
		{
			while( true )
			{
				if( vecChIdxes.size() > 0  )
				{
					StartOutbound( vecChIdxes.front(), sendReqs[i] );
					vecChIdxes.pop_front();
					break;
				}
			
				if( vecChIdxes_broad.size() > 0 )
				{
					StartOutbound( vecChIdxes_broad.front(), sendReqs[i] );
					vecChIdxes_broad.pop_front();
					break;
				}

				Sleep( 1000 );
			}
		}
		for( i = 0 ; i < (int)sendReqs_broad.size() ; ++i )
		{
			while( true )
			{
				if( vecChIdxes_broad.size() > 0 )
				{
					StartOutbound( vecChIdxes_broad.front(), sendReqs_broad[i] );
					vecChIdxes_broad.pop_front();
					break;
				}

				Sleep( 1000 );
			}
		}

		//Sleep(600000);
	}
}

bool CDbPollingThread::StartOutbound( int chIdx, CDbModule::SEND_REQ& sendReq )
{
	const char*	storageFile= sendReq.detail.tifFile_storage;
	const char*	localFile  = sendReq.detail.tifFile_send;

	++sendReq.detail.TRY_CNT;
	
	// -------------------------------------------------------------
	// P2-2. 발송할 TIF File이 이미 존재하는지 확인한다.
	//		동보 발송인 경우 이미 발송할 TIF File이 존재하기 때문에 2번, 3번 FTP 처리할 필요가 없다.
	// -------------------------------------------------------------
		
	if( !IsExistFileEx(localFile, false) )		// 발송할 TIF File이 존재하지 않는 경우
	{	
		// -------------------------------------------------------------
		// P2-3-1. 발송할 TIF File 가져오기
		// -------------------------------------------------------------
		if( CopyFile(storageFile, localFile, FALSE) )
		{
			APPLOG->Print(DBGLV_RPT, "[SUCC] CopyFile! SRC[%s] DEST[%s]", storageFile, localFile);
		}
		else 
		{
			APPLOG->Print(DBGLV_ERR, "[FAIL] CopyFile! SRC[%s] DEST[%s]", storageFile, localFile);

			CDbModule::Inst()->DbLock();
			if( sendReq.detail.TRY_CNT < CConfig::TRY_CNT_DOWNLOAD )
				CDbModule::Inst()->RetrySendReq( sendReq, CConfig::FAX_START_CH + chIdx, F_FILE_FAIL_TO_DOWNLOAD, CConfig::TRY_DELAY_DOWNLOAD );
			else
				CDbModule::Inst()->FinishSendReq( sendReq, CConfig::FAX_START_CH + chIdx, F_FILE_FAIL_TO_DOWNLOAD );
			CDbModule::Inst()->DbUnlock();

			return false;
		}
	}
	else
	{
		APPLOG->Print(DBGLV_RPT, "[SUCC] IsExistFileEx! DEST[%s] SRC[%s]", localFile, storageFile);
	}

	
	g_FaxChInfo[ chIdx ].reqSend( sendReq );

	return true;
}

// -------------------------------------------------------------------
// Module 명	: bool IsExistFileEx()
// -------------------------------------------------------------------
// Descriotion	: File 떠는 Folder 존재여부
// -------------------------------------------------------------------
// Argument		: LPCSTR lpszFileName;		File 또는 Folder FULL PATH
//				  bool bDirectory;			Folder 여부 (true:Folder, false:Folder 아님)
// -------------------------------------------------------------------
// Return 값	: true		File 또는 Folder 존재
//				  false		File 또는 Folder 존재안함
// -------------------------------------------------------------------
bool CDbPollingThread::IsExistFileEx(LPCSTR lpszFileName, bool bDirectory)
{
	WIN32_FIND_DATA		ffd;
	
	HANDLE  hFindFile = FindFirstFile(lpszFileName, &ffd);

	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFindFile);
		
		if((bDirectory) && (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			return true;
		if(!(bDirectory) && !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			return true;
	}

	return false;
}


// -------------------------------------------------------------------
// Module 명	: bool DirectoryExists()
// -------------------------------------------------------------------
// Descriotion	: File 떠는 Folder 존재여부
// -------------------------------------------------------------------
// Argument		: LPCSTR lpszFileName;		File 또는 Folder FULL PATH
//				  bool bDirectory;			Folder 여부 (true:Folder, false:Folder 아님)
// -------------------------------------------------------------------
// Return 값	: true		File 또는 Folder 존재
//				  false		File 또는 Folder 존재안함
// -------------------------------------------------------------------
bool DirectoryExists(const char* dirName) {
  DWORD attribs = ::GetFileAttributesA(dirName);
  if (attribs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}


// ---------------------------------------------------------------------------------
// Module 명	: bool CreateMultiDir();
// ---------------------------------------------------------------------------------
// Description	: Argument로 주어진 Folder를 생성한다. (부모 Folder가 없어도 생성해 준다)
// ---------------------------------------------------------------------------------
// Argument		: TCHAR* lpszPath;		생성하고자 하는 Folder
// ---------------------------------------------------------------------------------
// Return		: TRUE		SUCC
//				  FALSE		FAIL
// ---------------------------------------------------------------------------------
bool CDbPollingThread::CreateMultiDir(TCHAR* lpszPath)
{
	TCHAR	DirName[1024];
	TCHAR*	p = lpszPath;						// 인자로 받은 디렉토리 
	TCHAR*	q = DirName;   

	while(*p)
	{
		if (('\\' == *p) || ('/' == *p))		// 루트디렉토리 혹은 Sub디렉토리 
		{
            if (':' != *(p-1)) 
            {
				if(!::CreateDirectory(DirName, NULL))
				{
					if(GetLastError() != ERROR_ALREADY_EXISTS)
						return false;
				}
			}
		}

		*q++ = *p++;
		*q = '\0';
	}

	if(!::CreateDirectory(DirName, NULL))
	{
		if(GetLastError() != ERROR_ALREADY_EXISTS)
			return false;
	}

	return true;
}


// -------------------------------------------------------------------
// Module 명	: int Get_Next_Idle_ChIdx()
// -------------------------------------------------------------------
// Descriotion	: IDLE한 FAX 송신 CH을 얻어온다.
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return 값	: >= 0			SUCC
//				 -1				가용회선 없음
// -------------------------------------------------------------------
int CDbPollingThread::Get_Idle_ChIdx(bool bBroadcast, int cntMax, list<int>& vecChIdx)
{
	int chidx_beg, chidx_end;

	vecChIdx.clear();

	if (CConfig::FAX_TOTAL_CH_CNT <= 0)
		return -1;

	if(!bBroadcast)	
	{
		chidx_beg	= ( CConfig::FAX_CH_beg ) - CConfig::FAX_START_CH;
		chidx_end	= ( CConfig::FAX_CH_end ) - CConfig::FAX_START_CH;
	}
	else 
	{
		chidx_beg	= ( CConfig::FAX_BCH_beg ) - CConfig::FAX_START_CH;
		chidx_end	= ( CConfig::FAX_BCH_end ) - CConfig::FAX_START_CH;
	}

	for(int chidx = chidx_beg ; chidx <= chidx_end ; ++chidx)
	{
		if( g_FaxChInfo[chidx].ch_state == FAX_IDLE && g_FaxChInfo[chidx].IO_flag == IO_OUTBOUND ) 
		{
			vecChIdx.push_back(chidx);
			if( vecChIdx.size() >= cntMax )
				break;
		}
	}

	return vecChIdx.size();
}