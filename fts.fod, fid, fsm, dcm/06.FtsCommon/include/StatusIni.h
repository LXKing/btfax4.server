#pragma once

#include <afxmt.h>
#include "enum.h"
using namespace COMMON_LIB;

class CStatusIni
{
private:
	static CStatusIni*	s_pInstance;
public:
	static CStatusIni*	Inst();


public:
	CStatusIni();
	virtual ~CStatusIni();

	void Initialize( const char* p_szStatusFile, char p_chIO, int p_chanNumBegin, int p_chanNumCnt, int p_chanBegin, int p_chanEnd, int p_bchanBegin = -1, int p_bchanEnd = -1 );
	void Update( int p_chan, EN_STATUS p_status, const char* p_szStatusMsg, CString* pstrPrefix );

private:
	CString				m_strStatusFile;
	char				m_chIO;
	int					m_chanNumBegin;
	int					m_chanNumCnt;
	int					m_chanBegin;
	int					m_chanEnd;
	int					m_bchanBegin;
	int					m_bchanEnd;

	CCriticalSection	m_Lock;
	CString				m_strKey;
};
