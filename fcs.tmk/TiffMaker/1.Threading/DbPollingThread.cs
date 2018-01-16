using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.IO;
using Btfax.CommonLib;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using TiffMaker.Db;
using TiffMaker.Util;

namespace TiffMaker.Threading
{
    class DbPollingThread : BtfaxThread
    {
        #region static
        static DbPollingThread s_instance = null;
        static public DbPollingThread Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new DbPollingThread();
                return s_instance;
            }
        }
        #endregion

        #region constructor
        public DbPollingThread() { }
        #endregion

        #region method
        private void StartMakingTiffThreads()
        {
            for (int i = 0; i < Config.MAKE_THREAD_CNT; i++ )
            {
                BtfaxThread thread = new MakingTiffThread();
                thread.StartThread();

                m_lstMakingTiffThreads.Add(thread);
            }
        }

        private void JoinMakingTiffThreads()
        {
            foreach (BtfaxThread thread in m_lstMakingTiffThreads)
                thread.JoinThread();

            m_lstMakingTiffThreads.Clear();
        }

        #endregion

        #region override
        public override bool StartThread()
        {
            //// TIFMaking 스레드 시작 ////
            StartMakingTiffThreads();

            //// DB폴링 스레드 시작 ////
            return base.StartThread();
        }

        public override bool JoinThread(int p_nTimeout = -1)
        {
            //// TIFMaking 스레드 종료 ////
            JoinMakingTiffThreads();

            //// DB폴링 스레드 종료 ////
            if (!base.JoinThread(p_nTimeout))
                return false;

            return true;
        }

        protected override void ThreadEntry()
        {
            AppLog.Write(LOG_LEVEL.MSG, "### DB폴링 쓰레드 시작 ###");
            Thread.Sleep(Config.INITIAL_SLEEP);
            AppLog.Write(LOG_LEVEL.MSG, "### DB폴링 시작 ###");

            ReadySendReq();
            
            while (!BtfaxThread.Stop)
            {
                Thread.Sleep(Config.DB_POLLING_SLEEP);

                if (!ExistsFaxStorage())
                {   
                    AppLog.Write(LOG_LEVEL.ERR, string.Format("팩스스토리지를 찾을수 없습니다. 위치:{0}", Config.STG_HOME_PATH));
                    continue;
                }

				//// Tiff Making 디렉터리 생성 ////
				if (!Directory.Exists(Config.TIFF_MAKING_PATH))
					Directory.CreateDirectory(Config.TIFF_MAKING_PATH);
            
                if (OutFaxQueue.Instance.Count() > 0)
                    continue;

                if (m_lstTmkSendReqInfos.Count > 0)
                    m_lstTmkSendReqInfos.Clear();

                RESULT result = DbModule.Instance.FetchSendListToMake(Config.PROCESS_TYPE
                                                                    , Config.SYSTEM_PROCESS_ID
                                                                    , Config.FETCH_CNT
                                                                    , ref m_lstTmkSendReqInfos);
                switch (result)
                {   
                    case RESULT.EMPTY:  
                    case RESULT.SUCCESS:
                        break;

                    default:
                        AppLog.Write(LOG_LEVEL.ERR, "DB 폴링 중 오류가 발생하였습니다");                        
                        continue;
                }

                if (m_lstTmkSendReqInfos.Count <= 0)
                    continue;

                //// QUEUE 삽입 ////
                foreach (DbModule.SEND_REQUEST_TMK tmkReqInfo in m_lstTmkSendReqInfos)
                    OutFaxQueue.Instance.Enqueue(tmkReqInfo);
            }

            AppLog.Write(LOG_LEVEL.MSG, "### DB 폴링 쓰레드 종료 ###");            
        }

        /// <summary>
        /// 실행시 자신의 변환 대기건 초기화
        /// </summary>
        protected void ReadySendReq()
        {
            try
            {   
                if (!DbModule.Instance.ReadySendRequest(Config.PROCESS_TYPE.ToString(), Config.SYSTEM_PROCESS_ID))
                {
                    // 알람 - Fault
                    AppLog.Write(LOG_LEVEL.WRN, "처리대기건 초기화 실패.");
                }
                else
                {
                    AppLog.Write(LOG_LEVEL.MSG, "처리대기건 초기화 완료.");
                }
            }
            catch (Exception ex)
            {
                // 알람 - Fault
                AppLog.Write(LOG_LEVEL.WRN
                    , string.Format("처리대기건 초기화 중 다음과 같은 오류가 발생하였습니다. 오류:{0}", ex.Message));
            }
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

        #region field
        private List<BtfaxThread> m_lstMakingTiffThreads = new List<BtfaxThread>();
        private List<DbModule.SEND_REQUEST_TMK> m_lstTmkSendReqInfos = new List<DbModule.SEND_REQUEST_TMK>();
        #endregion
    }
}
