#include "StdAfx.h"
#include <windows.h>
#include <iostream>
#include "Structures.h"
#include "DsmIfSession.h"
#include "DcmDataTable.h"


CDsmIfSession* CDsmIfSession::s_pInstance = NULL;

CDsmIfSession* CDsmIfSession::Inst()
{
	if( s_pInstance == NULL )
		s_pInstance = new CDsmIfSession;

	return s_pInstance;
}


CDsmIfSession::CDsmIfSession(void)
{	
	m_dataSet = new CDcmDataSet("DATA_SET");
	m_dsmIp = "";
	m_dsmPort = 0;
	m_sessionKey = -1;
	m_isConnected = false;
	m_heartBeatSeq = 0;

	StartHeartBit(false);
}


CDsmIfSession::~CDsmIfSession(void)
{
	m_jsonAPI = NULL;
	delete m_jsonAPI;

	m_dataSet = NULL;
	delete m_dataSet;
	StopAll();
}


//--------------------------------------------------------
// Title	: SetDataSet
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : DSM과 IF를 위한 데이터셋 설정
//--------------------------------------------------------
void CDsmIfSession::SetDataSet(CDcmDataSet* p_dataSet)
{
	m_dataSet = p_dataSet;
}


//--------------------------------------------------------
// Title	: SetAddress
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : DSM과 IF를 위한 아이피, 포트 설정
//--------------------------------------------------------
void CDsmIfSession::SetAddress(char* p_ip, int p_port)
{
	m_dsmIp = p_ip;
	m_dsmPort = p_port;
}


//--------------------------------------------------------
// Title	: SetAddress
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : DSM과 IF를 위한 시스템ID, 프로세스ID 설정
//--------------------------------------------------------
void CDsmIfSession::SetInfo(char* p_serviceGroup
							, char* p_systemName
							, int p_systemId
							, char* p_procName
							, int p_procId
							)
{
	m_serviceGroup	= p_serviceGroup;
	m_systemName	= p_systemName;
	m_systemId		= p_systemId;
	m_processName	= p_procName;
	m_processId		= p_procId;
}


//--------------------------------------------------------
// Title	: Run
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 세션 시작
//--------------------------------------------------------
void CDsmIfSession::Run()
{
	// iThread 시작 : for send
	Start();
		
	iSleep(ONESEC);	
	
	// iThread 시작 : for receive
	m_receiveThread.Start( this, NULL, 0);

	// iThread 시작 : for heart bit
	m_heartBitThread.Start( this, NULL, 0);
}


//--------------------------------------------------------
// Title	: StopAll
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 모든스레드 중지.
//--------------------------------------------------------
void CDsmIfSession::StopAll()
{	
	m_receiveThread.Stop(ONESEC * 3);
	m_heartBitThread.Stop(ONESEC * 3);
	Stop(ONESEC * 3);
}


//--------------------------------------------------------
// Title	: SendHeartBit
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CDsmIfSession_HB 에서 호출
//--------------------------------------------------------
void CDsmIfSession::SendHeartBit()
{		
	if(!IsStartHeartBit())
		return;

	if(!IsRunning())
		return;

	// 연결상태 체크.
	if(!IsConnected())
	{
		LogErr("DSM Connection Fail.");
		return;
	}

	Handle_CMD_EMS_DCM_HEARTBEAT(CMD_EMS_DCM_HEARTBEAT);
}


//--------------------------------------------------------
// Title	: ThreadRun
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 세션 시작
//--------------------------------------------------------
NPVOID CDsmIfSession::ThreadRun(NPVOID p_arg)
{
	LogInfo("### DsmIfSession Thread Start ###");
	PacketCommand cmd;
	while(IsOn())
	{
		iSleep(ONESEC);
		
		if(!IsRunning())
			break;

		// 연결상태 체크.
		if(!IsConnected())
		{	
			Connect();
			continue;
		}
				
		bool result = false;
		cmd = GetPacketCommand();
		switch(cmd) {
			case CMD_EMS_PACKET_IDLE:
				result = Handle_CMD_EMS_PACKET_IDLE(cmd);
				break;
				
			case CMD_EMS_DCM_HEARTBEAT:
				result = Handle_CMD_EMS_DCM_HEARTBEAT(cmd);
				break;

			case CMD_EMS_DCM_REGISTER:
				result = Handle_CMD_EMS_DCM_REGISTER(cmd);
				break;

			case CMD_EMS_DCM_READY_RESP:
				result = Handle_CMD_EMS_DCM_READY_RESP(cmd);
				break;

			case CMD_EMS_DCM_STOP_RTD_RESP:
				result = Handle_CMD_EMS_DCM_STOP_RTD_RESP(cmd);
				SetPacketCommand(CMD_EMS_PACKET_IDLE);
				break;

			case CMD_EMS_DCM_DSET_START:
				result = Handle_CMD_EMS_DCM_DSET_START(cmd);
				break;

			case CMD_EMS_DCM_DSET_INFO:
				result = Handle_CMD_EMS_DCM_DSET_INFO(cmd);
				break;

			case CMD_EMS_DCM_DSET_END:
				result = Handle_CMD_EMS_DCM_DSET_END(cmd);
				break;

			case CMD_EMS_DCM_BULK_START:
				result = Handle_CMD_EMS_DCM_BULK_START(cmd);				 
				SetPacketCommand(CMD_EMS_DCM_BULK_DATA);
				break;

			case CMD_EMS_DCM_BULK_DATA: 
				result = Handle_CMD_EMS_DCM_BULK_DATA(cmd);
				SetPacketCommand(CMD_EMS_DCM_BULK_END);
				break;

			case CMD_EMS_DCM_BULK_END:
				result = Handle_CMD_EMS_DCM_BULK_END(cmd);
				SetPacketCommand(CMD_EMS_DCM_RT_MONIDATA);
				StartHeartBit(true);
				break;

			case CMD_EMS_DCM_RT_MONIDATA:
				result = Handle_CMD_EMS_DCM_RT_MONIDATA(cmd);
				break;

			case CMD_EMS_DCM_RT_RESET:
				LogWrn("CMD_EMS_DCM_RT_RESET Not supported packet command.");
				break; // Not Use

			default:
				LogWrn("Not supported packet command.");
				Disconnect();
				break;
		}
	}

	return 0;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_PACKET_IDLE
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_PACKET_IDLE 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_PACKET_IDLE(int p_currCmdId)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;	
	msg.Format("DCM >>> DSM [%s] | IDLE", strCmd);
	LogInfo((LPSTR)(LPCSTR)msg);
	
	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_HEARTBEAT
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DCM_HEARTBEAT 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_HEARTBEAT(int p_currCmdId)
{
	if(m_jsonAPI == NULL)
		return false;

	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;	
	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{	
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	SetAlive(false);

	char* jsonStr = NULL;
	IncreaseHeartBeatSeq();
	int len = m_jsonAPI->MakeEmsDcmHeartBeat(jsonStr, GetHeartBeatSeq(), m_processName, m_processId);
	if(len <=0 )
	{		
		msg.Format("DCM >>> DSM [%s] | MakeEmsDcmHeartBeat() error. length is zero.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	if(!SendJsonPacket(&header, jsonStr, len))
	{	
		msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
		LogErr((LPSTR)(LPCSTR)msg);
		iJson::FreeJson(jsonStr);
		return false;
	}	
	
	for(int i = 0; i<20 ; i++)
	{
		iSleep(ONESEC);
		if(IsAlive())
		{
			// success.
			//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
			msg.Format("DCM >>> DSM [%s] | Success.", strCmd);
			LogInfo((LPSTR)(LPCSTR)msg);
			iJson::FreeJson(jsonStr);
			return true;
		}
	}
		
	msg.Format("DCM >>> DSM [%s] | Not response. (%s)", strCmd, jsonStr);
	LogErr((LPSTR)(LPCSTR)msg);
	iJson::FreeJson(jsonStr);
	Disconnect();
	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_REGISTER
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DCM_REGISTER 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_REGISTER(int p_currCmdId)
{	
	if(m_jsonAPI == NULL)
		return false;

	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;	
	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{	
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	char* jsonStr = NULL;
	int len = m_jsonAPI->MakeEmsDsmRegister(jsonStr, m_serviceGroup, m_processId, m_processName);
	if(len <=0 )
	{
		msg.Format("DCM >>> DSM [%s] | MakeEmsDsmRegister() length is zero.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);		
		iJson::FreeJson(jsonStr);
		return false;
	}

	if(!SendJsonPacket(&header, jsonStr, len))
	{	
		msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
		LogErr((LPSTR)(LPCSTR)msg);
		iJson::FreeJson(jsonStr);
		return false;
	}	

	//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
	msg.Format("DCM >>> DSM [%s] | Success.", strCmd);
	LogInfo((LPSTR)(LPCSTR)msg);
	iJson::FreeJson(jsonStr);
	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_READY_RESP
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DCM_READY_RESP 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_READY_RESP(int p_currCmdId)
{	
	if(m_jsonAPI == NULL)
		return false;

	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;	
	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	char* jsonStr = NULL;
	int len = m_jsonAPI->MakeEmsDcmReadyResp(jsonStr, m_serviceGroup, m_processName, m_processId, 1, 0, "");
	if(len <=0 )
	{
		msg.Format("DCM >>> DSM [%s] | MakeEmsDcmReadyResp() length is zero.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);		
		return false;
	}

	if(!SendJsonPacket(&header, jsonStr, len))
	{
		iJson::FreeJson(jsonStr);
		msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}	

	//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
	msg.Format("DCM >>> DSM [%s] | Success.", strCmd);
	LogInfo((LPSTR)(LPCSTR)msg);
	iJson::FreeJson(jsonStr);

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_STOP_RTD_RESP
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DCM_STOP_RTD_RESP 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_STOP_RTD_RESP(int p_currCmdId)
{	
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	if(m_jsonAPI == NULL)
		return false;

	char* jsonStr = NULL;
	int len = m_jsonAPI->MakeEmsDcmStopRtdResp(jsonStr, m_serviceGroup, m_processName, m_processId, 1, 0, "");
	if(len <=0 )
	{
		msg.Format("DCM >>> DSM [%s] | MakeEmsDcmStopRtdResp() length is zero.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);		
		return false;
	}

	if(!SendJsonPacket(&header, jsonStr, len))
	{	
		msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
		LogErr((LPSTR)(LPCSTR)msg);
		iJson::FreeJson(jsonStr);
		return false;
	}	

	//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
	msg.Format("DCM >>> DSM [%s] | Success.", strCmd);
	LogInfo((LPSTR)(LPCSTR)msg);
	iJson::FreeJson(jsonStr);

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_DSET_START
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DCM_DSET_START 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_DSET_START(int p_currCmdId)
{	
	if(m_jsonAPI == NULL)
		return false;

	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	char* jsonStr = NULL;
	int len = m_jsonAPI->MakeEmsDcmDataSetStart(jsonStr, m_serviceGroup, m_processName, m_processId, m_dataSet);
	if(len <=0 )
	{
		msg.Format("DCM >>> DSM [%s] | MakeEmsDcmDataSetStart() length is zero.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);		
		return false;
	}

	if(!SendJsonPacket(&header, jsonStr, len))
	{
		msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
		LogErr((LPSTR)(LPCSTR)msg);
		iJson::FreeJson(jsonStr);
		return false;
	}	
	
	//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
	msg.Format("DCM >>> DSM [%s] | Success.", strCmd);
	LogInfo((LPSTR)(LPCSTR)msg);
	iJson::FreeJson(jsonStr);

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_DSET_INFO
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DCM_DSET_INFO 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_DSET_INFO(int p_currCmdId)
{	
	if(m_jsonAPI == NULL)
		return false;

	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	m_currTable = m_dataSet->GetNotRegistedTable();
	if(m_currTable == NULL)
	{	
		msg.Format("DCM >>> DSM [%s] | GetNotRegistedTable() GetNotRegistedTable error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	char* jsonStr = NULL;
	int len = m_jsonAPI->MakeEmsDcmDataSetInfo(jsonStr, m_serviceGroup, m_processName, m_processId, m_currTable);
	if(len <=0 )
	{
		msg.Format("DCM >>> DSM [%s] | MakeEmsDcmDataSetInfo() length is zero.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);		
		return false;
	}

	if(!SendJsonPacket(&header, jsonStr, len))
	{
		msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
		LogErr((LPSTR)(LPCSTR)msg);
		iJson::FreeJson(jsonStr);
		return false;
	}	

	//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
	msg.Format("DCM >>> DSM [%s] | Success.", strCmd);
	LogInfo((LPSTR)(LPCSTR)msg);
	iJson::FreeJson(jsonStr);

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_DSET_END
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DCM_DSET_END 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_DSET_END(int p_currCmdId)
{	
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{	
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	if(m_jsonAPI == NULL)
		return false;

	char* jsonStr = NULL;
	int len = m_jsonAPI->MakeEmsDcmDataSetEnd(jsonStr, m_serviceGroup, m_processName, m_processId, m_dataSet);
	if(len <=0 )
	{
		msg.Format("DCM >>> DSM [%s] | MakeEmsDcmDataSetEnd() length is zero.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);		
		return false;
	}

	if(!SendJsonPacket(&header, jsonStr, len))
	{
		msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
		LogErr((LPSTR)(LPCSTR)msg);
		iJson::FreeJson(jsonStr);
		return false;
	}	

	//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
	msg.Format("DCM >>> DSM [%s] | Success.", strCmd);
	LogInfo((LPSTR)(LPCSTR)msg);
	iJson::FreeJson(jsonStr);
	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_BULK_START
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 벌크(전체)데이터 전송 시작
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_BULK_START(int p_currCmdId)
{	
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);
		
	if(m_jsonAPI == NULL)
		return false;

	CString msg;
	char* jsonStr = NULL;
	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	int len = m_jsonAPI->MakeEmsDcmBulkStart(jsonStr, m_serviceGroup, m_processId, m_processName);
	if(len <=0 )
	{
		msg.Format("DCM >>> DSM [%s] | MakeEmsDcmBulkStart() length is zero.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);		
		return false;
	}

	if(!SendJsonPacket(&header, jsonStr, len))
	{
		msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
		LogErr((LPSTR)(LPCSTR)msg);
		iJson::FreeJson(jsonStr);
		return false;
	}	

	//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
	msg.Format("DCM >>> DSM [%s] | Success.", strCmd);
	LogInfo((LPSTR)(LPCSTR)msg);
	iJson::FreeJson(jsonStr);

	return true;
}

//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_BULK_DATA
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 벌크(전체)데이터 전송
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_BULK_DATA(int p_currCmdId)
{	
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	if(m_jsonAPI == NULL)
		return false;
    
	CString msg;
	char* jsonStr = NULL;
	EMS_PACKET_HEADER_ header;
	memset(&header, 0x00, sizeof(header));
	if(!MakePacketHeader(&header, p_currCmdId))
	{
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}
	
	int blockSize	= 10;
	int blockCnt	= 0;
	int block		= 0;

	int startIdx	= 0;
	int endIdx		= 0;
	int rowCnt		= 0;
	int len			= 0;
	
	
	list<CDcmDataTable*>::iterator iTable;
	for(iTable=m_dataSet->m_dataTables.begin(); iTable != m_dataSet->m_dataTables.end(); iTable++)
	{	
		if(m_jsonAPI == NULL)
			return false;

		rowCnt = (*iTable)->GetBulkDataRowCount();
		if(rowCnt <=0 )
			continue;

		// row를 10개씩 끊어서 보냄. (4k제한으로 인함.)
		blockCnt = (rowCnt / blockSize) + 1;		
		for(block = 0; block < blockCnt; block++)
		{
			startIdx	= block * blockSize;
			endIdx		= ((blockCnt - ((blockCnt-block) -1)) * blockSize);
			if(endIdx > rowCnt)
				endIdx = blockSize - (endIdx - rowCnt);

			if(endIdx <= 0)
				break;

			len = m_jsonAPI->MakeEmsDcmBulkData(jsonStr
												, m_serviceGroup
												, m_processName
												, m_processId
												, (*iTable)
												, startIdx
												, endIdx);
			if(len <= 0)
			{
				msg.Format("DCM >>> DSM [%s] | MakeEmsDcmBulkData() length is zero.", strCmd);
				LogErr((LPSTR)(LPCSTR)msg);						
				continue;
			}
		
			if(!SendJsonPacket(&header, jsonStr, len))
			{
				msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
				LogErr((LPSTR)(LPCSTR)msg);
				iJson::FreeJson(jsonStr);
				continue;
			}

			//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
			msg.Format("DCM >>> DSM [%s] | Success. TableName:[%s] idx:[%d-%d]", strCmd
																				 , (*iTable)->m_dataTableName.c_str()
																				 , startIdx
																				 , endIdx);
			LogInfo((LPSTR)(LPCSTR)msg);
			iJson::FreeJson(jsonStr);
			iSleep(ONESEC / 5);
		}
		
		iSleep(ONESEC / 2);
	}
		
	SetPacketCommand(CMD_EMS_DCM_RT_MONIDATA);
	return true;
}

//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_RT_MONIDATA
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DCM_RT_MONIDATA 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_RT_MONIDATA(int p_currCmdId)
{	
	if(m_jsonAPI == NULL)
		return false;

	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	int blockSize	= 10;
	int blockCnt	= 0;
	int block		= 0;
	
	int startIdx	= 0;
	int endIdx		= 0;
	int rowCnt		= 0;
	int len			= 0;
	
	char* jsonStr = NULL;	
	list<CDcmDataTable*>::iterator iTable;
	for(iTable=m_dataSet->m_dataTables.begin(); iTable != m_dataSet->m_dataTables.end(); iTable++)
	{			
		if((*iTable) == NULL)
			continue;

		rowCnt = (*iTable)->GetRTDataRowCount();
		if(rowCnt <= 0)
			continue;

		// row를 10개씩 끊어서 보냄. (4k제한으로 인함.)
		blockCnt = (rowCnt / blockSize) + 1;		
		for(block = 0; block < blockCnt; block++)
		{
			startIdx	= block * blockSize;
			endIdx		= ((blockCnt - ((blockCnt-block) -1)) * blockSize);
			if(endIdx > rowCnt)
				endIdx = blockSize - (endIdx - rowCnt);

			if(endIdx <= 0)
				break;

			len = m_jsonAPI->MakeEmsDsmRtData(jsonStr
											, m_serviceGroup
											, m_processName 
											, m_processId
											, (*iTable)
											, startIdx
											, endIdx);
			if(len <= 0)
			{
				msg.Format("DCM >>> DSM [%s] | MakeEmsDsmRtData() length is zero.", strCmd);
				LogErr((LPSTR)(LPCSTR)msg);						
				continue;
			}
		
			if(!SendJsonPacket(&header, jsonStr, len))
			{
				msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
				LogErr((LPSTR)(LPCSTR)msg);
				iJson::FreeJson(jsonStr);
				continue;
			}

			//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);			
			msg.Format("DCM >>> DSM [%s] | Success. TableName:[%s] idx:[%d-%d] \r\ndata=[%s]"
																				, strCmd
																				 , (*iTable)->m_dataTableName.c_str()
																				 , startIdx
																				 , endIdx
																				 , jsonStr
																				 );

			LogInfo((LPSTR)(LPCSTR)msg);
			iJson::FreeJson(jsonStr);
			iSleep(ONESEC / 5);
		}
	
		(*iTable)->ClearGarbageRows();
		iSleep(ONESEC / 5);
	}

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DCM_BULK_END
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DCM_BULK_END 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DCM_BULK_END(int p_currCmdId)
{	
	if(m_jsonAPI == NULL)
		return false;

	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	EMS_PACKET_HEADER_ header;
	if(!MakePacketHeader(&header, p_currCmdId))
	{
		msg.Format("DCM >>> DSM [%s] | MakePacketHeader() error.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}

	char* jsonStr = NULL;
	int len = m_jsonAPI->MakeEmsDcmBulkEnd(jsonStr, m_serviceGroup, m_processId, m_processName);
	if(len <=0 )
	{
		msg.Format("DCM >>> DSM [%s] | MakeEmsDcmBulkEnd() length is zero.", strCmd);
		LogErr((LPSTR)(LPCSTR)msg);			
		return false;
	}

	if(!SendJsonPacket(&header, jsonStr, len))
	{
		msg.Format("DCM >>> DSM [%s] | SendJsonPacket() error. (%s)", strCmd, jsonStr);
		LogErr((LPSTR)(LPCSTR)msg);
		iJson::FreeJson(jsonStr);
		return false;
	}	
		
	//msg.Format("DCM >>> DSM [%s] | Success. (%s)", strCmd, jsonStr);
	msg.Format("DCM >>> DSM [%s] | Success.", strCmd);
	LogInfo((LPSTR)(LPCSTR)msg);
	iJson::FreeJson(jsonStr);

	return true;
}


//--------------------------------------------------------
// Title	: StopReceive
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 전문수신 여부 설정.
//--------------------------------------------------------
void CDsmIfSession::StopReceive(bool p_wait)
{
	m_stopReceive = p_wait;
}


//--------------------------------------------------------
// Title	: IsStopReceive
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 수신중지 상태인지확인.
//--------------------------------------------------------
bool CDsmIfSession::IsStopReceive()
{
	return m_stopReceive;
}


//--------------------------------------------------------
// Title	: Receive
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 수신 -> CDsmIfSession_Recv에서 Call
//--------------------------------------------------------
void CDsmIfSession::Receive()
{	
	if(!IsConnected())
		return;

	int32 nRet, nRetry, iSelectRes;
	iSelectRes = RSelect(500);
	if(iSelectRes <= 0) 
		return;

	char szHeadBuf[sizeof(EMS_PACKET_HEADER_)];
	char szBodyBuf[EMS_DSM_PKT_BODYSIZE_];
	EMS_PACKET_HEADER_* pHdr = (EMS_PACKET_HEADER_*)szHeadBuf;

	memset(szHeadBuf, 0x00, sizeof(EMS_PACKET_HEADER_));
	memset(szBodyBuf, 0x00, EMS_DSM_PKT_BODYSIZE_);

	// header
	nRet = Recv(szHeadBuf, sizeof(*pHdr));
	if(nRet <= 0 || !CheckStx(pHdr))
		return;

	// body
	nRet = Recv(szBodyBuf, pHdr->nLen);
	if(nRet <= 0)
		return;

	// 수신 패킷 처리.
	HandleRecceivePacket(pHdr, szBodyBuf);
}


//--------------------------------------------------------
// Title	: HandleRecceivePacket
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : 수신 전문처리.
//--------------------------------------------------------
bool CDsmIfSession::HandleRecceivePacket( EMS_PACKET_HEADER_* pHeader
										, char* jsStr)
{	
	if(m_jsonAPI == NULL)
		return false;

	EMS_RESULT_INFO_ retInfo;
	if(!m_jsonAPI->GetResult(&retInfo, jsStr))
		return false;

	PacketCommand nextCommand = CMD_NULL;
	bool result = false;
	switch(pHeader->nCmdID)
	{
		case CMD_EMS_DSM_HEARTBEAT : 
			result = Handle_CMD_EMS_DSM_HEARTBEAT(retInfo, pHeader->nCmdID, &nextCommand);
			return true;

		case CMD_EMS_DSM_SESSIONKEY :
			result = Handle_CMD_EMS_DSM_SESSIONKEY(retInfo, pHeader->nCmdID, &nextCommand);
			break;

		case CMD_EMS_DSM_REGISTER_RESP :
			result = Handle_CMD_EMS_DSM_REGISTER_RESP(retInfo, pHeader->nCmdID, &nextCommand);
			break;

		case CMD_EMS_DSM_READY :
			result = Handle_CMD_EMS_DSM_READY(retInfo, pHeader->nCmdID, &nextCommand);
			break;                                                                      

		case CMD_EMS_DSM_STOP_RTD : 
			result = Handle_CMD_EMS_DSM_STOP_RTD(retInfo, pHeader->nCmdID, &nextCommand);
			break;

		case CMD_EMS_DSM_DSET_LIST :
			result = Handle_CMD_EMS_DSM_DSET_LIST(retInfo, pHeader->nCmdID, &nextCommand);
			break;                                                                      

		case CMD_EMS_DSM_DSET_START_RESP :
			result = Handle_CMD_EMS_DSM_DSET_START_RESP(retInfo, pHeader->nCmdID, &nextCommand);
			break;

		case CMD_EMS_DSM_DSET_INFO_RESP :
			result = Handle_CMD_EMS_DSM_DSET_INFO_RESP(retInfo, pHeader->nCmdID, &nextCommand);
			break;

		case CMD_EMS_DSM_DSET_END_RESP :
			result = Handle_CMD_EMS_DSM_DSET_END_RESP(retInfo, pHeader->nCmdID, &nextCommand);
			break;                                                                      

		case CMD_EMS_DSM_BULK :
			result = Handle_CMD_EMS_DSM_BULK(retInfo, pHeader->nCmdID, &nextCommand);
			break;

		case CMD_EMS_DSM_BULK_DATA_RESP : 
			result = Handle_CMD_EMS_DSM_BULK_DATA_RESP(retInfo, pHeader->nCmdID, &nextCommand);			
			break;

		case CMD_EMS_DSM_BULK_END_RESP : 
			result = Handle_CMD_EMS_DSM_BULK_END_RESP(retInfo, pHeader->nCmdID, &nextCommand);
			break;                                                                      
						
		case CMD_EMS_DSM_RT_MONIDATA_RESP :	
			result = Handle_CMD_EMS_DSM_RT_MONIDATA_RESP(retInfo, pHeader->nCmdID, &nextCommand);
			break;                                                                      
		case CMD_EMS_DSM_RT_MONIDATA_RESET_RESP :	
			//result = Handle_CMD_EMS_DSM_RT_MONIDATA_RESP(retInfo, pHeader->nCmdID, &nextCommand);
			break;

		default:
			LogErr("Invalid Packet ID.");
			nextCommand = CMD_NULL;
	}

	if(!result)
	{
		char strCmd[35];
		PacketCommandToString(strCmd, pHeader->nCmdID);	
		CString msg;
		msg.Format("DCM <<< DSM [%s] | Handle command error. ()", strCmd, jsStr);
		LogErr((LPSTR)(LPCSTR)msg);
	}

	SetPacketCommand(nextCommand);

	return result;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_HEARTBEAT
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_SESSIONKEY 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_HEARTBEAT(EMS_RESULT_INFO_ p_retInfo
												, int p_currCmdId
												, PacketCommand* p_nextCmd)
{	
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);	
	
	SetAlive(true);
	
	//CString msg;
	//msg.Format("DCM <<< DSM [%s] | Success.", strCmd);
	//LogInfo((LPSTR)(LPCSTR)msg);

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_SESSIONKEY
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_SESSIONKEY 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_SESSIONKEY(EMS_RESULT_INFO_ p_retInfo
												, int p_currCmdId
												, PacketCommand* p_nextCmd)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	msg.Format("DCM <<< DSM [%s] | Session key:(%d)"
					, strCmd
					, p_retInfo.session_key);

	if(p_retInfo.session_key <= 0)
	{			
		*p_nextCmd = CMD_EMS_PACKET_IDLE;
		LogErr((LPSTR)(LPCSTR)msg);

		return false;
	}
	else
	{	
		*p_nextCmd = CMD_EMS_DCM_REGISTER;
		m_sessionKey = p_retInfo.session_key;
		LogInfo((LPSTR)(LPCSTR)msg);

		return true;
	}
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_REGISTER_RESP
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_REGISTER_RESP 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_REGISTER_RESP(EMS_RESULT_INFO_ p_retInfo
														, int p_currCmdId
														, PacketCommand* p_nextCmd)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);	
	
	CString msg;
	msg.Format("DCM <<< DSM [%s] | Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);

	if(!p_retInfo.result)
	{
		*p_nextCmd = CMD_EMS_DCM_REGISTER;
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}
	else
	{	
		*p_nextCmd = CMD_EMS_PACKET_IDLE;
		LogInfo((LPSTR)(LPCSTR)msg);
		return true;
	}
}
	

//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_READY
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_READY 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_READY(EMS_RESULT_INFO_ p_retInfo
												, int p_currCmdId
												, PacketCommand* p_nextCmd)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);
	*p_nextCmd = CMD_EMS_DCM_READY_RESP;

	CString msg;
	msg.Format("DCM <<< DSM [%s] | Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);

	StartHeartBit(false);
	m_dataSet->UnRegistedTable();	
	LogInfo((LPSTR)(LPCSTR)msg);

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_STOP_RTD
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_STOP_RTD 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_STOP_RTD(EMS_RESULT_INFO_ p_retInfo
												, int p_currCmdId
												, PacketCommand* p_nextCmd)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);
	*p_nextCmd = CMD_EMS_DCM_STOP_RTD_RESP;	
	
	CString msg;
	msg.Format("DCM <<< DSM [%s] | Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);
	LogInfo((LPSTR)(LPCSTR)msg);
	
	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_DSET_LIST
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_DSET_LIST 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_DSET_LIST(EMS_RESULT_INFO_ p_retInfo
													, int p_currCmdId
													, PacketCommand* p_nextCmd)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);
	*p_nextCmd = CMD_EMS_DCM_DSET_START;
	
	CString msg;
	msg.Format("DCM <<< DSM [%s] | Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);

	LogInfo((LPSTR)(LPCSTR)msg);

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_DSET_START_RESP
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_DSET_START_RESP 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_DSET_START_RESP(EMS_RESULT_INFO_ p_retInfo
														, int p_currCmdId
														, PacketCommand* p_nextCmd)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	msg.Format("DCM <<< DSM [%s] | Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);

	if(!p_retInfo.result)
	{
		*p_nextCmd = CMD_EMS_DSM_DSET_LIST;
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}
	else
	{	
		*p_nextCmd = CMD_EMS_DCM_DSET_INFO;
		LogInfo((LPSTR)(LPCSTR)msg);
		return true;
	}
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_DSET_START_RESP
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_DSET_START_RESP 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_DSET_INFO_RESP(EMS_RESULT_INFO_ p_retInfo
														, int p_currCmdId
														, PacketCommand* p_nextCmd)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	msg.Format("DCM <<< DSM [%s] | Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);

	if(!p_retInfo.result)
	{
		*p_nextCmd = CMD_EMS_DCM_DSET_START;
		LogErr((LPSTR)(LPCSTR)msg);
		return false;
	}
	else
	{	
		if(m_currTable != NULL)
		{
			m_currTable->m_registed = true;
			m_currTable = NULL;
		}
				
		if(m_dataSet->GetNotRegistedTable() != NULL)
			*p_nextCmd = CMD_EMS_DCM_DSET_INFO;
		else
			*p_nextCmd = CMD_EMS_DCM_DSET_END;
		
		LogInfo((LPSTR)(LPCSTR)msg);
		
		return true;
	}
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_DSET_END_RESP
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_DSET_END_RESP 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_DSET_END_RESP(EMS_RESULT_INFO_ p_retInfo
														, int p_currCmdId
														, PacketCommand* p_nextCmd)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);
	*p_nextCmd = CMD_EMS_PACKET_IDLE;

	CString msg;
	msg.Format("DCM <<< DSM [%s] | Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);
	LogInfo((LPSTR)(LPCSTR)msg);

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_BULK
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_BULK 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_BULK(EMS_RESULT_INFO_ p_retInfo
											, int p_currCmdId
											, PacketCommand* p_nextCmd)
{
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);		

	CString msg;
	msg.Format("DCM <<< DSM [%s]"
					, strCmd					
					);

	LogInfo((LPSTR)(LPCSTR)msg);	
	*p_nextCmd = CMD_EMS_DCM_BULK_START;

	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_BULK_DATA_RESP
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_BULK_DATA_RESP 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_BULK_DATA_RESP(EMS_RESULT_INFO_ p_retInfo
														, int p_currCmdId
														, PacketCommand* p_nextCmd)
{	
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	msg.Format("DCM <<< DSM [%s] | Dataset:%s, ins_total_cnt:%d, ins_fail_cnt:%d Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.data_set_id
					, p_retInfo.ins_total_cnt
					, p_retInfo.ins_fail_cnt
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);

	if(!p_retInfo.result)
		LogErr((LPSTR)(LPCSTR)msg);
	else
		LogInfo((LPSTR)(LPCSTR)msg);

	*p_nextCmd = CMD_EMS_DCM_RT_MONIDATA;			
	return true;
}



//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_DSET_END_RESP
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_DSET_END_RESP 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_BULK_END_RESP(EMS_RESULT_INFO_ p_retInfo
														, int p_currCmdId
														, PacketCommand* p_nextCmd)
{	
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);

	CString msg;
	msg.Format("DCM <<< DSM [%s] | Dataset:%s, Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.data_set_id
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);

	if(!p_retInfo.result)
		LogErr((LPSTR)(LPCSTR)msg);
	else
		LogInfo((LPSTR)(LPCSTR)msg);

	*p_nextCmd = CMD_EMS_DCM_RT_MONIDATA;			
	return true;
}


//--------------------------------------------------------
// Title	: Handle_CMD_EMS_DSM_RT_MONIDATA_RESP
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : CMD_EMS_DSM_RT_MONIDATA_RESP 전문 처리.
//--------------------------------------------------------
bool CDsmIfSession::Handle_CMD_EMS_DSM_RT_MONIDATA_RESP(EMS_RESULT_INFO_ p_retInfo
														, int p_currCmdId
														, PacketCommand* p_nextCmd)
{	
	char strCmd[35];
	PacketCommandToString(strCmd, p_currCmdId);
	*p_nextCmd = CMD_EMS_DCM_RT_MONIDATA;

	CString msg;
	msg.Format("DCM <<< DSM [%s] | Dataset:%s, data_update_fail_cnt:%d, data_delete_fail_cnt:%d, Result:%d, Reason:%d, Msg:%s"
					, strCmd
					, p_retInfo.data_set_id
					, p_retInfo.data_update_fail_cnt
					, p_retInfo.data_delete_fail_cnt
					, p_retInfo.result
					, p_retInfo.reason
					, p_retInfo.errmsg
					);

	LogInfo((LPSTR)(LPCSTR)msg);
	return true;
}


//--------------------------------------------------------
// Title	: PacketCommandToString
// Writer	: KIMCG
// Date		: 2013.12.09
// Content  : Packet Command 를 문자열로 리턴한다.
//--------------------------------------------------------
char* CDsmIfSession::PacketCommandToString(char* p_strCmd, unsigned int p_cmd)
{
	char* strCmd = p_strCmd;
	//memset(cmd, 0x00, sizeof(cmd));
	switch(p_cmd)
	{
		case CMD_EMS_PACKET_IDLE				: strcpy(strCmd , "CMD_EMS_PACKET_IDLE");					break;
		case CMD_EMS_DCM_HEARTBEAT				: strcpy(strCmd , "CMD_EMS_DCM_HEARTBEAT");					break;
		case CMD_EMS_DSM_HEARTBEAT				: strcpy(strCmd , "CMD_EMS_DSM_HEARTBEAT");					break;
		case CMD_EMS_DSM_SESSIONKEY				: strcpy(strCmd , "CMD_EMS_DSM_SESSIONKEY");				break;
		case CMD_EMS_DCM_REGISTER				: strcpy(strCmd , "CMD_EMS_DCM_REGISTER");					break;
		case CMD_EMS_DSM_REGISTER_RESP			: strcpy(strCmd , "CMD_EMS_DSM_REGISTER_RESP");				break;
		case CMD_EMS_DSM_READY					: strcpy(strCmd , "CMD_EMS_DSM_READY");						break;
		case CMD_EMS_DCM_READY_RESP				: strcpy(strCmd , "CMD_EMS_DCM_READY_RESP");				break;
		case CMD_EMS_DSM_STOP_RTD				: strcpy(strCmd , "CMD_EMS_DSM_STOP_RTD");					break;
		case CMD_EMS_DCM_STOP_RTD_RESP			: strcpy(strCmd , "CMD_EMS_DCM_STOP_RTD_RESP");				break;
		case CMD_EMS_DSM_DSET_LIST				: strcpy(strCmd , "CMD_EMS_DSM_DSET_LIST");					break;
		case CMD_EMS_DCM_DSET_START				: strcpy(strCmd , "CMD_EMS_DCM_DSET_START");				break;
		case CMD_EMS_DSM_DSET_START_RESP		: strcpy(strCmd , "CMD_EMS_DSM_DSET_START_RESP");			break;
		case CMD_EMS_DCM_DSET_INFO				: strcpy(strCmd , "CMD_EMS_DCM_DSET_INFO");					break;
		case CMD_EMS_DSM_DSET_INFO_RESP			: strcpy(strCmd , "CMD_EMS_DSM_DSET_INFO_RESP");			break;
		case CMD_EMS_DCM_DSET_END				: strcpy(strCmd , "CMD_EMS_DCM_DSET_END");					break;
		case CMD_EMS_DSM_DSET_END_RESP			: strcpy(strCmd , "CMD_EMS_DSM_DSET_END_RESP");				break;
		case CMD_EMS_DSM_BULK					: strcpy(strCmd , "CMD_EMS_DSM_BULK");						break;
		case CMD_EMS_DCM_BULK_START				: strcpy(strCmd , "CMD_EMS_DCM_BULK_START");				break;
		case CMD_EMS_DCM_BULK_DATA				: strcpy(strCmd , "CMD_EMS_DCM_BULK_DATA");					break;
		case CMD_EMS_DSM_BULK_DATA_RESP			: strcpy(strCmd , "CMD_EMS_DSM_BULK_DATA_RESP");			break;
		case CMD_EMS_DCM_BULK_END				: strcpy(strCmd , "CMD_EMS_DCM_BULK_END");					break;
		case CMD_EMS_DSM_BULK_END_RESP			: strcpy(strCmd , "CMD_EMS_DSM_BULK_END_RESP");				break;
		case CMD_EMS_DCM_RT_MONIDATA			: strcpy(strCmd , "CMD_EMS_DCM_RT_MONIDATA");				break;
		case CMD_EMS_DSM_RT_MONIDATA_RESP		: strcpy(strCmd , "CMD_EMS_DSM_RT_MONIDATA_RESP");			break;
		case CMD_EMS_DCM_RT_RESET				: strcpy(strCmd , "CMD_EMS_DCM_RT_RESET");					break;
		case CMD_EMS_DSM_RT_MONIDATA_RESET_RESP	: strcpy(strCmd , "CMD_EMS_DSM_RT_MONIDATA_RESET_RESP");	break;
	}
	return strCmd;
}


//--------------------------------------------------------
// Title	: GetJsonAPI
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Json API 얻음.
//--------------------------------------------------------
CJsonAPI* CDsmIfSession::GetJsonAPI()
{
	return m_jsonAPI;
}


//--------------------------------------------------------
// Title	: Connect
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : DSM에 연결:TCP 멤버로 설정된 Ip, Port 사용
//--------------------------------------------------------
bool CDsmIfSession::Connect()
{
	//int		errNo = -1;
	//char	errMsg[512];	
	char	buf[20];
	memset(buf, 0x00, sizeof(buf));
	strcpy(buf, m_dsmIp.c_str());

	LogInfo("Try connect to DSM.");
	bool ret = Connect(buf, m_dsmPort);
	if(!ret)
	{
		m_isConnected = false;
		//errNo = GetErrNo();
		//memset(errMsg, 0x00, sizeof(errMsg));
		//GetErrStr(errMsg, sizeof(errMsg), errNo);
		LogErr("DsmIfSession Connection Fail.");
		//LogErr(errMsg);
	}
	else
	{

		LogInfo("DsmIfSession Connection Succ.");
		// 10 초간 세션키가 생성되지 않았다면 실패
		bool session = false;
		for(int i =0; i< 10; i++)
		{
			iSleep(ONESEC);
			if(m_sessionKey < 0)
				continue;

			session = true;
			break;
		}

		if(!session)
		{
			//m_isConnected = false;
			ret = false;
			LogErr("Did not arrive session key 10 seconds.");
			Disconnect();
		}
		else
		{
			LogInfo("DsmIfSession Connection Success.");
			ret = true;
		}
	}
	return ret;
}


//--------------------------------------------------------
// Title	: Connect
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 1. DCM ---> DSM
//			  2. DSM에 연결:TCP 멤버로 설정된 Ip, Port 사용
//			  3. ACK : EMS_DSM_SESSIONKEY
//--------------------------------------------------------
bool CDsmIfSession::Connect(char* p_ip, int p_port)
{
	if(!Open(PF_INET, SOCK_STREAM))
		return false;

	int errNo = -1;
	bool ret = iSocket::Connect(p_ip, p_port, (ONESEC * 5));
	if(!ret)
	{	
		m_isConnected = false;
		return m_isConnected;
	}

	m_isConnected = true;
	return m_isConnected;
}


//--------------------------------------------------------
// Title	: Disconnect
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Socket DisConnect
//--------------------------------------------------------
void CDsmIfSession::Disconnect()
{	
	if(!m_isConnected)
		return;

	m_dataSet->UnRegistedTable();
	if(GetSocket() > 0)
		Close();
	
	iSleep(ONESEC * 3);
	LogInfo("Dsm session disconnected.");
	m_sessionKey = -1;
	m_isConnected = false;
	StartHeartBit(false);
	iSleep(ONESEC / 2);
}


//--------------------------------------------------------
// Title	: IsConnected
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : DSM 연결상태 확인
//--------------------------------------------------------
bool CDsmIfSession::IsConnected()
{
	if(m_isConnected && GetSocket() > 0)
		return true;
	else
		return false;
}


//--------------------------------------------------------
// Title	: SendJsonPacket
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 1. 전문송신
//			  2. DCM ---> DSM
//--------------------------------------------------------
bool CDsmIfSession::SendJsonPacket(EMS_PACKET_HEADER_* p_header, NPSTR p_jsonStr, size_t p_len)
{	
	if(!IsConnected())
	{
		LogErr("Dsm is not connected. func:SendJsonPacket()");
		return false;
	}
	
	char errNo = -1;
	char errMsg[512];
	int headerLen = sizeof(*p_header);
	int bodyLen = p_len;
	int totalLen = (headerLen + bodyLen);	

	if(totalLen > SOCK_SEND_MAX_SIZE)
	{
		LogErr("Json data is over length SOCK_SEND_MAX_SIZE. func:SendJsonPacket()");
		return false;
	}

	p_header->nLen = p_len;
	char sockBuf[SOCK_SEND_MAX_SIZE];	
	memset(sockBuf, 0x00, sizeof(sockBuf));
	memcpy(sockBuf, p_header, headerLen);
	memcpy(sockBuf + headerLen, p_jsonStr, p_len);
	
	int	len			= 0;
	m_mutexLock.Lock();
	while(len < totalLen)
	{		
		len	+= Send((sockBuf + len), (totalLen - len));
		if(len < 0)
		{
			errNo = GetErrNo();
			memset(errMsg, 0x00, sizeof(errMsg));
			GetErrStr(errMsg, sizeof(errMsg), errNo);
			
			LogErr("Send socket error. func:SendJsonPacket()");
			LogErr(errMsg);

			Disconnect();
			m_mutexLock.Unlock();	
			return false;
		}		
		iSleep(100);
	}

	m_mutexLock.Unlock();	
	
	return true;
}


//--------------------------------------------------------
// Title	: MakePacketHeader
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 패킷헤더 생성
//--------------------------------------------------------
bool CDsmIfSession::MakePacketHeader(EMS_PACKET_HEADER_* p_header, unsigned int p_cmd)
{
	if(m_sessionKey < 0)
	{
		LogErr("Invalid session key.");
		Disconnect();
		return false;
	}

	EMS_PACKET_HEADER_ *pstHdr = p_header;
	memset( pstHdr, 0x00, sizeof(EMS_PACKET_HEADER_));
	strcpy(pstHdr->stx, EMS_STX_);
	sprintf(pstHdr->Ver, "%c%c%c", EMS_MAJOR_VER, EMS_MINOR_VER, EMS_RELEASE_VER);
	pstHdr->nLen		= 0;
	pstHdr->nCmdID		= p_cmd;
	pstHdr->nSessionKey = m_sessionKey;
	pstHdr->nSeq		= 0;
	pstHdr->nSysFrom	= m_systemId;
	pstHdr->nSysTo		= 0;
	pstHdr->nFrom		= m_processId;
	pstHdr->nTo			= 1;

	return true;
}


//--------------------------------------------------------
// Title	: GetHeartBeatSeq
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : HeartBeat 시퀀스 얻음.
//--------------------------------------------------------
unsigned int CDsmIfSession::GetHeartBeatSeq()
{
	return m_heartBeatSeq;
}


//--------------------------------------------------------
// Title	: IncreaseHeartBeatSeq
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : HeartBeat 카운트 증가.
//--------------------------------------------------------
void CDsmIfSession::IncreaseHeartBeatSeq()
{
	if(m_heartBeatSeq >= 65500)
		m_heartBeatSeq = 0;

	m_heartBeatSeq++;
}

//--------------------------------------------------------
// Title	: GetIpAddress
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 아이피주소 얻음.
//--------------------------------------------------------
string CDsmIfSession::GetIpAddress()
{
	return m_dsmIp;
}


//--------------------------------------------------------
// Title	: GetPort
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 포트얻음.
//--------------------------------------------------------
int CDsmIfSession::GetPort()
{
	return m_dsmPort;
}


//--------------------------------------------------------
// Title	: GetSessionKey
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 세션키 얻음. DSM에 Connect 연결시 DSM이 부여
//--------------------------------------------------------
int CDsmIfSession::GetSessionKey()
{	
	return m_sessionKey;
}


//--------------------------------------------------------
// Title	: CheckStx
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : STX 체크
//--------------------------------------------------------
bool CDsmIfSession::CheckStx(EMS_PACKET_HEADER_* pHdr)
{
	if((strcmp(pHdr->stx, EMS_STX_) && pHdr->nLen < EMS_DSM_PKT_BODYSIZE_))
		return true;
	else
		return false;
}


//--------------------------------------------------------
// Title	: SetPacketCommand
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Packet command 설정.
//--------------------------------------------------------
void CDsmIfSession::SetPacketCommand(PacketCommand p_command)
{
	m_mutexLock.Lock();	
	m_packetCmd = p_command;
	m_mutexLock.Unlock();
}

//--------------------------------------------------------
// Title	: StartHeartBit
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Heart bit start 
//--------------------------------------------------------
void CDsmIfSession::StartHeartBit(bool p_isStart)
{
	m_startHB = p_isStart;
}

//--------------------------------------------------------
// Title	: IsStartHeartBit
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Heart bit Running 상태여부
//--------------------------------------------------------
bool CDsmIfSession::IsStartHeartBit()
{
	return m_startHB;
}

//--------------------------------------------------------
// Title	: GetPacketCommand
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Packet command 얻음.
//--------------------------------------------------------
PacketCommand CDsmIfSession::GetPacketCommand()
{
	return m_packetCmd;
}

//--------------------------------------------------------
// Title	: GetDataSet
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : DataSet 얻음.
//--------------------------------------------------------
CDcmDataSet* CDsmIfSession::GetDataSet()
{
	return m_dataSet;
}


//--------------------------------------------------------
// Title	: GetDataTable
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : Data table 얻음.
//--------------------------------------------------------
CDcmDataTable* CDsmIfSession::GetDataTable(char* p_tableName)
{	
	list<CDcmDataTable*>::iterator itor;
	for(itor= m_dataSet->m_dataTables.begin(); itor != m_dataSet->m_dataTables.end(); itor++)
	{	
		if(!strcmp(p_tableName, (*itor)->m_dataTableName.c_str()))
			return (*itor);
	}
	
	return NULL;
}

bool CDsmIfSession::IsAlive()
{
	return m_isAlive;
}

void CDsmIfSession::SetAlive(bool p_isAlive)
{
	m_isAlive = p_isAlive;
}

//--------------------------------------------------------
// Title	: SetErrMsg
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 오류메시지 설정
//--------------------------------------------------------
void CDsmIfSession::SetErrMsg(char* Format, ...)
{	
	char	Buff[2048];	
	va_list	Args;
	memset(Buff, 0x00, sizeof(Buff));
	va_start( Args, Format );
	vsprintf_s( Buff, Format, Args );
	va_end( Args );

	m_errMsg = Buff;
}


//--------------------------------------------------------
// Title	: GetErrMsg
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 오류메시지 얻음
//--------------------------------------------------------
void CDsmIfSession::GetErrMsg(char* p_msg)
{	
	memset(p_msg, 0x00, sizeof(p_msg));
	strcpy(p_msg, m_errMsg.c_str());
}


//--------------------------------------------------------
// Title	: CDsmIfSession
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 메시지 로그
//--------------------------------------------------------
void CDsmIfSession::LogInfo(char* p_msg)
{	
	try
	{	
		APPLOG->Print(DBGLV_INF0, "[%s:%d(%d)] %s"						
						, m_dsmIp.c_str()						
						, m_dsmPort
						, m_sessionKey						
						, p_msg
						);
	}
	catch(DWORD e)
	{
		return;
	}
}


//--------------------------------------------------------
// Title	: CDsmIfSession
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 메시지 로그
//--------------------------------------------------------
void CDsmIfSession::LogWrn(char* p_msg)
{	
	try
	{	
		APPLOG->Print(DBGLV_WRN, "[%s:%d(%d)] %s"						
						, m_dsmIp.c_str()						
						, m_dsmPort
						, m_sessionKey						
						, p_msg
						);
	}
	catch(DWORD e)
	{
		return;
	}
}
//--------------------------------------------------------
// Title	: LogErr
// Writer	: KIMCG
// Date		: 2013.12.06
// Content  : 오류 로그
//--------------------------------------------------------
void CDsmIfSession::LogErr(char* p_msg)
{
	try
	{
		APPLOG->Print(DBGLV_ERR, "[%s:%d(%d)] %s"						
						, m_dsmIp.c_str()						
						, m_dsmPort
						, m_sessionKey						
						, p_msg
						);
	}
	catch(DWORD e)
	{
		return;
	}
}
