using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace BtfaxInstaller
{
	class ProcessHandler
	{
		public static bool StartProcess(string p_strFilePath, string p_strFileName, string p_strArgs = "")
		{
			string strFileFullname = string.Format("{0}\\{1}", p_strFilePath, p_strFileName);
			if(!File.Exists(strFileFullname))
			{
				SetErrMsg(string.Format("File not exists. ({0})", strFileFullname));
				return false;
			}

			FileInfo fInfo = new FileInfo(strFileFullname);
			
			bool ret = false;
			try
			{
				Process p = new Process();
				p.StartInfo.WorkingDirectory = fInfo.Directory.FullName;
				p.StartInfo.FileName = fInfo.FullName;
				p.StartInfo.CreateNoWindow = true;
				if (p_strArgs != "")
					p.StartInfo.Arguments = p_strArgs;

				p.Start();
				
				p.WaitForExit();
				ret = true;
			}
			catch(Exception ex)
			{	
				SetErrMsg(ex.Message);
				ret = false;
			}

			return ret;
		}

		public static bool StartProcess(string p_strFilePath, string p_strFileName, out string p_strRedirectMsg, string p_strArgs = "")
		{
			p_strRedirectMsg = "";
			string strFileFullname = string.Format("{0}\\{1}", p_strFilePath, p_strFileName);
			if (!File.Exists(strFileFullname))
			{
				SetErrMsg(string.Format("File not exists. ({0})", strFileFullname));
				return false;
			}

			FileInfo fInfo = new FileInfo(strFileFullname);

			bool ret = false;
			try
			{
				Process p = new Process();
				p.StartInfo.WorkingDirectory = fInfo.Directory.FullName;
				p.StartInfo.FileName = fInfo.FullName;
				p.StartInfo.CreateNoWindow = true;
				p.StartInfo.UseShellExecute = false;
				p.StartInfo.RedirectStandardOutput = true;
				if (p_strArgs != "")
					p.StartInfo.Arguments = p_strArgs;

				p.Start();
				string strOutput = p.StandardOutput.ReadToEnd();
				strOutput = strOutput.Replace("\r\r\n", "\n");
				p_strRedirectMsg = strOutput;

				p.WaitForExit();
				ret = true;
			}
			catch (Exception ex)
			{
				SetErrMsg(ex.Message);
				ret = false;
			}

			return ret;
		}

		/// <summary>
		/// 에러메시지 GET
		/// </summary>
		/// <returns></returns>
		public static string GetErrMsg()
		{
			return m_strErrMsg;
		}

		private static void SetErrMsg(string p_strMsg)
		{
			m_strErrMsg = p_strMsg;
		}

		private static string m_strErrMsg;
	}
}
