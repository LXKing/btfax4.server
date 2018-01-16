using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Runtime.InteropServices;

namespace DCV_Callback
{
    public class StringUtil
    {
        #region WIN32
        [DllImport("kernel32")]
        private static extern long WritePrivateProfileString(string section, string key, string val, string filePath);
        [DllImport("kernel32")]
        private static extern int GetPrivateProfileString(string section, string key, string def, StringBuilder retVal, int size, string filePath);
        #endregion

        #region static
        public static bool WritePrivateProfileStringEx(string p_strFileFullName, string p_strSection, string p_strField, string p_strValue)
        {
            long ret = WritePrivateProfileString(p_strSection, p_strField, p_strValue, p_strFileFullName);
            if (ret > 0)
                return true;

            return false;
        }

        public static string GetPrivateProfileStringEx(string p_strFileFullName, string p_strSection, string p_strField)
        {
            StringBuilder temp = new StringBuilder(255);
            int ret = GetPrivateProfileString(p_strSection, p_strField, "", temp, 255, p_strFileFullName);
            return temp.ToString();
        }

        private static bool IsKorean(char ch)
        {
            //( 한글자 || 자음 , 모음 )
            if ((0xAC00 <= ch && ch <= 0xD7A3) || (0x3131 <= ch && ch <= 0x318E))
                return true;
            else
                return false;
        }

        private static bool IsEnglish(char ch)
        {
            if ((0x61 <= ch && ch <= 0x7A) || (0x41 <= ch && ch <= 0x5A))
                return true;
            else
                return false;
        }

        private static bool IsNumeric(char ch)
        {
            if (0x30 <= ch && ch <= 0x39)
                return true;
            else
                return false;
        }

        private static bool IsAllowedCharacter(char ch)
        {
            return m_strAllowCharacters.Contains<char>(ch);
        }

        public static bool ContainsKorean(string p_str)
        {
            for (int i = 0; i < p_str.Length; i++)
            {
                if (IsKorean(p_str[i]))
                    return true;
            }

            return false;
        }

        /// <summary>
        /// 제목   : 유니크한파일명을 얻는다 -> 파일명(seq)
        /// 작성자 : KIMCG
        /// 작성일 : 2013.03.27
        /// </summary>
        public static string GetUniqueFileName(string p_filePath, string p_fileName)
        {
            string fileFullName = "";
            string uniqueFileName = p_fileName;
            int cnt = 0;
            while (true)
            {
                cnt++;
                fileFullName = string.Format("{0}\\{1}", p_filePath, uniqueFileName);
                if (!File.Exists(fileFullName))
                    break;

                int idx = p_fileName.LastIndexOf(".");
                if (idx < 0)
                    break;

                uniqueFileName = string.Format("{0}({1}).{2}", p_fileName.Substring(0, idx), cnt, p_fileName.Substring(idx + 1));
            }

            return uniqueFileName;
        }

        public static bool ContainsSpecialCharacters(string p_str)
        {
            for (int i = 0; i < p_str.Length; i++)
            {
                if (IsAllowedCharacter(p_str[i]))
                    return true;
            }

            return false;
        }

        public static DateTime ParseStringToDateTime(string p_yyyyMMddHHmmss)
        {
            string strDatetime = "";
            DateTime dateTime;
            try
            {
                strDatetime = string.Format("{0}-{1}-{2} {3}:{4}:{5}", p_yyyyMMddHHmmss.Substring(0, 4),
                                                                        p_yyyyMMddHHmmss.Substring(4, 2),
                                                                        p_yyyyMMddHHmmss.Substring(6, 2),
                                                                        p_yyyyMMddHHmmss.Substring(8, 2),
                                                                        p_yyyyMMddHHmmss.Substring(10, 2),
                                                                        p_yyyyMMddHHmmss.Substring(12, 2));

                if (!DateTime.TryParse(strDatetime, out dateTime))
                    dateTime = DateTime.Now;
            }
            catch
            {
                dateTime = DateTime.Now;
            }

            return dateTime;
        }

        public static DateTime ParseStringToDateTime2(string p_yyyyMMdd)
        {
            string strDatetime = "";
            DateTime dateTime;
            try
            {
                strDatetime = string.Format("{0}-{1}-{2}", p_yyyyMMdd.Substring(0, 4),
                                                                        p_yyyyMMdd.Substring(4, 2),
                                                                        p_yyyyMMdd.Substring(6, 2));

                if (!DateTime.TryParse(strDatetime, out dateTime))
                    dateTime = DateTime.Now;
            }
            catch
            {
                dateTime = DateTime.Now;
            }

            return dateTime;
        }
        #endregion

        #region field
        private const string m_strAllowCharacters = "!@#$%^&*()_+| ";
        #endregion
    }
}
