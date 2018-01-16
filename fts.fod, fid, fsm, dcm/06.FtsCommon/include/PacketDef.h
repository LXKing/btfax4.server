#ifndef __PACKET_DEF_H__
#define __PACKET_DEF_H__

enum _enumAPP
{
	__SHUTDOWN_SERVICE__,
	__FAX_COMPLETE__,
	__FAX_START__,
	__FAX_RESTART__,
	__FAX_ABORT__,
};


enum SIP_STATUS
{
	SIP_Uninit = -1,
	SIP_Init,
	SIP_Connect,
	SIP_Register
};

enum SIP2FAX_MSG
{
	// TCP Session
	Fax_Keep_Alive_Sip = 0,
	Fax_Register_Ack,

	// Inbound
	Fax_Rx_Start,
	Fax_Rx_Restart,

	// Inbound / Outbound
	Fax_Abort,

	// Outbound
	Fax_ReqCalling_Ack,
	Fax_Cancelled

};

enum FAX2SIP_MSG
{
	// TCP Session
	Fax_Keep_Alive_Fax = 0,
	Fax_Register,

	// Inbound / Outbound
	Fax_Complete,

	// Outbound
	Fax_ReqCalling

};




struct FAX_SIP_HEADER
{
	int		msgid;
	int		chan;
	int		len;
};

struct FAX_SIP_CONTROL
{
	FAX_SIP_HEADER	header;	
	char			body[256];
};


struct FAX_REGISTER
{
	char	fax_direction;										// 'I' - Inbound, 'O' - Outbound, 'B' - Both
	char	fax_ip[32];							
	int		fax_base_port;										// Default : 5060
	int		fax_port_spacing;
	int		fax_device_start;									// Default : 0 (시스템 내에서의 0 Based 시작 CH번호)
	int		fax_device_cnt;										// 해당 프로세스 내에서의 CH 개수
};

struct FAX_REGISTER_ACK
{
	int		result;												// 0 (정상)

};

struct FAX_RX_START
{
	char	from[64];											// 발신번호
	char	to[64];												// 착신번호1 (1차 착신)
	char	transfer[64];										// 착신번호2 (2차 착신)

	char	gw_ip[32];
	int		gw_port;
};

struct FAX_CALL
{
	char	fax_target[64];
	char	fax_from[64];
	char	client_callid[64];
};

struct FAX_CALL_ACK
{
	int		result;								// 0:success, 1이상: fail
};

struct FAX_CANCELLED
{
	int		sip_reason;
};


union FAX_RECV_BODY
{
	FAX_REGISTER_ACK	regi;
	FAX_RX_START		start;
	FAX_CALL_ACK		call_ack;
	FAX_CANCELLED		cancelled;
};

struct FAX_RECV_PACKET
{
	FAX_SIP_HEADER	h;
	FAX_RECV_BODY	b;
	char			dummy[100];	// 사용안함. 메모리 침범 방지 버퍼.
};

union FAX_SEND_BODY
{
	FAX_CALL	call;
};

struct FAX_SEND_PACKET
{
	FAX_SIP_HEADER	h;
	FAX_SEND_BODY	b;
	char			dummy[100];	// 사용안함. 메모리 침범 방지 버퍼.
};



inline const char* MsgName_FromFsm( int p_msgid )
{
	static char szNotDefined[50];

	switch( p_msgid )
	{
	case Fax_Keep_Alive_Sip:	return "Fax_Keep_Alive_Sip";
	case Fax_Register_Ack:		return "Fax_Register_Ack";
	case Fax_Rx_Start:			return "Fax_Rx_Start";
	case Fax_Rx_Restart:		return "Fax_Rx_Restart";
	case Fax_Abort:				return "Fax_Abort";
	case Fax_Cancelled:			return "Fax_Cancelled";
	case Fax_ReqCalling_Ack:	return "Fax_ReqCalling_Ack";
	}

	_snprintf( szNotDefined, sizeof(szNotDefined), "Not_Define[%d]", p_msgid );

	return szNotDefined;
}

inline int MsgLen_FromFsm( int p_msgid )
{
	switch( p_msgid )
	{
	case Fax_Keep_Alive_Sip:	return  sizeof(FAX_SIP_HEADER);
	case Fax_Register_Ack:		return  sizeof(FAX_SIP_HEADER) + sizeof(FAX_REGISTER_ACK);
	case Fax_Rx_Start:			return  sizeof(FAX_SIP_HEADER) + sizeof(FAX_RX_START);
	case Fax_Rx_Restart:		return  sizeof(FAX_SIP_HEADER) + sizeof(FAX_RX_START);
	case Fax_Abort:				return  sizeof(FAX_SIP_HEADER);
	case Fax_Cancelled:			return  sizeof(FAX_SIP_HEADER) + sizeof(FAX_CANCELLED);
	case Fax_ReqCalling_Ack:	return  sizeof(FAX_SIP_HEADER) + sizeof(FAX_CALL_ACK);
	}

	return -1;
}

inline const char* MsgName_FromClient( int p_msgid )
{
	static char szNotDefined[50];

	switch( p_msgid )
	{
	case Fax_Keep_Alive_Fax:	return "Fax_Keep_Alive_Fax";
	case Fax_Register:			return "Fax_Register";
	case Fax_Complete:			return "Fax_Complete";
	case Fax_ReqCalling:		return "Fax_ReqCalling";
	}

	_snprintf( szNotDefined, sizeof(szNotDefined), "Not_Define[%d]", p_msgid );

	return szNotDefined;
}

inline int MsgLen_FromClient( int p_msgid )
{
	switch( p_msgid )
	{
	case Fax_Keep_Alive_Fax:	return  sizeof(FAX_SIP_HEADER);
	case Fax_Register:			return  sizeof(FAX_SIP_HEADER) + sizeof(FAX_REGISTER);
	case Fax_Complete:			return  sizeof(FAX_SIP_HEADER);
	case Fax_ReqCalling:		return  sizeof(FAX_SIP_HEADER) + sizeof(FAX_CALL);
	}

	return -1;
}


#endif