using System;
using System.Collections.Generic;
using System.Data;
using System.Data.OracleClient;
using Btfax.CommonLib;
using Btfax.CommonLib.Db;
using PostFOD.Util;

namespace PostFOD.Db
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
        public class SEND_REQUEST_DATA_PSO : SEND_REQUEST
        {
            public SEND_REQUEST_DATA_PSO()
            {
                base.Clear();
				strDeptCode = "";
				strDeptName = "";
				strUserName = "";

                m_lstSendRequestDetails.Clear();
                //m_siteReq = null;
            }

            ///// BTF_FAX_SEND_DTL_DATA 리스트 ////
            public List<SEND_REQUEST_DTL_DATA_PSO> m_lstSendRequestDetails = new List<SEND_REQUEST_DTL_DATA_PSO>();

			public string strDeptCode = "";
			public string strDeptName = "";
			public string strUserName = "";
        }

        public class SEND_REQUEST_DTL_DATA_PSO : SEND_REQUEST_DTL
        {
            public SEND_REQUEST_DTL_DATA_PSO()
            {   
                base.Clear();
                this.Clear();
            }

            private void Clear()
            {
                strResult = "";
                strReason = "";
				strCIDName = "";
            }

            public string strResult = "";
            public string strReason = "";
			public string strCIDName = "";
        }


        #endregion
        
        #region method
        public RESULT FetchPostFODReqs(ref List<SEND_REQUEST_DATA_PSO> p_lstSendRequests)
        {
            string strProcedureName = "PKG_PRC_PSO.USP_OCCUPY_SEND_REQ";
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


                        command.ExecuteNonQuery();

                        OracleDataReader reader = null;
                        DataTable dtMstr = new DataTable("MSTR_DATA");
                        reader = (OracleDataReader)command.Parameters["P_TABLE1"].Value;
                        dtMstr.Load(reader);
                        reader.Close();
                        reader.Dispose();

                        DataTable dtDtl = new DataTable("DTL_DATA");
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

                        //// BTF_FAX_SEND_MSTR_DATA ////
                        foreach (DataRow mstrRow in dtMstr.Rows)
                        {
                            SEND_REQUEST_DATA_PSO sendReq    = new SEND_REQUEST_DATA_PSO();
                            sendReq.faxId               = (decimal)mstrRow["FAX_ID"];
                            sendReq.strIndexNo          = mstrRow["INDEX_NO"].ToString();
                            sendReq.strTrNo             = mstrRow["TR_NO"].ToString();
                            sendReq.strState            = mstrRow["STATE"].ToString();
							sendReq.strReqUserID		= mstrRow["REQ_USER_ID"].ToString();
							sendReq.strReqUserName		= mstrRow["REQ_USER_NAME"].ToString();
							sendReq.strUserName			= mstrRow["USER_NAME"].ToString();
							sendReq.strReqUserTelNo		= mstrRow["CID"].ToString();
							sendReq.strDeptCode			= mstrRow["DEPT_CD"].ToString();
							sendReq.strDeptName			= mstrRow["DEPT_NAME"].ToString();
                            sendReq.strReqType          = mstrRow["REQ_TYPE"].ToString();
                            sendReq.strReqSendType      = mstrRow["REQUESTER_TYPE"].ToString();
                            sendReq.previewYN           = mstrRow["PREVIEW_REQ"].ToString();
                            sendReq.approvedReq         = mstrRow["APPROVE_REQ"].ToString();
                            p_lstSendRequests.Add(sendReq);

                            //// BTF_FAX_SEND_DTL_DATA ////
                            DataRow[] dtlRows = dtDtl.Select(String.Format("FAX_ID={0}", sendReq.faxId));
                            foreach (DataRow dtlRow in dtlRows)
                            {
                                SEND_REQUEST_DTL_DATA_PSO dtlReq = new SEND_REQUEST_DTL_DATA_PSO();
                                dtlReq.faxId                = (decimal)dtlRow["FAX_ID"];
                                dtlReq.faxDtlId             = (decimal)dtlRow["SEQ"];                                
                                dtlReq.strFaxNo             = dtlRow["FAX_NO"].ToString();
                                dtlReq.strStateEach         = dtlRow["STATE_EACH"].ToString();
                                dtlReq.strRecipientName     = dtlRow["RECIPIENT_NAME"].ToString();
                                dtlReq.strTiffPath          = dtlRow["TIF_FILE"].ToString();
                                int idx = dtlReq.strTiffPath.LastIndexOf("\\");
                                if (idx > 0)
                                    dtlReq.strTiffName = dtlReq.strTiffPath.Substring(idx + 1);

								dtlReq.strCIDName			= dtlRow["CID_NAME"].ToString();
                                dtlReq.strResult            = dtlRow["RESULT"].ToString();
                                dtlReq.strReason            = dtlRow["REASON"].ToString();

                                sendReq.m_lstSendRequestDetails.Add(dtlReq);
                            }
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
      
        public bool ResultProcessingReq(decimal p_faxId, bool p_result)
        {
            lock (this)
            {
                if (!ReOpen())
                    return false;

                if (p_result)
                    return FinishProcessingDoc(Config.PROCESS_TYPE, p_faxId);
                else
                    return FailProcessingDoc(p_faxId);
            }
        }

        public bool ResultProcessingReq(decimal p_faxId, decimal p_faxDtlId, string p_result, string p_reason)
        {
            string strProcedureName = "PKG_PRC_PSO.USP_POST_OUTBOUND_DONE";
            lock (m_connection)
            {
                if (!ReOpen())
                    return false;

                bool ret = false;
                try
                {
                    using (OracleCommand command = new OracleCommand())
                    {
                        command.Connection = m_connection;
                        command.CommandText = strProcedureName;
                        command.CommandType = CommandType.StoredProcedure;

						command.Parameters.AddWithValue("P_FAX_ID", p_faxId);
						command.Parameters.AddWithValue("P_FAX_DTL_ID", p_faxDtlId);
                        command.Parameters.AddWithValue("P_RESULT", p_result);
                        command.Parameters.AddWithValue("P_REASON", p_reason);

                        int nCnt = command.ExecuteNonQuery();
                        if (nCnt <= 0)
                            throw new Exception("ResultProcessingFailed");

                        ret = true;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                    ret = false;
                }

                Close();
                return ret;
            }
        }
        #endregion
    }
}
