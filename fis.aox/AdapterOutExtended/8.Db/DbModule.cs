using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.IO;
using System.Data.OracleClient;
using Btfax.CommonLib.Db;

namespace AdapterOutExtended
{
    class DbModule : DbModuleBase
    {
        #region static
        private static DbModule s_instance = null;
        public static DbModule Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new DbModule();

                return s_instance;
            }
        }
        #endregion

        #region constructor
        public DbModule()
        {
        }
        #endregion

        #region inner - class
        public class SendRequestMasterInfo
        {
            public string INDEX_NO         { get; private set; }    /// 인덱스번호
            public string TR_NO            { get; private set; }    /// 전문번호
            public string STATE            { get; private set; }    /// 상태코드
            public string PRIORITY         { get; private set; }    /// 우선순위
            public string TITLE            { get; private set; }
            public string MEMO             { get; private set; }
            public string PRE_PROCESS_REQ  { get; private set; }    /// 전처리 수행여부
            public string REQ_TYPE         { get; set; }            /// 요청유형
            /// <summary>
            /// 팩스요청 구분
            /// 01 : IVR
            /// 02 : 복합기
            /// 03 : 웹
            /// 04 : 그룹웨어
            /// </summary>
            public string REQUESTER_TYPE   { get; private set; }
            public string REQ_USER_ID      { get; private set; }    /// 발신자아이디
            public string REQ_USER_NAME    { get; private set; }    /// 발신자이름
            public string REQ_USER_TELNO   { get; private set; }
            public DateTime REQ_DATE       { get; set; }
            public string PREVIEW_REQ      { get; private set; }    /// 미리보기여부
            public string PREVIEWED_ID     { get; private set; }
            public DateTime PREVIEWED_DATE { get; private set; }
            public string APPROVE_REQ      { get; private set; }    /// 승인필요여부
            public string APPROVED_YN      { get; private set; }
            public string APPROVED_ID      { get; private set; }
            public DateTime APPROVED_DATE  { get; private set; }
            public string APPROVED_COMMENT { get; private set; }
            public string RESERVED_YN      { get; private set; }    /// 발송예약여부
            public DateTime RESERVED_DATE  { get; private set; }    /// 발송예약일자시간
            public string PROCESS_FETCH    { get; private set; }
            public string SMS_CONTENTS     { get; private set; }
            public string SMS_SEND_YN      { get; private set; }
            public string RESULT_FORWARD   { get; private set; }
            public string TEST_TYPE        { get; private set; }    /// 테스트팩스여부
            public string BROADCAST_YN     { get; private set; }    /// 동보 여부
            public decimal OUT_FAX_ID      { get; set; }            /// 반환 받은 FAX_ID

            public SendRequestMasterInfo()
            {
                this.Clear();
            }
            public SendRequestMasterInfo(string requester_type = "file") : this()
            {
                this.REQUESTER_TYPE = requester_type == "file" ? "02" : "04";
            }

            public void Clear()
            {
                INDEX_NO = "";
                TR_NO = Config.PROCESS_TYPE.ToString();
                STATE = "00";
                PRIORITY = "2";
                TITLE = "";
                MEMO = "By Adapter for Outbound Extended.";
                PRE_PROCESS_REQ = "N";
                REQ_TYPE = "00";
                REQUESTER_TYPE = "02";
                REQ_USER_ID = "";
                REQ_USER_NAME = "";
                REQ_USER_TELNO = "";
                REQ_DATE = DateTime.Now;
                PREVIEW_REQ = "N";
                PREVIEWED_ID = "";
                PREVIEWED_DATE = DateTime.MinValue;
                APPROVE_REQ = "N";
                APPROVED_YN = "";
                APPROVED_ID = "";
                APPROVED_DATE = DateTime.MinValue;
                APPROVED_COMMENT = "";
                RESERVED_YN = "N";
                RESERVED_DATE = DateTime.MinValue;
                PROCESS_FETCH = "";
                SMS_CONTENTS = "";
                SMS_SEND_YN = "N";
                RESULT_FORWARD = "";
                TEST_TYPE = "00";
                BROADCAST_YN = "N";
                OUT_FAX_ID = -1;
            }

            public void DataSetting(string p_REQ_USER_ID, string p_REQ_USER_TELNO, string p_REQ_USER_NAME, string p_TITLE)
            {
                REQ_USER_ID = p_REQ_USER_ID;
                REQ_USER_TELNO = p_REQ_USER_TELNO;
                REQ_USER_NAME = p_REQ_USER_NAME;
                TITLE = p_TITLE;
            }
        }

        public class SendRequestDetailInfo
        {
            public decimal FAX_ID            { get; set; }          /// Master 의 FAX_ID
            public string STATE_EACH         { get; private set; }  /// 상태코드
            public string FAX_NO             { get; private set; }  /// 보낼 팩스 번호
            public string RECIPIENT_NAME     { get; private set; }  /// 수신자 이름
            public string TIF_FILE           { get; set; }          /// TIF 파일 이름 (201209\06\파일.TIF)
            public decimal TIF_FILE_SIZE     { get; private set; }  /// TIF 파일 크기 (byte)
            public string TIF_PAGE_CNT       { get; private set; }  /// TIF 페이지 수
            public string PAGES_TO_SEND      { get; private set; }
            public decimal LAST_PAGE_SENT    { get; private set; }
            public string TITLE              { get; private set; }
            public string MEMO               { get; private set; }
            public string RESULT             { get; private set; }
            public string REASON             { get; private set; }
            public decimal TRY_CNT           { get; private set; }
            public string SMSNO              { get; private set; }
            public string PROCESS_FETCH_EACH { get; private set; }
            public DateTime DATE_TO_SEND     { get; set; }
            public decimal OUT_SEQ           { get; set; }
            public string OUT_TIF_FILE       { get; set; }

            public SendRequestDetailInfo()
            {
                this.Clear();
            }

            public void Clear()
            {
                FAX_ID = -1;
                STATE_EACH = "00";
                FAX_NO = "";
                RECIPIENT_NAME = "";
                TIF_FILE = "";
                TIF_FILE_SIZE = -1;
                TIF_PAGE_CNT = "";
                PAGES_TO_SEND = "all";
                LAST_PAGE_SENT = 0;
                TITLE = "";
                MEMO = "";
                RESULT = "";
                REASON = "";
                TRY_CNT = -1;
                SMSNO = "";
                PROCESS_FETCH_EACH = "";
                DATE_TO_SEND = DateTime.Now;
                OUT_SEQ = -1;
                OUT_TIF_FILE = "";
            }

            public void DataSetting(string p_FAX_NO, string p_TITLE, string p_TIF_FILE)
            {
                FAX_NO = p_FAX_NO;
                TITLE = p_TITLE;
                TIF_FILE = p_TIF_FILE;
            }
        }

        public class SendRequestDocInfo : DOC_INFO
        {
            public decimal FAX_ID { get; set; }
            public string DOC_PATH { get; set; }
            //public string PROCESSING_STATE { get; set; }
            public decimal OUT_SEQ { get; set; }

            public override void Clear()
            {
 	            base.Clear();
                this.FAX_ID = -1;
                DOC_PATH = "";
                //PROCESSING_STATE = "";
                OUT_SEQ = -1;
                strProcessingMode = "11";
            }

            //public void DataSetting()
            //{

            //}
        }
        #endregion

        #region Business
        /// <summary>
        /// Name       : AOX_InsertSendMaster
        /// Parameters : 
        /// Content    : BTF_FAX_SEND_MSTR 테이블에 INSERT
        /// Return     : true / false
        /// Writer     : 장동훈
        /// Date       : 2012.09.06
        /// </summary>
        public decimal AOX_InsertSendMaster(SendRequestMasterInfo p_mstr)
        {
            string strProcedureName = "PKG_PRC_AOX.USP_INSERT_SEND_MSTR";
            decimal faxID = -1;

            try
            {
                using (OracleCommand command = new OracleCommand())
                {
                    command.Connection = m_connection;
                    command.CommandText = strProcedureName;
                    command.CommandType = CommandType.StoredProcedure;

                    //AddInParam("P_INDEX_NO        ", OracleType.VarChar, p_mstr.INDEX_NO);
                    //command.Parameters.AddWithValue("P_INDEX_NO        ".Trim(), p_mstr.INDEX_NO);
                    command.Parameters.AddWithValue("P_TR_NO           ".Trim(), p_mstr.TR_NO);
                    command.Parameters.AddWithValue("P_STATE           ".Trim(), p_mstr.STATE);
                    command.Parameters.AddWithValue("P_PRIORITY        ".Trim(), p_mstr.PRIORITY);
                    command.Parameters.AddWithValue("P_TITLE           ".Trim(), p_mstr.TITLE);
                    command.Parameters.AddWithValue("P_MEMO            ".Trim(), p_mstr.MEMO);
                    command.Parameters.AddWithValue("P_PRE_PROCESS_REQ ".Trim(), p_mstr.PRE_PROCESS_REQ);
                    command.Parameters.AddWithValue("P_REQ_TYPE        ".Trim(), p_mstr.REQ_TYPE);
                    command.Parameters.AddWithValue("P_REQUESTER_TYPE  ".Trim(), p_mstr.REQUESTER_TYPE);
                    command.Parameters.AddWithValue("P_REQ_USER_ID     ".Trim(), p_mstr.REQ_USER_ID);
                    command.Parameters.AddWithValue("P_REQ_USER_NAME   ".Trim(), p_mstr.REQ_USER_NAME);
                    command.Parameters.AddWithValue("P_REQ_USER_TELNO  ".Trim(), p_mstr.REQ_USER_TELNO);
                    command.Parameters.AddWithValue("P_REQ_DATE        ".Trim(), p_mstr.REQ_DATE);
                    command.Parameters.AddWithValue("P_PREVIEW_REQ     ".Trim(), p_mstr.PREVIEW_REQ);
                    //command.Parameters.AddWithValue("P_PREVIEWED_ID    ".Trim(), p_mstr.PREVIEWED_ID);
                    //command.Parameters.AddWithValue("P_PREVIEWED_DATE  ".Trim(), p_mstr.PREVIEWED_DATE);
                    command.Parameters.AddWithValue("P_APPROVE_REQ     ".Trim(), p_mstr.APPROVE_REQ);
                    //command.Parameters.AddWithValue("P_APPROVED_YN     ".Trim(), p_mstr.APPROVED_YN);
                    //command.Parameters.AddWithValue("P_APPROVED_ID     ".Trim(), p_mstr.APPROVED_ID);
                    //command.Parameters.AddWithValue("P_APPROVED_DATE   ".Trim(), p_mstr.APPROVED_DATE);
                    //command.Parameters.AddWithValue("P_APPROVED_COMMENT".Trim(), p_mstr.APPROVED_COMMENT);
                    command.Parameters.AddWithValue("P_RESERVED_YN     ".Trim(), p_mstr.RESERVED_YN);
                    //command.Parameters.AddWithValue("P_RESERVED_DATE   ".Trim(), p_mstr.RESERVED_DATE);
                    //command.Parameters.AddWithValue("P_PROCESS_FETCH   ".Trim(), p_mstr.PROCESS_FETCH);
                    //command.Parameters.AddWithValue("P_SMS_CONTENTS    ".Trim(), p_mstr.SMS_CONTENTS);
                    command.Parameters.AddWithValue("P_SMS_SEND_YN     ".Trim(), p_mstr.SMS_SEND_YN);
                    //command.Parameters.AddWithValue("P_RESULT_FORWARD  ".Trim(), p_mstr.RESULT_FORWARD);
                    command.Parameters.AddWithValue("P_TEST_TYPE       ".Trim(), p_mstr.TEST_TYPE);
                    command.Parameters.AddWithValue("P_BROADCAST_YN    ".Trim(), p_mstr.BROADCAST_YN);
                    OracleParameter faxIdParam = new OracleParameter("P_OUT_FAX_ID", OracleType.Number);
                    faxIdParam.Direction = ParameterDirection.Output;
                    command.Parameters.Add(faxIdParam);

                    int cnt = command.ExecuteNonQuery();
                    if (cnt > 0)
                    {
                        faxID = Convert.ToDecimal(faxIdParam.Value);
                    }
                }
            }
            catch (Exception ex)
            {
                ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                //return -1;
                throw ex;
            }
            return faxID;
        }

        /// <summary>
        /// Name       : AOX_InsertSendDetail
        /// Parameters : 
        /// Content    : BTF_FAX_SEND_DTL 테이블에 INSERT
        /// Return     : true / false
        /// Writer     : 장동훈
        /// Date       : 2012.09.06
        /// </summary>
        public decimal AOX_InsertSendDetail(SendRequestDetailInfo p_dtl)
        {
            string strProcedureName = "PKG_PRC_AOX.USP_INSERT_SEND_DTL";
            decimal dtlSeq = -1;
            try
            {
                using (OracleCommand command = new OracleCommand())
                {
                    command.Connection = m_connection;
                    command.CommandText = strProcedureName;
                    command.CommandType = CommandType.StoredProcedure;
                    command.Parameters.AddWithValue("P_FAX_ID            ".Trim(), p_dtl.FAX_ID);
                    command.Parameters.AddWithValue("P_STATE_EACH        ".Trim(), p_dtl.STATE_EACH);
                    command.Parameters.AddWithValue("P_FAX_NO            ".Trim(), p_dtl.FAX_NO);
                    command.Parameters.AddWithValue("P_RECIPIENT_NAME    ".Trim(), p_dtl.RECIPIENT_NAME);
                    command.Parameters.AddWithValue("P_TIF_FILE          ".Trim(), p_dtl.TIF_FILE);
                    //command.Parameters.AddWithValue("P_TIF_FILE_SIZE     ".Trim(), p_dtl.TIF_FILE_SIZE);
                    //command.Parameters.AddWithValue("P_TIF_PAGE_CNT      ".Trim(), p_dtl.TIF_PAGE_CNT);
                    command.Parameters.AddWithValue("P_PAGES_TO_SEND     ".Trim(), p_dtl.PAGES_TO_SEND);
                    command.Parameters.AddWithValue("P_LAST_PAGE_SENT    ".Trim(), p_dtl.LAST_PAGE_SENT);
                    //command.Parameters.AddWithValue("P_TITLE             ".Trim(), p_dtl.TITLE);
                    //command.Parameters.AddWithValue("P_MEMO              ".Trim(), p_dtl.MEMO);
                    //command.Parameters.AddWithValue("P_RESULT            ".Trim(), p_dtl.RESULT);
                    //command.Parameters.AddWithValue("P_REASON            ".Trim(), p_dtl.REASON);
                    //command.Parameters.AddWithValue("P_TRY_CNT           ".Trim(), p_dtl.TRY_CNT);
                    //command.Parameters.AddWithValue("P_SMSNO             ".Trim(), p_dtl.SMSNO);
                    //command.Parameters.AddWithValue("P_PROCESS_FETCH_EACH".Trim(), p_dtl.PROCESS_FETCH_EACH);
                    command.Parameters.AddWithValue("P_DATE_TO_SEND      ".Trim(), p_dtl.DATE_TO_SEND);
                    OracleParameter oraParam = new OracleParameter("P_OUT_SEQ", OracleType.Number);
                    oraParam.Direction = ParameterDirection.Output;
                    command.Parameters.Add(oraParam);
                    OracleParameter oraParam2 = new OracleParameter("P_OUT_TIF_FILE", OracleType.VarChar, 200);
                    oraParam2.Direction = ParameterDirection.Output;
                    command.Parameters.Add(oraParam2);
                    
                    ///command.Parameters.

                    int cnt = command.ExecuteNonQuery();
                    if (cnt > 0)
                    {
                        dtlSeq = Convert.ToDecimal(oraParam.Value);
                    }
                }
            }
            catch (Exception ex)
            {
                ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                //return -1;
                throw ex;
            }
            return dtlSeq;
        }

        public bool AOX_InsertSendDoc(decimal p_FAX_ID, SendRequestDocInfo p_doc)
        {
            string strProcedureName = "PKG_PRC_AOX.USP_INSERT_SEND_DOC";
            try
            {
                using (OracleCommand command = this.MakeOracleCommand(strProcedureName))
                {
                    OraParam param = new OraParam();
                    param.AddInParam("P_FAX_ID          ", OracleType.Number, p_FAX_ID);
                    param.AddInParam("P_DOC_PATH        ", OracleType.VarChar, p_doc.DOC_PATH);
                    param.AddInParam("P_DOC_FILE        ", OracleType.VarChar, p_doc.strDocFile);
                    param.AddInParam("P_DOC_EXT         ", OracleType.VarChar, p_doc.strDocExt);
                    param.AddInParam("P_PROCESSING_MODE ", OracleType.VarChar, p_doc.strProcessingMode);
                    param.AddInParam("P_TIF_EXTRACT_PAGE", OracleType.VarChar, p_doc.strTiffExtractPages);
                    command.Parameters.AddRange(param.oraParams.ToArray());

                    int cnt = command.ExecuteNonQuery();
                    if (cnt < 1) return false;
                }
            }
            catch (Exception ex)
            {
                ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                return false;
            }
            return true;
        }

        public bool AOX_InsertFAXBOX(SendRequestMasterInfo p_mstr)
        {
            string strProcedureName = "PKG_PRC_COMMON.USP_FAXSEND_SET_FAXBOX";
            //decimal faxID = -1;

            try
            {
                using (OracleCommand command = new OracleCommand())
                {
                    command.Connection = m_connection;
                    command.CommandText = strProcedureName;
                    command.CommandType = CommandType.StoredProcedure;

                    command.Parameters.AddWithValue("P_REQ_USER_ID     ".Trim(), p_mstr.REQ_USER_ID);
                    command.Parameters.AddWithValue("P_FAX_ID          ".Trim(), p_mstr.OUT_FAX_ID);

                    OracleParameter resultParam = new OracleParameter("P_RESULT", OracleType.VarChar, 1);
                    resultParam.Direction = ParameterDirection.Output;
                    command.Parameters.Add(resultParam);

                    int cnt = command.ExecuteNonQuery();
                    if (cnt > 0)
                    {
                        //faxID = Convert.ToDecimal(resultParam.Value);
                    }
                }
            }
            catch (Exception ex)
            {
                ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                //return -1;
                throw ex;
            }
            return true;
        }
        #endregion

        #region method

        private OracleCommand MakeOracleCommand(string procedureName)
        {
            OracleCommand command = new OracleCommand();
            command.Connection = m_connection;
            command.CommandText = procedureName;
            command.CommandType = CommandType.StoredProcedure;
            return command;
        }
        #endregion

        #region implementation - override
        #endregion

        #region 사이트별 커스터마이징
        /// <summary>
        /// Name       : InsertSendSite
        /// Parameters : 전문번호
        ///              팩스 요청건
        /// Content    : BTF_FAX_SEND_SITE 테이블에 INSERT
        /// Return     : true / false
        /// Writer     : KIMCG
        /// Date       : 2012.07.30
        /// </summary>
        public bool InsertSendSite()
        {
            //// 여기에 사이트 정보를 INSERT 하는 로직을 추가 해주세요. ////
            //// To - Do ////

            return true;
        }

        #region 사이트 정보를 저장할 클래스 입니다.
        /// <summary>
        /// Name       : SEND_REQ_SITE
        /// Content    : 사이트 정보를 저장할 클래스 입니다.
        /// Writer     : KIMCG
        /// Date       : 2012.07.30
        /// </summary>
        public class SEND_REQ_SITE
        {
            //// 아래의 예시 처럼 멤버들을 추가해주세요. /////
            #region 예시
            public string strIndexKey;          // 인덱스키
            public string strChequeNum;         // 수표번호
            public string strWarrantNum;        // 영장번호
            public string strFax_Sn_Proc_No;    // FAX전송처리번호
            public string strFax_Send_AMNO;     // FAX전송관리번호
            public string strTitle;             // 제목
            public string strUseSMS;            // 문자발송 사용여부
            public string strSMSTelNum;         // 문자발송할 전화번호
            public string strSMSContent;        // 문자내용
            public string strUseEmail;          // 메일발송 사용여부
            public string strEmailAddr;         // 메일주소
            public string strEmailContent;      // 메일내용
            public string strTemp;              // 임시

            public SEND_REQ_SITE()
            {
                clear();
            }

            public void clear()
            {
                strIndexKey = "";
                strChequeNum = "";
                strWarrantNum = "";
                strFax_Sn_Proc_No = "";
                strFax_Send_AMNO = "";
                strTitle = "";
                strUseSMS = "";
                strSMSTelNum = "";
                strSMSContent = "";
                strUseEmail = "";
                strEmailAddr = "";
                strEmailContent = "";
                strTemp = "";
            }
            #endregion
        }
        #endregion
        #endregion
    }

    public class OraParam
    {
        public List<OracleParameter> oraParams = new List<OracleParameter>();

        public OracleParameter AddInParam(string parameterName, OracleType oraType, object value, int size = 0)
        {
            OracleParameter oraParam = new OracleParameter(parameterName.Trim(), oraType);
            oraParam.Value = value;
            oraParam.Size = size;
            oraParam.Direction = ParameterDirection.Input;

            this.oraParams.Add(oraParam);

            return oraParam;
        }
        public OracleParameter AddOutParam(string parameterName, OracleType oraType)
        {
            OracleParameter oraParam = new OracleParameter(parameterName.Trim(), oraType);
            oraParam.Direction = ParameterDirection.Output;

            this.oraParams.Add(oraParam);

            return oraParam;
        }
    }
}

