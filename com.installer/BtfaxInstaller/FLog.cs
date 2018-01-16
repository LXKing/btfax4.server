using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace BtfaxInstaller
{
	public enum LogLevel
	{
		None = 0,
		Info = 1,
		Wrn = 2,
		Err = 3,
	}

	public class FLog 
	{
		#region static - property
		
		#endregion

		#region method
		protected static void Write(LogLevel p_logLevel, string p_strMessage)
		{
			if (p_strMessage.Length <= 0)
				return;

			if (string.IsNullOrEmpty(m_strLogPath))
				return;

			lock (typeof(FLog))
			{
				try
				{
					string strFileName = string.Format("{0}_{1:yyyyMMdd}.log", m_strLogPrefix, DateTime.Now);
					string strFileFullName = string.Format("{0}\\{1}", m_strLogPath, strFileName);


					// 파일 기록
					using (FileStream logFileStream = new FileStream(strFileFullName, FileMode.Append, FileAccess.Write, FileShare.ReadWrite))
					{
						StreamWriter logStreamWriter = new StreamWriter(logFileStream, Encoding.GetEncoding(949));
						logStreamWriter.WriteLine("[{0:yyyy/MM/dd HH:mm:ss}][{1}] {2}", DateTime.Now, p_logLevel, p_strMessage);
						logStreamWriter.Flush();
						logStreamWriter.Close();

						logFileStream.Close();
					}
				}
				catch
				{
					return;
				}
			}
		}

		public static void Msg(string p_strMsg)
		{
			Write(LogLevel.Info, p_strMsg);
		}

		public static void Err(string p_strMsg)
		{
			Write(LogLevel.Err, p_strMsg);
		}

		public static void Wrn(string p_strMsg)
		{
			Write(LogLevel.Wrn, p_strMsg);
		}


		public static void Init(string p_strPath, string p_strPreFix)
		{
			m_strLogPath = p_strPath;
			m_strLogPrefix = p_strPreFix;
		}
		#endregion

		#region field

		protected static string m_strLogPath;
		protected static string m_strLogPrefix;

		#endregion
	}
}
