using System;
using System.IO;
using System.Windows.Forms;
using Btfax.CommonLib.Log;
using DocumentConverter.Util;
using DocumentConverter.Print;
using DocumentConverter.Threading;

namespace DocumentConverter
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
        private void FrmMain_Load(object sender, EventArgs e)
        {   
        }
        
        private void FrmMain_Shown(object sender, EventArgs e)
        {
            Btfax.CommonLib.Util.ProcessUtil.HideProcess(System.Diagnostics.Process.GetCurrentProcess());
        }
        #endregion
    }
}
