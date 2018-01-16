using System;
using Btfax.CommonLib.Util;
using PostFcs.DB;



namespace PostFcs.Threading
{
    class PostProcessingQueue : QueueThreadSafe<DbModule.SEND_REQUEST_PSC>
    {
        #region fields
        private static PostProcessingQueue s_instance = null;
        #endregion

        #region properties
        public static PostProcessingQueue Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new PostProcessingQueue();
                return s_instance;
            }
        }
        #endregion
    }
}
