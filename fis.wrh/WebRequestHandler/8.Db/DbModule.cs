using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.OleDb;
using System.Data.OracleClient;
using Btfax.CommonLib.MsgBox;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Db;

namespace WebRequestHandler
{
    public class DbModule : DbModuleBase
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
        public struct RECV_INFO
        {
            public decimal  faxID;
            public string   strDID;
            public string   strCID;
            public string   strTifFile;
            public decimal  fileSize;
            public string   strSplitYN;
            public DateTime dtRecv;
        }

        public struct SPLIT_FILE_INFO
        {
            public string p_strFile;
            public decimal p_fileSize;
            public int p_nPageCnt;
            public string p_strPagesInOrg;

            public void Clear()
            {
                p_strFile   = "";
                p_fileSize  = 0;
                p_nPageCnt  = 0;
                p_strPagesInOrg = "";
            }
        }
        #endregion

        #region method
        public virtual bool GetWaitList(DataSet p_dsDataSet)
        {
            string strProcedureName = "PKG_PRC_WRH.USP_SELECT_WAIT_LST";
            OracleParameter param;

            
            try
            {
                p_dsDataSet.Clear();

                if (m_daWaitListAdapter != null)
                {
                    lock (m_daWaitListAdapter)
                    {
                        m_daWaitListAdapter.Fill(p_dsDataSet);
                    }
                    return true;
                }

                OracleCommand command = new OracleCommand();
                command.Connection = m_connection;
                command.CommandText = strProcedureName;
                command.CommandType = CommandType.StoredProcedure;

                command.Parameters.AddWithValue("P_FETCH_PROCESS", Config.PROCESS_TYPE.ToString());
                param = command.Parameters.Add("P_TABLE1", OracleType.Cursor);
                param.Direction = ParameterDirection.Output;

                lock (m_daWaitListAdapter = new OracleDataAdapter(command))
                {
                    m_daWaitListAdapter.Fill(p_dsDataSet);
                }
            }
            catch (Exception ex)
            {
                ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                return false;
            }

            return true;
        }

        public bool GetRecvFaxInfo(decimal p_faxID, out RECV_INFO p_recvInfo )
        {
            string strProcedureName = "PKG_PRC_WRH.USP_SELECT_RECV_MSTR_DATA";
            OracleParameter param;
            bool bResult = true;

            p_recvInfo = new RECV_INFO();

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

                        command.Parameters.AddWithValue("P_FAX_ID", p_faxID);
                        param = command.Parameters.Add("P_TABLE1", OracleType.Cursor);
                        param.Direction = ParameterDirection.Output;

                        OracleDataReader reader = command.ExecuteReader();
                        DataTable table = new DataTable();
                        table.Load(reader);

                        if (table.Rows.Count <= 0)
                            return false;

                        p_recvInfo.faxID        = table.Rows[0].Field<decimal>("FAX_ID");
                        p_recvInfo.strDID       = table.Rows[0].Field<string>("DID");
                        p_recvInfo.strCID       = table.Rows[0].Field<string>("CID");
                        p_recvInfo.strTifFile   = table.Rows[0].Field<string>("TIF_FILE");
                        p_recvInfo.fileSize     = table.Rows[0].Field<decimal>("TIF_FILE_SIZE");
                        p_recvInfo.strSplitYN   = table.Rows[0].Field<string>("SPLIT_YN");
                        p_recvInfo.dtRecv       = table.Rows[0].Field<DateTime>("RECV_DATE");
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

        public bool InsertSplitInfo(decimal p_faxID, SPLIT_FILE_INFO p_spitFileInfo)
        {
            string strProcedureName = "PKG_PRC_WRH.USP_INSERT_RECV_SPLIT";
            
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

                        command.Parameters.AddWithValue("P_FAX_ID",                 p_faxID);
                        command.Parameters.AddWithValue("P_SPLIT_TIF_FILE",         p_spitFileInfo.p_strFile);
                        command.Parameters.AddWithValue("P_SPLIT_TIF_FILE_SIZE",    p_spitFileInfo.p_fileSize);
                        command.Parameters.AddWithValue("P_SPLIT_TIF_PAGES_CNT",    p_spitFileInfo.p_nPageCnt);
                        command.Parameters.AddWithValue("P_SPLIT_TIF_PAGES_IN_ORG", p_spitFileInfo.p_strPagesInOrg);
                        
                        int nCnt = command.ExecuteNonQuery();
                        if (nCnt <= 0)
                            return false;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                }

                Close();
            }

            return true;
        }

        public bool DeleteSplitInfos(decimal p_faxID)
        {
            string strProcedureName = "PKG_PRC_WRH.USP_DELETE_RECV_SPLIT";

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

                        command.Parameters.AddWithValue("P_FAX_ID", p_faxID);

                        int nCnt = command.ExecuteNonQuery();
                        if (nCnt <= 0)
                            return false;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                }

                Close();
            }

            return true;
        }

        public bool UpdateSplitYN(decimal p_faxID, char p_chYN)
        {
            string strProcedureName = "PKG_PRC_WRH.USP_UPDATE_RECV_MSTR_SPLIT_YN";

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

                        command.Parameters.AddWithValue("P_FAX_ID", p_faxID);
                        command.Parameters.AddWithValue("P_SPLIT_YN", p_chYN.ToString());

                        int nCnt = command.ExecuteNonQuery();
                        if (nCnt <= 0)
                            return false;
                    }
                }
                catch (Exception ex)
                {
                    ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
                }

                Close();
            }

            return true;
        }
        #endregion

        #region field
        OracleDataAdapter m_daWaitListAdapter = null;
        #endregion
    }
}
