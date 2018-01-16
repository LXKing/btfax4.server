#if !defined(AFX_FAXCHTHREAD_H__E8201964_5728_4FBE_AB5D_44260D2A7936__INCLUDED_)
#define AFX_FAXCHTHREAD_H__E8201964_5728_4FBE_AB5D_44260D2A7936__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FaxChThread.h : header file
//


// -------------------------------------------------------------------
// System Header File
// -------------------------------------------------------------------
#include "afxmt.h"
#include <time.h>

// -------------------------------------------------------------------
// Application Header File
// -------------------------------------------------------------------
#include "enum.h"
#include "ThreadBase.h"
#include "DbModule.h"


/////////////////////////////////////////////////////////////////////////////
// CFaxChThread thread

class CFaxChThread : public CThreadBase
{
// method
public:
	CFaxChThread();
	virtual ~CFaxChThread();

	bool	FaxChInit( int chidx, int chnum );
	void	GetFaxChNum(int &chidx, int &chnum);
	
	void	RequestOutbound( CDbModule::SEND_REQ* p_pSendReq );

// Attributes
public:
	int					m_chnum;						// 0 Based FAX CH NO.
	int					m_chidx;						// g_FaxChInfo Array¿« Index 
	CCriticalSection	m_Sync;
	CDbModule::SEND_REQ* m_pSendReq;

// ovveride
private:
	virtual void onThreadEntry();

// implement
private:
	void				InitFaxChInfo();
	void				IdleFaxChInfo();
	void				SetShmState(EN_STATUS ChState);
	void				SetFaxChState( EN_STATUS ChState, const char* szStatusMsg, const char* szLogMsg );
	void				SetFaxChState( EN_STATUS enStatus, LPCSTR szFaxId, LPCSTR szSeq, LPCSTR szFaxNo, int nTryCnt, RESULT result = EMPTY );
	void				SetFaxSendInfo(CDbModule::SEND_REQ* pSendReq);
	void				SetFaxResult(RESULT result);
	RESULT				Fax_DialAndSend( CDbModule::SEND_REQ& sendReq, bool* p_pbWaitAbort );
	RESULT				SendT38( const char* szRemoteIp, int nRemotePort, CDbModule::SEND_REQ& sendReq, const char* p_szPages );
	void				UpdateResult(CDbModule::SEND_REQ& sendReq, RESULT result);
    RESULT				GetSIPResult(int nSipReasonCode);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FAXCHTHREAD_H__E8201964_5728_4FBE_AB5D_44260D2A7936__INCLUDED_)
