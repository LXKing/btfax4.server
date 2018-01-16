using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace BtfaxInstaller
{
	class MsgBox
	{
		public static void Err(string p_strMsg)
		{
			MessageBox.Show(p_strMsg, "Installer message", MessageBoxButtons.OK, MessageBoxIcon.Error);
		}

		public static void Info(string p_strMsg)
		{
			MessageBox.Show(p_strMsg, "Installer message", MessageBoxButtons.OK, MessageBoxIcon.Information);
		}
	}
}
