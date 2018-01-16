using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Log;

namespace WebRequestHandler
{
    public partial class FrmMain : Form
    {
        public FrmMain()
        {
            InitializeComponent();
        }

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
