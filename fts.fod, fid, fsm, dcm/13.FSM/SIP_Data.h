#pragma once

#include "SharedMemoryUtility.h"


#define	CHANNEL_LIST					map<int, SIP_Session*>
#define CHANNEL_DIALOG_MAP				map< BTDialog*, int >

#define USER_REGISTER_MAP				map< BTRegister*, SIP_User* >
#define USER_REGISTER_MAP_ITERATOR		USER_REGISTER_MAP::iterator
#define USER_LIST						map< unsigned int, SIP_User* >
#define USER_LIST_ITERATOR				USER_LIST::iterator

#define MIN_SESSION_ID					100000001
#define MAX_SESSION_ID					999999999

#define CHAN_BEG						0
#define CHAN_CNT						100

#define MIN_USER_ID						1001
#define MAX_USER_ID						9999






class SIP_User
{
private:
	unsigned int 	m_nState;
	unsigned int 	m_nUserId;
	void*			m_pPoolId;
	BTMutex			m_nUserMutex;
	BTRegister*		m_pRegister;
	unsigned int	m_nExpires;
	char			m_strTo[128];

public:
	typedef enum USER_STATES
	{
		USER_STATE_IDLE,
		USER_STATE_TRYING,
		USER_STATE_REGISTERED,
		USER_STATE_RETRYING,
		USER_STATE_REJECT,
		NUM_OF_USER_STATE
	} SessionState_t;

	static const char const_UserState[NUM_OF_USER_STATE][64];
    /**
    * @brief    기본생성자
    *
    * @param    none
    * @return   none
    */
	SIP_User() {}

    /**
    * @brief    소멸자
    *
    * @param    none
    * @return   none
    */
	~SIP_User() {}

    /**
    * @brief    초기화
    */
	inline void initUser()
	{
		m_nState		= USER_STATE_IDLE;
		m_nUserId		= 0;
		m_pPoolId 		= NULL;
		m_pRegister		= NULL;
		m_strTo[0]		= '\0';
		m_nExpires		= 0;
	}

	/**
	* @brief	이벤트 발생 시각
	*/
	time_t			nEventTime;

    /**
    * @brief    상태 관리 
    */
	inline void setIdle() 			{ m_nState 	= USER_STATE_IDLE; 		}
	inline void setTrying() 		{ m_nState 	= USER_STATE_TRYING; 	}
	inline void setRegistered() 	{ m_nState 	= USER_STATE_REGISTERED;}
	inline void setRetrying() 		{ m_nState 	= USER_STATE_RETRYING; 	}
	inline void setReject() 		{ m_nState 	= USER_STATE_REJECT; 	}
	inline int state()
	{	
		return m_nState;
	}

    /**
    * @brief    인덱스 관리 
    */
	inline void setPoolId(void* pPoolId)
	{
		m_pPoolId 	= pPoolId;
	}
	inline void* getPoolId()
	{
		return m_pPoolId;
	}

	inline void setUserId(int nUserId)
	{
		m_nUserId 	= nUserId;
	}
	inline int getUserId()
	{
		return m_nUserId;
	}

    /**
    * @brief    다이얼로그 관리
    */
	inline void setRegister(BTRegister* pRegister)
	{
		m_pRegister = pRegister;
	}
	inline BTRegister* regi()
	{
		return m_pRegister;
	}

    /**
    * @brief    Address Of Record 관리
    */
	inline void setTo(char* strTo)
	{
		strcpy( m_strTo, strTo );
	}
	inline char* getTo()
	{
		return m_strTo;
	}

    /**
    * @brief    Expires 관리
    */
	inline void setExpires(int nExpires)
	{
		m_nExpires	= nExpires;
	}
	inline int getExpires()
	{
		return m_nExpires;
	}

	/**
	* @brief	메시지 처리
	*/
	int proceed(int nStatus);
	int accept(int nStatus);
	int reject(int nStatus);
};

class SIP_Session : public BTObject
{
public:
	static void getEmptyInfo( CString* p_strInfo )
	{
		*p_strInfo = "[CHAN_   ][D:          ][CC:            ][C:]";
	}

	void getInfo( CString* p_strInfo )
	{
		p_strInfo->Format( "[CHAN_%03d][D:0x%08X][CC:%12s][C:%s]", m_chan, m_pDialog, (LPCSTR)m_strClientCallId, (LPCSTR)m_strCallId );
	}

private:
	struct MGW_INFO
	{
		bool	bAllocated;
		bool	bPrimary;
		CString strIp;
		int		nPort;

		inline MGW_INFO()
		{
			Clear();
		}

		inline void Clear()
		{
			bAllocated	= false;
			bPrimary	= false;
			strIp		= "";
			nPort		= 0;
		}
	};

private:

	MGW_INFO			m_MgwInfo;
	CCriticalSection	m_Lock;

	int				m_chan;
	char			m_chDirection;
	int				m_statusCode;
	unsigned int 	m_nState;
	unsigned int 	m_nSessionId;
	unsigned int	m_nReferCSeq;
	void*			m_pPoolId;
	BTDialog*		m_pDialog;
	BTDialog*		m_pRefer;
	dev_state*		m_devState;
	CString			m_strCallId;
	CString			m_strClientCallId;
	

public:
	typedef enum SESSION_STATES
	{
		SESSION_STATE_IDLE,
		SESSION_STATE_HUNTED,
		SESSION_STATE_TRYING,
		SESSION_STATE_ESTABLISHED,
		SESSION_STATE_RETRYING,
		SESSION_STATE_REJECT,
		SESSION_STATE_TRANSFER,
		SESSION_STATE_BYE,
		NUM_OF_SESSION_STATE
	} SessionState_t;

	const static char const_SessionState[NUM_OF_SESSION_STATE][64];
	
    /**
    * @brief    기본생성자
    *
    * @param    none
    * @return   none
    */
	SIP_Session()								{ m_chDirection = ' '; }
	SIP_Session( BTDialog* pDialog, int chan )	{ initSession( pDialog, chan ); }

    /**
    * @brief    소멸자
    *
    * @param    none
    * @return   none
    */
	~SIP_Session() {}

    /**
    * @brief    초기화
    */
	inline void initSession( BTDialog* pDialog = NULL, int chan = 0 )
	{
		m_chDirection = ' '; 

		m_nState		= SESSION_STATE_IDLE;
		m_statusCode	= -1;
		m_nSessionId	= 0;
		m_nReferCSeq	= 0;
		m_pPoolId 		= NULL;
		m_pDialog		= NULL;
		m_pRefer		= NULL;
		m_strCallId		= "";
		m_strClientCallId = "";

		m_chan			= chan;
		m_devState		= shmem->d + m_chan;
	}

	inline char direction()
	{
		return m_chDirection;
	}

	inline void setDirection( char p_chDirection )
	{
		m_chDirection = p_chDirection;
	}

	bool HuntMgw( MGW_INFO* p_pMgwInfo );
	bool SetMgw( const char* p_szIp, int p_port );
	bool GetMgw( MGW_INFO* p_pMgwInfo );
	void ClearMgw();

	/**
	* @brief	이벤트 발생 시각
	*/
	time_t			nEventTime;

    /**
    * @brief    상태 관리 
    */
	inline void setChannel( int chan ) { m_chan = chan; }
	
	inline void setIdle() 			{ m_nState 	= SESSION_STATE_IDLE; 		m_devState->state = m_nState;	
									  m_statusCode = -1; 
									  setCallId(""); 
									  setClientCallId(""); 
									}
	inline void setHunted() 		{ m_nState 	= SESSION_STATE_HUNTED; 	m_devState->state = m_nState; }
	inline void setTrying() 		{ m_nState 	= SESSION_STATE_TRYING; 	m_devState->state = m_nState; }
	inline void setEstablished() 	{ m_nState 	= SESSION_STATE_ESTABLISHED;m_devState->state = m_nState; }
	inline void setRetrying() 		{ m_nState 	= SESSION_STATE_RETRYING; 	m_devState->state = m_nState; }
	inline void setReject() 		{ m_nState 	= SESSION_STATE_REJECT; 	m_devState->state = m_nState; }
	inline void setTransfer() 		{ m_nState 	= SESSION_STATE_TRANSFER; 	m_devState->state = m_nState; }
	inline void setBye() 			{ m_nState 	= SESSION_STATE_BYE; 		m_devState->state = m_nState; }
	inline int channel()			{ return m_chan;   }
	inline int state()				{ return m_nState; }

    /**
    * @brief    인덱스 관리 
    */
	inline void setPoolId(void* pPoolId)
	{
		m_pPoolId 	= pPoolId;
	}
	inline void* getPoolId()
	{
		return m_pPoolId;
	}

	inline void setSessionId(int nSessionId)
	{
		m_nSessionId 	= nSessionId;
	}
	inline int getSessionId()
	{
		return m_nSessionId;
	}
	inline void setSDPMode(int sdpMode)
	{
		m_devState->sdp_mode = sdpMode;
	}
	inline int sdpMode()
	{
		return m_devState->sdp_mode;
	}
	inline void* callLeg()
	{
		return m_pDialog->dialogID();
	}
	inline dev_state* devState()
	{
		return m_devState;
	}
	inline void ResetDevState()
	{
		memset( (char *)m_devState + 8, 0x00, sizeof(*m_devState) - 8 );
	}
	inline int fstate()
	{
		return m_devState->fstate;
	}
	inline void resetFstate()
	{
		m_devState->fstate = 0;
	}
	inline void incrementFstate()
	{
		++m_devState->fstate;
	}

	inline int statusCode() { return m_statusCode; }
	inline void setStatusCode( int statusCode ) { m_statusCode = statusCode; }

	// From			: SIP : G/W
	// To			: SIP : FSM
	// Remote		: Session Info
	// Remote Port	: Session Info
	inline void setSessionInfoInfo( const char* szFrom, const char* szTo, const char* szRemote, int remotePort )
	{	
		strcpy( m_devState->s.from,		szFrom		);
		strcpy( m_devState->s.to,		szTo		);
		strcpy( m_devState->s.remote_ip, szRemote	);
		m_devState->s.remote_port = remotePort;
	}


    /**
    * @brief    다이얼로그 관리
    */
	inline void setDialog(BTDialog* pDialog)
	{
		m_pDialog 		= pDialog;
		m_devState->leg	= (unsigned int)m_pDialog->dialogID();
	}
	inline BTDialog* dialog()
	{
		return m_pDialog;
	}

	inline void setReferDialog(BTDialog* pDialog)
	{
		m_pRefer 	= pDialog;
	}
	inline BTDialog* referDialog()
	{
		return m_pRefer;
	}

    /**
    * @brief 	REFER-NOTIFY Sequence 관리
    */
	inline void setReferCSeq(int nReferCSeq)
	{ 
		m_nReferCSeq = nReferCSeq; 
	}
	inline int referCSeqStep()		
	{
		return m_nReferCSeq;
	}

	inline const char* callId()
	{
		return m_strCallId;
	}

	inline void setCallId( const char* p_szCallId )
	{
		if( p_szCallId )
			m_strCallId = p_szCallId;
		else
			m_strCallId = "";
	}

	inline const char* clientCallId()
	{
		return m_strClientCallId;
	}

	inline void setClientCallId( const char* p_szClientCallId )
	{
		if( p_szClientCallId )
			m_strClientCallId = p_szClientCallId;
		else
			m_strClientCallId = "";
	}

	

	/**
	* @brief	미디어 정보 관리
	*/
	bool setSDPMessage(BTSIPMessage* pMessage, bool bFax = false);
	bool getSDPMessage(BTSIPMessage* pMessage);

	/**
	* @brief	메시지 처리
	*/
	int proceed(int nStatus);
	int accept(int nStatus);
	int reject(int nStatus);
	int bye();
	int byeInUserContext();
	BTDialog* inviteInUserContext( const char* szTarget, const char* szFrom );
    BTDialog* inviteInUserContext( SIP_Session* pSession, const char* szTarget, const char* szFrom);
	int reinvite();
	int reinviteInUserContext();
	

	static int rejectNoSession( BTDialog* pDialog, int nStatus );
	static int byeNoSession( BTDialog* pDialog );

};




