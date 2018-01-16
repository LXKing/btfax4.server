using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Data;
using System.Net.Sockets;
using System.Diagnostics;
using FaxAgent.Common;
using FaxAgent.Common.Socket;
using Btfax.CommonLib;
using Btfax.CommonLib.Log;

namespace FaxAgent.Server
{
    class MessageParsing
    {
        #region public event
        public delegate void StringEventHandler(FACInfo facInfo);
        public event StringEventHandler OnLoginDuplicated;
        #endregion

        /// <summary>
        /// Request 된 전문에 해당하는 Response 전문 생성
        /// or
        /// 지정된 전문 생성
        /// 
        /// *** 보낼 SAEA 객체에는 Request 된 전문이 들어있음
        /// </summary>
        /// <param name="sendEventArgs">보낼 SAEA 객체</param>
        /// <param name="packetName">옵션 : 특정전문 지정</param>
        /// <param name="userId">옵션 : 사용자 ID</param>
        /// <param name="message">옵션 : 메세지</param>
        /// <returns>MessageStream</returns>
        public MessageStream GetResponse(SocketAsyncEventArgs sendEventArgs, MessagePacketNameEnum packetName = MessagePacketNameEnum.EMPTY, string userId = "")
        {
            MessageStream response = null;

			try
			{
				/// Request 된 전문 (sendEventArgs) 에 해당하는 Response 전문 생성
				if (packetName == MessagePacketNameEnum.EMPTY)
				{
					MessageReader reader = this.GetResponseHeader(sendEventArgs);
					response = this.GetResponseMake(sendEventArgs, reader);
				}
				else
				{
					//if (packetName == MessagePacketNameEnum.NTFTM) response = MessageNotifyTextMessage(sendEventArgs);
					if (packetName == MessagePacketNameEnum.ACCPT) response = MessageAccept();
					else if (packetName == MessagePacketNameEnum.CLOSE) response = MessageLoginDuplicated(userId);
				}
				return response;
			}
			catch
			{
				return null;
			}
        }

        /// <summary>
        /// 전문 헤더 생성
        /// </summary>
        /// <param name="sendEventArgs">보낼 SAEA 객체</param>
        /// <returns>MessageReader</returns>
        public MessageReader GetResponseHeader(SocketAsyncEventArgs sendEventArgs)
        {
            MessageReader reader = null;
            DataHoldingUserToken receiveSendToken = (sendEventArgs.UserToken as DataHoldingUserToken);

            /// STX, ETX 제거
            int bufferLength = receiveSendToken.dataToReceive.Length - 2;
            byte[] buffer = new byte[bufferLength];
            Buffer.BlockCopy(receiveSendToken.dataToReceive, 1, buffer, 0, bufferLength);

            /// 전문 헤더 검사
            try
            {
                reader = new MessageReader(buffer);
            }
            catch (Exception ex)
            {
                SocketListener.Log.ExceptionLog(ex, "전문 헤더 오류");
            }
            return reader;
        }

        /// <summary>
        /// 전문 바디 생성
        /// </summary>
        /// <param name="sendEventArgs">보낼 SAEA 객체</param>
        /// <param name="reader">전문 헤더</param>
        /// <param name="message">바디에 추가할 메세지</param>
        /// <returns>MessageStream</returns>
        public MessageStream GetResponseMake(SocketAsyncEventArgs sendEventArgs, MessageReader reader, string message = "")
        {
            MessageStream response = null;

            try
            {
                /// 전문 몸체 생성
                if (reader == null)
                {
                    /// 헤더 없을시 ERRHD 전문 생성
                    response = MessageErrorHeader();
                }
                else
                {
                    MessagePacketNameEnum packetName = MessagePacketNameEnum.EMPTY;

                    /// 전문 헤더의 PacketName 에 해당하는 응답전문 이름 파싱
                    if (Enum.TryParse<MessagePacketNameEnum>(reader.Header.PacketName, out packetName) == false)
                        packetName = MessagePacketNameEnum.EMPTY;

                    /// 파싱된 응답 전문 바디 생성
                    if (packetName == MessagePacketNameEnum.EMPTY) response = MessageErrorHeader();
                    else if (packetName == MessagePacketNameEnum.REQVI) response = MessageRequestVersionInfo(reader);
                    else if (packetName == MessagePacketNameEnum.LOGIN) response = MessageLogin(reader, sendEventArgs);
                    else if (packetName == MessagePacketNameEnum.LOGOT) response = MessageLogout(reader, sendEventArgs);
                    else if (packetName == MessagePacketNameEnum.ALIVE) response = MessageAlive(reader);
                }
            }
            catch (Exception ex)
            {
                SocketListener.Log.ExceptionLog(ex, "전문 생성 오류");

                /// 바디 생성 오류시 MMERR 전문 생성
                response = MessageErrorMakeMessage(reader);
            }
            return response;
        }

        #region Create Message
        /// <summary>
        /// 버전 정보
        /// </summary>
        private static MessageStream MessageRequestVersionInfo(MessageReader reader)
        {	
            DataTable dtResult = null;
            RESULT result = DbModule.Instance.FAS_CheckDeployInfo(reader.GetParam(0), ref dtResult);

            string repeatRowLength = dtResult.Rows.Count.ToString();    // row count
            string repeatColLength = "6";                               // column count
            List<string> repeatList = new List<string>();

            foreach (System.Data.DataRow dr in dtResult.Rows)
            {
                repeatList.Add(dr["BUILD_VER"].ToString());
                repeatList.Add(dr["FILE_TYPE"].ToString());
                repeatList.Add(dr["FILE_NAME"].ToString());
                repeatList.Add(dr["FILE_SIZE"].ToString());
                repeatList.Add(dr["FILE_SVR_PATH"].ToString());
                repeatList.Add(dr["FILE_LOCAL_PATH"].ToString());
            }

            /// 전문 생성
            MessageStream response = MessageStream.Response(MessagePacketNameEnum.REQVI, reader.Header, repeatRowLength, repeatColLength);
            response.AddRepeater(repeatList.ToArray());
			
            return response;
        }

        /// <summary>
        /// 로그인
        /// </summary>
        private MessageStream MessageLogin(MessageReader reader, SocketAsyncEventArgs receiveSendEventArgs)
        {	
            MessageStream response = null;
            string user_id = reader.GetParam(0);
            string user_pw = reader.GetParam(1);
            string user_name = "";
            string recv_faxbox_id = "";
            string send_faxbox_id = "";

            /// ipep.ToString() : 100.100.106.230:2038
            /// ipep.Address.ToString() : 100.100.106.230
            IPEndPoint ipep = (IPEndPoint)receiveSendEventArgs.AcceptSocket.RemoteEndPoint;

            RESULT result = DbModule.Instance.FAS_LoginAgentClient(user_id, user_pw, ipep.ToString(), ref user_name, ref recv_faxbox_id, ref send_faxbox_id);

            if (result == RESULT.F_DB_NOTEXIST_USER_ID ||
                result == RESULT.F_DB_PASSWORD_MISMATCH)
            {
                response = MessageStream.Response(MessagePacketNameEnum.LOGIN, reader.Header, false, "사용자 ID가 없거나 암호가 잘못되었습니다.");
            }
            else if (result == RESULT.SUCCESS ||
                     result == RESULT.F_DB_LOGIN_DUPLICATED)
            {
                /// /CLOSE 전문 발송 (중복 로그인)
                if (result == RESULT.F_DB_LOGIN_DUPLICATED)
                {
                    FACInfo oldFacInfo = FACContainer.FindFACInfo(user_id);
                    if (oldFacInfo != null)
                    {
                        if (this.OnLoginDuplicated != null) this.OnLoginDuplicated(oldFacInfo);
                    }
                }

                DataHoldingUserToken receiveSendToken = (receiveSendEventArgs.UserToken as DataHoldingUserToken);

                /// Login 된 Client 로 추가
                FACContainer.Update(receiveSendEventArgs.AcceptSocket, user_id, user_name);

                response = MessageStream.Response(MessagePacketNameEnum.LOGIN, reader.Header, "성공");
                response.AddPrameters(reader.GetParam(0));
                response.AddPrameters(user_name);
                response.AddPrameters(recv_faxbox_id);
                response.AddPrameters(send_faxbox_id);
                response.AddPrameters(((int)Config.CLIENT_ALIVE_INTERVAL).ToString());
                response.AddPrameters(((int)Config.CLIENT_LIMIT_RESPONSE_TIME).ToString());
                response.AddPrameters(Config.HOME_PATH_HTTP);
            }
            else
            {
                response = MessageStream.Response(MessagePacketNameEnum.LOGIN, reader.Header, false, result.ToString());
            }
			
            return response;
        }

        /// <summary>
        /// 로그아웃
        /// </summary>
        private MessageStream MessageLogout(MessageReader reader, SocketAsyncEventArgs receiveSendEventArgs)
        {
            RESULT result = RESULT.EMPTY;

            /// 등록된 user_id 와 IP endpoint 가 같은지 확인한다.
            /// 다르다면 중복 로그인으로 로그아웃을 요청한 것이므로
            /// 디비 처리를 하지 않는다.
            string user_id = reader.GetParam(0);
            EndPoint endPoint = receiveSendEventArgs.AcceptSocket.RemoteEndPoint;
            EndPoint endPointComp = FACContainer.FindFACInfo(user_id).RemoteEndPoint;

			if (endPointComp == null)
			{
				result = DbModule.Instance.FAS_LogoutAgentClient(reader.GetParam(0));
			}
			else
			{
				if (endPoint.ToString() == endPointComp.ToString())
					result = DbModule.Instance.FAS_LogoutAgentClient(reader.GetParam(0));
				else
					result = RESULT.SUCCESS;
			}

            /// 성공 실패 처리
            if (result == RESULT.SUCCESS)
                return MessageStream.Response(MessagePacketNameEnum.LOGOT, reader.Header, "성공", reader.GetParam(0));
            else
                return MessageStream.Response(MessagePacketNameEnum.LOGOT, reader.Header, false, "로그아웃 중 오류가 발생하였습니다.");
        }

        /// <summary>
        /// ALIVE
        /// </summary>
        private MessageStream MessageAlive(MessageReader reader)
        {
            return MessageStream.Response(MessagePacketNameEnum.ALIVE, reader.Header);
        }

        /// <summary>
        /// 접속 수락 응답
        /// </summary>
        public MessageStream MessageAccept()
        {
            return MessageStream.Notice(MessagePacketNameEnum.ACCPT, true, "성공");
        }

        ///// <summary>
        ///// 팩스 수신 알림
        ///// </summary>
        //public MessageStream MessageNotifyTextMessage(SocketAsyncEventArgs receiveSendEventArgs)
        //{
        //    return MessageStream.Notice(MessagePacketNameEnum.NTFTM, message);
        //}

        /// <summary>
        /// 중복 로그인 알림
        /// </summary>
        private MessageStream MessageLoginDuplicated(string userId)
        {
            return MessageStream.Notice(MessagePacketNameEnum.CLOSE, userId, "1");
        }

        /// <summary>
        /// 전문 헤더 읽기 오류
        /// </summary>
        private MessageStream MessageErrorHeader()
        {
            return MessageStream.Notice(MessagePacketNameEnum.ERRHD, false, "전문 헤더 읽기 오류가 발생하였습니다.");
        }

        /// <summary>
        /// 전문 생성 중 오류
        /// </summary>
        private MessageStream MessageErrorMakeMessage(MessageReader reader)
        {
            return MessageStream.Response(MessagePacketNameEnum.ERRMM, reader.Header, false, "전문 생성 중 오류가 발생하였습니다.");
        }
        #endregion
    }
}
