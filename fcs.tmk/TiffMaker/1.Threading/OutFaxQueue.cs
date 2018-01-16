using System;
using Btfax.CommonLib.Util;
using TiffMaker.Db;

namespace TiffMaker.Threading
{
    class OutFaxQueue : QueueThreadSafe< DbModule.SEND_REQUEST_TMK >
    {
        #region static
        static OutFaxQueue s_instance = null;
        static public OutFaxQueue Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new OutFaxQueue();
                return s_instance;
            }
        }
        #endregion

        #region constructor
        public OutFaxQueue() { }
        #endregion
    }
}
