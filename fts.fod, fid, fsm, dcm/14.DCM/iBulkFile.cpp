/**
 *
 *  iBulkFile.cpp
 *  iLib
 *
 *  Created by chiwoo on 13. 08. 28.
 *  Created Version : IPRON v3.2.0
 *  Copyright 2010 Bridgetec. All rights reserved.
 *
 **/
#include "StdAfx.h"
#include <iType.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef IWINDOWS
	#include <time.h>
	#include <io.h>
	#include <sys/locking.h>
	#include <direct.h>
#else
	#include <sys/time.h>
	#include <unistd.h>
	#include <dirent.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#include <iLog.h>

#include "iBulkFile.h"

struct tm *LocalTime(const time_t *timep, struct tm *result)
{
#ifdef IWINDOWS
	localtime_s(result, timep);
	return result;
#else
	return localtime_r(timep, result);
#endif
}

void SleepSec(int nSec)
{
#ifdef IWINDOWS
	Sleep(nSec*1000);
#else
	sleep(nSec);
#endif
}

int Read(int fd, void *buf, size_t count)
{
#ifdef IWINDOWS
	return _read(fd, buf, count);
#else
	return read(fd, buf, count);
#endif
}

int Write(int fd, void *buf, size_t count)
{
#ifdef IWINDOWS
	return _write(fd, buf, count);
#else
	return write(fd, buf, count);
#endif
}

iBulkFile* iBulkFile::s_pInstance = NULL;

iBulkFile* iBulkFile::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new iBulkFile;

	return s_pInstance;
}

iLog theBulkLog;

iBulkFile::iBulkFile()
{
    memset(szBasePath, 0x00, sizeof(szBasePath));
    memset(szFilePrefix, 0x00, sizeof(szFilePrefix));
    memset(szOpenFileName, 0x00, sizeof(szOpenFileName));

    blkHd = NULL;
    nOldFileChkDur = OLD_FILE_CHK_DUR;
    tOldFileChk = time(NULL) + nOldFileChkDur;

    nPreReadSeq = 0;
    nPreReadIdx = 0;

    SetError(eBFSuccess);

    return;
}

iBulkFile::~iBulkFile()
{
    Close();
}

void iBulkFile::SetOldFileChkDur(uint16 nDur)
{
    if (nOldFileChkDur < 5) return;
    nOldFileChkDur = nDur;
}

bool iBulkFile::RLock(FILE *fHd)
{
#ifdef IWINDOWS
    int nCode;
    int fd;

    if (fHd == NULL)
    {
        fHd = GetHd();
        if (fHd == NULL) return false;
    }

    fd=_fileno(fHd);

    _lseek(fd, 0L, SEEK_SET);
    nCode = _locking(fd, _LK_RLCK, sizeof(BULK_HEAD));

    mLock.Lock();

    return (nCode==0);
#else
    struct flock filelock;
    int fd;

    if (fHd == NULL)
    {
        fHd = GetHd();
        if (fHd == NULL) return false;
    }

    filelock.l_type=F_RDLCK;
    filelock.l_whence=SEEK_SET;
    filelock.l_start=0;
    filelock.l_len=sizeof(BULK_HEAD);

    fd=fileno(fHd);
    fcntl(fd, F_SETLKW, &filelock);

    mLock.Lock();

    return true;
#endif
}

bool iBulkFile::WLock(FILE *fHd)
{
#ifdef IWINDOWS
    int fd;
    int nCode;

    if (fHd == NULL)
    {
        fHd = GetHd();
        if (fHd == NULL) return false;
    }

    fd=_fileno(fHd);

    _lseek(fd, 0L, SEEK_SET);
    nCode = _locking(fd, _LK_LOCK, sizeof(BULK_HEAD));

    mLock.Lock();

    return (nCode==0);

#else
    struct flock filelock;
    int fd;
    int nCode;

    if (fHd == NULL)
    {
        fHd = GetHd();
        if (fHd == NULL) return false;
    }

    filelock.l_type=F_WRLCK;
    filelock.l_whence=SEEK_SET;
    filelock.l_start=0;
    filelock.l_len=sizeof(BULK_HEAD);

    fd=fileno(fHd);
    nCode = fcntl(fd, F_SETLKW, &filelock);

    mLock.Lock();

    return true;
#endif
}

bool iBulkFile::UnLock(FILE *fHd)
{
#ifdef IWINDOWS
    int nCode;
    int fd;

    if (fHd == NULL)
    {
        fHd = GetHd();
        if (fHd == NULL) return false;
    }

    fd=_fileno(fHd);

    _lseek(fd, 0L, SEEK_SET);
    nCode = _locking(fd, LK_UNLCK, sizeof(BULK_HEAD));

    mLock.Unlock();

    return (nCode==0);
    
#else
    struct flock filelock;
    int fd;

    if (fHd == NULL)
    {
        fHd = GetHd();
        if (fHd == NULL) return false;
    }

    filelock.l_type=F_UNLCK;
    filelock.l_whence=SEEK_SET;
    filelock.l_start=0;
    filelock.l_len=sizeof(BULK_HEAD);

    fd=fileno(fHd);
    fcntl(fd, F_SETLKW, &filelock);

    mLock.Unlock();

    return true;
#endif
}

void iBulkFile::Close(void)
{
    if (IsOpen()) fclose(GetHd());
    blkHd = NULL;
}


void iBulkFile::Init(NPCSTR szLogPath, NPCSTR szBasePath, NPCSTR szFilePfx, uint32 nMaxSize, uint32 nBlockSize)
{
    snprintf(this->szBasePath, sizeof(this->szBasePath), "%s", szBasePath);
    snprintf(this->szFilePrefix, sizeof(this->szFilePrefix), "%s", szFilePfx);

    if (nBlockSize > FILE_BLOCK_MAX_SIZE) nBlockSize = FILE_BLOCK_MAX_SIZE;

    nBulkMaxSize = nBlockSize*(nMaxSize/nBlockSize + (nMaxSize%nBlockSize == 0?0:1));
    nBulkBlockSize = nBlockSize;


    if (szLogPath != NULL)
    {
        theBulkLog.SetMaxSize(MAX_LOG_SIZE); // MByte 단위임.
        theBulkLog.Init("bulk", szLogPath, NLVL_ALL);
    }

    theBulkLog.Log(ILOG_INFO, "%s/%d: MaxSize=%d BulkSize=%d", __FUNCTION__, __LINE__, nBulkMaxSize, nBulkBlockSize);


    return;
}

void iBulkFile::SetLogLvl(uint32 nLogLvl)
{
    theBulkLog.SetLogLvl(nLogLvl);
}

void iBulkFile::SetLogLvlPtr(uint32* pLogLvl)
{
    if (pLogLvl == NULL) return;

    theBulkLog.SetLvlPtr(pLogLvl);
}

bool iBulkFile::InitBulkFile(void)
{
    FILE* hd;
    char sFileName[FILE_PATH_MAX];
    BULK_HEAD bulkHead;
    char sTemp[1024];
    int16 nSeq;
    uint32 nMod;
    uint32 nWSize;
    int fd;

    nSeq = GetLastBulkFileSeq() + 1;

    GetFileNameStr(nSeq, sFileName, sizeof(sFileName));

    hd = fopen(sFileName, "wb");   // 'w' : FILE STATUS #1
    if (hd == NULL)
    {
        theBulkLog.Log(ILOG_ERR, "%s/%d: Bulk File create error. '%s' err=%d str=%s", __FUNCTION__, __LINE__, sFileName, errno, strerror(errno));
        return false;
    }

    fd = fileno(hd);

    snprintf(bulkHead.sVer, sizeof(bulkHead.sVer), "%s", BULK_VER);
    bulkHead.nFileState = eBFStateInit;
    bulkHead.nCreateTime = time(NULL);

    bulkHead.nBlockMaxCnt = (nBulkMaxSize/nBulkBlockSize);
    bulkHead.nBlockSize = nBulkBlockSize;
    bulkHead.nBlockCnt = 1;
    bulkHead.nTotLen = sizeof(bulkHead); // 전체 길이
    bulkHead.nTotCnt = 0; // 전체 개수
    bulkHead.nRSeq = 1;   // 다음읽기 순번
    bulkHead.nRIdx = sizeof(bulkHead);   // 다음읽기 위치

    WLock(hd);

    nWSize = ::Write(fd, &bulkHead, sizeof(bulkHead));
    if (nWSize <= 0)
    {
        theBulkLog.Log(ILOG_ERR, "%s/%d: head write error. '%s' err=%d str=%s", __FUNCTION__, __LINE__, sFileName, errno, strerror(errno));
        UnLock(hd);
        fclose(hd);
        remove(sFileName);
        return false;
    }

    nWSize = 0;
    memset(sTemp, 0x00, sizeof(sTemp));
	
    for (uint32 i = 0; i < nBulkBlockSize/sizeof(sTemp); i++)
    {
        nWSize += ::Write(fd, sTemp, sizeof(sTemp));
    }

    nMod = nBulkBlockSize%sizeof(sTemp);
    if (nMod != 0)
    {
        nWSize += ::Write(fd, sTemp, nMod);
    }
	
    UnLock(hd);

    fclose(hd);

    if (nWSize < nBulkBlockSize)
    {
        SetError(eBFErrBulkCreateFail);
        remove(sFileName);
        theBulkLog.Log(ILOG_ERR, "%s/%d: Bulk block space create error. '%s' req-size=%d write-size=%d", 
            __FUNCTION__, __LINE__, sFileName, nBulkBlockSize, nWSize);
        return false;
    }

    return true;
}

bool iBulkFile::ExtendBulkFile(void)
{
    BULK_HEAD bulkHead;
    char sTemp[1024];
    uint32 nBlockSize;
    uint32 nMod;
    uint32 nWSize;
    int fd;

    if (!IsOpen()) return false;

    WLock();

    fd = fileno(GetHd());

    lseek(fd, 0L, SEEK_SET);
    ::Read(fd, &bulkHead, sizeof(bulkHead));

    if (bulkHead.nBlockMaxCnt <= bulkHead.nBlockCnt)
    {
        SetError(eBFInfoBulkExtendFull);
        UnLock();
        theBulkLog.Log(ILOG_INFO, "%s/%d: Bulk block space extend full. '%s' block=%d/%d", 
            __FUNCTION__, __LINE__, szOpenFileName, bulkHead.nBlockCnt, bulkHead.nBlockMaxCnt);
        return false;
    }

    nWSize = 0;
    nBlockSize = bulkHead.nBlockSize;

    if (bulkHead.nBlockSize > FILE_BLOCK_MAX_SIZE) bulkHead.nBlockSize = FILE_BLOCK_MAX_SIZE;

    memset(sTemp, 0x00, sizeof(sTemp));

    lseek(fd, 0L, SEEK_END);

    for (uint32 i = 0; i < nBlockSize/sizeof(sTemp); i++)
    {
        nWSize += ::Write(fd, sTemp, sizeof(sTemp));
    }

    nMod = nBlockSize%sizeof(sTemp);
    if (nMod != 0)
    {
        nWSize += ::Write(fd, sTemp, nMod);
    }

    if (nWSize < nBlockSize)
    {
        SetError(eBFErrBulkExtendFail);
        UnLock();
        CloseWrite();
        Close();
        theBulkLog.Log(ILOG_ERR, "%s/%d: Bulk block space extend error. auto write close '%s' req-size=%d write-size=%d", 
            __FUNCTION__, __LINE__, szOpenFileName, nBlockSize, nWSize);
        return false;
    }

    bulkHead.nBlockCnt++;

    lseek(fd, 0L, SEEK_SET);
    ::Write(fd, &bulkHead, sizeof(bulkHead));

    UnLock();

    return true;
}

bool iBulkFile::ExistFile(NPCSTR szFileName)
{
    struct stat stinfo;

    if (stat(szFileName, &stinfo) == 0) return true;

    return false;
}

NPCSTR iBulkFile::GetTodayStr(NPSTR szBuf, uint32 nBufSize)
{
    time_t  cTime;
    struct tm _tm;

    time(&cTime);
    LocalTime(&cTime, &_tm);
    snprintf(szBuf, nBufSize, "%04d%02d%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday);

    return szBuf;
}

NPCSTR iBulkFile::GetFileNameStr(uint16 nSeq, NPSTR szBuf, uint32 nBufSize)
{
    char sToday[32];

    GetTodayStr(sToday, sizeof(sToday));

    snprintf(szBuf, nBufSize, "%s/%s%s-%03d.blk", szBasePath, szFilePrefix, sToday, nSeq);

    return szBuf;
}

/**
 *  오날 날짜로된 파일목록중 마지막 파일시퀀스번호를 검색한다.
 **/
int16 iBulkFile::GetLastBulkFileSeq(void)
{
    char sFileName[FILE_PATH_MAX];
    int16 nLstSeq;
    int16 i;

    nLstSeq = -1;

    for (i = 0; i < MAX_BULK_SEQ; i++)
    {
        GetFileNameStr(i, sFileName, sizeof(sFileName));
        if (ExistFile(sFileName)) nLstSeq = i;
    }

    if (nLstSeq == (MAX_BULK_SEQ-1)) return -2;

    return nLstSeq;
}

bool iBulkFile::OpenWriteBulkFile(void)
{
    BULK_HEAD bulkHead;
    char sFileName[FILE_PATH_MAX];
    int16 nSeq;
    FILE* fHd;
    bool bOk = false;

    fHd = NULL;
    nSeq = GetLastBulkFileSeq();
    if (nSeq == -2)
    {
        theBulkLog.Log(ILOG_ERR, "%s/%d: Bulk File Sequence Full. max=%d", __FUNCTION__, __LINE__, MAX_BULK_SEQ);
        return false;
    }

    if (nSeq >= 0)
    {
        GetFileNameStr(nSeq, sFileName, sizeof(sFileName));

        fHd = fopen(sFileName, "r+b");
        if (fHd == NULL)
        {
            SetError(eBFErrWriteBulkFileOpenFail);
            theBulkLog.Log(ILOG_ERR, "%s/%d: File Open Fail. filename=%s", __FUNCTION__, __LINE__, sFileName);
            return false;
        }

        fseek(fHd, 0L, SEEK_SET);
        fread(&bulkHead, sizeof(bulkHead), 1, fHd);

        bOk = true;

        bOk = (strcmp(bulkHead.sVer, BULK_VER) == 0);
        bOk = bOk && !IsWriteClose(bulkHead);

        if (!bOk) fclose(fHd);
    }

    // 열기실패하는 경우 다음 벌크파일 생성
    if (!bOk)
    {
        szOpenFileName[0] = '\0';
        if (!InitBulkFile()) return false;

        return OpenWriteBulkFile();
    }

    snprintf(szOpenFileName, sizeof(szOpenFileName), "%s", sFileName);

    SetHd(fHd);

    theBulkLog.Log(ILOG_INFO, "%s/%d: Bulk Write File Open. filename=%s", __FUNCTION__, __LINE__, szOpenFileName);
    DbgBulkHead(&bulkHead);

    return true;
}


/**
 *  Old파일 검사 주기인지 확인
 */
bool iBulkFile::IsOldFileChkTime(void)
{
    // 이전파일 검사주기가 된 경우 이전파일 존재유무 검사
    if (tOldFileChk < time(NULL))
    {
        tOldFileChk = time(NULL) + nOldFileChkDur;
        return true;
    }
    return false;
}


bool iBulkFile::GetBulkTodayFileName(NPSTR sFullFileName)
{
    char sFileName[FILE_PATH_MAX];
    struct stat stinfo;

    for (int i = 0; i < MAX_BULK_SEQ; i++)
    {
        GetFileNameStr(i, sFileName, sizeof(sFileName));

        if (stat(sFileName, &stinfo) == -1) continue;

        if (ChkReadBulkFile(sFileName))
        {
            strcpy(sFullFileName, sFileName);
            return true;
        }
    }

    return false;
}


#ifdef IWINDOWS

bool iBulkFile::GetBulkOldFileName(NPSTR sFullFileName)
{
	HANDLE hList;
	WIN32_FIND_DATA fileData;

    char sFileName[FILE_PATH_MAX];
    char sToday[32];
    uint32 nLen;
    uint32 nPfxLen;

    theBulkLog.Log(ILOG_LV2, "%s/%d: Read Oldday File Check.", __FUNCTION__, __LINE__);

    sFileName[0] = '\0';

    GetTodayStr(sToday, sizeof(sToday));
    nPfxLen = strlen(szFilePrefix);

	hList = FindFirstFile(szBasePath, &fileData);
	if(hList == INVALID_HANDLE_VALUE) return false;

    do {
        nLen = snprintf(sFileName, sizeof(sFileName), "%s", fileData.cFileName);
        if (strncmp(sFileName, szFilePrefix, nPfxLen) != 0) continue;
        if (strcmp(&sFileName[nLen-4], ".blk") != 0) continue;

        // 오늘보다 이전 날짜 파일 검색
        if (strncmp(&sFileName[nPfxLen], sToday, strlen(sToday)) >= 0) continue;

        sprintf(sFullFileName, "%s/%s", szBasePath, sFileName);

        if (ChkReadBulkFile(sFullFileName))
        {
			FindClose(hList);
            theBulkLog.Log(ILOG_LV1, "%s/%d: Bulk Read Oldday", __FUNCTION__, __LINE__);
            return true;
        }
    } while (FindNextFile(hList, &fileData));

	FindClose(hList);

    return false;
}

#else

bool iBulkFile::GetBulkOldFileName(NPSTR sFullFileName)
{
    DIR *dp;
    struct dirent *dir_entry;
    char sFileName[FILE_PATH_MAX];
    char sToday[32];
    uint32 nLen;
    uint32 nPfxLen;

    theBulkLog.Log(ILOG_LV2, "%s/%d: Read Oldday File Check.", __FUNCTION__, __LINE__);

    sFileName[0] = '\0';

    GetTodayStr(sToday, sizeof(sToday));
    nPfxLen = strlen(szFilePrefix);

    dp = opendir(szBasePath);
    if (dp == NULL) return false;

    while ((dir_entry = readdir(dp)) != NULL)
    {
        nLen = snprintf(sFileName, sizeof(sFileName), "%s", dir_entry->d_name);
        if (strncmp(sFileName, szFilePrefix, nPfxLen) != 0) continue;
        if (strcmp(&sFileName[nLen-4], ".blk") != 0) continue;

        // 오늘보다 이전 날짜 파일 검색
        if (strncmp(&sFileName[nPfxLen], sToday, strlen(sToday)) >= 0) continue;

        sprintf(sFullFileName, "%s/%s", szBasePath, sFileName);

        if (ChkReadBulkFile(sFullFileName))
        {
            closedir(dp);
            theBulkLog.Log(ILOG_LV1, "%s/%d: Bulk Read Oldday", __FUNCTION__, __LINE__);
            return true;
        }
    }

    closedir(dp);

    return false;
}

#endif

bool iBulkFile::OpenReadBulkOldFile(void)
{
    char sBulkFileName[FILE_PATH_MAX] = "";
    BULK_HEAD bulkHead;

    if (GetBulkOldFileName(sBulkFileName))
    {
        if (OpenReadBulkFile(sBulkFileName))
        {
            WLock();
            _ReadBulkHead(&bulkHead);
            UnLock();

            if (bulkHead.nRSeq > bulkHead.nTotCnt)
            {
                if (!IsWriteClose(bulkHead))
                {
                    CloseReadWrite();
                } else {
                    CloseRead();
                }
                Close();
                return false;
            } else {
                if (!IsWriteClose(bulkHead))
                {
                    CloseWrite();
                }
            }

            return true;
        }
    }

    return false;
}


bool iBulkFile::OpenReadBulkTodayFile(void)
{
    char sBulkFileName[FILE_PATH_MAX] = "";

    if (GetBulkTodayFileName(sBulkFileName))
    {
        if (OpenReadBulkFile(sBulkFileName))
        {
            theBulkLog.Log(ILOG_LV1, "%s/%d: Bulk Read Today %s", __FUNCTION__, __LINE__, sBulkFileName);
            return true;
        }
    }

    return false;
}

/**
 *  지정 파일이 읽기 완료 마스크가 되어 있는지 확인. 
 */
bool iBulkFile::ChkReadBulkFile(NPCSTR sFileName)
{
    BULK_HEAD bulkHead;
    FILE* fHd;
    bool bOk = false;
    int fd;

    fHd = fopen(sFileName, "r+b");
    if (fHd == NULL)
    {
        theBulkLog.Log(ILOG_WARN, "%s/%d: File Open Fail. filename=%s", __FUNCTION__, __LINE__, sFileName);
        return false;
    }

    fd = fileno(fHd);

    lseek(fd, 0L, SEEK_SET);
    ::Read(fd, &bulkHead, sizeof(bulkHead));

    bOk = true;
    bOk = (strcmp(bulkHead.sVer, BULK_VER) == 0);
    bOk = bOk && !IsReadClose(bulkHead);

    // 열기실패하는 경우 다음 벌크파일 생성
    if (!bOk)
    {
        fclose(fHd);
        return false;
    }

    fclose(fHd);

    return true;
}


bool iBulkFile::OpenReadBulkFile(NPCSTR sFileName)
{
    BULK_HEAD bulkHead;
    FILE* fHd;
    bool bOk = false;
    int fd;

    fHd = fopen(sFileName, "r+b");
    if (fHd == NULL)
    {
        theBulkLog.Log(ILOG_WARN, "%s/%d: File Open Fail. filename=%s", __FUNCTION__, __LINE__, sFileName);
        return false;
    }

    fd = fileno(fHd);

    lseek(fd, 0L, SEEK_SET);
    ::Read(fd, &bulkHead, sizeof(bulkHead));

    bOk = true;
    bOk = (strcmp(bulkHead.sVer, BULK_VER) == 0);
    bOk = bOk && !IsReadClose(bulkHead);

    // 열기실패하는 경우 다음 벌크파일 생성
    if (!bOk)
    {
        fclose(fHd);
        return false;
    }

    Close();

    snprintf(szOpenFileName, sizeof(szOpenFileName), "%s", sFileName);

    SetHd(fHd);

    nPreReadSeq = bulkHead.nRSeq;
    nPreReadIdx = bulkHead.nRIdx;

    theBulkLog.Log(ILOG_INFO, "%s/%d: Bulk Read File Open. filename=%s", __FUNCTION__, __LINE__, sFileName);
    DbgBulkHead(&bulkHead);

    return true;
}

void iBulkFile::DbgBulkHead(BULK_HEAD* pHead)
{
    char sTime[32];

    GetTimeStr(pHead->nCreateTime, sTime, sizeof(sTime));

    theBulkLog.Log(ILOG_INFO, "               Ver : %s", pHead->sVer);
    theBulkLog.Log(ILOG_INFO, "             State : 0x%02x", pHead->nFileState);
    theBulkLog.Log(ILOG_INFO, "       Create Time : %s", sTime);
    theBulkLog.Log(ILOG_INFO, "BlkCnt / BlkMaxCnt : %d/%d", pHead->nBlockCnt, pHead->nBlockMaxCnt);
    theBulkLog.Log(ILOG_INFO, "           BlkSize : %d", pHead->nBlockSize);
    theBulkLog.Log(ILOG_INFO, "ReadPos / TotalLen : %d/%d", pHead->nRIdx, pHead->nTotLen);
    theBulkLog.Log(ILOG_INFO, "  ReadSeq / TotSeq : %d/%d", pHead->nRSeq, pHead->nTotCnt);
    theBulkLog.Log(ILOG_INFO, "");
}


bool iBulkFile::IsValidDate(BULK_HEAD* pHead)
{
    time_t  tTime;
    time_t  tToday;
    struct tm _tmTime;
    struct tm _tmToDay;
    bool bOk;

    tTime = pHead->nCreateTime;
    time(&tToday);

    LocalTime(&tTime, &_tmTime);
    LocalTime(&tToday, &_tmToDay);

    bOk = true;
    bOk = bOk && (_tmTime.tm_year == _tmToDay.tm_year);
    bOk = bOk && (_tmTime.tm_mon == _tmToDay.tm_mon);
    bOk = bOk && (_tmTime.tm_mday == _tmToDay.tm_mday);

    return bOk;
}


bool iBulkFile::_ReadBulkHead(BULK_HEAD* pHead)
{
    int fd;

    if (!IsOpen()) return false;

    fd = fileno(GetHd());

    lseek(fd, 0L, SEEK_SET);
    ::Read(fd, pHead, sizeof(*pHead));

    return true;
}

bool iBulkFile::_WriteBulkHead(BULK_HEAD* pHead)
{
    int fd;

    if (!IsOpen()) return false;

    fd = fileno(GetHd());

    lseek(fd, 0L, SEEK_SET);
    ::Write(fd, pHead, sizeof(*pHead));

    return true;
}

bool iBulkFile::_WriteBulkItem(BULK_HEAD* pHead, BulkItem* pItem)
{
    int fd;

    if (!IsOpen()) return false;

    fd = fileno(GetHd());

    pItem->SetSeq(pHead->nTotCnt+1);

    lseek(fd, pHead->nTotLen, SEEK_SET);
    ::Write(fd, pItem, pItem->GetSize());

    return true;
}

bool iBulkFile::_ReWriteBulkItem(BULK_HEAD* pHead, BulkItem* pItem)
{
    int fd;

    if (!IsOpen()) return false;

    fd = fileno(GetHd());

    lseek(fd, pHead->nRIdx, SEEK_SET);
    ::Write(fd, pItem, sizeof(*pItem));

    return true;
}

bool iBulkFile::_ReadBulkItem(BULK_HEAD* pHead, BulkItem* pItem, uint32 nBufSize)
{
    uint32 nDLen;
    int fd;

    if (!IsOpen()) return false;

    fd = fileno(GetHd());

    lseek(fd, pHead->nRIdx, SEEK_SET);
    ::Read(fd, pItem, sizeof(*pItem));

    if (nBufSize > 0)
    {
        nDLen = pItem->GetDataLen();
        if (nBufSize < nDLen) nDLen = nBufSize;

        ::Read(fd, (NPSTR)pItem->GetVal(), nDLen);
    }

    return true;
}


bool iBulkFile::CloseWrite(void)
{
    BULK_HEAD bulkHead;

    if (!IsOpen()) return false;

    WLock();

    _ReadBulkHead(&bulkHead);

    MaskFlag(bulkHead, eBFStateWClose);

    _WriteBulkHead(&bulkHead);

    UnLock();

    return true;
}

bool iBulkFile::CloseRead(void)
{
    BULK_HEAD bulkHead;

    if (!IsOpen()) return false;

    WLock();

    _ReadBulkHead(&bulkHead);

    MaskFlag(bulkHead, eBFStateRClose);

    _WriteBulkHead(&bulkHead);

    UnLock();

    return true;
}

bool iBulkFile::CloseReadWrite(void)
{
    BULK_HEAD bulkHead;

    if (!IsOpen()) return false;

    WLock();

    _ReadBulkHead(&bulkHead);

    MaskFlag(bulkHead, eBFStateRClose);
    MaskFlag(bulkHead, eBFStateWClose);

    _WriteBulkHead(&bulkHead);

    UnLock();

    return true;
}


bool iBulkFile::Write(uint32 nType, NPSTR pVal, uint32 nSize)
{
    theBulkLog.Log(ILOG_ERR, "size:%d, data:%s", nSize, pVal);
    char *pTmp;
    BulkItem* pBulkItem;
    bool bOk;

    if (nSize > BULKITEM_MAX_SIZE)
    {
        SetError(eBFErrBulkItemSizeOver);
        return false;
    }

    SetError(eBFSuccess);

    pTmp = new char [nSize + sizeof(BulkItem)];
    pBulkItem = (BulkItem*) pTmp;

    if (pTmp == NULL)
    {
        SetError(eBFErrMallocFail);
        return false;
    }

    pBulkItem->SetData(nType, pVal, nSize);
    pBulkItem->SetState(eBIStateInit);

    mDupLock.Lock();

    bOk = WriteItem(pBulkItem);

    mDupLock.Unlock();

    delete [] pTmp;

    return bOk;
}

bool iBulkFile::WriteItem(BulkItem* pItem)
{
    BULK_HEAD bulkHead;

    if (!IsOpen())
    {
        if (!OpenWriteBulkFile())
        {
            theBulkLog.Log(ILOG_ERR, "%s/%d: Data Writing Error.", __FUNCTION__, __LINE__);
            return false;
        }
    }

    // 파일이 제거 되었다면 파일 닫기
    if (!ExistFile(szOpenFileName))
    {
        SetError(eBFErrBulkFileRemoved);
        Close();
        theBulkLog.Log(ILOG_WARN, "%s/%d: Detected Write File Removed. filename=%s", __FUNCTION__, __LINE__, szOpenFileName);
        return false;
    }

    WLock();

    _ReadBulkHead(&bulkHead);

    // 날짜가 변경되어 벌크파일 다시생성
    if (!IsValidDate(&bulkHead))
    {
        UnLock();
        CloseWrite();
        Close();

        return WriteItem(pItem);
    }

    // 블록이 찻다며 블록을 확장하거나 신규벌크파일을 생성한다.
    if (IsBlockFull(bulkHead, pItem))
    {
        UnLock();

        if (!ExtendBulkFile())
        {
            CloseWrite();
            Close();
        }

        return WriteItem(pItem);
    }

    _WriteBulkItem(&bulkHead, pItem);

    // 헤더정보 쓰기
    bulkHead.nTotCnt++;
    bulkHead.nTotLen += pItem->GetSize();

    _WriteBulkHead(&bulkHead);

    UnLock();

    return true;
}

bool iBulkFile::ReadComplete(void)
{
    return ReadStateMark(eBIStateComplete);
}

bool iBulkFile::ReadFail(void)
{
    return ReadStateMark(eBIStateFail);
}

bool iBulkFile::ReadStateMark(uint32 nState)
{
    BULK_HEAD bulkHead;
    BulkItem  bulkItem;

    SetError(eBFSuccess);

    if (!IsOpen())
    {
        SetError(eBFErrBulkFileNotOpen);
        return false;
    }

    WLock();

    _ReadBulkHead(&bulkHead);

    _ReadBulkItem(&bulkHead, &bulkItem, 0);

    // Item 상태 변경
    bulkItem.SetState(nState);

    _ReWriteBulkItem(&bulkHead, &bulkItem);
    // 헤더부 기록
    bulkHead.nRSeq  = bulkItem.GetSeq()+1;
    bulkHead.nRIdx += bulkItem.GetSize();

    _WriteBulkHead(&bulkHead);

    UnLock();

    return true;
}

bool iBulkFile::PreReadComplete(void)
{
    BULK_HEAD bulkHead;

    SetError(eBFSuccess);

    if (!IsOpen())
    {
        SetError(eBFErrBulkFileNotOpen);
        return false;
    }

    WLock();

    _ReadBulkHead(&bulkHead);

    if (bulkHead.nRIdx == nPreReadIdx)
    {
        UnLock();
        return false;
    }

    // 헤더부 기록
    bulkHead.nRSeq = nPreReadSeq;
    bulkHead.nRIdx = nPreReadIdx;

    _WriteBulkHead(&bulkHead);

    UnLock();

    // 벌크파일내에 읽을 대상이 없고, 쓰기상태가 Close되면,
    // 읽기상태를 Close하고, 파일을 닫는다.
    if (bulkHead.nRSeq > bulkHead.nTotCnt)
    {
        if (IsWriteClose(bulkHead))
        {
            CloseRead();
            Close();
            return true;
        }

        // 날짜가 변경되어 벌크파일 읽기/쓰기 종료
        if (!IsValidDate(&bulkHead))
        {
            CloseReadWrite();
            Close();
            return true;
        }
    }

    return true;
}



int32 iBulkFile::Read(uint32& nSeq, uint32& nType, NPSTR pVal, uint32 nSize)
{
    char *pTmp;
    BulkItem* pBulkItem;
    uint32 nRLen;

    SetError(eBFSuccess);

    *pVal = '\0';

    pTmp = new char [nSize + sizeof(BulkItem)];
    if (pTmp == NULL)
    {
        SetError(eBFErrMallocFail);
        return 0;
    }

    memset(pTmp, 0x00, nSize + sizeof(BulkItem));
    pBulkItem = (BulkItem*) pTmp;

    mDupLock.Lock();

    if (!ReadItem(pBulkItem, nSize))
    {
        mDupLock.Unlock();
        delete [] pTmp;
        return 0;
    }

    nSeq  = pBulkItem->GetSeq();
    nType = pBulkItem->GetCode();
    nRLen = pBulkItem->GetDataLen();

    memcpy(pVal, pBulkItem->GetVal(), nRLen);

    mDupLock.Unlock();

    delete [] pTmp;

    return nRLen;
}

int32 iBulkFile::PreRead(uint32& nSeq, uint32& nType, NPSTR pVal, uint32 nSize)
{
    char *pTmp;
    BulkItem* pBulkItem;
    uint32 nRLen;

    SetError(eBFSuccess);

    *pVal = '\0';

    pTmp = new char [nSize + sizeof(BulkItem)];
    if (pTmp == NULL)
    {
        SetError(eBFErrMallocFail);
        return 0;
    }

    memset(pTmp, 0x00, nSize + sizeof(BulkItem));
    pBulkItem = (BulkItem*) pTmp;

    mDupLock.Lock();

    if (!PreReadItem(pBulkItem, nSize))
    {
        mDupLock.Unlock();
        delete [] pTmp;
        return 0;
    }

    nSeq  = pBulkItem->GetSeq();
    nType = pBulkItem->GetCode();
    nRLen = pBulkItem->GetDataLen();

    memcpy(pVal, pBulkItem->GetVal(), nRLen);

    mDupLock.Unlock();

    delete [] pTmp;

    return nRLen;
}


bool iBulkFile::ReadItem(BulkItem* pItem, uint32 nBufSize)
{
    BULK_HEAD bulkHead;

    memset(&bulkHead, 0x00, sizeof(bulkHead));

    if (!IsOpen())
    {
        if (!OpenReadBulkTodayFile())
        {
            if (IsOldFileChkTime() && OpenReadBulkOldFile())
            {
                return ReadItem(pItem, nBufSize);
            }

            SetError(eBFInfoReadBulkFileNotFound);
            return false;
        }
    }

    // 파일이 제거 되었다면 파일 닫기
    if (!ExistFile(szOpenFileName))
    {
        SetError(eBFErrBulkFileRemoved);
        Close();
        theBulkLog.Log(ILOG_WARN, "%s/%d: Detected Read File Removed. filename=%s", __FUNCTION__, __LINE__, szOpenFileName);
        return false;
    }

    WLock();

    _ReadBulkHead(&bulkHead);

    // 벌크파일내에 읽을 대상이 없다면, 쓰기상태가 Close되면,
    // 읽기상태를 Close하고, 다음 벌크파일을 읽는다.
    if (bulkHead.nRSeq > bulkHead.nTotCnt)
    {
        UnLock();

        if (IsWriteClose(bulkHead))
        {
            CloseRead();
            Close();
            return ReadItem(pItem, nBufSize);
        }

        // 날짜가 변경되어 벌크파일 읽기 종료
        if (!IsValidDate(&bulkHead))
        {
            SetError(eBFInfoDateChange);
            CloseReadWrite();
            Close();
            return false;
        } else {
            if (IsOldFileChkTime() && OpenReadBulkOldFile())
            {
                return ReadItem(pItem, nBufSize);
            }
        }

        SetError(eBFInfoReadNoData);

        return false;
    }

    _ReadBulkItem(&bulkHead, pItem, nBufSize);

    tOldFileChk = time(NULL) + nOldFileChkDur;

    UnLock();

    return true;
}

bool iBulkFile::PreReadItem(BulkItem* pItem, uint32 nBufSize)
{
    BULK_HEAD bulkHead;
    char sOldFileName[FILE_PATH_MAX];

    memset(&bulkHead, 0x00, sizeof(bulkHead));

    if (!IsOpen())
    {
        if (!OpenReadBulkTodayFile())
        {
            if (IsOldFileChkTime() && OpenReadBulkOldFile())
            {
                return PreReadItem(pItem, nBufSize);
            }

            SetError(eBFInfoReadBulkFileNotFound);
            return false;
        }
    }

    // 파일이 제거 되었다면 파일 닫기
    if (!ExistFile(szOpenFileName))
    {
        Close();

        SetError(eBFErrBulkFileRemoved);
        theBulkLog.Log(ILOG_WARN, "%s/%d: Detected Read File Removed. filename=%s", __FUNCTION__, __LINE__, szOpenFileName);
        return false;
    }

    WLock();

    _ReadBulkHead(&bulkHead);

    bulkHead.nRSeq = nPreReadSeq;
    bulkHead.nRIdx = nPreReadIdx;

    // 벌크파일내에 읽을 대상이 없다면, 쓰기상태가 Close되면,
    // 읽기상태를 Close하고, 다음 벌크파일을 읽는다.
    if (bulkHead.nRSeq > bulkHead.nTotCnt)
    {
        UnLock();

        if (IsWriteClose(bulkHead))
        {
            SetError(eBFInfoReadNoData);
            return false;
        }

        // 날짜가 변경되어 벌크파일 읽기 종료
        if (!IsValidDate(&bulkHead))
        {
            SetError(eBFInfoDateChange);
            return false;
        } else {
            if (IsOldFileChkTime() && GetBulkOldFileName(sOldFileName))
            {
                SetError(eBFInfoBulkOldFileChk);
                return false;
            }
        }

        SetError(eBFInfoReadNoData);
        return false;
    }

    _ReadBulkItem(&bulkHead, pItem, nBufSize);

    nPreReadSeq  = pItem->GetSeq()+1;
    nPreReadIdx += pItem->GetSize();

    tOldFileChk = time(NULL) + nOldFileChkDur;

    UnLock();

    return true;
}


#ifdef IWINDOWS

bool iBulkFile::CheckBulkFiles(NPCSTR szFileName)
{
	HANDLE hList;
	WIN32_FIND_DATA fileData;

    char sFileName[FILE_PATH_MAX];
    char sFullFileName[FILE_PATH_MAX];
    uint32 nLen;
    BULK_HEAD bulkHead;

    if (szFileName != NULL)
    {
        if (CheckOpenFile(sFullFileName, &bulkHead))
        {
            theBulkLog.Log(ILOG_INFO, "BULK-FILE-DUMP : filename=%s", sFullFileName);
            DbgBulkHead(&bulkHead);
        }

        return true;
    }

    hList = FindFirstFile(szBasePath, &fileData);
    if(hList == INVALID_HANDLE_VALUE) return false;

    do
    {
        nLen = snprintf(sFileName, sizeof(sFileName), "%s", fileData.cFileName);
        if (sFileName[0] == '.') continue;
        if (strcmp(&sFileName[nLen-4], ".blk") != 0) continue;

        snprintf(sFullFileName, sizeof(sFullFileName), "%s/%s", szBasePath, sFileName);

        if (CheckOpenFile(sFullFileName, &bulkHead))
        {
            theBulkLog.Log(ILOG_INFO, "BULK-FILE-DUMP : filename=%s", sFullFileName);
            DbgBulkHead(&bulkHead);
        }
    } while(FindNextFile(hList, &fileData));

    FindClose(hList);

    return true;
}

#else

bool iBulkFile::CheckBulkFiles(NPCSTR szFileName)
{
    DIR *dp;
    struct dirent *dir_entry;
    char sFileName[FILE_PATH_MAX];
    char sFullFileName[FILE_PATH_MAX];
    uint32 nLen;
    BULK_HEAD bulkHead;

    if (szFileName != NULL)
    {
        if (CheckOpenFile(sFullFileName, &bulkHead))
        {
            theBulkLog.Log(ILOG_INFO, "BULK-FILE-DUMP : filename=%s", sFullFileName);
            DbgBulkHead(&bulkHead);
        }

        return true;
    }

    dp = opendir(szBasePath);
    if (dp == NULL) return false;

    while ((dir_entry = readdir(dp)) != NULL)
    {
        nLen = snprintf(sFileName, sizeof(sFileName), "%s", dir_entry->d_name);
        if (sFileName[0] == '.') continue;
        if (strcmp(&sFileName[nLen-4], ".blk") != 0) continue;

        snprintf(sFullFileName, sizeof(sFullFileName), "%s/%s", szBasePath, sFileName);

        if (CheckOpenFile(sFullFileName, &bulkHead))
        {
            theBulkLog.Log(ILOG_INFO, "BULK-FILE-DUMP : filename=%s", sFullFileName);
            DbgBulkHead(&bulkHead);
        }
    }

    closedir(dp);

    return true;
}

#endif

bool iBulkFile::CheckOpenFile(NPCSTR sFileName, BULK_HEAD* pHead)
{
    FILE* fHd;
    bool bOk = false;

    fHd = fopen(sFileName, "r+b");
    if (fHd == NULL) return false;

    fseek(fHd, 0L, SEEK_SET);
    fread(pHead, sizeof(*pHead), 1, fHd);

    bOk = true;
    bOk = (strcmp(pHead->sVer, BULK_VER) == 0);

    fclose(fHd);

    return bOk;
}

void iBulkFile::GetTimeStr(time_t tVal, NPSTR szBuf, uint32 nSize)
{
    struct tm _tm;

    LocalTime(&tVal, &_tm);

    snprintf(szBuf, nSize, "%04d-%02d-%02d %02d:%02d:%02d"
            , _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday
            , _tm.tm_hour, _tm.tm_min, _tm.tm_sec);

    return;
}




bool iBulkFile::OpenBulkFile(NPCSTR sFileName)
{
    BULK_HEAD bulkHead;
    FILE* fHd;
    bool bOk = false;
    int fd;

    fHd = fopen(sFileName, "r+b");
    if (fHd == NULL)
    {
        theBulkLog.Log(ILOG_WARN, "%s/%d: File Open Fail. filename=%s", __FUNCTION__, __LINE__, sFileName);
        return false;
    }

    fd = fileno(fHd);

    lseek(fd, 0L, SEEK_SET);
    ::Read(fd, &bulkHead, sizeof(bulkHead));

    bOk = true;
    bOk = (strcmp(bulkHead.sVer, BULK_VER) == 0);

    // 열기실패하는 경우 다음 벌크파일 생성
    if (!bOk)
    {
        fclose(fHd);
        return false;
    }

    snprintf(szOpenFileName, sizeof(szOpenFileName), "%s", sFileName);

    SetHd(fHd);

    theBulkLog.Log(ILOG_INFO, "%s/%d: Bulk Read File Open. filename=%s", __FUNCTION__, __LINE__, sFileName);
    DbgBulkHead(&bulkHead);

    return true;
}

bool iBulkFile::GetHead(BULK_HEAD *pHead)
{
    bool bOk;

    RLock();

    bOk = _ReadBulkHead(pHead);

    UnLock();

    return bOk;
}

bool iBulkFile::SetHeadReadSeq(uint32 nRSeq)
{
    bool bOk = false;
    int fd;
    uint32 nPos;
    uint32 nSeq;
    BULK_HEAD bulkHead;
    BulkItem  bulkItem;

    if (!IsOpen()) return false;

    fd = fileno(GetHd());

    WLock();

    bOk = _ReadBulkHead(&bulkHead);
    if (!bOk)
    {
        UnLock();
        return false;
    }

    if (bulkHead.nTotCnt < nRSeq-1)
    {
        UnLock();
        return false;
    }

    nPos = sizeof(BULK_HEAD);
    nSeq = 1;

    while (nSeq < nRSeq)
    {
        lseek(fd, nPos, SEEK_SET);
        ::Read(fd, &bulkItem, sizeof(bulkItem));

        nSeq = bulkItem.GetSeq()+1;
        nPos += bulkItem.GetSize();
    }

    bulkHead.nRSeq = nSeq;
    bulkHead.nRIdx = nPos;
    if (bulkHead.nTotCnt < nRSeq)
        MaskFlag(bulkHead, eBFStateRClose);
    else
        UnMaskFlag(bulkHead, eBFStateRClose);

    bOk = _WriteBulkHead(&bulkHead);

    UnLock();

    return true;
}

void iBulkFile::CloseBulkFile(void)
{
    Close();
}



uint32 iBulkFile::FirstRead(void)
{
    return sizeof(BULK_HEAD);
}

bool iBulkFile::NextRead(uint32& rPos, BulkItem* pItem, int32 nBufSize)
{
    BULK_HEAD head;

    if (!GetHead(&head)) return false;

    if (rPos == 0 || head.nTotLen <= rPos) return false;

    head.nRIdx = rPos;

    if (!_ReadBulkItem(&head, pItem, nBufSize)) return false;

    head.nRSeq  = pItem->GetSeq()+1;
    head.nRIdx += pItem->GetSize();

    rPos = head.nRIdx;

    return true;

}



int iBulkFile::UnitTest(int argc, char const *argv[])
{
	//ctl.Init("/bridgetec/ipronv32/ie/data", "blkTest");

	if (argc == 2 && strcmp(argv[1], "w") == 0)
	{
		iBulkFile ctl;
        char sTemp[1024];

		ctl.Init(".", ".", "blkTest", 1024*10, 1024);

		for (int i = 0; i < 200; i++)
		{
            sprintf(sTemp, "%05dabcdef12345678901234567890", i);

			if (ctl.Write(0, sTemp, strlen(sTemp)) == false)
            {
                switch (ctl.GetError())
                {
                    case eBFErrMallocFail:              // 임시 메모리 할당 실패
                    case eBFErrBulkCreateFail:          // 벌크파일 생성에 실패하였음. (디렉토리가 없거나, Disk Full인경우)
                    case eBFErrBulkExtendFail:          // 벌크파일 확장에 실패하였음. (Disk Full인경우)
                    case eBFErrBulkFileRemoved:         // 현재 오픈된 벌크파일이 제거되었음.
                    case eBFErrWriteBulkFileOpenFail:   // 쓰기용 벌크파일 오픈 실패.
                    default:
                        break;
                }
            }
		}

		return 0;
	}

	if (argc == 2 && strcmp(argv[1], "r") == 0)
	{
		iBulkFile ctl;
		uint32 nSeq;
		uint32 nCode;
		char sBuf[128];

		ctl.Init(".", ".", "blkTest");

		while (1)
		//for (int i = 0; i < 10; i++)
		{
			memset(sBuf, 0x00, sizeof(sBuf));

			if (ctl.Read(nSeq, nCode, sBuf, sizeof(sBuf)) == 0)
            {
                switch (ctl.GetError())
                {
                    case eBFInfoDateChange:              // 오픈된 벌크파일날짜가 현재 날짜와 다름.
                    case eBFErrBulkFileRemoved:          // 현재 오픈된 벌크파일이 제거되었음.
                        break;
                    case eBFInfoReadBulkFileNotFound:    // 읽기용 벌크파일이 없어 파일오픈을 못함.
                    case eBFInfoReadNoData:              // 벌크파일내 읽기용 데이터가 더이상 없음.
                    default:
                        SleepSec(1);
                        break;
                }
                continue;
            }
			printf("%d: %s\n", nSeq, sBuf);
            ctl.ReadComplete();
		}

		return 0;
	}

    if (argc == 2 && strcmp(argv[1], "pre") == 0)
    {
#define COMMIT_CNT 10

        iBulkFile ctl;
        uint32 nSeq;
        uint32 nCode;
        char sBuf[128];
        int32 nCnt;

        ctl.Init(".", ".", "blkTest");

        nCnt = 0;

        while (1)
        {
            memset(sBuf, 0x00, sizeof(sBuf));

            if (ctl.PreRead(nSeq, nCode, sBuf, sizeof(sBuf)) == 0)
            {
                switch (ctl.GetError())
                {
                    case eBFErrBulkFileRemoved:          // 현재 오픈된 벌크파일이 제거되었음.
                        break;
                    case eBFInfoDateChange:              // 오픈된 벌크파일날짜가 현재 날짜와 다름.
                    case eBFInfoReadNoData:              // 벌크파일내 읽기용 데이터가 더이상 없음.
                        if (nCnt > 0)
                        {
                            ctl.PreReadComplete();
                            nCnt = 0;
                        }
                        SleepSec(1);
                        break;
                    case eBFInfoBulkOldFileChk:
                        if (nCnt > 0)
                        {
                            ctl.PreReadComplete();
                            nCnt = 0;
                        }
                        ctl.OpenReadBulkOldFile();

                        break;
                    case eBFInfoReadBulkFileNotFound:    // 읽기용 벌크파일이 없어 파일오픈을 못함.
                        SleepSec(1);
                        break;
                    default:
                        SleepSec(1);
                        break;
                }
                continue;
            }

            printf("%d: %s\n", nSeq, sBuf);

            nCnt++;

            if (nCnt >= COMMIT_CNT)
            {
                ctl.PreReadComplete();
                nCnt = 0;
            }

        }

        return 0;
    }

	if (argc == 2 && strcmp(argv[1], "chk") == 0)
	{
		iBulkFile ctl;

		ctl.Init(".", ".", "blkTest");
		ctl.CheckBulkFiles();
	}

	return 0;
}

