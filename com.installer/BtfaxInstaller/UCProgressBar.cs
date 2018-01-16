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
	public partial class UCProgressBar : UserControl
	{
		public UCProgressBar()
		{
			InitializeComponent();
		}

		public void Init(int p_nCurr, int p_nMax)
		{
			//this.label_item.Text = string.Format("{0}/{1}", p_nCurr, p_nMax);
			this.progressBar1.Value = p_nCurr;
			this.progressBar1.Maximum = p_nMax;
		}

		public void IncreaseVal(int p_nVal)
		{
			if (this.InvokeRequired)
			{
				IncreaseValDelegate d = new IncreaseValDelegate(IncreaseVal);
				this.Invoke(d, new object[] { p_nVal });
			}
			else
			{
				this.progressBar1.Value += p_nVal;
			}
		}

		private delegate void IncreaseValDelegate(int p_nVal);
	}
}
