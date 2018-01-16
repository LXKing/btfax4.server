using System;
using System.Windows.Forms;
using System.Threading;
using System.Diagnostics;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.MsgBox;
using Btfax.CommonLib.Util;
using System.Reflection;
using PostFOD.Util;
using PostFOD.Threading;
using PostFOD.Db;

namespace PostFOD
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
                MessageBox.Show("PSO실행시, 프로세스 일련번호가 입력되지 않았습니다",
                                 "PSO 실행 오류",
                                 MessageBoxButtons.OK,
                                 MessageBoxIcon.Error);
                return;
            }
            if (!FnString.IsNumber(strArgs[0]))
            {
                MessageBox.Show(string.Format("PSO 실행시,  프로세스 일련번호 ({0}) 가 숫자로 입력되지 않았습니다", strArgs[0]),
                                 "PSO 실행 오류",
                                 MessageBoxButtons.OK,
                                 MessageBoxIcon.Error);
                return;
            }
            Config.PROCESS_NO = Convert.ToInt32(strArgs[0]);

			//// 환경 파일 로딩 ////
			string iniFile = string.Format("{0}\\..\\Btfax.ini", Application.StartupPath);
			if (!Config.LoadIni(iniFile))
			{
				MessageBox.Show(string.Format("환경파일({0}) 읽기 실패.", iniFile), "실행 오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}

            //// 윈폼 생성 ////
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Program.s_frmMain = new FrmMain();

            //// 초기화 ////
            AppLog.LogDefaultPath = Application.StartupPath + "\\log\\";
			AppLog.LogPrefix = string.Format("FAX_{0}_PSO_{1}", Config.SYSTEM_NO, Config.PROCESS_NO);
			//AppLog.DisplayDelegate = Program.s_frmMain.Display_AddLog;
			//Config.DisplayDelegate = Program.s_frmMain.Display_Config;
			CommonMsgBox.IsMsgPopup = false;

			//// 프로세스 시작 ////
			Config.APP_VER = string.Format("v{0}", Assembly.GetExecutingAssembly().GetName().Version);
			AppLog.Write(LOG_LEVEL.MSG, string.Format("### PSO {0} 프로세스 시작 ###", Config.APP_VER));

            //// DB 접속 ////
            if (!DbModule.Instance.Open(Config.DB_CONNECTION_STRING))
            {
                AppLog.Write(LOG_LEVEL.ERR, "DB 접속에 실패하였습니다");
                return;
            }
            AppLog.Write(LOG_LEVEL.MSG, "DB 접속 완료");

            //// 환경 DB 로딩 ////
            if (!Config.LoadDb())
            {
                AppLog.Write(LOG_LEVEL.ERR, "환경파일 읽기를 실패하였습니다");
                return;
            }
            AppLog.LogLevel = Config.LOG_LEVEL;
            AppLog.Write(LOG_LEVEL.MSG, "실행환경 적용 완료");

            //// 쓰레드 객체 생성 ////
            s_WorkerThread = WorkerThread.Instance;

			//// UDP 소켓 초기화
			UdpSingle.Instance.InitUdpSocket(false);

            //// 윈폼 실행 ////
            try
            {
				//// 쓰레드 시작 ////
				Program.Thread_Start();
                AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);
                Application.ThreadException += new ThreadExceptionEventHandler(ApplicationThreadException);

				//// 쓰레드 종료 ////
				Program.Thread_Start();
                Application.Run(Program.s_frmMain);
            }
            catch (Exception ex)
            {
				//// 쓰레드 종료 ////
				Program.Thread_Start();
                AppLog.ExceptionLog(ex, "어플리케이션 예외가 발생하였습니다.");
            }

            //// 프로세스 종료 로그 기록 ////
            AppLog.Write(LOG_LEVEL.MSG, "### PSO 프로세스  종료 ###");
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

        static public void Thread_Start()
        {
            if (BtfaxThread.Stop)
            {
                AppLog.Write(LOG_LEVEL.ERR, "아직 쓰레드가 실행중입니다");
                return;
            }

            AppLog.Write(LOG_LEVEL.MSG, "### 쓰레드 시작 ###");
            BtfaxThread.Stop = false;
            Program.s_WorkerThread.StartThread();
        }

        static public void Thread_Stop()
        {
            AppLog.Write(LOG_LEVEL.MSG, "### 쓰레드 종료 ###");
            BtfaxThread.Stop = true;
            Program.s_WorkerThread.JoinThread();
        }

        public static void LogAndDisplay(LOG_LEVEL p_logLevel, string p_strMsg)
        {
            AppLog.Write(p_logLevel, p_strMsg);
        }
        #endregion

        #region fields
        //// Main Form ////
        public static FrmMain s_frmMain = null;

        //// Work Thread ////
        static WorkerThread s_WorkerThread = null;
        #endregion
    }
}
