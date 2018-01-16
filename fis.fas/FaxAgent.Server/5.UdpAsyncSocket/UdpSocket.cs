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
        /// <summary>
        /// Send 이벤트
        /// </summary>
        public delegate void UdpMessageSentEventHandler(byte[] messageBytes);
        public event UdpMessageSentEventHandler OnSent;

        /// <summary>
        /// Receive 이벤트
        /// </summary>
        public delegate void UdpMessageReceivedEventHandler(byte[] messageBytes);
        public event UdpMessageReceivedEventHandler OnReceived;

        /// <summary>
        /// 에러 발생 이벤트
        /// </summary>
        /// <param name="methodName">에러발생 위치</param>
        /// <param name="errorString">에러 문구</param>
        public delegate void UdpMessageRaisedErrorEventHandler(string methodName, string errorString);
        public event UdpMessageRaisedErrorEventHandler OnRaisedError;
        #endregion

        #region constructor
        public UdpSocket()
        {
        }

        /// <summary>
        /// UDP Socket 초기화
        /// </summary>
        /// <param name="ListenPort">UDP Listen Port</param>
        /// <param name="BufferSize">버퍼 크기</param>
        /// <param name="LogPrefix">로그 파일 Prefix 이름</param>
        /// <param name="useListen">UDP Listen 사용 여부</param>
        public void UdpSocketInit(int ListenPort, int BufferSize, string LogPrefix, bool useListen, LOG_LEVEL logLevel = LOG_LEVEL.TRC)
        {   
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
            this.LogWrite("UdpSocket()", "Udp Socket begin async receive.");

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

                    /// 패킷 로그
                    this.LogWrite("SEND    ", m_SendIpep.ToString(), m_Socket.Handle.ToInt32(), messageBytes.Length, message);

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

                //this.LogWrite("SendCallback()", "SendTo", m_SendIpep.ToString(), "SendByte", sendByte.ToString(), "byte");
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

                //this.LogWrite("UdpBeginReceive()", "Udp Socket begin async receive.");
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
                if (OnReceived != null) 
					OnReceived(m_Buffer.Take(readByte).ToArray());

                /// 패킷 로그
                this.LogWrite("RECEIVE ", remoteEnd.ToString(), m_Socket.Handle.ToInt32(), readByte, m_Encoding.GetString(m_Buffer, 0, readByte));
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
                    // 
                    // 
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
			
        }

        public void LogWriteErr(params string[] messages)
        {
			AppLog.Write(LOG_LEVEL.ERR, string.Join("\t", messages));
        }

        public void LogWriteErr(Exception ex, params string[] messages)
        {
			AppLog.Write(LOG_LEVEL.ERR, string.Join("\t", messages, ex.Message, ex.StackTrace));
        }

        public void LogWrite(string direction, string recvSendIpPort, int socketHandle, int byteCount, string message)
        {
			AppLog.Write(LOG_LEVEL.TRC, string.Join("|",
                            "UdpSocket",
                            direction,
                            string.Format("{0,-21}", recvSendIpPort),
                            string.Format("T:{0,5}", System.Threading.Thread.CurrentThread.ManagedThreadId),
                            string.Format("H:{0,5}", socketHandle),
                            string.Format("{0,5} byte", byteCount),
                            message));
        }

        public string GetString(byte[] bytes)
        {
            return m_Encoding.GetString(bytes);
        }

        #endregion

        #region field
        private Encoding m_Encoding;        // 내부 인코딩
        private EndPoint m_ReceiveIpep;     // 수신 대기 IP 정보
        private EndPoint m_SendIpep;        // 보낼 IP 정보
        private Int32 m_BufferSize;         // 수신 대기 버퍼 크기

        private Socket m_Socket;            // UDP 소켓
        private byte[] m_Buffer;            // 수신 대기 버퍼

        private bool m_UseListen;           // UDP Listen 사용 여부
        #endregion
    }
}
