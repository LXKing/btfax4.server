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
	public partial class UCIntro : UCBase
	{
		public UCIntro()
		{
			InitializeComponent();
			base.SetTitle("설치순서");
			base.UCType = UCType.Intro;
			FLog.Msg(string.Format("{0}", base.GetTitle()));
		}
	}
}
