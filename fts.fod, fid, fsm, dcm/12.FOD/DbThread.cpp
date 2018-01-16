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
	int				dbRet;									// DB ���� ���
	
	CString			connStr, id, pwd;
	
	
	int				i;
	bool			bGeneral;
	list<int>		vecChIdxes, vecChIdxes_broad;
	vector<CDbModule::SEND_REQ> sendReqs;					// �߼� ��û ���� - �Ϲݰ�
	vector<CDbModule::SEND_REQ> sendReqs_broad;				// �߼� ��û ���� - ������
	
	// P1-2. �� �߼� �������̴� ��, �����·� �ʱ�ȭ
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
		// P2. IDLE ä�� �˻�
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
		// P3. �߼� ��û�� GET
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
		// P4. �߼� ����
		//		r : ��û�� INDEX
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
	// P2-2. �߼��� TIF File�� �̹� �����ϴ��� Ȯ���Ѵ�.
	//		���� �߼��� ��� �̹� �߼��� TIF File�� �����ϱ� ������ 2��, 3�� FTP ó���� �ʿ䰡 ����.
	// -------------------------------------------------------------
		
	if( !IsExistFileEx(localFile, false) )		// �߼��� TIF File�� �������� �ʴ� ���
	{	
		// -------------------------------------------------------------
		// P2-3-1. �߼��� TIF File ��������
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
// Module ��	: bool IsExistFileEx()
// -------------------------------------------------------------------
// Descriotion	: File ���� Folder ���翩��
// -------------------------------------------------------------------
// Argument		: LPCSTR lpszFileName;		File �Ǵ� Folder FULL PATH
//				  bool bDirectory;			Folder ���� (true:Folder, false:Folder �ƴ�)
// -------------------------------------------------------------------
// Return ��	: true		File �Ǵ� Folder ����
//				  false		File �Ǵ� Folder �������
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
// Module ��	: bool DirectoryExists()
// -------------------------------------------------------------------
// Descriotion	: File ���� Folder ���翩��
// -------------------------------------------------------------------
// Argument		: LPCSTR lpszFileName;		File �Ǵ� Folder FULL PATH
//				  bool bDirectory;			Folder ���� (true:Folder, false:Folder �ƴ�)
// -------------------------------------------------------------------
// Return ��	: true		File �Ǵ� Folder ����
//				  false		File �Ǵ� Folder �������
// -------------------------------------------------------------------
bool DirectoryExists(const char* dirName) {
  DWORD attribs = ::GetFileAttributesA(dirName);
  if (attribs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}


// ---------------------------------------------------------------------------------
// Module ��	: bool CreateMultiDir();
// ---------------------------------------------------------------------------------
// Description	: Argument�� �־��� Folder�� �����Ѵ�. (�θ� Folder�� ��� ������ �ش�)
// ---------------------------------------------------------------------------------
// Argument		: TCHAR* lpszPath;		�����ϰ��� �ϴ� Folder
// ---------------------------------------------------------------------------------
// Return		: TRUE		SUCC
//				  FALSE		FAIL
// ---------------------------------------------------------------------------------
bool CDbPollingThread::CreateMultiDir(TCHAR* lpszPath)
{
	TCHAR	DirName[1024];
	TCHAR*	p = lpszPath;						// ���ڷ� ���� ���丮 
	TCHAR*	q = DirName;   

	while(*p)
	{
		if (('\\' == *p) || ('/' == *p))		// ��Ʈ���丮 Ȥ�� Sub���丮 
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
// Module ��	: int Get_Next_Idle_ChIdx()
// -------------------------------------------------------------------
// Descriotion	: IDLE�� FAX �۽� CH�� ���´�.
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return ��	: >= 0			SUCC
//				 -1				����ȸ�� ����
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