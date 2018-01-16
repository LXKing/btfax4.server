#ifndef BTFAX_H
#define BTFAX_H

#include <time.h>
#include <winsock2.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char	uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;


#define SEMNAME		"Global\\btfax_process"
#define SEMDEV		"Global\\faxdev"
#define SEMLIC		"Global\\faxlic"
#define SHMNAME		"Global\\btfax_pm"		// terminal-service을 통해서도 shared memory를 볼려면 Global을 붙여야 한다.

#define MAXFAXPROC		60
#define SEMID_FAX_PM	0
#define SEMID_FAX_EMA	1
#define SEMID_FAX_LOGD	2
#define SEMID_FAX_CDR	3
#define SEMID_FAX_T38	4
#define MAXFCHILD		(SEMID_FAX_T38+MAXFAXPROC)

#define SEMID_LIC_FAX	0
#define MAXFLICENSE		8

#define MAXFAX			480

#define MAXFCDR			1000

#define MAXFBUF			128000

#define LOGFBUFLEN		8000

#define Max_Tx_Fax_Segment	10
#define Max_Rx_Fax_Segment	10

#define Max_Redundancy_Packet	8
#define Max_Packet_Wait			16


enum{
	Ipc_Fax_Close,
	Ipc_Fax_Start_Process,
	Ipc_Fax_Stop_Process,
	Ipc_Fax_Restart_Process,
	Ipc_Fax_Reload_Config,
	Ipc_Fax_Trace_Terminal,
	Ipc_Fax_Open_Channel=101,
	Ipc_Fax_Close_Channel,
	Ipc_Fax_Send,
	Ipc_Fax_Recv,
	Ipc_Fax_Stop,
	Ipc_Fax_Abort,
	Ipc_Fax_Bad_File,
	Ipc_Fax_Start_Cng,
	Ipc_Fax_Send_Next_Page,
	Ipc_Fax_Send_Curr_Page,
	Ipc_Fax_Start_Ced,
	Ipc_Fax_Trace,
	Ipc_Indi_Fax_Reset_Channel=1101,
	Ipc_Indi_Fax_Send_Complete=1111,
	Ipc_Indi_Fax_Send_Page,
	Ipc_Indi_Fax_Read_Page,
	Ipc_Indi_Fax_Reread_Page,
	Ipc_Indi_Fax_Need_Image,
	Ipc_Indi_Fax_Recv_Complete=1121,
	Ipc_Indi_Fax_Recv_Page,
	Ipc_Indi_Fax_Store_Image,
	Ipc_Indi_Fax_Stop_Complete=1131,
	Ipc_User_Fax_Msg_Base=100000,
};

enum{
	Fax_Error_Not_Opened=1,
	Fax_Error_License,
	Fax_Error_Invalid_Channel,
	Fax_Error_Fax_Reset,
	Fax_Error_Fax_Already_Opened,
	Fax_Error_Fax_Already_Stopped,
	Fax_Error_Device_Busy,
	Fax_Error_Socket_Fail,
	Fax_Error_Bad_File,
	Fax_Error_Abort,
	Fax_Error_Open_To_Stop_Active_Channel=1001,
	Fax_Error_Close_To_Stop_Active_Channel,
};

enum{
	Ft_Ready=1,
	Ft_Sending_Cng=2,
	Ft_Preamble_Detected=3,
	Ft_Sending_Dcs=4,
	Ft_Sending_Train_Tcf=6,
	Ft_Sending_Resync=7,
	Ft_Sending_Image=8,
	Ft_Sending_Post_Msg=9,
	Ft_Waiting_Mcf=10,
	Ft_Rereading_Page=11,
	Ft_Sending_Ctc=12,
	Ft_Waiting_Ctr=13,
	Ft_Sending_Interrupt=14,
	Ft_Sending_Dcn=15,
};

enum{
	Fr_Ready=1,
	Fr_Sending_Ced=2,
	Fr_Sending_Dis=4,
	Fr_Dcs_Received=5,
	Fr_Train_Received=6,
	Fr_Sending_Cfr=7,
	Fr_Receiving_Image=8,
	Fr_Receiving_Post_Msg=9,
	Fr_Sending_Mcf=10,
	Fr_Sending_Last_Mcf=11,
	Fr_Decoding_Ecm_Block=12,
	Fr_Sending_Interrupt=14,
	Fr_Sending_Dcn=15,
};

enum{
	Fax_Tx_Fail_Dis_Timeover=1,
	Fax_Tx_Fail_Train,
	Fax_Tx_Fail_Rtn,
	Fax_Tx_Fail_Ctc_Error,
	Fax_Tx_Fail_Cfr_Timeout,
	Fax_Tx_Fail_Mcf_Timeout,
	Fax_Tx_Fail_Mcf_Last_Timeout,
	Fax_Tx_Fail_Ctr_Timeout,
	Fax_Tx_Fail_Dis_In_Image,
	Fax_Tx_Fail_Dis_In_Waiting_Mcf,
	Fax_Tx_Fail_Dcn_Recv=21,
	Fax_Tx_Fail_Local_Interrupt,
	Fax_Tx_Fail_Pin_Recv,
	Fax_Tx_Fail_Pip_Recv,
	Fax_Tx_Fail_Flow_Control,
	Fax_Tx_Fail_Stop=31,
};

enum{
	Fax_Rx_Fail_Dcs_Timeover=1,
	Fax_Rx_Fail_Dcn_Recv=21,
	Fax_Rx_Fail_Local_Interrupt,
	Fax_Rx_Fail_Pri_Recv,
	Fax_Rx_Fail_Timeover,
};

enum{
	Fax_First_Cmd_Dcs_Dis=1,
};

enum{
	Fax_Last_Cmd_Dcn=0,
	Fax_Last_Cmd_Pri,
	Fax_Last_Cmd_Eop=5946325,
};

enum{
	Fax_Resol_File=0,
	Fax_Resol_Standard,
	Fax_Resol_High,
	Fax_Resol_Super_High,
};


typedef struct{
	int		msgid;
	int		chan;
	int		sync;
	uint	trans;
	int		err;
	int		len;
	char	para[4072];
}ipc_fax_msg;

typedef struct{
	int		msgid;
	int		chan;
	int		sync;
	uint	trans;
	int		err;
	int		len;
	char	para[104];
}ipc_fax_msg_rcv;

typedef struct{
	char	text[256];
	int	overwrite;
	int	align;		// 0: center, 1: left, 2: right
	int	xoff;
	int	yoff;
}header_overlay;

typedef struct{
	char	ip[64];
	char	srcip[64];
	char	localid[24];
	int		port;
	int		resolution;
	int		ecm;
	int		max_rate;
	int		redundancy_level;
	int		first_cmd;
	int		last_cmd;
	int		timer_t1;
	int		ignore_pri;
	int		version;
	int		use_fec;
	header_overlay ovl;
}ipc_fax_header;

typedef struct{
	int		sr;
	int		error;
	int		state;
	int		encoding;
	int		ecm;
	int		page;
	int		sig_rate;
	int		resolution;
	char	remoteid[24];
}ipc_fax_state;


typedef struct{
	int		owner;
	int		state;
	int		para;
	int		para2;
	uint	end;
}FIMER;

typedef struct{
	int		base_fax_port;
	int		fax_port_spacing;
}fax_media_info;

typedef struct{
	int		pid;
	char	cmd[256];
}fax_open_state;

typedef struct{
	int		chan;
	time_t	start_time;
	time_t	end_time;
	ipc_fax_state s;
}fax_recent_cdr;

typedef struct{
	ushort	tag;
	ushort	type;
	uint	cnt;
	uint	value;
}ifd_entry;

typedef struct{
	int		NewSubFileType;			// 254
	int		ImageWidth;				// 256
	int		ImageLength;			// 257
	int		BitsPerSample;			// 258
	int		Compression;			// 259
	int		PhotometricInterpretation;	// 262
	int		FillOrder;				// 266
	int		StripOffsets;			// 273
	int		SamplesPerPixel;		// 277
	int		RowsPerStrip;			// 278
	int		StripByteCounts;		// 279
	int		XResolution;			// 282
	int		YResolution;			// 283
	int		T4Options;				// 292
	int		T6Options;				// 293
	int		ResolutionUnit;			// 296
	int		PageNumber;				// 297
	int		BadFaxLines;			// 326
	int		CleanFaxData;			// 327
	int		ConsBadFaxLines;		// 328
	int	flag;
}tiff_attr;

typedef struct{
	uchar	b0_reserved_7:1;
	uchar	b0_oct256_64:1;
	uchar	b0_v8:1;
	uchar	b0_reserved_4:1;
	uchar	b0_3g_mobile:1;
	uchar	b0_t38:1;
	uchar	b0_reserved_1:1;
	uchar	b0_t37:1;
	uchar	b1_2d_coding:1;
	uchar	b1_resol:1;
	uchar	b1_rate:4;
	uchar	b1_recv:1;
	uchar	b1_poll_doc_ready:1;
	uchar	b2_ext:1;
	uchar	b2_scan_time:3;
	uchar	b2_record_length:2;
	uchar	b2_record_width:2;
	uchar	b3_ext:1;
	uchar	b3_t6_coding:1;
	uchar	b3_reserved_5:1;
	uchar	b3_reserved_4:1;
	uchar	b3_0:1;
	uchar	b3_ecm:1;
	uchar	b3_uncomp_mode:1;
	uchar	b3_reserved_0:1;
	uchar	b4_ext:1;
	uchar	b4_reserved_6:1;
	uchar	b4_adpcm:1;
	uchar	b4_plane_interleave:1;
	uchar	b4_t43_coding:1;
	uchar	b4_poll_subaddr:1;
	uchar	b4_sel_poll:1;
	uchar	b4_field_valid_cap:1;
	uchar	b5_ext:1;
	uchar	b5_sel_poll:1;
	uchar	b5_scan_time:1;
	uchar	b5_meter_prefer:1;
	uchar	b5_inch_prefer:1;
	uchar	b5_resol2:1;
	uchar	b5_resol1:1;
	uchar	b5_resol0:1;
	uchar	b6_ext:1;
	uchar	b6_edi:1;
	uchar	b6_dtm:1;
	uchar	b6_bft:1;
	uchar	b6_reserved_3:1;
	uchar	b6_poll_data_ready:1;
	uchar	b6_passwd:1;
	uchar	b6_subaddr:1;
}fax_control_cap;

typedef struct{
	int		len;
	uchar	buf[156];
}t38_redundancy_packet;

typedef struct{
	ushort	seq;
	int		len;
	uchar	buf[1024];
}t38_wait_buffer;

typedef struct{
	uint	heart;
	int		sock;
	uint	ptrans;
	uint	strans;
	uint	remoteip;
	time_t	start_time;
	ipc_fax_header h;
	tiff_attr tf;
	char	sr;
	char	state;
	char	psync;
	char	ssync;
	char	byte_order;
	char	tiff_attr_change;
	int		prev_ifd;
	int		curr_ifd;
	int		next_ifd;
	int		ifd_offset;
	int		total_page;
	int		strip_sent;
	uint	tts;
	ushort	next_seq;
	ushort	recv_seq;
	char	needf;
	char	packet_start;
	char	packet_lost;
	char	retrans_max;
	char	retrans_cnt;
	char	resolution;
	char	maxk;
	char	rline;
	char	skip;
	char	bskip;
	uchar	scnt;
	char	rtn;
	char	rtc_len;
	char	rtc_sent;
	char	rcpcnt;
	char	r2h;
	char	encmode;
	char	minbpl;
	char	nosig;
	char	disrupted;
	char	pried;
	char	cmdtry;
	char	ppr;
	char	pprcnt;
	char	image_start;
	char	preamble;
	char	sent_lid;
	char	remoteid[24];
	char	sid[24];
	char	subaddr[24];
	int		scanptr;
	int		retrans_len;
	int		hdlc_wp;
	int		hdlclen;
	int		hdlcptr;
	int		locallen;
	uchar	retrans_buf[1024];
	uchar	hdlc_buf[128];
	uchar	hdlc_cmd[64];
	fax_control_cap rcap;
	uchar	lhdlc[2];
	uchar	msg_resp;
	uchar	curr_fcf;
	uchar	ecm_post_cmd;
	uchar	ecm_flag;
	int		ecm_dec_ptr;
	int		sig_rate;
	int		train_type;
	int		data_type;
	int		train_max;
	int		train_frame;
	int		train_ptr;
	int		rx_train_len;
	int		rx_train_err;
	char	freeze_rate;
	char	last_page;
	char	mode;
	char	horizontal;
	char	pass;
	char	eol;
	char	black;
	char	bcnt;
	char	last_error;
	char	eolcnt;
	char	eolcnt2;
	uint	zcnt;
	ushort	pcnt;
	ushort	bseq;
	uint	imagetime;
	int		pageno;
	int		blockno;
	int		frameno;
	int		framecnt;
	int		framelen;
	int		frameptr;
	int		maxframe;
	char	badframe;
	char	wait_t1;
	char	wait_t5;
	char	wait_cfr;
	char	wait_mcf;
	char	wait_ctr;
	char	wait_train_tcf;
	char	wait_image;
	char	wait_post_msg;
	char	wait_dcn;
	char	wait_lost_packet;
	char	fax_error;
	char	crp;
	char	delay_retrans;
	char	reserved[106];
}fax_state;


typedef struct{
	int		max_fax_process;
	int		fax_in_process;
	int		ipc_port_pm;
	int		ipc_port_ema;
	int		ipc_port_fax;
	int		alloc_device_hunting;
	int		base_fax_port;	
	int		fax_port_spacing;
	int		dbg_level;
	int		log_keep_day;
	int		log_lib;
	int		dscp;
	int		train_err_rate_good;
	int		rtp_threshold;
	int		rtn_threshold;
	int		disable_tx_ecm;
	int		disable_rx_ecm;
	int		tx_compression_max;
	int		rx_compression_max;
	int		redundancy_level;
	int		store_raw_image;
	int		add_line_to_rx_blank;
	int		reserved[7];
}fax_cfg_base;

typedef struct{
	int	t5;
	int	dcs;
	int	cfr;
	int	mcf;
	int	ctr;
	int	tcf;
	int	image;
	int	post;
	int	dcn;
	int	reserved[7];
}fax_cfg_timer;

typedef struct{
	char	font[256];
	int		size;
}fax_cfg_overlay;

typedef struct{
	uint	fax[MAXFAXPROC];
}fax_cfg_affinity;

typedef struct{
	fax_cfg_base	 fax;
	fax_cfg_timer	 timer;
	fax_cfg_overlay	 overlay;
	fax_cfg_affinity aff;
	int		next_fax;
	int		trace_t4;
}fax_cfg_set;

typedef struct{
	int		fax;
	time_t	expire;
	int		test;
	int		reserve[4];
}fax_license;


typedef struct{
	int		pin[MAXFCHILD];
	int		dont_restart[MAXFCHILD];
	time_t	start_time[MAXFCHILD];
	uint	heart[MAXFCHILD];
	uint	use[MAXFAXPROC];
	fax_license lic;
	fax_cfg_set cfg;
	fax_media_info media;
	fax_state  fax[MAXFAX];
	int		open_fax[MAXFAX];
	int		reset_flag[MAXFAX];
	int		trace_fax[MAXFAX];
	struct sockaddr_in from[MAXFAX];
	fax_open_state own[MAXFAX];
	int		spwp[MAXFAX];
	int		sprp[MAXFAX];
	char	spbuf[MAXFAX][MAXFBUF];
	int		cdr_wrp;
	int		cdr_rdp;
	fax_recent_cdr cdr[MAXFCDR];
	int		lwp[MAXFAXPROC];
	int		lrp[MAXFAXPROC];
	char	logbuf[LOGFBUFLEN*MAXFAX];
}fax_share;


typedef struct{
	char	order[2];
	ushort	signature;
	uint	offset;
}tiff_header;

typedef struct{
	int		last;
	int		retrans;
}ipc_fax_page;


#ifdef __cplusplus
}
#endif

#endif
