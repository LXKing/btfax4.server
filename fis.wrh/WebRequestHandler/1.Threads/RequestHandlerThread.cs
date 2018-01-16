using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Threading;
using System.IO;
using System.Net;
using Btfax.CommonLib;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Alarm;

namespace WebRequestHandler
{
    class RequestHandlerThread : BtfaxThread
    {
        #region constant
        static readonly int QUEUE_POLLING_SLEEP = 500; // 0.5초
        #endregion

        #region type
        protected struct HEADER
        {
            public string Category;
            public string CommandNo;
            public string Command;
            public string RequesterId;
            public string DateTime;
        }
        #endregion

        #region override
        protected override void ThreadEntry()
        {
            HttpListenerContext context;

            AppLog.Write(LOG_LEVEL.MSG, string.Format("### [{0}] 큐 폴링 쓰레드 시작 ###", m_nThreadNo));
            while (!BtfaxThread.Stop)
            {
                Thread.Sleep(QUEUE_POLLING_SLEEP);
                context = WebRequestQueue.Instance.Dequeue();
                if (context == null)
                    continue;

                try
                {
                    RequestHandler(context);
                }
                catch (Exception ex)
                {
                    AppLog.ExceptionLog(ex, string.Format("[{0}] WEB 요청 처리시, 예외가 발생하였습니다.", m_nThreadNo));
                }
            }

            AppLog.Write(LOG_LEVEL.MSG, string.Format("### [{0}] 큐 폴링 쓰레드 종료 ###", m_nThreadNo));
        }
        #endregion

        #region implementation
        protected void RequestHandler(HttpListenerContext p_context)
        {
            int nPos;
            string strUrl;
            string strMsg, strResponseMsg;
            XmlDocument xml;
            HEADER header;
            RESULT result;

            // HTTP:REQUEST 로그
            AppLog.Write(LOG_LEVEL.TRC, string.Format("[{0}][REQ][Remote={1:15}][{2:4}][Url={3}]", 
                                                      m_nThreadNo,
                                                      p_context.Request.RemoteEndPoint.Address,
                                                      p_context.Request.HttpMethod,
                                                      p_context.Request.Url));

            result = RESULT.SUCCESS;
            header = new HEADER();
            xml = new XmlDocument();
            strMsg = "";
            try
            {
                // WRH MSG 추출 <- URL
                strUrl = p_context.Request.RawUrl;
                strUrl = strUrl.Replace("%3C", "<");
                strUrl = strUrl.Replace("%3E", ">");
                strUrl = strUrl.Replace("%20", " ");

                nPos = strUrl.IndexOf("<WRH_MSG>");
                strMsg = strUrl.Substring(nPos);


                // 헤더 파싱
                if (!ParseHeader(strMsg, ref xml, ref header))
                    result = RESULT.F_PARSE_ERROR;
            }
            catch (Exception ex)
            {
                result = RESULT.F_PARSE_ERROR;
            }

            
            // 요청 처리
            strResponseMsg = "";
            if (result == RESULT.SUCCESS)
            {   
                switch (header.Category)
                {
                    case "OUTBOUND":
                        switch (header.CommandNo)
                        {
                            case "1":
                                Handle_OUTBOUND_01(xml, strMsg, header, out strResponseMsg);
                                break;

                            default:
                                result = RESULT.F_SYSTEMHEADER_INVALID_COMMAND_NO;
                                break;
                        }
                        break;

                    case "INBOUND":
                        switch (header.CommandNo)
                        {
                            case "1":
                                result = Handle_INBOUND_01(xml, strMsg, header, out strResponseMsg);
                                break;

                            default:
                                result = RESULT.F_SYSTEMHEADER_INVALID_COMMAND_NO;
                                break;
                        }
                        break;

                    default:
                        result = RESULT.F_SYSTEMHEADER_INVALID_CATEGORY;
                        break;
                }
            }

            // 에러 응답 메세지 생성
            if( result != RESULT.SUCCESS )
            {
                strResponseMsg = ComposeErrorResponse(result);

                // 알람메시지 전송
                AlarmAPI.Instance.SendFaultAlarm((UInt32)result);
            }

            // HTTP:RESPONSE 로그
            string strResponseMsgLog;
            strResponseMsgLog = strResponseMsg.Replace(   "\r\n", "");
            strResponseMsgLog = strResponseMsgLog.Replace(">  <",   "><");
            strResponseMsgLog = strResponseMsgLog.Replace(">    <", "><");
            AppLog.Write(LOG_LEVEL.TRC, string.Format("[{0}][RES][Remote={1:15}][Msg={2}]",
                                                      m_nThreadNo,
                                                      p_context.Request.RemoteEndPoint.Address,
                                                      strResponseMsgLog));

            // 응답 메세지 전송
            try
            {
                p_context.Response.AddHeader("Content-Type",                        "text/plain" );
                p_context.Response.AddHeader("Access-Control-Allow-Origin",         "*");
                p_context.Response.AddHeader("Access-Control-Allow-Methods",        "POST,GET,PUT,DELETE,OPTIONS");
                p_context.Response.AddHeader("Access-Control-Allow-Credentials",    "true" );
                p_context.Response.AddHeader("Access-Control-Max-Age",              "86400");
                p_context.Response.AddHeader("Access-Control-Allow-Headers",        "X-Requested-With,X-HTTP-Method_override,Content-Type,Accept");

                StreamWriter writer = new StreamWriter(p_context.Response.OutputStream, Encoding.UTF8);
                writer.Write(strResponseMsg);
                writer.Close();
            }
            catch(Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[{0}] WEB 요청에 대한 처리 중, 예외가 발생하였습니다.", m_nThreadNo));
                return;
            }
        }

        protected bool ParseHeader(string p_strMsg, ref XmlDocument p_xml, ref HEADER p_header)
        {
            try
            {   
                p_xml.LoadXml(p_strMsg);
                XmlElement headerTag = p_xml.SelectSingleNode("WRH_MSG/HEADER") as XmlElement;

                XmlElement element = headerTag.SelectSingleNode("Category") as XmlElement;
                p_header.Category = element.InnerText;

                element = headerTag.SelectSingleNode("CommandNo") as XmlElement;
                p_header.CommandNo = element.InnerText;

                element = headerTag.SelectSingleNode("Command") as XmlElement;
                p_header.Command = element.InnerText;

                element = headerTag.SelectSingleNode("RequesterId") as XmlElement;
                p_header.RequesterId = element.InnerText;

                element = headerTag.SelectSingleNode("DateTime") as XmlElement;
                p_header.DateTime = element.InnerText;
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[{0}] WRH 메세지 헤더 파싱중 예외가 발생하였습니다.[{1}]", m_nThreadNo, p_strMsg));
                return false;
            }

            return true;
        }

        protected string ComposeErrorResponse(RESULT p_result)
        {
            try
            {
                return string.Format("<WRH_MSG><RESULT><Result>F</Result><Reason>{0}{1}</Reason><ReasonName>{2}</ ReasonName ></RESULT></WRH_MSG>",
                                        'H', p_result.ToString("D3"), p_result.ToString());
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[{0}] WRH 에러 메세지 생성 중 예외가 발생하였습니다.[{1}]", m_nThreadNo, p_result));
                return "<WRH_MSG><RESULT><Result>F</Result><Reason>H381</Reason><ReasonName>F_COMPOSE_ERROR</ ReasonName ></RESULT></WRH_MSG>";
            }
        }

        protected RESULT Handle_OUTBOUND_01(XmlDocument p_xml, string p_strMsg, HEADER p_header, out string p_strResponseMsg)
        {
            XmlElement tagRoot, tagHeader, tagBody, tagResult, tag;
            decimal faxID = 0;
            char    chWaitType = ' ';
            bool    bWait = false;
            
            p_strResponseMsg = "";

            //// 요청 메세지 파싱
            try
            {
                tagRoot = p_xml.SelectSingleNode("WRH_MSG") as XmlElement;
                tagHeader = p_xml.SelectSingleNode("WRH_MSG/HEADER") as XmlElement;
                tagBody = p_xml.SelectSingleNode("WRH_MSG/BODY") as XmlElement;


                tag = tagBody.SelectSingleNode("FaxId") as XmlElement;
                faxID = Convert.ToDecimal(tag.InnerText);

                tag = tagBody.SelectSingleNode("WaitType") as XmlElement;
                chWaitType = Convert.ToChar(tag.InnerText);
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[{0}] WRH 메세지 파싱 중, 예외가 발생하였습니다.[Request={1}]", m_nThreadNo, p_strMsg));
                return RESULT.F_PARSE_ERROR_BODY;
            }

            //// 요청 처리
            try
            {
                // 대기 건 조회
                if (WaitListMgr.Instance.IsWaitList(faxID, chWaitType))
                    bWait = true;
                else
                    bWait = false;
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[{0}] WRH 메세지 요청 처리 중, 예외가 발생하였습니다.[Request={1}]", m_nThreadNo, p_strMsg));
                return RESULT.F_PARSE_ERROR_BODY;
            }

            //// 응답 메세지 생성
            try
            {
                tagResult = p_xml.CreateElement("RESULT");
                tagRoot.AppendChild(tagResult);

                tag = p_xml.CreateElement("Result"); tag.InnerText = "S";
                tagResult.AppendChild(tag);

                tag = p_xml.CreateElement("Reason"); tag.InnerText = "H001";
                tagResult.AppendChild(tag);

                tag = p_xml.CreateElement("ReasonName"); tag.InnerText = "SUCCESS";
                tagResult.AppendChild(tag);

                tag = tagHeader.SelectSingleNode("Command") as XmlElement;
                tag.InnerText = '/' + p_header.Command;

                tag = p_xml.CreateElement("WaitYN"); 
                if( bWait ) tag.InnerText = "Y";
                else        tag.InnerText = "N";
                tagBody.AppendChild(tag);


                //  응답 문자열로 변환
                MemoryStream stream = new MemoryStream();
                p_xml.Save(stream);

                stream.Seek(0, SeekOrigin.Begin);
                StreamReader reader = new StreamReader(stream);
                p_strResponseMsg = reader.ReadToEnd();

                reader.Close();
                stream.Close();
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[{0}] WRH 응답 메세지 생성 중, 예외가 발생하였습니다.[Request={1}]", m_nThreadNo, p_strMsg));
                return RESULT.F_COMPOSE_ERROR;
            }

            return RESULT.SUCCESS;
        }

        protected RESULT Handle_INBOUND_01(XmlDocument p_xml, string p_strMsg, HEADER p_header, out string p_strResponseMsg)
        {
            XmlElement tagRoot, tagHeader, tagBody, tagResult, tag;
            decimal faxID = 0;
            string  strSplitMethod = "";
            string  strSplitDetail = "";
            List<DbModule.SPLIT_FILE_INFO> outFileInfos = new List<DbModule.SPLIT_FILE_INFO>();
            
            p_strResponseMsg = "";

            //// WRH 메세지 파싱
            try
            {
                tagRoot = p_xml.SelectSingleNode("WRH_MSG") as XmlElement;
                tagHeader = p_xml.SelectSingleNode("WRH_MSG/HEADER") as XmlElement;
                tagBody = p_xml.SelectSingleNode("WRH_MSG/BODY") as XmlElement;


                tag = tagBody.SelectSingleNode("FaxId") as XmlElement;
                faxID = Convert.ToDecimal(tag.InnerText);

                tag = tagBody.SelectSingleNode("SplitMethod") as XmlElement;
                strSplitMethod = tag.InnerText;

                tag = tagBody.SelectSingleNode("SplitDetail") as XmlElement;
                strSplitDetail = tag.InnerText;
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[{0}] WRH 메세지 파싱중 예외가 발생하였습니다.[Request={1}]", m_nThreadNo, p_strMsg));
                return RESULT.F_PARSE_ERROR_BODY;
            }

            //// 요청 처리
            try
            {
                // 수신 건 조회
                DbModule.RECV_INFO recvInfo;

                if (!DbModule.Instance.GetRecvFaxInfo(faxID, out recvInfo))
                    return RESULT.F_DB_NOTEXIST_FAX;

                string strSrcPath;         // 이미지 파일 디렉토리 전체 경로
                string strSrcFileFullName; // 확장자 포함 파일 이름
                string strSrcFileName;     // 확장자 제외 파일이름

                strSrcPath          = Config.INBOUND_TIF_FULL_PATH + "\\" + recvInfo.strTifFile.Substring(0, recvInfo.strTifFile.LastIndexOf("\\") + 1);
                strSrcFileFullName  = recvInfo.strTifFile.Substring(recvInfo.strTifFile.LastIndexOf("\\")+1);
                strSrcFileName      = strSrcFileFullName.Substring(0, strSrcFileFullName.LastIndexOf('.'));
            
                
                // TIF 원본 파일 다운로드
                AppLog.Write(LOG_LEVEL.MSG, string.Format("[{0}][ACT] 파일 다운로드 시작 {1} => {2}", 
                                m_nThreadNo,
                                strSrcPath+"\\"+strSrcFileFullName, 
                                Config.TIF_MANIPULATE_PATH+"\\"+strSrcFileFullName));
                File.Copy(strSrcPath+"\\"+strSrcFileFullName,
                          Config.TIF_MANIPULATE_PATH+"\\"+strSrcFileFullName, 
                          true);
                AppLog.Write(LOG_LEVEL.MSG,string.Format("[{0}][ACT] 파일 다운로드 완료", m_nThreadNo));


                // TIF 원본 파일 분리
                AppLog.Write(LOG_LEVEL.MSG, string.Format("[{0}][ACT] TIF 페이지 분리 시작. Method={1} Detail={2} File={3}",
                                m_nThreadNo, strSplitMethod, strSplitDetail, strSrcFileFullName));
                if (strSplitMethod == "A")  // 페이지 수 동일, 분리
                {   
                    if (!TifManipulator.SplitTif(Config.TIF_MANIPULATE_PATH,
                                                    strSrcFileFullName,
                                                    Convert.ToInt32(strSplitDetail),
                                                    strSrcFileName,
                                                    ref outFileInfos))
                    {
                        return RESULT.F_TIFFPROCESS_FAIL_TO_EXTRACT;
                    }
                }
                else if (strSplitMethod == "I") // 페이지수 다르게, 분리
                {
                    if (!TifManipulator.SplitTif(Config.TIF_MANIPULATE_PATH,
                                                    strSrcFileFullName,
                                                    strSplitDetail,
                                                    strSrcFileName,
                                                    ref outFileInfos))
                    {
                        return RESULT.F_TIFFPROCESS_FAIL_TO_EXTRACT;
                    }
                }
                else
                {
                    return RESULT.F_PARSE_ERROR_BODY;
                }
                AppLog.Write(LOG_LEVEL.MSG, string.Format("[{0}][ACT] TIF 페이지 분리 완료", m_nThreadNo));
                

                // 분리 TIF 업로드
                AppLog.Write(LOG_LEVEL.MSG, string.Format("[{0}][ACT] 파일 업로드 시작", m_nThreadNo));
                foreach (DbModule.SPLIT_FILE_INFO fileInfo in outFileInfos)
                {   
                    File.Copy(Config.TIF_MANIPULATE_PATH+"\\"+fileInfo.p_strFile,
                              strSrcPath+"\\"+fileInfo.p_strFile,
                              true);
                    File.Delete(Config.TIF_MANIPULATE_PATH + "\\" + fileInfo.p_strFile);
                }
                AppLog.Write(LOG_LEVEL.MSG, string.Format("[{0}][ACT] 파일 업로드 완료", m_nThreadNo));
                File.Delete(Config.TIF_MANIPULATE_PATH + "\\" + strSrcFileFullName);


                // 기존 SPLIT 정보 DB 업데이트
                AppLog.Write(LOG_LEVEL.MSG, string.Format("[{0}][ACT] DB 정보 업데이트 시작", m_nThreadNo));
                if (!DbModule.Instance.DeleteSplitInfos(recvInfo.faxID))
                    return RESULT.F_DB_ERROR;

                // SPLIT 정보 DB 업데이트

                foreach (DbModule.SPLIT_FILE_INFO fileInfo in outFileInfos)
                {
                    if (!DbModule.Instance.InsertSplitInfo(recvInfo.faxID, fileInfo))
                        return RESULT.F_DB_UPDATE_ERROR;
                }
                if (!DbModule.Instance.UpdateSplitYN(recvInfo.faxID, 'Y'))
                    return RESULT.F_DB_UPDATE_ERROR;
                AppLog.Write(LOG_LEVEL.MSG, string.Format("[{0}][ACT] DB 정보 업데이트 완료", m_nThreadNo));
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[{0}] WRH 메세지 요청 처리 중, 예외가 발생하였습니다.[Request={1}]", m_nThreadNo, p_strMsg));
                return RESULT.F_HANDLE_REQUEST;
            }

            //// 성공 응답 메세지 생성
            try
            {
                tagResult = p_xml.CreateElement("RESULT");
                tagRoot.AppendChild(tagResult);
                
                tag = p_xml.CreateElement("Result"); tag.InnerText = "S";
                tagResult.AppendChild(tag);

                tag = p_xml.CreateElement("Reason"); tag.InnerText = "H001";
                tagResult.AppendChild(tag);

                tag = p_xml.CreateElement("ReasonName"); tag.InnerText = "SUCCESS";
                tagResult.AppendChild(tag);

                tag = tagHeader.SelectSingleNode("Command") as XmlElement;
                tag.InnerText = '/' + p_header.Command;

                tag = p_xml.CreateElement("FileCount"); tag.InnerText = outFileInfos.Count().ToString();
                tagBody.AppendChild(tag);
                
                //  응답 문자열로 변환
                MemoryStream stream = new MemoryStream();
                p_xml.Save(stream);

                stream.Seek(0, SeekOrigin.Begin);
                StreamReader reader = new StreamReader(stream);
                p_strResponseMsg = reader.ReadToEnd();

                reader.Close();
                stream.Close();
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[{0}] WRH 응답 메세지 생성 중, 예외가 발생하였습니다.[Request={1}]", m_nThreadNo, p_strMsg));
                return RESULT.F_COMPOSE_ERROR;
            }

            return RESULT.SUCCESS;
        }
        #endregion
    }
}
