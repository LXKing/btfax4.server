using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace WindowsService
{
	public class WIN32
	{
		#region Dll Import
		[DllImport("kernel32")]
		public static extern long WritePrivateProfileString(string section, string key, string val, string filePath);
		[DllImport("kernel32")]
		public static extern int GetPrivateProfileString(string section, string key, string def, StringBuilder retVal, int size, string filePath);
		#endregion
	}
}
