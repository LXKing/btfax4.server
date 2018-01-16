using System;
using System.Collections.Generic;
using System.Data;
using System.Data.OracleClient;
using Btfax.CommonLib.Db;

namespace AdapterOut.Db
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

        #region type
        // SST : Send Site Table
        public struct SST_COLUMN_MAP
        {
            public string strTrField;
            public string strTrFieldType;
            public string strTrFieldExtraInfo;
            public string strColumnName;
            public string strColumnType;
        }
        #endregion

        #region inner class
        public class SEND_REQUEST_DTL_AOB : SEND_REQUEST_DTL
        {
            public SEND_REQUEST_DTL_AOB()
            {
                base.Clear();
            }
        }

        public class SEND_REQUEST_AOB : SEND_REQUEST
        {
            public SEND_REQUEST_AOB()
            {   
                m_lstSendRequestDtlAobInfos.Clear();
                base.Clear();
            }

            #region field
            public List<SEND_REQUEST_DTL_AOB> m_lstSendRequestDtlAobInfos = new List<SEND_REQUEST_DTL_AOB>();
            #endregion
        }
        
        public class AddressInfo
        {
            #region constructor
            public AddressInfo()
            {
                Clear();
            }
            #endregion

            #region methods
            public void Clear()
            {
                strRecipientName = "";
                strFaxNo = "";
            }
            #endregion

            #region fields
            public string strRecipientName;
            public string strFaxNo;
            #endregion
        }        
        #endregion

        #region constructor
        public DbModule() { }
        #endregion

        #region method
        public virtual bool GetAddressInfos(string strReqUserID, string strShortNumber, ref List<AddressInfo> p_addrInfos)
        {
            string strProcedureName = "PKG_PRC_AOB.USP_SELECT_ADDR_GROUP_LST";
            bool ret = false;

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

                        dbCommand.Parameters.AddWithValue("P_REQ_USER_ID", strReqUserID);
                        dbCommand.Parameters.AddWithValue("P_SHORT_NUMBER", strShortNumber);

                        OracleParameter paramCursor = new OracleParameter("P_CURSOR", OracleType.Cursor);
                        paramCursor.Direction = ParameterDirection.Output;
                        dbCommand.Parameters.Add(paramCursor);

                        using (OracleDataReader reader = dbCommand.ExecuteReader())
                        {
                            while (reader.Read())
                            {
                                //// 동보발송 주소록 처리 ////
                                AddressInfo addrInfo = new AddressInfo();
                                addrInfo.strRecipientName = reader.GetString(0).Trim();
                                addrInfo.strFaxNo = reader.GetString(1).Trim();
                                p_addrInfos.Add(addrInfo);
                            }

                            reader.Close();
                        }
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

        public virtual bool GetAddressInfos_Dept(string strReqUserID, string strShortNumber, ref List<AddressInfo> p_addrInfos)
        {
            string strProcedureName = "PKG_PRC_AOB.USP_SELECT_ADDR_GROUP_LST_DEPT";
            bool ret = false;

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

                        dbCommand.Parameters.AddWithValue("P_REQ_USER_ID", strReqUserID);
                        dbCommand.Parameters.AddWithValue("P_SHORT_NUMBER", strShortNumber);

                        OracleParameter paramCursor = new OracleParameter("P_CURSOR", OracleType.Cursor);
                        paramCursor.Direction = ParameterDirection.Output;
                        dbCommand.Parameters.Add(paramCursor);

                        using (OracleDataReader reader = dbCommand.ExecuteReader())
                        {
                            while (reader.Read())
                            {
                                //// 동보발송 주소록 처리 ////
                                AddressInfo addrInfo = new AddressInfo();
                                addrInfo.strRecipientName = reader.GetString(0).Trim();
                                addrInfo.strFaxNo = reader.GetString(1).Trim();
                                p_addrInfos.Add(addrInfo);
                            }

                            reader.Close();
                        }
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
        public virtual bool SetBroadcastCntUser(decimal nFaxID, string strReqUserID, string strShortNumber, ref List<AddressInfo> p_addrInfos)
        {
            string strProcedureName = "PKG_PRC_AOB.USP_UPDATE_BROADCAST_CNT_USER";
            bool ret = false;
            decimal broadcastCnt = -1;

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

                        dbCommand.Parameters.AddWithValue("P_FAX_ID", nFaxID);
                        dbCommand.Parameters.AddWithValue("P_REQ_USER_ID", strReqUserID);
                        dbCommand.Parameters.AddWithValue("P_SHORT_NUMBER", strShortNumber);

                        OracleParameter param = new OracleParameter("P_BROADCAST_CNT", OracleType.Number);
                        param.Direction = ParameterDirection.Output;
                        dbCommand.Parameters.Add(param);

                        int cnt = dbCommand.ExecuteNonQuery();
                        if (cnt < 1)
                            return false;

                        broadcastCnt = Convert.ToDecimal(dbCommand.Parameters["P_BROADCAST_CNT"].Value);

                        /*
                        using (OracleDataReader reader = dbCommand.ExecuteReader())
                        {
                            while (reader.Read())
                            {
                                //// 동보발송 주소록 처리 ////
                                AddressInfo addrInfo = new AddressInfo();
                                addrInfo.strRecipientName = reader.GetString(0).Trim();
                                addrInfo.strFaxNo = reader.GetString(1).Trim();
                                p_addrInfos.Add(addrInfo);
                            }

                            reader.Close();
                        }
                         */
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

        public virtual bool SetBroadcastCntDept(decimal nFaxID, string strReqUserID, string strShortNumber, ref List<AddressInfo> p_addrInfos)
        {
            string strProcedureName = "PKG_PRC_AOB.USP_UPDATE_BROADCAST_CNT_DEPT";
            bool ret = false;
            decimal broadcastCnt = -1;

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

                        dbCommand.Parameters.AddWithValue("P_FAX_ID", nFaxID);
                        dbCommand.Parameters.AddWithValue("P_REQ_USER_ID", strReqUserID);
                        dbCommand.Parameters.AddWithValue("P_SHORT_NUMBER", strShortNumber);

                        OracleParameter param = new OracleParameter("P_BROADCAST_CNT", OracleType.Number);
                        param.Direction = ParameterDirection.Output;
                        dbCommand.Parameters.Add(param);

                        int cnt = dbCommand.ExecuteNonQuery();
                        if (cnt < 1)
                            return false;

                        broadcastCnt = Convert.ToDecimal(dbCommand.Parameters["P_BROADCAST_CNT"].Value);

                        /*
                        using (OracleDataReader reader = dbCommand.ExecuteReader())
                        {
                            while (reader.Read())
                            {
                                //// 동보발송 주소록 처리 ////
                                AddressInfo addrInfo = new AddressInfo();
                                addrInfo.strRecipientName = reader.GetString(0).Trim();
                                addrInfo.strFaxNo = reader.GetString(1).Trim();
                                p_addrInfos.Add(addrInfo);
                            }

                            reader.Close();
                        }
                         */
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

        public virtual bool SetBroadcastCnt(decimal nFaxID, ref List<AddressInfo> p_addrInfos)
        {
            string strProcedureName = "PKG_PRC_COMMON.USP_UPDATE_BROADCAST_CNT";
            bool ret = false;
            decimal broadcastCnt = -1;

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

                        dbCommand.Parameters.AddWithValue("P_FAX_ID", nFaxID);

                        OracleParameter param = new OracleParameter("P_BROADCAST_CNT", OracleType.Number);
                        param.Direction = ParameterDirection.Output;
                        dbCommand.Parameters.Add(param);

                        int cnt = dbCommand.ExecuteNonQuery();
                        if (cnt < 1)
                            return false;

                        broadcastCnt = Convert.ToDecimal(dbCommand.Parameters["P_BROADCAST_CNT"].Value);

                        /*
                        using (OracleDataReader reader = dbCommand.ExecuteReader())
                        {
                            while (reader.Read())
                            {
                                //// 동보발송 주소록 처리 ////
                                AddressInfo addrInfo = new AddressInfo();
                                addrInfo.strRecipientName = reader.GetString(0).Trim();
                                addrInfo.strFaxNo = reader.GetString(1).Trim();
                                p_addrInfos.Add(addrInfo);
                            }

                            reader.Close();
                        }
                         */
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

        public virtual bool GetSstMapInfo(ref Dictionary<string, List<SST_COLUMN_MAP>> p_sstMapH, ref Dictionary<string, List<SST_COLUMN_MAP>> p_sstMap)
        {
            string strProcedureName = "PKG_PRC_AOB.USP_SELECT_SST_MAP";
            bool ret = false;
            string strTrNo;
            SST_COLUMN_MAP  sstColumnMap = new SST_COLUMN_MAP();
            List<SST_COLUMN_MAP> sstColumnMapList;
            Dictionary<string, List<SST_COLUMN_MAP>> sstMap;

            p_sstMap.Clear();

            lock (m_connection)
            {
                if (!ReOpen())
                    return false;

                try
                {
                    using (OracleCommand dbCommand = new OracleCommand())
                    {
                        dbCommand.Connection  = m_connection;
                        dbCommand.CommandText = strProcedureName;
                        dbCommand.CommandType = CommandType.StoredProcedure;
                        
                        OracleParameter paramCursor = new OracleParameter("P_CURSOR", OracleType.Cursor);
                        paramCursor.Direction       = ParameterDirection.Output;
                        dbCommand.Parameters.Add(paramCursor);

                        using (OracleDataReader reader = dbCommand.ExecuteReader())
                        {   
                            while (reader.Read())
                            {
                                strTrNo                             = reader["TR_NO"].ToString();
                                sstColumnMap.strTrField             = reader["TR_FIELD"].ToString();
                                sstColumnMap.strTrFieldType         = reader["TR_FIELD_TYPE"].ToString();
                                sstColumnMap.strTrFieldExtraInfo    = reader["TR_FIELD_EXTRA_INFO"].ToString();
                                sstColumnMap.strColumnName          = reader["COLUMN_NAME"].ToString();
                                sstColumnMap.strColumnType          = reader["COLUMN_TYPE"].ToString();

                                if (sstColumnMap.strTrFieldType == "H") // 헤더 필드(H)
                                    sstMap = p_sstMapH;
                                else // 전문 필드(B), 반복부 필드(R)
                                    sstMap = p_sstMap;
                                
                                if (sstMap.ContainsKey(strTrNo))
                                {
                                    sstMap[strTrNo].Add(sstColumnMap);
                                }
                                else
                                {
                                    sstColumnMapList = new List<SST_COLUMN_MAP>();
                                    sstColumnMapList.Add(sstColumnMap);
                                    sstMap.Add(strTrNo, sstColumnMapList);
                                }
                            }

                            reader.Close();
                        }
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
            //// 여기에 사이트 정보를 INSERT 하는 로직을 추가해주세요. ////
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
}

