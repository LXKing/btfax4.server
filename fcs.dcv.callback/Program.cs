using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.IO;
using Microsoft.Win32;


namespace DCV_Callback
{
    static class Program
    {
        /// <summary>
        /// 해당 응용 프로그램의 주 진입점입니다.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {   
            string INI_FILENAME = System.Windows.Forms.Application.StartupPath + "\\DCV_CALLBACK.INI";

            FileLog.Initialize(System.Windows.Forms.Application.StartupPath + "\\log\\", "dcb");
            FileLog.Write("========================================================================");
            FileLog.Write("ImageMaker Session start !!");

            if (!File.Exists(INI_FILENAME))
            {
                FileLog.Write("DCV_CALLBACK.INI File Not Exists");
                FileLog.Write("ImageMaker Session End !!");
                FileLog.Write("========================================================================");
                return;
            }

            if (args.Length <= 0)
            {
                FileLog.Write("ImageMaker Post Parameters is 0");
                FileLog.Write("ImageMaker Session End !!");
                FileLog.Write("========================================================================");
                return;
            }

            FileInfo fInfo = null;
            for (int i = 0; i < args.Length; i++)
            {
                FileLog.Write(string.Format("Parameters {0} : {1}", i, args[0]));
                if (File.Exists(args[0]))
                {
                    fInfo = new FileInfo(args[0]);
                    break;
                }
            }

            if (fInfo == null)
            {
                FileLog.Write("ImageMaker Tif File Not Exits.");
                FileLog.Write("ImageMaker Session End !!");
                FileLog.Write("========================================================================");
                return;
            }

            FileLog.Write(string.Format("ImageMaker is create Tif File done. ({0})", fInfo.FullName));

            string dcvOutPut    = StringUtil.GetPrivateProfileStringEx(INI_FILENAME, "CONV_INFO", "DCV_OUTPUT");
            string state        = StringUtil.GetPrivateProfileStringEx(INI_FILENAME, "CONV_INFO", "STATE");

            FileLog.Write(string.Format("DCV_OUTPUT. ({0})", dcvOutPut));
            FileLog.Write(string.Format("STATE. ({0})", state));

            if (string.IsNullOrEmpty(dcvOutPut) || string.IsNullOrEmpty(state))
            {
                FileLog.Write("Invalid DCV_OUTPUT Info!!");
                FileLog.Write("ImageMaker Session End !!");
                FileLog.Write("========================================================================");
                return;
            }

            bool ret = false;
            for (int i = 0; i < 3; i++)
            {
                try
                {
                    File.Copy(fInfo.FullName, dcvOutPut, true);
                    ret = true;
                    break;
                }
                catch(Exception ex)
                {
                    ret = false;
                    FileLog.Write(ex.Message);
                    System.Threading.Thread.Sleep(200);
                    continue;
                }
            }

            if (!ret)
            {
                FileLog.Write(string.Format("Copy Fail!! src[{0}] dest[{1}]", fInfo.FullName, dcvOutPut));
                FileLog.Write("ImageMaker Session End !!");
                FileLog.Write("========================================================================");
            }

            FileLog.Write(string.Format("Copy Success!! src[{0}] dest[1]", fInfo.FullName, dcvOutPut));

            ret = false;
            for (int i = 0; i < 3; i++)
            {
                ret = StringUtil.WritePrivateProfileStringEx(INI_FILENAME, "CONV_INFO", "STATE", "COMPLETED");
                if (!ret)
                {
                    ret = false;
                    FileLog.Write("Write COMPLETED STATE error!!");
                    continue;
                }

                ret = true;
                break;
            }

			try
			{
				fInfo.Delete();
			}
			catch (Exception ex)
			{
				FileLog.Write(string.Format("Delete Fail!! src[{0}], msg[{1}]", fInfo.FullName, ex.Message));
				FileLog.Write("ImageMaker Session End !!");
				FileLog.Write("========================================================================");
				return;

			}

			// 로그 정리
			try
			{
				string[] logs = Directory.GetFiles(System.Windows.Forms.Application.StartupPath + "\\log\\", "*.log");
				foreach (string logFile in logs)
				{
					FileInfo fLogInfo = new FileInfo(logFile);
					if (fLogInfo.CreationTime < DateTime.Now.AddDays(-7))
					{
						FileLog.Write(string.Format("old log delete:{0}", fLogInfo.FullName));
						fLogInfo.Delete();
					}
				}
			}
			catch (Exception ex)
			{
				FileLog.Write(string.Format("Delete Fail!! msg[{0}]", fInfo.FullName, ex.Message));
			}

            FileLog.Write("ImageMaker Session End !!");
            FileLog.Write("========================================================================");

            //Application.EnableVisualStyles();
            //Application.SetCompatibleTextRenderingDefault(false);
            //Application.Run(new MainForm());
        }
    }
}
