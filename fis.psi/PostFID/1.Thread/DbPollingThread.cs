using System;
using System.Collections.Generic;
using System.Threading;
using System.Data;
using System.IO;
using System.Text;         //WCD(2014-05-28)
using System.Net;          //WCD(2014-05-28)
using System.Net.Sockets;  //WCD(2014-05-28)

using Btfax.CommonLib;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Alarm;
using PostFid.Util;
using PostFid.DB;

namespace PostFid.Threading
{
	class DbPollingThread : BtfaxThread
	{
		#region static
		private static DbPollingThread s_instance = null;
		public static DbPollingThread Instance
		{
			get
			{
				if (s_instance == null)
					s_instance = new DbPollingThread();
				return s_instance;
			}
		}
		#endregion

		#region method
		private void StartPostProcessingThreads()
		{
			//for (int i = 0; i < Config.THREAD_CNT; i++)
			//{
			//    BtfaxThread thread = new PostProcessingThread();
			//    thread.StartThread();

			//    m_lstPostProcessingThreads.Add(thread);
			//}
		}

		private void JoinPostProcessingThreads()
		{
			//foreach (BtfaxThread thread in m_lstPostProcessingThreads)
			//{
			//    if (!thread.JoinThread())
			//        AppLog.Write(LOG_LEVEL.MSG, "쓰레드 종료중 오류가 발생하였습니다.");
			//}
			//m_lstPostProcessingThreads.Clear();
		}

		/// <summary>
		/// 팩스 스토리지에 접근 가능한지 시험한다.
		/// </summary>
		/// <returns></returns>
		protected bool ExistsFaxStorage()
		{
			try
			{
				// 팩스스토리지 점검
				if (!Directory.Exists(Config.STG_HOME_PATH))
					return false;
			}
			catch (Exception ex)
			{
				AppLog.Write(LOG_LEVEL.ERR
					, string.Format("팩스스토리지({0})에 접근중 오류가 발생하였습니다. 오류:{1}", Config.STG_HOME_PATH, ex.Message));

				return false;
			}

			return true;
		}

		#endregion

		#region override
		public override bool StartThread()
		{
			StartPostProcessingThreads();
			return base.StartThread();
		}

		public override bool JoinThread(int p_nTimeout = -1)
		{
			JoinPostProcessingThreads();
			if (!base.JoinThread(p_nTimeout))
				return false;
			return true;
		}

		protected override void ThreadEntry()
		{
			AppLog.Write(LOG_LEVEL.MSG, "### DB폴링 쓰레드 시작 ###");
			Thread.Sleep(Config.INITIAL_SLEEP);
			AppLog.Write(LOG_LEVEL.MSG, "### DB폴링 시작 ###");

			int nPolling = 0;
			while (!BtfaxThread.Stop)
			{
				try
				{
					Thread.Sleep(Config.DB_POLLING_SLEEP);

					//if (!ExistsFaxStorage())
					//{
					//    AppLog.Write(LOG_LEVEL.ERR, string.Format("팩스스토리지를 찾을수 없습니다. 위치:{0}", Config.STG_HOME_PATH));
					//    continue;
					//}

					if (m_lstPostProcessingReqs.Count > 0)
						m_lstPostProcessingReqs.Clear();

					/// 처리건 조회
					RESULT ret = DbModule.Instance.PSI_FetchPostProcessingRequest(Config.PROCESS_TYPE,
																					Config.SYSTEM_PROCESS_ID,
																					Config.FETCH_CNT,
																					ref m_lstPostProcessingReqs
																					);

					nPolling++;
					if (nPolling % 10 == 0)
					{
						AppLog.Write(LOG_LEVEL.MSG, "DB Polling Check OK.");
						if (nPolling == 1000)
							nPolling = 0;
					}

					switch (ret)
					{
						case RESULT.EMPTY:
						case RESULT.SUCCESS:
							break;

						default:							
							AppLog.Write(LOG_LEVEL.ERR, "DB 폴링 중 오류가 발생하였습니다");							
							continue;
					}

					if (m_lstPostProcessingReqs.Count <= 0)
						continue;

					//// 요철건 처리 ////
					AppLog.Write(LOG_LEVEL.MSG, string.Format("처리건 개수:{0}", m_lstPostProcessingReqs.Count));					
					foreach (DbModule.RECV_FAX_DATA_PSI req in m_lstPostProcessingReqs)
					{
						if (req.SPAM_YN == "Y")
						{
							AppLog.Write(LOG_LEVEL.MSG, "스팸팩스함으로 분류되었습니다.");
							continue;
						}

						PostProcessingReq(req);
					}
				}
				catch (Exception ex)
				{
					AppLog.ExceptionLog(ex, "DB폴링 쓰레드에서 다음과 같은 오류가 발생하였습니다.");
				}
			}

			AppLog.Write(LOG_LEVEL.ERR, "### DB폴링 종료 ###");
		}

		#endregion

		#region processing item

		bool PostProcessingReq(DbModule.RECV_FAX_DATA_PSI p_reqItem)
		{
			LogMessage("팩스수신 후처리 시작.", p_reqItem);

			if (p_reqItem.SERVICE_FAXNO == Config.LOOP_FAXNO)
			{
				// LOOP 송신에 대한 결과 송신.
				// PSI --> LTM
				SendSendToLTM(p_reqItem);
			}
			else
			{
				// 수신 알림메시지 처리.
				SendRecvNoti(p_reqItem);
			}

			// E-Mail 처리.
			SendEMail(p_reqItem);

			// 대리자 처리.
			SendProxyUser(p_reqItem);

			// 리턴팩스 처리 - 우리카드 커스터마이징
			//SendReturnFax(p_reqItem);

			LogMessage("팩스수신 후처리 완료.", p_reqItem);

			return true;
		}

		void SendSendToLTM(DbModule.RECV_FAX_DATA_PSI p_reqItem)
		{
			string strMsg = string.Format("LOOP_FAX|{0}", p_reqItem.SERVICE_FAXNO);
			byte[] buffer = Encoding.Default.GetBytes(strMsg);

			IPAddress ipAddr;
			if (!IPAddress.TryParse(Config.LTM_IP, out ipAddr))
			{
				LogMessage(string.Format("LOOP 팩스 IP 파싱 실패. IP={0}", Config.LTM_IP));
				return;
			}

			IPEndPoint ep = new IPEndPoint(ipAddr, Config.LTM_PORT);
			using (Socket udpSock = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp))
			{
				int nLen = udpSock.SendTo(buffer, 0, buffer.Length, SocketFlags.None, ep);
				if (nLen <= 0)
				{
					LogMessage(string.Format("LOOP 팩스 데이터 전송 실패. IP={0}", Config.LTM_IP));
				}

				LogMessage("LOOP FAX 결과 전송 완료.", p_reqItem);
			}
		}

		void SendRecvNoti(DbModule.RECV_FAX_DATA_PSI p_reqItem)
		{
			List<string> notiUsers = new List<string>();

			//// 부서, 업무에 속한 사용자 아이디 얻음.
			if (!string.IsNullOrEmpty(p_reqItem.DEPT_CD) || !string.IsNullOrEmpty(p_reqItem.TASK_ID))
				DbModule.Instance.GetReceiveNotiUsers(p_reqItem.DEPT_CD, p_reqItem.TASK_ID, ref notiUsers);

			//// 개인팩스함으로 수신
			if (!string.IsNullOrEmpty(p_reqItem.USER_ID))
			{
				notiUsers.Clear();
				notiUsers.Add(p_reqItem.USER_ID);
			}

			//// 사용자에게 수신 알람 메시지 전송
			foreach (string userId in notiUsers)
			{
				if (!UdpSingle.Instance.SendMessageEx(userId, p_reqItem))
					LogError(string.Format("팩스수신 알림메시지 전송 실패. USER_ID:{0}", userId));

				Thread.Sleep(10);
			}

			LogMessage("팩스수신건 알림메시지 전송 완료.", p_reqItem);
		}

		void SendEMail(DbModule.RECV_FAX_DATA_PSI p_reqItem)
		{
			if (p_reqItem.EMAIL_NOTIFY_YN == "Y")
			{
				bool isSuccess = MailHelper.SendMail(p_reqItem.EMAIL_NOTIFY);
				//if (this.InsertProcessLog(m_recvInfo.FAX_ID, isSuccess,
				//    "사용자에게 EMail 알림 보내기",
				//    "보낸 EMail 주소 : " + m_recvInfo.EMAIL_NOTIFY,
				//    "SMTP IP : " + Config.SMTP_IP,
				//    "SMTP Port : " + Config.SMTP_PORT.ToString()
				//    ) != RESULT.SUCCESS)
				if (this.InsertProcessLog(p_reqItem.FAX_ID, isSuccess, "Send EMail") != RESULT.SUCCESS)
				{
					LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 (EMAIL 알림 보내기)");
					// AlarmAPI.Instance.SendFaultAlarm((UInt32)RESULT.F_DB_UPDATE_ERROR);
				}

				LogMessage("팩스수신건 이메일 전송 완료.", p_reqItem);
			}
		}

		void SendProxyUser(DbModule.RECV_FAX_DATA_PSI p_reqItem)
		{
			if (p_reqItem.PROXY_USER_ID != "")
			{
				bool isSuccess = UdpSingle.Instance.SendMessage(p_reqItem.PROXY_USER_ID);
				//if (this.InsertProcessLog(m_recvInfo.FAX_ID, isSuccess,
				//    "대리자에게 UDP 메세지 보내기",
				//    "대리자 ID : " + m_recvInfo.PROXY_USER_ID,
				//    "FAS IP : " + Config.FAS_IP,
				//    "FAS Port : " + Config.FAS_PORT.ToString()
				//    ) != RESULT.SUCCESS)
				if (this.InsertProcessLog(p_reqItem.FAX_ID, isSuccess, "Send UDP Message") != RESULT.SUCCESS)
				{
					LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 (UDP 메세지 보내기)");
					// AlarmAPI.Instance.SendFaultAlarm((UInt32)RESULT.F_DB_UPDATE_ERROR);
				}

				if (p_reqItem.PROXY_EMAIL_NOTIFY_YN == "Y")
				{
					isSuccess = MailHelper.SendMail(p_reqItem.PROXY_EMAIL_NOTIFY);
					//if (this.InsertProcessLog(m_recvInfo.FAX_ID, isSuccess,
					//    "사용자에게 EMail 알림 보내기",
					//    "보낸 EMail 주소 : " + m_recvInfo.PROXY_EMAIL_NOTIFY,
					//    "SMTP IP : " + Config.SMTP_IP,
					//    "SMTP Port : " + Config.SMTP_PORT.ToString()
					//    ) != RESULT.SUCCESS)
					if (this.InsertProcessLog(p_reqItem.FAX_ID, isSuccess, "Send EMail") != RESULT.SUCCESS)
					{
						LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 (EMAIL 알림 보내기)");
						// AlarmAPI.Instance.SendFaultAlarm((UInt32)RESULT.F_DB_UPDATE_ERROR);
					}
				}

				LogMessage("팩스수신건 대리자 처리 완료.", p_reqItem);
			}
		}

		void SendReturnFax(DbModule.RECV_FAX_DATA_PSI p_reqItem)
		{
			//LogError(string.Format("RET FAX => |{0}|{1}|", Config.RET_DEPT_CD, m_recvInfo.DEPT_CD) );
			if (Config.RET_DEPT_CD.CompareTo(p_reqItem.DEPT_CD) == 0)    //WCD 2014-06-24
			//if (Config.RET_DEPT_CD.IndexOf(m_recvInfo.DEPT_CD) >= 0)
			{
				if (!this.SendToRetFax(p_reqItem))
				{
					LogError("리턴팩스 전송 도중 오류가 발생하였습니다!!");
					// AlarmAPI.Instance.SendFaultAlarm((UInt32)RESULT.F_DB_UPDATE_ERROR);
				}

				LogMessage("팩스수신건 리턴팩스 처리 완료.", p_reqItem);
			}
		}

		private bool SendToRetFax(DbModule.RECV_FAX_DATA_PSI p_info)   //WCD(2014-05-28)
		{
			//string faxErr = "N";
			byte[] m_postOutboundPacket = GetRetFaxPacket(p_info);
			if (m_postOutboundPacket == null)
			{
				LogError("RET FAX Packet 생성실패");
				return false;
			}
			return SendPostOutboundPacket(m_postOutboundPacket);

		}

		private bool SendPostOutboundPacket(byte[] p_data)   //WCD(2014-05-28)
		{
			try
			{
				LogMessage(string.Format("PACKET [SEND] => |{0}|", Encoding.Default.GetString(p_data), RESULT.EMPTY));
				using (Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
				{
					sock.SendTimeout = SOCK_SEND_TIMEOUT;
					sock.ReceiveTimeout = SOCK_RECV_TIMEOUT;
					sock.Connect(Config.SYSTEM_IP, Config.AOB_PORT);

					//// 송신처리 ////
					int len = 0;
					for (int i = 0; i < (SOCK_SEND_TIMEOUT / 1000); i++)
					{
						len = sock.Send(p_data, len, p_data.Length, SocketFlags.None);
						if (len >= p_data.Length)
							break;

						len += len;
						System.Threading.Thread.Sleep(1000);
					}
					if (len < p_data.Length)
					{
						if (sock.Connected)
							sock.Close();

						return false;
					}

					//// 수신처리 ////
					byte[] tempBuf = new byte[10000];
					len = sock.Receive(tempBuf, SocketFlags.None);
					byte[] rcvBuf = new byte[len];
					Buffer.BlockCopy(tempBuf, 0, rcvBuf, 0, len);
					sock.Close();

					LogMessage(string.Format("PACKET [RECV] <= |{0}|", Encoding.Default.GetString(rcvBuf), RESULT.EMPTY));

				}

				return true;
			}
			catch (Exception ex)
			{
				LogError(string.Format("다음과같은 이유로 데이터전송을 실패하였습니다. {0}", ex.Message));
				return false;
			}
		}

		private static void ClearPacket(ref byte[] p_packet)    //WCD(2014-05-28)
		{
			for (int i = 0; i < p_packet.Length; i++)
				p_packet[i] = 0x20;
		}

		public static byte[] GetRetFaxPacket(DbModule.RECV_FAX_DATA_PSI p_info)   //WCD(2014-05-28)
		{
			string strHead = "";
			string strBody = "";
			string strPacket = "";

			byte[] bprPacket = new byte[RET_FAX_PACKET_LEN];
			ClearPacket(ref bprPacket);
			try
			{
				strHead = String.Format("{0:3}{1:D06}{2:yyyyMMddHHmmss}{3,-50}{4,-10}{5,2}{6,-30}{7,-50}{8,2}{9,-50}{10,-50}{11,-30}{12,1}{13,1}{14,1}{15,1}{16,8}{17,6}{18,1}{19,-30}{20,-200}{21,2}{22,52}"
										 , "4.0", RET_FAX_PACKET_LEN, DateTime.Now, "RETURN FAX", "1010", "01", p_info.CID, "", "02", "RETFAX", "RETURN FAX", p_info.SERVICE_FAXNO, "N", "N", "N", "N", "", "", "1", ".", "", "", "");
				strBody = String.Format("{0,-20}{1,-20}{2,8}{3,4}{4,2}{5,-50}", p_info.CID, p_info.SERVICE_FAXNO, p_info.RECV_DATE, p_info.RECV_TIME, p_info.PAGE_CNT, p_info.DEPT_NAME);
				strPacket = strHead + strBody;
				byte[] tmpPacket = Encoding.Default.GetBytes(strPacket);

				Buffer.BlockCopy(tmpPacket, 0, bprPacket, 0, bprPacket.Length);
			}
			catch
			{
				bprPacket = null;
			}

			return bprPacket;
		}

		#endregion


		#region method - log

		private void LogMessage(string message, DbModule.RECV_FAX_DATA_PSI p_info, RESULT db_result = RESULT.EMPTY)
		{
			LogMessage(String.Format("[RESULT:{0}] [FAX_ID:{1}][DEPT_CD:{2}][TASK_ID:{3}][USER_ID:{4}][CID:{5}][CID_NAME:{6}][EMAIL_NOTIFY_YN:{7}][EMAIL_NOTIFY:{8}] {9}",
												db_result,
												p_info.FAX_ID,
												p_info.DEPT_CD,
												p_info.TASK_ID,
												p_info.USER_ID,
												p_info.CID,
												p_info.CID_NAME,
												p_info.EMAIL_NOTIFY_YN,
												p_info.EMAIL_NOTIFY,
												message
												));
		}

		private void LogError(string p_strMsg)
		{
			LogWrite(LOG_LEVEL.ERR, p_strMsg);
		}

		private void LogMessage(string p_strMsg)
		{
			LogWrite(LOG_LEVEL.MSG, p_strMsg);
		}

		private void LogWrite(LOG_LEVEL p_logLevel, string p_strMsg)
		{
			p_strMsg = String.Format("[TH:{0:D02}]{1}", base.ThreadNo, p_strMsg);
			AppLog.Write(p_logLevel, p_strMsg);
		}

		private RESULT InsertProcessLog(decimal faxId, bool isSuccess, params string[] descriptions)
		{
			return DbModule.Instance.PSI_InsertProcessLog
				(
					P_TYPE.PSI
					, Config.SYSTEM_PROCESS_ID
					, faxId
					, isSuccess == true ? RESULT.SUCCESS : RESULT.F_SYSTEM_ERROR
					, string.Format("작업 성공 : {0} / {1}", isSuccess.ToString(), string.Join(" / ", descriptions))
				);
		}
		#endregion

		#region delegate
		public delegate void delegate_DbPollingCheck(string p_strDot);
		#endregion

		#region field
		private List<DbModule.RECV_FAX_DATA_PSI> m_lstPostProcessingReqs = new List<DbModule.RECV_FAX_DATA_PSI>();
		//private List<BtfaxThread> m_lstPostProcessingThreads = new List<BtfaxThread>();


		//// 기정의 항목 ////
		private const int SOCK_SEND_TIMEOUT = 1000 * 5;       //WCD(2014-05-28)
		private const int SOCK_RECV_TIMEOUT = 1000 * 5;       //WCD(2014-05-28)
		private const int RET_FAX_PACKET_LEN = 704;           //WCD(2014-05-28)
		private const int RET_FAX_HEAD_LEN = 600;             //WCD(2014-05-28) 
		#endregion
	}
}
