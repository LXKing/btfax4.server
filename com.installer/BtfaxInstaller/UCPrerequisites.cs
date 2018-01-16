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
	public partial class UCPrerequisites : UCInstallBase
	{
		public UCPrerequisites()
		{
			InitializeComponent();

			base.UCType = UCType.Prerequisites;
			base.SetTitle("1. 서버환경 필수 설치 항목");
			base.VisibleSelector = false;

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
			string strItemPath = string.Format("{0}\\01.Prerequisites\\", Application.StartupPath);
			if (!Directory.Exists(strItemPath))
			{
				strMsg = string.Format("필수 설치 항목 디렉터리가 존재하지 않습니다. \n{0}", strItemPath);
				MsgBox.Err(strMsg);
				FLog.Err(strMsg);
				return false;
			}
			
			string[] arrFiles = Directory.GetFiles(strItemPath);
			if (arrFiles.Length <= 0)
			{
				strMsg = string.Format("필수 설치 항목 아이템이 존재하지 않습니다. \n{0}", strItemPath);
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

		protected override void ProgressInstall(object sender, DoWorkEventArgs e)
		{
			base.MenuBar.DisableAllButton();
			List<InstallItemInfo> items = base.GetInstallItems();
			foreach (InstallItemInfo item in items)
			{
				base.UpdateItem(item.nItemIdx, item.strName, item.strPath, InstallState.Processing, item.strMsg);
				System.Threading.Thread.Sleep(1000);
				base.ProgressBar.IncreaseVal(1);

				if (!ProcessHandler.StartProcess(item.strPath, item.strName))
				{
					base.UpdateItem(item.nItemIdx, item.strName, item.strPath, InstallState.Failure, ProcessHandler.GetErrMsg());
					FLog.Err(ProcessHandler.GetErrMsg());
				}
				else
				{
					base.UpdateItem(item.nItemIdx, item.strName, item.strPath, InstallState.Success, item.strMsg);
					FLog.Msg(string.Format("프로세스 실행 완료 ({0})", item.strName ));
				}
			}

			base.ProgressInstall(sender, e);
		}

		protected override void CompleteInstall(object sender, RunWorkerCompletedEventArgs e)
		{
			base.MenuBar.EnableAllButton();
			base.CompleteInstall(sender, e);
		}
	}
}
