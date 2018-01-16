#pragma once

#include <list>
#include <string.h>
#include "DcmDataSet.h"
#include "DcmDataTable.h"
#include "DcmDataColumn.h"
#include "DcmDataRow.h"
#include "DcmDataCell.h"

#include "Structures.h"
#include "iJson.h"

// json fields define.
#define FIELD_CONNECT_TIME			"connect_time"
#define FIELD_SESSION_KEY			"session_key"
#define FIELD_SVR_GRP				"svr_grp"
#define FIELD_MODULE_ID				"module_id"
#define FIELD_MODULE_NAME			"module_name"
#define FIELD_REG_TIME				"reg_time"
#define FIELD_RESP_RESULT			"resp_result"
#define FIELD_RESULT				"result"
#define FIELD_REASON				"reason"
#define FIELD_ERRMSG				"errmsg"
#define FIELD_READY_TIME			"ready_time"
#define FIELD_STOP_TIME				"stop_time"
#define FIELD_DATA_SET_CNT			"data_set_cnt"
#define FIELD_DATA_SET_LIST			"data_set_list"
#define FIELD_DATA_SET_ID			"data_set_id"
#define FIELD_COLUMNS				"columns"
#define FIELD_DATA_SET_TOTCNT		"data_set_totcnt"
#define FIELD_FROM_SEQ				"from_seq"
#define FIELD_TO_SEQ				"to_seq"
#define FIELD_DATA					"data"
#define FIELD_INS_TOTAL_CNT			"ins_total_cnt"
#define FIELD_INS_FAIL_CNT			"ins_fail_cnt"
#define FIELD_UPDATE_DATA			"update_data"
#define FIELD_DELETE_DATA			"delete_data"
#define FIELD_DATA_UPDATE_FAIL_CNT	"data_update_fail_cnt"
#define FIELD_DATA_DELETE_FAIL_CNT	"data_delete_fail_cnt"
#define FIELD_SYSTEM_ID				"system_id"
#define FIELD_NODE_ID				"node_id"
#define FIELD_TIME					"time"
#define FIELD_SEQ					"seq"

class CJsonAPI : public iJson
{
public:
	CJsonAPI(void);
	~CJsonAPI(void);

	size_t				MakeEmsDcmHeartBeat(NPSTR& jsonStr, int p_seq, string p_procName, int p_procId);
	size_t				MakeEmsDcmDataSetEnd(NPSTR& jsonStr, string p_svcGroup, string p_procName, int p_procId, CDcmDataSet* p_dataset);
	
	size_t				MakeEmsDcmDataSetInfo(NPSTR& jsonStr, string p_svcGroup, string p_procName, int p_procId, CDcmDataTable* p_dataTable);
	size_t				MakeEmsDcmDataSetStart(NPSTR& jsonStr , string p_svcGroup, string p_procName, int p_procId, CDcmDataSet* p_dataset);
	size_t				MakeEmsDcmStopRtdResp(NPSTR& jsonStr, string p_svcGroup, string p_procName, int p_procId, int p_result, int p_reason, string p_msg );
	size_t				MakeEmsDcmReadyResp(NPSTR& p_jsonStr, string p_svcGroup, string p_procName, int procId, int p_result, int p_reason, string p_msg);
	size_t				MakeEmsDsmRegister(NPSTR& p_jsonStr, string p_svcGroup, int p_procId, string p_procName);
	size_t				MakeEmsDcmBulkStart(NPSTR& p_jsonStr, string p_svcGroup, int p_procId, string p_procName);
	size_t				MakeEmsDcmBulkData(NPSTR& jsonStr, string p_svcGroup, string p_procName, int p_procId, CDcmDataTable* p_dataTable);
	size_t				MakeEmsDcmBulkData(NPSTR& jsonStr, string p_svcGroup, string p_procName, int p_procId, CDcmDataTable* p_dataTable, int p_startIdx, int p_endIdx);
	size_t				MakeEmsDcmBulkEnd(NPSTR& p_jsonStr, string p_svcGroup, int p_procId, string p_procName);
	size_t				MakeEmsDsmRtData(NPSTR& jsonStr, string p_svcGroup, string p_procName, int p_procId, CDcmDataTable* p_dataTable);
	size_t				MakeEmsDsmRtData(NPSTR& jsonStr, string p_svcGroup, string p_procName, int p_procId, CDcmDataTable* p_dataTable, int p_startIdx, int p_endIdx);
	bool				GetEmsDsmRegister_Response(EMS_RESULT_INFO_* p_result, char* p_jsStr);
	bool				GetResult(EMS_RESULT_INFO_* p_result, char* p_jsStr);
	int					GetSessionKey(char* p_jsStr);
	bool				TimeToStr(char* p_str);
};

