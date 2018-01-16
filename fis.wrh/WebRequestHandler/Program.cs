using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Threading;
using System.Diagnostics;
using System.Reflection;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.MsgBox;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Alarm;

namespace WebRequestHandler
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
            
            // 윈폼 생성
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Program.s_frmMain = new FrmMain();

            // 초기화
            AppLog.LogDefaultPath = Application.StartupPath + "\\log";
            AppLog.LogPrefix = string.Format("FAX_{0}_WRH_{1}", Config.SYSTEM_NO, Config.PROCESS_NO);
			//AppLog.DisplayDelegate = s_frmMain.Display_AddLog;
			//AppLog.DisplayIncrementErrorDelegate = s_frmMain.IncrementError;
			//Config.DisplayDelegate = Program.s_frmMain.Display_Config;
            
            // 프로세스 시작
            Config.APP_VER = string.Format("v{0}", Assembly.GetExecutingAssembly().GetName().Version);
            AppLog.Write(LOG_LEVEL.MSG, string.Format("### WRH {0} 프로세스  시작 ###", Config.APP_VER));
#if DEBUG
            AppLog.Write(LOG_LEVEL.MSG, "### This is DEBUG mode. ###");
#endif   

            // DB 접속
            if (!DbModule.Instance.Open(Config.DB_CONNECTION_STRING))
            {
                AppLog.Write(LOG_LEVEL.ERR, "DB 접속에 실패하였습니다");
                return;
            }
            AppLog.Write(LOG_LEVEL.MSG, "DB : 접속 완료");

            // 환경 DB 로딩
            if (!Config.LoadDb())
            {
                AppLog.Write(LOG_LEVEL.ERR, "환경파일 읽기를 실패하였습니다");
                return;
            }
            AppLog.LogLevel = Config.LOG_LEVEL;
            AppLog.Write(LOG_LEVEL.MSG, "실행환경 적용 완료");

            //// 알람API 초기화 ////
            int alarmRet = AlarmAPI.Instance.Init((uint)Config.SYSTEM_NO
                                                , Config.SYSTEM_GROUP
                                                , Config.IOA_MODULE_ID
                                                , Config.IOA_ALARM_PORT
                                                , Config.IOA_ALARM_BASE_CODE
                                                , (uint)Config.PROCESS_NO
                                                , Config.PROCESS_TYPE.ToString()
                                                , 1
												, AppLog.LogDefaultPath + "\\alarm\\"
                                                );
            if (alarmRet != 0)
                AppLog.Write(LOG_LEVEL.WRN, "Alarm API 설정을 실패하였습니다");
            else
                AppLog.Write(LOG_LEVEL.MSG, "Alarm API 로드 완료");

            if (!System.IO.Directory.Exists(Config.TIF_MANIPULATE_PATH))
                System.IO.Directory.CreateDirectory(Config.TIF_MANIPULATE_PATH);

            // 쓰레드 객체 생성
            // Making Tiff 쓰레드
            for (int i = 0; i < Config.REQUEST_HANDLER_THREAD_CNT; i++)
            {
                RequestHandlerThread thread = new RequestHandlerThread();
                Program.s_requestHandlerThreads.Add(thread);
            }
            
            // 윈폼 실행
            try
            {
				Program.StartService();
                AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);
                Application.ThreadException += new ThreadExceptionEventHandler(ApplicationThreadException);
                Application.Run(Program.s_frmMain);
				Program.StopService();
            }
            catch (Exception ex)
            {
				Program.StopService();
                AppLog.ExceptionLog(ex, "어플리케이션 예외가 발생하였습니다.");
            }
            
            // 프로세스 종료 로그 기록
            AppLog.Write(LOG_LEVEL.MSG, "### WRH 프로세스  종료 ###");
        }

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


        #region static - method
        public static void StartService()
        {
            BtfaxThread.Stop = false;

            foreach (BtfaxThread thread in Program.s_requestHandlerThreads)
                thread.StartThread();

            WebListener.Instance.Start();
        }

        public static void StopService()
        {
            BtfaxThread.Stop = true;
            AlarmAPI.Instance.StopAlarmAPI();
            foreach (BtfaxThread thread in Program.s_requestHandlerThreads)
                thread.JoinThread();

            WebListener.Instance.Stop();
        }
        #endregion

        #region static - fields
        public static FrmMain s_frmMain = null;
        static List<RequestHandlerThread> s_requestHandlerThreads = new List<RequestHandlerThread>();
        #endregion
    }
}
