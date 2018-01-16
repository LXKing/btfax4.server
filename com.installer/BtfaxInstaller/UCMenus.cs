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
	public partial class UCMenus : UserControl
	{
		public enum ButtonType
		{
			Install = 0,
			Prev = 1,
			Next = 2,
			Finish = 3,
		}

		public UCMenus()
		{
			InitializeComponent();
		}

		public void EnableAllButton()
		{
			if (this.InvokeRequired)
			{
				MethodInvoker invoker = new MethodInvoker(EnableAllButton);
				this.Invoke(invoker);
				return;
			}
			else
			{
				this.EnableButton(ButtonType.Install);
				this.EnableButton(ButtonType.Prev);
				this.EnableButton(ButtonType.Next);
				this.EnableButton(ButtonType.Finish);
			}
		}

		public void DisableAllButton()
		{
			if (this.InvokeRequired)
			{
				MethodInvoker invoker = new MethodInvoker(DisableAllButton);
				this.Invoke(invoker);
				return;
			}
			else
			{
				this.DisableButton(ButtonType.Install);
				this.DisableButton(ButtonType.Prev);
				this.DisableButton(ButtonType.Next);
				this.DisableButton(ButtonType.Finish);
			}
		}

		public void EnableButton(ButtonType p_btnType)
		{	
			switch (p_btnType)
			{
				case ButtonType.Install: this.button_install.Enabled = true; break;
				case ButtonType.Prev: this.button_prev.Enabled = true; break;
				case ButtonType.Next: this.button_next.Enabled = true; break;
				case ButtonType.Finish: this.button_finish.Enabled = true; break;
			}
		}

		public void DisableButton(ButtonType p_btnType)
		{
			switch (p_btnType)
			{
				case ButtonType.Install: this.button_install.Enabled = false; break;
				case ButtonType.Prev: this.button_prev.Enabled = false; break;
				case ButtonType.Next: this.button_next.Enabled = false; break;
				case ButtonType.Finish: this.button_finish.Enabled = false; break;
			}
		}

		public void VisibleButton(ButtonType p_btnType, bool p_visible)
		{
			switch (p_btnType)
			{
				case ButtonType.Install: this.button_install.Visible = p_visible; break;
				case ButtonType.Prev: this.button_prev.Visible = p_visible; break;
				case ButtonType.Next: this.button_next.Visible = p_visible; break;
				case ButtonType.Finish: this.button_finish.Visible = p_visible; break;
			}
		}


		private void button_install_Click(object sender, EventArgs e)
		{
			if (this.OnInstallButtonClickEvent != null)
				OnInstallButtonClickEvent();
		}

		private void button_prev_Click(object sender, EventArgs e)
		{
			if (this.OnPrevButtonClickEvent != null)
				OnPrevButtonClickEvent();
		}

		private void button_next_Click(object sender, EventArgs e)
		{
			if (this.OnNextButtonClickEvent != null)
				OnNextButtonClickEvent();
		}

		private void button_finish_Click(object sender, EventArgs e)
		{
			if (this.OnFinishButtonClickEvent != null)
				OnFinishButtonClickEvent();
		}


		public delegate void OnInstallButtonClickDelegate();
		public delegate void OnPrevButtonClickDelegate();
		public delegate void OnNextButtonClickDelegate();
		public delegate void OnFinishButtonClickDelegate();

		public event OnInstallButtonClickDelegate OnInstallButtonClickEvent;
		public event OnPrevButtonClickDelegate OnPrevButtonClickEvent;
		public event OnNextButtonClickDelegate OnNextButtonClickEvent;
		public event OnFinishButtonClickDelegate OnFinishButtonClickEvent;
	}
}
