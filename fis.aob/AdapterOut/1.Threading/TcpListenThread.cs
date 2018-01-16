using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using Btfax.CommonLib;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using AdapterOut.Util;

namespace AdapterOut.Threading
{
    class TcpListenThread : BtfaxThread
    {
        #region static
        static TcpListenThread s_instance = null;
        static public TcpListenThread Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new TcpListenThread();
                return s_instance;
            }
        }
        #endregion

        #region constructor && destructor
        public TcpListenThread() { }
        ~TcpListenThread() { }
        #endregion

        #region method
        public void ListenLoop_()
        {
            bool bResult = ThreadPool.SetMaxThreads(Config.SESSION_CNT, Config.SESSION_CNT);
            
            Socket sock_listen = null;
            try
            {
                IPEndPoint ep = new IPEndPoint(IPAddress.Any, Config.LISTEN_PORT);

                sock_listen = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                sock_listen.Bind(ep);
                sock_listen.Listen(Config.LISTEN_PORT);
                AppLog.Write(LOG_LEVEL.MSG, string.Format("### 리슨 시작 : 포트[{0}] ###", Config.LISTEN_PORT));

                while (!BtfaxThread.Stop)
                {   
                    if (!sock_listen.Poll(ONE_SECOND, SelectMode.SelectRead))
                        continue;

                    Socket sock_session = sock_listen.Accept();

                    int nThreadCnt, nNotUse;
                    ThreadPool.GetAvailableThreads(out nThreadCnt, out nNotUse);
                    if (nThreadCnt <= 0)
                    {
                        AppLog.Write(LOG_LEVEL.WRN, "사용가능한 세션이 존재하지 않습니다. : GetAvailableThreads()");                        
                        SendErrorPacket(sock_session, RESULT.F_SESSION_FULL);
                        continue;
                    }
                    
                    TcpSessionThread sessionThread = AssignSessionThread_();
                    if (sessionThread == null) // add bok - 20130528
                    {
                        AppLog.Write(LOG_LEVEL.WRN, "사용가능한 세션이 존재하지 않습니다. : AssignSessionThread_()");
                        SendErrorPacket(sock_session, RESULT.F_SESSION_FULL);
                        continue;
                    }
 
                    sessionThread.Socket = sock_session;
                    ThreadPool.QueueUserWorkItem(new WaitCallback(sessionThread.ThreadEntry));
                }
            }
            catch (Exception ex)
            {   
                AppLog.ExceptionLog(ex, "리슨 소켓 쓰레드에서 다음과 같은 오류가 발생하였습니다.");
            }

            if (sock_listen != null)
            {
                sock_listen.Close();
            }

            AppLog.Write(LOG_LEVEL.MSG, string.Format("### 리슨 종료 : 포트[{0}] ###", Config.LISTEN_PORT));
        }

        public TcpSessionThread AssignSessionThread_()
        {
            TcpSessionThread thread = null;

            lock (this.m_sessionThreads)
            {
                foreach (TcpSessionThread th in this.m_sessionThreads)
                {
                    if (!th.Running)
                    {
                        th.Running = true;
                        return th;
                    }
                }
                if (this.m_sessionThreads.Count >= Config.SESSION_CNT) // add bok - 20130528
                    return null;   

                thread = new TcpSessionThread();
                thread.Running = true;
                this.m_sessionThreads.Add(thread);
            }

            return thread;
        }

        protected void SendErrorPacket(Socket p_sessionSocket, RESULT p_result)
        {
            int reasonCode = (int)p_result;
            string desc = p_result.ToString();

            byte[] responBuffer = new byte[RESPONSE_BUFFER_SIZE];
            byte[] buf = Encoding.Default.GetBytes(String.Format("{0:D3}|{1:D3}|{2}|{3}", RESPONSE_BUFFER_SIZE, reasonCode, desc, -1));
            Buffer.BlockCopy(buf, 0, responBuffer, 0, buf.Length);
            if (p_sessionSocket != null && p_sessionSocket.Connected)
                p_sessionSocket.Send(responBuffer);
        }
        #endregion

        #region override
        protected override void ThreadEntry()
        {
            AppLog.Write(LOG_LEVEL.MSG, "### 리슨 소켓 쓰레드 시작 ###");

            ListenLoop_();

            AppLog.Write(LOG_LEVEL.MSG, "### 리슨 소켓 쓰레드 종료 ###");
        }
        #endregion

        #region field
        List<TcpSessionThread> m_sessionThreads = new List<TcpSessionThread>();
        static readonly int ONE_SECOND = 1000 * 1000;
        static readonly int RESPONSE_BUFFER_SIZE = 200;
        #endregion
    }
}
