using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DCV_Callback
{
    public class FileLog
    {
        public static bool Initialize(string p_logPath, string p_preFix = "")
        {
            m_logPath = p_logPath;
            m_preFix = p_preFix;
            return true;
        }

        protected static string GetLogFileName()
        {
            string strLogFileName;
            if (string.IsNullOrEmpty(m_preFix))
                strLogFileName = string.Format("{0:yyyyMMdd}.log", DateTime.Now);
            else
                strLogFileName = string.Format("{0}_{1:yyyyMMdd}.log", m_preFix, DateTime.Now);

            return strLogFileName;
        }

        protected static string GetLogPath()
        {
            string strLogPath = string.Format("{0}\\", m_logPath);
            if (Directory.Exists(strLogPath))
                return strLogPath;

            Directory.CreateDirectory(strLogPath);
            return strLogPath;
        }

        public static void Write(string Msg)
        {
            try
            {
                string strLogFileName = GetLogFileName();
                if (string.IsNullOrEmpty(strLogFileName))
                    return;

                string strLogPath = GetLogPath();
                string strLogFullPath = string.Format("{0}{1}", strLogPath, strLogFileName);

                //// 파일 기록 ////
                FileStream logFileStream = new FileStream(strLogFullPath,
                                                 FileMode.Append,
                                                 FileAccess.Write,
                                                 FileShare.ReadWrite);

                StreamWriter logStreamWriter = new StreamWriter(logFileStream, Encoding.GetEncoding(949));

                logStreamWriter.WriteLine("[{0:yy/MM/dd hh:mm:ss.ffff}] {1}",
                                          DateTime.Now,
                                          Msg);
                logStreamWriter.Flush();
                logStreamWriter.Close();
                logStreamWriter.Dispose();

                logFileStream.Close();
                logFileStream.Dispose();
            }
            catch
            {
                return;
            }
        }

        private static string m_logPath = "";
        private static string m_preFix = "";
    }
}
