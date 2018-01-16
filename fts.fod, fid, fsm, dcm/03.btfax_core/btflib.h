#ifndef BTFAX_LIB_H
#define BTFAX_LIB_H

#include <windows.h>
#include <list>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif


enum{
	Flib_Error_Timeout=-1,
	Flib_Error_Config_Fail=-11,
	Flib_Error_Shmem_Id_Fail=-12,
	Flib_Error_Shmem_Attach_Fail=-13,
	Flib_Error_Memory_Allocation_Fail=-14,
	Flib_Error_Create_Thread_Fail=-15,
	Flib_Error_Invalid_Channel=-21,
	Flib_Error_Invalid_Queue_Id=-24,
	Flib_Error_Invalid_Para=-26,
	Flib_Error_Bound_Process_Not_Running=-27,
	Flib_Error_Semget_Fail=-28,
	Flib_Error_Semop_Fail=-29,
	Flib_Error_Channel_Not_Owned=-31,
	Flib_Error_Send_Message_Fail=-33,
	Flib_Error_Device_Alloc_Fail=-41,
	Flib_Error_Device_Busy=-51,
	Flib_Error_Device_Idle=-52,
};

enum{
	Fwait_Sec,
};


typedef struct{
	uint	t;
	int		e;
	HANDLE	m;
	HANDLE	c;
	void	* end;
}ftrans_list;

typedef struct{
	int	max_fax_endpoint;
	int	max_fax_process;
}fax_capacity;


typedef struct{
	char	path[256];
	char	page[208];
	char	sid[24];
	char	subaddr[24];
}tiff_file_info;

#define Max_Tiff_File	10
typedef struct{
	ipc_fax_header h;
	int		cnt;
	tiff_file_info f[Max_Tiff_File];
}send_tiff_info;

typedef struct{
	ipc_fax_header h;
	tiff_file_info f;
}recv_tiff_info;

typedef struct{
	int		sync;
	int		strip_sent;
	int		file_len;
	FILE	* fp;
	char	ppath[256];
	tiff_file_info f;
}fax_str_info;

#define Max_Page_Frag	104
typedef struct{
	int	cnt;
	int	pos;
	short	spage[Max_Page_Frag];
	short	pagecnt[Max_Page_Frag];
}fax_page_info;


#define DllExport   __declspec(dllexport)

int		FaxConfigure();
int		AttachFaxMemory();
int		InitBtFaxIpcCallback(int qcnt, int thrcnt, void (* callback)(int qid, ipc_fax_msg_rcv * msg));


DllExport	char	* GetBtfaxLibVersion();
DllExport	char	* GetBtfaxLibBuildInfo();
DllExport	int		InitBtFax(int qcnt);
DllExport	int		InitBtFaxCallback(int qcnt, int thrcnt, void (* callback)(int qid, ipc_fax_msg_rcv * msg));
DllExport	void	CloseBtFax();
DllExport	fax_share	* GetFaxSharedMemory();
DllExport	uint	HmpFaxGetTransactionId();
DllExport	int		FaxWaitEvent(int qid, ipc_fax_msg_rcv * msg, int milisec);
DllExport	int		FaxPushEvent(int qid, ipc_fax_msg_rcv * msg);
DllExport	int		FaxClearQueue(int qid);
DllExport	void	FaxGetLocalMediaInfo(fax_media_info * media);
DllExport	void	FaxGetLicense(fax_license * lic);
DllExport	void	FaxGetMaxCapacity(fax_capacity * cap);
DllExport	int		FaxGetProcessState(ushort state[MAXFCHILD]);
DllExport	char	* GetFaxError(int err);

DllExport	int		HmpFaxGetState			(int chan, ipc_fax_state * fax);
DllExport	char	* HmpFaxGetTxStateName	(int s);
DllExport	char	* HmpFaxGetRxStateName	(int s);

DllExport	int		HmpFaxOpen			(int chan, int qid);
DllExport	int		HmpFaxOpenSync		(int chan, int qid);
DllExport	int		HmpFaxAllocOpen		(int * chan, int qid);
DllExport	int		HmpFaxAllocOpenSync	(int * chan, int qid);
DllExport	int		HmpFaxClose			(int chan);
DllExport	int		HmpFaxCloseSync		(int chan);
DllExport	int		HmpFaxSend			(int chan, send_tiff_info * fax);
DllExport	int		HmpFaxSendSync		(int chan, send_tiff_info * fax, ipc_fax_state * end);
DllExport	int		HmpFaxRecv			(int chan, recv_tiff_info * fax);
DllExport	int		HmpFaxRecvSync		(int chan, recv_tiff_info * fax, ipc_fax_state * end);
DllExport	int		HmpFaxStop			(int chan, int last_cmd);
DllExport	int		HmpFaxStopSync		(int chan, int last_cmd, ipc_fax_state * end);
DllExport	int		HmpFaxAbort			(int chan);
DllExport	int		HmpFaxAbortSync		(int chan);

DllExport	void	WriteFax (int pin, int chan, int msgid, int len, char * para, int sync);
DllExport	int		WriteFax2(         int chan, int msgid, int len, char * para, int sync, ftrans_list * t);


unsigned __stdcall FaxTimerThread(LPVOID arg);
unsigned __stdcall RxFaxMsg(LPVOID arg);

void	ClearFileList(int chan);

int		AcquireFax(int chan);
void	ReleaseFax(int chan);

void	FimerEvent();
int		GetFimer(int owner, int state, int timeout, int para, int para2);
int		StopFimer(int owner, int state);

int		Split2Foken(char * str, char enc, int numtoken, char tok[][256]);

void	dbgf(int i, char * str,...);


extern	fax_share  * shfax;
extern	fax_state  * shmf;
extern	char	BTFAX_DIR[256];

extern	int		fax_qcnt;
extern	int		fax_thrcnt;
extern	int		fax_qid[MAXFAX];
extern	void	(* btfax_callback)(int qid, ipc_fax_msg_rcv * msg);
extern	int		* sock_fax;
extern	volatile int init_fax;
extern	FILE	* logflibfp;
extern	int		flogline;
extern	char	lastfc;
extern	HANDLE	fax_write;
extern	HANDLE	m_ftimer;
extern	HANDLE	m_flog;
extern	fax_open_state fapp_own;
extern	int		fown_len;

extern	char	fconfigfile[256];
extern	uint	ftransaction_id;
extern	/* __declspec(thread) */uint tls_ftrans_id;
extern	int		fpid;
extern	int		fsemid_man, semid_fax[MAXFAXPROC];

extern	struct sockaddr_in dafax[MAXFAXPROC];
extern	struct sockaddr_in * fax_app_addr;

extern	int		fax_reg[MAXFAX];

extern	HANDLE	* fcb_c;
extern	HANDLE	* fcb_m;
extern	HANDLE	ftl_m[MAXFAX];
extern	HANDLE	tfl_m[MAXFAX];

extern	list<ftrans_list>	ftl[MAXFAX];
extern	list<ipc_fax_msg_rcv> * fmsg;

extern	list<tiff_file_info> fflist[MAXFAX];
extern	fax_str_info	fxstr[MAXFAX];

extern	int		fhlen;

extern	DWORD	ftid_timer;
extern	uint	fmili;
extern	FIMER	* ftimer;
extern	int		fmax_timer;

extern	uchar	bswap[256];


#ifdef __cplusplus
}
#endif

#endif
