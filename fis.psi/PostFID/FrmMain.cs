using System;
using System.Windows.Forms;
using System.Threading;
using System.Reflection;
using Btfax.CommonLib.Log;
using PostFid.Threading;
using PostFid.Util;

namespace PostFid
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
        private void FrmMain_Load(object sender, EventArgs e){ }

        private void FrmMain_Shown(object sender, EventArgs e)
        {
            Btfax.CommonLib.Util.ProcessUtil.HideProcess(System.Diagnostics.Process.GetCurrentProcess());
        }

        #endregion

    }
}
