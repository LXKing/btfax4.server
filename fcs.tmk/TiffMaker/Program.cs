using System;
using System.Windows.Forms;
using System.Threading;
using System.IO;
using System.Diagnostics;
using System.Reflection;
using Btfax.CommonLib;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.MsgBox;
using Btfax.CommonLib.Util;
using TiffMaker.Util;
using TiffMaker.Threading;
using TiffMaker.Db;
using TiffMaker.Util;

namespace TiffMaker
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] strArgs)
        {
            // 프로세스 아규먼트 처리
            if (strArgs.Length <= 0)
            {
                MessageBox.Show("프로세스 아이디가 입력되지 않았습니다",
                              "실행 오류",
                              MessageBoxButtons.OK,
                              MessageBoxIcon.Error);
                return;
            }

            if (!long.TryParse(strArgs[0], out Config.PROCESS_NO))
            {
                MessageBox.Show("프로세스 아이디가 잘못되었습니다.",
                              "실행 오류",
                              MessageBoxButtons.OK,
                              MessageBoxIcon.Error);
                return;
            }

            //// 환경 파일 로딩 ////
            string iniFile = string.Format("{0}\\..\\Btfax.ini", Application.StartupPath);
            if (!Config.LoadIni(iniFile))
            {
                MessageBox.Show(string.Format("환경파일({0}) 읽기 실패.", iniFile), "실행 오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            CommonMsgBox.IsMsgPopup = false;

            //// 윈폼 생성 ////
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Program.s_frmMain = new FrmMain();

            //// 초기화 ////
            AppLog.LogDefaultPath = Application.StartupPath + "\\log";
            AppLog.LogPrefix = string.Format("FAX_{0}_TMK_{1}", Config.SYSTEM_NO, Config.PROCESS_NO);
            //AppLog.IsDisplay = false;
            //AppLog.DisplayDelegate = Program.s_frmMain.Display_AddLog;
            //Config.DisplayDelegate = Program.s_frmMain.Display_Config;

            //// 프로세스 시작 ////
            Config.APP_VER = string.Format("v{0}", Assembly.GetExecutingAssembly().GetName().Version);
            AppLog.Write(LOG_LEVEL.MSG, string.Format("### TMK {0} 프로세스 시작 ###", Config.APP_VER));
#if DEBUG
            AppLog.Write(LOG_LEVEL.MSG, "### This is DEBUG mode. ###");
#endif
            //// DB 접속 ////
            if (!DbModule.Instance.Open(Config.DB_CONNECTION_STRING))
            {
                AppLog.Write(LOG_LEVEL.ERR, "DB 접속에 실패하였습니다");
                return;
            }
            AppLog.Write(LOG_LEVEL.MSG, "DB : 접속 완료");

            //// 환경 DB 로딩 ////
            if (!Config.LoadDb())
            {
                AppLog.Write(LOG_LEVEL.ERR, "DB환경설정 읽기를 실패하였습니다");
                return;
            }
            AppLog.LogLevel = Config.LOG_LEVEL;
            AppLog.Write(LOG_LEVEL.MSG, "실행환경 적용 완료");

			//// 암호화 모듈 초기화 ////
			if (Config.ENCRYPT_FIELD_YN == "Y")
			{
				if (File.Exists(Config.ENCRYPT_DLL_FILE))
				{
					if (!EncryptApi.InitApi(Config.ENCRYPT_DLL_FILE))
						AppLog.Write(LOG_LEVEL.WRN, string.Format("암호화 모듈({0})로드 실패", Config.ENCRYPT_DLL_FILE));
					else
						AppLog.Write(LOG_LEVEL.MSG, string.Format("암호화 모듈({0})로드 성공", Config.ENCRYPT_DLL_FILE));
				}
				else
				{
					AppLog.Write(LOG_LEVEL.WRN, string.Format("암호화 모듈({0}) 이 존재하지 않습니다.", Config.ENCRYPT_DLL_FILE));
				}
			}

            //// 쓰레드 객체 생성 ////
            s_DbPollingThread = DbPollingThread.Instance;

            //// 윈폼 실행 ////
            try
            {
				//// 쓰레드 시작 ////
				Program.Thread_Start();

                AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);
                Application.ThreadException += new ThreadExceptionEventHandler(ApplicationThreadException);
                Application.Run(Program.s_frmMain);

				//// 쓰레드 종료 ////
				Program.Thread_Stop();
            }
            catch (Exception ex)
            {
				//// 쓰레드 종료 ////
				Program.Thread_Stop();

                AppLog.ExceptionLog(ex, "어플리케이션 예외가 발생하였습니다.");
            }

            //// 프로세스 종료 로그 기록 ////
            AppLog.Write(LOG_LEVEL.MSG, "### TMK 프로세스  종료 ###");
        }

        #region method

        /// <summary>
        /// 처리되지 않은 예외
        /// </summary>
        public static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            Exception ex = (Exception)e.ExceptionObject;
            AppLog.ExceptionLog(ex, string.Join("\t", "처리되지 않은 예외입니다.", "종료 여부 : ", e.IsTerminating.ToString()));
            Process.GetCurrentProcess().Kill();
            //throw ex;
        }

        /// <summary>
        /// 쓰레드 예외
        /// </summary>
        private static void ApplicationThreadException(object sender, ThreadExceptionEventArgs e)
        {
            AppLog.ExceptionLog(e.Exception, string.Format("어플리케이션 쓰레드 에러가 발생했습니다."));
            //throw e.Exception;
        }
        
        public static void Thread_Start()
        {
            AppLog.Write(LOG_LEVEL.MSG, string.Format("### DB폴링 시작 : INTERVAL[{0}] ###", Config.DB_POLLING_SLEEP));

            BtfaxThread.Stop = false;
            s_DbPollingThread.StartThread();
        }

        public static void Thread_Stop()
        {
            AppLog.Write(LOG_LEVEL.MSG, "### DB폴링 쓰레드 종료 ###");
            BtfaxThread.Stop = true;
            s_DbPollingThread.JoinThread();
        }
        #endregion

        #region static
        public static FrmMain s_frmMain = null;
        public static DbPollingThread s_DbPollingThread = null;
        #endregion
    }
}
