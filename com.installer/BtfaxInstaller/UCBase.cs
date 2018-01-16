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
	public enum UCType
	{
		None = 0,
		Intro = 1,
		Prerequisites = 2,
		Bthmp = 3,
		FaxDriver = 4,
		Package = 5,
		Finish = 6,
	}

	public partial class UCBase : UserControl
	{
		public UCBase()
		{
			InitializeComponent();
			this.UCType = UCType.None;
		}

		protected virtual void SetTitle(string p_title)
		{
			this.label_title.Text = string.Format("\t [{0}]", p_title);
		}

		protected virtual void SetDescription(string p_msg)
		{
			this.label_desc.Text = string.Format("{0} \t", p_msg);
		}

		protected virtual string GetTitle()
		{
			return this.label_title.Text.Trim(); ;
		}


		public UCMenus MenuBar { get; set; }

		public UCType UCType { set; get; }
	}
}
