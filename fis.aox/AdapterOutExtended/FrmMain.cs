using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Log;

namespace AdapterOutExtended
{
    public partial class FrmMain : Form
    {
        #region constructor
        public FrmMain()
        {
            InitializeComponent();
        }
        #endregion

        #region event handler
        private void frmMain_Load(object sender, EventArgs e)
        {   
        }

        private void FrmMain_Shown(object sender, EventArgs e)
        {
            Btfax.CommonLib.Util.ProcessUtil.HideProcess(System.Diagnostics.Process.GetCurrentProcess());
        }
        #endregion

        #region field
        object lockthis = new object();
        #endregion
    }
}
