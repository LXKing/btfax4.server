using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Btfax.CommonLib.Log;

namespace AdapterOut.Util
{
    class AppLog : FileLog
    {
        static AppLog s_singleteon = null;
        static public AppLog Singleton
        {
            get
            {
                if (s_singleteon == null)
                    s_singleteon = new AppLog();

                return s_singleteon;
            }
        }

        public AppLog()
        {
        }

        public void ExceptionLog(Exception p_ex, String p_strMessage, params Object[] p_args)
        {
            Write(LOG_LEVEL.ERR, string.Format("{0}\t{1}\t{2}", p_strMessage, p_ex.Message, p_ex.StackTrace), p_args);
        }
    }
}
