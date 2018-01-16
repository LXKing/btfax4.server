/**
 *  
 *  iBulkFile.h
 *  iLib
 *
 *  Created by chiwoo on 13. 08. 28.
 *  Created Version : IPRON v3.2.0
 *  Copyright 2010 Bridgetec. All rights reserved.
 *
 **/

#pragma once

#include <iType.h>
#include <iMutex.h>

#define OLD_FILE_CHK_DUR        (60)

#define FILE_PATH_MAX 256
#define BULKITEM_MAX_SIZE       (24*1024)  // 24 KByte

#define MAX_BULK_SEQ            (100)
#define FILE_BLOCK_SIZE  (5*1024*1024)           // 5MByte
#define FILE_BLOCK_MAX_SIZE  (100*1024*1024)           // 5MByte
#define FILE_MAX_SIZE    (FILE_BLOCK_SIZE*100)   // 500MByte

#define MAX_LOG_SIZE    (300 * 1000 * 1000)

enum eBulkFileState
{
    eBFStateInit   = 0x00,
    eBFStateUsing  = 0x01,
    eBFStateRClose = 0x04,
    eBFStateWClose = 0x08,
};

enum eBulkItemState
{
    eBIStateInit = 0x00,
    eBIStateComplete = 0x01,
    eBIStateFail = 0x02,
};

enum eBulkFileErr
{
    eBFErrBulkFileNotOpen = -12,        // (RW) 현재 오픈된 벌크파일이 없음. 
    eBFErrBulkItemSizeOver = -11,       // (W) 쓰기요청 Item 크기 초과
    eBFErrMallocFail = -10,             // (RW) 임시 메모리 할당 실패
    eBFErrBulkCreateFail = -9,          // (W) 벌크파일 생성에 실패하였음. (디렉토리가 없거나, Disk Full인경우)
    eBFErrBulkExtendFail = -8,          // (W) 벌크파일 확장에 실패하였음. (Disk Full인경우)
    eBFErrBulkFileRemoved = -7,         // (RW) 현재 오픈된 벌크파일이 제거되었음. 
    eBFErrWriteBulkFileOpenFail = -6,   // (W) 쓰기용 벌크파일 오픈 실패.
    eBFSuccess = 0,
    eBFInfoReadBulkFileNotFound = 1,    // (R) 읽기용 벌크파일이 없어 파일오픈을 못함.
    eBFInfoDateChange = 2,              // (R) 오픈된 벌크파일날짜가 현재 날짜와 다름.
    eBFInfoReadNoData = 3,              // (R) 벌크파일내 읽기용 데이터가 더이상 없음.
    eBFInfoBulkExtendFull = 4,          // (W) 벌크파일 확장이 최대치까지 증가된 경우
    eBFInfoBulkOldFileChk = 5,          // (R) 현재파일처리 읽기완료상태에서 이전파일이 존재하는 경우.
};

#define IsReadClose(x) ((x.nFileState&eBFStateRClose) != 0)
#define IsWriteClose(x) ((x.nFileState&eBFStateWClose) != 0)

#define UnMaskFlag(x, flag) (x.nFileState &= ~flag )
#define MaskFlag(x, flag) (x.nFileState |= flag)

#pragma once

#define BULK_VER ("Bulk_v1")

#ifdef IWINDOWS
	#pragma pack(push,1)
#else
	PRAGMA_PACK_BEGIN(1)
#endif

typedef struct
{
    char   sVer[8];
    uint8  nFileState;  // eBulkFileState 참조
    uint64 nCreateTime;
    uint32 nBlockMaxCnt; // 확장가능 최대 블럭 수
    uint32 nBlockSize;   // 1-Block Size
    uint32 nBlockCnt;    // 현재 블럭수
    uint32 nTotLen;      // 전체 길이
    uint32 nTotCnt;      // 전체 개수
    uint32 nRSeq;        // 다음 읽을 순번
    uint32 nRIdx;        // 다음 읽을 위치
} BULK_HEAD;

#define IsBlockFull(x, y) ((x.nTotLen-sizeof(x) + y->GetSize()) > (x.nBlockSize * x.nBlockCnt))

#ifdef IWINDOWS
	#pragma pack(pop)
#else
	PRAGMA_PACK_END
#endif

#ifdef IWINDOWS
	#pragma pack(push,1)
#else
	PRAGMA_PACK_BEGIN(1)
#endif

class BulkItem
{
public:
    uint32 nSeq;
    uint32 nCode;
    uint32 nState;  // eBulkItemState 참조
    uint16 nLen;    // 항목 길이 : Sub 해더부포함
    char   pValue[0];

public:
    inline NPCSTR GetVal(void) { return pValue; };
    inline uint16 GetSize(void) { return nLen; };
    inline uint16 GetDataLen(void) { return (nLen - sizeof(BulkItem)); };
    inline uint32 GetCode(void) { return nCode; };
    inline uint32 GetSeq(void) { return nSeq; };
    inline void   SetSeq(uint32 nSeq) { this->nSeq = nSeq; };
    inline void   SetState(uint32 nState) { this->nState = nState; };
    inline uint32 GetState(void) { return nState; };

    inline void SetCode(uint32 nVal) { nCode = nVal; };
    inline void SetData(NPVOID pVal, uint16 nDLen)
    {
        memcpy(pValue, pVal, nDLen);
        nLen = nDLen + sizeof(BulkItem);
    }
    void SetData(uint32 nCode, NPVOID pVal, uint16 nDLen)
    {
        SetCode(nCode);
        SetData(pVal, nDLen);
    }
};

#ifdef IWINDOWS
	#pragma pack(pop)
#else
	PRAGMA_PACK_END
#endif



class iBulkFile 
{
public:
	static iBulkFile* Inst();
protected:
	static iBulkFile* s_pInstance;

private:
    iMutex mLock;
    iMutex mDupLock;

    char szBasePath[FILE_PATH_MAX];
    char szFilePrefix[FILE_PATH_MAX];
    char szOpenFileName[FILE_PATH_MAX];
    uint32 nBulkMaxSize;
    uint32 nBulkBlockSize;
    time_t tOldFileChk;
    uint16 nOldFileChkDur;

    uint32 nPreReadSeq;
    uint32 nPreReadIdx;

    uint32 nErrCode;

    //BulkItem mLastBulkItem;
    FILE *blkHd;

private:
    /**
     *  벌크파일 헨들러 처리
     **/
    FILE* GetHd(void) { return blkHd; };
    void SetHd(FILE* hd) { blkHd = hd; };
    bool IsOpen(void) { return (GetHd() != NULL); };
    void Close(void);
    void SetError(uint32 nCode) { nErrCode = nCode; };

    /**
     *  벌크파일 Lock 처리
     **/
    bool RLock(FILE *fHd = NULL);
    bool WLock(FILE *fHd = NULL);
    bool UnLock(FILE *fHd = NULL);

    /**
     *  벌크파일 초기 생성 및 크기 확장 처리
     **/
    bool InitBulkFile(void);
    bool ExtendBulkFile(void);

    /**
     *  벌크파일 처리를 위한 각종 유틸리티 함수
     **/
    bool IsValidDate(BULK_HEAD* pHead);
    bool ExistFile(NPCSTR szFileName);
    void GetTimeStr(time_t tVal, NPSTR szBuf, uint32 nSize);
    bool IsOldFileChkTime(void);
    NPCSTR GetTodayStr(NPSTR szBuf, uint32 nBufSize);
    NPCSTR GetFileNameStr(uint16 nSeq, NPSTR szBuf, uint32 nBufSize);

    bool GetBulkTodayFileName(NPSTR sFullFileName);
    bool GetBulkOldFileName(NPSTR sFullFileName);
    bool ChkReadBulkFile(NPCSTR sFileName);

    /**
     *  벌크파일 읽기/쓰기용으로 파일 열기
     **/
    bool OpenWriteBulkFile(void);
    bool OpenReadBulkTodayFile(void);
    bool OpenReadBulkFile(NPCSTR sFileName);

    int16 GetLastBulkFileSeq(void);
    bool OpenLastBulkFile(void);

    /**
     *  헤더부에 읽기/쓰기 완료 상태 플래그 기록
     **/
    bool CloseWrite(void);
    bool CloseRead(void);
    bool CloseReadWrite(void);

    /**
     *  BulkItem 데이터를 읽기/쓰기
     **/
    bool WriteItem(BulkItem* pItem);
    bool ReadItem(BulkItem* pItem, uint32 nBufSize);
    bool ReadStateMark(uint32 nState);

    bool PreReadItem(BulkItem* pItem, uint32 nBufSize);

    /**
     *  파일핸들러에 실제 쓰기/읽기 동작 수행
     **/
    bool _ReadBulkHead(BULK_HEAD* pHead);
    bool _WriteBulkHead(BULK_HEAD* pHead);
    bool _WriteBulkItem(BULK_HEAD* pHead, BulkItem* pItem);
    bool _ReWriteBulkItem(BULK_HEAD* pHead, BulkItem* pItem);
    bool _ReadBulkItem(BULK_HEAD* pHead, BulkItem* pItem, uint32 nBufSize);

    void DbgBulkHead(BULK_HEAD* pHead);

    bool CheckOpenFile(NPCSTR sFileName, BULK_HEAD* pHead);

public:
    iBulkFile();
    ~iBulkFile();

    void Init(NPCSTR szLogPath, NPCSTR szBasePath, NPCSTR szFilePfx, uint32 nMaxSize = FILE_MAX_SIZE, uint32 nBlockSize = FILE_BLOCK_SIZE);
    void SetLogLvl(uint32 nLogLvl);
    void SetLogLvlPtr(uint32* pLogLvl);
    void SetOldFileChkDur(uint16 nDur);
    inline uint32 GetError(void) { return nErrCode; };

    bool Write(uint32 nType, NPSTR pVal, uint32 nSize);
    int32 Read(uint32& nSeq, uint32& nType, NPSTR pVal, uint32 nSize);
    bool ReadComplete(void);
    bool ReadFail(void);

    int32 PreRead(uint32& nSeq, uint32& nType, NPSTR pVal, uint32 nSize);
    bool PreReadComplete(void);

    bool OpenReadBulkOldFile(void);

    bool CheckBulkFiles(NPCSTR szFileName = NULL);

    /**
     *  Utility 지원 함수
     **/
    bool OpenBulkFile(NPCSTR szFileName);
    bool GetHead(BULK_HEAD *pHead);
    bool SetHeadReadSeq(uint32 nRSeq);
    void CloseBulkFile(void);
    uint32 FirstRead(void);
    bool  NextRead(uint32& rPos, BulkItem* pItem, int32 nBufSize);

	static int32 UnitTest(int argc, char const *argv[]);
};



