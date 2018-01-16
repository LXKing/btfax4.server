namespace BtfaxInstaller
{
	partial class UCMenus
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
			this.button_install = new System.Windows.Forms.Button();
			this.button_prev = new System.Windows.Forms.Button();
			this.button_next = new System.Windows.Forms.Button();
			this.button_finish = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// button_install
			// 
			this.button_install.Dock = System.Windows.Forms.DockStyle.Right;
			this.button_install.Location = new System.Drawing.Point(20, 5);
			this.button_install.Name = "button_install";
			this.button_install.Size = new System.Drawing.Size(75, 20);
			this.button_install.TabIndex = 5;
			this.button_install.Text = "▲ Install";
			this.button_install.UseVisualStyleBackColor = true;
			this.button_install.Click += new System.EventHandler(this.button_install_Click);
			// 
			// button_prev
			// 
			this.button_prev.Dock = System.Windows.Forms.DockStyle.Right;
			this.button_prev.Location = new System.Drawing.Point(95, 5);
			this.button_prev.Name = "button_prev";
			this.button_prev.Size = new System.Drawing.Size(75, 20);
			this.button_prev.TabIndex = 4;
			this.button_prev.Text = "<< Prev";
			this.button_prev.UseVisualStyleBackColor = true;
			this.button_prev.Click += new System.EventHandler(this.button_prev_Click);
			// 
			// button_next
			// 
			this.button_next.Dock = System.Windows.Forms.DockStyle.Right;
			this.button_next.Location = new System.Drawing.Point(170, 5);
			this.button_next.Name = "button_next";
			this.button_next.Size = new System.Drawing.Size(75, 20);
			this.button_next.TabIndex = 3;
			this.button_next.Text = "Next >>";
			this.button_next.UseVisualStyleBackColor = true;
			this.button_next.Click += new System.EventHandler(this.button_next_Click);
			// 
			// button_finish
			// 
			this.button_finish.Dock = System.Windows.Forms.DockStyle.Right;
			this.button_finish.Location = new System.Drawing.Point(245, 5);
			this.button_finish.Name = "button_finish";
			this.button_finish.Size = new System.Drawing.Size(75, 20);
			this.button_finish.TabIndex = 6;
			this.button_finish.Text = "▼ Finish";
			this.button_finish.UseVisualStyleBackColor = true;
			this.button_finish.Click += new System.EventHandler(this.button_finish_Click);
			// 
			// UCMenus
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.button_install);
			this.Controls.Add(this.button_prev);
			this.Controls.Add(this.button_next);
			this.Controls.Add(this.button_finish);
			this.Margin = new System.Windows.Forms.Padding(10);
			this.Name = "UCMenus";
			this.Padding = new System.Windows.Forms.Padding(5);
			this.Size = new System.Drawing.Size(325, 30);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Button button_install;
		private System.Windows.Forms.Button button_prev;
		private System.Windows.Forms.Button button_next;
		private System.Windows.Forms.Button button_finish;
	}
}
