using System;
using System.Collections.Generic;
using System.Data;
using System.Data.OracleClient;
using Btfax.CommonLib;
using Btfax.CommonLib.Db;

namespace PostFcs.DB
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
        
        #region inner class
        public class SEND_REQUEST_DTL_PSC : SEND_REQUEST_DTL
        {
            public SEND_REQUEST_DTL_PSC()
            {
                base.Clear();
            }

            public string strLastPagesSent { get; set; }
        }

        public class SEND_REQUEST_PSC : SEND_REQUEST
        {
            public SEND_REQUEST_PSC()
            {   
                m_lstSendRequestDtlPscInfos.Clear();
                base.Clear();
            }

            #region field
            public List<SEND_REQUEST_DTL_PSC> m_lstSendRequestDtlPscInfos = new List<SEND_REQUEST_DTL_PSC>();
            #endregion
        }
        #endregion

        #region method
        public RESULT FetchPostProcessingRequest(P_TYPE p_processType, string p_strSystemProcessId, int p_occupyCnt, ref List<SEND_REQUEST_PSC> p_lstSendRequests)
        {
            RESULT ret = RESULT.EMPTY;
            string strProcedureName = "PKG_PRC_PSC.USP_OCCUPY_SEND_REQ";
            
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

                        if (dtMstr.Rows.Count < 1 || dtDtl.Rows.Count < 1)
                        {
                            if (dtMstr != null) dtMstr.Dispose();
                            if (dtDtl != null) dtDtl.Dispose();
                            return RESULT.EMPTY;
                        }

                        //// BTF_FAX_SEND_MSTR ////
                        foreach (DataRow mstrRow in dtMstr.Rows)
                        {
                            SEND_REQUEST_PSC pscReq = new SEND_REQUEST_PSC();
                            pscReq.faxId = (decimal)mstrRow["FAX_ID"];
                            pscReq.strTrNo = mstrRow["TR_NO"].ToString();
                            pscReq.strState = mstrRow["STATE"].ToString();
                            pscReq.strReqType = mstrRow["REQ_TYPE"].ToString();
                            pscReq.previewYN = mstrRow["PREVIEW_REQ"].ToString();
                            pscReq.approvedReq = mstrRow["APPROVE_REQ"].ToString();
                            pscReq.reserveYN = mstrRow["RESERVED_YN"].ToString();
                            pscReq.strReserveDate = mstrRow["RESERVED_DATE"].ToString();
                            pscReq.strBroadcastYN = mstrRow["BROADCAST_YN"].ToString();
                            pscReq.strReqUserTelNo = mstrRow["REQ_USER_TELNO"].ToString();

                            //// BTF_FAX_SEND_DTL ////
                            DataRow[] dtlRows = dtDtl.Select(String.Format("FAX_ID={0}", pscReq.faxId));
                            foreach (DataRow dtlRow in dtlRows)
                            {
                                SEND_REQUEST_DTL_PSC dtlReq = new SEND_REQUEST_DTL_PSC();
                                dtlReq.faxId = (decimal)dtlRow["FAX_ID"];
                                dtlReq.faxDtlId = (decimal)dtlRow["SEQ"];
                                dtlReq.strStateEach = dtlRow["STATE_EACH"].ToString();
                                dtlReq.strFaxNo = dtlRow["FAX_NO"].ToString();
                                dtlReq.strRecipientName = dtlRow["RECIPIENT_NAME"].ToString();
                                dtlReq.strTiffPath = dtlRow["TIF_FILE"].ToString();
                                dtlReq.strProcessFetchEach = dtlRow["PROCESS_FETCH_EACH"].ToString();
                                dtlReq.strLastPagesSent = dtlRow["LAST_PAGE_SENT"].ToString();

                                //dcvReq.m_lstSendRequestDetails.Add(dtlReq);
                                pscReq.m_lstSendRequestDtlPscInfos.Add(dtlReq);
                            }

                            p_lstSendRequests.Add(pscReq);
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

        public bool UpdateTifInfo(decimal p_faxId, decimal p_faxDtlId, string p_strTifFile, int p_nTifSize, int p_nTifPageCnt)
        {
            bool ret = false;
            string strProcedureName = "PKG_PRC_PSC.USP_UPDATE_TIF_INFO";
            lock (m_connection)
            {
                if (!ReOpen())
                    return false;
                try
                {
                    using (OracleCommand dbCommand = new OracleCommand())
                    {
                        dbCommand.Connection = m_connection;
                        dbCommand.CommandText = strProcedureName;
                        dbCommand.CommandType = CommandType.StoredProcedure;

                        dbCommand.Parameters.AddWithValue("P_FAX_DTL_ID", p_faxDtlId);
                        dbCommand.Parameters.AddWithValue("P_FAX_ID", p_faxId);
                        dbCommand.Parameters.AddWithValue("P_TIF_FILE_SIZE", p_nTifSize);
                        dbCommand.Parameters.AddWithValue("P_TIF_PAGE_CNT", p_nTifPageCnt);

                        int nCnt = dbCommand.ExecuteNonQuery();
                        if (nCnt < 0)
                            ret = false;
                        else
                            ret = true;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    ret = false;
                }
            }
            Close();
            return ret;
        }

        public bool IsInboundFaxNo(string p_strFaxNo, ref string p_strDid)
        {
            string strProcedureName = "PKG_PRC_PSC.USP_CHECK_IB_FAXNO";

            int cnt = 0;
            bool bResult;

            bResult = true;
            lock (m_connection)
            {
                if (!ReOpen())
                    return false;

                try
                {
                    using (OracleCommand command = new OracleCommand())
                    {
                        command.Connection = m_connection;
                        command.CommandText = strProcedureName;
                        command.CommandType = CommandType.StoredProcedure;

                        command.Parameters.AddWithValue("P_FAX_NO", p_strFaxNo);

                        OracleParameter param = new OracleParameter("P_TABLE1", OracleType.Cursor);
                        param.Direction = ParameterDirection.Output;
                        command.Parameters.Add(param);

                        cnt = command.ExecuteNonQuery();

                        OracleDataReader reader = (OracleDataReader)command.Parameters["P_TABLE1"].Value;
                        DataTable dtInbound = new DataTable("IB_FAXNO");
                        dtInbound.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        if (dtInbound.Rows.Count <= 0)
                            bResult = false;

                        if (dtInbound != null)
                            dtInbound.Dispose();
                                               
                        foreach (DataRow mstrRow in dtInbound.Rows)
                        {
                            p_strDid = mstrRow["DID01"].ToString();
                        }

                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    bResult = false;
                }

                Close();
            }

            return bResult;
        }

        public decimal InsertRecvMaster(long p_systemId, long p_procNo, long lTiffFileSize, int nTiffPageCnt, string strReqUserTelno, string p_strDid, SEND_REQUEST_DTL_PSC p_sendRequestInfo)
        {
            string strProcedureName = "PKG_PRC_FID.USP_INSERT_RECV_MSTR_EX";
            decimal faxID = -1;

            lock (m_connection)
            {
                if (!ReOpen())
                    return -1;

                try
                {
                    using (OracleCommand command = new OracleCommand())
                    {
                        command.Connection = m_connection;
                        command.CommandText = strProcedureName;
                        command.CommandType = CommandType.StoredProcedure;
                        command.Parameters.AddWithValue("P_RECV_TYPE"       , "I");
                        command.Parameters.AddWithValue("P_SYSTEM_ID"       , p_systemId);
                        command.Parameters.AddWithValue("P_PROCESS_ID"      , p_procNo);
                        command.Parameters.AddWithValue("P_MGW_IP"          , DBNull.Value);
                        command.Parameters.AddWithValue("P_MGW_PORT"        , DBNull.Value);
                        command.Parameters.AddWithValue("P_CHANNEL"         , 999);
                        command.Parameters.AddWithValue("P_CID"             , strReqUserTelno);
                        command.Parameters.AddWithValue("P_DID"             , p_strDid);
                        command.Parameters.AddWithValue("P_TIF_FILE"        , p_sendRequestInfo.strTiffPath);
                        command.Parameters.AddWithValue("P_TIF_FILE_SIZE"   , lTiffFileSize);
                        command.Parameters.AddWithValue("P_TIF_PAGE_CNT"    , nTiffPageCnt);
                        
                        OracleParameter param = new OracleParameter("P_OUT_FAX_ID", OracleType.Number);
                        param.Direction = ParameterDirection.Output;
                        command.Parameters.Add(param);

                        int cnt = command.ExecuteNonQuery();
                        if (cnt < 1)
                            return -1;

                        faxID = Convert.ToDecimal(param.Value);
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    return -1;
                }

                Close();

                return faxID;
            }
        }

        public bool InsertFaxbox(decimal deFaxId)
        {
            string strProcedureName = "PKG_PRC_PSC.USP_INSERT_FAXBOX";
            
            lock (m_connection)
            {
                if (!ReOpen())
                    return false;

                try
                {
                    using (OracleCommand command = new OracleCommand())
                    {
                        command.Connection = m_connection;
                        command.CommandText = strProcedureName;
                        command.CommandType = CommandType.StoredProcedure;
                        command.Parameters.AddWithValue("P_FAX_ID", deFaxId);

                        OracleParameter param = new OracleParameter("P_RESULT", OracleType.VarChar, 100);
                        param.Direction = ParameterDirection.Output;
                        command.Parameters.Add(param);

                        int cnt = command.ExecuteNonQuery();
                        if (cnt < 1)
                            return false;

                        if(param.ToString().Substring(0, 1) == "F")
                            return false;

                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    return false;
                }

                Close();

                return true;
            }
        }

        public bool FinishIBSendDtl(long p_systemNo, P_TYPE p_processType, string strSystemProcessID, string strBroadcastYN, SEND_REQUEST_DTL_PSC p_sendRequestInfo, RESULT p_result = RESULT.SUCCESS)
        {
            string strProcedureName = "PKG_PRC_FOD.USP_FINISH_SEND_DTL";

            lock (m_connection)
            {
                if (!ReOpen())
                    return false;

                try
                {
                    using (OracleCommand command = new OracleCommand())
                    {
                        command.Connection = m_connection;
                        command.CommandText = strProcedureName;
                        command.CommandType = CommandType.StoredProcedure;
                        command.Parameters.AddWithValue("P_FAX_ID", p_sendRequestInfo.faxId);
                        command.Parameters.AddWithValue("P_FAX_DTL_ID", p_sendRequestInfo.faxDtlId);
                        command.Parameters.AddWithValue("P_PROCESS_TYPE", p_processType.ToString());
                        command.Parameters.AddWithValue("P_SYSTEM_ID", p_systemNo);
                        command.Parameters.AddWithValue("P_SYSTEM_PROCESS_ID", strSystemProcessID);
                        command.Parameters.AddWithValue("P_FOD_CHANNEL", 99999);
                        command.Parameters.AddWithValue("P_RESULT_CODE_3", string.Format("{0:D03}", (int)p_result));
                        command.Parameters.AddWithValue("P_BROADCAST_YN", strBroadcastYN);
                        command.Parameters.AddWithValue("P_LAST_PAGE_SENT", p_sendRequestInfo.strLastPagesSent);
                        
                        int cnt = command.ExecuteNonQuery();
                        if (cnt < 1)
                            return false;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    return false;
                }

                Close();

                return true;
            }
        }

        public bool UpdatePreviewInfo(decimal p_faxId, string strPreviewReq)
        {
            bool ret = false;
            string strProcedureName = "PKG_PRC_PSC.USP_UPDATE_PREVIEW_INFO";
            lock (m_connection)
            {
                if (!ReOpen())
                    return false;
                try
                {
                    using (OracleCommand dbCommand = new OracleCommand())
                    {
                        dbCommand.Connection = m_connection;
                        dbCommand.CommandText = strProcedureName;
                        dbCommand.CommandType = CommandType.StoredProcedure;

                        dbCommand.Parameters.AddWithValue("P_FAX_ID", p_faxId);
                        dbCommand.Parameters.AddWithValue("P_PREVIEW_REQ", strPreviewReq);
                        
                        int nCnt = dbCommand.ExecuteNonQuery();
                        if (nCnt < 0)
                            ret = false;
                        else
                            ret = true;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    ret = false;
                }
            }
            Close();
            return ret;
        }


        public bool UpdateRecvStateRunning(decimal p_faxId)
        {
            bool ret = false;
            string strProcedureName = "PKG_PRC_FID.USP_UPDATE_RECV_STATE_RUNNING";
            lock (m_connection)
            {
                if (!ReOpen())
                    return false;
                try
                {
                    using (OracleCommand dbCommand = new OracleCommand())
                    {
                        dbCommand.Connection = m_connection;
                        dbCommand.CommandText = strProcedureName;
                        dbCommand.CommandType = CommandType.StoredProcedure;

                        dbCommand.Parameters.AddWithValue("P_FAX_ID", p_faxId);

                        int nCnt = dbCommand.ExecuteNonQuery();
                        if (nCnt < 0)
                            ret = false;
                        else
                            ret = true;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    ret = false;
                }
            }
            Close();
            return ret;
        }


        public bool UpdateRecvStateFinished(decimal p_faxId, int p_result, int p_reason, string p_tifFileName, long lTiffFileSize, int nTiffPageCnt)
        {
            bool ret = false;
            string strProcedureName = "PKG_PRC_FID.USP_UPDATE_RECV_FINISHED";
            lock (m_connection)
            {
                if (!ReOpen())
                    return false;
                try
                {
                    string strReason = string.Format("{0:D04}", p_reason);
                    using (OracleCommand dbCommand = new OracleCommand())
                    {
                        dbCommand.Connection = m_connection;
                        dbCommand.CommandText = strProcedureName;
                        dbCommand.CommandType = CommandType.StoredProcedure;

                        dbCommand.Parameters.AddWithValue("P_FAX_ID"        , p_faxId);
                        dbCommand.Parameters.AddWithValue("P_RESULT"        , p_result);
                        dbCommand.Parameters.AddWithValue("P_REASON"        , strReason);
                        dbCommand.Parameters.AddWithValue("P_TIF_FILE"      , p_tifFileName);
                        dbCommand.Parameters.AddWithValue("P_TIF_FILE_SIZE" , lTiffFileSize);
                        dbCommand.Parameters.AddWithValue("P_TIF_PAGE_CNT"  , nTiffPageCnt);

                        int nCnt = dbCommand.ExecuteNonQuery();
                        if (nCnt < 0)
                            ret = false;
                        else
                            ret = true;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    ret = false;
                }
            }
            Close();
            return ret;
        }

        #endregion
    }
}
