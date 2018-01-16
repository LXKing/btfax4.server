using System;
using System.Windows.Forms;
using System.Threading;
using System.Diagnostics;
using System.Reflection;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.MsgBox;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Alarm;
using PostFid.Util;
using PostFid.DB;
using PostFid.Threading;

namespace PostFid
{
    static class Program
    {
        /// <summary>
        /// 해당 응용 프로그램의 주 진입점입니다.
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
            AppLog.LogPrefix = string.Format("FAX_{0}_PSI_{1}", Config.SYSTEM_NO, Config.PROCESS_NO);
			//AppLog.IsDisplay = false;
			//AppLog.DisplayDelegate = Program.s_frmMain.Display_AddLog;
			//Config.DisplayDelegate = Program.s_frmMain.Display_Config;
            
            //// 프로세스 시작 ////
            Config.APP_VER = string.Format("v{0}", Assembly.GetExecutingAssembly().GetName().Version);
            AppLog.Write(LOG_LEVEL.MSG, string.Format("### PSI {0} 프로세스 시작 ###", Config.APP_VER));            
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
                AppLog.Write(LOG_LEVEL.ERR, "환경DB 읽기를 실패하였습니다");
                return;
            }
            AppLog.LogLevel = Config.LOG_LEVEL;
            AppLog.Write(LOG_LEVEL.MSG, "실행환경 적용 완료");

			////// 알람API 초기화 ////
			//int alarmRet = AlarmAPI.Instance.Init((uint)Config.SYSTEM_NO
			//                                    , Config.SYSTEM_GROUP
			//                                    , Config.IOA_MODULE_ID
			//                                    , Config.IOA_ALARM_PORT
			//                                    , Config.IOA_ALARM_BASE_CODE
			//                                    , (uint)Config.PROCESS_NO
			//                                    , Config.PROCESS_TYPE.ToString()
			//                                    , 1
			//                                    , AppLog.LogDefaultPath + "\\alarm\\"
			//                                    );
			//if (alarmRet != 0)
			//    AppLog.Write(LOG_LEVEL.WRN, "Alarm API 설정을 실패하였습니다");
			//else
			//    AppLog.Write(LOG_LEVEL.MSG, "Alarm API 로드 완료");

            //// 쓰레드 객체 생성 ////
            s_DbPollingThread = DbPollingThread.Instance;

            /// UDP 소켓 초기화
            UdpSingle.Instance.InitUdpSocket(false);

            //// 윈폼 실행 ////
            try
            {
				//// 쓰레드 시작 ////
				Program.Thread_Start();

                AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);
                Application.ThreadException += new ThreadExceptionEventHandler(ApplicationThreadException);
                Application.Run(Program.s_frmMain);

				//// 쓰레드 종료 ////
				Program.Thread_Start();
            }
            catch (Exception ex)
            {
				//// 쓰레드 종료 ////
				Program.Thread_Start();

                AppLog.ExceptionLog(ex, "어플리케이션 예외가 발생하였습니다.");
            }

            //// 프로세스 종료 로그 기록 ////
            AppLog.Write(LOG_LEVEL.MSG, "### PSI 프로세스  종료 ###");
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
            AppLog.Write(LOG_LEVEL.MSG, string.Format("### DB폴링 시작 : INTERVAL[{0}] ###", Config.DB_POLLING_SLEEP));

            BtfaxThread.Stop = false;         
            s_DbPollingThread.StartThread();
        }

        static public void Thread_Stop()
        {
            AppLog.Write(LOG_LEVEL.MSG, "### DB폴링 쓰레드 종료 ###");
            BtfaxThread.Stop = true;
            AlarmAPI.Instance.StopAlarmAPI();
            s_DbPollingThread.JoinThread();
        }
        #endregion

        #region static
        public static FrmMain s_frmMain = null;
        public static DbPollingThread s_DbPollingThread = null;
        #endregion
    }
}
