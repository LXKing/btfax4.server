using System;
using System.Net;
using Btfax.CommonLib.Util;

namespace WebRequestHandler
{
    class WebRequestQueue : QueueThreadSafe<HttpListenerContext>
    {
        #region static
        static WebRequestQueue s_instance = null;
        static public WebRequestQueue Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new WebRequestQueue();
                return s_instance;
            }
        }
        #endregion
    }
}
