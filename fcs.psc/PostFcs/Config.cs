using System;
using Btfax.CommonLib;
using Btfax.CommonLib.Config;
using Btfax.CommonLib.Log;
using PostFcs.DB;

namespace PostFcs.Util
{
    class Config : ConfigBase
    {
        #region const
        //// DB 항목 ////
        public static string    SYSTEM_GROUP = "";
        public static int       INITIAL_SLEEP;
        public static int       DB_POLLING_SLEEP;
        public static int       FETCH_CNT;

        public static UInt32    IOA_ALARM_PORT = 0;
        public static UInt32    IOA_ALARM_BASE_CODE = 0;
        public static UInt32    IOA_MODULE_ID = 0;

        public static int       THREAD_CNT;
		public static long		TIF_FILE_MAX_SIZE;
        public static string    DOC_FILTERING_YN;
        public static string    STG_HOME_PATH;
        public static string    FINISHED_TIF_PATH;
        public static string    INBOUND_TIF_PATH;

		// ADD - KIMCG : 20150914
		public static string	LOOP_FAXNO = "";
		// ADD - END
        #endregion

        #region constructor
        static Config()
        {
           PROCESS_TYPE = P_TYPE.PSC;
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

            if (!ReadConfigDb_(DbModule.Instance,"INITIAL_SLEEP", ref Config.INITIAL_SLEEP))
                return false;

            if (!ReadConfigDb_(DbModule.Instance,"DB_POLLING_SLEEP", ref Config.DB_POLLING_SLEEP))
                return false;

            if (!ReadConfigDb_(DbModule.Instance,"FETCH_CNT", ref Config.FETCH_CNT))
                return false;

			if (!ReadConfigDb_(DbModule.Instance, "TIF_FILE_MAX_SIZE", ref Config.TIF_FILE_MAX_SIZE))
				Config.TIF_FILE_MAX_SIZE = 0;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_PORT", ref Config.IOA_ALARM_PORT))
                Config.IOA_ALARM_PORT = 9361;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_BASE_CODE", ref Config.IOA_ALARM_BASE_CODE))
                Config.IOA_ALARM_BASE_CODE = 6010000;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_MODULE_ID", ref Config.IOA_MODULE_ID))
                Config.IOA_MODULE_ID = 589825;

            if (!ReadConfigDb_(DbModule.Instance,"THREAD_CNT", ref Config.THREAD_CNT))
                return false;

            if (!ReadConfigDb_(DbModule.Instance,"DOC_FILTERING_YN", ref Config.DOC_FILTERING_YN))
                return false;

            if (!ReadConfigDb_(DbModule.Instance,"STG_HOME_PATH", ref Config.STG_HOME_PATH))
                return false;

            if (!ReadConfigDb_(DbModule.Instance,"FINISHED_TIF_PATH", ref strTemp))
                return false;

            Config.FINISHED_TIF_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadConfigDb_(DbModule.Instance, "INBOUND_TIF_PATH", ref strTemp))
                return false;

            Config.INBOUND_TIF_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);
			
			if (!ReadConfigDb_(DbModule.Instance, "LOOP_FAXNO", ref strTemp))
				Config.LOOP_FAXNO = "";
			else
				Config.LOOP_FAXNO = strTemp;

            return true;
        }
        #endregion
    }
}
