#if !defined(AFX_FAXCHTHREAD_H__87E6E5C0_D864_4A88_B71A_3C860752CF7A__INCLUDED_)
#define AFX_FAXCHTHREAD_H__87E6E5C0_D864_4A88_B71A_3C860752CF7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FaxChThread.h : header file
//

// -------------------------------------------------------------------
// System Header File
// -------------------------------------------------------------------
#include "afxmt.h"
#include "enum.h"
#include "ThreadBase.h"

using namespace COMMON_LIB;

/////////////////////////////////////////////////////////////////////////////
// CFaxChThread thread

class CFaxChThread : public CThreadBase
{
// method
public:
	CFaxChThread();
	virtual ~CFaxChThread();

	void			InitChannel( int p_chidx, int p_chnum );
	void			GetFaxChNum(int &chidx, int &chnum);

// Attributes
private:
	bool				m_bStart;						// CFaxChThread Thread 구동여부

	int					m_chnum;						// 0 Based FAX CH NO.
	int					m_chidx;						// g_FaxChInfo Array의 Index 
	CCriticalSection	m_Sync;

	char				m_faxEvtName[32];				// FaxWaitEvent()에서 얻은 BTFAX Event 명

// ovveride
private:
	virtual void onThreadEntry();

// implement
private:
	bool				SetFaxChNum(int chidx, int chnum);
	void			    SetShmState(EN_STATUS ChState);
	void				SetFaxChState( EN_STATUS ChState, const char* szStatusMsg, const char* szLogMsg );
	void				SetFaxChState( EN_STATUS ChState, const char* szAni, const char* szDnis, const char* szSrcFile );
	void				InitFaxChInfo();
	void				IdleFaxChInfo();

	bool				GetTel( const char* pSipTel, CString* pstrPstnTel );
	bool				IsExistFileEx(LPCSTR lpszFileName, bool bDirectory);
	bool				IsExistFileEx(LPCSTR p_szBasePath, LPCSTR p_szSubPath, bool bDirectory);
	bool				CreateMultiDir(LPCSTR lpszPath);
	bool				CreateMultiDir(CString p_strBasePath, CString p_strSubPath);
	bool				WriteRecvTifInfo(int pagecnt, CString* pstrFaxId);
	int					InsertRecvTifInfo( const char* pAni, const char* pDnis, const char* pTifFile, int	nTifSize, int nPageCnt, CString* pstrFaxId );
	int					InsertRecvTifInfoEx( const char* pMgwIp, int pMgwPort, const char* pAni, const char* pDnis, const char* pTifFile, int	nTifSize, int nPageCnt, CString* pstrFaxId );

	int					UpdateRecvRunningState(CString pFaxId);
	int					UpdateRecvFinished(CString pstrFaxId, int pResult, int pReason, CString pTifFile, int pTifFileSize, int pTifPageCnt);

	char*				GetFaxMsgName(int msgid);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FAXCHTHREAD_H__87E6E5C0_D864_4A88_B71A_3C860752CF7A__INCLUDED_)
