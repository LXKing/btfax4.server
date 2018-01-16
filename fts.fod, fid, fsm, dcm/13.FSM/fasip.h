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
#define SIPSHMNAME		"Global\\fasip_pm"				// terminal-service�� ���ؼ��� shared memory�� ������ Global�� �ٿ��� �Ѵ�.
#define SEMID_SIP_PM	0
#define SEMID_SIP_IN	1
/* ********** FASIP_OUT �߰� Update By SSY (2011.08.18) START ********** */
#define SEMID_SIP_OUT	2
/* ********** FASIP_OUT �߰� Update By SSY (2011.08.18) END   ********** */


// ---------------------------------------------------------------------------
// famon.exe�� �ְ���� Msg ����
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
// fasip_in.exe -> SIP_FID Msg ����
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
// fasip_in.exe <- SIP_FID Msg ����
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
// fasip_in.exe�� SIP_FID �� ���� ����
// ---------------------------------------------------------------------------
typedef struct{
	int				msgid;
	int				chan;								// fax device ��ȣ
	int				len;								// para�� len
	char			para[256];
}fax_sip_control;

typedef struct{
	char			fax_direction;						// 'I' - Inbound, 'O' - Outbound, 'B' - Both
	char			fax_ip[32];							// ��/���� Process IP
	int				fax_base_port;						// Base Port
	int				fax_port_spacing;					// Port Spacing
	int				fax_device_start;
	int				fax_device_cnt;
}fax_register;

typedef struct{
	int				result;								// 0:success, 1�̻�: fail
}fax_register_ack;

typedef struct{
	char			fax_target[64];
	char			fax_from[64];
	char			client_callid[64];
}fax_call;

typedef struct{
	int				result;								// 0:success, 1�̻�: fail
}fax_call_ack;

typedef struct{
	int				sip_reason;
}fax_cancelled;

// ---------------------------------------------------------------------------
// ȣ ����
// ---------------------------------------------------------------------------

typedef struct{
	char			from[64];							// �߽���ȭ��ȣ
	char			to[64];								// ������ȭ��ȣ
	char			transfer[64];						// 
	char			remote_ip[32];						// SDP Remote IP
	int				remote_port;						// SDP Remote Port
}fax_rx_start;

typedef struct{
	int				accept;								// accept socket (SIP_FOD�� ����)
	time_t			start_time;							// ���ӽ��۽ð�
	time_t			last;								// ���������� �������� �ð�
	unsigned int	ip;									// ������ SIP_FOD IP
	int				port;								// ������ SIP_FOD port
	fax_register	f;									// SIP_FOD�� register ����
	int				state;								// 0:Data�ޱ���, Data����:1, Data�����Ϸ�:2
	int				ptr;								// �������� ����
	fax_sip_control	m;									// SIP_FID���� ��������
}reg_state;

typedef struct{
	int				reg;								// SIP_FID�� ����� shmem->r[i]�� i+1
	int				block;								// 1�̸� Block, 0�̸� Unblock
	unsigned int	leg;								// callleg ��
	int				state;
	int				fstate;
	int				reinvite_succeed;
	int				sdp_ver;
	int				sdp_mode;
	char			call_direction;						// �߽�:'O', ô��:'I'
	int				disconnector;						// Disconnect ��ü -> Local:'L', Remote:'R'
	time_t			start_time;							// ȣ ���Խð�
	fax_rx_start	s;									// ȣ���� (�߽�/���Ź�ȣ/Remote�� SDP ���� 
}dev_state;

// ---------------------------------------------------------------------------
// SIP IN ó������ ����� Shared Memory ����ü
// ---------------------------------------------------------------------------
typedef struct{
	unsigned int	pin[MAXCHILD];						// Process ID
	int				dont_restart[MAXCHILD];				// ����� ���� (TRUE:�����, FALSE:����۾���)
	time_t			start_time[MAXCHILD];				// Process ���۽ð�
	reg_state		r[MAXREG];							// register ����
	dev_state		d[MAXDEVICE];						// FAX CH ����
}sip_share;


// ---------------------------------------------------------------------------
// famon.exe���� ��������
// ---------------------------------------------------------------------------
typedef struct{
	int				msgid;								// Ipc_Rq_Ready, Ipc_Rq_Reload, Ipc_Rq_Disc, Ipc_Rq_Block, Ipc_Rq_Unblock, Ipc_Rq_Close_Reg
	int				b;									// UnRegist ó���� SIP_FID Index
														// Block/Unblock�� �� ä�� ����
	int				chan;								// Block/Unblock�� �� ���� ä��
	int				len;								// para ����
	char			para[112];							// �߰� Data
}ipc_msg;

#ifdef __cplusplus
}
#endif

#endif
