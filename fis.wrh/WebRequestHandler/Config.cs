using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Btfax.CommonLib;
using Btfax.CommonLib.Xml;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.MsgBox;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Config;


namespace WebRequestHandler
{
    class Config : ConfigBase
    {
        #region fields
        
        //// DB 항목 ////
        public static string    SYSTEM_GROUP                = "";
        public static int       HTTP_PORT                   = 0;
        public static string    HTTP_URL                    = "";
        public static int       REQUEST_HANDLER_THREAD_CNT  = 3;
        public static int       WAITLIST_INVALID_SECONDS    = 5; // [second]
        public static string    STG_HOME_PATH               = "";
        public static string    INBOUND_TIF_PATH            = "";
        public static string    INBOUND_TIF_FULL_PATH
        {
            get { return STG_HOME_PATH + "\\" + INBOUND_TIF_PATH; }
        }
        public static string TIF_MANIPULATE_PATH;

        public static UInt32 IOA_ALARM_PORT                 = 0;
        public static UInt32 IOA_ALARM_BASE_CODE            = 0;
        public static UInt32 IOA_MODULE_ID                  = 0;

        #endregion

        # region constructor
        static Config()
        {
            //// ConfigBase 항목 ////
            PROCESS_TYPE = P_TYPE.WRH;

            //// 경로 지정 ////
            TIF_MANIPULATE_PATH = Application.StartupPath + "\\tif\\";
        }

        #endregion
        
        #region implementation

        public static bool LoadDb()
        {
            if (!LoadDbCommon(DbModule.Instance))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "SYSTEM_GROUP", ref Config.SYSTEM_GROUP))
                Config.SYSTEM_GROUP = "90000";

            if (!ReadConfigDb_(DbModule.Instance, "HTTP_PORT", ref Config.HTTP_PORT))
                return false;
            HTTP_URL = string.Format("http://{0}:{1}/", SYSTEM_IP, HTTP_PORT);

            if (!ReadConfigDb_(DbModule.Instance, "REQUEST_HANDLER_THREAD_CNT", ref Config.REQUEST_HANDLER_THREAD_CNT))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "WAITLIST_INVALID_SECONDS", ref Config.WAITLIST_INVALID_SECONDS))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "STG_HOME_PATH", ref Config.STG_HOME_PATH))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "INBOUND_TIF_PATH", ref Config.INBOUND_TIF_PATH))
                return false;
            
            if (!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_PORT", ref Config.IOA_ALARM_PORT))
                Config.IOA_ALARM_PORT = 9361;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_ALARM_BASE_CODE", ref Config.IOA_ALARM_BASE_CODE))
                Config.IOA_ALARM_BASE_CODE = 6010000;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_MODULE_ID", ref Config.IOA_MODULE_ID))
                Config.IOA_MODULE_ID = 589825;

            return true;
        }

        #endregion
    }
}
