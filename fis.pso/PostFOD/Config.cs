using System;
using System.Windows.Forms;
using System.Reflection;
using Btfax.CommonLib;
using Btfax.CommonLib.Config;
using Btfax.CommonLib.Log;
using PostFOD.Db;

namespace PostFOD.Util
{
    class Config : ConfigBase
    {
        #region const
        //// DB항목 ///
        public static int           INITIAL_SLEEP         = 0; // [ms]
        public static int           DB_POLLING_SLEEP      = 0; // [ms]
        public static int           FETCH_CNT = 10;
		public static string		SEND_FAXBOX_URL = "";
		public static string		SEND_NOTIFY_TITLE = "";
		public static string		SEND_NOTIFY_MESSAGE = "";

		public static int BUFFER_SIZE = -1;
		public static string		FAS_IP = "";
		public static int			FAS_PORT = 0;



        #endregion

        #region constructor
        static Config()
        {
            //// ConfigBase 항목 ////
            //APP_VER = "v4.0.4";
            //// ConfigBase 항목 //// 
            APP_VER = string.Format("v{0}", Assembly.GetExecutingAssembly().GetName().Version);
            PROCESS_TYPE = P_TYPE.PSO;
        }
        #endregion

        #region method
        public static bool LoadDb()
        {
            string strTemp = "";

            if (!LoadDbCommon(DbModule.Instance))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "INITIAL_SLEEP", ref Config.INITIAL_SLEEP))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "DB_POLLING_SLEEP", ref Config.DB_POLLING_SLEEP))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "FETCH_CNT", ref Config.FETCH_CNT))
                return false;

			if (!ReadConfigDb_(DbModule.Instance, "SEND_FAXBOX_URL", ref Config.SEND_FAXBOX_URL))
				Config.SEND_FAXBOX_URL = "";

			if (!ReadConfigDb_(DbModule.Instance, "SEND_NOTIFY_TITLE", ref Config.SEND_NOTIFY_TITLE))
				Config.SEND_NOTIFY_TITLE = "팩스 송신";

			if (!ReadConfigDb_(DbModule.Instance, "SEND_NOTIFY_MESSAGE", ref Config.SEND_NOTIFY_MESSAGE))
				Config.SEND_NOTIFY_MESSAGE = "송신 팩스가 있습니다.";
			
			if (!DbModule.Instance.ReadConfig("COMMON", "FAS", "FAS_IP", out Config.FAS_IP))
				Config.FAS_IP = "127.0.0.1";

			if (!DbModule.Instance.ReadConfig("COMMON", "FAS", "LISTEN_PORT", out strTemp))
			{
				FAS_PORT = 1010;
			}
			else
			{
				if (!Int32.TryParse(strTemp, out Config.FAS_PORT))
					FAS_PORT = 1010;
			}
			
            return true;
        }
        #endregion
    }

}
