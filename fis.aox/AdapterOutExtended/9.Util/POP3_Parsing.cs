using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Btfax.CommonLib.Log;
using System.IO;

namespace AdapterOutExtended
{
    class POP3_Parsing
    {
        #region constructor && destructor
        public POP3_Parsing()
        {
            IsParseSuccess = false;
        }
        #endregion

        #region Retrieve
        public void MailMessage(string message)
        {
            m_MailMessage = message;
            m_MailMessageLines = message.Split(new string[] { "\r\n" }, StringSplitOptions.None);
        }
        #endregion

        #region Header
        public void ParseHeader()
        {
            string lastLine = "";
            int row = 0;

            string lastKey = "";
            string lastValue = "";
            bool isNew = false;

            m_HeaderStartRow = -1;
            foreach (string line in m_MailMessageLines)
            {
                if (row == 0 && line == "") continue;
                else if (line == "") break;

                if (m_HeaderStartRow == -1) m_HeaderStartRow = row;     /// 헤더 시작

                row += 1;

                lastLine = line;
                lastValue = "";
                isNew = false;

                if (line.Length > 0 &&
                    line.Substring(0, 1) != "\t" &&
                    line.Substring(0, 1) != " " &&
                    line.IndexOf(":") > 0)
                {
                    int lineIndex = line.IndexOf(":");

                    lastKey = line.Substring(0, lineIndex);
                    lastLine = line.Substring(lineIndex + 1);

                    isNew = true;
                }
                lastValue = DeleteSpecialCharacter(DecodingOneLine(lastLine));

                if (isNew == false &&
                    HeaderInfo.Count > 0 &&
                    HeaderInfo[HeaderInfo.Count].Keys.ToArray()[0] == lastKey)
                {
                    HeaderInfo[HeaderInfo.Count][lastKey] += lastValue;
                }
                else
                {
                    Dictionary<string, string> headerLine = new Dictionary<string, string>();
                    headerLine.Add(lastKey, lastValue);
                    HeaderInfo.Add(HeaderInfo.Count + 1, headerLine);
                }
                lastValue = HeaderInfo[HeaderInfo.Count][lastKey];
                ParseHeaderType(lastKey.ToUpper(), lastValue);      /// 헤더 정보 분석
            }
            m_HeaderEndRow = row;   /// 헤더 종료
        }
       
        private void ParseHeaderType(string key, string value)
        {
            if (key == "MIME-VERSION") MIMEVersion = value;
            else if (key == "RETURN-PATH") ReturnPath = value;
            else if (key == "DATE") Date = value;
            else if (key == "FROM") From = value;
            else if (key == "REPLY-TO") ReplyTo = value;
            else if (key == "TO") To = value;
            else if (key == "MESSAGE-ID") MessageID = value;
            else if (key == "SUBJECT") Subject = value;
            else if (key == "CONTENT-TYPE")
            {
                string[] split = value.Split(';');

                ContentType = split[0];
                ContentTypeMain = split[0].Split('/')[0];
                ContentTypeSub = split[0].Split('/')[1];

                if (split.Length == 2 && split[1] != "")
                {
                    //split[1]
                    int lineIndex = split[1].IndexOf('=');

                    string subKey = split[1].Substring(0, lineIndex).Trim();
                    string subValue = split[1].Substring(lineIndex + 1).Trim();

                    if (subKey.ToUpper() == "BOUNDARY") m_BoundaryID = subValue;
                }
            }
        }
        #endregion

        #region Body
        public void ParseBody()
        {
            if (string.Compare(ContentTypeMain, "multipart", true) == 0)
            {
                ParseBodyBoundary(m_BoundaryID, m_HeaderEndRow);
            }
        }

        private int ParseBodyBoundary(string startBoundaryID, int startRow, bool isAlternative = false)
        {
            string boundaryID = startBoundaryID;
            int row = startRow;
            int resultRow = 0;

            for (; row < m_MailMessageLines.Length; row++)
            {
                resultRow = FindStringInRow(string.Format("--{0}", boundaryID), row);
                if (resultRow == -1)
                {
                    resultRow = FindStringInRow(string.Format("--{0}--", boundaryID), row);
                    return resultRow;
                }
                POP3_BoundaryInfo bi = ParseBoundaryHeader(boundaryID, resultRow);
                BoundaryInfoList.Add(bi);

                resultRow = bi.HeaderEndPosition;

                /// child 여부 확인
                if (bi.ChildBoundaryID == "")
                {
                    resultRow = FindStringInRow("", resultRow, false);

                    bi.BodyStartPosition = resultRow;

                    for (; resultRow < m_MailMessageLines.Length; resultRow++)
                    {
                        if (bi.BodyEndPosition < 0)
                        {
                            if (m_MailMessageLines[resultRow].IndexOf("--") == 0 ||
                                m_MailMessageLines[resultRow] == "")
                            {
                                bi.BodyEndPosition = resultRow - 1;
                            }
                        }
                        if (m_MailMessageLines[resultRow].IndexOf("--") == 0)
                        {
                            row = resultRow - 1;

                            if (isAlternative == true)
                                return row;
                            else
                                break;
                        }
                    }
                }
                else
                {
                    int parseCount = 0;
                    for (; resultRow < m_MailMessageLines.Length; resultRow++)
                    {
                        if (m_MailMessageLines[resultRow] == string.Format("--{0}", bi.ChildBoundaryID))
                        {
                            resultRow = ParseBodyBoundary(bi.ChildBoundaryID, resultRow, true);

                            if ((string.Compare(bi.ContentTypeSub, "alternative", true) == 0) &&
                                (++parseCount < 2))
                            {
                                continue;
                            }

                            row = resultRow;
                            break;
                        }
                    }
                }
            }
            return row;
        }

        public POP3_BoundaryInfo ParseBoundaryHeader(string boundaryID, int headerStartRow)
        {
            string lastLine = "";
            int row = 0;

            POP3_BoundaryInfo bi = new POP3_BoundaryInfo();
            bi.BoundaryID = boundaryID;
            bi.HeaderStartPosition = headerStartRow;

            for (row = headerStartRow; row < m_MailMessageLines.Length; row++)
            {
                string line = m_MailMessageLines[row];

                if (line == "") break;

                if (line.Length > 0 &&
                    line.Substring(0, 1) != "\t" &&
                    line.Substring(0, 1) != " " &&
                    line.IndexOf(":") > 0)
                {
                    lastLine = line;
                }
                else
                {
                    lastLine += line;
                }
                bi.BoundaryHeaderInfo.Add(lastLine);        /// 헤더 정보 추가
                ParseBoundaryHeaderType(bi, lastLine);      /// 헤더 정보 분석
            }
            bi.HeaderEndPosition = row;

            /// 첨부파일 여부 확인
            if ( (string.Compare(bi.ContentTypeMain, "image", true) == 0 || string.Compare(bi.ContentTypeMain, "application", true) == 0) &&
                 (bi.ContentDispositionAttachFileName.Length > 0 || bi.ContentTypeName.Length > 0) )
            {
                    bi.IsFileAttach = true;
            }
            return bi;
        }

        private void ParseBoundaryHeaderType(POP3_BoundaryInfo bi, string headerType)
        {
            string[] splitLines = headerType.Split(';');

            foreach (string line in splitLines)
            {
                Match m = Regex.Match(line, @"(?<key>[\wd\-<>@+=./?]+)[ ]*[:=][ ]*(?<value>[\w\d\-=_/.,: ()""?+]+)");
                if (m.Success == true)
                {
                    string key = m.Groups["key"].Value.ToUpper();
                    string value = DeleteSpecialCharacter(DecodingOneLine(m.Groups["value"].Value));

                    if (key == "CONTENT-TYPE")
                    {
                        bi.ContentType = value;
                        bi.ContentTypeMain = value.Split('/')[0];
                        bi.ContentTypeSub = value.Split('/')[1];
                    }
                    else if (key == "CHARSET") bi.ContentTypeCharSet = value;
                    else if (key == "BOUNDARY") bi.ChildBoundaryID = value;
                    else if (key == "NAME") { bi.ContentTypeName = value; bi.IsFileAttach = true; }

                    else if (key == "CONTENT-TRANSFER-ENCODING") bi.ContentTransferEncoding = value;

                    else if (key == "CONTENT-DISPOSITION") bi.ContentDisposition = value;
                    else if (key == "FILENAME") { bi.ContentDispositionAttachFileName = value; bi.IsFileAttach = true; }
                }
            }
        }
        #endregion

        #region After Parse Header & Body
        public void ConvertStringBoundaryBody()
        {
            foreach (POP3_BoundaryInfo bi in BoundaryInfoList.ToArray())
            {
                if (bi.BodyStartPosition == 0 && bi.BodyEndPosition == 0) continue;

                if (string.Compare(bi.ContentTypeMain, "text", true) == 0)
                {
                    if (bi.ContentTypeCharSet.Length > 0)
                    {
                        bi.Encoding = Encoding.GetEncoding(bi.ContentTypeCharSet);
                    }
                    else
                    {
                        bi.Encoding = Encoding.Default;
                    }
                }
                else
                {
                    bi.Encoding = null;
                }

                /// 헤더 파싱
                if (string.Compare(bi.ContentTransferEncoding, "base64", true) == 0)
                {
                    if (bi.Encoding == null)
                    {
                        /// 첨부파일 -> Local File (ConvertBase64ToFile() 사용)
                    }
                    else
                    {
                        bi.ConvertBody = ConvertBase64ToString(GetBodyLines(bi));
                    }
                }
                else if (string.Compare(bi.ContentTransferEncoding, "quoted-printable", true) == 0)
                {
                    m_Encoding = bi.Encoding;
                    bi.ConvertBody = ConvertQuotedPrintableToString(GetBodyLines(bi));
                }
            }
        }

        public string ConvertBase64ToFile(string localPath, POP3_BoundaryInfo bi)
        {
            if (bi.IsFileAttach == false) return "";

            if (string.Compare(bi.ContentTransferEncoding, "base64", true) != 0) return "";

            if (bi.Encoding != null) return "";
                
            string fileName = bi.RepresentationFilename;
            if (fileName.Length == 0) return "";



            if (Directory.Exists(localPath) == false) Directory.CreateDirectory(localPath);
            string filePath = string.Format("{0}{1}", localPath, fileName);
            
            try
            {
                byte[] data = ConvertBase64ToByteArray(GetBodyLines(bi));

                using (FileStream fs = new FileStream(filePath, FileMode.Create, FileAccess.Write))
                {
                    BinaryWriter bw = new BinaryWriter(fs);
                    bw.Write(data);
                    bw.Flush();
                }
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, "파일 생성에 실패했습니다.");
                filePath = "";
            }
            return filePath;
        }
        #endregion

        #region Parse Method
        static public Encoding m_Encoding
        {
            get;
            set;
        }

        private string ReplaceFunc(System.Text.RegularExpressions.Match m)
        {
            byte[] b = new byte[m.Groups[0].Value.Length / 3];
            for (int i = 0; i < m.Groups[0].Value.Length / 3; i++)
            {
                b[i] = byte.Parse(m.Groups[0].Value.Substring(i * 3 + 1, 2), System.Globalization.NumberStyles.AllowHexSpecifier);
            }
            return m_Encoding.GetString(b);
        }

        public string DecodingOneLine(string content)
        {
            string convertValue = "";
            string returnValue = "";

            MatchCollection mc = Regex.Matches(content, @"[=][?](?<encoding>[\w\d-]+)[?](?<charset>[\w]+)[?](?<value>[\w\d\-+=./<>@?]+)[?][=]");
            if (mc.Count == 0)
            {
                returnValue = content;
            }
            else
            {
                foreach (Match m in mc)
                {
                    try
                    {
                        m_Encoding = Encoding.GetEncoding(m.Groups["encoding"].Value);

                        /// 헤더 파싱
                        if (m.Groups["charset"].Value == "B")
                        {
                            convertValue = ConvertBase64ToString(m.Groups["value"].Value);
                        }
                        else if (m.Groups["charset"].Value == "Q")
                        {
                            convertValue = ConvertQuotedPrintableToString(m.Groups["value"].Value);
                        }
                        returnValue += Regex.Replace(content, @"[=][?].*[?][=]", convertValue);
                    }
                    catch (Exception ex)
                    {
                        AppLog.Write(LOG_LEVEL.ERR, string.Format("Encoding 문자가 잘못되었습니다. 인코딩 문자 : {0}", m.Groups["encoding"].Value, ex.ToString()));
                    }
                }
            }
            return returnValue;
        }

        /// <summary>
        /// Base64 -> byte[] -> string
        /// </summary>
        public byte[] ConvertBase64ToByteArray(string content)
        {
            return Convert.FromBase64String(content);
        }
        public byte[] ConvertBase64ToByteArray(string[] contents)
        {
            return ConvertBase64ToByteArray(string.Join("", contents));
        }
        public string ConvertBase64ToString(string content)
        {
            return m_Encoding.GetString(ConvertBase64ToByteArray(content));
        }
        public string ConvertBase64ToString(string[] contents)
        {
            return ConvertBase64ToString(string.Join("", contents));
        }

        /// <summary>
        /// quoted-printable -> string
        /// </summary>
        public string ConvertQuotedPrintableToString(string content)
        {
            return ConvertQuotedPrintableToString(new string[] { content });
        }
        public string ConvertQuotedPrintableToString(string[] contents)
        {
            string convertString = "";
            string removeSoftLineBreak = "";

            foreach (string line in contents)
            {
                string trimLine = line.Trim();

                if (trimLine.Length > 0)
                {
                    if (trimLine.Substring(trimLine.Length - 1) == "=")
                    {
                        removeSoftLineBreak += line.Remove(trimLine.Length - 1);
                        continue;
                    }
                    else
                    {
                        removeSoftLineBreak += line;
                    }
                    convertString += Regex.Replace(removeSoftLineBreak, @"(?<value>(?:=[0-9A-Z]{2}){1,})", ReplaceFunc);
                    convertString += "\r\n";

                    removeSoftLineBreak = "";
                }
            }
            return convertString;
        }
        #endregion

        #region util
        public string DeleteSpecialCharacter(string content)
        {
            return content.Trim().Replace("\t", "").Replace("\"", "");
        }

        private int FindStringInRow(string findString, int startRow, bool findCorrect = true)
        {
            for (int row = startRow; row < m_MailMessageLines.Length; row++)
            {
                if (findCorrect == true)
                {
                    if (m_MailMessageLines[row] == findString) return row;
                }
                else
                {
                    if (m_MailMessageLines[row] != findString) return row;
                }
            }
            return -1;
        }

        private string[] GetBodyLines(POP3_BoundaryInfo bi)
        {
            return GetBodyLines(bi.BodyStartPosition, bi.BodyEndPosition);
        }
        private string[] GetBodyLines(int start, int end)
        {
            if (m_MailMessageLines.Length < start || m_MailMessageLines.Length < end)
                return new string[] { "" };
            else
                return m_MailMessageLines.Where((line, index) => index >= start && index <= end).ToArray();
        }
        #endregion

        #region fields
        // public
        public List<POP3_BoundaryInfo> BoundaryInfoList = new List<POP3_BoundaryInfo>();
        public Dictionary<int, Dictionary<string, string>> HeaderInfo = new Dictionary<int, Dictionary<string, string>>();
        public bool IsParseSuccess { get; set; }
        public string MIMEVersion { get; set; }
        public string ReturnPath { get; set; }
        public string Date { get; set; }
        public string From { get; set; }
        public string ReplyTo { get; set; }
        public string To { get; set; }
        public string MessageID { get; set; }
        public string Subject { get; set; }
        public string ContentType { get; set; }
        public string ContentTypeMain { get; set; }
        public string ContentTypeSub { get; set; }

        // private
        private string m_BoundaryID { get; set; }
        private string m_MailMessage { get; set; }
        private string[] m_MailMessageLines { get; set; }
        private int m_HeaderStartRow { get; set; }
        private int m_HeaderEndRow { get; set; }
        private int m_BodyStartRow { get; set; }
        private int m_BodyEndRow { get; set; }
        #endregion
    }
}
