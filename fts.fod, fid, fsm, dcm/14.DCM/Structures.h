#ifndef STRUCTURES_H_
#define STRUCTURES_H_

typedef		int				key_t;

// EMS version Info
#define EMS_STX_			"BTEMS"
#define EMS_MAJOR_VER		10
#define EMS_MINOR_VER		0
#define EMS_RELEASE_VER		0

// EMS common info
#define	EMS_DSM_PKT_SIZE_			4096
#define	EMS_DSM_PKT_BODYSIZE_		(4096 - sizeof(EMS_PACKET_HEADER_))
#define EMS_MAX_ERROR_MSG_			4096



typedef struct EMS_PACKET_HEADER_
{
	EMS_PACKET_HEADER_()		{;}
	~EMS_PACKET_HEADER_()	{;}
	char					stx[5];
	char					Ver[3];
	unsigned int			nLen;
	unsigned int			nCmdID;
	unsigned short			nSessionKey;
	unsigned short			nSeq;
	unsigned short			nSysFrom;		//system ID
	unsigned short			nSysTo;
	unsigned int			nFrom;			//Mo ID
	unsigned int			nTo;
};

typedef struct EMS_RESULT_INFO_
{
	EMS_RESULT_INFO_() 
	{
		Clear();
	}
	~EMS_RESULT_INFO_() {;}
	int result;							
	int reason;							
	char errmsg[EMS_MAX_ERROR_MSG_];
	
	// extension infos
	int session_key;				// 세션 키
	int module_id;					// 모듈아이디
	int data_set_cnt;				// 데이터셋 개수
	int ins_total_cnt;				// Insert 요청건수
	int ins_fail_cnt;				// Insert 샐패건수
	int data_update_fail_cnt;		// 모니터링 데이터 저장 실패 건수
	int data_delete_fail_cnt;		// 모니터링 데이터 삭제 실패 건수
	char connect_time[15];			// DSM 접속 일시
	char svr_grp[10];				// 서비스 그룹
	char module_name[20];			// 모듈명
	char ready_time[15];			// DSM 준비 일시
	char stop_time[15];				// 모니터링 중지 일시
	char data_set_list[1024];		// 데이터셋 목록(테이블 Row 개수)
	char data_set_id[40];			// 데이터셋 아이디(테이블명)
	
	void Clear()
	{
		result = 0;
		reason = 0;
		strcpy(errmsg, "");

		session_key = -1;
		strcpy(connect_time, "");

		strcpy(svr_grp, "");
		module_id = -1;
		strcpy(module_name, "");

		strcpy(ready_time, "");
		strcpy(stop_time, "");
		data_set_cnt =-1;
		strcpy(data_set_list, "");

		strcpy(data_set_id, "");
		
		ins_total_cnt = -1;
		ins_fail_cnt = -1;

		data_update_fail_cnt = -1;
		data_delete_fail_cnt = -1;

	}
};

#endif