using System;
using System.Windows.Forms;
using Btfax.CommonLib;
using Btfax.CommonLib.Config;
using Btfax.CommonLib.Log;
using TiffProcessor.Db;

namespace TiffProcessor.Util
{
    class Config : ConfigBase
    {
        #region const
        public static string TIFF_PROCESSING_PATH = Application.StartupPath + "\\tif_processing";

        //// DB항목 ///
        public static string    SYSTEM_GROUP        = "";
        public static int       INITIAL_SLEEP       = 0; // [ms]
        public static int       DB_POLLING_SLEEP    = 0; // [ms]
        public static int       FETCH_CNT           = 0;
        public static UInt32    IOA_ALARM_PORT      = 0;
        public static UInt32    IOA_ALARM_BASE_CODE = 0;
        public static UInt32    IOA_MODULE_ID       = 0;

        
        public static int       POLLING_SLEEP       = 0; // [ms]

        public static string    STG_HOME_PATH       = "";
        public static string    INPUT_DOCS_PATH     = "";
        public static string    MADE_TIF_PATH       = "";
        public static string    CONVERTED_TIF_PATH  = "";
        public static string    FINISHED_TIF_PATH   = "";

        #endregion

        #region constructor
        static Config()
        {
            //// ConfigBase 항목 ////
            APP_VER = string.Format("v{0}", System.Reflection.Assembly.GetExecutingAssembly().GetName().Version);
            //SYSTEM_TYPE = S_TYPE.FCS;
            PROCESS_TYPE = P_TYPE.TPC;
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

            if(!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_PORT", ref Config.IOA_ALARM_PORT))
                Config.IOA_ALARM_PORT = 9361;

            if(!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_BASE_CODE", ref Config.IOA_ALARM_BASE_CODE))
                Config.IOA_ALARM_BASE_CODE = 6010000;
            
            if(!ReadConfigDb_(DbModule.Instance, "IOA_MODULE_ID", ref Config.IOA_MODULE_ID))
                Config.IOA_MODULE_ID = 589825;
            
            if (!ReadConfigDb_(DbModule.Instance, "STG_HOME_PATH", ref Config.STG_HOME_PATH))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "INPUT_DOCS_PATH", ref strTemp))
                return false;
            Config.INPUT_DOCS_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadConfigDb_(DbModule.Instance, "MADE_TIF_PATH", ref strTemp))
                return false;
            Config.MADE_TIF_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadConfigDb_(DbModule.Instance, "CONVERTED_TIF_PATH", ref strTemp))
                return false;
            Config.CONVERTED_TIF_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadConfigDb_(DbModule.Instance, "FINISHED_TIF_PATH", ref strTemp))
                return false;
            Config.FINISHED_TIF_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            return true;
        }
        #endregion
    }
}
