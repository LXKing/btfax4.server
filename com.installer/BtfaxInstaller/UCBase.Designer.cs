namespace BtfaxInstaller
{
	partial class UCBase
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
			this.label_title = new System.Windows.Forms.Label();
			this.panel1 = new System.Windows.Forms.Panel();
			this.label_desc = new System.Windows.Forms.Label();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// label_title
			// 
			this.label_title.Dock = System.Windows.Forms.DockStyle.Fill;
			this.label_title.Font = new System.Drawing.Font("맑은 고딕", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label_title.Location = new System.Drawing.Point(0, 0);
			this.label_title.Name = "label_title";
			this.label_title.Size = new System.Drawing.Size(165, 28);
			this.label_title.TabIndex = 0;
			this.label_title.Text = "title";
			this.label_title.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.label_title);
			this.panel1.Controls.Add(this.label_desc);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(403, 28);
			this.panel1.TabIndex = 1;
			// 
			// label_desc
			// 
			this.label_desc.Dock = System.Windows.Forms.DockStyle.Right;
			this.label_desc.Font = new System.Drawing.Font("맑은 고딕", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label_desc.Location = new System.Drawing.Point(165, 0);
			this.label_desc.Name = "label_desc";
			this.label_desc.Size = new System.Drawing.Size(238, 28);
			this.label_desc.TabIndex = 1;
			this.label_desc.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			// 
			// UCBase
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.White;
			this.Controls.Add(this.panel1);
			this.Name = "UCBase";
			this.Size = new System.Drawing.Size(403, 206);
			this.panel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Label label_title;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Label label_desc;
	}
}
