#if !defined(AFX_FASIPTHREAD_H__CC2F950E_1CDA_406D_A504_991EA779E131__INCLUDED_)
#define AFX_FASIPTHREAD_H__CC2F950E_1CDA_406D_A504_991EA779E131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FASIPThread.h : header file
//

#include "PacketDef.h"
#include "ThreadBase.h"
#include <vector>
#include <deque>

using namespace std;



struct CHAN_MSG
{
	FAX_SIP_HEADER	h;
	void*			pPacket;
};

struct CHAN_RECV_PACKET
{
	FAX_RECV_PACKET	packet;
	bool			bReceived;
};

/////////////////////////////////////////////////////////////////////////////
// CFsmIfThread thread

class CFsmIfThread : public CThreadBase
{	
public:
	static CFsmIfThread* Inst();
protected:
	static CFsmIfThread* s_pInstance;


// method
public:
	CFsmIfThread();
	virtual ~CFsmIfThread();
	
	bool	IsRegistered();

// Attributes
private:
	bool		m_bStart;								// CFsmIfThread Thread 구동여부

	SOCKET		m_hClient;								// FASIP와 연결된 Socket Handle

	SIP_STATUS	m_sip_status;							// CFsmIfThread Thread 상태

	time_t		m_tLastSend;								// 마지막 송신시간
	time_t		m_tLastRecv;								// 마지막 수신 시간

	CCriticalSection			m_csLock;
	deque< FAX_SEND_PACKET >	m_SndPackets;
	vector< CHAN_RECV_PACKET >	m_RcvPackets;
	int							m_chanBegin;
	

	// ovveride
private:
	virtual void onThreadEntry();

// implement
private:
	bool		HandleRcvPacket( const char* p_pBuffer, int p_len );

	bool		Connect();
	int			FASIPRegister();
	int			FASIPKeepalive();
	int			Send( const void* p_sPacket, int p_len );

	int			ReadPacket(SOCKET hInSocket, char *pszBuf4Recv, int nSize, int nTimeOut);
	int			ReadFASIP(SOCKET hInSocket, char *pszBuf4Recv);

	void		ReleaseResource(WSAEVENT hWasEvent);

public:
	bool		SndPacket_Push( int chan, FAX_SEND_PACKET& ChanMsg );
	bool		SndPacket_Pop( FAX_SEND_PACKET* pChanMsg );

	bool		RcvPacket_Init( int chanBegin, int chanCnt );
	bool		RcvPacket_Push( FAX_RECV_PACKET& FsmMsg );
	bool		RcvPacket_Pop( int chanNum, FAX_RECV_PACKET* pFsmMsg );
	bool		RcvPacket_Wait( int chanNum, int nWaitMilli, FAX_RECV_PACKET* pFsmMsg );
	bool		RcvPacket_Wait( int chanNum, int nMsgId, int nWaitMilli, FAX_RECV_PACKET* pFsmMsg );
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FASIPTHREAD_H__CC2F950E_1CDA_406D_A504_991EA779E131__INCLUDED_)
