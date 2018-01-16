using System;
using System.Threading;
using System.Windows.Forms;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Threading;
using AdapterOut.Threading;
using AdapterOut.Util;

namespace AdapterOut
{
    public partial class FrmMain : Form
    {
        public FrmMain()
        {
            InitializeComponent();
        }

        #region event handler
        private void frmMain_Load(object sender, EventArgs e){}
        
        private void FrmMain_Shown(object sender, EventArgs e)
        {
            Btfax.CommonLib.Util.ProcessUtil.HideProcess(System.Diagnostics.Process.GetCurrentProcess());
        }

		protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
		{	
			base.OnClosing(e);
		}      
        #endregion
    }
}
