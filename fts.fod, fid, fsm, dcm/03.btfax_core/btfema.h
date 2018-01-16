#ifndef BTFEMA_H
#define BTFEMA_H

#ifdef __cplusplus
extern "C" {
#endif


enum{
	Oam_Fax_Keep_Alive,
	Oam_Fax_Process_State,
	Oam_Fax_Process_Start,
	Oam_Fax_Process_Stop,
	Oam_Fax_Process_Restart,
	Oam_Fax_Reload_Config,
	Oam_Fax_Get_Config,
	Oam_Fax_Owner,
	Oam_Fax_State,
	Oam_Fax_Recent_Cdr,
};


typedef struct{
	int		msgid;
	int		err;
	int		cont;
	int		chan;
	int		len;
	char	para[4076];
}oam_fax_msg;

typedef struct{
	char	prcname[32];
	int		instance;
	uint	start_time;
}oam_fax_process_state;

typedef struct{
	char	prcname[32];
	int		instance;
}oam_fax_process_control;

typedef struct{
	int		chan;
	int		pid;
	char	cmd[4];
}oam_fax_open_state;

typedef struct{
	int		chan;
	ipc_fax_state f;
}oam_fax_state;

typedef fax_recent_cdr oam_fax_recent_cdr;


#ifdef __cplusplus
}
#endif

#endif
