#ifndef FASIP_H
#define FASIP_H

#ifdef __cplusplus
extern "C" {
#endif


//typedef unsigned char	uchar;
//typedef unsigned short  ushort;
//typedef unsigned int    uint;


//#define MAXCHILD	8
#define MAXCHILD	(1024*1024)
#define MAXREG		32
#define MAXDEVICE	960
#define SIPSEMNAME		"Global\\fasip_process"
#define SIPSHMNAME		"Global\\fasip_pm"				// terminal-service을 통해서도 shared memory를 볼려면 Global을 붙여야 한다.
#define SEMID_SIP_PM	0
#define SEMID_SIP_IN	1
/* ********** FASIP_OUT 추가 Update By SSY (2011.08.18) START ********** */
#define SEMID_SIP_OUT	2
/* ********** FASIP_OUT 추가 Update By SSY (2011.08.18) END   ********** */


// ---------------------------------------------------------------------------
// famon.exe와 주고받은 Msg 종류
// ---------------------------------------------------------------------------
enum{
	Ipc_Rq_Ready=0,
	Ipc_Rq_Reload,
	Ipc_Rq_Disc,
	Ipc_Rq_Block,
	Ipc_Rq_Unblock,
	Ipc_Rq_Close_Reg,
};

// ---------------------------------------------------------------------------
// fasip_in.exe -> SIP_FID Msg 종류
// ---------------------------------------------------------------------------
typedef enum{
	// TCP Session
	Fax_Keep_Alive_Sip=0,
	Fax_Register_Ack,

	// Inbound
	Fax_Rx_Start,
	Fax_Rx_Restart,

	// Inbound / Outbound
	Fax_Abort,

	// Outbound
	Fax_ReqCalling_Ack,
	Fax_Cancelled
}sip2fax_msg;



// ---------------------------------------------------------------------------
// fasip_in.exe <- SIP_FID Msg 종류
// ---------------------------------------------------------------------------
typedef enum{
	Fax_Keep_Alive_Fax=0,
	Fax_Register,

	// Inbound
	Fax_Complete,

	// Outbound
	Fax_ReqCalling
}fax2sip_msg;


// ---------------------------------------------------------------------------
// fasip_in.exe와 SIP_FID 간 연동 전문
// ---------------------------------------------------------------------------
typedef struct{
	int				msgid;
	int				chan;								// fax device 번호
	int				len;								// para의 len
	char			para[256];
}fax_sip_control;

typedef struct{
	char			fax_direction;						// 'I' - Inbound, 'O' - Outbound, 'B' - Both
	char			fax_ip[32];							// 송/수신 Process IP
	int				fax_base_port;						// Base Port
	int				fax_port_spacing;					// Port Spacing
	int				fax_device_start;
	int				fax_device_cnt;
}fax_register;

typedef struct{
	int				result;								// 0:success, 1이상: fail
}fax_register_ack;

typedef struct{
	char			fax_target[64];
	char			fax_from[64];
	char			client_callid[64];
}fax_call;

typedef struct{
	int				result;								// 0:success, 1이상: fail
}fax_call_ack;

typedef struct{
	int				sip_reason;
}fax_cancelled;

// ---------------------------------------------------------------------------
// 호 정보
// ---------------------------------------------------------------------------

typedef struct{
	char			from[64];							// 발신전화번호
	char			to[64];								// 착신전화번호
	char			transfer[64];						// 
	char			remote_ip[32];						// SDP Remote IP
	int				remote_port;						// SDP Remote Port
}fax_rx_start;

typedef struct{
	int				accept;								// accept socket (SIP_FOD와 연동)
	time_t			start_time;							// 접속시작시간
	time_t			last;								// 마지막으로 전문받은 시간
	unsigned int	ip;									// 접속한 SIP_FOD IP
	int				port;								// 접속한 SIP_FOD port
	fax_register	f;									// SIP_FOD의 register 정보
	int				state;								// 0:Data받기전, Data받음:1, Data받은완료:2
	int				ptr;								// 수신전문 길이
	fax_sip_control	m;									// SIP_FID와의 연동전문
}reg_state;

typedef struct{
	int				reg;								// SIP_FID가 저장된 shmem->r[i]의 i+1
	int				block;								// 1이면 Block, 0이면 Unblock
	unsigned int	leg;								// callleg 값
	int				state;
	int				fstate;
	int				reinvite_succeed;
	int				sdp_ver;
	int				sdp_mode;
	char			call_direction;						// 발신:'O', 척신:'I'
	int				disconnector;						// Disconnect 주체 -> Local:'L', Remote:'R'
	time_t			start_time;							// 호 인입시간
	fax_rx_start	s;									// 호정보 (발신/착신번호/Remote이 SDP 정보 
}dev_state;

// ---------------------------------------------------------------------------
// SIP IN 처리에서 사용할 Shared Memory 구조체
// ---------------------------------------------------------------------------
typedef struct{
	unsigned int	pin[MAXCHILD];						// Process ID
	int				dont_restart[MAXCHILD];				// 재시작 여부 (TRUE:재시작, FALSE:재시작안함)
	time_t			start_time[MAXCHILD];				// Process 시작시간
	reg_state		r[MAXREG];							// register 상태
	dev_state		d[MAXDEVICE];						// FAX CH 상태
}sip_share;


// ---------------------------------------------------------------------------
// famon.exe와의 연동전문
// ---------------------------------------------------------------------------
typedef struct{
	int				msgid;								// Ipc_Rq_Ready, Ipc_Rq_Reload, Ipc_Rq_Disc, Ipc_Rq_Block, Ipc_Rq_Unblock, Ipc_Rq_Close_Reg
	int				b;									// UnRegist 처리할 SIP_FID Index
														// Block/Unblock을 걸 채널 개수
	int				chan;								// Block/Unblock을 걸 시작 채널
	int				len;								// para 길이
	char			para[112];							// 추가 Data
}ipc_msg;

#ifdef __cplusplus
}
#endif

#endif
