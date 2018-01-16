using System;
using System.Windows.Forms;
using System.Reflection;
using Btfax.CommonLib;
using Btfax.CommonLib.Config;
using Btfax.CommonLib.Log;
using TiffMaker.Db;

namespace TiffMaker.Util
{
    public class Config : ConfigBase
    {
        #region const
        //// 기정의 항목 ////
        public static string TIFF_MAKING_PATH       = Application.StartupPath + "\\tif_making";
        
        //// DB 항목 ////
        public static string SYSTEM_GROUP           = "";
        public static int MAKE_THREAD_CNT           = 0;
        public static int INITIAL_SLEEP             = 0; // [ms]
        public static int DB_POLLING_SLEEP          = 0; // [ms]
        public static int FETCH_CNT                 = 0;        
        public static UInt32 IOA_MODULE_ID          = 0;

        public static string STG_HOME_PATH          = "";
		public static string LOG_HOME_PATH			= "";
        public static string FAXFORM_PATH           = "";
        public static string INPUT_DOCS_PATH        = "";
        public static string MADE_TIF_PATH          = "";
        public static string FINISHED_TIF_PATH      = "";
		public static string ENCRYPT_FIELD_YN		= "";
		public static string ENCRYPT_DLL_FILE		= "";
        #endregion
        
        #region constructor
        static Config()
        {
            //// ConfigBase 항목 ////
            PROCESS_TYPE = P_TYPE.TMK;
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

            if (!ReadConfigDb_(DbModule.Instance, "MAKE_THREAD_CNT", ref Config.MAKE_THREAD_CNT))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "INITIAL_SLEEP", ref Config.INITIAL_SLEEP))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "DB_POLLING_SLEEP", ref Config.DB_POLLING_SLEEP))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "FETCH_CNT", ref Config.FETCH_CNT))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_MODULE_ID", ref Config.IOA_MODULE_ID))
                Config.IOA_MODULE_ID = 589825;

			// ADD - KIMCG : 20140902
			// 암호화 모듈 환경설정
			if (!ReadConfigDb_(DbModule.Instance, "ENCRYPT_FIELD_YN", ref Config.ENCRYPT_FIELD_YN))
				Config.ENCRYPT_FIELD_YN = "N";

			if (Config.ENCRYPT_FIELD_YN == "Y")
			{	
				if (!ReadConfigDb_(DbModule.Instance, "ENCRYPT_DLL_FILE", ref Config.ENCRYPT_DLL_FILE))
					Config.ENCRYPT_DLL_FILE = "";
			}
			// ADD - END

            if (!ReadConfigDb_(DbModule.Instance, "STG_HOME_PATH", ref Config.STG_HOME_PATH))
                return false;

			if (!ReadConfigDb_(DbModule.Instance, "LOG_HOME_PATH", ref Config.LOG_HOME_PATH))
				Config.LOG_HOME_PATH = "";

            if (!ReadConfigDb_(DbModule.Instance, "FAXFORM_PATH", ref Config.FAXFORM_PATH))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "INPUT_DOCS_PATH", ref strTemp))
                return false;
            Config.INPUT_DOCS_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadConfigDb_(DbModule.Instance, "MADE_TIF_PATH", ref strTemp))
                return false;
            Config.MADE_TIF_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadConfigDb_(DbModule.Instance, "FINISHED_TIF_PATH", ref strTemp))
                return false;
            Config.FINISHED_TIF_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            return true;
        }
        #endregion
    }
}
