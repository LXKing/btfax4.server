using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using PostFOD.Util;

using PostFOD.Db;
using FaxAgent.Server;
using FaxAgent.Common.Socket;

namespace PostFOD
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
                    udpSocket.UdpSocketInit(0, Config.BUFFER_SIZE * 2, string.Format("FAX_{0}_PSO_{1}_UdpSocket", Config.SYSTEM_NO, Config.PROCESS_NO), useListen);
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
                        Config.SEND_NOTIFY_TITLE,
                        Config.SEND_NOTIFY_MESSAGE,
                        NotifyTextMessageUrlType.UrlWithFaxboxId_UserId.ToString(),
                        Config.SEND_FAXBOX_URL
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
		public bool SendMessageEx(DbModule.SEND_REQUEST_DATA_PSO mstrReq, DbModule.SEND_REQUEST_DTL_DATA_PSO dtlReq)
		{
			bool result = false;
			try
			{
				string msg = Config.SEND_NOTIFY_MESSAGE;

				string userName = "";
				if (!string.IsNullOrEmpty(mstrReq.strReqUserName))
					userName = mstrReq.strReqUserName;
				else
					userName = mstrReq.strUserName;
				

				//{FAX_ID},{FAX_DTL_ID},{USER_NAME},{FAX_NO},{RESULT},{CURR_TIME}

				if (msg.Contains("{FAX_ID}"))
					msg = msg.Replace("{FAX_ID}", string.Format("{0}", dtlReq.faxId));

				if (msg.Contains("{FAX_DTL_ID}"))
					msg = msg.Replace("{FAX_DTL_ID}", string.Format("{0}", dtlReq.faxDtlId));

				if (msg.Contains("{USER_NAME}"))
					msg = msg.Replace("{USER_NAME}", userName);
				
				if (msg.Contains("{FAX_NO}"))
					msg = msg.Replace("{FAX_NO}", dtlReq.strFaxNo);

				if (msg.Contains("{RESULT}"))
					msg = msg.Replace("{RESULT}", dtlReq.strResult);

				if (msg.Contains("{CURR_TIME}"))
					msg = msg.Replace("{CURR_TIME}", string.Format("{0:yyyy-MM-dd HH:mm:ss}", DateTime.Now));


				/// 파라미터 만들기
				MessageStream notify =
					MessageStream.Notice
					(
						MessagePacketNameEnum.NTFTM,
						mstrReq.strReqUserID,
						Config.SYSTEM_PROCESS_ID,
						NotifyTextMessageType.OUTBOUND.ToString(),
						Config.SEND_NOTIFY_TITLE,
						msg,
						NotifyTextMessageUrlType.UrlWithFaxboxId_UserId.ToString(),
						//NotifyTextMessageUrlType.UrlWithUserId.ToString(),
						Config.SEND_FAXBOX_URL
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
