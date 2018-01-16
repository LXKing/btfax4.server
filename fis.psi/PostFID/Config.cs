using System;
using Btfax.CommonLib;
using Btfax.CommonLib.Config;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Alarm;
using System.Reflection;
using PostFid.DB;

namespace PostFid.Util
{
	class Config : ConfigBase
	{
		#region const
		//// DB 항목 ////
		public static string SYSTEM_GROUP = "";
		public static int INITIAL_SLEEP = 0;
		public static int DB_POLLING_SLEEP = 0;
		public static int FETCH_CNT = 0;
		public static int THREAD_CNT = 0;
		public static string STG_HOME_PATH = "";

		public static UInt32 IOA_ALARM_PORT = 0;
		public static UInt32 IOA_ALARM_BASE_CODE = 0;
		public static UInt32 IOA_MODULE_ID = 0;

		public static string LOOP_FAXNO = "";	// LOOP 팩스 번호
		public static string LTM_IP = "";
		public static int LTM_PORT = 0;


		public static string FAS_IP = "";
		public static int FAS_PORT = -1;
		public static int BUFFER_SIZE = -1;
		public static string RECV_FAXBOX_URL = "";
		public static string RECV_NOTIFY_TITLE = "";
		public static string RECV_NOTIFY_MESSAGE = "";

		public static string SMTP_IP = "";
		public static int SMTP_PORT = -1;
		public static string SMTP_FROM = "";
		public static string SMTP_SUBJECT = "";
		public static bool SMTP_USEDEFAULTCREDENTIALS = false;
		public static bool SMTP_ENABLESSL = false;
		public static int SMTP_TIMEOUT = -1;
		public static int AOB_PORT = -1;  //WCD(2014-05-28)
		public static string RET_DEPT_CD = "";  //WCD(2014-05-28)
		#endregion

		#region constructor
		static Config()
		{
			PROCESS_TYPE = P_TYPE.PSI;
		}
		#endregion

		#region method
		public static bool LoadDb()
		{
			string strTemp = "";

			if (!LoadDbCommon(DbModule.Instance))
				return false;

			if (!ReadConfigDb_(DbModule.Instance, "SYSTEM_GROUP", ref Config.SYSTEM_GROUP))
				Config.SYSTEM_GROUP = "90000";

			if (!ReadConfigDb_(DbModule.Instance, "INITIAL_SLEEP", ref Config.INITIAL_SLEEP))
				return false;

			if (!ReadConfigDb_(DbModule.Instance, "DB_POLLING_SLEEP", ref Config.DB_POLLING_SLEEP))
				return false;

			if (!ReadConfigDb_(DbModule.Instance, "FETCH_CNT", ref Config.FETCH_CNT))
				return false;

			if (!ReadConfigDb_(DbModule.Instance, "THREAD_CNT", ref Config.THREAD_CNT))
				return false;

			if (!ReadConfigDb_(DbModule.Instance, "STG_HOME_PATH", ref Config.STG_HOME_PATH))
				return false;

			ReadConfigDb_(DbModule.Instance, "LOOP_FAXNO", ref Config.LOOP_FAXNO);

			if (!ReadConfigDb_(DbModule.Instance, "LTM_IP", ref Config.LTM_IP))
				LTM_IP = "127.0.0.1";

			if (!ReadConfigDb_(DbModule.Instance, "LTM_PORT", ref Config.LTM_PORT))
				LTM_PORT = 1050;




			//if (!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_PORT", ref Config.IOA_ALARM_PORT))
			//    Config.IOA_ALARM_PORT = 9361;

			//if (!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_BASE_CODE", ref Config.IOA_ALARM_BASE_CODE))
			//    Config.IOA_ALARM_BASE_CODE = 6010000;

			//if (!ReadConfigDb_(DbModule.Instance, "IOA_MODULE_ID", ref Config.IOA_MODULE_ID))
			//    Config.IOA_MODULE_ID = 589825;

			if (!ReadConfigDb_(DbModule.Instance, "RECV_FAXBOX_URL", ref Config.RECV_FAXBOX_URL))
				return false;

			if (!ReadConfigDb_(DbModule.Instance, "RECV_NOTIFY_TITLE", ref Config.RECV_NOTIFY_TITLE))
				return false;

			if (Config.RECV_NOTIFY_TITLE == "")
				Config.RECV_NOTIFY_TITLE = "팩스 수신";

			if (!ReadConfigDb_(DbModule.Instance, "RECV_NOTIFY_MESSAGE", ref Config.RECV_NOTIFY_MESSAGE))
				Config.RECV_NOTIFY_MESSAGE = "수신 팩스가 있습니다.";

			if (Config.RECV_NOTIFY_MESSAGE == "")
				Config.RECV_NOTIFY_MESSAGE = "수신된 팩스가 있습니다.";

			if (!ReadConfigDb_(DbModule.Instance, "SMTP_IP", ref Config.SMTP_IP))
				return false;

			if (Config.SMTP_IP == "")
				Config.SMTP_IP = "127.0.0.1";

			if (!ReadConfigDb_(DbModule.Instance, "SMTP_PORT", ref Config.SMTP_PORT))
				return false;

			if (Config.SMTP_PORT == -1)
				Config.SMTP_PORT = 25;

			if (!ReadConfigDb_(DbModule.Instance, "SMTP_FROM", ref Config.SMTP_FROM))
				return false;

			if (Config.SMTP_FROM == "")
				Config.SMTP_FROM = "btfaxv4.0_admin@bridgetec.co.kr";

			if (!ReadConfigDb_(DbModule.Instance, "SMTP_SUBJECT", ref Config.SMTP_SUBJECT))
				return false;

			if (Config.SMTP_SUBJECT == "")
				Config.SMTP_SUBJECT = "수신된 팩스가 있습니다.";

			string tmpValue = "";
			if (!ReadConfigDb_(DbModule.Instance, "SMTP_USEDEFAULTCREDENTIALS", ref tmpValue))
				return false;

			Config.SMTP_USEDEFAULTCREDENTIALS = tmpValue == "1" ? true : false;

			if (!ReadConfigDb_(DbModule.Instance, "SMTP_ENABLESSL", ref tmpValue))
				return false;

			Config.SMTP_ENABLESSL = tmpValue == "1" ? true : false;

			if (!ReadConfigDb_(DbModule.Instance, "SMTP_TIMEOUT", ref Config.SMTP_TIMEOUT))
				return false;

			if (!ReadConfigDb_(DbModule.Instance, "RET_DEPT_CD", ref Config.RET_DEPT_CD))            //WCD 2014-06-24
				Config.RET_DEPT_CD = "";

			if (Config.SMTP_TIMEOUT == -1) Config.SMTP_TIMEOUT = 5000;

			// -----------------------------------------------------------------------------------------------
			//  FAS 설정 정보 (현재 임시로 등록)
			// -----------------------------------------------------------------------------------------------
			int tryParseInt = -1;
			bool tryParseIntResult = false;

			if (!DbModule.Instance.ReadConfig("COMMON", "FAS", "FAS_IP", out Config.FAS_IP))
				return false;

			if (Config.FAS_IP == "") Config.FAS_IP = "127.0.0.1";

			if (!DbModule.Instance.ReadConfig("COMMON", "FAS", "LISTEN_PORT", out strTemp))
				return false;

			tryParseIntResult = int.TryParse(strTemp, out tryParseInt);
			Config.FAS_PORT = tryParseIntResult == true ? tryParseInt : 1010;

			if (!DbModule.Instance.ReadConfig("COMMON", "FAS", "BUFFER_SIZE", out strTemp))
				return false;

			tryParseIntResult = int.TryParse(strTemp, out tryParseInt);
			Config.BUFFER_SIZE = tryParseIntResult == true ? tryParseInt : 2048;
			// -----------------------------------------------------------------------------------------------
			// -----------------------------------------------------------------------------------------------
			//  AOB 설정 정보 (현재 임시로 등록)
			// -----------------------------------------------------------------------------------------------
			//if (!DbModule.Instance.ReadConfig("COMMON", "AOB", "LISTEN_PORT", out strTemp))    //WCD(2014-05-28)   
			//    return false;

			DbModule.Instance.ReadConfig("COMMON", "AOB", "LISTEN_PORT", out strTemp);    //WCD(2014-05-28)   			

			tryParseIntResult = int.TryParse(strTemp, out tryParseInt);                        //WCD(2014-05-28)   
			Config.AOB_PORT = tryParseIntResult == true ? tryParseInt : 1001;                   //WCD(2014-05-28)   

			return true;
		}
		#endregion
	}
}
