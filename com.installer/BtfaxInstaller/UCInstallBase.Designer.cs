namespace BtfaxInstaller
{
	partial class UCInstallBase
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

		#region 구성 요소 디자이너에서 생성한 코드

		/// <summary> 
		/// 디자이너 지원에 필요한 메서드입니다. 
		/// 이 메서드의 내용을 코드 편집기로 수정하지 마십시오.
		/// </summary>
		private void InitializeComponent()
		{
			this.listView1 = new System.Windows.Forms.ListView();
			this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.ucProgressBar1 = new BtfaxInstaller.UCProgressBar();
			this.panel_selector = new System.Windows.Forms.Panel();
			this.textBox_path = new System.Windows.Forms.TextBox();
			this.button_find = new System.Windows.Forms.Button();
			this.label_selector = new System.Windows.Forms.Label();
			this.panel_selector.SuspendLayout();
			this.SuspendLayout();
			// 
			// listView1
			// 
			this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4});
			this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.listView1.Font = new System.Drawing.Font("맑은 고딕", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.listView1.FullRowSelect = true;
			this.listView1.GridLines = true;
			this.listView1.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
			this.listView1.Location = new System.Drawing.Point(0, 58);
			this.listView1.Name = "listView1";
			this.listView1.Size = new System.Drawing.Size(414, 217);
			this.listView1.Sorting = System.Windows.Forms.SortOrder.Ascending;
			this.listView1.TabIndex = 1;
			this.listView1.UseCompatibleStateImageBehavior = false;
			this.listView1.View = System.Windows.Forms.View.Details;
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Name";
			this.columnHeader1.Width = 144;
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "Path";
			this.columnHeader2.Width = 236;
			// 
			// columnHeader3
			// 
			this.columnHeader3.Text = "State";
			this.columnHeader3.Width = 94;
			// 
			// columnHeader4
			// 
			this.columnHeader4.Text = "Message";
			// 
			// ucProgressBar1
			// 
			this.ucProgressBar1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.ucProgressBar1.Location = new System.Drawing.Point(0, 275);
			this.ucProgressBar1.Name = "ucProgressBar1";
			this.ucProgressBar1.Size = new System.Drawing.Size(414, 11);
			this.ucProgressBar1.TabIndex = 2;
			// 
			// panel_selector
			// 
			this.panel_selector.Controls.Add(this.textBox_path);
			this.panel_selector.Controls.Add(this.button_find);
			this.panel_selector.Controls.Add(this.label_selector);
			this.panel_selector.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel_selector.Location = new System.Drawing.Point(0, 30);
			this.panel_selector.Name = "panel_selector";
			this.panel_selector.Padding = new System.Windows.Forms.Padding(3);
			this.panel_selector.Size = new System.Drawing.Size(414, 28);
			this.panel_selector.TabIndex = 3;
			// 
			// textBox_path
			// 
			this.textBox_path.Dock = System.Windows.Forms.DockStyle.Fill;
			this.textBox_path.Font = new System.Drawing.Font("맑은 고딕", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.textBox_path.Location = new System.Drawing.Point(45, 3);
			this.textBox_path.Name = "textBox_path";
			this.textBox_path.Size = new System.Drawing.Size(315, 22);
			this.textBox_path.TabIndex = 5;
			// 
			// button_find
			// 
			this.button_find.Dock = System.Windows.Forms.DockStyle.Right;
			this.button_find.Font = new System.Drawing.Font("맑은 고딕", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.button_find.Location = new System.Drawing.Point(360, 3);
			this.button_find.Name = "button_find";
			this.button_find.Size = new System.Drawing.Size(51, 22);
			this.button_find.TabIndex = 6;
			this.button_find.Text = "Find";
			this.button_find.UseVisualStyleBackColor = true;
			this.button_find.Click += new System.EventHandler(this.button_find_Click);
			// 
			// label_selector
			// 
			this.label_selector.AutoSize = true;
			this.label_selector.Dock = System.Windows.Forms.DockStyle.Left;
			this.label_selector.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label_selector.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
			this.label_selector.Location = new System.Drawing.Point(3, 3);
			this.label_selector.Name = "label_selector";
			this.label_selector.Size = new System.Drawing.Size(42, 15);
			this.label_selector.TabIndex = 4;
			this.label_selector.Text = "(none)";
			this.label_selector.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// UCInstallBase
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.listView1);
			this.Controls.Add(this.panel_selector);
			this.Controls.Add(this.ucProgressBar1);
			this.Name = "UCInstallBase";
			this.Size = new System.Drawing.Size(414, 286);
			this.Controls.SetChildIndex(this.ucProgressBar1, 0);
			this.Controls.SetChildIndex(this.panel_selector, 0);
			this.Controls.SetChildIndex(this.listView1, 0);
			this.panel_selector.ResumeLayout(false);
			this.panel_selector.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.ListView listView1;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private UCProgressBar ucProgressBar1;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.Panel panel_selector;
		private System.Windows.Forms.TextBox textBox_path;
		private System.Windows.Forms.Button button_find;
		private System.Windows.Forms.Label label_selector;


	}
}
