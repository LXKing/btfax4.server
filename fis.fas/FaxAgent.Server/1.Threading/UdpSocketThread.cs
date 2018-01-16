using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Threading;
using FaxAgent.Common.Socket;

namespace FaxAgent.Server
{
    class UdpSocketThread : BtfaxThread
    {
        #region constructor && destructor
        public UdpSocketThread() { }
        ~UdpSocketThread() { }
        #endregion

        #region IMPLEMENT
        /// <summary>
        /// Name       : ThreadEntry
        /// Content    : 쓰레드 진입점
        /// Writer     : 장동훈
        /// Date       : 2012.10.24
        /// </summary>
        protected override void ThreadEntry()
        {
            AppLog.Write(LOG_LEVEL.MSG, string.Format("### {0} 쓰레드 시작 ###", m_ThreadName));
            PollingLoop_();
            AppLog.Write(LOG_LEVEL.MSG, string.Format("### {0} 쓰레드 종료 ###", m_ThreadName));
        }

        /// <summary>
        /// Name       : PollingLoop_
        /// Content    : 폴링 루프
        /// Writer     : 장동훈
        /// Date       : 2012.10.24
        /// </summary>
        private void PollingLoop_()
        {
            /// 마지막 실행시간 (Sleep 실행의 기준시간)
            /// 첫 Sleep 실행시 바로 실행 되도록 시간 수정
            DateTime lastExecuteTime = DateTime.Now.AddMilliseconds(Config.UDP_CHECK_POLLING_TIME * -1);

            try
            {
                while (!BtfaxThread.Stop)
                {
                    if (DateTime.Now.Subtract(lastExecuteTime).TotalMilliseconds < Config.UDP_CHECK_POLLING_TIME)
                    {
                        Thread.Sleep(1000);
                        continue;
                    }

                    lastExecuteTime = DateTime.Now;

                    try
                    {   
                        if (m_UdpSocket == null)
                        {
                            m_UdpSocket = new UdpSocket();
                            m_UdpSocket.UdpSocketInit(Config.LISTEN_PORT, Config.BUFFER_SIZE * 2, string.Format("FAX_{0}_FAS_{1}_UdpSocket", Config.SYSTEM_NO, Config.PROCESS_NO), true);
                            m_UdpSocket.OnSent += new UdpSocket.UdpMessageSentEventHandler(this.UdpMessage_OnSent);
                            m_UdpSocket.OnReceived += new UdpSocket.UdpMessageReceivedEventHandler(this.UdpMessage_OnReceived);
                            m_UdpSocket.OnRaisedError += new UdpSocket.UdpMessageRaisedErrorEventHandler(m_UdpSocket_OnRaisedError);

                            m_UdpSocket.StartListen();
                        }

                        lastExecuteTime = DateTime.Now;
                    }
                    catch (Exception ex)
                    {
                        lastExecuteTime = DateTime.Now;

                        m_UdpSocket.CloseClientSocket();
                        m_UdpSocket = null;

                        AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다.", m_ThreadName));
                    }
                }

                m_UdpSocket.CloseClientSocket();
                m_UdpSocket = null;
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다. 쓰레드를 종료합니다.", m_ThreadName));
                throw ex;
            }
        }
        #endregion

        #region method
        private void m_UdpSocket_OnRaisedError(string methodName, string errorString)
        {   
        }

        private void UdpMessage_OnSent(byte[] messageBytes)
        {
            //m_UdpSocket.LogWrite("UdpMessage_OnSent");
        }

        private void UdpMessage_OnReceived(byte[] messageBytes)
        {
            try
            {
                if (messageBytes.Length > 0 &&
                    messageBytes[0] == MessageConfiguration.STX &&
                    messageBytes[messageBytes.Length - 1] == MessageConfiguration.ETX)
                {
                    byte[] buff = new byte[messageBytes.Length - 2];
                    Buffer.BlockCopy(messageBytes, 1, buff, 0, buff.Length);

                    MessageReader reader = new MessageReader(buff);
                    string userId = reader.GetParam(0);

                    Program.s_TcpSocketListener.m_SocketListener.SendMessageNotiftyToClient(messageBytes, userId);
                }
            }
            catch (Exception ex)
            {
                m_UdpSocket.LogWriteErr(ex, "UdpMessage_OnReceived()");
            }
        }
        #endregion

        #region properties
        public static UdpSocketThread Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new UdpSocketThread();
                return s_instance;
            }
        }
        private static UdpSocketThread s_instance = null;

        /// <summary>
        ///  현재 쓰레드 이름
        /// </summary>
        private string m_ThreadName   { get { return "UDP Socket"; } }
        public string ThreadName { get { return m_ThreadName; } }
        #endregion

        #region fields
        private UdpSocket m_UdpSocket;
        #endregion
    }
}
