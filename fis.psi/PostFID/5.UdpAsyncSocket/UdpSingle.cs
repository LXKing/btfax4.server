using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using PostFid.Util;
using PostFid.DB;
using FaxAgent.Server;
using FaxAgent.Common.Socket;

namespace PostFid
{
    class UdpSingle
    {
        #region constructor
        private UdpSingle()
        {
        }
        #endregion

        #region methods
        public void InitUdpSocket(bool useListen)
        {
            if (udpSocket == null)
            {
                try
                {
                    udpSocket = new UdpSocket();
                    udpSocket.UdpSocketInit(0, Config.BUFFER_SIZE * 2, string.Format("FAX_{0}_PSI_{1}_UdpSocket", Config.SYSTEM_NO, Config.PROCESS_NO), useListen);
                    udpSocket.OnSent += new UdpSocket.UdpMessageSentEventHandler(UdpMessage_OnSent);
                    udpSocket.OnReceived += new UdpSocket.UdpMessageReceivedEventHandler(UdpMessage_OnReceived);
                    udpSocket.OnRaisedError += new UdpSocket.UdpMessageRaisedErrorEventHandler(UdpMessage_OnRaisedError);
                }
                catch (Exception ex)
                {   
                    udpSocket.LogWriteErr(ex, "UdpSingle()", "InitUdpSocket()");
                }
            }
        }

        private void UdpMessage_OnSent(byte[] messageBytes)
        {
            udpSocket.LogWrite("SEND ->", Encoding.Default.GetString(messageBytes));
        }

        private void UdpMessage_OnReceived(byte[] messageBytes)
        {
            udpSocket.LogWrite("RECV <-", Encoding.Default.GetString(messageBytes));
        }

        private void UdpMessage_OnRaisedError(string methodName, string errorString)
        {   
        }

        /// <summary>
        /// UDP 메세지 보내기
        /// </summary>
        /// <param name="userId">파라미터1 : USER_ID</param>
        /// <param name="message">파라미터2 : 수신 메세지</param>
        public bool SendMessage(string userId)
        {
            bool result = false;
            try
            {
                /// 파라미터 만들기
                MessageStream notify =
                    MessageStream.Notice
                    (
                        MessagePacketNameEnum.NTFTM,
                        userId,
                        Config.SYSTEM_PROCESS_ID,
                        NotifyTextMessageType.INBOUND.ToString(),
                        Config.RECV_NOTIFY_TITLE,
                        Config.RECV_NOTIFY_MESSAGE,
                        NotifyTextMessageUrlType.UrlWithFaxboxId_UserId.ToString(),
                        Config.RECV_FAXBOX_URL
                    );
                
                /// STX, ETX 추가
                byte[] convertBytes = Encoding.Default.GetBytes(notify.ToString());
                byte[] messageBytes = new byte[convertBytes.Length + 2];
                messageBytes[0] = MessageConfiguration.STX;
                messageBytes[messageBytes.Length - 1] = MessageConfiguration.ETX;
                Buffer.BlockCopy(convertBytes, 0, messageBytes, 1, convertBytes.Length);
                
                /// UDP 보내기
                udpSocket.Send(Config.FAS_IP, Config.FAS_PORT, Encoding.Default.GetString(messageBytes));
                
                result = true;
            }
            catch (Exception ex)
            {   
                udpSocket.LogWriteErr(ex, "UdpSingle()", "UdpSocket()", "SendMessage()");
            }
            return result;
        }


		/// <summary>
		/// UDP 메세지 보내기
		/// </summary>
		/// <param name="userId">파라미터1 : USER_ID</param>
		/// <param name="message">파라미터2 : 수신 메세지</param>
		public bool SendMessageEx(string userId, DbModule.RECV_FAX_DATA_PSI p_psiData)
		{
			bool result = false;
			try
			{
				string msg = Config.RECV_NOTIFY_MESSAGE;

				//{USER_ID},{DEPT_CD},{DEPT_NAME},{TASK_NAME},{CID},{CID_NAME},{CURR_TIME}

				if (msg.Contains("{USER_ID}"))
					msg = msg.Replace("{USER_ID}", p_psiData.USER_ID);

				if (msg.Contains("{DEPT_CD}"))
					msg = msg.Replace("{DEPT_CD}", p_psiData.DEPT_CD);

				if (msg.Contains("{DEPT_NAME}"))
					msg = msg.Replace("{DEPT_NAME}", p_psiData.DEPT_NAME);

				if (msg.Contains("{TASK_NAME}"))
					msg = msg.Replace("{TASK_NAME}", p_psiData.TASK_NAME);

				if(msg.Contains("{CID}"))
					msg = msg.Replace("{CID}"		, p_psiData.CID);

				if (msg.Contains("{CID_NAME}"))
					msg = msg.Replace("{CID_NAME}"	, p_psiData.CID_NAME);

				if (msg.Contains("{CURR_TIME}"))
					msg = msg.Replace("{CURR_TIME}"	, string.Format("{0:yyyy-MM-dd HH:mm:ss}", DateTime.Now));

				/// 파라미터 만들기
				MessageStream notify =
					MessageStream.Notice
					(
						MessagePacketNameEnum.NTFTM,
						userId,
						Config.SYSTEM_PROCESS_ID,
						NotifyTextMessageType.INBOUND.ToString(),
						Config.RECV_NOTIFY_TITLE,
						msg,
						NotifyTextMessageUrlType.UrlWithFaxboxId_UserId.ToString(),
						//NotifyTextMessageUrlType.UrlWithUserId.ToString(),
						Config.RECV_FAXBOX_URL
					);

				/// STX, ETX 추가
				byte[] convertBytes = Encoding.Default.GetBytes(notify.ToString());
				byte[] messageBytes = new byte[convertBytes.Length + 2];
				messageBytes[0] = MessageConfiguration.STX;
				messageBytes[messageBytes.Length - 1] = MessageConfiguration.ETX;
				Buffer.BlockCopy(convertBytes, 0, messageBytes, 1, convertBytes.Length);

				/// UDP 보내기
				udpSocket.Send(Config.FAS_IP, Config.FAS_PORT, Encoding.Default.GetString(messageBytes));

				result = true;
			}
			catch (Exception ex)
			{
				udpSocket.LogWriteErr(ex, "UdpSingle()", "UdpSocket()", "SendMessage()");
			}
			return result;
		}
        #endregion

        #region static
        private static UdpSingle s_instance = null;
        public static UdpSingle Instance
        {
            get
            {
                if (UdpSingle.s_instance == null)
                    UdpSingle.s_instance = new UdpSingle();

                return UdpSingle.s_instance;
            }
        }
        #endregion

        #region fields
        private UdpSocket udpSocket = null;
        #endregion
    }
}
