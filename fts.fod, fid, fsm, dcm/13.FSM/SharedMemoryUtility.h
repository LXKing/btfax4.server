#pragma once


#define MAXCHILD	8
#define MAXREG		32
#define MAXDEVICE	960
#define FSM_SHM		"Global\\FSM_SHM"		// Shared Memory 이름 접두어
#define SHMID_BASE	0
#define SHMID_PM	SHMID_BASE



// ---------------------------------------------------------------------------
// 호 정보
// ---------------------------------------------------------------------------
//
//struct reg_state
//{
//	int				accept;								// accept socket (SIP_FOD와 연동)
//	time_t			start_time;							// 접속시작시간
//	time_t			last;								// 마지막으로 전문받은 시간
//	unsigned int	ip;									// 접속한 SIP_FOD IP
//	int				port;								// 접속한 SIP_FOD port
//	fax_register	f;									// SIP_FOD의 register 정보
//	int				state;								// 0:Data받기전, Data받음:1, Data받은완료:2
//	int				ptr;								// 수신전문 길이
//	fax_sip_control	m;									// SIP_FID와의 연동전문
//};
//
//struct dev_state
//{
//	int				reg;								// SIP_FID가 저장된 shmem->r[i]의 i+1
//	int				block;								// 1이면 Block, 0이면 Unblock
//	unsigned int	leg;								// callleg 값
//	int				state;
//	int				fstate;
//	int				reinvite_succeed;
//	int				sdp_ver;
//	int				sdp_mode;
//	int				call_direction;						// 발신:'O', 척신:'I'
//	int				disconnector;						// Disconnect 주체 -> Local:'L', Remote:'R'
//	time_t			start_time;							// 호 인입시간
//	fax_rx_start	s;									// 호정보 (발신/착신번호/Remote이 SDP 정보 
//};
//
//// ---------------------------------------------------------------------------
//// SIP IN 처리에서 사용할 Shared Memory 구조체
//// ---------------------------------------------------------------------------
//typedef struct{
//	int				pin[MAXCHILD];						// Process ID
//	int				dont_restart[MAXCHILD];				// 재시작 여부 (TRUE:재시작, FALSE:재시작안함)
//	time_t			start_time[MAXCHILD];				// Process 시작시간
//	reg_state		r[MAXREG];							// register 상태
//	dev_state		d[MAXDEVICE];						// FAX CH 상태
//}sip_share;
//

extern sip_share *	shmem;
extern unsigned int	SHMID_FSM;


class CSharedMemory
{
public:
	CSharedMemory();
	virtual ~CSharedMemory();

public:
	static void* MakeSharedMemory( const char* p_szName, int p_nSize );
	static void* OpenSharedMemory( const char* p_szName, int p_nSize, bool p_bCreate );
	
	static void	InitCdr();
	static void	WriteCdr(int i);
};

class CCdr
{
public:
	CCdr();
	virtual ~CCdr();

public:
	static void	InitCdr();
	static void	WriteCdr(int i);
};

#define STRCPY_( __szDst, __szSrc ) \
	{ \
		int __nSrcLen__ = strlen( __szSrc ); \
		memset( __szDst, 0x00, sizeof(__szDst) ); \
		if( sizeof(__szDst) > __nSrcLen__) \
			strncpy( __szDst, __szSrc, __nSrcLen__ ); \
		else \
			strncpy( __szDst, __szSrc, sizeof(__szDst)-1 ); \
	}



