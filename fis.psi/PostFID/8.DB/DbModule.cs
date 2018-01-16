using System;
using System.Collections.Generic;
using System.Data;
using System.Data.OracleClient;
using Btfax.CommonLib;
using Btfax.CommonLib.Db;
using System.Linq;

namespace PostFid.DB
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
		public class RECV_FAX_DATA_PSI
		{
			#region methods
			public void Clear()
			{
				FAX_ID = -1;
				SUCC_YN = "N";
				USER_ID = "";
				DEPT_CD = "";
				DEPT_NAME = "";
				TASK_ID = "";
				TASK_NAME = "";
				USER_NAME = "";
				SERVICE_FAXNO = "";  //WCD(2014-05-28)
				DID = "";  //WCD(2014-05-28)
				CID = "";
				CID_NAME = "";
				RECV_DATE = "";  //WCD(2014-05-28)
				RECV_TIME = "";  //WCD(2014-05-28)
				PAGE_CNT = "";  //WCD(2014-05-28)
				EMAIL_NOTIFY_YN = "";
				EMAIL_NOTIFY = "";
				PROXY_USER_ID = "";
				PROXY_USER_NAME = "";
				PROXY_EMAIL_NOTIFY_YN = "";
				PROXY_EMAIL_NOTIFY = "";
				SPAM_YN = "";
			}
			#endregion

			#region fields
			public decimal FAX_ID;
			public string SUCC_YN;
			public string DEPT_CD;
			public string DEPT_NAME;
			public string TASK_ID;
			public string TASK_NAME;
			public string USER_ID;
			public string USER_NAME;
			public string SERVICE_FAXNO;      //WCD(2014-04-29)
			public string DID;                //WCD(2014-04-29)
			public string CID;
			public string CID_NAME;
			public string RECV_DATE;          //WCD(2014-04-29)
			public string RECV_TIME;          //WCD(2014-04-29)
			public string PAGE_CNT;           //WCD(2014-04-29)
			public string EMAIL_NOTIFY_YN;
			public string EMAIL_NOTIFY;
			public string PROXY_USER_ID;
			public string PROXY_USER_NAME;
			public string PROXY_EMAIL_NOTIFY_YN;
			public string PROXY_EMAIL_NOTIFY;
			public string SPAM_YN;
			#endregion
		}
		#endregion

		#region method
		public RESULT PSI_FetchPostProcessingRequest(P_TYPE p_processType, string p_strSystemProcessId, int p_occupyCnt, ref List<RECV_FAX_DATA_PSI> p_result)
		{
			RESULT ret = RESULT.EMPTY;
			string strProcedureName = "PKG_PRC_PSI.USP_PROCESS_POST_INBOUND";

			lock (m_connection)
			{

				if (!ReOpen()) return RESULT.F_DB_OPENERROR;

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

						OracleParameter param1 = new OracleParameter("P_TABLE1", OracleType.Cursor);
						param1.Direction = ParameterDirection.Output;
						command.Parameters.Add(param1);

						OracleParameter param2 = new OracleParameter("P_TABLE2", OracleType.Cursor);
						param2.Direction = ParameterDirection.Output;
						command.Parameters.Add(param2);

						using (OracleDataReader reader = command.ExecuteReader())
						{
							DataTable dt = new DataTable();
							dt.Load(reader);
							DataTable dt2 = new DataTable();
							dt2.Load(reader);

							/// dt, dt2 outer join 과 같은 결과를 내기 위한 loop
							foreach (DataRow dr in dt.Rows)
							{
								RECV_FAX_DATA_PSI data = new RECV_FAX_DATA_PSI();
								data.FAX_ID = decimal.Parse(dr["FAX_ID"].ToString());
								//data.DEPT_CD				= dr["OWNER_DEPT_CD"].ToString();    //WCD(2014-05-28)  부서정보 조회
								//data.DEPT_NAME			= dr["OWNER_DEPT_NAME"].ToString();  //WCD(2014-05-28)  부서정보 조회
								
								// TEST - 추후 주석제거
								//data.DEPT_CD = dr["UPPER_DEPT_CD"].ToString();    //WCD(2014-05-28)
								//data.DEPT_NAME = dr["UPPER_DEPT_NAME"].ToString();  //WCD(2014-05-28)
								//data.TASK_ID = dr["OWNER_TASK_ID"].ToString();
								// TEST


								//if(!decimal.TryParse(dr["OWNER_TASK_ID"].ToString(), out data.TASK_ID))
								//    data.TASK_ID = 0;

								data.TASK_NAME = dr["OWNER_TASK_NAME"].ToString();
								data.USER_ID = dr["OWNER_USER_ID"].ToString();

								// TEST - 추후 주석제거
								data.USER_NAME = dr["OWNER_USER_NAME"].ToString(); //WCD(2014-04-29)
								data.SERVICE_FAXNO = dr["SERVICE_FAXNO"].ToString();   //WCD(2014-04-29)
								data.DID = dr["DID"].ToString();             //WCD(2014-04-29)
								//data.RECV_DATE = string.Format("{0:yyyyMMdd}", dr["RECV_DATE"]);//WCD(2014-04-29)
								//data.RECV_TIME = string.Format("{0:HHmm}", dr["RECV_DATE"]);  //WCD(2014-04-29)
								//data.PAGE_CNT = dr["TIF_PAGE_CNT"].ToString();    //WCD(2014-04-29)
								// TEST - 추후 주석제거


								
								data.CID = dr["CID"].ToString();
								data.CID_NAME = dr["CID_NAME"].ToString();
								data.EMAIL_NOTIFY_YN = dr["EMAIL_NOTIFY_YN"].ToString();
								data.EMAIL_NOTIFY = dr["EMAIL_NOTIFY"].ToString();
								data.PROXY_USER_ID = dr["PROXY_USER_ID"].ToString();
								data.PROXY_USER_ID = dr["PROXY_USER_NAME"].ToString();
								data.PROXY_EMAIL_NOTIFY_YN = dr["PROXY_EMAIL_NOTIFY_YN"].ToString();
								data.PROXY_EMAIL_NOTIFY = dr["PROXY_EMAIL_NOTIFY"].ToString();
								data.SPAM_YN = dr["SPAM_YN"].ToString();

								foreach (DataRow dr2 in dt2.Rows)
								{
									decimal fax_id = decimal.Parse(dr2["FAX_ID"].ToString());
									if (data.FAX_ID == fax_id)
									{
										data.SUCC_YN = dr2["SUCC_YN"].ToString();
										break;
									}
								}
								p_result.Add(data);
							}
						}

						if (p_result.Count > 0) ret = RESULT.SUCCESS;
					}
				}
				catch (Exception ex)
				{
					ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
					ret = RESULT.F_SYSTEM_ERROR;
				}
			}
			return ret;
		}


		public bool GetReceiveNotiUsers(string p_deptCd, string p_taskId, ref List<string> p_userLst)
		{
			string strProcedureName = "PKG_PRC_PSI.USP_GET_RECV_NOTI_USER_LIST";

			bool ret = false;

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

						command.Parameters.AddWithValue("P_DEPT_CD", p_deptCd);
						if (string.IsNullOrEmpty(p_taskId))
							command.Parameters.AddWithValue("P_TASK_ID", DBNull.Value);
						else
							command.Parameters.AddWithValue("P_TASK_ID", p_taskId);

						OracleParameter param1 = new OracleParameter("P_TABLE1", OracleType.Cursor);
						param1.Direction = ParameterDirection.Output;
						command.Parameters.Add(param1);

						using (OracleDataReader reader = command.ExecuteReader())
						{
							DataTable dt = new DataTable();
							dt.Load(reader);

							foreach (DataRow dr in dt.Rows)
								p_userLst.Add(dr["USER_ID"].ToString());
						}
					}

					ret = true;
				}
				catch (Exception ex)
				{
					ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
					ret = false;
				}
			}

			return ret;
		}


		public RESULT PSI_InsertProcessLog(P_TYPE p_processType, string p_strSystemProcessId, decimal p_FAX_ID, RESULT p_REASON, string p_INFO_EXTRA)
		{
			RESULT ret = RESULT.EMPTY;
			string strProcedureName = "PKG_PRC_PSI.USP_INSERT_PROCESS_LOG";

			lock (m_connection)
			{
				if (!ReOpen()) return RESULT.F_DB_OPENERROR;

				try
				{
					using (OracleCommand command = new OracleCommand())
					{
						command.Connection = m_connection;
						command.CommandText = strProcedureName;
						command.CommandType = CommandType.StoredProcedure;

						command.Parameters.AddWithValue("P_FAX_ID", p_FAX_ID);
						command.Parameters.AddWithValue("P_PROCESS_TYPE", p_processType.ToString());
						command.Parameters.AddWithValue("P_FETCH_PROCESS", p_strSystemProcessId);
						command.Parameters.AddWithValue("P_REASON", ((int)p_REASON).ToString().PadLeft(3, '0'));
						command.Parameters.AddWithValue("P_INFO_EXTRA", p_INFO_EXTRA);

						command.ExecuteNonQuery();

						ret = RESULT.SUCCESS;
					}
				}
				catch (Exception ex)
				{
					ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
					ret = RESULT.F_SYSTEM_ERROR;
				}
			}

			return ret;
		}


		#endregion
	}
}
