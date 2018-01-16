using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.IO;
using System.Data.OracleClient;
using Btfax.CommonLib.Db;
using Btfax.CommonLib;

namespace FaxAgent.Server
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
        private DbModule()
        {
        }
        #endregion

        #region Business
        /// <summary>
        /// 최신버전 정보 확인 (FAU 사용)
        /// </summary>
        /// <param name="p_BUILD_VER">client 버전</param>
        /// <param name="dtResult">최신 정보 버전</param>
        /// <returns>RESULT</returns>
        public RESULT FAS_CheckDeployInfo(string p_BUILD_VER, ref DataTable dtResult)
        {	
            string strProcedureName = "PKG_PRC_FAS.USP_SELECT_DEPLOY_INFO";
			lock (this)
			{
				try
				{
					if (!ReOpen())
						return RESULT.F_DB_OPENERROR;

					using (OracleCommand command = this.MakeOracleCommand(strProcedureName))
					{
						OraParam param = new OraParam();
						param.AddInParam("P_BUILD_VER ", OracleType.VarChar, p_BUILD_VER);
						param.AddOutParam("P_TABLE1", OracleType.Cursor);
						command.Parameters.AddRange(param.ParamsArray);

						OracleDataReader reader = command.ExecuteReader();
						dtResult = new DataTable();
						dtResult.Load(reader);
					}
				}
				catch (Exception ex)
				{
					ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
					return RESULT.F_DB_ERROR;
				}
			}
            return RESULT.SUCCESS;
        }

        /// <summary>
        /// 로그인 : ID 확인 -> 암호 확인 -> 로그인여부 기록 -> 성공
        /// </summary>
        /// <returns>
        /// EMPTY = 000
        /// SUCCESS = 001
        /// F_DB_NOTEXIST_USER_ID = 115
        /// F_DB_PASSWORD_MISMATCH = 116
        /// F_DB_ERROR = 101
        /// </returns>
        public RESULT FAS_LoginAgentClient(string p_USER_ID, string p_USER_PW, string p_IP, ref string p_USER_NAME, ref string p_RECV_FAXBOX_ID, ref string p_SEND_FAXBOX_ID)
        {
            string strProcedureName = "PKG_PRC_FAS.USP_LOGIN_AGENT_CLIENT";
			lock (this)
			{
				try
				{
					if (!ReOpen())
						return RESULT.F_DB_OPENERROR;

					using (OracleCommand command = this.MakeOracleCommand(strProcedureName))
					{
						OraParam param = new OraParam();
						param.AddInParam("P_USER_ID ", OracleType.VarChar, p_USER_ID);
						param.AddInParam("P_USER_PW", OracleType.VarChar, p_USER_PW);
						param.AddInParam("P_LOGIN_IP", OracleType.VarChar, p_IP);
						param.AddOutParam("P_OUT_RESULT", OracleType.Number);
						param.AddOutParam("P_OUT_USER_NAME", OracleType.VarChar, 30);
						param.AddOutParam("P_OUT_RECV_FAXBOX_ID", OracleType.Number, 12);
						param.AddOutParam("P_OUT_SEND_FAXBOX_ID", OracleType.Number, 12);
						command.Parameters.AddRange(param.ParamsArray);

						command.ExecuteNonQuery();

						RESULT result = (RESULT)(decimal)command.Parameters["P_OUT_RESULT"].Value;
						p_USER_NAME = command.Parameters["P_OUT_USER_NAME"].Value.ToString();
						p_RECV_FAXBOX_ID = command.Parameters["P_OUT_RECV_FAXBOX_ID"].Value.ToString();
						p_SEND_FAXBOX_ID = command.Parameters["P_OUT_SEND_FAXBOX_ID"].Value.ToString();

						if (result != RESULT.SUCCESS) return result;
					}
				}
				catch (Exception ex)
				{
					ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
					return RESULT.F_DB_ERROR;
				}
			}
            return RESULT.SUCCESS;
        }

        /// <summary>
        /// 로그아웃
        /// </summary>
        /// <param name="p_USER_ID">사용자 ID</param>
        /// <param name="byForce">강제 로그아웃 (현재 기능 없음)</param>
        /// <returns>
        /// EMPTY = 000
        /// SUCCESS = 001
        /// F_DB_ERROR = 101
        /// </returns>
        public RESULT FAS_LogoutAgentClient(string p_USER_ID)
        {
            string strProcedureName = "PKG_PRC_FAS.USP_LOGOUT_AGENT_CLIENT";
			lock (this)
			{
				try
				{
					if (!ReOpen())
						return RESULT.F_DB_OPENERROR;

					using (OracleCommand command = this.MakeOracleCommand(strProcedureName))
					{
						OraParam param = new OraParam();
						param.AddInParam("P_USER_ID ", OracleType.VarChar, p_USER_ID);
						param.AddOutParam("P_OUT_RESULT", OracleType.Number);
						command.Parameters.AddRange(param.ParamsArray);

						command.ExecuteNonQuery();

						RESULT result = (RESULT)(decimal)command.Parameters["P_OUT_RESULT"].Value;
						if (result != RESULT.SUCCESS) return result;
					}
				}
				catch (Exception ex)
				{
					ExceptionMsg(ex, "PROCEDURE : " + strProcedureName);
					return RESULT.F_DB_ERROR;
				}
			}
            return RESULT.SUCCESS;
        }
        #endregion

        #region method
        /// <summary>
        /// 오라클 연결 정보 생성
        /// </summary>
        /// <param name="procedureName">실행할 프로시져 이름</param>
        /// <returns>OracleCommand</returns>
        private OracleCommand MakeOracleCommand(string procedureName)
        {
            OracleCommand command = new OracleCommand();
            command.Connection = m_connection;
            command.CommandText = procedureName;
            command.CommandType = CommandType.StoredProcedure;
            return command;
        }
        #endregion
    }
}

