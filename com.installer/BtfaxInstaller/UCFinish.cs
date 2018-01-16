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
	public partial class UCFinish : UCBase
	{
		public UCFinish()
		{
			InitializeComponent();
			base.SetTitle("설치완료");
			base.UCType = UCType.Finish;
			FLog.Msg(string.Format("{0}", base.GetTitle()));
		}
	}
}
