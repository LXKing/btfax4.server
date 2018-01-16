using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using Btfax.CommonLib.Log;

namespace FaxAgent.Server
{
    class UdpSocket
    {
        #region public event
        public delegate void UdpMessageSentEventHandler(byte[] messageBytes);
        public event UdpMessageSentEventHandler OnSent;

        public delegate void UdpMessageReceivedEventHandler(byte[] messageBytes);
        public event UdpMessageReceivedEventHandler OnReceived;

        public delegate void UdpMessageRaisedErrorEventHandler(string methodName, string errorString);
        public event UdpMessageRaisedErrorEventHandler OnRaisedError;
        #endregion

        #region constructor
        public UdpSocket()
        {
        }

        public void UdpSocketInit(int ListenPort, int BufferSize, string LogPrefix, bool useListen)
        {
            /// 로그 설정
			//m_fileLog = new FileLog();
			//m_fileLog.LogLevel = LOG_LEVEL.MSG;
			//m_fileLog.LogPrefix = LogPrefix;
			
			
            this.LogWrite("UdpSocket()", "UdpSocketInit()");

            /// 인코딩 설정
            m_Encoding = Encoding.Default;

            /// 변수 설정
            m_ReceiveIpep = new IPEndPoint(IPAddress.Any, ListenPort);
            m_BufferSize = BufferSize;
            m_UseListen = useListen;

            /// 소켓 생성
            m_Socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        }

        /// <summary>
        /// Udp Socket 생성 -> 수신 대기
        /// </summary>
        public void StartListen()
        {
            if (m_UseListen == false) return;


            if (m_UseListen) m_Socket.Bind(m_ReceiveIpep);

            this.LogWrite("UdpSocket()", "Create Udp Socket.", m_ReceiveIpep.ToString());

            /// 수신 대기
            this.UdpBeginReceive();
        }
        #endregion

        #region methods - Send
        public bool Send(string ip, int port, string message)
        {
            bool result = false;
            try
            {
                if (m_Socket != null)
                {
                    byte[] messageBytes = m_Encoding.GetBytes(message);

                    m_SendIpep = new IPEndPoint(IPAddress.Parse(ip), port);
                    m_Socket.BeginSendTo(messageBytes, 0, messageBytes.Length, SocketFlags.None, m_SendIpep, new AsyncCallback(SendCallback), messageBytes);
                    result = true;
                }
                else
                {
                    this.LogWrite("Send()", "Can't sent to a null socket.");

                    if (OnRaisedError != null) OnRaisedError("Send", "Can't sent to a null socket.");   /// 에러 이벤트 발생
                }
            }
            catch (ObjectDisposedException ex)
            {
                this.LogWriteErr(ex, "Send()");

                if (OnRaisedError != null) OnRaisedError("Send:ObjectDisposedException", ex.ToString());    /// 에러 이벤트 발생
            }
            catch (Exception ex)
            {
                this.LogWriteErr(ex, "Send()");

                if (OnRaisedError != null) OnRaisedError("Send:Exception", ex.ToString());  /// 에러 이벤트 발생
            }
            return result;
        }

        private void SendCallback(IAsyncResult result)
        {
            try
            {
                int sendByte = m_Socket.EndSend(result);

                /// 이벤트 발생
                if (OnSent != null) OnSent((result.AsyncState as byte[]));

                this.LogWrite("SendCallback()", "SendTo", m_SendIpep.ToString(), "SendByte", sendByte.ToString(), "byte");
            }
            catch (Exception ex)
            {
                this.LogWriteErr(ex, "SendCallback()");

                if (OnRaisedError != null) OnRaisedError("SendCallback:Exception", ex.ToString());  /// 에러 이벤트 발생
            }
        }
        #endregion

        #region methods - Receive
        /// <summary>
        /// Udp Socket 비동기 받기 시작
        /// </summary>
        private void UdpBeginReceive()
        {
            if (m_Socket == null)
            {
                this.LogWrite("UdpBeginReceive()", "Udp Socket is null.");

                if (OnRaisedError != null) OnRaisedError("UdpBeginReceive", "Udp Socket is null.");  /// 에러 이벤트 발생

                return;
            }

            try
            {
                m_Buffer = new byte[m_BufferSize];
                m_Socket.BeginReceiveFrom(m_Buffer, 0, m_Buffer.Length, SocketFlags.None, ref m_ReceiveIpep, new AsyncCallback(ReceiveCallback), null);

                this.LogWrite("UdpBeginReceive()", "Udp Socket begin async receive.");
            }
            catch (ObjectDisposedException ex)
            {
                this.LogWriteErr(ex, "UdpBeginReceive()", "ObjectDisposedException", "Socket is Disposed");

                if (OnRaisedError != null) OnRaisedError("UdpBeginReceive:ObjectDisposedException", ex.ToString());     /// 에러 이벤트 발생

                /// 소켓 종료
                this.CloseClientSocket();
                /// 수신 대기
                this.StartListen();
            }
            catch (Exception ex)
            {
                this.LogWriteErr(ex, "UdpBeginReceive()", "Exception");

                if (OnRaisedError != null) OnRaisedError("UdpBeginReceive:Exception", ex.ToString());   /// 에러 이벤트 발생

                /// 소켓 종료
                this.CloseClientSocket();
                /// 수신 대기
                this.StartListen();
            }
        }

        /// <summary>
        /// Udp Socket Receive Callback 함수
        /// </summary>
        private void ReceiveCallback(IAsyncResult ar)
        {
            if (m_Socket == null)
            {
                this.LogWrite("ReceiveCallback()", "Udp Socket is null.");

                if (OnRaisedError != null) OnRaisedError("ReceiveCallback", "Udp Socket is null."); /// 에러 이벤트 발생

                return;
            }

            try
            {
                EndPoint remoteEnd = (EndPoint)new IPEndPoint(IPAddress.Any, 0);

                int readByte = m_Socket.EndReceiveFrom(ar, ref remoteEnd);

                /// 이벤트 발생
                if (OnReceived != null) OnReceived(m_Buffer.Take(readByte).ToArray());

                this.LogWrite("ReceiveCallback()", "ReceivedFrom", remoteEnd.ToString(), "ReceivedData", m_Encoding.GetString(m_Buffer, 0, readByte));
            }
            catch (ArgumentNullException ex)
            {
                this.LogWriteErr(ex, "ReceiveCallback()", "Catch ArgumentNullException");

                if (OnRaisedError != null) OnRaisedError("ReceiveCallback:ArgumentNullException", ex.ToString());   /// 에러 이벤트 발생
            }
            catch (ArgumentException ex)
            {
                this.LogWriteErr(ex, "ReceiveCallback()", "Catch ArgumentException");

                if (OnRaisedError != null) OnRaisedError("ReceiveCallback:ArgumentException", ex.ToString());   /// 에러 이벤트 발생
            }
            catch (ObjectDisposedException ex)
            {
                this.LogWriteErr(ex, "ReceiveCallback()", "Catch ObjectDisposedException");

                if (OnRaisedError != null) OnRaisedError("ReceiveCallback:ObjectDisposedException", ex.ToString());   /// 에러 이벤트 발생
            }
            catch (InvalidOperationException ex)
            {
                this.LogWriteErr(ex, "ReceiveCallback()", "Catch InvalidOperationException");
                
                if (OnRaisedError != null) OnRaisedError("ReceiveCallback:InvalidOperationException", ex.ToString());   /// 에러 이벤트 발생
            }
            catch (SocketException ex)
            {
                this.LogWriteErr(ex, "ReceiveCallback()", "Catch SocketException");

                if (OnRaisedError != null) OnRaisedError("ReceiveCallback:SocketException", ex.ToString());   /// 에러 이벤트 발생

                if (ex.ErrorCode == 10040)
                {
                    this.m_BufferSize *= 2;     // 버퍼 크기 X 2
                    this.m_Buffer = new byte[this.m_BufferSize];

                    this.LogWriteErr("BufferSize UP ->", m_BufferSize.ToString(), "byte");

                    // 재전송 요청 (현재 구현 안됨)
                }
            }
            catch (Exception ex)
            {
                this.LogWriteErr(ex, "ReceiveCallback()", "Catch Exception");

                if (OnRaisedError != null) OnRaisedError("ReceiveCallback:Exception", ex.ToString());   /// 에러 이벤트 발생
            }

            this.UdpBeginReceive();
        }
        #endregion

        #region methods

        public void CloseClientSocket()
        {
            try
            {
                m_Socket.Close();
            }
            catch (Exception ex)
            {
                this.LogWriteErr(ex, "CloseClientSocket()");

                if (OnRaisedError != null) OnRaisedError("CloseClientSocket:Exception", ex.ToString());   /// 에러 이벤트 발생
            }
            m_Socket = null;
        }
        #endregion

        #region methods - Log (UDP 소켓 전용 로그 기록)

        public void LogWrite(params string[] messages)
        {
			AppLog.Write(LOG_LEVEL.MSG, string.Join("\t", messages));
            //m_fileLog.Write(LOG_LEVEL.MSG, string.Join("\t", messages));
        }

        public void LogWriteErr(params string[] messages)
        {
			AppLog.Write(LOG_LEVEL.ERR, string.Join("\t", messages));
            //m_fileLog.Write(LOG_LEVEL.ERR, string.Join("\t", messages));
        }

        public void LogWriteErr(Exception ex, params string[] messages)
        {
			AppLog.Write(LOG_LEVEL.ERR, string.Join("\t", messages, ex.Message, ex.StackTrace));
            //m_fileLog.Write(LOG_LEVEL.ERR, string.Join("\t", messages, ex.Message, ex.StackTrace));
        }

        public string GetString(byte[] bytes)
        {
            return m_Encoding.GetString(bytes);
        }

        #endregion

        #region field
        private Encoding m_Encoding;
        private EndPoint m_ReceiveIpep;
        private EndPoint m_SendIpep;
        private Int32 m_BufferSize;

        private Socket m_Socket;
        private byte[] m_Buffer;

        private bool m_UseListen;

        //public FileLog m_fileLog;
        #endregion
    }
}
