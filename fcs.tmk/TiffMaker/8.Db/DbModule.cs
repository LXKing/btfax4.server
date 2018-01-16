using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.OracleClient;
using Btfax.CommonLib;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Log;
using TiffMaker.Threading;
using TiffMaker.Util;

namespace TiffMaker.Db
{
    class DbModule : DbModuleBase
    {
        #region static
        static DbModule s_instance = null;
        static public DbModule Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new DbModule();

                return s_instance;
            }
        }
        #endregion
        
        #region inner class
        public class DOC_INFO_ALL_TMK : DOC_INFO_ALL
        {
            public DOC_INFO_ALL_TMK()
            {
                base.Clear();
            }
        }
        
        public class SEND_REQUEST_DTL_TMK : SEND_REQUEST_DTL
        {
            #region constructor
            public SEND_REQUEST_DTL_TMK()
            {   
                base.Clear();
            }
            #endregion
        }

        public class SEND_REQUEST_TMK : SEND_REQUEST
        {
            #region constructor
            public SEND_REQUEST_TMK()
            {
                m_strPacket = "";
                m_strFaxFormFileName = "";
                m_strFaxFormPath = "";
                m_tmkDocInfoAll = null;
                m_lstTmkSendRequestDtlInfos.Clear();
                base.Clear();
            }
            #endregion

            #region field
            public string m_strPacket;
            public string m_strFaxFormFileName;
            public string m_strFaxFormPath;
            public DOC_INFO_ALL_TMK m_tmkDocInfoAll;

            public List<SEND_REQUEST_DTL_TMK> m_lstTmkSendRequestDtlInfos = new List<SEND_REQUEST_DTL_TMK>();
            #endregion
        }


        #endregion
        
        #region constructor
        public DbModule() { }
        #endregion
        
        #region method
        public RESULT FetchSendListToMake(P_TYPE p_processType, string p_strSystemProcessId, int p_occupyCnt, ref List<DbModule.SEND_REQUEST_TMK> p_lstTmkSendReqInfos)
        {
            RESULT ret = RESULT.EMPTY;
            string strProcedureName = "PKG_PRC_TMK.USP_OCCUPY_SEND_REQ";

            lock (m_connection)
            {
                
                if (!ReOpen())
                    return RESULT.F_DB_OPENERROR;
                try
                {
                    using (OracleCommand command = new OracleCommand())
                    {
                        command.Connection = m_connection;
                        command.CommandText = strProcedureName;
                        command.CommandType = CommandType.StoredProcedure;

                        command.Parameters.AddWithValue("P_PROCESS_TYPE", p_processType.ToString());
                        command.Parameters.AddWithValue("P_FETCH_PROCESS", p_strSystemProcessId);
                        command.Parameters.AddWithValue("P_OCCUPY_CNT", p_occupyCnt);

                        OracleParameter param = new OracleParameter("P_TABLE1", OracleType.Cursor);
                        param.Direction = ParameterDirection.Output;
                        command.Parameters.Add(param);

                        param = new OracleParameter("P_TABLE2", OracleType.Cursor);
                        param.Direction = ParameterDirection.Output;
                        command.Parameters.Add(param);

                        command.ExecuteNonQuery();

                        DataTable dtMstr = new DataTable("MSTR");
                        OracleDataReader reader = (OracleDataReader)command.Parameters["P_TABLE1"].Value;
                        dtMstr.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        DataTable dtDtl = new DataTable("DETAIL");
                        reader = (OracleDataReader)command.Parameters["P_TABLE2"].Value;
                        dtDtl.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        if (dtMstr.Rows.Count <= 0 || dtDtl.Rows.Count <= 0)
                        {
                            if (dtMstr != null) dtMstr.Dispose();
                            if (dtDtl != null) dtDtl.Dispose();
                            return RESULT.EMPTY;
                        }

                        //// BTF_FAX_SEND_MSTR ////
                        foreach (DataRow mstrRow in dtMstr.Rows)
                        {
                            SEND_REQUEST_TMK tmkReqInfo = new SEND_REQUEST_TMK();
                            tmkReqInfo.faxId = (decimal)mstrRow["FAX_ID"];
                            tmkReqInfo.strTrNo = mstrRow["TR_NO"].ToString();
                            tmkReqInfo.strState = mstrRow["STATE"].ToString();
                            tmkReqInfo.strReqType = mstrRow["REQ_TYPE"].ToString();
                            tmkReqInfo.previewYN = mstrRow["PREVIEW_REQ"].ToString();
                            tmkReqInfo.approvedReq = mstrRow["APPROVE_REQ"].ToString();

                            //// BTF_FAX_SEND_DTL ////
                            DataRow[] dtlRows = dtDtl.Select(String.Format("FAX_ID={0}", tmkReqInfo.faxId));
                            foreach (DataRow dtlRow in dtlRows)
                            {
                                SEND_REQUEST_DTL_TMK tmkDtlReqInfo = new SEND_REQUEST_DTL_TMK();
                                tmkDtlReqInfo.faxId = (decimal)dtlRow["FAX_ID"];
                                tmkDtlReqInfo.faxDtlId = (decimal)dtlRow["SEQ"];
                                tmkDtlReqInfo.strStateEach = dtlRow["STATE_EACH"].ToString();
								tmkDtlReqInfo.strFaxNo = dtlRow["FAX_NO"].ToString();
                                tmkDtlReqInfo.strRecipientName = dtlRow["RECIPIENT_NAME"].ToString();
                                tmkDtlReqInfo.strTiffPath = dtlRow["TIF_FILE"].ToString();
                                tmkDtlReqInfo.strProcessFetchEach = dtlRow["PROCESS_FETCH_EACH"].ToString();

                                //dcvReq.m_lstSendRequestDetails.Add(dtlReq);
                                tmkReqInfo.m_lstTmkSendRequestDtlInfos.Add(tmkDtlReqInfo);
                            }

                            p_lstTmkSendReqInfos.Add(tmkReqInfo);
                        }

                        ret = RESULT.SUCCESS;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    ret = RESULT.F_SYSTEM_ERROR;
                }

                Close();
                return ret;
            }
        }

        public RESULT GetTiffMakingInfo(ref DbModule.SEND_REQUEST_TMK p_tmkReqinfo)
        {
            RESULT ret = RESULT.EMPTY;
            string strProcedureName = "PKG_PRC_TMK.USP_SELECT_TIF_MAKING_INFO";

            lock (m_connection)
            {
                if (!ReOpen())
                    return RESULT.F_DB_OPENERROR;
                try
                {
                    using (OracleCommand command = new OracleCommand())
                    {
                        command.Connection = m_connection;
                        command.CommandText = strProcedureName;
                        command.CommandType = CommandType.StoredProcedure;

                        command.Parameters.AddWithValue("P_FAX_ID", p_tmkReqinfo.faxId);
                        command.Parameters.AddWithValue("P_FETCH_PROCESS", Config.SYSTEM_PROCESS_ID);
                        command.Parameters.AddWithValue("P_TR_NO", p_tmkReqinfo.strTrNo);

                        OracleParameter param = new OracleParameter("P_TABLE1", OracleType.Cursor);
                        param.Direction = ParameterDirection.Output;
                        command.Parameters.Add(param);

                        param = new OracleParameter("P_TABLE2", OracleType.Cursor);
                        param.Direction = ParameterDirection.Output;
                        command.Parameters.Add(param);

                        param = new OracleParameter("P_TABLE3", OracleType.Cursor);
                        param.Direction = ParameterDirection.Output;
                        command.Parameters.Add(param);

                        command.ExecuteNonQuery();

                        DataTable dtDoc = new DataTable("DOC");
                        OracleDataReader reader = (OracleDataReader)command.Parameters["P_TABLE1"].Value;
                        dtDoc.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        DataTable dtFaxform = new DataTable("FAX_FORM_MAP");
                        reader = (OracleDataReader)command.Parameters["P_TABLE2"].Value;
                        dtFaxform.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        DataTable dtData = new DataTable("DATA");
                        reader = (OracleDataReader)command.Parameters["P_TABLE3"].Value;
                        dtData.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        if (dtFaxform.Rows.Count <= 0 || dtData.Rows.Count <= 0 )
                        {
                            if (dtDoc != null) dtDoc.Dispose();
                            if (dtFaxform != null) dtFaxform.Dispose();
                            if (dtData != null) dtData.Dispose();
                            return RESULT.EMPTY;
                        }

                        DataRow[] faxFormRows = dtFaxform.Select(String.Format("TR_NO='{0}'", p_tmkReqinfo.strTrNo.Trim()));
                        foreach (DataRow faxFormRow in faxFormRows)
                        {
                            p_tmkReqinfo.m_strFaxFormFileName = faxFormRow["FAX_FORM_FILE"].ToString();
                            p_tmkReqinfo.m_strFaxFormPath = faxFormRow["FAX_FORM_PATH"].ToString();
                            break;
                        }

                        DataRow[] dataRows = dtData.Select(String.Format("FAX_ID={0}", p_tmkReqinfo.faxId));
                        foreach (DataRow dataRow in dataRows)
                        {
                            p_tmkReqinfo.m_strPacket = dataRow["PACKET_XML"].ToString();
                            break;
                        }
                        if (string.IsNullOrEmpty(p_tmkReqinfo.m_strPacket))
                            return RESULT.F_DB_NOTEXIST_SENDDATA;



                        DataRow[] docRows = dtDoc.Select(String.Format("FAX_ID={0}", p_tmkReqinfo.faxId));
                        foreach (DataRow docRow in docRows)
                        {
                            DOC_INFO_ALL_TMK tmkDocInfo = new DOC_INFO_ALL_TMK();
                            tmkDocInfo.seq = (decimal)docRow["SEQ"];
                            tmkDocInfo.strDocPath = docRow["DOC_PATH"].ToString();
                            tmkDocInfo.strDocFile = docRow["DOC_FILE"].ToString();
                            tmkDocInfo.strDocExt = docRow["DOC_EXT"].ToString();
                            tmkDocInfo.strProcessingMode = docRow["PROCESSING_MODE"].ToString();
                            
                            tmkDocInfo.strTiffExtractPages = docRow["TIF_EXTRACT_PAGE"].ToString();
                            if (string.IsNullOrEmpty(tmkDocInfo.strTiffExtractPages))
                                tmkDocInfo.strTiffExtractPages = "all";

                            tmkDocInfo.processingState = docRow["PROCESSING_STATE"].ToString();
                            if (string.IsNullOrEmpty(tmkDocInfo.processingState))
                                tmkDocInfo.processingState = "z";

                            p_tmkReqinfo.m_tmkDocInfoAll = tmkDocInfo;

                            break;
                        }
                        
                        ret = RESULT.SUCCESS;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    ret = RESULT.F_DB_ERROR;
                }

                Close();
                return ret;
            }
        }

        
        #endregion

        /*
        public RESULT FetchSendListToMake(OutFaxQueue p_sendQueue)
        {
            this.fetchedReqs.Clear();

            lock (this)
            {
                if (!ReOpen())
                    return RESULT.F_DB_OPENERROR;

                string strSql = "";

                // Send 건 점유
                int cnt = OccupySendReqs(Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, Config.FETCH_CNT);
                
                // Send 건 얻기
                string strTable = "";                
                if(Config.FAX_TABLE_DIVISION=="Y")
                    strTable = string.Format("FAX_SEND_{0:yyyy_MM}", DateTime.Now);
                else
                    strTable = "FAX_SEND_MSTR";
                
                strSql = string.Format("SELECT ID_REQ, ID_REQ_SITE, STATE, TR_NO, FAX_NO, FAX_TO_NAME, REQ_TYPE, REQ_DATE, REQ_TIME, APPROVED_YN, LAST_TIFF_FILE, TIFF_MAKE_TEST from {0} where FETCH_PROCESS = '{1}' and STATE = '{2}' order by ID_REQ",
                                        strTable,
                                        Config.SYSTEM_PROCESS_ID,
                                        (int)DbModuleBase.R_STATE.MAKE_W
                                        );
                using (OracleDataReader dr = ExecuteReader(strSql))
                {
                    if (dr == null || !dr.HasRows)
                        return RESULT.EMPTY;

                    // Send Queue 에 Add
                    bool fetch = false;
                    string strFaxNum = "";
                    while (dr.Read())
                    {   
                        OutFaxQueue.SEND_REQ sendItem = new OutFaxQueue.SEND_REQ();

                        sendItem.idReq = dr.GetDecimal(0);
                        sendItem.strIdSite = dr.GetString(1);
                        sendItem.strState = dr.GetString(2);
                        sendItem.strTrNo = dr.GetString(3);
                        sendItem.strFaxNo = dr.GetString(4);
                        if (!dr.IsDBNull(5))
                            sendItem.strFaxToName = dr.GetString(5);
                        else
                            sendItem.strFaxToName = "NoName";
                        sendItem.strReqType = dr.GetString(6);
                        sendItem.strReqDate = dr.GetString(7);
                        sendItem.strReqTime = dr.GetString(8);
                        sendItem.ApproveYN = dr.GetString(9);
                        sendItem.strLastTiffFile = dr.GetString(10);
                        sendItem.tiffMakeTest = dr.GetString(11);

                        fetchedReqs.Add(sendItem);
                        fetch = true;
                    }

                    if (!fetch)
                        return RESULT.F_DB_ADO_NOTEXPECTED_RESULT;
                }

                foreach (OutFaxQueue.SEND_REQ sendReq in this.fetchedReqs)
                {
                    #region ADD - KCG : 20110706 @승인이 되지 않았을때 처리 
                    try { 
                        if (sendReq.ApproveYN == "N") {
                            continue;
                        }
                    }
                    catch { continue; }
                    #endregion

                    // 작업시작 DB 반영
                    if (DbModule.Instance.RunningSendReq(sendReq.idReq, Config.PROCESS_TYPE) <= 0)
                    {
                        AppLog.Instance.Write(LOG_LEVEL.ERR, "[ID={0}] 작업중 상태를 DB에 반영하는 도중 오류가 발생하였습니다", sendReq.idReq);
                        continue;
                    }

                    p_sendQueue.Enqueue(sendReq);
                }
            }

            return RESULT.SUCCESS;
        }


        /// <summary>
        /// GET사인파일명
        /// </summary>
        public void GetSignFileName(decimal p_idReq, out string p_strValue)
        {
            p_strValue = "";

            lock (this)
            {
                if (!ReOpen())
                    return;

                
                string strTable = string.Format("FAX_SEND_EXT_{0:yyyy_MM}", DateTime.Now);
                string strSql = string.Format("SELECT SIGN_FILENAME FROM {0} WHERE SEND_ID_REQ = '{1}'",
                                        strTable,
                                        p_idReq
                                        );
                
                using (OracleDataReader dr = ExecuteReader(strSql))
                {
                    if (dr == null || !dr.HasRows)
                        return;

                    if (dr.Read()) {
                        try {
                            p_strValue = dr.GetString(0);
                        }
                        catch {}
                    }
                }
            }
        }

        public bool GetFaxFormInfo(string p_strTrno, out string p_strFaxFormFile, out string p_strFaxFormPath)
        {
            p_strFaxFormFile = "";
            p_strFaxFormPath = "";

            lock (this)
            {
                if (!ReOpen())
                    return false;

                string strSql = string.Format("select TR_NO, FAX_FORM_FILE, FAX_FORM_PATH from BTFAX.FAX_FORM_MAP where TR_NO = '{0}'", p_strTrno);
                using (OracleDataReader dr = ExecuteReader(strSql))
                {
                    if (dr == null || !dr.HasRows)
                        return false;

                    if (dr.Read())
                    {
                        p_strFaxFormFile = dr.GetString(1);
                        p_strFaxFormPath = dr.GetString(2);
                    }
                }
            }

            return true;
        }

        public bool GetSendReqData(decimal p_idReq, out string p_strPacketXml)
        {
            p_strPacketXml = "";

            lock (this)
            {
                if (!ReOpen())
                    return false;

                string strTable = "";
                if (Config.FAX_TABLE_DIVISION == "N")
                    strTable = "FAX_SEND_DATA";
                else
                    strTable = string.Format("FAX_SEND_DATA_{0:yyyy_MM}", DateTime.Now);

                string strSql = string.Format("select ID_REQ, PACKET_XML from {0} where ID_REQ = '{1}'", strTable, p_idReq);
                using (OracleDataReader dr = ExecuteReader(strSql))
                {
                    if (dr == null || !dr.HasRows)
                        return false;

                    if (!dr.Read())
                        return false;
                    p_strPacketXml = dr.GetString(1);
                    
                }
            }
            return true;
        }

        public bool GetSendDoc(decimal p_idReq, out DOC_INFO_ALL p_docInfoAll)
        {
            p_docInfoAll = null;

            lock (this)
            {
                if (!ReOpen())
                    return false;

                string strTable = "";
                if (Config.FAX_TABLE_DIVISION == "N")
                    strTable = "FAX_SEND_DOC";
                else
                    strTable = string.Format("FAX_SEND_DOC_{0:yyyy_MM}", DateTime.Now);
                
                string strSql = string.Format("select SEQ, DOC_PATH, DOC_FILE, DOC_EXT, PROCESSING_MODE, TIFF_EXTRACT_PAGE, PROCESSING_STATE from {0} where ID_REQ = {1} and PROCESSING_MODE = '01' order by SEQ",
                                        strTable,
                                        p_idReq);
                using (OracleDataReader dr = ExecuteReader(strSql))
                {
                    if (dr == null || !dr.HasRows)
                        return false;
                    
                    if (!dr.Read())
                        return false;

                    p_docInfoAll = new DOC_INFO_ALL();
                    p_docInfoAll.seq = dr.GetDecimal(0);
                    p_docInfoAll.strDocPath = dr.GetString(1);
                    p_docInfoAll.strDocFile = dr.GetString(2);
                    p_docInfoAll.strDocExt = dr.GetString(3);
                    p_docInfoAll.strProcessingMode = dr.GetString(4);
                    try { p_docInfoAll.strTiffExtractPages = dr.GetString(5); }
                    catch { p_docInfoAll.strTiffExtractPages = "all"; }
                    if (!dr.IsDBNull(6))
                    {
                        string strTemp = dr.GetString(6);
                        p_docInfoAll.processingState = strTemp[0];
                    }
                    else
                    {
                        p_docInfoAll.processingState = 'z';
                    }
                }
            }

            return true;
        }

        public override bool InsertSendDoc(decimal p_idReq, P_TYPE p_processType, string p_strDocPath, DOC_INFO p_docInfo)
        {
            lock (this)
            {
                if (!ReOpen())
                    return false;

                return base.InsertSendDoc(p_idReq, p_processType, p_strDocPath, p_docInfo);
            }
        }

        public override bool FinishProcessingDocAndUpdate(P_TYPE p_processType, DOC_INFO_ALL p_docInfoAll)
        {
            lock (this)
            {
                if (!ReOpen())
                    return false;

                return base.FinishProcessingDocAndUpdate(p_processType, p_docInfoAll);
            }
        }

        // ADD - KCG : 20110802
        public override bool UpdateTifPageCount(decimal p_idReq, int p_nPageCnt)
        {
            lock (this)
            {
                if (!ReOpen())
                    return false;

                return base.UpdateTifPageCount(p_idReq, p_nPageCnt);
            }
        }
        // ADD - END


        #region FIELDS
        List<OutFaxQueue.SEND_REQ> fetchedReqs = new List<OutFaxQueue.SEND_REQ>();
        #endregion
        */
    }
}

