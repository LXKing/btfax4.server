namespace BtfaxInstaller
{
	partial class MainForm
	{
		/// <summary>
		/// 필수 디자이너 변수입니다.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// 사용 중인 모든 리소스를 정리합니다.
		/// </summary>
		/// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form 디자이너에서 생성한 코드

		/// <summary>
		/// 디자이너 지원에 필요한 메서드입니다.
		/// 이 메서드의 내용을 코드 편집기로 수정하지 마십시오.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
			this.panel_main = new System.Windows.Forms.Panel();
			this.label_title = new System.Windows.Forms.Label();
			this.ucMenus1 = new BtfaxInstaller.UCMenus();
			this.SuspendLayout();
			// 
			// panel_main
			// 
			this.panel_main.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel_main.Location = new System.Drawing.Point(0, 36);
			this.panel_main.Name = "panel_main";
			this.panel_main.Size = new System.Drawing.Size(584, 339);
			this.panel_main.TabIndex = 1;
			// 
			// label_title
			// 
			this.label_title.Dock = System.Windows.Forms.DockStyle.Top;
			this.label_title.Font = new System.Drawing.Font("맑은 고딕", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label_title.Location = new System.Drawing.Point(0, 0);
			this.label_title.Name = "label_title";
			this.label_title.Size = new System.Drawing.Size(584, 36);
			this.label_title.TabIndex = 2;
			this.label_title.Text = "BTFAX4 package installation";
			this.label_title.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// ucMenus1
			// 
			this.ucMenus1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.ucMenus1.Location = new System.Drawing.Point(0, 375);
			this.ucMenus1.Margin = new System.Windows.Forms.Padding(10);
			this.ucMenus1.Name = "ucMenus1";
			this.ucMenus1.Padding = new System.Windows.Forms.Padding(5);
			this.ucMenus1.Size = new System.Drawing.Size(584, 37);
			this.ucMenus1.TabIndex = 0;
			this.ucMenus1.OnInstallButtonClickEvent += new BtfaxInstaller.UCMenus.OnInstallButtonClickDelegate(this.ucMenus1_OnInstallButtonClickEvent);
			this.ucMenus1.OnPrevButtonClickEvent += new BtfaxInstaller.UCMenus.OnPrevButtonClickDelegate(this.ucMenus1_OnPrevButtonClickEvent);
			this.ucMenus1.OnNextButtonClickEvent += new BtfaxInstaller.UCMenus.OnNextButtonClickDelegate(this.ucMenus1_OnNextButtonClickEvent);
			this.ucMenus1.OnFinishButtonClickEvent += new BtfaxInstaller.UCMenus.OnFinishButtonClickDelegate(this.ucMenus1_OnFinishButtonClickEvent);
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(584, 412);
			this.Controls.Add(this.panel_main);
			this.Controls.Add(this.label_title);
			this.Controls.Add(this.ucMenus1);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MinimumSize = new System.Drawing.Size(600, 450);
			this.Name = "MainForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			this.Text = "Installer";
			this.ResumeLayout(false);

		}

		#endregion

		private UCMenus ucMenus1;
		private System.Windows.Forms.Panel panel_main;
		private System.Windows.Forms.Label label_title;

	}
}

