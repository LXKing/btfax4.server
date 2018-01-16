using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace WindowsService
{
	public enum LogType
	{
		NONE = 0,
		INF = 1,
		WRN = 2,
		ERR = 3,
	}

	public class FileLog
	{
		protected static string m_preFix = "";
		protected static string m_logHomePath = "";
		protected static string m_errMsg = "";

		public static bool Init(string p_logPath, string p_preFix)
		{
			try
			{	
				if (!Directory.Exists(p_logPath))
					Directory.CreateDirectory(p_logPath);
				
				m_logHomePath = p_logPath;
				m_preFix = p_preFix;

				return true;
			}
			catch (Exception ex)
			{
				m_errMsg = ex.Message;
				return false;
			}
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
			string strLogPath = string.Format("{0}\\{1:yyyy-MM}\\", m_logHomePath, DateTime.Now);
			if (Directory.Exists(strLogPath))
				return strLogPath;

			Directory.CreateDirectory(strLogPath);
			return strLogPath;
		}

		protected static bool Write(LogType p_logLevel, string Msg)
		{	
			try
			{
				string strLogFileName = GetLogFileName();
				if (string.IsNullOrEmpty(strLogFileName))
				{
					m_errMsg = "Invalid Log File Name.";
					return false;
				}

				//string strLogPath = GetLogPath();
				string strLogFullPath = string.Format("{0}{1}", m_logHomePath, strLogFileName);

				//// 파일 기록 ////
				FileStream logFileStream = new FileStream(strLogFullPath,
													FileMode.Append,
													FileAccess.Write,
													FileShare.ReadWrite);

				StreamWriter logStreamWriter = new StreamWriter(logFileStream, Encoding.GetEncoding(949));

				logStreamWriter.WriteLine("[{0:hh:mm:ss.ff}][{1}] {2}",
											DateTime.Now,
											p_logLevel,
											Msg);
				logStreamWriter.Flush();
				logStreamWriter.Close();
				logStreamWriter.Dispose();

				logFileStream.Close();
				logFileStream.Dispose();

				return true;
			}
			catch (Exception ex)
			{
				m_errMsg = ex.Message;
				return false;
			}
			
		}

		public static void MSG(string p_msg, bool p_console = false)
		{
			if (p_console)
				Console.WriteLine(p_msg);

			if (!Write(LogType.INF, p_msg))
			{
			}
		}

		public static void WRN(string p_msg, bool p_console = false)
		{
			if (p_console)
				Console.WriteLine(p_msg);

			if (!Write(LogType.WRN, p_msg))
			{
			}
		}

		public static void ERR(string p_msg, bool p_console = false)
		{
			if (p_console)
				Console.WriteLine(p_msg);

			if (!Write(LogType.ERR, p_msg))
			{
			}
		}

	}
}
