using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace BtfaxInstaller
{
	class Win32API
	{	
		[DllImport("kernel32")]
		public static extern int WritePrivateProfileString(string p_strSection, string p_strKey, string p_strValue, string p_strFileFullName);
		[DllImport("kernel32")]
		public static extern int GetPrivateProfileString(string p_strSection, string p_strKey, string p_strDefValue, StringBuilder p_strRetValue, int p_nSize, string p_strFileFullName);
	}
}
