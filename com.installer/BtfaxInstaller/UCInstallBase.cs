using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace BtfaxInstaller
{
	public partial class UCInstallBase : UCBase
	{
		protected enum InstallState
		{
			None = 0,
			Ready = 1,
			Processing = 2,
			Success = 3,
			Failure = 4,
		}

		protected struct InstallItemInfo
		{
			public int nItemIdx;
			public string strName;
			public string strPath;
			public string strState;
			public string strMsg;
		}

		/// <summary>
		/// 생성자
		/// </summary>
		public UCInstallBase()
		{
			InitializeComponent();
			this.IsInstalling = false;
			this.ucProgressBar1.Visible = false;
		}

		/// <summary>
		/// 프로그래스바
		/// </summary>
		protected UCProgressBar ProgressBar
		{
			get { return this.ucProgressBar1; }
		}

		/// <summary>
		/// 파일또는 디렉터리 선택 패널
		/// </summary>
		protected bool VisibleSelector
		{
			get { return this.panel_selector.Visible; }
			set { this.panel_selector.Visible = value; }
		}

		protected string SelectorText
		{
			set { this.label_selector.Text = value; }
		}

		protected virtual string SelectorPath
		{
			set { this.textBox_path.Text = value; }
			get { return this.textBox_path.Text; }
		}

		public bool IsInstalling { get; set; }

		/// <summary>
		/// 설치할 아이템 로드
		/// </summary>
		protected virtual bool LoadInstallItems() 
		{
			this.ProgressBar.Init(0, this.listView1.Items.Count);
			return true; 
		}

		/// <summary>
		/// 아이템 설치
		/// </summary>
		public virtual bool Install()
		{	
			if (this.MenuBar == null)
			{
				MsgBox.Err("MenuBar object is null");
				return false;
			}

			this.ProgressBar.Visible = true;
			this.ProgressBar.Init(0, this.listView1.Items.Count);

			IsInstalling = true;

			BackgroundWorker worker = new BackgroundWorker();
			worker.DoWork += new DoWorkEventHandler(ProgressInstall);
			worker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(CompleteInstall);
			worker.RunWorkerAsync();
			return true;
		}

		protected virtual void CompleteInstall(object sender, RunWorkerCompletedEventArgs e) 
		{
			IsInstalling = false;
		}
		protected virtual void ProgressInstall(object sender, DoWorkEventArgs e) { }
		

		/// <summary>
		/// 리스트뷰 아이템추가
		/// </summary>
		protected void AddItem(string p_strName, string p_strPath, InstallState p_state)
		{
			ListViewItem item = new ListViewItem();
			item.Text = p_strName;
			item.SubItems.Add(p_strPath);
			item.SubItems.Add(p_state.ToString());
			item.SubItems.Add("");

			this.AddItem(item);
		}

		/// <summary>
		/// 리스트뷰 아이템추가
		/// </summary>
		protected void AddItem(ListViewItem p_item)
		{
			this.listView1.Items.Add(p_item);
		}

		/// <summary>
		/// 리스트뷰 아이템 클리어
		/// </summary>
		protected void ClearItems()
		{
			if (this.listView1.Items.Count > 0)
				this.listView1.Items.Clear();
		}

		protected void UpdateItem(int p_nIndex, string p_strName, string p_strPath, InstallState p_state, string p_strMsg)
		{
			if (this.InvokeRequired)
			{
				UpdateItemDelegate d = new UpdateItemDelegate(UpdateItem);
				this.Invoke(d, new object[] { p_nIndex, p_strName, p_strPath, p_state, p_strMsg });
				return;
			}
			else
			{
				ListViewItem item = this.listView1.Items[p_nIndex];
				item.Text = p_strName;
				item.SubItems[1].Text = p_strPath;
				item.SubItems[2].Text = p_state.ToString();
				item.SubItems[3].Text = p_strMsg;

				this.listView1.EnsureVisible(p_nIndex);
			}
		}

		protected List<InstallItemInfo> GetInstallItems()
		{	
			if (this.InvokeRequired)
			{
				GetInstallItemsDelegate d = new GetInstallItemsDelegate(GetInstallItems);
				List<InstallItemInfo> items = this.Invoke(d) as List<InstallItemInfo>;
				return items;
			}
			else
			{
				List<InstallItemInfo> lst = new List<InstallItemInfo>();
				foreach (ListViewItem item in this.listView1.Items)
				{
					InstallItemInfo info = new InstallItemInfo();
					info.nItemIdx = item.Index;
					info.strName = item.Text.ToString();
					info.strPath = item.SubItems[1].Text.ToString();
					info.strState = item.SubItems[2].Text.ToString();
					lst.Add(info);
				}
				return lst;
			}
		}

		private delegate void UpdateItemDelegate(int p_nIndex, string p_strName, string p_strPath, InstallState p_state, string p_strMsg);
		private delegate List<InstallItemInfo> GetInstallItemsDelegate();


		protected virtual void button_find_Click(object sender, EventArgs e) { }
	}
}
