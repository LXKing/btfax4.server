using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using Btfax.CommonLib.Log;

namespace FaxAgent.Server
{
    public class SocketListenerLog
    {
        #region constructor
        internal SocketListenerLog(LOG_LEVEL logLevel = LOG_LEVEL.TRC)
        {
			//fileLog = new FileLog();
			//fileLog.LogLevel = logLevel;
			//fileLog.LogPrefix = string.Format("FAX_{0}_FAS_{1}_SocketListener", Config.SYSTEM_NO, Config.PROCESS_NO);
        }
        #endregion

        #region method
		internal void ERR(params string[] messages)
		{
			AppLog.Write(LOG_LEVEL.ERR, string.Join("\t", messages));
		}

		internal void ERR(SocketAsyncEventArgs e, params string[] messages)
		{
			string strMsg = "";
			string strRemoteIp = "";
			int nRemotePort = 0;
			int nToken = 0;

			DataHoldingUserToken dToken = e.UserToken as DataHoldingUserToken;
			if (dToken != null)
			{
				strRemoteIp = dToken.RemoteIp;
				nRemotePort = dToken.RemotePort;
				nToken = dToken.TokenId;
			}
			else
			{
				AcceptOpUserToken aToken = e.UserToken as AcceptOpUserToken;
				strRemoteIp = aToken.RemoteIp;
				nRemotePort = aToken.RemotePort;
				nToken = aToken.TokenId;
			}

			strMsg = string.Format("{0,-15}:{1}/{2}|{3}"
										, strRemoteIp
										, nRemotePort
										, nToken
										, string.Join("\t", messages)
										);

			AppLog.Write(LOG_LEVEL.ERR, strMsg);
		}

		internal void MSG(SocketAsyncEventArgs e, params string[] messages)
		{
			string strMsg = "";
			string strRemoteIp = "";
			int nRemotePort = 0;
			int nToken = 0;

			DataHoldingUserToken dToken = e.UserToken as DataHoldingUserToken;
			if (dToken != null)
			{
				strRemoteIp = dToken.RemoteIp;
				nRemotePort = dToken.RemotePort;
				nToken = dToken.TokenId;
			}
			else
			{
				AcceptOpUserToken aToken = e.UserToken as AcceptOpUserToken;
				strRemoteIp = aToken.RemoteIp;
				nRemotePort = aToken.RemotePort;
				nToken = aToken.TokenId;
			}

			strMsg = string.Format("{0,-15}:{1}/{2}|{3}"
										, strRemoteIp
										, nRemotePort
										, nToken
										, string.Join("\t", messages)
										);

			AppLog.Write(LOG_LEVEL.MSG, strMsg);
		}

		internal void MSG( params string[] messages)
		{
			AppLog.Write(LOG_LEVEL.MSG, string.Join("\t", messages));
		}

		internal void WRN(SocketAsyncEventArgs e, params string[] messages)
		{
			string strMsg = "";
			string strRemoteIp = "";
			int nRemotePort = 0;
			int nToken = 0;

			DataHoldingUserToken dToken = e.UserToken as DataHoldingUserToken;
			if (dToken != null)
			{
				strRemoteIp = dToken.RemoteIp;
				nRemotePort = dToken.RemotePort;
				nToken = dToken.TokenId;
			}
			else
			{
				AcceptOpUserToken aToken = e.UserToken as AcceptOpUserToken;
				strRemoteIp = aToken.RemoteIp;
				nRemotePort = aToken.RemotePort;
				nToken = aToken.TokenId;
			}
			strMsg = string.Format("{0,-15}:{1}/{2}|{3}"
											, strRemoteIp
											, nRemotePort
											, nToken
											, string.Join("\t", messages)
											);

			AppLog.Write(LOG_LEVEL.WRN, strMsg);
		}

		internal void WRN(params string[] messages)
		{
			AppLog.Write(LOG_LEVEL.WRN, string.Join("\t", messages));
		}

		internal void TRC(SocketAsyncEventArgs e, params string[] messages)
		{
			string strMsg = "";
			string strRemoteIp = "";
			int nRemotePort = 0;
			int nToken = 0;

			DataHoldingUserToken dToken = e.UserToken as DataHoldingUserToken;
			if (dToken != null)
			{
				strRemoteIp = dToken.RemoteIp;
				nRemotePort = dToken.RemotePort;
				nToken = dToken.TokenId;
			}
			else
			{
				AcceptOpUserToken aToken = e.UserToken as AcceptOpUserToken;
				strRemoteIp = aToken.RemoteIp;
				nRemotePort = aToken.RemotePort;
				nToken = aToken.TokenId;
			}

			strMsg = string.Format("{0,-15}:{1}/{2}|{3}"
										, strRemoteIp
										, nRemotePort
										, nToken
										, string.Join("\t", messages)
										);

			AppLog.Write(LOG_LEVEL.TRC, strMsg);
		}

		internal void TRC(params string[] messages)
		{
			AppLog.Write(LOG_LEVEL.TRC, string.Join("\t", messages));
		}

        internal void Write(params string[] messages)
        {
			AppLog.Write(LOG_LEVEL.MSG, string.Join("\t", messages));
        }

        /// <summary>
        /// 패킷로그
        /// </summary>
		internal void LogWrite(LOG_LEVEL p_logLevel, string direction, string recvSendIpPort, int socketHandle, Int32 tokenId, int byteCount = 0, string message = "")
        {
			AppLog.Write(p_logLevel
							, string.Join("|",
                            "SocketListener",
                            direction,
                            string.Format("{0,-21}", recvSendIpPort),
                            string.Format("T:{0,5}", System.Threading.Thread.CurrentThread.ManagedThreadId),
                            string.Format("H:{0,5}", socketHandle),
                            string.Format("I:{0,7}", tokenId),
                            string.Format("{0,5} byte", byteCount),
                            message
                            ));
        }

        internal void ExceptionLog(Exception p_ex, String p_strMessage)
        {
			AppLog.Write(LOG_LEVEL.ERR, string.Format("{0}\t{1}\t{2}", p_strMessage, p_ex.Message, p_ex.StackTrace));
                          
        }
        #endregion

        #region field
        #endregion
    }
}


