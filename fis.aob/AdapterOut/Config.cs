using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Windows.Forms;
using Btfax.CommonLib;
using Btfax.CommonLib.Xml;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.MsgBox;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Config;
using AdapterOut.Db;

namespace AdapterOut.Util
{   
    class Config : ConfigBase
    {
        //// DB 항목 ////
        public static string    SYSTEM_GROUP           = "";
        public static int       INITIAL_SLEEP          = 0; // [ms]
        public static int       LISTEN_PORT            = 0;
        public static int       SESSION_CNT            = 0;
        public static int       SIZE_FIELD_POS         = 0;
        public static int       SIZE_FIELD_LEN         = 0;
        public static int       TRNO_FIELD_POS         = 0;
        public static int       TRNO_FIELD_LEN         = 0;

        public static string    STG_HOME_PATH          = "";
        public static string    FINISHED_TIF_PATH      = "";
        public static string    PACKET_XML_PATH        = "";
        public static string    PACKET_XML_FILE        = "";
		public static string	ENCRYPT_FIELD_YN	   = "N";
		public static string	ENCRYPT_DLL_FILE	   = "";
        public static string    ENABLE_FAXBOX          = "";
		public static long		TIF_FILE_MAX_SIZE	   = 0;	 // 즉시발송유형에서 tif파일 크기 제한. 0일경우 체크안함.
        public static string    RECOVERY_YN            = ""; // 장애발생시 리커버리 쓰레드 수행여부
        public static int       RECOVERY_BASE_TIME     = 10; // miniute - 장애발생시 리커버리 요청 기준시간 : 기본값 10분

        public static string    INPUT_DOCS_PATH        = "";
        public static UInt32    IOA_MODULE_ID          = 0;

        public static string    SEND_SITE_TABLE        = "";
        public static string    SEND_SITE_IDFIELD      = "";
        public static string    SEND_SITE_DTL_TABLE    = "";
        public static string    SEND_SITE_DTL_SEQFIELD = "";
        public static string    SEND_SITE_DTL_SEQ      = "";
		public static string	DELETE_INPUT_FILE_YN   = "N";
        public static Dictionary<string, List<DbModule.SST_COLUMN_MAP>> SST_MAP = new Dictionary<string,List<DbModule.SST_COLUMN_MAP>>();         // SendSiteTable Map ( TrNo : Column Map )
        public static Dictionary<string, List<DbModule.SST_COLUMN_MAP>> SST_MAP_header = new Dictionary<string,List<DbModule.SST_COLUMN_MAP>>();  // SendSiteTable Map ( TrNo : Column Map ) : 전문헤더
        
        #region constructor
        static Config()
        {   
            PROCESS_TYPE = P_TYPE.AOB;

            //// 추가 항목 //////////
            SST_MAP        = new Dictionary<string, List<DbModule.SST_COLUMN_MAP>>();
            SST_MAP_header = new Dictionary<string, List<DbModule.SST_COLUMN_MAP>>();
        }
        #endregion
        
        public static bool LoadDb()
        {
            string strTemp = "";

            if (!LoadDbCommon(DbModule.Instance))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "SYSTEM_GROUP", ref Config.SYSTEM_GROUP))
                Config.SYSTEM_GROUP = "90000";

            if (!ReadConfigDb_(DbModule.Instance, "INITIAL_SLEEP", ref Config.INITIAL_SLEEP))
                return false;
            
            if (!ReadConfigDb_(DbModule.Instance, "LISTEN_PORT", ref Config.LISTEN_PORT))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "SESSION_CNT", ref Config.SESSION_CNT))
                return false;
            
            if (!ReadConfigDb_(DbModule.Instance, "SIZE_FIELD_POS", ref Config.SIZE_FIELD_POS))
                return false;
            
            if (!ReadConfigDb_(DbModule.Instance, "SIZE_FIELD_LEN", ref Config.SIZE_FIELD_LEN))
                return false;

            if (!ReadConfigDb_(DbModule.Instance, "TRNO_FIELD_POS", ref Config.TRNO_FIELD_POS))
                return false;
            
            if (!ReadConfigDb_(DbModule.Instance, "TRNO_FIELD_LEN", ref Config.TRNO_FIELD_LEN))
                return false;

			if (!ReadConfigDb_(DbModule.Instance, "STG_HOME_PATH", ref Config.STG_HOME_PATH))
				return false;

			if (!ReadConfigDb_(DbModule.Instance, "LOG_HOME_PATH", ref Config.LOG_HOME_PATH))
				Config.LOG_HOME_PATH = "";

            if (!ReadConfigDb_(DbModule.Instance, "IOA_MODULE_ID", ref Config.IOA_MODULE_ID))
                Config.IOA_MODULE_ID = 589825;

            if (!ReadConfigDb_(DbModule.Instance, "FINISHED_TIF_PATH", ref strTemp))
                return false;
            Config.FINISHED_TIF_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadConfigDb_(DbModule.Instance, "PACKET_XML_PATH", ref strTemp))
                return false;
            Config.PACKET_XML_PATH = string.Format("{0}\\{1}", Config.STG_HOME_PATH, strTemp);

            if (!ReadConfigDb_(DbModule.Instance, "PACKET_XML_FILE", ref Config.PACKET_XML_FILE))
                return false;

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

            if (!ReadConfigDb_(DbModule.Instance, "ENABLE_FAXBOX", ref Config.ENABLE_FAXBOX))
            {   
                Config.ENABLE_FAXBOX = "Y";
                AppLog.Write(Btfax.CommonLib.Log.LOG_LEVEL.WRN, "ENABLE_FAXBOX 항목을 읽지 못하여 기본값 'Y' 로 설정합니다.");
            }

			if (!ReadConfigDb_(DbModule.Instance, "TIF_FILE_MAX_SIZE", ref Config.TIF_FILE_MAX_SIZE))
            {
				Config.TIF_FILE_MAX_SIZE = 0;
				AppLog.Write(Btfax.CommonLib.Log.LOG_LEVEL.WRN, "TIF_FILE_MAX_SIZE 항목을 읽지 못하여 기본값 0 로 설정합니다.");
            }

            if (!ReadConfigDb_(DbModule.Instance, "RECOVERY_YN", ref Config.RECOVERY_YN))
            {
                Config.RECOVERY_YN = "Y";
                AppLog.Write(Btfax.CommonLib.Log.LOG_LEVEL.WRN, "RECOVERY_YN 항목을 읽지 못하여 기본값 'Y' 로 설정합니다.");
            }

            if (!ReadConfigDb_(DbModule.Instance, "RECOVERY_BASE_TIME", ref Config.RECOVERY_BASE_TIME))
            {
                Config.RECOVERY_BASE_TIME = 10;
                AppLog.Write(Btfax.CommonLib.Log.LOG_LEVEL.WRN, "RECOVERY_BASE_TIME 항목을 읽지 못하여 기본값 '10' 로 설정합니다.");
            }

			if (!ReadConfigDb_(DbModule.Instance, "DELETE_INPUT_FILE_YN", ref Config.DELETE_INPUT_FILE_YN))
				Config.DELETE_INPUT_FILE_YN = "N";

            if (ReadConfigDb_(DbModule.Instance, "SEND_SITE_TABLE", ref Config.SEND_SITE_TABLE))
            {
                if (!ReadConfigDb_(DbModule.Instance, "SEND_SITE_IDFIELD", ref Config.SEND_SITE_IDFIELD))
                    return false;
            }

            if (ReadConfigDb_(DbModule.Instance, "SEND_SITE_DTL_TABLE", ref Config.SEND_SITE_DTL_TABLE))
            {
                if (!ReadConfigDb_(DbModule.Instance, "SEND_SITE_DTL_SEQFIELD", ref Config.SEND_SITE_DTL_SEQFIELD))
                    return false;
                if (!ReadConfigDb_(DbModule.Instance, "SEND_SITE_DTL_SEQ", ref Config.SEND_SITE_DTL_SEQ))
                    return false;
            }

            if (!ReadConfigDb_(DbModule.Instance, "INPUT_DOCS_PATH", ref Config.INPUT_DOCS_PATH))
                return false;

            if (!DbModule.Instance.GetSstMapInfo(ref SST_MAP_header, ref SST_MAP))
            {
                DisplayAndWrite_("DB", 'R', LOG_LEVEL.ERR, "BTF_FAX_SEND_SITE_MAP", "읽지 못함", true);
                return false;
            }

            return true;
        }
    }
}
