using System;
using System.Drawing;
using System.Diagnostics;
using System.Drawing.Printing;
using System.Runtime.InteropServices;

using System.ComponentModel;
using System.ComponentModel.Design;
using System.Xml;
using System.IO;

using System.Drawing.Imaging;
using System.Windows.Interop;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;



using Btfax.CommonLib.Log;
using DocumentConverter.Util;

namespace DocumentConverter.Print
{
    class PrintingHandler
    {
        #region win32
        [DllImport("winspool.drv", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern bool SetDefaultPrinter(string Name);

        [DllImport("Shell32.dll")]
        public static extern bool ShellExecuteEx(SHELLEXECUTEINFO lpExecInfo);

        [DllImport("user32.dll")]
        internal static extern bool ShowWindowAsync(IntPtr hWnd, int nCmdShow);
        #endregion
        #region enum @ 현재 사용안함
        private enum WINDOW_STYLE
        {
            SW_HIDE = 0,
            SW_SHOWNORMAL = 1,
            SW_SHOWMINIMIZED = 2,
            SW_SHOWMAXIMIZED = 3,
            SW_MAXIMIZE = 3,
            SW_SHOWNOACTIVATE = 4,
            SW_SHOW = 5,
            SW_MINIMIZE = 6,
        }
        #endregion

        #region static
        private static PrintingHandler s_instance = null;
        public static PrintingHandler Instance
        {
            get 
            {
                if (s_instance == null)
                    s_instance = new PrintingHandler();
                return s_instance;
            }
        }
        #endregion
        
        #region inner class
        [StructLayout(LayoutKind.Sequential)]
        public class SHELLEXECUTEINFO
        {
            #region constructor
            public SHELLEXECUTEINFO()
            {
                Clear();
            }
            #endregion

            #region method
            public void Clear()
            {
                cbSize = 60;
                fMask = 0;
                hwnd = 0;
                lpVerb = null;
                lpFile = null;
                lpParameters = null;
                lpDirectory = null;
                nShow = 0;
                hInstApp = 0;
                lpIDList = 0;
                lpClass = null;
                hkeyClass = 0;
                dwHotKey = 0;
                hIcon = 0;
                hProcess = 0;
            }
            #endregion

            #region field
            public int cbSize;
            public int fMask;
            public int hwnd;
            public string lpVerb;
            public string lpFile;
            public string lpParameters;
            public string lpDirectory;
            public int nShow;
            public int hInstApp;
            public int lpIDList;
            public string lpClass;
            public int hkeyClass;
            public int dwHotKey;
            public int hIcon;
            public int hProcess;
            #endregion
        }
        #endregion

        #region method
        public bool ExistsPrintDriver(string p_strPrinterName)
        {
            foreach (string prnName in PrinterSettings.InstalledPrinters)
            {
                if (prnName == p_strPrinterName)
                    return true;
            }
            return false;
        }
        
        public bool IsDefaultPrinter(string p_strPrinterName)
        {
            PrinterSettings prn = new PrinterSettings();
            bool ret;
            if (prn.PrinterName == p_strPrinterName)
                ret = true;
            else
                ret = false;

            prn = null;

            return ret;
        }

        public bool SetPrinter(string p_strPrinterName)
        {
            return SetDefaultPrinter(p_strPrinterName);
        }

        public bool IsImageFormat(string p_strFileName, string p_strConvertableImgFormat)
        {
            try
            {
                int idx = p_strFileName.LastIndexOf(".");
                if (idx < 0)
                    return false;

                string ext = p_strFileName.Substring(idx + 1).ToUpper();
                if (!p_strConvertableImgFormat.Contains(ext))
                    return false;

                return true;
            }
            catch
            {
                return false;
            }
        }
        
        public bool Print(string p_strFileFullName)
        {
		

            #region win32
            //// 인쇄정보 SET ////
            //ClearSheddExecuteInfo();

            //m_shellExecuteInfo.cbSize = 60;
            //m_shellExecuteInfo.nShow = 0; // SW_HIDE=0, SW_SHOWNORMAL=1

            //if (p_bIsImageFormat)
            //{
            //    m_shellExecuteInfo.lpFile = @"C:\WINDOWS\system32\rundll32.exe";
            //    m_shellExecuteInfo.lpParameters = string.Format(@"shimgvw.dll,ImageView_PrintTo /pt ""{0}"" ""{1}"" """" """"", p_strFileFullName, Config.EXEC_PRINT_DRIVER);
            //}
            //else
            //{
            //    m_shellExecuteInfo.lpVerb = "print";
            //    m_shellExecuteInfo.lpFile = p_strFileFullName;
            //}

            ////// 인쇄 ////
            //return ShellExecuteEx(m_shellExecuteInfo);
            #endregion

            try
            {
                using (Process p = new Process())
                {
					p.StartInfo.Verb = "Print";
					p.StartInfo.UseShellExecute = true;
					p.StartInfo.FileName = p_strFileFullName;
                    p.StartInfo.CreateNoWindow = true;
                    p.StartInfo.WindowStyle = ProcessWindowStyle.Minimized;
                    p.Start();

                    if (!p.HasExited)
                    {
                        p.WaitForExit(Config.CONVERTING_WAIT_TIME);

                        if (!p.HasExited)
                        {
                            AppLog.Write(LOG_LEVEL.ERR, string.Format("변환문서({0})를 ({1})초동안 변환하지 못했습니다.", p_strFileFullName, (Config.CONVERTING_WAIT_TIME / 1000)));
                            p.CloseMainWindow();
                            return false;
                        }
                    }
                    return true;
                }
            }
            catch (Exception ex)
            {
                AppLog.Write(LOG_LEVEL.ERR, string.Format("문서변환({0})중 다음과같은 오류가 발생하였습니다 {1}", p_strFileFullName, ex.Message));
                return false;
            }
        }

        private void ClearSheddExecuteInfo()
        {
            m_shellExecuteInfo.Clear();
        }


		public bool Converert(string p_strSrc, string p_strDest)
		{
			string strTempFile = string.Format("{0}.temp", p_strDest);

			// 임시파일 생성.
			if (!SaveAs2Tiff(p_strSrc, strTempFile))
				return false;

			if (!File.Exists(strTempFile))
				return false;

			if (!Create1BppTif(strTempFile, p_strDest))
				return false;

			return true;
		}


		private bool SaveAs2Tiff(string p_strSrcFileFullName, string p_strDestFileFullName)
		{
			FileStream srcFs = File.Open(p_strSrcFileFullName, FileMode.Open);
			if (srcFs == null)
				return false;

			bool ret = false;
			try
			{
				Image img = Bitmap.FromStream(srcFs);
				img.Save(p_strDestFileFullName, ImageFormat.Tiff);
				img.Dispose();

				srcFs.Close();
				srcFs.Dispose();
				ret = true;
			}
			catch (Exception ex)
			{
				ret = false;
			}

			return ret;
		}


		private bool Resize4Fax(Bitmap p_srcBmp, out Bitmap p_destBmp)
		{
			p_destBmp = null;
			int width = 1728;
			int height = 2200;
			if (p_srcBmp.Width > p_srcBmp.Height)
				p_srcBmp = this.Rotate90(p_srcBmp);

			p_destBmp = new Bitmap(width, height);
			p_destBmp.SetResolution(204, 196);


			Graphics g = Graphics.FromImage(p_destBmp);
			g.FillRectangle(System.Drawing.Brushes.White, new Rectangle(0, 0, width, height));


			//고해상도 그래픽은 메모리 오버플로 발생
			//g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
			//g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;
			//g.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.HighQuality;
			//g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighQuality;

			int nMarX = 20;
			int nMarY = 20;

			g.DrawImage(p_srcBmp, nMarX, nMarY, p_destBmp.Width - (nMarX * 2), p_destBmp.Height - (nMarY * 2));
			g.Dispose();

			return true;
		}

		private Bitmap Rotate90(Bitmap p_bmp)
		{
			MemoryStream memStrm = new MemoryStream();
			p_bmp.Save(memStrm, ImageFormat.Tiff);

			Image img = Image.FromStream(memStrm);
			img.RotateFlip(RotateFlipType.Rotate90FlipNone);
			memStrm.Close();
			memStrm.Dispose();
			return (Bitmap)img;
		}

		private bool Create1BppTif(string p_srcFileName, string p_destFileName)
		{
			bool result = false;
			Bitmap srcBitmap = null;
			FileStream srcFs = null;
			FileStream dstFs = null;
			TiffBitmapEncoder tiffEncoder = new TiffBitmapEncoder();
			tiffEncoder.Compression = TiffCompressOption.Ccitt3;

			try
			{
				srcFs = new FileStream(p_srcFileName, FileMode.Open, FileAccess.Read);
				dstFs = new FileStream(p_destFileName, FileMode.Create);
				srcBitmap = new Bitmap(srcFs);

				int pageCnt = srcBitmap.GetFrameCount(FrameDimension.Page);
				if (pageCnt <= 0)
					throw new Exception("GetFrameCount() : Tiff page count is 0");

				int frame = -1;
				for (int pg = 0; pg < pageCnt; pg++)
				{
					frame = srcBitmap.SelectActiveFrame(FrameDimension.Page, pg);

					// resize..
					Bitmap faxBmp = null;
					using (Bitmap tmpBitmap = srcBitmap.Clone() as Bitmap)
					{
						if (tmpBitmap == null)
							throw new Exception("SelectActiveFrame error.");

						if (!this.Resize4Fax(tmpBitmap, out faxBmp))
							throw new Exception("Resize4Fax error.");
					}

					if (faxBmp == null)
						throw new Exception("Fax bmp is null.");

					// save memory stream..
					BitmapSource bitmapSrc = null;
					MemoryStream memStrm = new MemoryStream();
					faxBmp.Save(memStrm, ImageFormat.Tiff);
					faxBmp.Dispose();

					TiffBitmapDecoder decoder = new TiffBitmapDecoder(memStrm, BitmapCreateOptions.PreservePixelFormat, BitmapCacheOption.Default);
					bitmapSrc = decoder.Frames[0];
					tiffEncoder.Frames.Add(BitmapFrame.Create(bitmapSrc));
					bitmapSrc.Freeze();
				}

				dstFs.Flush();
				tiffEncoder.Save(dstFs);
				dstFs.Close();
				dstFs.Dispose();
				tiffEncoder.Frames.Clear();

				result = true;
			}
			catch (Exception ex)
			{
				result = false;
			}
			finally
			{
				if (srcFs != null)
				{
					srcFs.Close();
					srcFs.Dispose();
				}

				if (srcBitmap != null)
				{
					srcBitmap.Dispose();
				}
			}

			return result;
		}





        #endregion

        #region field
        private SHELLEXECUTEINFO m_shellExecuteInfo = new SHELLEXECUTEINFO();
        #endregion




    }
}
