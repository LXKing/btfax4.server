using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Text;

namespace BtfaxInstaller
{
	static class Program
	{
		/// <summary>
		/// 해당 응용 프로그램의 주 진입점입니다.
		/// </summary>
		[STAThread]
		static void Main()
		{
			// get version 
			string strCfgFile = string.Format("{0}\\Installer.cfg", Application.StartupPath);
			string strVersion = "";
			StringBuilder sb = new StringBuilder(32);
			int len = Win32API.GetPrivateProfileString("INSTALL", "Version", "", sb, 32, strCfgFile);
			if (len <= 0 || sb.ToString() == "")
				strVersion = "Unknown";
			else
				strVersion = sb.ToString();

			// log init..
			FLog.Init(Application.StartupPath, "installer");
			FLog.Msg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
			FLog.Msg(string.Format(">> Btfax4 package(v{0}) installer start !!", strVersion));
			FLog.Msg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

			MainForm mainForm = new MainForm();
			mainForm.PackageVersion = strVersion;
			
			Application.Run(mainForm);

			FLog.Msg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
			FLog.Msg(string.Format(">> Btfax4 package(v{0}) installer Exit !!", strVersion));
			FLog.Msg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		}
	}
}
