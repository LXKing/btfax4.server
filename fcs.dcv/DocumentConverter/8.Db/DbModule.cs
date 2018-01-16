using System;
using System.Collections.Generic;
using System.Data;
using System.Data.OracleClient;
using Btfax.CommonLib;
using Btfax.CommonLib.Db;
using DocumentConverter.Util;

namespace DocumentConverter.Db
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

        #region constructor
        public DbModule() { }
        #endregion

        #region inner class
        public class SEND_REQUEST_DTL_DCV : SEND_REQUEST_DTL
        {
            public SEND_REQUEST_DTL_DCV()
            {
                base.Clear();
            }
        }

        public class DOC_INFO_ALL_DCV : DOC_INFO_ALL
        {
            public DOC_INFO_ALL_DCV()
            {
                base.Clear();
            }
        }
        
        public class SEND_REQUEST_DCV : SEND_REQUEST
        {
            public SEND_REQUEST_DCV() 
            {
                m_lstDocInfoAllDcv.Clear();
                m_lstSendRequestDtlDcvInfos.Clear();
                base.Clear();
            }

            #region field
            public List<SEND_REQUEST_DTL_DCV> m_lstSendRequestDtlDcvInfos = new List<SEND_REQUEST_DTL_DCV>();
            public List<DOC_INFO_ALL_DCV> m_lstDocInfoAllDcv = new List<DOC_INFO_ALL_DCV>();
            #endregion
        }
        #endregion

        #region method
        public RESULT FetchSendReqToConvert(ref List<DbModule.SEND_REQUEST_DCV> p_lstSendReqDcvInfos)
        {
            string strProcedureName = "PKG_PRC_DCV.USP_OCCUPY_SEND_REQ";
            RESULT ret = RESULT.EMPTY;      
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

                        command.Parameters.AddWithValue("P_PROCESS_TYPE", Config.PROCESS_TYPE.ToString());
                        command.Parameters.AddWithValue("P_FETCH_PROCESS", Config.SYSTEM_PROCESS_ID);
                        command.Parameters.AddWithValue("P_OCCUPY_CNT", Config.FETCH_CNT);
                        
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

                        OracleDataReader reader = null;
                        DataTable dtMstr = new DataTable("MSTR");
                        reader = (OracleDataReader)command.Parameters["P_TABLE1"].Value;
                        dtMstr.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        DataTable dtDtl = new DataTable("DETAIL");
                        reader = (OracleDataReader)command.Parameters["P_TABLE2"].Value;
                        dtDtl.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        DataTable dtDoc = new DataTable("DOC");
                        reader = (OracleDataReader)command.Parameters["P_TABLE3"].Value;
                        dtDoc.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        if (dtMstr.Rows.Count <= 0 || dtDtl.Rows.Count <= 0 || dtDoc.Rows.Count <= 0)
                        {
                            if (dtMstr != null) dtMstr.Dispose();
                            if (dtDtl != null) dtDtl.Dispose();
                            if (dtDoc != null) dtDoc.Dispose();
                            return RESULT.EMPTY;
                        }

                        //// BTF_FAX_SEND_MSTR ////
                        foreach (DataRow mstrRow in dtMstr.Rows)
                        {
                            DbModule.SEND_REQUEST_DCV dcvReq = new DbModule.SEND_REQUEST_DCV();
                            dcvReq.faxId = (decimal)mstrRow["FAX_ID"];
                            dcvReq.strIndexNo = mstrRow["INDEX_NO"].ToString();
                            dcvReq.strTrNo = mstrRow["TR_NO"].ToString();
                            dcvReq.strState = mstrRow["STATE"].ToString();
                            dcvReq.strReqType = mstrRow["REQ_TYPE"].ToString();
                            DateTime dt = (DateTime)mstrRow["REQ_DATE"];
                            dcvReq.previewYN = mstrRow["PREVIEW_REQ"].ToString();
                            dcvReq.approvedReq = mstrRow["APPROVE_REQ"].ToString();

                            //// BTF_FAX_SEND_DTL ////
                            DataRow[] dtlRows = dtDtl.Select(String.Format("FAX_ID={0}", dcvReq.faxId));
                            foreach (DataRow dtlRow in dtlRows)
                            {
                                SEND_REQUEST_DTL_DCV dtlReq = new SEND_REQUEST_DTL_DCV();
                                dtlReq.faxId = (decimal)dtlRow["FAX_ID"];
                                dtlReq.faxDtlId = (decimal)dtlRow["SEQ"];
                                dtlReq.strStateEach = dtlRow["STATE_EACH"].ToString();
                                dtlReq.strFaxNo = dtlRow["FAX_NO"].ToString();
                                dtlReq.strRecipientName = dtlRow["RECIPIENT_NAME"].ToString();
                                dtlReq.strTiffPath = dtlRow["TIF_FILE"].ToString();
                                dtlReq.strProcessFetchEach = dtlRow["PROCESS_FETCH_EACH"].ToString();

                                dcvReq.m_lstSendRequestDtlDcvInfos.Add(dtlReq);
                            }

                            //// BTF_FAX_SEND_DOC ////
                            DataRow[] docRows = dtDoc.Select(String.Format("FAX_ID={0}", dcvReq.faxId));
                            foreach (DataRow docRow in docRows)
                            {
                                DOC_INFO_ALL_DCV docInfo = new DOC_INFO_ALL_DCV();
                                docInfo.seq = (decimal)docRow["SEQ"];
                                docInfo.strDocPath = docRow["DOC_PATH"].ToString();
                                docInfo.strDocFile = docRow["DOC_FILE"].ToString();
                                docInfo.strDocExt = docRow["DOC_EXT"].ToString();
                                docInfo.strProcessingMode = docRow["PROCESSING_MODE"].ToString();
                                
                                docInfo.processingState = docRow["PROCESSING_STATE"].ToString();
                                if (string.IsNullOrEmpty(docInfo.processingState))
                                    docInfo.processingState = "N";

                                dcvReq.m_lstDocInfoAllDcv.Add(docInfo);
                            }

                            p_lstSendReqDcvInfos.Add(dcvReq);
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

        public bool ResultProcessingReq(decimal p_seq, bool p_result)
        {
            lock (this)
            {
                if (!ReOpen())
                    return false;

                if (p_result)
                    return FinishProcessingDoc(Config.PROCESS_TYPE, p_seq);
                else
                    return FailProcessingDoc(p_seq);
            }
        }

        public RESULT FailSendReqs(decimal p_faxId, decimal p_faxDtlId, P_TYPE p_processType, string p_processId, RESULT p_result)
        {
            //// DTL건 업데이트 ////
            int ret = FailSendReqDetail(p_faxId, p_faxDtlId, p_processType, p_processId, p_result);
            if (ret < 0)
                return RESULT.F_DB_UPDATE_ERROR;
            
            //// MSTR건 업데이트 ////
            ret = DbModule.Instance.FailSendReqMaster(p_faxId, p_processType, p_processId);
            if (ret < 0)
                return RESULT.F_DB_UPDATE_ERROR;

            return RESULT.SUCCESS;
        }
        #endregion

    }
}
