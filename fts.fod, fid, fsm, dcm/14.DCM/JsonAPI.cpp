#include "StdAfx.h"
#include "JsonAPI.h"
#include <iostream>
#include <string.h>
#include <list>
#include <vector>
#include <iLib.h>

CJsonAPI::CJsonAPI(void)
{
}


CJsonAPI::~CJsonAPI(void)
{
}

//--------------------------------------------------------
// Title	: MakeEmsDcmHeartBeat
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 1. Heart beat
//			  2. 20초간 응답이 없으면 Disconnect
//			  3. sequence 는 1~65500 도달하면 0으로 리셋
//--------------------------------------------------------
size_t	CJsonAPI::MakeEmsDcmHeartBeat(NPSTR& jsonStr, int p_seq, string p_procName, int p_procId)
{
	try
	{
		iJson js;
		char szTime[15];
		TimeToStr(szTime);
		js.SetItemStr(FIELD_TIME		, szTime);
		js.SetItemInt(FIELD_SEQ			, p_seq);
		js.SetItemInt(FIELD_MODULE_ID	, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		return js.AlocJson(0, &jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}


//--------------------------------------------------------
// Title	: MakeEmsDcmReadyResp
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : dcm ready packet.
//--------------------------------------------------------
size_t CJsonAPI::MakeEmsDcmReadyResp(NPSTR& p_jsonStr
									, string p_svcGroup
									, string p_procName
									, int procId
									, int p_result
									, int p_reason
									, string p_msg)
{
	try
	{
		iJson js, jsItem;
		js.SetItemStr(FIELD_SVR_GRP		, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID	, procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		jsItem.SetItemInt(FIELD_RESULT	, p_result);
		jsItem.SetItemInt(FIELD_REASON	, p_reason);
		jsItem.SetItemStr(FIELD_ERRMSG	, p_msg.c_str());	
		js.SetItem(FIELD_RESP_RESULT	, jsItem);

		return js.AlocJson(0, &p_jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}

//--------------------------------------------------------
// Title	: MakeEmsDcmStopRtdResp
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 실시간데이터 요청 중지 전문.
//--------------------------------------------------------
size_t CJsonAPI::MakeEmsDcmStopRtdResp(NPSTR& jsonStr
										, string p_svcGroup
										, string p_procName
										, int p_procId
										, int p_result
										, int p_reason
										, string p_msg )
{
	try
	{
		iJson js, jsItem;

		js.SetItemStr(FIELD_SVR_GRP		, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID	, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		jsItem.SetItemInt(FIELD_RESULT	, p_result);
		jsItem.SetItemInt(FIELD_REASON	, p_reason);
		jsItem.SetItemStr(FIELD_ERRMSG	, p_msg.c_str());
		js.SetItem(FIELD_RESP_RESULT	, jsItem);
		return js.AlocJson(0, &jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}

//--------------------------------------------------------
// Title	: MakeEmsDcmDataSetStart
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Dataset List 요청
//--------------------------------------------------------
size_t CJsonAPI::MakeEmsDcmDataSetStart(NPSTR& jsonStr
										, string p_svcGroup
										, string p_procName
										, int p_procId
										, CDcmDataSet* p_dataset
										)
{
	try
	{
		iJson js, jsArray, jsArrayItem;

		js.SetItemStr(FIELD_SVR_GRP		, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID	, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		jsArray.Reset(eJsArr);

		list<CDcmDataTable*>::iterator itor;
		for(itor= p_dataset->m_dataTables.begin(); itor != p_dataset->m_dataTables.end(); itor++)
		{	
			jsArrayItem.SetStr((*itor)->m_dataTableName.c_str());
			jsArray.PushItem(jsArrayItem);
		}
	
		js.SetItemInt(FIELD_DATA_SET_CNT, jsArray.GetCnt());
		js.SetItem(FIELD_DATA_SET_LIST	, jsArray);

		return js.AlocJson(0, &jsonStr);
	}
	catch(DWORD dwError)
	{
		return 0;
	}
}

//--------------------------------------------------------
// Title	: MakeEmsDcmDataSetEnd
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Dataset List 종료
//--------------------------------------------------------
size_t CJsonAPI::MakeEmsDcmDataSetEnd(NPSTR& jsonStr
										, string p_svcGroup
										, string p_procName
										, int p_procId
										, CDcmDataSet* p_dataset
										)
{
	return MakeEmsDcmDataSetStart(jsonStr, p_svcGroup, p_procName, p_procId, p_dataset);
}

//--------------------------------------------------------
// Title	: MakeEmsDcmDataSetStart
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Dataset List 요청
//--------------------------------------------------------
size_t CJsonAPI::MakeEmsDcmDataSetInfo(NPSTR& jsonStr
										, string p_svcGroup
										, string p_procName
										, int p_procId
										, CDcmDataTable* p_dataTable
										)
{
	try
	{
		iJson js, jsArray, jsArrayItem;
		js.SetItemStr(FIELD_SVR_GRP		, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID	, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		js.SetItemStr(FIELD_DATA_SET_ID	, p_dataTable->m_dataTableName.c_str());
		jsArray.Reset(eJsArr);

		list<CDcmDataColumn*>::iterator itor;
		for(itor= p_dataTable->m_dataColumns.begin(); itor != p_dataTable->m_dataColumns.end(); itor++)
		{	
			jsArrayItem.SetStr((*itor)->m_columnName.c_str());
			jsArray.PushItem(jsArrayItem);
		}
	
		js.SetItem(FIELD_COLUMNS, jsArray);
		return js.AlocJson(0, &jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}

//--------------------------------------------------------
// Title	: MakeEmsDsmRegister
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 프로세스 Register
//--------------------------------------------------------
size_t	CJsonAPI::MakeEmsDsmRegister(NPSTR& p_jsonStr, string p_svcGroup, int p_procId, string p_procName)
{
	try
	{
		iJson js;
		char szTime[15];
		TimeToStr(szTime);

		js.SetItemStr(FIELD_SVR_GRP		, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID	, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		js.SetItemStr(FIELD_REG_TIME, szTime);

		return js.AlocJson(0, &p_jsonStr);
	}
	catch(DWORD)
	{
		return 0;
	}
}

//--------------------------------------------------------
// Title	: MakeEmsDcmBulkStart
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : BULK 데이터 송신 시작
//--------------------------------------------------------
size_t	CJsonAPI::MakeEmsDcmBulkStart(NPSTR& p_jsonStr, string p_svcGroup, int p_procId, string p_procName)
{
	try
	{
		iJson js;
		char szTime[15];
		TimeToStr(szTime);
	
		js.SetItemStr(FIELD_SVR_GRP		, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID	, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		js.SetItemStr(FIELD_REG_TIME	, szTime);

		return js.AlocJson(0, &p_jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}

//--------------------------------------------------------
// Title	: MakeEmsDcmBulkData
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 벌크데이터 생성.
//--------------------------------------------------------
size_t CJsonAPI::MakeEmsDcmBulkData(NPSTR& jsonStr
										, string p_svcGroup
										, string p_procName
										, int p_procId
										, CDcmDataTable* p_dataTable
										)
{
	try
	{
		iJson js, jsArray, jsSubArray, jsArrayItem;
		char	strData[200];

		js.SetItemStr(FIELD_SVR_GRP			, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID		, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME		, p_procName.c_str());
		js.SetItemStr(FIELD_DATA_SET_ID		, p_dataTable->m_dataTableName.c_str());
		js.SetItemInt(FIELD_DATA_SET_TOTCNT	, p_dataTable->GetBulkDataRowCount());
		js.SetItemInt(FIELD_FROM_SEQ		, 0);
		js.SetItemInt(FIELD_TO_SEQ			, p_dataTable->GetBulkDataRowCount() - 1 );
		jsArray.Reset(eJsArr);

		list<CDcmDataRow*>::iterator rowItor;
		list<CDcmDataCell*>::iterator cellItor;
		for(rowItor= p_dataTable->m_dataRows.begin(); rowItor != p_dataTable->m_dataRows.end(); rowItor++)
		{	
			CDcmDataRow* pRow = (*rowItor);
			
			// 삭제할 행이라면 continue
			if(pRow->m_doDelete)
				continue;

			// 벌크 로우가 아니라면 continue
			if(!pRow->m_isBulkRow)
				continue;

			jsSubArray.Reset(eJsArr);
			for(cellItor= pRow->m_dataCells.begin(); cellItor != pRow->m_dataCells.end(); cellItor++)
			{	
				CDcmDataCell* pCell = (*cellItor);

				switch(pCell->m_columnType)
				{
					case DcmColumnType_Integer :
						jsArrayItem.SetInt(pCell->GetValueInt());
						break;

					case DcmColumnType_String :
						memset(strData, 0x00, sizeof(strData));
						pCell->GetValueStr(strData);
						jsArrayItem.SetStr(strData);
						break;

					case DcmColumnType_Long	:
						jsArrayItem.SetInt((int)pCell->GetValueLong());
						break;

					default :
						continue;
				}

				jsSubArray.PushItem(jsArrayItem);
			}

			pRow->m_doDelete = false;
			jsArray.PushItem(jsSubArray);
		}
	
		js.SetItem(FIELD_DATA, jsArray);
		return js.AlocJson(0, &jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}


//--------------------------------------------------------
// Title	: MakeEmsDcmBulkData
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 벌크데이터 생성.
//--------------------------------------------------------
size_t CJsonAPI::MakeEmsDcmBulkData(NPSTR& jsonStr
										, string p_svcGroup
										, string p_procName
										, int p_procId
										, CDcmDataTable* p_dataTable
										, int p_startIdx
										, int p_endIdx
										)
{
	try
	{
		iJson js, jsArray, jsSubArray, jsArrayItem;
		char	strData[200];

		js.SetItemStr(FIELD_SVR_GRP			, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID		, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME		, p_procName.c_str());
		js.SetItemStr(FIELD_DATA_SET_ID		, p_dataTable->m_dataTableName.c_str());
		js.SetItemInt(FIELD_DATA_SET_TOTCNT	, p_dataTable->GetBulkDataRowCount());		
		// 0 base
		js.SetItemInt(FIELD_FROM_SEQ		, p_startIdx);
		js.SetItemInt(FIELD_TO_SEQ			, p_endIdx - 1);
		jsArray.Reset(eJsArr);

		list<CDcmDataRow*>::iterator rowItor;
		list<CDcmDataCell*>::iterator cellItor;
		int idx = -1;
		for(rowItor= p_dataTable->m_dataRows.begin(); rowItor != p_dataTable->m_dataRows.end(); rowItor++)
		{	
			idx++;
			if(idx < p_startIdx || idx > p_endIdx -1)
				continue;

			CDcmDataRow* pRow = (*rowItor);
			
			// 삭제할 행이라면 continue
			if(pRow->m_doDelete)
				continue;

			// 벌크 로우가 아니라면 continue
			if(!pRow->m_isBulkRow)
				continue;

			jsSubArray.Reset(eJsArr);
			for(cellItor= pRow->m_dataCells.begin(); cellItor != pRow->m_dataCells.end(); cellItor++)
			{	
				CDcmDataCell* pCell = (*cellItor);

				switch(pCell->m_columnType)
				{
					case DcmColumnType_Integer :
						jsArrayItem.SetInt(pCell->GetValueInt());
						break;

					case DcmColumnType_String :
						memset(strData, 0x00, sizeof(strData));
						pCell->GetValueStr(strData);
						jsArrayItem.SetStr(strData);
						break;

					case DcmColumnType_Long	:
						jsArrayItem.SetInt((int)pCell->GetValueLong());
						break;

					default :
						continue;
				}

				jsSubArray.PushItem(jsArrayItem);
			}

			pRow->m_doDelete = false;
			jsArray.PushItem(jsSubArray);
		}
	
		js.SetItem(FIELD_DATA, jsArray);
		return js.AlocJson(0, &jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}


//--------------------------------------------------------
// Title	: MakeEmsDsmRtData
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 실시간데이터 생성.
//--------------------------------------------------------
size_t CJsonAPI::MakeEmsDsmRtData(NPSTR& jsonStr
										, string p_svcGroup
										, string p_procName
										, int p_procId
										, CDcmDataTable* p_dataTable
										)
{
	try
	{
		iJson js, jsArray, jsSubArray, jsArrayItem;
		char	strData[200];

		js.SetItemStr(FIELD_SVR_GRP		, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID	, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		js.SetItemStr(FIELD_DATA_SET_ID	, p_dataTable->m_dataTableName.c_str());
		jsArray.Reset(eJsArr);

		bool ret			= true;
		int nRet			= -1;
		CDcmDataRow* pRow	= NULL;
		CDcmDataCell* pCell = NULL;

		list<CDcmDataRow*>::iterator rowItor;
		for(rowItor= p_dataTable->m_dataRows.begin(); rowItor != p_dataTable->m_dataRows.end(); rowItor++)
		{	
			pRow = (*rowItor);
			
			// 삭제할 행이라면 continue
			if(pRow->m_doDelete)
				continue;

			// 벌크 로우라면 continue
			if(pRow->m_isBulkRow)
				continue;

			jsSubArray.Reset(eJsArr);
			list<CDcmDataCell*>::iterator cellItor;
			for(cellItor= pRow->m_dataCells.begin(); cellItor != pRow->m_dataCells.end(); cellItor++)
			{
				pCell = (*cellItor);

				switch((*cellItor)->m_columnType)
				{
					case DcmColumnType_Integer :
						jsArrayItem.SetInt(pCell->GetValueInt());
						break;

					case DcmColumnType_String :
						memset(strData, 0x00, sizeof(strData));
						pCell->GetValueStr(strData);
						ret = jsArrayItem.SetStr(strData);
						break;

					case DcmColumnType_Long	:
						jsArrayItem.SetInt((int)pCell->GetValueLong());	
						break;

					default :
						continue;
				}			
				jsSubArray.PushItem(jsArrayItem);
			}

			pRow->m_doDelete = true;
			jsArray.PushItem(jsSubArray);			
		}
	
		js.SetItem(FIELD_UPDATE_DATA, jsArray);
		return js.AlocJson(0, &jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}


//--------------------------------------------------------
// Title	: MakeEmsDsmRtData
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 실시간데이터 생성.
//--------------------------------------------------------
size_t CJsonAPI::MakeEmsDsmRtData(NPSTR& jsonStr
										, string p_svcGroup
										, string p_procName
										, int p_procId
										, CDcmDataTable* p_dataTable
										, int p_startIdx
										, int p_endIdx
										)
{
	try
	{
		iJson js, jsArray, jsSubArray, jsArrayItem;
		char	strData[100];

		js.SetItemStr(FIELD_SVR_GRP		, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID	, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		js.SetItemStr(FIELD_DATA_SET_ID	, p_dataTable->m_dataTableName.c_str());
		jsArray.Reset(eJsArr);

		bool ret			= true;
		int nRet			= -1;
		int idx				= -1;
		CDcmDataRow* pRow	= NULL;
		CDcmDataCell* pCell = NULL;

		int bulkRowCnt = p_dataTable->GetBulkDataRowCount();
		list<CDcmDataRow*>::iterator rowItor;
		for(rowItor= p_dataTable->m_dataRows.begin(); rowItor != p_dataTable->m_dataRows.end(); rowItor++)
		{	
			idx++;
			if(idx < bulkRowCnt + p_startIdx || idx > (bulkRowCnt + p_endIdx) -1)
				continue;

			pRow = (*rowItor);
			
			// 삭제할 행이라면 continue
			if(pRow->m_doDelete)
				continue;

			// 벌크 로우라면 continue
			if(pRow->m_isBulkRow)
				continue;

			jsSubArray.Reset(eJsArr);
			list<CDcmDataCell*>::iterator cellItor;
			for(cellItor= pRow->m_dataCells.begin(); cellItor != pRow->m_dataCells.end(); cellItor++)
			{
				pCell = (*cellItor);

				switch((*cellItor)->m_columnType)
				{
					case DcmColumnType_Integer :
						jsArrayItem.SetInt(pCell->GetValueInt());
						break;

					case DcmColumnType_String :
						memset(strData, 0x00, sizeof(strData));
						pCell->GetValueStr(strData);
						ret = jsArrayItem.SetStr(strData);
						break;

					case DcmColumnType_Long	:
						jsArrayItem.SetInt((int)pCell->GetValueLong());	
						break;

					default :
						continue;
				}			
				jsSubArray.PushItem(jsArrayItem);
			}

			pRow->m_doDelete = true;
			jsArray.PushItem(jsSubArray);			
		}
	
		js.SetItem(FIELD_UPDATE_DATA, jsArray);
		return js.AlocJson(0, &jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}


//--------------------------------------------------------
// Title	: MakeEmsDcmBulkStart
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : BULK 데이터 송신 시작
//--------------------------------------------------------
size_t	CJsonAPI::MakeEmsDcmBulkEnd(NPSTR& p_jsonStr, string p_svcGroup, int p_procId, string p_procName)
{
	try
	{
		iJson js;
		char szTime[15];
		TimeToStr(szTime);

		js.SetItemStr(FIELD_SVR_GRP		, p_svcGroup.c_str());
		js.SetItemInt(FIELD_MODULE_ID	, p_procId);
		js.SetItemStr(FIELD_MODULE_NAME	, p_procName.c_str());
		js.SetItemStr(FIELD_REG_TIME	, szTime);

		return js.AlocJson(0, &p_jsonStr);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}

//--------------------------------------------------------
// Title	: GetEmsDsmRegister_Response
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 1. EmsDsmRegister 에대한 처리결과를 얻음.
//			  2. DSM ---> DCM
//--------------------------------------------------------
bool CJsonAPI::GetEmsDsmRegister_Response(EMS_RESULT_INFO_* p_result, char* p_jsStr)
{	
	return GetResult(p_result, p_jsStr);
}

//--------------------------------------------------------
// Title	: GetResult
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : json 문자열 파싱후 결과 얻음.
//--------------------------------------------------------
bool CJsonAPI::GetResult(EMS_RESULT_INFO_* p_result, char* p_jsStr)
{
	try
	{
		EMS_RESULT_INFO_ *stRet = p_result;
		iJson js, respJs;
		if(!js.SetJson(p_jsStr))
			return false;
	
		js.GetItem(FIELD_RESP_RESULT, respJs);
		stRet->result = respJs.GetItemInt(FIELD_RESULT);
		stRet->reason = respJs.GetItemInt(FIELD_REASON);
		strcpy(stRet->errmsg, respJs.GetItemStr(FIELD_ERRMSG));

		stRet->session_key				= js.GetItemInt(FIELD_SESSION_KEY);
		stRet->module_id				= js.GetItemInt(FIELD_MODULE_ID);
		stRet->data_set_cnt				= js.GetItemInt(FIELD_DATA_SET_CNT);
		stRet->ins_total_cnt			= js.GetItemInt(FIELD_INS_TOTAL_CNT);
		stRet->ins_fail_cnt				= js.GetItemInt(FIELD_INS_FAIL_CNT);
		stRet->data_update_fail_cnt		= js.GetItemInt(FIELD_DATA_UPDATE_FAIL_CNT);
		stRet->data_delete_fail_cnt		= js.GetItemInt(FIELD_DATA_DELETE_FAIL_CNT);

		strcpy(stRet->connect_time,		js.GetItemStr(FIELD_CONNECT_TIME));
		strcpy(stRet->svr_grp,			js.GetItemStr(FIELD_SVR_GRP));
		strcpy(stRet->module_name,		js.GetItemStr(FIELD_MODULE_NAME));
		strcpy(stRet->ready_time,		js.GetItemStr(FIELD_READY_TIME));
		strcpy(stRet->stop_time,		js.GetItemStr(FIELD_STOP_TIME));
		strcpy(stRet->data_set_list,	js.GetItemStr(FIELD_DATA_SET_LIST));
		strcpy(stRet->data_set_id,		js.GetItemStr(FIELD_DATA_SET_ID));

		return true;
	}
	catch(DWORD ex)
	{
		return 0;
	}
}

//--------------------------------------------------------
// Title	: GetSessionKey
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 1. 세션키얻음.
//			  2. DSM ---> DCM
//--------------------------------------------------------
int CJsonAPI::GetSessionKey(char* p_jsStr)
{
	try
	{
		int sessionKey;
		iJson js;
		js.SetJson(p_jsStr);
		sessionKey = js.GetItemInt(FIELD_SESSION_KEY);
		return (sessionKey==0 ? -1 : sessionKey);
	}
	catch(DWORD ex)
	{
		return 0;
	}
}

//--------------------------------------------------------
// Title	: TimeToStr
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 시간을 문자열로 변환
//--------------------------------------------------------
bool CJsonAPI::TimeToStr(char* p_str)
{
	time_t	tTime;
	struct tm* _tm;

	time( &tTime );
	_tm = localtime(&tTime);

	sprintf( p_str,	"%04d%02d%02d%02d%02d%02d",
					_tm->tm_year+1900, _tm->tm_mon+1, _tm->tm_mday,
					_tm->tm_hour, _tm->tm_min, _tm->tm_min );

	return true;
}

