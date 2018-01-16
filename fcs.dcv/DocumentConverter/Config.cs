using System;
using Btfax.CommonLib;
using Btfax.CommonLib.Config;
using Btfax.CommonLib.Log;
using System.Reflection;
using DocumentConverter.Db;
using System.Data;

namespace DocumentConverter.Util
{
    class Config : ConfigBase
    {
        #region const
        public static string    TIFF_CONVERTING_PATH    = System.Windows.Forms.Application.StartupPath + "\\tif_converting";
        public static string    CONVERT_ABLE_IMG_FORMAT = "";

        
        //// DB항목 ///
        public static string    SYSTEM_GROUP = "";
        public static int       INITIAL_SLEEP           = 0; // [ms]
        public static int       DB_POLLING_SLEEP        = 0; // [ms]
        public static string    EXEC_PRINT_DRIVER       = "";
        public static string    DCV_CALLBACK_PATH       = "";
        public static string    DCV_CALLBACK_INI_FILE   = "";
        public static int       FETCH_CNT               = 1;

        public static UInt32    IOA_ALARM_PORT          = 0;
        public static UInt32    IOA_ALARM_BASE_CODE     = 0;
        public static UInt32    IOA_MODULE_ID           = 0;

        public static int       CONVERTING_WAIT_TIME    = 0; // [ms]
        public static string    STG_HOME_PATH           = "";
        public static string    INPUT_DOCS_PATH         = "";
        public static string    CONVERTED_TIF_PATH      = "";
        public static string    FINISHED_TIF_PATH       = "";
		public static string DELETE_INPUT_FILE_YN = "N";
        #endregion

        #region constructor
        static Config()
        {   
            PROCESS_TYPE = P_TYPE.DCV;
        }
        #endregion
        
        #region method
        public static bool LoadDb()
        {
            DataTable ReadConfigTable;

            string strTemp = "";

            //DataRow ReadTable;

            if (!ReadDb_begin_(DbModule.Instance, out ReadConfigTable))
                return false;
            
            if (!LoadDbCommon(DbModule.Instance))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "SYSTEM_GROUP", ref Config.SYSTEM_GROUP))
                Config.SYSTEM_GROUP = "90000";

            if (!ReadDb_(ReadConfigTable, "INITIAL_SLEEP", ref Config.INITIAL_SLEEP))
                return false;

            if (!ReadDb_(ReadConfigTable, "DB_POLLING_SLEEP", ref Config.DB_POLLING_SLEEP))
                return false;

            if (!ReadDb_(ReadConfigTable, "FETCH_CNT", ref strTemp))
                return false;
            if (!Int32.TryParse(strTemp, out Config.FETCH_CNT))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_PORT", ref Config.IOA_ALARM_PORT))
                Config.IOA_ALARM_PORT = 9361;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_BASE_CODE", ref Config.IOA_ALARM_BASE_CODE))
                Config.IOA_ALARM_BASE_CODE = 6010000;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_MODULE_ID", ref Config.IOA_MODULE_ID))
                Config.IOA_MODULE_ID = 589825;

			if (!ReadConfigDb_(DbModule.Instance, "DELETE_INPUT_FILE_YN", ref Config.DELETE_INPUT_FILE_YN))
				Config.DELETE_INPUT_FILE_YN = "N";

            if (!ReadDb_(ReadConfigTable, "CONVERT_ABLE_IMG_FORAMT", ref Config.CONVERT_ABLE_IMG_FORMAT))
                return false;

            if (!ReadDb_(ReadConfigTable, "CONVERTING_WAIT_TIME", ref Config.CONVERTING_WAIT_TIME))
                return false;

            if (!ReadDb_(ReadConfigTable, "EXEC_PRINT_DRIVER", ref Config.EXEC_PRINT_DRIVER))
                return false;

            if (!ReadDb_(ReadConfigTable, "DCV_CALLBACK_PATH", ref Config.DCV_CALLBACK_PATH))
                return false;

            Config.DCV_CALLBACK_INI_FILE = string.Format("{0}\\DCV_CALLBACK.INI", Config.DCV_CALLBACK_PATH);

            if (!ReadDb_(ReadConfigTable, "STG_HOME_PATH", ref Config.STG_HOME_PATH))
                return false;

            if (!ReadDb_(ReadConfigTable, "INPUT_DOCS_PATH", ref strTemp))
                return false;
            Config.INPUT_DOCS_PATH = string.Format("{0}{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadDb_(ReadConfigTable, "CONVERTED_TIF_PATH", ref strTemp))
                return false;
            Config.CONVERTED_TIF_PATH = string.Format("{0}{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadDb_(ReadConfigTable, "FINISHED_TIF_PATH", ref strTemp))
                return false;
            Config.FINISHED_TIF_PATH = string.Format("{0}{1}", Config.STG_HOME_PATH, strTemp);

            ReadDb_end_(ReadConfigTable);
            
            return true;
        }


        #endregion
    }
}
