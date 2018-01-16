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
	public partial class UCBthmp : UCInstallBase
	{
		public UCBthmp()
		{
			InitializeComponent();

			base.UCType = UCType.Bthmp;
			base.SetTitle("2. BTHMP");
			base.SetDescription("설치위치 : C:\\BTFAX\\");

			base.VisibleSelector = true;
			base.SelectorText = "license file (*.lic)";

			FLog.Msg(string.Format("{0}", base.GetTitle()));
			this.LoadInstallItems();
		}

		/// <summary>
		/// 설치할 아이템 로드
		/// </summary>
		protected override bool LoadInstallItems()
		{
			base.ClearItems();

			string strMsg;
			string strItemPath = string.Format("{0}\\02.bthmp\\", Application.StartupPath);
			if (!Directory.Exists(strItemPath))
			{
				strMsg = string.Format("BTHMP 디렉터리가 존재하지 않습니다. \n{0}", strItemPath);
				MsgBox.Err(strMsg);
				FLog.Err(strMsg);
				return false;
			}

			string[] arrFiles = Directory.GetFiles(strItemPath, "*.*", SearchOption.AllDirectories);
			if (arrFiles.Length <= 0)
			{
				strMsg = string.Format("BTHMP 아이템이 존재하지 않습니다. \n{0}", strItemPath);
				MsgBox.Err(strMsg);
				return false;
			}

			FLog.Msg("설치항목 로드 시작");
			foreach (string strFile in arrFiles)
			{
				FLog.Msg(strFile);
				
				FileInfo fInfo = new FileInfo(strFile);
				base.AddItem(fInfo.Name, fInfo.Directory.FullName, InstallState.Ready);
			}
			FLog.Msg("설치항목 로드 완료");

			return base.LoadInstallItems();
		}

		protected override void button_find_Click(object sender, EventArgs e)
		{
			using (OpenFileDialog dlg = new OpenFileDialog())
			{
				dlg.RestoreDirectory = true;
				dlg.Filter = "btmp license file (*.lic)|*.lic";
				dlg.Multiselect = false;
				DialogResult ret = dlg.ShowDialog();
				if (ret != DialogResult.OK)
					return;

				FileInfo fInfo = new FileInfo(dlg.FileName);
				if (fInfo.Extension.ToLower() != ".lic")
				{
					MsgBox.Err(string.Format("BTMP라이센스 파일이 아닙니다. \n({0})", fInfo.FullName));
					return;
				}

				base.SelectorPath = dlg.FileName;
			}

		}

		protected override void ProgressInstall(object sender, DoWorkEventArgs e)
		{
			base.MenuBar.DisableAllButton();
			string strItemPath = string.Format("{0}\\02.bthmp\\", Application.StartupPath);

			string strDestPath = "";

			// 디렉터리 생성
			FLog.Msg("디렉터리 생성 시작");
			string[] dirs = Directory.GetDirectories(strItemPath, "*.*", SearchOption.AllDirectories);
			foreach (string strDir in dirs)
			{
				try
				{
					System.Threading.Thread.Sleep(10);
					strDestPath = string.Format("{0}\\{1}", BTMP_HOME_PATH, strDir.Replace(strItemPath, ""));
					if (!Directory.Exists(strDestPath))
						Directory.CreateDirectory(strDestPath);
					FLog.Msg(strDestPath);
				}
				catch(Exception ex)
				{
					FLog.Err(ex.Message);
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
					strDestPath = string.Format("{0}\\{1}", BTMP_HOME_PATH, fInfo.Directory.FullName.Replace(strItemPath, ""));
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
			string licFilePath = string.Format("{0}\\cfg\\", BTMP_HOME_PATH);
			string strMsg;
			try
			{
				if (string.IsNullOrEmpty(base.SelectorPath) || !File.Exists(base.SelectorPath))
				{
					strMsg = string.Format("BTMP 라이센스 파일(btfax.lic)을 선택하지 않았습니다.\n 추후 라이센스파일을 해당위치 ({0})에 복사하여 주십시요.", licFilePath);
					MsgBox.Info(strMsg);
					FLog.Wrn(strMsg);
					base.MenuBar.EnableAllButton();
					base.CompleteInstall(sender, e);

				}
				else
				{
					string strDestFileFullName = string.Format("{0}\\btfax.lic", licFilePath);
					File.Copy(base.SelectorPath, strDestFileFullName, true);
					FLog.Msg(string.Format("라이센스 복사 완료 ({0}) -> ({1})", base.SelectorPath, strDestFileFullName));
				}
			}
			catch (Exception ex)
			{
				MsgBox.Info(string.Format("BTMP 라이센스 파일복사중 오류가 발생하였습니다.\n ({0}).\n 추후 라이센스파일을 해당위치 ({1})에 복사하여 주십시요.", ex.Message, licFilePath));
				FLog.Msg(ex.Message);
			}

			// btmp_pm.exe install
			string strRedirectMsg = "";
			string strBtfaxPmPath = string.Format("{0}\\bin\\", BTMP_HOME_PATH);
			if (!ProcessHandler.StartProcess(strBtfaxPmPath, "btfax_pm.exe", out strRedirectMsg, " install"))
			{
				strMsg = string.Format("btfax_pm.exe install 실행을 실패하였습니다.\n {0}", ProcessHandler.GetErrMsg());
				MsgBox.Info(strMsg);
				FLog.Err(strMsg);
			}

			if (!string.IsNullOrEmpty(strRedirectMsg))
			{
				strMsg = strRedirectMsg;
				MsgBox.Info(string.Format("{0}", strRedirectMsg));
				FLog.Msg(strMsg);
			}

			base.MenuBar.EnableAllButton();
			base.CompleteInstall(sender, e);
		}

		protected const string BTMP_HOME_PATH = "C:\\BTFAX\\";
	}
}
