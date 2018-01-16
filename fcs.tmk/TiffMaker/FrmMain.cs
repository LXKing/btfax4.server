using System;
using System.IO;
using System.Windows.Forms;
using Btfax.CommonLib.Log;
using TiffMaker.Threading;
using TiffMaker.Util;
using System.Diagnostics;

namespace TiffMaker
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
        private void FrmMain_Load(object sender, EventArgs e) {}

        private void FrmMain_Shown(object sender, EventArgs e)
        {   
            Btfax.CommonLib.Util.ProcessUtil.HideProcess(System.Diagnostics.Process.GetCurrentProcess());
        }

        private void FrmMain_FormClosing(object sender, FormClosingEventArgs e) {}

        private void tsmuItemDbPollingStart_Click(object sender, EventArgs e)
        {
            Program.Thread_Start();
        }

        private void tsmuItemDbPollingStop_Click(object sender, EventArgs e)
        {
            Program.Thread_Stop();
        }
        #endregion

        
    }
}
