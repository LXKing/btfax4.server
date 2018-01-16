using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace BtfaxInstaller
{
	public partial class MainForm : Form
	{
		/// <summary>
		/// constructor
		/// </summary>
		public MainForm()
		{
			InitializeComponent();
			this.SetIntro();
		}

		/// <summary>
		/// Package version
		/// </summary>
		public string PackageVersion 
		{
			get { return m_strPackageVersion; }
			set 
			{ 
				m_strPackageVersion = value;
				this.label_title.Text += string.Format(" (v{0})", value); 
			}
		}

		protected void AddUserControl(UCBase p_userControl)
		{
			this.ClearUserControl();
			p_userControl.Dock = DockStyle.Fill;
			this.panel_main.Controls.Add(p_userControl);
		}

		protected void ClearUserControl()
		{
			if (this.panel_main.Controls.Count <= 0)
				return;

			this.panel_main.Controls.Clear();
		}

		protected void SetIntro()
		{
			UCIntro uc = new UCIntro();
			uc.MenuBar = this.ucMenus1;
			this.AddUserControl(uc);
			this.ucMenus1.DisableButton(UCMenus.ButtonType.Install);
			this.ucMenus1.DisableButton(UCMenus.ButtonType.Prev);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Next);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Finish);
			

			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Install, false);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Prev, false);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Next, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Finish, false);
		}

		protected void SetPrerequisites()
		{
			UCPrerequisites uc = new UCPrerequisites();
			uc.MenuBar = this.ucMenus1;
			this.AddUserControl(uc);

			this.ucMenus1.EnableButton(UCMenus.ButtonType.Install);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Prev);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Next);
			this.ucMenus1.DisableButton(UCMenus.ButtonType.Finish);

			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Install, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Prev, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Next, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Finish, false);
		}

		protected void SetBthmp()
		{
			UCBthmp uc = new UCBthmp();
			uc.MenuBar = this.ucMenus1;
			this.AddUserControl(uc);

			this.ucMenus1.EnableButton(UCMenus.ButtonType.Install);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Prev);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Next);
			this.ucMenus1.DisableButton(UCMenus.ButtonType.Finish);

			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Install, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Prev, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Next, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Finish, false);
		}

		protected void SetFaxDriver()
		{
			UCFaxDriver uc = new UCFaxDriver();
			uc.MenuBar = this.ucMenus1;
			this.AddUserControl(uc);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Install);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Prev);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Next);
			this.ucMenus1.DisableButton(UCMenus.ButtonType.Finish);

			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Install, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Prev, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Next, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Finish, false);
		}

		protected void SetPackage()
		{
			UCPackage uc = new UCPackage();
			uc.MenuBar = this.ucMenus1;
			this.AddUserControl(uc);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Install);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Prev);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Next);
			this.ucMenus1.DisableButton(UCMenus.ButtonType.Finish);

			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Install, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Prev, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Next, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Finish, false);
		}

		protected void SetFinish()
		{
			UCFinish uc = new UCFinish();
			uc.MenuBar = this.ucMenus1;
			this.AddUserControl(uc);
			this.ucMenus1.DisableButton(UCMenus.ButtonType.Install);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Prev);
			this.ucMenus1.DisableButton(UCMenus.ButtonType.Next);
			this.ucMenus1.EnableButton(UCMenus.ButtonType.Finish);

			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Install, false);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Prev, true);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Next, false);
			this.ucMenus1.VisibleButton(UCMenus.ButtonType.Finish, true);
		}

		private void ucMenus1_OnInstallButtonClickEvent()
		{
			UCInstallBase uc = this.panel_main.Controls[0] as UCInstallBase;
			if (uc == null)
				return;

			uc.Install();
		}

		private void ucMenus1_OnNextButtonClickEvent()
		{
			UCBase uc = this.panel_main.Controls[0] as UCBase;
			if (uc == null)
				return;

			switch (uc.UCType)
			{
				case UCType.Intro:
					this.SetPrerequisites();
					break;

				case UCType.Prerequisites:
					this.SetBthmp();
					break;

				case UCType.Bthmp :
					this.SetFaxDriver();
					break;

				case UCType.FaxDriver:
					this.SetPackage();
					break;

				case UCType.Package:
					this.SetFinish();
					break;

				default:
					return;
			}
		}

		private void ucMenus1_OnPrevButtonClickEvent()
		{
			UCBase uc = this.panel_main.Controls[0] as UCBase;
			if (uc == null)
				return;

			switch (uc.UCType)
			{
				case UCType.Prerequisites:
					this.SetIntro();
					break;

				case UCType.Bthmp:
					this.SetPrerequisites();
					break;

				case UCType.FaxDriver:
					this.SetBthmp();
					break;

				case UCType.Package:
					this.SetFaxDriver();
					break;

				case UCType.Finish:
					this.SetPackage();
					break;

				default:
					return;
			}
		}

		private void ucMenus1_OnFinishButtonClickEvent()
		{
			base.Close();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			UCInstallBase uc = this.panel_main.Controls[0] as UCInstallBase;
			if (uc == null)
			{
				base.OnClosing(e);
				return;
			}

			if (uc.IsInstalling)
			{
				MsgBox.Info(string.Format("[{0}]를 설치중입니다. 설치가 완료되면 시도해 주십시요.", uc.UCType.ToString()));
				e.Cancel = true;
			}

			base.OnClosing(e);
		}


	
		// fields..
		private string m_strPackageVersion = "";

	
	}
}
