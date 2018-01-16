using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Btfax.CommonLib.Log;
using PostFOD.Util;
using PostFOD.Threading;

namespace PostFOD
{
    public partial class FrmMain : Form
    {
        #region constructor
        public FrmMain()
        {
            //// 컴포넌트 초기화 ////
            InitializeComponent();
        }
        #endregion

        #region event handler
        private void FrmMain_Load(object sender, EventArgs e){}
		#endregion

		private void FrmMain_Shown(object sender, EventArgs e)
		{
			Btfax.CommonLib.Util.ProcessUtil.HideProcess(System.Diagnostics.Process.GetCurrentProcess());
		}
    }
}
