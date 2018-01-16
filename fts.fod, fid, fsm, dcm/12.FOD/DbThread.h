#if !defined(AFX_ORATHREAD_H__A25C62FD_EDD8_4C3F_A04F_ECF5C94673AC__INCLUDED_)
#define AFX_ORATHREAD_H__A25C62FD_EDD8_4C3F_A04F_ECF5C94673AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OraThread.h : header file
//

#include "DbModule.h"
#include "ThreadBase.h"

#include <list>
using namespace std;




/////////////////////////////////////////////////////////////////////////////
// CDbPollingThread thread

class CDbPollingThread : public CThreadBase
{
public:
	static CDbPollingThread* Inst();
protected:
	static CDbPollingThread* s_pInstance;

public:
	CDbPollingThread();
	virtual ~CDbPollingThread();

// ovveride
private:
	virtual void onThreadEntry();

// operation
private:
	bool	StartOutbound( int chIdx, CDbModule::SEND_REQ& sendReq );
	bool	IsExistFileEx( LPCSTR lpszFileName, bool bDirectory );
	bool	CreateMultiDir( TCHAR* lpszPath );
	int		Get_Idle_ChIdx(bool bBroadcast, int cntMax, list<int>& vecChIdx);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ORATHREAD_H__A25C62FD_EDD8_4C3F_A04F_ECF5C94673AC__INCLUDED_)
