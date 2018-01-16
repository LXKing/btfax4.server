// FaxChThread.cpp : implementation file
//

#include "stdafx.h"
#include "APP.h"
#include "ShmCtrl.h";
#include "FaxChThread.h"
#include "FsmIfThread.h"
#include "DbModule.h"
#include "Config.h"
#include "btfconvert.h"
#include "StatusIni.h"
#include "Utility.h"
#include "PacketDef.h"

#include "btfax.h"
#include "btflib.h"

#include "EncryptApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFaxChThread

CFaxChThread::CFaxChThread()
{
	m_bStart	= false;

	m_chidx		= -1;
	m_chnum		= -1;
}

CFaxChThread::~CFaxChThread()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFaxChThread message handlers

// -------------------------------------------------------------------
// Module 명	: void GetFaxChNum()
// -------------------------------------------------------------------
// Descriotion	: CFaxChThread 객체의 0 Based FAX CH 번호를 얻는다.
// -------------------------------------------------------------------
// Argument		: int &chidx;		g_FaxChInfo Array의 Index
//				  int &chnum;		0 Based FAX CH 번호
// -------------------------------------------------------------------
// Return 값	: 0 ~ MAX_FODCH-1		SUCC
//				  -1					FAIL
// -------------------------------------------------------------------
void CFaxChThread::GetFaxChNum(int &chidx, int &chnum)
{
	if(m_chidx < 0 || m_chidx >= MAX_CHANNEL || m_chnum < 0)
	{
		chidx = -1;
		chnum = -1;
	}
	else
	{
		chidx = m_chidx;
		chnum = m_chnum;
	}
	return;
}

// -------------------------------------------------------------------
// Module 명	: bool SetFaxChNum()
// -------------------------------------------------------------------
// Descriotion	: CFaxChThread 객체의 0 Based FAX CH 번호를 설정한다.
// -------------------------------------------------------------------
// Argument		: int chidx;		g_FaxChInfo Array의 Index
//				  int chnum;		0 Based FAX CH 번호
// -------------------------------------------------------------------
// Return 값	: true			SUCC
//				  false			FAIL
// -------------------------------------------------------------------
bool CFaxChThread::SetFaxChNum(int chidx, int chnum)
{
	// CH개수가 MAX_FODCH을 넘지 않도록 이미 LoadIni()에서 Check
	if( chidx < 0 || chidx >= MAX_CHANNEL || chnum < 0 )
	{
		m_chidx = -1;
		m_chnum = -1;

		return false;
	}
	else
	{
		m_chidx = chidx;
		m_chnum = chnum;

		return true;
	}
}


// -------------------------------------------------------------------
// Module 명	: void SetShmState()
// -------------------------------------------------------------------
// Descriotion	: SharedMemory 상태 업데이트
// -------------------------------------------------------------------
// Argument		: EN_STATUS ch_state;		FAX CH 상태 (EN_STATUS)
// -------------------------------------------------------------------
// Return 값	: void
// -------------------------------------------------------------------
void CFaxChThread::SetShmState( EN_STATUS ChState)
{
	char currTime[20];
	CUtility::CurrentTimeToStr(currTime);

	SHM_CH_MONI_DATA* shmData = g_shmCtrl.GetShmData(m_chnum);
	if(shmData == NULL)
		return;
	
	shmData->channel_state	= ChState;
	shmData->chg_flag		= 1;	
	switch(ChState)
	{
		case FAX_INIT	: 
			shmData->Clear();
			shmData->use_flag				= 1;
			shmData->channel				= m_chnum;
			shmData->call_diretion			= IO_INBOUND;
			shmData->system_id				= CConfig::SYSTEM_NO;
			shmData->module_id				= CConfig::PROCESS_NO;			
			break;

		case FAX_IDLE	: 			
			memset(shmData->ani,			0x00, sizeof(shmData->ani));
			memset(shmData->dnis,			0x00, sizeof(shmData->dnis));
			memset(shmData->service_id,		0x00, sizeof(shmData->service_id));
			memset(shmData->service_name,	0x00, sizeof(shmData->service_name));

			memset(shmData->call_connected_time		,	0x00, sizeof(shmData->call_connected_time));
			memset(shmData->call_incoming_time		,	0x00, sizeof(shmData->call_incoming_time));
			memset(shmData->call_disconnected_time	,	0x00, sizeof(shmData->call_disconnected_time));
			break;

		case FAX_OCCUPY : break;
		case FAX_DIAL	: break;
		case FAX_SEND	: break;

		case FAX_RECV	: 
			strcpy(shmData->call_connected_time, currTime);
			strcpy(shmData->call_incoming_time, currTime);

			shmData->inbound_cnt++;

			strcpy(shmData->ani, g_FaxChInfo[m_chidx].Ani);			
			strcpy(shmData->dnis, g_FaxChInfo[m_chidx].Dnis);
			break;

		case FAX_SUCC_SEND : 
		case FAX_FAIL_SEND : 
		case FAX_SUCC_RECV : 
		case FAX_FAIL_RECV : 
			strcpy(shmData->call_disconnected_time, currTime);
			break;
		
		case FAX_ABORT	: break;
		default : break;
	}

	APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] SHM_INFO (channel:%d, channel_state:%d, chg_flag:%d, inbound_cnt:%d, system_id:%d, module_id:%d, ani:%s, dnis:%s)"
		, m_chnum		
		, shmData->channel		
		, shmData->channel_state
		, shmData->chg_flag
		, shmData->inbound_cnt		
		, shmData->system_id
		, shmData->module_id
		, shmData->ani
		, shmData->dnis									
		);
}
	

// -------------------------------------------------------------------
// Module 명	: void SetFaxChState()
// -------------------------------------------------------------------
// Descriotion	: FAX CH의 상태를 설정
// -------------------------------------------------------------------
// Argument		: EN_STATUS ch_state;		FAX CH 상태 (EN_STATUS)
//				  char* ch_msg;				CH 상태 Message
// -------------------------------------------------------------------
// Return 값	: void
// -------------------------------------------------------------------
void CFaxChThread::SetFaxChState( EN_STATUS ChState, const char* szStatusMsg, const char* szLogMsg )
{
	CString strLogLine;

	if(m_chidx < 0 || m_chidx >= MAX_CHANNEL || m_chnum < 0)
		return;

	// P1. 상태 변경
	g_FaxChInfo[m_chidx].ch_state	= ChState;

	// P2. 채널 상태  INI 갱신
	CStatusIni::Inst()->Update( m_chidx, ChState, szStatusMsg, &strLogLine );

	// P3. 상태 DISPLAY
	if( szLogMsg ) {
		strLogLine += ":";
		strLogLine += szLogMsg;
	}

	//gPAPP->DisplayState(m_chidx, ChState, strLogLine );
	APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] %s", m_chnum, (LPCSTR)strLogLine );
		
	// P4. Shared Memory 갱신
	SetShmState(ChState);
}

void CFaxChThread::SetFaxChState( EN_STATUS ChState, const char* szAni, const char* szDnis, const char* szSrcFile )
{
	CString strStatusMsg, strLogMsg;

	strStatusMsg.Format( "ANI[%s]DID[%s]", szAni, szDnis );
	strLogMsg.Format( "수신 ANI[%s] DN[%s] File[%s]", szAni, szDnis, szSrcFile );
	
	SetFaxChState( ChState, strStatusMsg, strLogMsg );
}

// -------------------------------------------------------------------
// Module 명	: bool InitFaxChInfo();
// -------------------------------------------------------------------
// Descriotion	: g_FaxChInfo[m_chidx]를 사용가능 상태로 설정한다.
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return 값	: void
// -------------------------------------------------------------------
void CFaxChThread::InitFaxChInfo()
{
	g_FaxChInfo[m_chidx].chnum		= m_chnum;
	g_FaxChInfo[m_chidx].IO_flag	= IO_INBOUND;

	// shared memory init
	SetShmState(FAX_INIT);
}

// -------------------------------------------------------------------
// Module 명	: bool IdleFaxChInfo();
// -------------------------------------------------------------------
// Descriotion	: 다음 팩스 수신을 위해 자신의 g_FaxChInfo[m_chidx]을 초기화 한다.
// -------------------------------------------------------------------
// Argument		: void
// -------------------------------------------------------------------
// Return 값	: void
// -------------------------------------------------------------------
void CFaxChThread::IdleFaxChInfo()
{
	// Update By SSY [ANI번호 깨지는 현상 보정] ----- START (2012.01.26)
	if(m_chidx < 0 || m_chidx >= MAX_CHANNEL || m_chnum < 0)
		return;
	// Update By SSY [ANI번호 깨지는 현상 보정] ----- END   (2012.01.26)

	memset(g_FaxChInfo[m_chidx].Ani, 0x00, sizeof(g_FaxChInfo[m_chidx].Ani));
	memset(g_FaxChInfo[m_chidx].Dnis, 0x00, sizeof(g_FaxChInfo[m_chidx].Dnis));

	memset(g_FaxChInfo[m_chidx].recvDateTime, 0x00, sizeof(g_FaxChInfo[m_chidx].recvDateTime));
	memset(g_FaxChInfo[m_chidx].srcFile, 0x00, sizeof(g_FaxChInfo[m_chidx].srcFile));
	memset(g_FaxChInfo[m_chidx].destFile, 0x00, sizeof(g_FaxChInfo[m_chidx].destFile));

	SetFaxChState( FAX_IDLE, NULL, NULL );
}


// -------------------------------------------------------------------
// Module 명	: bool GetTel();
// -------------------------------------------------------------------
// Descriotion	: SIP 전화번호(sip:4199@100.100.104.75)를
//						일반 전화번호(4199)로 변경한다.
// -------------------------------------------------------------------
// Argument		: char* pSipTel;		SIP 전화번호
//				  char* pPstnTel;		PSTN 전화번호
//				  bool tel4len;			true : PSTN 전화번호 중 끝 4자리만 사용
//										false : PSTN 전화번호 모두 사용
// -------------------------------------------------------------------
// Return 값	: true			전화번호 얻기 성공
//				  false			전화번호 얻기 실패
// -------------------------------------------------------------------
bool CFaxChThread::GetTel( const char* pSipTel, CString* pstrPstnTel )
{
	const char	*pStartPos, *pEndPos;
	
	*pstrPstnTel = "";

	// P1. 시작 위치 찾기
	pStartPos = strchr(pSipTel, ':'); 
	if( pStartPos )
	{
		// CASE : sip:4199@100.100.104.75	=> tel: "4199"
		pStartPos += 1; // ':' 제외
	}
	else
	{
		// CASE : "4199@100.100.104.75"		=> tel: "4199"
		pStartPos = pSipTel;
	}

	// P2. 끝 위치 찾기
	pEndPos = strchr(pSipTel, '@');
	if( pEndPos == NULL )
	{
		// CASE : "sip:100.100.104.75"	=> tel: ""
		return false;
	}

	// P3. 전화번호만 추출
	pstrPstnTel->Append( pStartPos, pEndPos - pStartPos );

	return true;
}


// -------------------------------------------------------------------
// Module 명	: bool IsExistFileEx();
// -------------------------------------------------------------------
// Descriotion	: 주어진 File 또는 Folder 존재여부 확인
// -------------------------------------------------------------------
// Argument		: LPCSTR lpszFileName;		File 또는 Folder
//				  bool bDirectory;			true : Folder, false : File
// -------------------------------------------------------------------
// Return 값	: true			File 또는 Folder 존재
//				  false			File 또는 Folder 존재하지 않음
// -------------------------------------------------------------------
bool CFaxChThread::IsExistFileEx(LPCSTR lpszFileName, bool bDirectory)
{
	WIN32_FIND_DATA ffd;
	
	HANDLE  hFindFile = FindFirstFile(lpszFileName, &ffd);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFindFile);
		
		if ( (bDirectory) && (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			return true;
		if ( !(bDirectory) && !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			return true;
	}
	return false;
}

bool CFaxChThread::IsExistFileEx(LPCSTR p_szBasePath, LPCSTR p_szSubPath, bool bDirectory)
{
	CFileStatus status;

	if( !CFile::GetStatus( g_FaxChInfo[m_chidx].srcFile, status ) )	
		return false;
	
	if( bDirectory )
	{
		if( status.m_attribute | CFile::directory )
			return false;
	}
	else
	{
		if( status.m_attribute | CFile::normal )
			return false;
	}

	return false;
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
bool CFaxChThread::CreateMultiDir(LPCSTR lpszPath)
{
	TCHAR	DirName[1024];
	LPCSTR	p = lpszPath;						// 인자로 받은 디렉토리 
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

bool CFaxChThread::CreateMultiDir(CString p_strBasePath, CString p_strSubPath)
{
	int		nPos_prev, nPos;
	bool	bResult;

	nPos_prev = nPos = 0;
	while( true )
	{
		nPos = p_strSubPath.Find( "\\", nPos_prev );
		if( nPos < 0 )
		{
			if( p_strSubPath.GetLength() > nPos_prev )
				bResult = CreateDirectory( p_strBasePath + "\\" + p_strSubPath, NULL) ? true : false;

			break;
		}

		bResult = CreateDirectory( p_strBasePath + "\\" + p_strSubPath.Left(nPos), NULL)  ? true : false;
		nPos_prev = nPos + 1;
	}

	return bResult;
}


// ---------------------------------------------------------------------------------
// Module 명	: bool WriteRecvTifInfo();
// ---------------------------------------------------------------------------------
// Description	: TIFF 파일을 FTP(Move)하고 DB에 등록한다.
// ---------------------------------------------------------------------------------
// Argument		: int pagecnt;			수신 TIFF File의 Page수
// ---------------------------------------------------------------------------------
// Return		: true		SUCC
//				  false		FAIL
// ---------------------------------------------------------------------------------
bool CFaxChThread::WriteRecvTifInfo(int pagecnt, CString* pstrFaxId)
{
	int				convertRet;

	CString			strStorageDir_full		= "";				// TIFF 저장 Folder
	CString			strStorageDir_sub		= "";				// FTP PUT할 때 SubFolder
	CString			strTifFile		= "";						// TIFF File 명
	CString			strTifFullFile	= "";						// TIFF File 명 (FULL Path)
	CTime			ctNow = CTime::GetCurrentTime();	

    CString         strDestTifFilePath = "";                    // 대상TIF 파일경로
    CString         strDestTifFileName = "";                    // 대상TIF 파일명
    CString         strOutTifFullFile = "";                     // 이미지 변환후 파일명 (Full Path)
    CString         strStdTifFullFile = "";                     // 이미지 변환후 원본파일 (Full Path)

	// P1. 최종 수신 TIFF File명 만들기	
	strStorageDir_sub	= ctNow.Format( "%Y_%m\\%d" );
	strStorageDir_full	= CConfig::INBOUND_TIF_FULL_PATH + "\\", (LPCSTR)strStorageDir_sub;
	strTifFile.Format( "%s\\%s_%02d_%03d.tif", (LPCSTR)strStorageDir_sub, g_FaxChInfo[m_chidx].recvDateTime, CConfig::SYSTEM_NO, m_chnum );
	strTifFullFile		= CConfig::INBOUND_TIF_FULL_PATH + "\\" + strTifFile;
	
    TCHAR szDir[_MAX_DIR];
    TCHAR szDrive[_MAX_DRIVE];
    TCHAR szFileName[50];
    TCHAR szFileExt[5];

    _tsplitpath(g_FaxChInfo[m_chidx].srcFile,szDrive, szDir, szFileName, szFileExt);

    strDestTifFilePath.Format("%s\\%s", szDrive, szDir);
    strDestTifFileName.Format("OUT_%s%s", szFileName, szFileExt );
    strStdTifFullFile.Format("%s.standard", strTifFullFile );
    strOutTifFullFile.Format("%s\\%s", strDestTifFilePath, strDestTifFileName);
    

	
    // P1. 디렉터리 생성후 업로드
	if( !IsExistFileEx( CConfig::INBOUND_TIF_FULL_PATH, strStorageDir_sub, true ) )
		CreateMultiDir( CConfig::INBOUND_TIF_FULL_PATH, strStorageDir_sub );

    // P2. 수신팩스 이미지 변환
    convertRet = HmpTiffConvert2High( g_FaxChInfo[m_chidx].srcFile,  LPSTR(LPCTSTR(strOutTifFullFile)));    
    switch(convertRet)
    {
        // 이미지 변환 필요없음.
        case Sc_Nothing_To_Convert:
            APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] [SUCC] Convert[%s] ret[%d]", m_chnum, g_FaxChInfo[m_chidx].srcFile, convertRet );
            break;

        // 이미지 변환 성공
        case Sc_Convert_Success:
            APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] [SUCC] Convert[%s] => [%s] ret[%d]", m_chnum, g_FaxChInfo[m_chidx].srcFile, strOutTifFullFile, convertRet );
        
            // 원본파일 이동
            if(!MoveFileEx( g_FaxChInfo[m_chidx].srcFile, strStdTifFullFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH))
                APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] [FAIL] MoveFileEx error !![%s] => [%s]", m_chnum, g_FaxChInfo[m_chidx].srcFile, strStdTifFullFile );

            strcpy( g_FaxChInfo[m_chidx].srcFile, (LPCSTR)strOutTifFullFile );
            break;

        default:
            APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] [FAIL] Convert[%s] ret[%d]", m_chnum, g_FaxChInfo[m_chidx].srcFile, convertRet );
    }

	strcpy( g_FaxChInfo[m_chidx].destFile, (LPCSTR)strTifFullFile );

	// P3. 파일 크기 얻기
	CFileStatus status;

	if( !CFile::GetStatus( g_FaxChInfo[m_chidx].srcFile, status ) )	
		return false;
	int	nFileSize = (int)status.m_size;
    
    // P4. 파일업로드
	if( MoveFileEx( g_FaxChInfo[m_chidx].srcFile, g_FaxChInfo[m_chidx].destFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) )
	{
		APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] [-] Move SUCC... [%s->%s]",
								m_chnum,
								g_FaxChInfo[m_chidx].srcFile,
								g_FaxChInfo[m_chidx].destFile );
	}
	else
	{
		APPLOG->Print( DBGLV_RPT, "[CHAN_%03d] [-] Move FAIL... [%s->%s]",
								m_chnum,
								g_FaxChInfo[m_chidx].srcFile,
								g_FaxChInfo[m_chidx].destFile );
		return false;
	}
	
	if(UpdateRecvFinished(*pstrFaxId, 1, 1, strTifFile, nFileSize, pagecnt) <0)
		return false;
	
	return true;
}


int CFaxChThread::InsertRecvTifInfo( const char* pAni, const char* pDnis, const char* pTifFile, int	nTifSize, int nPageCnt, CString* pstrFaxId )
{
	int				dbRet;									// DB 접속 결과
	int				effectCnt;

	CString			connStr, id, pwd;
	

	CDbModule::RECV_INFO recvInfo;

	recvInfo.CID			= pAni;
	recvInfo.DID			= pDnis;
	recvInfo.TIF_FILE		= pTifFile;
	recvInfo.TIF_FILE_SIZE	= nTifSize;
	recvInfo.TIF_PAGE_CNT	= nPageCnt;


	effectCnt = CDbModule::Inst()->InsertRecvInfo(m_chnum, recvInfo, pstrFaxId);
	if(effectCnt <= 0)								/* Oracle 처리 오류 -> 재 초기화 */
	{
		APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] P_FOD_INSERT_FAX_RCV_INFO() Fail[%d]!",
									m_chnum,
									effectCnt);
	}
	else
	{
		APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] P_FOD_INSERT_FAX_RCV_INFO() SUCC[%d]!",
									m_chnum,
									effectCnt);
	}

	return effectCnt;
}


int CFaxChThread::InsertRecvTifInfoEx( const char* pMgwIp, int pMgwPort, const char* pAni, const char* pDnis, const char* pTifFile, int	nTifSize, int nPageCnt, CString* pstrFaxId )
{
	int				dbRet;									// DB 접속 결과
	int				effectCnt;

	CString			connStr, id, pwd, strTemp, strAni, strDnis;
	

	CDbModule::RECV_INFO recvInfo;

    // ADD - KIMCG : 20140903
    if(CConfig::ENCRYPT_FIELD_YN == "Y")
    {
        if(EncryptApi::Inst()->TryEncrypt(pAni, strTemp)) 
            strAni = strTemp;
        else 
            strAni = pAni;

        if(EncryptApi::Inst()->TryEncrypt(pDnis, strTemp)) 
            strDnis = strTemp;
        else 
            strDnis = pDnis;
    }
    else
    {
        strAni = pAni;
        strDnis = pDnis;
    }
    // ADD - END

	recvInfo.CID			= strAni;
	recvInfo.DID			= pDnis;
	recvInfo.TIF_FILE		= pTifFile;
	recvInfo.TIF_FILE_SIZE	= nTifSize;
	recvInfo.TIF_PAGE_CNT	= nPageCnt;


	effectCnt = CDbModule::Inst()->InsertRecvInfoEx(m_chnum, pMgwIp, pMgwPort, recvInfo, pstrFaxId);
	if(effectCnt <= 0)								/* Oracle 처리 오류 -> 재 초기화 */
	{
		APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] P_FID_INSERT_FAX_RCV_INFO_EX() Fail[%d]!",
									m_chnum,
									effectCnt);
	}
	else
	{
		APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] P_FID_INSERT_FAX_RCV_INFOEX() SUCC[%d]!",
									m_chnum,
									effectCnt);
	}

	return effectCnt;
}


// -------------------------------------------------------------------
// Module 명	: int UpdateRecvRunningState();
// -------------------------------------------------------------------
// Descriotion	: FAX_ID 상태를 RUNNING 상태로 변경함.
// -------------------------------------------------------------------
// Argument		: pFaxId
// -------------------------------------------------------------------
// Return 값	: numstr			행개수
// -------------------------------------------------------------------
int	CFaxChThread::UpdateRecvRunningState(CString pFaxId)
{
	int				dbRet;									// DB 접속 결과
	int				effectCnt;

	CString			connStr, id, pwd;
	
	effectCnt = CDbModule::Inst()->UpdateRecvRunningState(pFaxId);
	if(effectCnt <= 0)								/* Oracle 처리 오류 -> 재 초기화 */
	{
		APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] P_FID_USP_UPDATE_RECV_STATE_RUNNING() Fail[%d]!",
									m_chnum,
									effectCnt);
	}
	else
	{
		APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] P_FID_USP_UPDATE_RECV_STATE_RUNNING() SUCC[%d]!",
									m_chnum,
									effectCnt);
	}

	return effectCnt;
}


// -------------------------------------------------------------------
// Module 명	: int UpdateRecvFinished();
// -------------------------------------------------------------------
// Descriotion	: FAX_ID 상태를 FINISHED 상태로 변경함.
// -------------------------------------------------------------------
// Argument		: pFaxId
// -------------------------------------------------------------------
// Return 값	: numstr			행개수
// -------------------------------------------------------------------
int	CFaxChThread::UpdateRecvFinished(CString pstrFaxId, int pResult, int pReason, CString pTifFile, int pTifFileSize, int pTifPageCnt)
{
	int				dbRet;									// DB 접속 결과
	int				effectCnt;

	CString			connStr, id, pwd;
	
	effectCnt = CDbModule::Inst()->UpdateRecvFinished(pstrFaxId, pResult, pReason, pTifFile, pTifFileSize, pTifPageCnt);
	if(effectCnt <= 0)								/* Oracle 처리 오류 -> 재 초기화 */
	{
		APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] P_FID_USP_UPDATE_RECV_FINISHED() Fail[%d]!",
									m_chnum,
									effectCnt);
	}
	else
	{
		APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] P_FID_USP_UPDATE_RECV_FINISHED() SUCC[%d]!",
									m_chnum,
									effectCnt);
	}

	return effectCnt;
}

// -------------------------------------------------------------------
// Module 명	: bool GetFaxMsgName();
// -------------------------------------------------------------------
// Descriotion	: FaxWaitEvent()에서 얻은 BTFAX Event 종류를 Display
// -------------------------------------------------------------------
// Argument		: int msgid;		BTFAX msgid
// -------------------------------------------------------------------
// Return 값	: numstr			BTFAX msgid 명
// -------------------------------------------------------------------
char* CFaxChThread::GetFaxMsgName(int msgid)
{
	switch(msgid)
	{
	case Ipc_Fax_Close:					sprintf(m_faxEvtName, "%s", "Ipc_Fax_Close");				return m_faxEvtName;
	case Ipc_Fax_Start_Process:			sprintf(m_faxEvtName, "%s", "Ipc_Fax_Start_Process");		return m_faxEvtName;
	case Ipc_Fax_Stop_Process:			sprintf(m_faxEvtName, "%s", "Ipc_Fax_Stop_Process");		return m_faxEvtName;
	case Ipc_Fax_Restart_Process:		sprintf(m_faxEvtName, "%s", "Ipc_Fax_Restart_Process");		return m_faxEvtName;
	case Ipc_Fax_Reload_Config:			sprintf(m_faxEvtName, "%s", "Ipc_Fax_Reload_Config");		return m_faxEvtName;
	case Ipc_Fax_Trace_Terminal:		sprintf(m_faxEvtName, "%s", "Ipc_Fax_Trace_Terminal");		return m_faxEvtName;
	case Ipc_Fax_Open_Channel:			sprintf(m_faxEvtName, "%s", "Ipc_Fax_Open_Channel");		return m_faxEvtName;
	case Ipc_Fax_Close_Channel:			sprintf(m_faxEvtName, "%s", "Ipc_Fax_Close_Channel");		return m_faxEvtName;
	case Ipc_Fax_Send:					sprintf(m_faxEvtName, "%s", "Ipc_Fax_Send");				return m_faxEvtName;
	case Ipc_Fax_Recv:					sprintf(m_faxEvtName, "%s", "Ipc_Fax_Recv");				return m_faxEvtName;
	case Ipc_Fax_Stop:					sprintf(m_faxEvtName, "%s", "Ipc_Fax_Stop");				return m_faxEvtName;
	case Ipc_Fax_Abort:					sprintf(m_faxEvtName, "%s", "Ipc_Fax_Abort");				return m_faxEvtName;
	case Ipc_Fax_Bad_File:				sprintf(m_faxEvtName, "%s", "Ipc_Fax_Bad_File");			return m_faxEvtName;
	case Ipc_Fax_Trace:					sprintf(m_faxEvtName, "%s", "Ipc_Fax_Trace");				return m_faxEvtName;
	case Ipc_Indi_Fax_Reset_Channel:	sprintf(m_faxEvtName, "%s", "Ipc_Indi_Fax_Reset_Channel");	return m_faxEvtName;
	case Ipc_Indi_Fax_Send_Complete:	sprintf(m_faxEvtName, "%s", "Ipc_Indi_Fax_Send_Complete");	return m_faxEvtName;
	case Ipc_Indi_Fax_Send_Page:		sprintf(m_faxEvtName, "%s", "Ipc_Indi_Fax_Send_Page");		return m_faxEvtName;
	case Ipc_Indi_Fax_Recv_Complete:	sprintf(m_faxEvtName, "%s", "Ipc_Indi_Fax_Recv_Complete");	return m_faxEvtName;
	case Ipc_Indi_Fax_Recv_Page:		sprintf(m_faxEvtName, "%s", "Ipc_Indi_Fax_Recv_Page");		return m_faxEvtName;
	case Ipc_Indi_Fax_Stop_Complete:	sprintf(m_faxEvtName, "%s", "Ipc_Indi_Fax_Stop_Complete");	return m_faxEvtName;
	default:							sprintf(m_faxEvtName, "%d", msgid);							return m_faxEvtName;
	}
}


// -------------------------------------------------------------------
// Module 명	: long OnFaxChInit();
// -------------------------------------------------------------------
// Descriotion	: WM_FAXCH_INIT Message를 받았을 때 처리 (초기에 1번만 수행)
// -------------------------------------------------------------------
// Argument		: WPARAM wParam;	0 Based FAX CH Idx (g_FaxChInfo Array의 Index)
//				  LPARAM lParam;	0 Based FAX CH NO.
// -------------------------------------------------------------------
// Return 값	: 0					SUCC
//				 -1					FAIL
// -------------------------------------------------------------------
void CFaxChThread::InitChannel( int p_chidx, int p_chnum )
{
	int	result;
	

	// p0. WM_FAXCH_INIT를 받았을 때 Argument로 넘겨받은 0 Based FAX CH 번호를 설정
	if( !SetFaxChNum( p_chidx, p_chnum ) )
		return;

	APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] SetFaxChNum...SUCC! chIdx[%d]", m_chnum, m_chidx);

	// p1. WM_FAXCH_INIT를 받았을 때 HMP FAX CH Open (qid는 m_chidx로...)
	result = HmpFaxOpenSync(m_chnum, m_chidx);
	if( result )
	{
		APPLOG->Print(DBGLV_MAJOR, "[CHAN_%03d] HmpFaxOpenSync...FAIL reason[%s]", m_chnum, GetFaxError(result));
		return;
	}

	// P2. 자신의 정보를 담을 g_FaxChInfo 구조체에 chnum과 IO_flag 초기화
	InitFaxChInfo();
	APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] HmpFaxOpenSync...SUCC! qid[%d]", m_chnum, m_chidx);

	// p3. FAX 발송을 할 수 있는 상태로 FAX CH 상태 설정
	IdleFaxChInfo();
}


// -------------------------------------------------------------------
// Module 명	: long OnFaxChStart();
// -------------------------------------------------------------------
// Descriotion	: WM_FAXCH_START Message를 받았을 때 처리
// -------------------------------------------------------------------
// Argument		: WPARAM wParam;	NULL
//				  LPARAM lParam;	NULL
// -------------------------------------------------------------------
// Return 값	: 0					SUCC
//				 -1					FAIL
// -------------------------------------------------------------------
void CFaxChThread::onThreadEntry()
{
	BOOL			bResult;								// 전송 결과

	int*			pCompleteCH	= NULL;
	
	CString			strAni;									// 발신번호 (고객팩스번호)
	CString			strDnis;								// 착신번호 (DN)

	int				err;
	recv_tiff_info	r;

	int					ret;
	ipc_fax_msg_rcv		msg;
	ipc_fax_state*		s;
	tiff_attr*			tf;

	CTime			ctNow;
	CString			strFaxId;

	FAX_RECV_PACKET	RcvPacket;
	FAX_SEND_PACKET	SndPacket;
	

	m_bStart = true;

	while( !IsReqStop() )
	{
		// P2. Wait 'Fax_ReqCalling_Ack'
		bResult = CFsmIfThread::Inst()->RcvPacket_Wait( m_chnum, 200, &RcvPacket );

		if (bResult)
		{
			switch( RcvPacket.h.msgid )
			{
			case Fax_Rx_Start :				
				APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] [R] Fax_Rx_Start... ani[%s] dn1[%s] dn2[%s] gw[%s] port[%d]",
									m_chnum,
									RcvPacket.b.start.from,
									RcvPacket.b.start.to,
									RcvPacket.b.start.transfer,
									RcvPacket.b.start.gw_ip,
									RcvPacket.b.start.gw_port);


				// P1. FAX_IDLE 상태가 아니면 오류 처리
				if(g_FaxChInfo[m_chidx].ch_state != FAX_IDLE)
				{
					APPLOG->Print(DBGLV_ERR, "[CHAN_%03d] [R] Fax_Rx_Start...FAIL not FAX_IDLE[%d]",
									m_chnum, g_FaxChInfo[m_chidx].ch_state);
					break;
				}

				// P2. ANI 구하기
				GetTel( RcvPacket.b.start.from, &strAni );
				strAni.Replace( "-", "" );
				if( strAni.GetLength() > 0 && strAni[0] != '0' )
					strAni = "0" + strAni;
				STRCPY( g_FaxChInfo[m_chidx].Ani, strAni );
				

				// P3. DNIS 구하기
				GetTel( RcvPacket.b.start.to, &strDnis );
				STRCPY( g_FaxChInfo[m_chidx].Dnis, strDnis );

				APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] [R] Fax_Rx_Start... ani[%s] dnis[%s]",
									m_chnum, g_FaxChInfo[m_chidx].Ani, g_FaxChInfo[m_chidx].Dnis);

				// P4. 수신 TIFF 정보 설정 (Media G/W IP, Port)
				memset(&r, 0, sizeof(r));
				strcpy(r.h.ip, RcvPacket.b.start.gw_ip);
				r.h.port = RcvPacket.b.start.gw_port;
				r.h.redundancy_level = -1;

				// P5. 수신 TIFF 정보 설정 (수신시작시간)
				ctNow = CTime::GetCurrentTime();
				strcpy( g_FaxChInfo[m_chidx].recvDateTime, (LPCSTR)ctNow.Format("%Y%m%d%H%M%S") );

				// P6. 수신 TIFF 정보 설정 (수신TIFF File명 : YYYYMMDDhhmmss_FODx_CHxxx_ANI_DNIS.tif)
				sprintf(g_FaxChInfo[m_chidx].srcFile, "%s\\%s_%s_CH%03d_%s_%s.tif",
												(LPCSTR)CConfig::LOCAL_TIF_PATH,
												g_FaxChInfo[m_chidx].recvDateTime,
												(LPCSTR)CConfig::SYSTEM_ID,
												m_chidx,
												g_FaxChInfo[m_chidx].Ani,
												g_FaxChInfo[m_chidx].Dnis);

				sprintf(r.f.path, "%s", g_FaxChInfo[m_chidx].srcFile);
				r.h.resolution = Fax_Resol_High;

				// DB정보 Insert
				if(InsertRecvTifInfoEx(RcvPacket.b.start.gw_ip
										, RcvPacket.b.start.gw_port
										, g_FaxChInfo[m_chidx].Ani
										, g_FaxChInfo[m_chidx].Dnis
										, g_FaxChInfo[m_chidx].srcFile
										, 0
										, 0
										, &strFaxId) < 0)
				{	
					break;
				}

				// DB정보 RUNNING 상태로 업데이트 : 42
				Sleep( 500 );
				if(UpdateRecvRunningState(strFaxId) < 0 )
				{
					break;
				}
				
				// P7. FAX 수신
				SetFaxChState( FAX_RECV, g_FaxChInfo[m_chidx].Ani, g_FaxChInfo[m_chidx].Dnis, g_FaxChInfo[m_chidx].srcFile );
				err = HmpFaxRecv(m_chnum, &r);                
				break;

			case Fax_Rx_Restart :

				//// P1. 팩스 수신 중지 (Sync)
				//err = HmpFaxAbortSync(m_chnum);

				//APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] [R] Fax_Rx_Restart... ani[%s] dn1[%s] dn2[%s] gw[%s] port[%d]",
				//					m_chnum,
				//					RcvPacket.b.start.from,
				//					RcvPacket.b.start.to,
				//					RcvPacket.b.start.transfer,
				//					RcvPacket.b.start.gw_ip,
				//					RcvPacket.b.start.gw_port);


				//// P2. ANI 구하기
				//GetTel( RcvPacket.b.start.from, &strAni );
				//strAni.Replace( "-", "" );
				//if( strAni.GetLength() > 0 && strAni[0] != '0' )
				//	strAni = "0" + strAni;
				//STRCPY( g_FaxChInfo[m_chidx].Ani, strAni );

				//// P3. DNIS 구하기
				//GetTel( RcvPacket.b.start.to, &strDnis );
				//STRCPY( g_FaxChInfo[m_chidx].Dnis, strDnis );

				//APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] [R] __FAX_RESTART__... ani[%s] dnis[%s]",
				//					m_chnum, g_FaxChInfo[m_chidx].Ani, g_FaxChInfo[m_chidx].Dnis);

				//// P4. 수신 TIFF 정보 설정 (Media G/W IP, Port)
				//memset(&r, 0, sizeof(r));
				//strcpy(r.h.ip, RcvPacket.b.start.gw_ip);
				//r.h.port = RcvPacket.b.start.gw_port;
				//r.h.redundancy_level = -1;

				//// P5. 수신 TIFF 정보 설정 (수신시작시간)
				//ctNow = CTime::GetCurrentTime();
				//strcpy( g_FaxChInfo[m_chidx].recvDateTime, (LPCSTR)ctNow.Format("%Y%m%d%H%M%S") );

				//// P6. 기존에 수신받고 있는 TIF File이 있었으면 삭제처리한다.
				//if(strlen(g_FaxChInfo[m_chidx].srcFile) > 0)
				//{
				//	if(IsExistFileEx(g_FaxChInfo[m_chidx].srcFile, false))
				//		DeleteFile(g_FaxChInfo[m_chidx].srcFile);
				//}

				//// P7. 수신 TIFF 정보 설정 (수신TIFF File명 : YYYYMMDDhhmmss_FODx_CHxxx_ANI_DNIS.tif)
				//memset(g_FaxChInfo[m_chidx].srcFile, 0x00, sizeof(g_FaxChInfo[m_chidx].srcFile));
				//sprintf(g_FaxChInfo[m_chidx].srcFile, "%s\\%s_%s_CH%03d_%s_%s.tif",
				//								(LPCSTR)CConfig::LOCAL_TIF_PATH,
				//								g_FaxChInfo[m_chidx].recvDateTime,
				//								(LPCSTR)CConfig::SYSTEM_ID,
				//								m_chidx,
				//								g_FaxChInfo[m_chidx].Ani,
				//								g_FaxChInfo[m_chidx].Dnis);

				//sprintf(r.f.path, "%s", g_FaxChInfo[m_chidx].srcFile);
				//r.h.resolution = Fax_Resol_High;

				//// P8. FAX 수신
				//err = HmpFaxRecv(m_chnum, &r);

				//SetFaxChState( FAX_RECV, g_FaxChInfo[m_chidx].Ani, g_FaxChInfo[m_chidx].Dnis, g_FaxChInfo[m_chidx].srcFile );
				break;

			case Fax_Abort :

				APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] [R] Fax_Abort...", m_chnum);

				err = HmpFaxAbort(m_chnum);

				SetFaxChState( FAX_IDLE, NULL, NULL );

				break;

			}
		}
		else
		{
			ret=FaxWaitEvent(m_chidx, &msg, 200);
			if(!ret)
			{
				APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] [-] FAX Event[%d:%s] Occured...", msg.chan, msg.msgid, GetFaxMsgName(msg.msgid));

				switch(msg.msgid)
				{
				case Ipc_Indi_Fax_Recv_Page:
					tf=(tiff_attr *)msg.para;

					APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] [-] Ipc_Indi_Fax_Recv_Page...last_page=%d, retrans=%d",
								msg.chan, tf->flag&1, tf->flag&0x0100?1:0);

					break;

				case Ipc_Indi_Fax_Recv_Complete:
					s = (ipc_fax_state *)msg.para;
					APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] [-] Ipc_Indi_Fax_Recv_Complete...page=%d sig_rate=%d",
								msg.chan, s->page, s->sig_rate);


  					if(s->page >= 1)
					{
						// 성공
						WriteRecvTifInfo(s->page, &strFaxId);
					}
					else
					{	
						// 실패
						UpdateRecvFinished(strFaxId, -1, F_RECV_FAX_ERROR, g_FaxChInfo[m_chidx].srcFile, 0, 0);
					}

					SndPacket.h.msgid	= Fax_Complete;
					SndPacket.h.len		= 0;
					CFsmIfThread::Inst()->SndPacket_Push( m_chidx, SndPacket );

					SetFaxChState( FAX_SUCC_RECV, NULL, NULL );

					break;

				case Ipc_Fax_Abort:

					if(IsExistFileEx(g_FaxChInfo[m_chidx].srcFile, FALSE) == TRUE)
						DeleteFile(g_FaxChInfo[m_chidx].srcFile);

					APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] [-] Ipc_Fax_Abort...", msg.chan);

					//SetFaxChState( FAX_FAIL_RECV, NULL, NULL );
					IdleFaxChInfo();
					break;

				default:
					break;
				}
			}

		}
	}

	HmpFaxClose(m_chnum);
	APPLOG->Print(DBGLV_RPT, "[CHAN_%03d] FaxChThread Stop", m_chnum);
}
