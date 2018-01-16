using System;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.ServiceProcess;


using System.Management;

namespace WindowsService
{
    class WindowsService : ServiceBase
    {
		private const string SVC_NAME		= "Bridgetec BTFAX4 Package Service";
		private const string SYS32_PATH		= @"C:\Windows\System32";
		private const string IOA_CFG_PATH	= @"\bin_ioa\cfg\watcher.cfg";
		
		
        /// <summary>
        /// Public Constructor for WindowsService.
        /// - Put all of your Initialization code here.
        /// </summary>
        public WindowsService()
        {
			this.ServiceName = SVC_NAME;
			this.EventLog.Source = SVC_NAME;
            this.EventLog.Log = "Application";
            

            // These Flags set whether or not to handle that specific
            //  type of event. Set to true if you need it, false otherwise.
            this.CanHandlePowerEvent			= true;
            this.CanHandleSessionChangeEvent	= true;
            this.CanPauseAndContinue			= true;
            this.CanShutdown					= true;
            this.CanStop						= true;

			if (!EventLog.SourceExists(SVC_NAME))
				EventLog.CreateEventSource(SVC_NAME, "Application");
        }

        /// <summary>
        /// The Main Thread: This is where your Service is Run.
        /// </summary>
        static int Main(string[] args)
        {
			// init log..
			FileLog.Init("c:\\temp\\", "faxsvc");
			FileLog.MSG("----------------------------------------------------------------------------------------------------", true);
			FileLog.MSG(">> btfax4 service start.");
			
			// 서비스 시작
			if (args.Length <= 0)
			{
				// 서비스 등록여부
				if (IsServiceInstalled(SVC_NAME))
				{
					ServiceBase.Run(new WindowsService());
				}
				else
				{
					FileLog.WRN(string.Format("[{0}] Service is not installed", SVC_NAME), true);
					Console.WriteLine("useage : btfax_service.exe install");
				}
								
				FileLog.MSG(">> btfax4 service Exit.");
				return 0;
			}

			for (int i = 0; i < args.Length; i++)
				FileLog.MSG(string.Format("commands [{0}]={1}", i, args[i].ToString()));

			// command 처리.
			bool ret = false;
			string cmd = args[0].ToString().ToLower().Trim();
			switch (cmd)
			{
				case "install":
					ret = InstallService();
					break;

				case "uninstall":
					ret = UnInstallService();
					break;

				case "start":
					ret = SvcStart();
					break;

				case "stop":
					ret = SvcStop();
					break;
				case "-i":
					DisplayProcessInfo(args);
					break;

				case "-s":
					ExecProc(args);
					break;

				case "-k":
					KillProc(args);
					break;

				default:
					FileLog.WRN("Invalid command", true);
					break;
			}

			FileLog.MSG(">> btfax4 service Exit.");			
			return 0;
        }

		// 서비스 등록여부
		private static bool IsServiceInstalled(string serviceName)
		{
		  // get list of Windows services
		  ServiceController[] services = ServiceController.GetServices();

		  // try to find service name
		  foreach (ServiceController service in services)
		  {
			if (service.ServiceName == serviceName)
			  return true;
		  }
		  return false;
		}

		protected static void CollectAllProcInfo()
		{
			Process[] procs = Process.GetProcesses();
			for (int i = 0; i < procs.Length; i++)
			{
				ProcItem item = new ProcItem();
				item.Clear();
				item.pid = procs[i].Id;
				item.ProcName = procs[i].ProcessName;
				
				try
				{
					using (ManagementObjectSearcher mos = new ManagementObjectSearcher(string.Format("SELECT CommandLine FROM Win32_Process WHERE ProcessId = {0}", procs[i].Id)))
					{
						foreach (ManagementObject mo in mos.Get())
						{
							item.commandLine = string.Format("{0}", mo["CommandLine"]);
							//item.FilePath = string.Format("{0}", mo["PathName"]);
							//Console.WriteLine(mo["CommandLine"]);
						}

					}
				}
				catch
				{
					item.commandLine = "";
					item.FilePath = "";
				}

				m_procInfoCollection.Add(procs[i].Id, item);
			}
		}


		protected static string GetProcessList()
		{
			string fileFullName = Assembly.GetExecutingAssembly().Location;
			FileInfo fInfo = new FileInfo(fileFullName);
			string currPath = fInfo.Directory.ToString();
			string cfgPath = string.Format("{0}\\{1}", currPath, IOA_CFG_PATH);
			if (!File.Exists(cfgPath))
			{	
				FileLog.ERR(string.Format("IOA Cfg file not exists. ({0})", cfgPath), true);
				return null;
			}

			StringBuilder sb = new StringBuilder();
			WIN32.GetPrivateProfileString("Watch", "list", null, sb, 255, cfgPath);
			return sb.ToString();
		}

		protected static void ExecProc(string[] args)
		{
			string procList = GetProcessList();
			FileLog.MSG(string.Format("LIST={0}", procList), true);

			if (args.Length <= 1)
			{
				FileLog.WRN("Invalid Process Name.", true);
				return;
			}
						
			string[] procLst = args[1].ToString().Split(',');
			if (procList.Length <= 0)
				return;

			foreach (string arg in procLst)
			{
				System.Threading.Thread.Sleep(500);
				FileLog.MSG("--------------------------------------------------------------------------------------------", true);
				string procName = arg.ToUpper().Trim();

				string section = "";
				string[] procs = procList.Split(',');
				foreach (string str in procs)
				{
					if (str == procName)
					{
						section = str;
						break;
					}
				}

				if (string.IsNullOrEmpty(section))
				{
					FileLog.WRN(string.Format("Not Found Data.({0})", procName), true);
					return;
				}

				//string fileFullName = Assembly.GetExecutingAssembly().Location;
				//FileInfo fInfo = new FileInfo(fileFullName);
				//string currPath = fInfo.Directory.ToString();
				string currPath = AppDomain.CurrentDomain.BaseDirectory;
				string cfgPath = string.Format("{0}\\{1}", currPath, IOA_CFG_PATH);
				//string cfgPath = string.Format("{0}\\{1}", Directory.GetCurrentDirectory(), IOA_CFG_PATH);
				if (!File.Exists(cfgPath))
				{
					FileLog.ERR(string.Format("IOA Cfg file not exists. ({0})", cfgPath), true);
					return;
				}

				string filePath;
				string fileName;
				string startArg;

				StringBuilder sb = new StringBuilder();
				WIN32.GetPrivateProfileString(section, "Path", null, sb, 255, cfgPath);
				filePath = sb.ToString();

				WIN32.GetPrivateProfileString(section, "ExeName", null, sb, 255, cfgPath);
				fileName = sb.ToString();

				WIN32.GetPrivateProfileString(section, "StartArg", null, sb, 255, cfgPath);
				startArg = sb.ToString();

				StartProc(filePath, fileName, startArg, false);
			}
		}

		protected static void KillProc(string[] args)
		{
			//Console.Clear();
			List<ProcItem> tmpProc = new List<ProcItem>();

			string procList = GetProcessList();
			FileLog.MSG(string.Format("LIST={0}", procList), true);
			string[] sections = procList.Split(',');
			if(sections.Length <= 0 )
				return;
			
			if (args.Length <= 1)
			{
				FileLog.WRN("Invalid Process Name.", true);
				return;
			}

			string[] procLst = args[1].ToString().Split(',');
			if (procLst.Length <= 0)
				return;

			string procName;
			string section;

			// 전체 프로세스 목록 수집
			CollectAllProcInfo();

			if (args[1].ToLower().Trim() == "all")
			{
				FileLog.MSG("All process kill !!", true);
				procLst = procList.Split(',');
			}

			foreach (string arg in procLst)
			{
				System.Threading.Thread.Sleep(300);
				tmpProc.Clear();
				procName = "";
				procName = arg.ToUpper().Trim();

				// get section
				section = "";
				foreach (string str in sections)
				{
					if (str.ToUpper().Trim() == procName)
					{
						section = str;
						break;
					}
				}

				if (string.IsNullOrEmpty(section))
				{
					FileLog.WRN(string.Format("Not Found Data Section.({0})", procName), true);
					continue;
				}

				// get filepath				
				string currPath = AppDomain.CurrentDomain.BaseDirectory;
				string cfgPath = string.Format("{0}\\{1}", currPath, IOA_CFG_PATH);
				//string cfgPath = string.Format("{0}\\{1}", Directory.GetCurrentDirectory(), IOA_CFG_PATH);
				//FileLog.MSG(string.Format("curr_path:{0}, ioa_cfg_path:{1}", currPath, cfgPath), true);
				if (!File.Exists(cfgPath))
				{
					FileLog.ERR(string.Format("IOA Cfg file not exists. ({0})", cfgPath), true);
					return;
				}

				string filePath;
				string fileName;
				string startArg;

				StringBuilder sb = new StringBuilder();
				WIN32.GetPrivateProfileString(section, "Path", null, sb, 100, cfgPath);
				filePath = sb.ToString().Trim();

				sb.Clear();
				WIN32.GetPrivateProfileString(section, "ExeName", null, sb, 100, cfgPath);
				fileName = sb.ToString().Trim();

				sb.Clear();
				WIN32.GetPrivateProfileString(section, "StartArg", null, sb, 100, cfgPath);
				startArg = sb.ToString().Trim();


				FileLog.MSG(string.Format("Section:[{0}], FilePath:{1}, FileNae:{2}, StartArg:{3}", section, filePath, fileName, startArg), true);
				
				try
				{
					foreach (KeyValuePair<int, ProcItem> pair in m_procInfoCollection)
					{
						if (pair.Value.ProcName.ToString() == fileName.Replace(".exe", "").Trim())
							tmpProc.Add(pair.Value);
					}

					if (tmpProc.Count <= 0)
					{
						FileLog.WRN(string.Format("Not Found Data : file_path:{0}, fileName:{1}, startArg:{2}", filePath, fileName, startArg), true);
						continue;
					}

					foreach (ProcItem item in tmpProc)
					{
						if (item.commandLine.Contains(startArg))
						{
							Process.GetProcessById(item.pid).Kill();
							FileLog.MSG(string.Format("Process Kill:{0}, {1}", item.ProcName, item.commandLine), true);
						}
					}
				}
				catch (Exception ex)
				{
					FileLog.MSG(string.Format("{0}", ex.Message));
				}
			}
			
		}

		protected static void DisplayProcessInfo(string[] args)
		{
			string procList = GetProcessList();
			FileLog.MSG(string.Format("LIST={0}", procList), true);

			if (args.Length <= 1)
			{
				FileLog.WRN("Invalid Process Name.", true);
				return;
			}

			if (args[1].ToLower().Trim() == "list")
				return;

			string procName = args[1].ToUpper().Trim();
			string section = "";
			string[] procs = procList.Split(',');
			foreach (string str in procs)
			{
				if (str == procName)
				{
					section = str;
					break;
				}
			}

			if (string.IsNullOrEmpty(section))
				return;

			string cfgPath = string.Format("{0}\\{1}", Directory.GetCurrentDirectory(), IOA_CFG_PATH);
			if (!File.Exists(cfgPath))
			{
				FileLog.ERR(string.Format("IOA Cfg file not exists. ({0})", cfgPath), true);
				return;
			}

			StringBuilder sb = new StringBuilder();
			WIN32.GetPrivateProfileString(section, "0", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("RoloID:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "OnOff", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("OnOff:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "Method", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("Method:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "ExeName", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("ExeName:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "Path", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("Path:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "Pattern", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("Pattern:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "Run", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("Run:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "RunWaitTime", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("RunWaitTime:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "StartCmd", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("StartCmd:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "StartArg", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("StartArg:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "StopMethod", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("StopMethod:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "StopCmd", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("StopCmd:{0}", sb.ToString()), true);

			WIN32.GetPrivateProfileString(section, "StopArg", null, sb, 255, cfgPath);
			FileLog.MSG(string.Format("StopArg:{0}", sb.ToString()), true);
		}

		protected static bool StartProc(string p_filePath, string p_fileName, string p_args, bool p_redirectOutput = true)
		{
			try
			{
				FileLog.MSG("Process Info", true);
				FileLog.MSG(string.Format("FilePath:{0}", p_filePath), true);
				FileLog.MSG(string.Format("FileName:{0}", p_fileName), true);
				FileLog.MSG(string.Format("Args:{0}", p_args), true);

				string fileName = string.Format("{0}\\{1}", p_filePath, p_fileName);
				if (!File.Exists(fileName))
				{
					FileLog.ERR(string.Format("Process file not exists !!. ({0})", fileName), true);
					return false;
				}


				using (Process Proc = new Process())
				{
					ProcessStartInfo ProcStartInfo = new ProcessStartInfo();
					ProcStartInfo.FileName = fileName;
					ProcStartInfo.Arguments = p_args;
					//ProcStartInfo.Verb = "RUNAS";
					ProcStartInfo.WorkingDirectory = p_filePath;
					ProcStartInfo.CreateNoWindow = true;
					ProcStartInfo.UseShellExecute = !p_redirectOutput;
					ProcStartInfo.RedirectStandardOutput = p_redirectOutput;

					Proc.StartInfo = ProcStartInfo;
					
					if (Proc.Start())
						FileLog.MSG(string.Format("Process start success. ({0}{1})", Proc.StartInfo.FileName, Proc.StartInfo.Arguments), true);
					else
						FileLog.ERR(string.Format("Process start fail. ({0}{1})", Proc.StartInfo.FileName, Proc.StartInfo.Arguments), true);

					//if (p_redirectOutput)
					//{
					//    string strOutput = Proc.StandardOutput.ReadToEnd();
					//    strOutput = strOutput.Replace("\r\r\n", "\n");

					//    FileLog.MSG(strOutput, true);
					//}
				}

				return true;
			}
			catch (Exception ex)
			{
				FileLog.ERR(ex.Message);
				return false;
			}
		}
		
		/// <summary>
		/// 서비스 설치
		/// </summary>
		/// <returns></returns>
		private static bool InstallService()
		{	
			try
			{
				if (IsServiceInstalled(SVC_NAME))
				{	
					FileLog.MSG(string.Format(" [{0}] already are installed.", SVC_NAME), true);
					return true;
				}
				
				string filePath = SYS32_PATH;
				string fileName = "SC.EXE";

				//string fileFullName = Assembly.GetExecutingAssembly().Location;
				//FileInfo fInfo = new FileInfo(fileFullName);
				//string currPath = fInfo.Directory.ToString();
				string currPath = AppDomain.CurrentDomain.BaseDirectory;
				
				string launcherName = string.Format("{0}\\btfax_svc.exe", currPath);
				string args = string.Format(" CREATE \"{0}\" binpath= \"{1}\"", SVC_NAME, launcherName);

				if (!StartProc(filePath, fileName, args))
					return false;

				return true;
			}
			catch (Exception ex)
			{	
				FileLog.ERR(string.Format("btfax_service.exe install fail. ({0})", ex.Message), true);
				return false;
			}
		}


		private static bool UnInstallService()
		{	
			try
			{
				SvcStop();
				string filePath = SYS32_PATH;
				string fileName = "SC.EXE";
				string args = string.Format(" DELETE \"{0}\"", SVC_NAME);

				if (!StartProc(filePath, fileName, args))
					return false;

				return true;
			}
			catch (Exception ex)
			{	
				FileLog.ERR(string.Format("btfax_service.exe uninstall fail. ({0})", ex.Message), true);
				return false;
			}
		}

		private static bool SvcStart()
		{	
			try
			{
				if (!IsServiceInstalled(SVC_NAME))
				{
					FileLog.MSG(string.Format(" [{0}] Not installed.", SVC_NAME), true);
					return true;
				}

				string filePath = SYS32_PATH;
				string fileName = "NET.EXE";
				string args = string.Format(" START \"{0}\"", SVC_NAME);

				if (!StartProc(filePath, fileName, args))
					return false;

				return true;
			}
			catch (Exception ex)
			{	
				FileLog.ERR(string.Format("btfax_service.exe start fail. ({0})", ex.Message), true);
				return false;
			}
		}

		private static bool SvcStop()
		{	
			try
			{
				string filePath = SYS32_PATH;
				string fileName = "NET.EXE";
				string args = string.Format(" STOP \"{0}\"", SVC_NAME);

				if (!StartProc(filePath, fileName, args))
					return false;

				Wait5Second();
				return true;
			}
			catch (Exception ex)
			{	
				FileLog.ERR(string.Format("btfax_service.exe stop fail. ({0})", ex.Message), true);
				return false;
			}
		}

		private static void Wait5Second()
		{
			string strWaitMsg = string.Format("\"{0}\" Please Wait ", SVC_NAME);
			Console.Write(strWaitMsg);

			string strDot = ".";
			for (int i = 0; i < 10; i++)
			{
				Console.Write(strDot);
				System.Threading.Thread.Sleep(500);
			}

			Console.WriteLine("\n");
		}

        /// <summary>
        /// Dispose of objects that need it here.
        /// </summary>
        /// <param name="disposing">Whether or not disposing is going on.</param>
        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
        }

        /// <summary>
        /// OnStart: Put startup code here
        ///  - Start threads, get inital data, etc.
        /// </summary>
        /// <param name="args"></param>
        protected override void OnStart(string[] args)
        {
			//string procList = GetProcessList();
			//FileLog.MSG(string.Format("LIST={0}", procList), true);
			
			string fileFullName = Assembly.GetExecutingAssembly().Location;
			FileInfo fInfo = new FileInfo(fileFullName);
			string currPath = fInfo.Directory.ToString();
			////string currPath = AppDomain.CurrentDomain.BaseDirectory;

			string cfgPath = string.Format("{0}\\{1}", currPath, IOA_CFG_PATH);
			if (!File.Exists(cfgPath))
			{
				FileLog.ERR(string.Format("IOA Cfg file not exists. ({0})", cfgPath), true);
				return;
			}

			string section = "IFIOA";
			string filePath;
			string fileName;
			string startArg;

			StringBuilder sb = new StringBuilder();
			WIN32.GetPrivateProfileString(section, "Path", null, sb, 255, cfgPath);
			filePath = sb.ToString();

			WIN32.GetPrivateProfileString(section, "ExeName", null, sb, 255, cfgPath);
			fileName = sb.ToString();

			WIN32.GetPrivateProfileString(section, "StartArg", null, sb, 255, cfgPath);
			startArg = sb.ToString();

			StartProc(filePath, fileName, startArg, false);
			
            //base.OnStart(args);
        }

        /// <summary>
        /// OnStop: Put your stop code here
        /// - Stop threads, set final data, etc.
        /// </summary>
        protected override void OnStop()
        {
			//string procList = GetProcessList();
			//FileLog.MSG(string.Format("LIST={0}", procList), true);

			string fileFullName = Assembly.GetExecutingAssembly().Location;
			FileInfo fInfo = new FileInfo(fileFullName);
			string currPath = fInfo.Directory.ToString();

			StartProc(fInfo.Directory.FullName, fInfo.Name, "-k all", false);

			//string procList = GetProcessList();
			//FileLog.MSG(string.Format("LIST={0}", procList), true);

			//string[] arrProcLst = procList.Split(',');
			//foreach (string strSection in arrProcLst)
			//{
			//    string[] param = new string[2] { "-k", strSection };
			//    KillProc(param);
			//    System.Threading.Thread.Sleep(500);
			//}

            //base.OnStop();
        }

        /// <summary>
        /// OnPause: Put your pause code here
        /// - Pause working threads, etc.
        /// </summary>
        protected override void OnPause()
        {
            base.OnPause();
        }

        /// <summary>
        /// OnContinue: Put your continue code here
        /// - Un-pause working threads, etc.
        /// </summary>
        protected override void OnContinue()
        {
            base.OnContinue();
        }

        /// <summary>
        /// OnShutdown(): Called when the System is shutting down
        /// - Put code here when you need special handling
        ///   of code that deals with a system shutdown, such
        ///   as saving special data before shutdown.
        /// </summary>
        protected override void OnShutdown()
        {
            base.OnShutdown();
        }

        /// <summary>
        /// OnCustomCommand(): If you need to send a command to your
        ///   service without the need for Remoting or Sockets, use
        ///   this method to do custom methods.
        /// </summary>
        /// <param name="command">Arbitrary Integer between 128 & 256</param>
        protected override void OnCustomCommand(int command)
        {
            //  A custom command can be sent to a service by using this method:
            //#  int command = 128; //Some Arbitrary number between 128 & 256
            //#  ServiceController sc = new ServiceController("NameOfService");
            //#  sc.ExecuteCommand(command);

            base.OnCustomCommand(command);
        }

        /// <summary>
        /// OnPowerEvent(): Useful for detecting power status changes,
        ///   such as going into Suspend mode or Low Battery for laptops.
        /// </summary>
        /// <param name="powerStatus">The Power Broadcase Status (BatteryLow, Suspend, etc.)</param>
        protected override bool OnPowerEvent(PowerBroadcastStatus powerStatus)
        {
            return base.OnPowerEvent(powerStatus);
        }

        /// <summary>
        /// OnSessionChange(): To handle a change event from a Terminal Server session.
        ///   Useful if you need to determine when a user logs in remotely or logs off,
        ///   or when someone logs into the console.
        /// </summary>
        /// <param name="changeDescription"></param>
        protected override void OnSessionChange(SessionChangeDescription changeDescription)
        {
            base.OnSessionChange(changeDescription);
        }

		struct ProcItem
		{	
			public int pid;
			public string ProcName;
			public string FileName;
			public string FilePath;
			public string commandLine;

			public void Clear()
			{
				pid = -1;
				ProcName = "";
				FileName = "";
				FilePath = "";
				commandLine = "";
			}
		}
		private static Dictionary<int, ProcItem> m_procInfoCollection = new Dictionary<int, ProcItem>();
    }
}
