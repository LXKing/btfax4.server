using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace TiffMaker.Util
{
	class EncryptApi
	{
		#region dll import
		[DllImport("kernel32.dll")]
		static extern IntPtr LoadLibrary(string dllName);

		[DllImport("kernel32.dll")]
		static extern IntPtr GetProcAddress(IntPtr hModule, string procName);
		#endregion

		#region delegates
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		delegate int InitDelegate();

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		delegate void DisposeDelegate();

		[UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		delegate IntPtr EncryptDelegate(string strSrc);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
		delegate IntPtr DecryptDelegate(string strSrc);
		#endregion

		public static bool InitApi(string p_strDllPath)
		{
			if (!File.Exists(p_strDllPath))
				return false;

			m_hDllHandle = LoadLibrary(p_strDllPath);
			if (m_hDllHandle == IntPtr.Zero)
				return false;

			IntPtr pFunc = GetProcAddress(m_hDllHandle, "Init");
			InitDelegate d = (InitDelegate)Marshal.GetDelegateForFunctionPointer(pFunc, typeof(InitDelegate));
			int ret = d();
			if (ret <= 0)
				return false;

			m_strDllFile = p_strDllPath;
			return true;
		}

		public static bool Encrypt(string p_strSrc, out string p_strOut)
		{
			p_strOut = "";
			bool bRet = false;

			if (m_hDllHandle == IntPtr.Zero)
			{
				p_strOut = p_strSrc;
				return false;
			}

			lock (typeof(EncryptApi))
			{
				IntPtr pFunc = GetProcAddress(m_hDllHandle, "Encrypt");
				EncryptDelegate d = (EncryptDelegate)Marshal.GetDelegateForFunctionPointer(pFunc, typeof(EncryptDelegate));
				IntPtr pRet = d(p_strSrc);
				if (pRet == IntPtr.Zero)
				{
					p_strOut = p_strSrc;
					bRet = false;
				}
				else
				{
					p_strOut = Marshal.PtrToStringAnsi(pRet);
					bRet = true;
				}
			}

			return bRet;
		}

		public static bool Decrypt(string p_strSrc, out string p_strOut)
		{
			p_strOut = "";
			bool bRet = false;
			
			if (m_hDllHandle == IntPtr.Zero)
			{
				p_strOut = p_strSrc;
				return false;
			}

			lock (typeof(EncryptApi))
			{	
				IntPtr pFunc = GetProcAddress(m_hDllHandle, "Decrypt");
				DecryptDelegate d = (DecryptDelegate)Marshal.GetDelegateForFunctionPointer(pFunc, typeof(DecryptDelegate));
				IntPtr pRet = d(p_strSrc);
				if (pRet == IntPtr.Zero)
				{
					p_strOut = p_strSrc;
					bRet = false;
				}
				else
				{
					p_strOut = Marshal.PtrToStringAnsi(pRet);
					bRet = true;
				}
			}
			
			return bRet;
		}

		public static int DisposeApi()
		{	
			if (m_hDllHandle == IntPtr.Zero)
				return 0;

			IntPtr pFunc = GetProcAddress(m_hDllHandle, "Dispose");
			DisposeDelegate d = (DisposeDelegate)Marshal.GetDelegateForFunctionPointer(pFunc, typeof(DisposeDelegate));
			d();

			m_hDllHandle = IntPtr.Zero;
			m_strDllFile = "";

			return 1;
		}

		#region fields
		protected static IntPtr m_hDllHandle = IntPtr.Zero;
		protected static string m_strDllFile = "";
		#endregion

	}
}
