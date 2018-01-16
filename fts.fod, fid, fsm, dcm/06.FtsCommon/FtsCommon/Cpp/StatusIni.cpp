#include "StdAfx.h"
#include "StatusIni.h"



CStatusIni*	CStatusIni::s_pInstance = NULL;

CStatusIni*	CStatusIni::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new CStatusIni();

	return s_pInstance;
}


CStatusIni::CStatusIni()
{
	m_strStatusFile	= "";
	m_chIO			= ' ';
	m_chanNumBegin	= -1;
	m_chanNumCnt	= -1;
	m_chanBegin		= -1;
	m_chanEnd		= -1;
	m_bchanBegin	= -1;
	m_bchanEnd		= -1;
}

CStatusIni::~CStatusIni()
{
}

void CStatusIni::Initialize( const char* p_szStatusFile, char p_chIO, int p_chanNumBegin, int p_chanNumCnt, int p_chanBegin, int p_chanEnd, int p_bchanBegin, int p_bchanEnd )
{
	int		chan;
	CString strValue;
	
	
	m_strStatusFile	= p_szStatusFile;
	m_chIO			= p_chIO;
	m_chanNumBegin	= p_chanNumBegin;
	m_chanNumCnt	= p_chanNumCnt;
	m_chanBegin		= p_chanBegin;
	m_chanEnd		= p_chanEnd;
	m_bchanBegin	= p_bchanBegin;
	m_bchanEnd		= p_bchanEnd;

	
	strValue.Format( "%c", m_chIO );
	WritePrivateProfileString( "CHANNEL_STATUS", "IO", strValue, m_strStatusFile );

	strValue.Format( "%d", p_chanNumBegin );
	WritePrivateProfileString( "CHANNEL_STATUS", "CH_START", strValue, m_strStatusFile );

	strValue.Format( "%d", p_chanNumCnt );
	WritePrivateProfileString( "CHANNEL_STATUS", "CH_CNT", strValue, m_strStatusFile );


	for( chan = m_chanBegin ; chan <= m_chanEnd ; ++chan  )
		Update( chan, FAX_INIT, NULL, &strValue );

	if( m_chIO == 'O' )
	{
		for( chan = m_bchanBegin ; chan <= m_bchanEnd ; ++chan  )
			Update( chan, FAX_INIT, NULL, &strValue );
	}
}

void CStatusIni::Update( int p_chan, EN_STATUS p_status, const char* p_szStatusMsg, CString* pstrPrefix )
{
	CString strStatusLine;

	if( m_chanBegin <= p_chan && p_chan <= m_chanEnd )
		*pstrPrefix = "G:";
	else if( m_bchanBegin <= p_chan && p_chan <= m_bchanEnd )
		*pstrPrefix = "B:";

	switch( p_status )
	{
	case FAX_INIT:
		*pstrPrefix += "00:초기화  ";
		break;
	case FAX_IDLE:
		if( this->m_chIO == 'O' )
			*pstrPrefix += "01:송신대기";
		else
			*pstrPrefix += "11:수신대기";
		break;
	case FAX_OCCUPY:
		*pstrPrefix += "02:점유    ";
		break;
	case FAX_DIAL:
		*pstrPrefix += "02:다이얼  ";
		break;
	case FAX_SEND:
		*pstrPrefix += "02:송신중  ";
		break;
	case FAX_SUCC_SEND:
		*pstrPrefix += "05:송신완료";
		break;
	case FAX_FAIL_SEND:
		*pstrPrefix += "09:송신실패";
		break;
	case FAX_RECV:
		*pstrPrefix += "12:수신중  ";
		break;
	case FAX_SUCC_RECV:
		*pstrPrefix += "15:수신완료";
		break;
	case FAX_FAIL_RECV:
		*pstrPrefix += "19:수신실패";
		break;
	
	default :
		*pstrPrefix += "99:미정의  ";
		break;
	}

	if( p_szStatusMsg && strlen(p_szStatusMsg) > 0 )
		strStatusLine.Format( "%s:%s", (LPCSTR)*pstrPrefix, p_szStatusMsg );
	else
		strStatusLine = *pstrPrefix;
	
	{CSingleLock Locker( &m_Lock, TRUE );
		
		m_strKey.Format("CH_%03d", p_chan );
		WritePrivateProfileString( "CHANNEL_STATUS", m_strKey, strStatusLine, m_strStatusFile );
	}
}


