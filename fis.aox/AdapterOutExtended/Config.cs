using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Text.RegularExpressions;
using Btfax.CommonLib;
using Btfax.CommonLib.Xml;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.MsgBox;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Config;

namespace AdapterOutExtended
{
    public class FileCheckInfo
    {
        public int seq
        {
            get;
            set;
        }
        public string key
        {
            get;
            set;
        }
        public int length
        {
            get;
            set;
        }
        public int startPosition
        {
            get;
            set;
        }
    }

    enum FILE_CHECK_TYPE_Enum
    {
        F1,
        F2
    }

    class Config : ConfigBase
    {
        #region const
        /// <summary>
        /// AOX Config File 항목
        /// </summary>
        public static FILE_CHECK_TYPE_Enum FILE_CHECK_TYPE = FILE_CHECK_TYPE_Enum.F1;
        public static string FILE_UPLOAD_PATH = "";
        public static string FILE_PROCESS_PATH = "";
        public static string FILE_EXCEPTION_PATH = "";
        public static string FILE_TIF_FILE_FORMAT = "";
        public static string FILE_TIF_FILE_REAL_FORMAT = "";    /// 자리수가 계산된 실제 확인용 포맷
        public static string FILE_DELIMITER = "";
        public static string FILE_IMAGE_FILE_EXT = "";      /// 업로드할 tif 이미지 확장자명 (tif or tiff)
        public static string FILE_INFO_FILE_EXT = "";
        public static int FILE_PROCESS_COUNT = 0;
        public static string EMAIL_POP_SERVER = "";
        public static int EMAIL_POP3_SERVER_PORT = 0;
        public static string EMAIL_ADDR = "";
        public static string EMAIL_ID = "";
        public static string EMAIL_PW = "";
        public static string EMAIL_TITLE_FORMAT = "";
        public static string EMAIL_TITLE_REAL_FORMAT = "";    /// 자리수가 계산된 실제 확인용 포맷
        public static int SOTRAGE_POLLING_TIME = 0;
        public static int EMAIL_POLLING_TIME = 0;
        public static string EMAIL_DELIMITER = "";

        public static string FILE_CHECK_EXT = "";       /// 확인용 확장자
        public static string FILE_WORK_EXT = "";        /// 작업용 확장자

        public static List<FileCheckInfo> FileTifFileFormatInfo = new List<FileCheckInfo>();

        public static List<FileCheckInfo> EmailTitleFormatInfo = new List<FileCheckInfo>();

        /// <summary>
        /// Tif 파일 포맷 가변/고정 여부
        /// F:고정,   V:가변 (구분자 사용)
        /// </summary>
        public static string FILE_IsFixOrVariable = "V";

        /// <summary>
        /// Title 가변/고정 여부
        /// F:고정,   V:가변 (구분자 사용)
        /// </summary>
        public static string EMAIL_IsFixOrVariable = "V";

        // DB 항목

        public static string FTP_HOME_PATH = "";
        public static string FINISHED_TIF_PATH = "";
        
        // 커스터마이징 항목
        public static string INPUT_DOCS_PATH = "";
        public static string AOX_INPUT_DOCS_PATH = "";
        #endregion

        #region constructor
        static Config()
        {  
            PROCESS_TYPE = P_TYPE.AOX;
        }
        #endregion

        public static bool LoadDb()
        {
            string strTemp = "";

            if (!LoadDbCommon(DbModule.Instance))
                return false;

            // Enum 파싱

            if (!ReadConfigDb_(DbModule.Instance, "STG_HOME_PATH", ref Config.FTP_HOME_PATH)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "FINISHED_TIF_PATH", ref strTemp)) return false;
            Config.FINISHED_TIF_PATH = string.Format("{0}{1}", Config.FTP_HOME_PATH, strTemp);

            if (!ReadConfigDb_(DbModule.Instance, "INPUT_DOCS_PATH", ref Config.INPUT_DOCS_PATH)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "AOX_INPUT_DOCS_PATH", ref Config.AOX_INPUT_DOCS_PATH)) return false;

            if (!ReadConfigDb_(DbModule.Instance, "FILE_CHECK_TYPE", ref strTemp)) return false;
            // Enum 파싱
            FILE_CHECK_TYPE_Enum tryFileCheckType = FILE_CHECK_TYPE_Enum.F1;
            if (!Enum.TryParse(strTemp, out tryFileCheckType)) return false;
            Config.FILE_CHECK_TYPE = tryFileCheckType;

            if (!ReadConfigDb_(DbModule.Instance, "FILE_UPLOAD_PATH", ref Config.FILE_UPLOAD_PATH)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "FILE_PROCESS_PATH", ref Config.FILE_PROCESS_PATH)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "FILE_EXCEPTION_PATH", ref Config.FILE_EXCEPTION_PATH)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "FILE_TIF_FILE_FORMAT", ref Config.FILE_TIF_FILE_FORMAT)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "FILE_DELIMITER", ref Config.FILE_DELIMITER)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "FILE_IMAGE_FILE_EXT", ref Config.FILE_IMAGE_FILE_EXT)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "FILE_INFO_FILE_EXT", ref Config.FILE_INFO_FILE_EXT)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "FILE_PROCESS_COUNT", ref Config.FILE_PROCESS_COUNT)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "EMAIL_POP_SERVER", ref Config.EMAIL_POP_SERVER)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "EMAIL_POP3_SERVER_PORT", ref Config.EMAIL_POP3_SERVER_PORT)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "EMAIL_ADDR", ref Config.EMAIL_ADDR)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "EMAIL_ID", ref Config.EMAIL_ID)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "EMAIL_PW", ref Config.EMAIL_PW)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "EMAIL_TITLE_FORMAT", ref Config.EMAIL_TITLE_FORMAT)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "EMAIL_DELIMITER", ref Config.EMAIL_DELIMITER)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "SOTRAGE_POLLING_TIME", ref Config.SOTRAGE_POLLING_TIME)) return false;
            if (!ReadConfigDb_(DbModule.Instance, "EMAIL_POLLING_TIME", ref Config.EMAIL_POLLING_TIME)) return false;
            
            /// Tif File 명칭 포맷 정의 (FILE_TIF_FILE_FORMAT 참고)
            if (Config.ParseTifFileFormat() == false) return false;

            /// Email Title 명칭 포맷 정의 (EMAIL_TITLE_FORMAT 참고)
            if (Config.ParseEmailTitleFormat(Config.EMAIL_TITLE_FORMAT) == false) return false;

            /// 작업용, 확인용 확장자 설정
            if (Config.FILE_CHECK_TYPE == FILE_CHECK_TYPE_Enum.F1)
            {
                Config.FILE_CHECK_EXT = Config.FILE_IMAGE_FILE_EXT.ToUpper();   /// TIF
                Config.FILE_WORK_EXT = Config.FILE_IMAGE_FILE_EXT.ToUpper();    /// TIF
            }
            else if (Config.FILE_CHECK_TYPE == FILE_CHECK_TYPE_Enum.F2)
            {
                Config.FILE_CHECK_EXT = Config.FILE_INFO_FILE_EXT.ToUpper();    /// DAT
                Config.FILE_WORK_EXT = Config.FILE_IMAGE_FILE_EXT.ToUpper();    /// TIF
            }

            return true;
        }

        /// <summary>
        /// Tif 파일 포맷 정의
        /// </summary>
        protected static bool ParseTifFileFormat()
        {
            /// 초기화
            Config.FileTifFileFormatInfo.Clear();

            /// 유효성 확인
            if (Config.FILE_TIF_FILE_FORMAT.Trim().Length == 0) return false;

            /// 가변/고정 판별 정규식
            string strFixRx = @"(?<key>[@][a-zA-Z]+)(?<value>[(][\d]+[)])";
            string strVariableRx = @"(?<key>[@][a-zA-Z]+)";

            /// 확장자 제거
            string strTemp = Config.FILE_TIF_FILE_FORMAT.Substring(0, Config.FILE_TIF_FILE_FORMAT.LastIndexOf("."));

            /// 정규식 일치 집합
            MatchCollection matchesFix = Regex.Matches(strTemp, strFixRx);
            MatchCollection matchesVariable = Regex.Matches(strTemp, strVariableRx);

            /// 유효성 확인
            if (matchesFix.Count == 0 && matchesVariable.Count == 0) return false;

            // 고정/가변 여부 확인
            if (matchesFix.Count == 0)
            {
                // 가변
                Config.FILE_IsFixOrVariable = "V";

                int i = 0;
                foreach (Match m in matchesVariable)
                {
                    FileCheckInfo item = new FileCheckInfo();
                    item.seq = i++;
                    item.key = m.Groups["key"].Value;
                    Config.FileTifFileFormatInfo.Add(item);
                }
            }
            else 
            {
                // 섞여있는 경우 확인
                if (matchesFix.Count == matchesVariable.Count)
                {
                    // 고정
                    Config.FILE_IsFixOrVariable = "F";

                    Config.FILE_TIF_FILE_REAL_FORMAT = Config.FILE_TIF_FILE_FORMAT;
                    int i = 0;
                    foreach (Match m in matchesFix)
                    {
                        FileCheckInfo item = new FileCheckInfo();
                        item.seq = i++;
                        item.key = m.Groups["key"].Value;
                        item.length = int.Parse(m.Groups["value"].Value.Replace("(", "").Replace(")", ""));
                        
                        /// 실질적인 위치
                        item.startPosition = Config.FILE_TIF_FILE_REAL_FORMAT.IndexOf(m.Groups["key"].Value + m.Groups["value"].Value);
                        
                        /// 실질적인 위치를 위한 Tif File 명칭 포맷 수정
                        Config.FILE_TIF_FILE_REAL_FORMAT = Config.FILE_TIF_FILE_REAL_FORMAT.Replace(m.Groups["key"].Value + m.Groups["value"].Value,
                                                                                                    "".PadLeft(item.length, '^'));
                        Config.FileTifFileFormatInfo.Add(item);
                    }
                }
                else
                {
                    // 에러 (가변과 고정형식이 섞여있음)
                    return false;
                }
            }

            return true;
        }

        /// <summary>
        /// Title 포맷 정의
        /// </summary>
        protected static bool ParseEmailTitleFormat(string formatString)
        {
            /// 초기화
            Config.EmailTitleFormatInfo.Clear();

            /// 유효성 확인
            if (formatString.Trim().Length == 0) return false;

            /// 가변/고정 판별 정규식
            string strFixRx = @"(?<key>[@][a-zA-Z]+)(?<value>[(][\d]+[)])";
            string strVariableRx = @"(?<key>[@][a-zA-Z]+)";

            ///// 확장자 제거
            //string strTemp = formatString.Substring(0, formatString.LastIndexOf("."));
            string strTemp = formatString;

            /// 정규식 일치 집합
            MatchCollection matchesFix = Regex.Matches(strTemp, strFixRx);
            MatchCollection matchesVariable = Regex.Matches(strTemp, strVariableRx);

            /// 유효성 확인
            if (matchesFix.Count == 0 && matchesVariable.Count == 0) return false;

            // 고정/가변 여부 확인
            if (matchesFix.Count == 0)
            {
                // 가변
                Config.EMAIL_IsFixOrVariable = "V";

                int i = 0;
                foreach (Match m in matchesVariable)
                {
                    FileCheckInfo item = new FileCheckInfo();
                    item.seq = i++;
                    item.key = m.Groups["key"].Value;
                    Config.EmailTitleFormatInfo.Add(item);
                }
            }
            else
            {
                // 섞여있는 경우 확인
                if (matchesFix.Count == matchesVariable.Count)
                {
                    //// 고정
                    //Config.EMAIL_IsFixOrVariable = "F";

                    Config.EMAIL_TITLE_FORMAT = formatString;
                    int i = 0;
                    foreach (Match m in matchesFix)
                    {
                        FileCheckInfo item = new FileCheckInfo();
                        item.seq = i++;
                        item.key = m.Groups["key"].Value;
                        item.length = int.Parse(m.Groups["value"].Value.Replace("(", "").Replace(")", ""));

                        /// 실질적인 위치
                        item.startPosition = Config.EMAIL_TITLE_REAL_FORMAT.IndexOf(m.Groups["key"].Value + m.Groups["value"].Value);

                        /// 실질적인 위치를 위한 Tif File 명칭 포맷 수정
                        Config.EMAIL_TITLE_REAL_FORMAT = Config.EMAIL_TITLE_FORMAT.Replace(m.Groups["key"].Value + m.Groups["value"].Value,
                                                                                           "".PadLeft(item.length, '^'));
                        Config.EmailTitleFormatInfo.Add(item);
                    }
                }
                else
                {
                    // 에러 (가변과 고정형식이 섞여있음)
                    return false;
                }
            }

            return true;
        }
    }
}
