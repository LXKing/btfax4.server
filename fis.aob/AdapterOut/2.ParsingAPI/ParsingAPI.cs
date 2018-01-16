using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using Btfax.CommonLib;
using Btfax.CommonLib.Util;

namespace AdapterOut.PacketParsing
{
    public class PacketParser
    {
        #region CONSTANT
        readonly string ROOTNAME = "/PacketLayout/";
        #endregion
        
        #region TYPE
        public class FieldInfo
        {
            public string strType;
            public string strName;
            public string strFullName;
            public string strValue;
            // ADD - KCG :20110524
            public string strAttribute;
            // ADD - END

            public List<FieldInfo> childInfos;

            public FieldInfo()
            {
                Clear();
            }

            public void Clear()
            {
                this.strType = "";
                this.strName = "";
                this.strFullName = "";
                this.strValue = "";
                // ADD - KCG :20110524
                this.strAttribute = "";
                // ADD - END
                this.childInfos = null;
            }
        }
        
        #endregion

        #region PROPERTY
        public string LayoutPathFile 
        {
            get { return m_layoutPathFile; }
            set { m_layoutPathFile = value; }
        }
        public bool Result
        {
            get { return m_result; }
        }
        public RESULT ResultCode
        {
            get { return m_resultCode; }
        }
        public string ResultMsg
        {
            get { return m_resultMsg; }
        }

        public string ReasonMsg
        {
            get { return m_reasonMsg; }
        }

        #region CUSTOMIZE - KIMCG : 20111214
        public List<FieldInfo> FieldInfos
        {
            get { return this.m_fieldInfos; }
        }
        #endregion

        #endregion

        #region METHOD - Parsing By PacketLayout
        public PacketParser()
        {
        }

        public bool Load()
        {
            if (IsLoaded())
                Unload();

            ClearFields_();
            ClearError_();
            try
            {
                this.m_xmlDoc.Load(this.LayoutPathFile);
                this.m_xmlRootElement = this.m_xmlDoc.DocumentElement;
            }
            catch (Exception ex)
            {
                SetError_(string.Format("전문XML파일({0}) 로딩 실패", this.LayoutPathFile), ex);
                return false;
            }

            return true;
        }

        public void Unload()
        {
            this.m_xmlRootElement = null;
            this.m_xmlDoc.RemoveAll();

            ClearFields_();

            this.m_xmlDocData.RemoveAll();
        }

        public bool IsLoaded()
        {
            return (this.m_xmlRootElement != null);
        }

        public bool Parse(string p_strTrNo, byte[] p_packet)
        {
            if (!IsLoaded())
                return false;

            XmlElement xmlPacketElement = this.m_xmlRootElement.SelectSingleNode(ROOTNAME+p_strTrNo) as XmlElement;
            if (xmlPacketElement == null)
            {
                SetError_(string.Format("전문노드(/PacketLayout/{0}) 찾기 실패", p_strTrNo));
                return false;
            }

            XmlAttribute attrType = xmlPacketElement.Attributes["type"];
            if (attrType == null) {
                SetError_(xmlPacketElement.Name + " : type attribute가 없습니다");
                return false;
            }
            if (attrType.Value != "Packet") {
                SetError_(string.Format("{0} : type ('{1}'이 'Packet'이 아닙니다", xmlPacketElement.Name, attrType.Value));
                return false;
            }

            FieldInfo fieldInfo = new FieldInfo();
            fieldInfo.strType = attrType.Value;
            fieldInfo.strName = xmlPacketElement.Name;
            fieldInfo.strFullName = xmlPacketElement.Name;
            fieldInfo.strValue = "";

            AddFiled_(null, fieldInfo);

            int pos = 0;

            bool ret = ParsePacket_(xmlPacketElement, fieldInfo, p_packet, ref pos);
            return ret;

            
        }

        #endregion

        #region METHOD - Build Packet Data

        public void StartBuild()
        {
            ClearFields_();
            ClearError_();

            this.m_parentFieldInfo = null;
            this.m_repeatName = "";
            this.m_repeatTurn = 0;
        }

        public void EndBuild(out bool p_result, out string p_strResultMsg)
        {
            p_result = this.Result;
            p_strResultMsg = this.ResultMsg;
        }

        public void CreatePacket(string p_strTrNo)
        {
            FieldInfo fieldInfo = new FieldInfo();
            fieldInfo.strType = "Packet";
            fieldInfo.strName = p_strTrNo;
            fieldInfo.strFullName = p_strTrNo;
            fieldInfo.strValue = "";

            AddFiled_(null, fieldInfo);
            this.m_parentFieldInfo = fieldInfo;
        }

        public void CreateHeader(string p_strHeaderName)
        {
            FieldInfo fieldInfo = new FieldInfo();
            fieldInfo.strType = "Packet";
            fieldInfo.strName = p_strHeaderName;
            fieldInfo.strFullName = p_strHeaderName;
            fieldInfo.strValue = "";

            AddFiled_(null, fieldInfo);

            this.m_parentFieldInfo = fieldInfo;
        }

        public void CreateRepeat(string p_strRepeatName)
        {
            if (p_strRepeatName != this.m_repeatName) {
                this.m_repeatName = p_strRepeatName;
                this.m_repeatTurn = 0;
            }
            else {
                this.m_repeatTurn++;
            }

            FieldInfo fieldInfo = new FieldInfo();
            fieldInfo.strType = "Repeat";
            fieldInfo.strName = string.Format("{0}[{1}]", p_strRepeatName, this.m_repeatTurn);
            fieldInfo.strFullName = fieldInfo.strName;
            fieldInfo.strValue = "";

            AddFiled_(null, fieldInfo);

            this.m_parentFieldInfo = fieldInfo;
        }

        public void SetField(string p_strFieldName, string p_value)
        {
            FieldInfo fieldInfo = new FieldInfo();
            fieldInfo.strType = "Field";
            fieldInfo.strName = p_strFieldName;
            fieldInfo.strFullName = this.m_parentFieldInfo.strFullName + "." + p_strFieldName;
            fieldInfo.strValue = p_value;

            AddFiled_(this.m_parentFieldInfo, fieldInfo);
        }

        public void SetField(string p_strFieldName, int p_value)
        {
            string value = p_value.ToString();
            SetField(p_strFieldName, value);
        }

        public void SetField(string p_strFieldName, char p_value)
        {
            string value = p_value.ToString();
            SetField(p_strFieldName, value);
        }

        public void SetField_h(string p_strFieldName)
        {
            // 생성하지 않아도 됨
        }

        public void SetField_r(string p_strFieldName)
        {
            // 생성하지 않아도 됨
        }

        #endregion

        #region METHOD - Parsing By PacketLayout

        public bool GetFieldValue(string p_strFieldPath, out string p_value)
        {
            p_value = "";

            FieldInfo fieldInfo;
            if (!GetFiled_(p_strFieldPath, out fieldInfo))
                return false;

            p_value = fieldInfo.strValue;
            return true;
        }

        public bool GetFieldValue(string p_strFieldPath, out char p_value)
        {
            p_value = ' ';

            FieldInfo fieldInfo;
            if (!GetFiled_(p_strFieldPath, out fieldInfo))
                return false;

            if (fieldInfo.strValue.Length > 0)
                p_value = fieldInfo.strValue[0];

            return true;
        }

        public bool GetFieldValue(string p_strFieldPath, out int p_value)
        {
            p_value = ' ';

            FieldInfo fieldInfo;
            if (!GetFiled_(p_strFieldPath, out fieldInfo))
                return false;

            if (fieldInfo.strValue.Length > 0)
                p_value = Convert.ToInt32(fieldInfo.strValue);

            return true;
        }

        public string GetPacketXml()
        {
            if (this.m_fieldInfos.Count <= 0 )
                return "";

            if (this.m_packetXml.Length > 0 )
                return this.m_packetXml;

            this.m_xmlDocData.RemoveAll();
            this.m_xmlDocData.LoadXml("<PacketLayout/>");

            try
            {
                if (!ToXml_(this.m_fieldInfos, this.m_xmlDocData.DocumentElement))
                {
                    SetError_("전문 XML 데이터를 생성하는데 실패하였습니다");
                    return "";
                }
            }
            catch (Exception ex)
            {
                SetError_("전문 XML 데이터를 생성하는데 실패하였습니다", ex);
                return "";
            }

            // TEST
            #if TEST 
            try {
                File.Delete(FileLog.LogDefaultPath + "/test.dxml");
                this.xmlDocData.Save(FileLog.LogDefaultPath + "/test.dxml");
            }
            catch { }
            #endif
            // TEST-END

            this.m_xmlDocData.Save(this.m_stream);
            this.m_stream.Seek(0, SeekOrigin.Begin);
            StreamReader sr = new StreamReader(this.m_stream);
            this.m_packetXml = sr.ReadToEnd();
            this.m_stream.SetLength(0);

            return this.m_packetXml;
        }

        #endregion

        #region IMPLEMENTATION

        void ClearError_()
        {
            this.m_result = true;
            this.m_resultCode = RESULT.EMPTY;
            this.m_resultMsg = "";
        }

        void SetError_(string p_strErrorMsg, RESULT p_resultCode = RESULT.EMPTY)
        {
            if(!this.m_result)
                return;

            this.m_result = false;
            this.m_resultCode = p_resultCode;
            this.m_resultMsg = p_strErrorMsg;
        }

        void SetError_(string p_strErrorMsg, Exception p_ex, RESULT p_resultCode = RESULT.EMPTY)
        {
            if (!this.m_result)
                return;

            this.m_result = false;
            this.m_resultCode = p_resultCode;
            this.m_reasonMsg = p_ex.Message;
            this.m_resultMsg = string.Format("{0} [{1}] [{2}]", p_strErrorMsg, p_ex.Message, p_ex.StackTrace);
        }

        bool ParsePacket_(XmlElement p_element, FieldInfo p_parentFieldInfo, byte[] p_packet, ref int pos)
        {
            if (p_element == null || !p_element.HasChildNodes)
                return false;

            XmlAttribute attr;
            XmlElement childElement = p_element.FirstChild as XmlElement;
            FieldInfo fieldInfo;

            while (childElement != null)
            {
                XmlAttribute attrType = childElement.Attributes["type"];
                if (attrType == null) {
                    SetError_(childElement.Name + " : type attribute가 없습니다");
                    return false;
                }

                switch (attrType.Value)
                {
                    case "Field": // type="Field"
                        
                        if (childElement.Name == "__순번")
                            break;
                        
                        // 필드 정보 얻기
                        attr = childElement.Attributes["len"];
                        if (attr == null) {
                            SetError_(childElement.Name + " : 'len' attribute가 없습니다");
                            return false;
                        }
                        int len = Convert.ToInt32(attr.Value);

                        attr = childElement.Attributes["trim_dir"];
                        if (attr == null) {
                            SetError_(childElement.Name + " : 'trim_dir' attribute가 없습니다");
                            return false;
                        }
                        string strTrimDir = attr.Value;

                        attr = childElement.Attributes["trim_char"];
                        if (attr == null) {
                            SetError_(childElement.Name + " : 'trim_char' attribute가 없습니다");
                            return false;
                        }
                        string strTrimChar = ( attr.Value == "SPACE" ) ? " " : attr.Value;
                        
                        //// type_f : 데이터타입 - String, Number, Raw, Packet
                        attr = childElement.Attributes["type_f"];
                        if (attr == null) {
                            SetError_(childElement.Name + " : 'type_f' attribute가 없습니다");
                            return false;
                        }
                        string strType_f = attr.Value;


                        // 전문에서 필드값 추출
                        if (p_packet.Length < pos + len) {
                            SetError_(string.Format(" '{0}' 필드값을 얻기에 패킷길이({1})가 작습니다", childElement.Name, p_packet.Length),
                                      RESULT.F_PARSE_ERROR_PACKETSIZ_ENOTENOUGH);
                            return false;
                        }

                        string strFieldValue = Encoding.Default.GetString(p_packet, pos, len);
                        strFieldValue = strFieldValue.Replace("\0", "");
                        pos += len;
                        
                        //// Trim 처리 ////
                        switch(strTrimDir)
                        {
                            case "Left":
                                strFieldValue = strFieldValue.TrimStart(strTrimChar.ToCharArray());
                                break;
                            case "Right":
                                strFieldValue = strFieldValue.TrimEnd(strTrimChar.ToCharArray());
                                break;
                            case "Both":
                                strFieldValue = strFieldValue.Trim();
                                break;
                        }

                        //// 값 처리 ////
                        switch (strType_f)
                        {
                            case "Number":
                                if (string.IsNullOrEmpty(strFieldValue))
                                {
                                    strFieldValue = "0";
                                }
                                else
                                {
                                    double nFieldValue = -1;
                                    if (Double.TryParse(strFieldValue, out nFieldValue))
                                        strFieldValue = string.Format("{0}", nFieldValue);
                                    else
                                        strFieldValue = string.Format("{0}", 0);
                                }
                                break;

                            case "Raw":
                                if (string.IsNullOrEmpty(strFieldValue))
                                {
                                    strFieldValue = "0";
                                }

                                break;

                            default:
                                break;
                        }

                        // 필드값 등록
                        fieldInfo = new FieldInfo();
                        fieldInfo.strType = attrType.Value;
                        fieldInfo.strName = childElement.Name;
                        fieldInfo.strFullName = string.Format("{0}.{1}", p_parentFieldInfo.strFullName, childElement.Name);

                        //// 개행처리 - KIMCG : 2013.02.28 ////
                        if (strFieldValue.Contains("\\r\\n"))
                            strFieldValue = strFieldValue.Replace("\\r\\n", Environment.NewLine);

                        fieldInfo.strValue = strFieldValue;

                        AddFiled_(p_parentFieldInfo, fieldInfo);

                        break;

                    case "SubpacketField": // type='SubpacketField'

                        // "Packet" 찾기
                        XmlElement packetElement = this.m_xmlRootElement.SelectSingleNode(string.Format(ROOTNAME + childElement.Name)) as XmlElement;
                        if (packetElement == null)
                        {
                            SetError_(string.Format("{0}/{1} 엘리먼트가 없습니다", ROOTNAME, childElement));
                            return false;
                        }

                        attr = packetElement.Attributes["type"];
                        if (attr == null) {
                            SetError_(packetElement.Name + " : type attribute가 없습니다");
                            return false;
                        }
                        if (attr.Value != "Packet") {
                            SetError_(string.Format("{0} : type ('{1}'이 'Packet'이 아닙니다", packetElement.Name, attr.Value));
                            return false;
                        }

                        fieldInfo = new FieldInfo();
                        //fieldInfo.strType = attrType.Value;
                        
                        // MODIFY - KCG : 20110522
                        //fieldInfo.strType = "Subpacket";
                        fieldInfo.strType = "Packet";
                        // MODIFY - END
                        
                        fieldInfo.strName = childElement.Name;
                        fieldInfo.strFullName = childElement.Name;
                        fieldInfo.strValue = "";

                        AddFiled_(null, fieldInfo);

                        ParsePacket_(packetElement, fieldInfo, p_packet, ref pos);
                        break;

                    case "RepeatField":    // type="RepeatField"
                        attr = childElement.Attributes["field_extra"];
                        if (attr == null) {
                            SetError_(childElement.Name + " : 'field_extra' attribute가 없습니다");
                            return false;
                        }
                        string strRepeatCntField = attr.Value;

                        attr = childElement.Attributes["FixRowCount"];
                        if (attr == null) {
                            SetError_(childElement.Name + " : 'fixRowCnt' attribute가 없습니다");
                            return false;
                        }
                        string strFixRowCnt = attr.Value;

                        if (!GetFiled_(string.Format("{0}.{1}", p_parentFieldInfo.strFullName, strRepeatCntField), out fieldInfo))
                        {
                            SetError_(string.Format( "{0}의 반복부 건수 필드('{1}')가 존재하지 않습니다.", childElement.Name, strRepeatCntField),
                                      RESULT.F_PARSE_ERROR_REPEAT_COUNT);
                            return false;
                        }

                        int nRepeatCnt;
                        int nFixCnt;
                        int nMaxCnt;
                        try 
                        { 
                            nRepeatCnt = Convert.ToInt32(fieldInfo.strValue);

                            if (!string.IsNullOrEmpty(strFixRowCnt))
                            {
                                nFixCnt = Convert.ToInt32(strFixRowCnt);
                                nMaxCnt = nFixCnt;
                            }
                            else
                            {
                                nMaxCnt = nRepeatCnt;
                            }
                         }
                        catch
                        {
                            SetError_(string.Format("{0}의 반복부 건수 필드의 값('{1}')이 숫자값이 아닙니다.", childElement.Name, strRepeatCntField),
                                      RESULT.F_PARSE_ERROR_REPEAT_COUNT);
                            return false;
                        }

                        // "Repeat" 찾기
                        // MODIFY - KCG : 20110522
                        //XmlElement repeatElement = this.xmlRootElement.SelectSingleNode(string.Format(ROOTNAME + childElement.Name)) as XmlElement;
                        XmlElement repeatElementBegin = this.m_xmlRootElement.SelectSingleNode(string.Format("{0}{1}/{2}", ROOTNAME, p_parentFieldInfo.strFullName, childElement.Name)) as XmlElement;
                        XmlElement repeatElement = repeatElementBegin.NextSibling as XmlElement;
                        childElement = repeatElement;
                        // MOCIFY - END

                        attr = repeatElement.Attributes["type"];
                        if (attr == null) {
                            SetError_(repeatElement.Name + " : type attribute가 없습니다");
                            return false;
                        }
                        if (attr.Value != "Repeat") {
                            SetError_(string.Format("{0} : type ('{1}'이 'Repeat'이 아닙니다", repeatElement.Name, attr.Value));
                            return false;
                        }
                        
 						int nLastOfRepeatPos = 0;
                        for (int i = 0; i < nMaxCnt; i++)
                        {
                            fieldInfo = new FieldInfo();
                            fieldInfo.strType = attr.Value;
                            fieldInfo.strName = string.Format("{0}[{1}]", repeatElement.Name, i);
                            fieldInfo.strFullName = fieldInfo.strName;
                            fieldInfo.strValue = "";
                            AddFiled_(null, fieldInfo);
                            
                            //// 순번처리 ////
                            FieldInfo seqFiledInfo = new FieldInfo();
                            seqFiledInfo.strType = "Field";
                            seqFiledInfo.strName = "__순번";
                            seqFiledInfo.strFullName = string.Format("{0}.{1}", fieldInfo.strFullName, seqFiledInfo.strName);
                            seqFiledInfo.strValue = Convert.ToString(i + 1);
                            AddFiled_(fieldInfo, seqFiledInfo);
                            
                            if (!ParsePacket_(repeatElement, fieldInfo, p_packet, ref pos))
                            {
                                SetError_(string.Format("'{0}' 반복부 {1}번째를 파싱하면서 오류가 발생하였습니다.", fieldInfo.strName, i + 1),
                                          RESULT.F_PARSE_ERROR_REPEATPART);
                                return false;
                            }
                        }
                                                
                        break;

                    default:
                        SetError_(string.Format("{0} : 유효하지 않은 type('{1}')입니다.", childElement.Name, attrType.Value));
                        return false;
                }

                childElement = childElement.NextSibling as XmlElement;

            } // while

            return true;
        }

        bool AddFiled_(FieldInfo p_parentFileInfo, FieldInfo p_fieldInfo)
        {
            if (this.m_dictFieldInfos.ContainsKey(p_fieldInfo.strFullName))
                return false;

            if (p_parentFileInfo == null)
                this.m_fieldInfos.Add(p_fieldInfo);
            else
            {
                if (p_parentFileInfo.childInfos == null)
                    p_parentFileInfo.childInfos = new List<FieldInfo>();
                p_parentFileInfo.childInfos.Add(p_fieldInfo);
            }
            this.m_dictFieldInfos.Add(p_fieldInfo.strFullName, p_fieldInfo);

            return true;
        }

        bool GetFiled_(string p_strFullName, out FieldInfo p_fieldInfo)
        {
            p_fieldInfo = null;

            if (!this.m_dictFieldInfos.ContainsKey(p_strFullName)) {
                SetError_(string.Format("파싱된 결과에 '{0}' 필드가 존재하지 않습니다", p_strFullName));
                return false;
            }

            p_fieldInfo = this.m_dictFieldInfos[p_strFullName];
            return true;
        }

        void ClearFields_()
        {
            this.m_dictFieldInfos.Clear();
            this.m_fieldInfos.Clear();
            this.m_packetXml = "";
        }

        bool ToXml_(List<FieldInfo> p_fieldInfos, XmlElement p_parentElement)
        {
            foreach (FieldInfo fieldInfo in p_fieldInfos)
            {
                string strName;
                int idx = fieldInfo.strName.IndexOf('[');
                if (idx > 0) 
                    strName = fieldInfo.strName.Substring(0, idx);
                else
                    strName = fieldInfo.strName;

                XmlElement element = this.m_xmlDocData.CreateElement(strName);

                element.SetAttribute("type", fieldInfo.strType);
                if (!string.IsNullOrEmpty(fieldInfo.strAttribute))
                    element.SetAttribute("FixRowCount", fieldInfo.strAttribute);
                
                if (fieldInfo.strType == "Field")
                    element.InnerText = fieldInfo.strValue;

                p_parentElement.AppendChild(element);

                if (fieldInfo.childInfos != null)
                    ToXml_(fieldInfo.childInfos, element);
            }

            return true;
        }
        
        #endregion

        #region fields
        // 전문 데이터 저장 메모리
        Dictionary<string, FieldInfo> m_dictFieldInfos = new Dictionary<string,FieldInfo>();
        List<FieldInfo> m_fieldInfos = new List<FieldInfo>();
        
        // 전문레이아웃 파싱 용 
        string m_layoutPathFile = "";
        XmlDocument m_xmlDoc = new XmlDocument();
        XmlElement m_xmlRootElement = null;

        // 전문 데이터 구성 용
        FieldInfo m_parentFieldInfo = null;
        string    m_repeatName = "";
        int m_repeatTurn = 0;

        // 결과 저장
        bool m_result = false;
        RESULT m_resultCode = RESULT.EMPTY;
        string m_resultMsg = "PARSER 초기상태";
        string m_reasonMsg = "PARSER 초기상태";

        
        // 전문 XML 생성용
        XmlDocument m_xmlDocData = new XmlDocument();
        MemoryStream m_stream = new MemoryStream();
        string m_packetXml = "";

        #endregion
    }
}
