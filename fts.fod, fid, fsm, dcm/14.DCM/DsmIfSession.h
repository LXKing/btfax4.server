#pragma once
#include <stdio.h>
#include <list>
#include <string.h>

// iCore..
#include "iLib.h"
#include "iSocket.h"
#include "iThread.h"
#include "iMutex.h"
#include "iType.h"
#include "iTimer.h"
// iCore

#include "DsmIfSession_Recv.h"
#include "DsmIfSession_HB.h"

#include "DcmDataSet.h"
#include "DcmDataTable.h"
#include "Structures.h"
#include "JsonAPI.h"

#define SOCK_SEND_MAX_SIZE					(1024 * 4)	// 4k
#define SOCK_RECV_MAX_SIZE					(1024 * 4)	// 4k

typedef enum PacketCommand
{
	CMD_NULL								= -1,
	CMD_EMS_PACKET_IDLE						= 0,
	CMD_EMS_DCM_HEARTBEAT					= 0x00000001,
	CMD_EMS_DSM_HEARTBEAT					= 0x10000001,
	CMD_EMS_DSM_SESSIONKEY					= 0x00000002,
	CMD_EMS_DCM_REGISTER					= 0x00000003,
	CMD_EMS_DSM_REGISTER_RESP				= 0x10000003,
	CMD_EMS_DSM_READY						= 0x00041001,
	CMD_EMS_DCM_READY_RESP					= 0x10041001,
	CMD_EMS_DSM_STOP_RTD					= 0x00041002,
	CMD_EMS_DCM_STOP_RTD_RESP				= 0x10041002,
	CMD_EMS_DSM_DSET_LIST					= 0x00041003,
	CMD_EMS_DCM_DSET_START					= 0x00041004,
	CMD_EMS_DSM_DSET_START_RESP				= 0x10041004,
	CMD_EMS_DCM_DSET_INFO					= 0x00041005,
	CMD_EMS_DSM_DSET_INFO_RESP				= 0x10041005,
	CMD_EMS_DCM_DSET_END					= 0x00041006,
	CMD_EMS_DSM_DSET_END_RESP				= 0x10041006,
	CMD_EMS_DSM_BULK						= 0x00042001,
	CMD_EMS_DCM_BULK_START					= 0x00042002,
	CMD_EMS_DCM_BULK_DATA					= 0x00042003,
	CMD_EMS_DSM_BULK_DATA_RESP				= 0x10042003,
	CMD_EMS_DCM_BULK_END					= 0x00042004,
	CMD_EMS_DSM_BULK_END_RESP				= 0x10042004,
	CMD_EMS_DCM_RT_MONIDATA					= 0x00042005,
	CMD_EMS_DSM_RT_MONIDATA_RESP			= 0x10042005,
	CMD_EMS_DCM_RT_RESET					= 0x00042010,
	CMD_EMS_DSM_RT_MONIDATA_RESET_RESP		= 0x10042010
};

class CDsmIfSession : public iSocket
{
public:
	static CDsmIfSession* Inst();
protected:
	static CDsmIfSession* s_pInstance;

public:
	CDsmIfSession(void);
	~CDsmIfSession(void);
	
	void SetInfo(char* p_serviceGroup, char* p_systemName, int p_systemId, char* p_procName, int p_procId);
	void SetDataSet(CDcmDataSet* p_dataSet);
	CDcmDataSet* GetDataSet();
	CDcmDataTable* GetDataTable(char* p_tableName);
	void SetAddress(char* p_ip, int p_port);
	void Run();
	void StopAll();
	void StartHeartBit(bool p_isStart);
	bool IsStartHeartBit();
	void SendHeartBit();
	void Receive();

	NPVOID ThreadRun(NPVOID pArg);

	bool Connect();
	bool Connect(char* p_ip, int p_port);		
	bool IsConnected();
	void Disconnect();
	bool CheckStx(EMS_PACKET_HEADER_* pHdr);
	
	//send
	bool Handle_CMD_EMS_PACKET_IDLE(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_HEARTBEAT(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_REGISTER(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_READY_RESP(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_STOP_RTD_RESP(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_DSET_START(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_DSET_INFO(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_DSET_END(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_BULK_START(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_BULK_DATA(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_BULK_END(int p_currCmdId);
	bool Handle_CMD_EMS_DCM_RT_MONIDATA(int p_currCmdId);

	// receive
	bool HandleRecceivePacket						(EMS_PACKET_HEADER_* pHeader, char* jsStr);
	bool Handle_CMD_EMS_DSM_HEARTBEAT				(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_SESSIONKEY				(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_REGISTER_RESP			(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_READY					(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_STOP_RTD				(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_DSET_LIST				(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_DSET_START_RESP			(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_DSET_INFO_RESP			(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_DSET_END_RESP			(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_BULK					(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_BULK_DATA_RESP			(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_BULK_END_RESP			(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	bool Handle_CMD_EMS_DSM_RT_MONIDATA_RESP		(EMS_RESULT_INFO_ p_retInfo, int p_currCmdId, PacketCommand* p_nextCmd);
	
	bool IsStopReceive();
	void StopReceive(bool p_wait);
	void SetPacketCommand(PacketCommand p_cmd);
	PacketCommand GetPacketCommand();
	char* PacketCommandToString(char* p_strCmd, unsigned int p_cmd);
	CJsonAPI* GetJsonAPI();
	string GetIpAddress();
	int GetPort();
	int GetSessionKey();
	bool IsAlive();
	void SetAlive(bool p_isAlive);
	void IncreaseHeartBeatSeq();
	unsigned int GetHeartBeatSeq();
	void GetErrMsg(char* buf);
	void SetErrMsg(char* Format, ...);	
	void LogInfo(char* p_msg);
	void LogWrn(char* p_msg);
	void LogErr(char* p_msg);

	//timer
	iTimer					m_HbTimer;

protected:
	bool MakePacketHeader(EMS_PACKET_HEADER_* p_header, unsigned int p_cmd);
	bool SendJsonPacket(EMS_PACKET_HEADER_* p_header, NPSTR p_jsonStr, size_t p_len);

// field
protected:
	string					m_errMsg;
	string					m_systemName;
	int						m_systemId;
	string					m_processName;
	int						m_processId;
	string					m_serviceGroup;
	CDcmDataSet*			m_dataSet;
	CDcmDataTable*			m_currTable;
	bool					m_stopReceive;
	unsigned int			m_heartBeatSeq;
	bool					m_startHB;
	bool					m_isAlive;
	
	
	// sock
	string					m_dsmIp;
	int						m_dsmPort;	
	bool					m_isConnected;
	int 					m_sessionKey;

	// thread
	CDsmIfSession_Recv		m_receiveThread;
	CDsmIfSession_HB		m_heartBitThread;

	// json
	CJsonAPI				*m_jsonAPI;
		
	PacketCommand			m_packetCmd;
	iMutex					m_mutexLock;
};

