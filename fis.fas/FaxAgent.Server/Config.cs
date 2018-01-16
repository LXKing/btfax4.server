using System;
using System.Text;
using System.Windows.Forms;
using System.Reflection;
using Btfax.CommonLib;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Config;

namespace FaxAgent.Server
{
    class Config : ConfigBase
    {
        // DB 항목
        public static string    SYSTEM_GROUP                = "";
        public static int       CLIENT_ALIVE_INTERVAL       = 0;
        public static int       CLIENT_LIMIT_RESPONSE_TIME  = 0;
        public static int       SERVER_LIMIT_RESPONSE_TIME  = 0;

        public static int       MAX_CONENCTION              = -1;
        public static int       LISTEN_PORT                 = -1;
        public static int       BUFFER_SIZE                 = -1;
        public static int       MAX_ACCEPT_OPERATION        = -1;
        public static int       BACK_LOG                    = -1;
        
        public static UInt32    IOA_MODULE_ID               = 0;

        public static int       UDP_CHECK_POLLING_TIME      = -1;
        public static string    HOME_PATH_HTTP              = "";

        #region constructor
        static Config()
        {   
            PROCESS_TYPE = P_TYPE.FAS;
        }
        #endregion

        public static bool LoadDb()
        {
            if (!LoadDbCommon(DbModule.Instance))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "SYSTEM_GROUP", ref Config.SYSTEM_GROUP))
                Config.SYSTEM_GROUP = "90000";

            if (!ReadConfigDb_(DbModule.Instance, "CLIENT_ALIVE_INTERVAL", ref Config.CLIENT_ALIVE_INTERVAL)) 
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "CLIENT_LIMIT_RESPONSE_TIME", ref Config.CLIENT_LIMIT_RESPONSE_TIME)) 
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "SERVER_LIMIT_RESPONSE_TIME", ref Config.SERVER_LIMIT_RESPONSE_TIME)) 
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "MAX_CONENCTION", ref Config.MAX_CONENCTION)) 
                return false;

            if (Config.MAX_CONENCTION == -1) 
                Config.MAX_CONENCTION = 1000;

            if (!ReadConfigDb_(DbModule.Instance, "LISTEN_PORT", ref Config.LISTEN_PORT)) 
                return false;

            if (Config.LISTEN_PORT == -1) 
                Config.LISTEN_PORT = 1010;

            if (!ReadConfigDb_(DbModule.Instance, "BUFFER_SIZE", ref Config.BUFFER_SIZE)) 
                return false;

            if (Config.BUFFER_SIZE == -1) 
                Config.BUFFER_SIZE = 2048;

            if (!ReadConfigDb_(DbModule.Instance, "MAX_ACCEPT_OPERATION", ref Config.MAX_ACCEPT_OPERATION)) 
                return false;

            if (Config.MAX_ACCEPT_OPERATION == -1) 
                Config.MAX_ACCEPT_OPERATION = 10;

            if (!ReadConfigDb_(DbModule.Instance, "BACK_LOG", ref Config.BACK_LOG)) 
                return false;

            if (Config.BACK_LOG == -1) 
                Config.BACK_LOG = 100;

            if (!ReadConfigDb_(DbModule.Instance, "IOA_MODULE_ID", ref Config.IOA_MODULE_ID))
                Config.IOA_MODULE_ID = 589825;

            if (!ReadConfigDb_(DbModule.Instance, "UDP_CHECK_POLLING_TIME", ref Config.UDP_CHECK_POLLING_TIME)) 
                return false;

            if (Config.UDP_CHECK_POLLING_TIME == -1) 
                Config.UDP_CHECK_POLLING_TIME = 5000;

            if (!ReadConfigDb_(DbModule.Instance, "HOME_PATH_HTTP", ref Config.HOME_PATH_HTTP)) 
                return false;

            if (Config.HOME_PATH_HTTP == "")
                return false;

            return true;
        }
    }
}
