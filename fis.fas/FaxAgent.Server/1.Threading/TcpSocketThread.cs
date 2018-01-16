using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Threading;

namespace FaxAgent.Server
{
    class TcpSocketThread : BtfaxThread
    {
        #region constructor && destructor
        public TcpSocketThread() { }
        ~TcpSocketThread() { }
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
                        if (m_SocketListener == null)
                        {
                            m_SocketListener = new SocketListener();
                        }
                    }
                    catch (Exception ex)
                    {
                        lastExecuteTime = DateTime.Now;
                        AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다.", m_ThreadName));
                        AppLog.ExceptionLog(ex, "리슨 소켓 쓰레드에서 다음과 같은 오류가 발생하였습니다.");
                    }
                }

                m_SocketListener.CleanUpOnExit();
                m_SocketListener = null;
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다. 쓰레드를 종료합니다.", m_ThreadName));
                throw ex;
            }
        }
        #endregion

        #region properties
        public static TcpSocketThread Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new TcpSocketThread();
                return s_instance;
            }
        }
        private static TcpSocketThread s_instance = null;

        /// <summary>
        ///  현재 쓰레드 이름
        /// </summary>
        private string m_ThreadName { get { return "TCP Socket"; } }
        public string ThreadName { get { return m_ThreadName; } }
        #endregion

        #region fields
        public SocketListener m_SocketListener;
        #endregion
    }
}
