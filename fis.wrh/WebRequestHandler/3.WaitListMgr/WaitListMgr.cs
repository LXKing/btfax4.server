using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading;
using System.Data.OracleClient;
using Btfax.CommonLib;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Threading;

namespace WebRequestHandler
{
    class WaitListMgr
    {
        #region static
        protected static WaitListMgr s_instance = null;
        public static WaitListMgr Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new WaitListMgr();
                return s_instance;
            }
        }
        #endregion

        #region constructor
        public WaitListMgr()
        {
            FetchWaitList();
        }
        #endregion

        #region method
        public bool FetchWaitList()
        {
            lock (this)
            {
                if (!DbModule.Instance.GetWaitList(m_dsWaitList))
                    return false;
                m_dtLastTime = DateTime.Now;
            }       
            
            return true;
        }

        public bool IsWaitList(decimal p_faxID, char p_chType)
        {
            int nCnt = 0;
            string strState = "";

            switch(p_chType)
            {
                case 'P': strState = Convert.ToInt32(R_STATE.PSC_WP).ToString(); break;
                case 'A': strState = Convert.ToInt32(R_STATE.PSC_WA).ToString(); break;
                default:  return false;
            }

            lock (this)
            {
                if ((DateTime.Now - m_dtLastTime).Seconds > Config.WAITLIST_INVALID_SECONDS)
                {
                    if (!FetchWaitList())
                        return false;
                }
            
                var query = from    data in m_dsWaitList.Tables[0].AsEnumerable()
                            where   ( data.Field<decimal>("FAX_ID") == p_faxID && 
                                      data.Field<string>("STATE") == strState
                                    )
                            select  data;
                nCnt = query.Count();
            }

            return (nCnt > 0) ? true : false;
        }
        #endregion

        #region field
        DataSet  m_dsWaitList = new DataSet();
        DateTime m_dtLastTime;
        #endregion
    }
}
