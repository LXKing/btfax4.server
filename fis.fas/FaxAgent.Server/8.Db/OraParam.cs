using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.OracleClient;

namespace FaxAgent.Server
{
    class OraParam
    {
        /// <summary>
        /// IN, OUT 파라미터 리스트
        /// </summary>
        private List<OracleParameter> m_oraParams = new List<OracleParameter>();
        public OracleParameter[] ParamsArray { get { return m_oraParams.ToArray(); } }
        
        /// <summary>
        /// 오라클 IN 파라미터
        /// </summary>
        /// <param name="parameterName">파라미터 이름</param>
        /// <param name="oraType">파라미터 타입</param>
        /// <param name="value">파라미터 값</param>
        /// <param name="size">파라미터 크기</param>
        /// <returns>OracleParameter</returns>
        public OracleParameter AddInParam(string parameterName, OracleType oraType, object value, int size = 0)
        {
            OracleParameter oraParam = new OracleParameter(parameterName.Trim(), oraType);
            oraParam.Value = value;
            oraParam.Size = size;
            oraParam.Direction = ParameterDirection.Input;
            m_oraParams.Add(oraParam);
            return oraParam;
        }

        /// <summary>
        /// 오라클 OUT 파라미터
        /// </summary>
        /// <param name="parameterName">파라미터 이름</param>
        /// <param name="oraType">파라미터 타입</param>
        /// <param name="size">파라미터 크기</param>
        /// <returns>OracleParameter</returns>
        public OracleParameter AddOutParam(string parameterName, OracleType oraType, int size = 0)
        {
            OracleParameter oraParam = new OracleParameter(parameterName.Trim(), oraType);
            oraParam.Direction =  ParameterDirection.Output;
            oraParam.Size = size;
            m_oraParams.Add(oraParam);
            return oraParam;
        }
    }
}
