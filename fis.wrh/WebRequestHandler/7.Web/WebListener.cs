using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using Btfax.CommonLib.Log;

namespace WebRequestHandler
{
    class WebListener
    {
        #region static
        static WebListener s_instance = null;
        static public WebListener Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new WebListener();

                return s_instance;
            }
        }
        #endregion

        #region method
        public bool Start()
        {
            try
            {
                AppLog.Write(LOG_LEVEL.MSG, string.Format("### 웹리스터 오픈 시도 : {0}", Config.HTTP_URL));
                m_listener.Prefixes.Add(Config.HTTP_URL);
                m_listener.Start();
                AppLog.Write(LOG_LEVEL.MSG, "### 웹리스터 오픈 시작됨 ###");

                IAsyncResult result = m_listener.BeginGetContext(new AsyncCallback(HttpRequestCallback), this);
            }
            catch(Exception ex)
            {
                AppLog.ExceptionLog(ex, "Web Listener 시작 시, 예외가 발생하였습니다.");
                return false;
            }

            return true;
        }

        public bool Stop()
        {
            try
            {
                m_listener.Stop();
                m_listener.Close();
            }
            catch
            {
                return false;
            }

            return true;
        }
        #endregion

        #region callback
        public void HttpRequestCallback(IAsyncResult p_result)
        {

            HttpListenerContext context = m_listener.EndGetContext(p_result);
            WebRequestQueue.Instance.Enqueue(context);

            IAsyncResult result = m_listener.BeginGetContext(new AsyncCallback(HttpRequestCallback), this);
        }
        #endregion

        #region field
        HttpListener m_listener = new HttpListener();
        #endregion
    }
}
