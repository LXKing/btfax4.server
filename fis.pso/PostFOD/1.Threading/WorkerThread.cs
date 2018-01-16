using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Management;
using Btfax.CommonLib;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Util;
using PostFOD.Util;
using PostFOD.Db;

namespace PostFOD.Threading
{   
    class WorkerThread : BtfaxThread
    {
        #region static
        static WorkerThread s_instance = null;
        static public WorkerThread Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new WorkerThread();
                return s_instance;
            }
        }
        #endregion

        #region constructor
        public WorkerThread() { }
        #endregion

        #region override
        public override bool StartThread()
        {
            return base.StartThread();
        }

        public override bool JoinThread(int p_nTimeout = -1)
        {
            return base.JoinThread(p_nTimeout);
        }

        protected override void ThreadEntry()
        {   
            System.Threading.Thread.Sleep(Config.INITIAL_SLEEP);
            LogMessage("### DB 폴링 시작 ###");

            RESULT result = RESULT.EMPTY;
            while (!BtfaxThread.Stop)
            {
                ClearReqInfo();
                result = DbModule.Instance.FetchPostFODReqs(ref m_lstSendReqInfos);
                switch (result)
                {
                    //// 발송후처리건 없음 ////
                    case RESULT.EMPTY:
                        Thread.Sleep(Config.DB_POLLING_SLEEP);
                        continue;

                    //// 발송후처리건 있음 ////
                    case RESULT.SUCCESS:
                        break;

                    //// DbPolling 오류 ////
                    default:
                        LogError("Db폴링중 오류가 발생하였습니다", result);
                        Thread.Sleep(Config.DB_POLLING_SLEEP);
                        continue;
                }

                foreach (DbModule.SEND_REQUEST_DATA_PSO mstrReq in m_lstSendReqInfos)
                {
                    foreach (DbModule.SEND_REQUEST_DTL_DATA_PSO dtlReq in mstrReq.m_lstSendRequestDetails)
                    {   
						//// 1. UDP 소켓 처리						
						if (UdpSingle.Instance.SendMessageEx(mstrReq, dtlReq))
						    LogMessage("Send noti success.", RESULT.SUCCESS);
						else
						    LogError(string.Format("Send receive noti fail."));
						
                        //// 2. 업데이트 처리 
						if (!DbModule.Instance.ResultProcessingReq(mstrReq.faxId, dtlReq.faxDtlId, dtlReq.strResult, dtlReq.strReason))
							LogError("송신후처리 결과를 업데이트도중 오류가 발생하였습니다.");

						Thread.Sleep(10);
                    }
                }
                
                Thread.Sleep(Config.DB_POLLING_SLEEP);
            }

            LogMessage("### 쓰레드 종료 ###");            
        }
        #endregion

		#region implementations
		private void ClearReqInfo()
        {
            m_lstSendReqInfos.Clear();
        }

        private void LogMessage(string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            if(p_result != RESULT.EMPTY)
                AppLog.Write(LOG_LEVEL.MSG, string.Format("[RESULT:{0}]{1}", p_result, p_strMsg));
            else
                AppLog.Write(LOG_LEVEL.MSG, string.Format("{0}", p_strMsg));
        }

        private void LogError(string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            if (p_result != RESULT.EMPTY)
                AppLog.Write(LOG_LEVEL.ERR, string.Format("[RESULT:{0}]{1}", p_result, p_strMsg));
            else
                AppLog.Write(LOG_LEVEL.ERR, string.Format("{0}", p_strMsg));
        }
        #endregion

        #region field
        private List<DbModule.SEND_REQUEST_DATA_PSO> m_lstSendReqInfos = new List<DbModule.SEND_REQUEST_DATA_PSO>();
        #endregion
    }
}



