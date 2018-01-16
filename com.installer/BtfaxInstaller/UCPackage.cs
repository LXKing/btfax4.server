using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace BtfaxInstaller
{
	public partial class UCPackage : UCInstallBase
	{
		public UCPackage()
		{
			base.UCType = UCType.Package;
			base.SetTitle("4. 패키지 (Process, Web source, Tools, FaxStorage)");

			base.VisibleSelector = true;
			base.SelectorText = "Install path ";
			base.SelectorPath = "D:\\btfax4\\";

			FLog.Msg(string.Format("{0}", base.GetTitle()));
			this.LoadInstallItems();
		}

		protected override string SelectorPath
		{
			get
			{
				return base.SelectorPath;
			}
			set
			{
				base.SelectorPath = value;
				PACKAGE_HOME_PATH = value;
			}
		}

		/// <summary>
		/// 설치할 아이템 로드
		/// </summary>
		protected override bool LoadInstallItems()
		{
			base.ClearItems();

			string strMsg;
			string strItemPath = string.Format("{0}\\04.package\\", Application.StartupPath);
			if (!Directory.Exists(strItemPath))
			{
				strMsg = string.Format("패키지 디렉터리가 존재하지 않습니다. \n{0}", strItemPath);
				MsgBox.Err(strMsg);
				FLog.Err(strMsg);
				return false;
			}

			string[] arrFiles = Directory.GetFiles(strItemPath, "*.*", SearchOption.AllDirectories);
			if (arrFiles.Length <= 0)
			{
				strMsg = string.Format("패키지 아이템이 존재하지 않습니다. \n{0}", strItemPath);
				MsgBox.Err(strMsg);
				FLog.Err(strMsg);
				return false;
			}

			FLog.Msg("설치항목 로드 시작");
			foreach (string strFile in arrFiles)
			{
				FileInfo fInfo = new FileInfo(strFile);
				FLog.Msg(fInfo.FullName);
				base.AddItem(fInfo.Name, fInfo.Directory.FullName, InstallState.Ready);
			}
			FLog.Msg("설치항목 로드 완료");

			return base.LoadInstallItems();
		}

		protected override void button_find_Click(object sender, EventArgs e)
		{
			using (FolderBrowserDialog dlg = new FolderBrowserDialog())
			{
				dlg.Description = "Select package installation path";
				DialogResult ret = dlg.ShowDialog();
				if (ret != DialogResult.OK)
					return;

				this.SelectorPath = dlg.SelectedPath;
			}

		}

		protected override void ProgressInstall(object sender, DoWorkEventArgs e)
		{
			PACKAGE_HOME_PATH = base.SelectorPath;

			base.MenuBar.DisableAllButton();
			string strItemPath = string.Format("{0}\\04.package\\", Application.StartupPath);

			string strDestPath = "";

			// 디렉터리 생성
			FLog.Msg("디렉터리 생성 시작");
			string[] dirs = Directory.GetDirectories(strItemPath, "*.*", SearchOption.AllDirectories);
			foreach (string strDir in dirs)
			{
				try
				{
					System.Threading.Thread.Sleep(10);
					strDestPath = string.Format("{0}\\{1}", PACKAGE_HOME_PATH, strDir.Replace(strItemPath, ""));
					if (!Directory.Exists(strDestPath))
						Directory.CreateDirectory(strDestPath);
					FLog.Msg(strDestPath);
				}
				catch
				{
					continue;
				}
			}
			FLog.Msg("디렉터리 생성 완료");
			
			// 파일복사
			FLog.Msg("파일복사 시작");
			List<InstallItemInfo> items = base.GetInstallItems();
			foreach (InstallItemInfo item in items)
			{
				base.UpdateItem(item.nItemIdx, item.strName, item.strPath, InstallState.Processing, item.strMsg);
				System.Threading.Thread.Sleep(5);
				
				base.ProgressBar.IncreaseVal(1);

				try
				{
					string fileFullName = string.Format("{0}\\{1}", item.strPath, item.strName);
					if (!File.Exists(fileFullName))
					{
						FLog.Wrn(string.Format("파일이 존재하지 않습니다. ({0})", fileFullName));
						continue;
					}

					FileInfo fInfo = new FileInfo(fileFullName);
					strDestPath = string.Format("{0}\\{1}", PACKAGE_HOME_PATH, fInfo.Directory.FullName.Replace(strItemPath, ""));
					if (!Directory.Exists(strDestPath))
						Directory.CreateDirectory(strDestPath);

					string destFileFullName = string.Format("{0}\\{1}", strDestPath, fInfo.Name);
					File.Copy(fInfo.FullName, destFileFullName, true);
					FLog.Msg(string.Format("{0} -> {1}", fInfo.FullName, destFileFullName));

					base.UpdateItem(item.nItemIdx, item.strName, item.strPath, InstallState.Success, item.strMsg);
				}
				catch(Exception ex)
				{
					FLog.Err(ex.Message);
					base.UpdateItem(item.nItemIdx, item.strName, item.strPath, InstallState.Failure, ex.Message);
				}
			}

			FLog.Msg("파일복사 완료");
			base.ProgressInstall(sender, e);
		}

		protected override void CompleteInstall(object sender, RunWorkerCompletedEventArgs e)
		{
			string strMsg;
			try
			{
				// watcher cfg 파일 조절
				FLog.Msg("Watcher.cfg 프로세스 위치 조정");
				string strIoaCfgPath = string.Format("{0}\\server\\bin_ioa\\cfg\\watcher.cfg", PACKAGE_HOME_PATH);
				if (!File.Exists(strIoaCfgPath))
				{
					strMsg = string.Format("프로세스 환경설정 파일({0})이 존재하지 않습니다", strIoaCfgPath);
					MsgBox.Err(strMsg);
					FLog.Err(strMsg);

					base.MenuBar.EnableAllButton();
					base.CompleteInstall(sender, e);
					return;
				}

				StringBuilder sb = new StringBuilder(256);
				int len = Win32API.GetPrivateProfileString("Watch", "list", "", sb, 256, strIoaCfgPath);
				if (len <= 0 || sb.ToString() == "")
				{
					strMsg = "프로세스 목록 데이터가 존재하지 않습니다";
					FLog.Err(strMsg);

					base.MenuBar.EnableAllButton();
					base.CompleteInstall(sender, e);
					return;
				}

				string strSections = sb.ToString().Trim();
				string[] arrProcsInfo = strSections.Split(',');
				if (arrProcsInfo.Length <= 0)
				{
					strMsg = "프로세스 목록 데이터가 존재하지 않습니다 (Split)";
					MsgBox.Err(strMsg);
					FLog.Err(strMsg);

					base.MenuBar.EnableAllButton();
					base.CompleteInstall(sender, e);
					return;
				}

				strMsg = string.Format("LIST={0}", strSections);
				FLog.Msg(strMsg);

				string strReplace = string.Format("{0}\\server", PACKAGE_HOME_PATH);
				DirectoryInfo dirInfo = new DirectoryInfo(strReplace);

				foreach (string strProcSection in arrProcsInfo)
				{	
					// Path 정보 변경
					len = Win32API.GetPrivateProfileString(strProcSection, "Path", "", sb, 256, strIoaCfgPath);
					if (len <= 0 || sb.ToString() == "")
						continue;

					string strValue = sb.ToString();
					strValue = strValue.Replace("[INSTALL_PATH]", dirInfo.FullName);
					len = Win32API.WritePrivateProfileString(strProcSection, "Path", strValue, strIoaCfgPath);
					FLog.Msg(string.Format("section:{0}, filed:Path, value:{1}", strProcSection, strValue));


					// Pattern 정보 변경
					len = Win32API.GetPrivateProfileString(strProcSection, "Pattern", "", sb, 256, strIoaCfgPath);
					if (len <= 0 || sb.ToString() == "")
						continue;

					strValue = sb.ToString();
					strValue = strValue.Replace("[INSTALL_PATH]", dirInfo.FullName);
					len = Win32API.WritePrivateProfileString(strProcSection, "Pattern", strValue, strIoaCfgPath);
					FLog.Msg(string.Format("section:{0}, filed:Pattern, value:{1}", strProcSection, strValue));
					
					System.Threading.Thread.Sleep(5);
				}

				strMsg = string.Format("패키지 설치를 완료 하였습니다. \n({0})", PACKAGE_HOME_PATH);
				MsgBox.Info(strMsg);
				FLog.Msg(strMsg);
			}
			catch(Exception ex)
			{
				MsgBox.Info(ex.Message);
			}

			base.MenuBar.EnableAllButton();
			base.CompleteInstall(sender, e);
		}

		protected string PACKAGE_HOME_PATH = "D:\\btfax4\\";
	}
}
