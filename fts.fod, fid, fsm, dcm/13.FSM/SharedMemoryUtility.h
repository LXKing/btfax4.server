#pragma once


#define MAXCHILD	8
#define MAXREG		32
#define MAXDEVICE	960
#define FSM_SHM		"Global\\FSM_SHM"		// Shared Memory �̸� ���ξ�
#define SHMID_BASE	0
#define SHMID_PM	SHMID_BASE



// ---------------------------------------------------------------------------
// ȣ ����
// ---------------------------------------------------------------------------
//
//struct reg_state
//{
//	int				accept;								// accept socket (SIP_FOD�� ����)
//	time_t			start_time;							// ���ӽ��۽ð�
//	time_t			last;								// ���������� �������� �ð�
//	unsigned int	ip;									// ������ SIP_FOD IP
//	int				port;								// ������ SIP_FOD port
//	fax_register	f;									// SIP_FOD�� register ����
//	int				state;								// 0:Data�ޱ���, Data����:1, Data�����Ϸ�:2
//	int				ptr;								// �������� ����
//	fax_sip_control	m;									// SIP_FID���� ��������
//};
//
//struct dev_state
//{
//	int				reg;								// SIP_FID�� ����� shmem->r[i]�� i+1
//	int				block;								// 1�̸� Block, 0�̸� Unblock
//	unsigned int	leg;								// callleg ��
//	int				state;
//	int				fstate;
//	int				reinvite_succeed;
//	int				sdp_ver;
//	int				sdp_mode;
//	int				call_direction;						// �߽�:'O', ô��:'I'
//	int				disconnector;						// Disconnect ��ü -> Local:'L', Remote:'R'
//	time_t			start_time;							// ȣ ���Խð�
//	fax_rx_start	s;									// ȣ���� (�߽�/���Ź�ȣ/Remote�� SDP ���� 
//};
//
//// ---------------------------------------------------------------------------
//// SIP IN ó������ ����� Shared Memory ����ü
//// ---------------------------------------------------------------------------
//typedef struct{
//	int				pin[MAXCHILD];						// Process ID
//	int				dont_restart[MAXCHILD];				// ����� ���� (TRUE:�����, FALSE:����۾���)
//	time_t			start_time[MAXCHILD];				// Process ���۽ð�
//	reg_state		r[MAXREG];							// register ����
//	dev_state		d[MAXDEVICE];						// FAX CH ����
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



