using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.IO;
using Btfax.CommonLib;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Log;
using AdapterOut.PacketParsing;
using AdapterOut.Db;
using AdapterOut.Util;
 
namespace AdapterOut.Threading
{
    class TcpSessionThread : BtfaxThread
    {
        #region constant
        static readonly int     BUFFER_SIZE                 = 1000 * 1000;
        static readonly int     RESPONSE_BUFFER_SIZE        = 200;
        static readonly int     ONE_SECOND                  = 1000 * 1000;
        static readonly int     PACKET_RECEIVE_MAX_TIME     = 30;    
        static readonly char[]  DOCFILES_DELIMITER          = { '|' };
        static readonly char[]  DOCFILE_INFO_DELIMITER      = { ':' };
        #endregion

        #region constructor && destructor
        public TcpSessionThread()
        {
            m_socketLog.LogPrefix = "sock_" + Config.PROCESS_NO;
            m_socketLog.LogLevel = Config.LOG_LEVEL;

            m_parser.LayoutPathFile = string.Format("{0}\\{1}", Config.PACKET_XML_PATH, Config.PACKET_XML_FILE);
        }
        ~TcpSessionThread() { }
        #endregion

        #region methods
        public void ThreadEntry(Object state)
        {
            SessionLoop_();

            if (m_sock != null)
                m_sock = null;

            if (m_remote != null)
            {
                m_socketLog.Write(LOG_LEVEL.MSG, string.Format("[{0}] 연결 종료", m_remote.Address));
                m_remote = null;
            }

            Running = false;
            ClearInfos();
        }

        protected void SessionLoop_()
        {
            bool recvFinish = false;
            string strPacketLen = "";
            string strPacket = "";
            string strTrNo = "";
            int packetLen = 0;

            int rcvlen = 0;
            int total_rcvlen = 0;
            int len = 0;

            try
            {
                m_remote = (IPEndPoint)m_sock.RemoteEndPoint;
                m_socketLog.Write(LOG_LEVEL.MSG, string.Format("[{0}/{1}] 연결 시작", m_remote.Address.ToString(), m_remote.Port));
                
                Array.Clear(m_buffer, 0, m_buffer.Length);
                Array.Clear(m_responBuffer, 0, m_responBuffer.Length);

                for (int i = 1; !BtfaxThread.Stop; i++)
                {
                    //// 타임아웃 시간 초과시 처리 ////
                    if (i > PACKET_RECEIVE_MAX_TIME)
                    {   
                        Log_Error_(RESULT.F_SOCK_RECEIVE_LEN_NOT_ENOUGHT, string.Format("수신전문길이:{0:D06} 규정전문길이:{1:D06} 전문이 규정된 길이만큼 수신되지 않았습니다 ", rcvlen, packetLen));
                        ResponseSend_(RESULT.F_SOCK_RECEIVE_LEN_NOT_ENOUGHT);
                        break;
                    }
                    
                    //// 소켓 이벤트 대기 ////
                    if (!m_sock.Poll(ONE_SECOND, SelectMode.SelectRead))
                        continue;

                    //// 수신 처리 ////
                    len = m_sock.Receive(m_buffer, rcvlen, BUFFER_SIZE - rcvlen, SocketFlags.None);
                    if (len == 0)
                    {
                        //// 접속 종료 처리 ////
                        m_socketLog.Write(LOG_LEVEL.MSG, string.Format("[{0}] 연결 끊어짐", m_remote.Address.ToString()));
                        break;
                    }
                    else if (len < 0)
                    {
                        m_socketLog.Write(LOG_LEVEL.ERR, string.Format("[{0}] 수신오류. 리턴:{1} ", m_remote.Address.ToString(), len));
                        continue;
                    }

                    //// 수신 완료시, Remote Close 까지 대기 ////
                    if (recvFinish)
                        continue;  

                    string strPartialPacket = Encoding.Default.GetString(m_buffer, rcvlen, len);
                    m_socketLog.Write(LOG_LEVEL.MSG, string.Format("[{0}] 수신 ({1})[{2}] ", m_remote.Address.ToString(), len, strPartialPacket));

                    strPacket += strPartialPacket;
                    rcvlen += len;

                    //// 수신 길이 처리 ////
                    if (packetLen == 0 )
                    {
                        if (rcvlen < Config.SIZE_FIELD_POS + Config.SIZE_FIELD_LEN) // 길이필드까지 수신되지 않았으면, 전문수신 계속
                            continue;

                        //// 전문 길이 얻기 ////
                        strPacketLen = Encoding.Default.GetString(m_buffer, Config.SIZE_FIELD_POS, Config.SIZE_FIELD_LEN);
                        if (!Int32.TryParse(strPacketLen, out packetLen))
                        {   
                            Log_StepMsg_(string.Format("전문길이필드 CONVERT오류 [전문길이:{0}] [변환데이터:{1}] ", packetLen, strPacketLen));
                            ResponseSend_(RESULT.F_PARSE_ERROR);
                            break;
                        }
                    }
                    
                    //// 전문번호 SET ////
                    if (String.IsNullOrEmpty(strTrNo))
                    {   
                        ClearError_();
                        strTrNo = Encoding.Default.GetString(m_buffer, Config.TRNO_FIELD_POS, Config.TRNO_FIELD_LEN);
                        strTrNo = strTrNo.TrimEnd(" ".ToCharArray());
                        SetTrNo_(strTrNo);
                    }
                    
                    //// 전문길이 만큼 수신하지 않았으면, 전문 수신 계속 ////
                    if (rcvlen < packetLen) 
                        continue;

                    //// 수신전문 길이가 규정된 길이보다 클때 실패처리 ////
                    if (rcvlen > packetLen)
                    {
                        Log_Error_(RESULT.F_SOCK_RECEIVE_LEN_TOO_MANY, string.Format("수신전문길이:{0:D06} 규정전문길이:{1:D06} 전문이 규정된길이를 초과하였습니다. ", rcvlen, packetLen));
                        ResponseSend_(RESULT.F_SOCK_RECEIVE_LEN_TOO_MANY);
                        break;
                    }   

                    //// 수신 완료 플래그 SET ////
                    recvFinish = true;
                    total_rcvlen = rcvlen;
                    rcvlen = 0;

                    Log_StepMsg_(string.Format("수신 ({0}) |{1}|", total_rcvlen, strPacket));

					////// 전문 처리 -> 3회까지 처리////
					RESULT result = ProcessPacket_(m_trNo, m_buffer);
					
                    switch (result)
                    {
                        case RESULT.SUCCESS:
                            Log_Success_("요청건 처리작업 성공");
                            break;

                        case RESULT.F_SYSTEM_ERROR:
                        case RESULT.F_DB_ERROR:
                            Log_Error_(result, "요청건 처리 작업 실패");

                            //// 팩스요청데이터 로컬보관 ////
                            WriteRecoveryData(m_buffer, 0, total_rcvlen);

                            //// 리커버리 쓰레드 시작 ////
                            if(Config.RECOVERY_YN == "Y")
                                RecoveryThread.Instance.StartThread();
                            break;

                        default:
                            Log_Error_(result, "요청건 처리 작업 실패");
                            break;
                    }

                    //// 응답 전문 전송 ///
                    ResponseSend_(result);
                   
                    break;
                }
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, String.Format("세션 소켓 쓰레드에서 다음과 같은 오류가 발생하였습니다. {0}", m_remote.Address));
                ResponseSend_(RESULT.F_SOCK_RECEIVE_ERROR);
            }
        }

        private void WriteRecoveryData(byte[] p_data, int p_startIdx, int p_count)
        {
            string path = string.Format("{0}\\recovery\\", System.Windows.Forms.Application.StartupPath);
            if (!Directory.Exists(path))
                Directory.CreateDirectory(path);

            string fileName = string.Format("{0:yyMMddHHmmss.ff}.recovery", DateTime.Now);
            string fileFullName = string.Format("{0}\\{1}", path, fileName);
            using (FileStream fs = File.Open(fileFullName, FileMode.CreateNew, FileAccess.Write, FileShare.ReadWrite))
            {
                fs.Write(p_data, p_startIdx, p_count);
                fs.Flush();
                fs.Close();
            }
        }

        protected void ResponseSend_(RESULT p_result)
        {
            int reasonCode = (int)p_result;
            string desc = p_result.ToString();

            byte[] buf = Encoding.Default.GetBytes(String.Format("{0:D3}|{1:D3}|{2}|{3}", RESPONSE_BUFFER_SIZE, reasonCode, desc, m_faxID));
            Buffer.BlockCopy(buf, 0, m_responBuffer, 0, buf.Length);
            if (m_sock != null && m_sock.Connected)
                m_sock.Send(m_responBuffer);
        }

        protected RESULT ProcessPacket_(string p_strTrNo, byte[] p_packet)
        {
            //// 전문 파싱. (전문 레이아웃 파일이용) ////
			//// 실패시 3회 처리 
			RESULT result = RESULT.EMPTY;
			for (int i = 0; i < 3; i++)
			{
				result = ParsingByPacketLayout_(m_parser, "전문___" + m_trNo, p_packet);
				if (result == RESULT.SUCCESS)
					break;

				Log_StepMsg_(string.Format("[RESULT={0}]전문파싱(전문 레이아웃 파일이용) 실패 [전문번호={1}][시도횟수:{2}]", result, p_strTrNo, i + 1));
			}

			if (result != RESULT.SUCCESS)
				return result;

            Log_StepMsg_(string.Format("[RESULT={0}]전문파싱(전문 레이아웃 파일이용) 성공 [전문번호={1}]", result, p_strTrNo));
            

            //// 전문 정보 분석 & 패킷XML 생성 ////
            result = ExtractInfos_(m_parser,
                                   ref m_reqType,
                                   ref m_nextProcessType,
                                   ref m_sendRequestInfo,
                                   ref m_packetXml,
                                   ref m_docPath,
                                   ref m_docInfos);
            
            if (result != RESULT.SUCCESS)
            {   
                Log_StepMsg_(string.Format("[RESULT={0}]전문 파싱 (전문 정보 분석) 실패 [전문번호={1}]", result, p_strTrNo));
                return result;
            }
            Log_StepMsg_(string.Format("[RESULT={0}]전문 파싱 (전문 정보 분석) 성공 [전문번호={1}]", result, p_strTrNo));            

            //// 최종 TIFF 파일 정보 SET ////
            ////  AOB 에서만, SEND_REQ.strLastTifFile 이 LAST TIF 디렉토리를 의미. ////
            m_sendRequestInfo.strLastTifFile = string.Format("{0}\\{1}\\", DateTime.Now.ToString("yyyy_MM"), DateTime.Now.ToString("dd"));
            string strTifFullPath = string.Format("{0}\\{1}", Config.FINISHED_TIF_PATH, m_sendRequestInfo.strLastTifFile);

            ////  최종 TIFF 일자별 디렉토리 생성 ////
            try
            {
                if (!Directory.Exists(strTifFullPath))
                    Directory.CreateDirectory(strTifFullPath);
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("일자별 TIF파일 디렉토리 ({0}) 생성 실패", strTifFullPath));
                return RESULT.F_DIR_FAIL_TO_CREATE_DIR;
            }

			//// 즉시발송요청 TIF처리 ////
			if (m_reqType == REQ_TYPE.FOD)
			{
				if (m_docInfos.Count < 0)
					return RESULT.F_SYSTEMHEADER_DOCFILE_INVALIED_DOC_INFO;

				DbModule.DOC_INFO docInfo = m_docInfos[0];
				if (docInfo == null)
					return RESULT.F_SYSTEMHEADER_DOCFILE_INVALIED_DOC_INFO;

				// DB에 REQ_DTL 데이터가 삽입 전이기 때문에 목적지는 알수 없음.
				// 소스파일 존재 유무만 체크후 나중에 복사.
				string strSrc = String.Format("{0}\\{1}\\{2}.{3}", Config.STG_HOME_PATH, Config.INPUT_DOCS_PATH, docInfo.strDocFile, docInfo.strDocExt);
				if (!File.Exists(strSrc))
					return RESULT.F_FILE_NOT_EXIST;
			}
			            
            //// DB 에 등록 ////
            result = InsertToDb_(m_trNo,
                                 m_reqType,
                                 m_nextProcessType,
                                 m_sendRequestInfo,
                                 m_sendRequestSiteInfo,
                                 m_packetXml,
                                 m_docPath,
                                 m_docInfos);
            
            return result;
        }

        protected RESULT ParsingByPacketLayout_(PacketParser p_parser, string p_strTrNo, byte[] p_packet)
        {   
            if (!p_parser.Load())
            {
                Log_Error_(RESULT.F_SYSTEM_ERROR, p_parser.ResultMsg);
                return RESULT.F_SYSTEM_ERROR;
            }
                     
            if (!p_parser.Parse(p_strTrNo, p_packet))
            {
                RESULT parsingResult = RESULT.F_PARSE_ERROR;
                if (p_parser.ResultCode != RESULT.EMPTY)
                    parsingResult = p_parser.ResultCode;

                return parsingResult;
            }
            
            return RESULT.SUCCESS;
        }
      
        protected RESULT ExtractInfos_(PacketParser p_parser,
                                     ref REQ_TYPE p_reqType,
                                     ref P_TYPE p_destProcessType,
                                     ref DbModule.SEND_REQUEST_AOB p_sendReqInfo,
                                     ref string p_strPacketXml,
                                     ref string p_strDocPath,
                                     ref List<DbModule.DOC_INFO> p_docInfos)
        {            
            p_destProcessType = P_TYPE.NONE;
            p_sendReqInfo.Clear();
            p_strPacketXml = "";
            p_docInfos.Clear();
            p_sendReqInfo.Clear();
            
            //// 시스템 헤더정보 얻기 ////
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.공통헤더버전", out p_sendReqInfo.strBtfaxHeaderVersion);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.전문길이", out p_sendReqInfo.strPacketLen);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.전문일시", out p_sendReqInfo.strPacketDateTime);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.인덱스번호", out p_sendReqInfo.strIndexNo);
            SetIndexNo(p_sendReqInfo.strIndexNo);

            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.전문번호", out p_sendReqInfo.strTrNo);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.요청유형", out p_sendReqInfo.strReqType);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.수신자팩스번호", out p_sendReqInfo.strRecipientFaxNo);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.수신자이름", out p_sendReqInfo.strRecipientName);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.발신자유형", out p_sendReqInfo.strReqSendType);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.발신자아이디", out p_sendReqInfo.strReqUserID);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.발신자이름", out p_sendReqInfo.strReqUserName);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.발신자번호", out p_sendReqInfo.strReqUserTelNo);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.전처리수행여부", out p_sendReqInfo.preProcessingYN);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.미리보기여부", out p_sendReqInfo.previewYN);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.승인필요여부", out p_sendReqInfo.approvedReq);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.발송예약여부", out p_sendReqInfo.reserveYN);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.발송예약일자", out p_sendReqInfo.strReserveDate);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.발송예약시각", out p_sendReqInfo.strReserveTime);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.발송우선순위", out p_sendReqInfo.priority);

            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.변환필요문서위치", out p_sendReqInfo.strConvFilePath);
            p_strDocPath = p_sendReqInfo.strConvFilePath;
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.변환필요문서리스트", out p_sendReqInfo.strConvFileName);
            p_parser.GetFieldValue("헤더___HEADER_O_COMMON.시험유형", out p_sendReqInfo.testType);
            
            //// 유효성 체크 ////
            //// 팩스번호 유효성 체크 ////
            if (p_sendReqInfo.strRecipientFaxNo != null)
                p_sendReqInfo.strRecipientFaxNo = p_sendReqInfo.strRecipientFaxNo.Replace("-", "");

            if (string.IsNullOrEmpty(p_sendReqInfo.strRecipientFaxNo))
                return RESULT.F_SYSTEMHEADER_FAXNO_INVALID;

            if (p_sendReqInfo.strReqUserTelNo != null)
                p_sendReqInfo.strReqUserTelNo = p_sendReqInfo.strReqUserTelNo.Replace("-", "");

			if (Config.ENCRYPT_FIELD_YN == "N")
			{
				if (!p_sendReqInfo.strRecipientFaxNo.Contains("@"))
				{
					if (!FnString.IsNumber(p_sendReqInfo.strRecipientFaxNo))
						return RESULT.F_SYSTEMHEADER_FAXNO_INVALID;
				}
			}

            p_sendReqInfo.strRecipientFaxNo = p_sendReqInfo.strRecipientFaxNo.Trim();

            //// 발송우선순위 유효성 체크 ////
            if (p_sendReqInfo.priority != "0" && p_sendReqInfo.priority != "1" && p_sendReqInfo.priority != "2") {
                return RESULT.F_SYSTEMHEADER_PRIORITY_INVALID;
            }

            //// 승인여부 유효성 체크 ////
            if (p_sendReqInfo.approvedReq != "Y" && p_sendReqInfo.approvedReq != "N")
                return RESULT.F_SYSTEMHEADER_APPROVEYN_INVALID;

            //// 발송예약 여부 유효성 체크 ////
            if (p_sendReqInfo.reserveYN != "Y" && p_sendReqInfo.reserveYN != "N")
                return RESULT.F_SYSTEMHEADER_RESERVEYN_INVALID;

            //// 발송 예약 일자, 발송 예약 시각 유효성 체크 ////
            p_sendReqInfo.strReserveDate = p_sendReqInfo.strReserveDate.Trim(); // 발송 예약 일자 가 시스템헤더에 Raw 유형으로 정의되어있으므로, 강제 트림처리함
            p_sendReqInfo.strReserveTime = p_sendReqInfo.strReserveTime.Trim(); // 발송 예약 시각 이 시스템헤더에 Raw 유형으로 정의되어있으므로, 강제 트림처리함
            if (p_sendReqInfo.reserveYN == "Y")
            {
                if (string.IsNullOrEmpty(p_sendReqInfo.strReserveDate))
                    return RESULT.F_SYSTEMHEADER_RESERVEDATE_INVALID;
                if (string.IsNullOrEmpty(p_sendReqInfo.strReserveTime))
                    return RESULT.F_SYSTEMHEADER_RESERVETIME_INVALID;
            }
            
            //// 요청유형 체크 및 다음 프로세스 선정 ////
            switch (Convert.ToInt32(p_sendReqInfo.strReqType))
            {
                case (int)REQ_TYPE.FOD:         p_destProcessType = P_TYPE.PSC; break;

                case (int)REQ_TYPE.TMK:
                case (int)REQ_TYPE.TMK_TPC:
                case (int)REQ_TYPE.TMK_DCV_TPC: p_destProcessType = P_TYPE.TMK; break;

                case (int)REQ_TYPE.DCV:
                case (int)REQ_TYPE.DCV_TPC:     p_destProcessType = P_TYPE.DCV; break;

                case (int)REQ_TYPE.TPC:         p_destProcessType = P_TYPE.TPC; break;

                default:
                    Log_Error_(RESULT.F_SYSTEMHEADER_REQTYPE_INVALID, string.Format("올바르지 않은 발송 요청 유형[{0}]", p_sendReqInfo.strReqType));
                    return RESULT.F_SYSTEMHEADER_REQTYPE_INVALID;
            }
            p_reqType = (REQ_TYPE)Convert.ToInt32(p_sendReqInfo.strReqType);

            //// 전처리 수행여부 ////
            if (p_sendReqInfo.preProcessingYN == "Y")
                p_destProcessType = P_TYPE.PRC;
            
            //// STATE 초기화 ////
            p_sendReqInfo.strState = string.Format("{0:D02}", (int)R_STATE.INIT);

            //// 첨부 파일 정보 분석 - [01:TMK]유형일경우만 제외 ////
            if (p_reqType != REQ_TYPE.TMK)
            {
                m_result = ExtractDocInfos_(p_reqType,
                                            p_sendReqInfo.strConvFilePath,
                                            p_sendReqInfo.strConvFileName,
                                            ref p_docInfos);
                if (m_result != RESULT.SUCCESS)
                    return m_result;
            }
            
            //// 전문 데이터 XML 얻기 ////
            p_strPacketXml = m_parser.GetPacketXml();
            if (string.IsNullOrEmpty(p_strPacketXml))
            {
                Log_Error_(RESULT.F_SYSTEM_ERROR, p_parser.ResultMsg);
                return RESULT.F_SYSTEM_ERROR;
            }
            

            return RESULT.SUCCESS;
        }

        protected RESULT ExtractDocInfos_(REQ_TYPE p_reqType, 
                                        string p_strDocPath, 
                                        string p_strDocFiles, 
                                        ref List<DbModule.DOC_INFO> p_docInfos)
        {   
            p_strDocPath.Trim();
            p_strDocFiles.Trim();

            if (string.IsNullOrEmpty(p_strDocFiles))
                return RESULT.F_SYSTEMHEADER_DOCFILELIST_FORMAT_INVALID;

            //// 즉시발송 유형 처리 ////
            if (p_reqType == REQ_TYPE.FOD)
            {
                DbModule.DOC_INFO docInfo = new DbModule.DOC_INFO();
                // 확장자 길이 체크
                int dotPos = p_strDocFiles.LastIndexOf('.');
                if (dotPos <= 0 || dotPos >= p_strDocFiles.Length - 2)
                    return RESULT.F_SYSTEMHEADER_DOCFILE_EXTENSION_NOTSUPPORTED;

                // 파일이름, 확장자, 첨부문서처리방식, 처리방식부가정보 추출
                docInfo.strDocFile = p_strDocFiles.Substring(0, dotPos);
                docInfo.strDocExt = p_strDocFiles.Substring(dotPos + 1);

                p_docInfos.Add(docInfo);
                return RESULT.SUCCESS;
            }
            

            //// TMK 요청 유형(01,11)인 경우, TMK 키워드가 없으면, 처음부분에 TMK 키워드를 삽입한다 ////
            if (p_reqType == REQ_TYPE.TMK || 
                p_reqType == REQ_TYPE.TMK_TPC || 
                p_reqType == REQ_TYPE.TMK_DCV_TPC)
            {
                if (string.Format("|{0}|", p_strDocFiles).IndexOf("|TMK|") < 0)
                    p_strDocFiles = (p_strDocFiles.Length > 0) ? "TMK|" + p_strDocFiles : "TMK";
            }
            

            //// 첨부문서스트 1차 토큰 분리. 구분자(|) ////
            string[] fileInfos = p_strDocFiles.Split(DOCFILES_DELIMITER);

            //// 키워드 처리 ('TMK') ////
            for (int i=0 ; i<fileInfos.Length ; ++i)
            {
                switch (fileInfos[i])
                {
                    case "TMK":
                        fileInfos[i] = "tmk.TIF:01";
                        break;
                }
            }
            

            //// 첨부문서정보 파싱 및 정보 등록 ////
            foreach (string strFileInfo in fileInfos)
            {
                // 첨부문서 2차 토큰 분리. 구분자(:)
                string[] details = strFileInfo.Split(DOCFILE_INFO_DELIMITER);
                if (details.Length < 2)
                {
                    return RESULT.F_SYSTEMHEADER_DOCFILELIST_FORMAT_INVALID;
                }

                DbModule.DOC_INFO docInfo = new DbModule.DOC_INFO();

                // 확장자 길이 체크
                int dotPos = details[0].LastIndexOf('.');
                if (dotPos <= 0 || dotPos >= details[0].Length - 2)
                    return RESULT.F_SYSTEMHEADER_DOCFILE_EXTENSION_NOTSUPPORTED;

                // 파일이름, 확장자, 첨부문서처리방식, 처리방식부가정보 추출
                docInfo.strDocFile = details[0].Substring(0, dotPos);
                docInfo.strDocExt = details[0].Substring(dotPos + 1);
                docInfo.strProcessingMode = details[1];
                if (details.Length >= 3)
                    docInfo.strTiffExtractPages = details[2];

                // 첨부문서 처리방식 & 확장자 체크
                string strExtCapital = docInfo.strDocExt.ToUpper();
                switch (Convert.ToInt32(docInfo.strProcessingMode))
                {   
                    case (int)DOC_PROCESSING_MODE.TMK:
                        break;

                    case (int)DOC_PROCESSING_MODE.DCV:
                        if (m_convertableDocs.IndexOf(string.Format(",{0},", strExtCapital)) < 0)
                            return RESULT.F_SYSTEMHEADER_DOCFILE_EXTENSION_NOTSUPPORTED;
                        break;

                    case (int)DOC_PROCESSING_MODE.TPC_EXTRACT:
                    case (int)DOC_PROCESSING_MODE.TPC_NO_EXTRACT:
						if (strExtCapital != "TIF" && strExtCapital != "TIFF")
							return RESULT.F_SYSTEMHEADER_DOCFILE_EXTENSION_NOTSUPPORTED;
                        break;

                    default:
                        return RESULT.F_SYSTEMHEADER_DOCMODE_NOTSUPPORTED;
                }

                // 파일정보 저장
                p_docInfos.Add(docInfo);
            }
            
            return RESULT.SUCCESS;
        }

        protected RESULT InsertToDb_(string                 p_strTrNo,
                                  REQ_TYPE                  p_reqType,
                                  P_TYPE                    p_destProcessType,
                                  DbModule.SEND_REQUEST_AOB p_aobReqInfo,
                                  DbModule.SEND_REQ_SITE    p_sendReqSiteInfo,
                                  string                    p_strPacketXml,
                                  string                    p_strDocPath,
                                  List<DbModule.DOC_INFO>   p_docInfos)
        {
            //// FAX발송요청 INSERT - BTF_FAX_SEND_MSTR ////
            decimal faxID = DbModule.Instance.InsertSendMaster(p_strTrNo, p_aobReqInfo);
            if (faxID < 0)
            {   
                faxID = DbModule.Instance.InsertSendMaster(p_strTrNo, p_aobReqInfo);
                if (faxID < 0)
                {
                    Log_Error_(RESULT.F_DB_ERROR, "발송요청건 정보를 INSERT하는 도중 오류가 발생하였습니다.");
                    return RESULT.F_DB_ERROR;
                }
            }
            
            p_aobReqInfo.faxId = faxID;
            SetFaxID(faxID);

            //// FAX발송요청 상세건 INSERT - BTF_FAX_SEND_DTL ////
            if (p_aobReqInfo.m_lstSendRequestDtlAobInfos.Count > 0)
                p_aobReqInfo.m_lstSendRequestDtlAobInfos.Clear();

            decimal faxDtlId = -1;
            if (p_aobReqInfo.strRecipientFaxNo.Contains("@@"))
            {

                //// 부서단축번호 주소록 처리 ////
                string strShortNumber = p_aobReqInfo.strRecipientFaxNo.Replace("@", "");
                if (m_lstAddressInfos.Count > 0)
                    m_lstAddressInfos.Clear();

                if (!DbModule.Instance.GetAddressInfos_Dept(p_aobReqInfo.strReqUserID, strShortNumber, ref m_lstAddressInfos) || m_lstAddressInfos.Count < 1)
                {
                    Log_Error_(RESULT.F_DB_ERROR, "동보발송을 위한 주소록 데이터를 얻는중 오류가 발생하였습니다.");
                    return RESULT.F_DB_ERROR;
                }

                foreach (DbModule.AddressInfo addrInfo in m_lstAddressInfos)
                {
                    faxDtlId = DbModule.Instance.InsertSendDetail(faxID, addrInfo.strFaxNo, addrInfo.strRecipientName, p_aobReqInfo);
                    if (faxDtlId < 0)
                    {
                        Log_Error_(RESULT.F_DB_ERROR, "발송요청상세건 정보를 INSERT하는 도중 오류가 발생하였습니다.");
                        m_lstAddressInfos.Clear();
                        return RESULT.F_DB_ERROR;
                    }
                    DbModule.SEND_REQUEST_DTL_AOB aobDtlReq = new DbModule.SEND_REQUEST_DTL_AOB();
                    aobDtlReq.faxId = faxID;
                    aobDtlReq.faxDtlId = faxDtlId;
                    aobDtlReq.strFaxNo = addrInfo.strFaxNo;
                    aobDtlReq.strRecipientName = addrInfo.strRecipientName;
                    aobDtlReq.strStateEach = p_aobReqInfo.strState;
                    aobDtlReq.strTiffPath = p_aobReqInfo.strLastTifFile;

                    p_aobReqInfo.m_lstSendRequestDtlAobInfos.Add(aobDtlReq);
                }
            }
            else if (p_aobReqInfo.strRecipientFaxNo.Contains("@"))
            {
                //// 개인단축번호 주소록 처리 ////
                string strShortNumber = p_aobReqInfo.strRecipientFaxNo.Replace("@", "");
                if (m_lstAddressInfos.Count > 0)
                    m_lstAddressInfos.Clear();

                if (!DbModule.Instance.GetAddressInfos(p_aobReqInfo.strReqUserID, strShortNumber, ref m_lstAddressInfos) || m_lstAddressInfos.Count < 1)
                {
                    Log_Error_(RESULT.F_DB_ERROR, "동보발송을 위한 주소록 데이터를 얻는중 오류가 발생하였습니다.");
                    return RESULT.F_DB_ERROR;
                }

                foreach (DbModule.AddressInfo addrInfo in m_lstAddressInfos)
                {
                    faxDtlId = DbModule.Instance.InsertSendDetail(faxID, addrInfo.strFaxNo, addrInfo.strRecipientName, p_aobReqInfo);
                    if (faxDtlId < 0)
                    {
                        Log_Error_(RESULT.F_DB_ERROR, "발송요청상세건 정보를 INSERT하는 도중 오류가 발생하였습니다.");
                        m_lstAddressInfos.Clear();
                        return RESULT.F_DB_ERROR;
                    }
                    DbModule.SEND_REQUEST_DTL_AOB aobDtlReq = new DbModule.SEND_REQUEST_DTL_AOB();
                    aobDtlReq.faxId = faxID;
                    aobDtlReq.faxDtlId = faxDtlId;
                    aobDtlReq.strFaxNo = addrInfo.strFaxNo;
                    aobDtlReq.strRecipientName = addrInfo.strRecipientName;
                    aobDtlReq.strStateEach = p_aobReqInfo.strState;
                    aobDtlReq.strTiffPath = p_aobReqInfo.strLastTifFile;

                    p_aobReqInfo.m_lstSendRequestDtlAobInfos.Add(aobDtlReq);
                }
            }
            else
            {
                faxDtlId = DbModule.Instance.InsertSendDetail(faxID, p_aobReqInfo.strRecipientFaxNo, p_aobReqInfo.strRecipientName, p_aobReqInfo);
                if (faxDtlId < 0)
                {
                    Log_Error_(RESULT.F_DB_ERROR, "발송요청상세건 정보를 INSERT하는 도중 오류가 발생하였습니다.");
                    return RESULT.F_DB_ERROR;
                }
                DbModule.SEND_REQUEST_DTL_AOB dtlSendReq = new DbModule.SEND_REQUEST_DTL_AOB();
                dtlSendReq.faxId = faxID;
                dtlSendReq.faxDtlId = faxDtlId;
                dtlSendReq.strFaxNo = p_aobReqInfo.strRecipientFaxNo;
                dtlSendReq.strRecipientName = p_aobReqInfo.strRecipientName;
                dtlSendReq.strStateEach = p_aobReqInfo.strState;
                dtlSendReq.strTiffPath = p_aobReqInfo.strLastTifFile;

                p_aobReqInfo.m_lstSendRequestDtlAobInfos.Add(dtlSendReq);
            }
            

            //// 동보정보 세팅 ////
            if (!DbModule.Instance.SetBroadcastCnt(p_aobReqInfo.faxId, ref m_lstAddressInfos))
            {
                Log_Error_(RESULT.F_DB_ERROR, "동보발송을 위한 주소록 데이터를 얻는중 오류가 발생하였습니다.");
                return RESULT.F_DB_ERROR;
            }
            
            //// FAX발송요청(싸이트정보) INSERT - BTF_FAX_SEND_SITE ////
            InsertSendSiteTable_(faxID, p_strTrNo, m_parser);
            
            //// FAX발송요청 첨부파일 INSERT - BTF_FAX_SEND_DOC ////
            //// TMK -> TPC 유형이면 TMK예약어에 FAX_ID를 붙인다 ////
            if (p_docInfos.Count > 0 && p_reqType == REQ_TYPE.TMK_TPC)
            {
                foreach (DbModule.DOC_INFO docInfo in p_docInfos)
                {
                    if (docInfo.strDocFile == "tmk")
                    {
                        docInfo.strDocFile = string.Format("{0}_tmk", faxID);
                        break;
                    }
                }
            }

            //// 첨부파일 INSERT
            if (p_docInfos.Count > 0
                && p_reqType != REQ_TYPE.FOD 
                && !DbModule.Instance.InsertSendDocs(faxID, Config.PROCESS_TYPE, p_strDocPath, p_docInfos))
            {
                Log_Error_(RESULT.F_DB_ERROR, "변환문서 정보를 DB에 INSERT하는 도중 오류가 발생하였습니다");
                return RESULT.F_DB_ERROR;
            }
            
            //// FAX발송요청 전문데이터 INSERT - BTF_FAX_SEND_DATA ////
            if (!DbModule.Instance.InsertSendData(faxID, p_strPacketXml))
            {
                Log_Error_(RESULT.F_DB_ERROR, "전문데이터 XML을 DB에 INSERT하는 도중 오류가 발생하였습니다");
                return RESULT.F_DB_ERROR;
            }
            
            //// FAX발송요청 FAXBOX INSERT - BTF_FAXBOX ////
            if (Config.ENABLE_FAXBOX == "Y")
            {
                if (!DbModule.Instance.InsertFaxBox(p_aobReqInfo))
                {
                    Log_Error_(RESULT.F_DB_INSERT_FAXBOX_ERROR, "FAXBOX에 데이터를 Insert도중 오류가 발생하였습니다.");
                    return RESULT.F_DB_INSERT_FAXBOX_ERROR;
                }
            }
            
            //// FAX발송 요청건 대기상태로 전환 ////
            //// 즉시발송요청 TIF처리 ////
            if (p_reqType == REQ_TYPE.FOD)
            {
				if (p_docInfos.Count < 0)
					return RESULT.F_SYSTEMHEADER_DOCFILE_INVALIED_DOC_INFO;

				if (!CopyToFinishedDir_(p_docInfos, p_aobReqInfo))
					return RESULT.F_FILE_FAIL_TO_DOPY_FINISHED_DIR;

                if (!DbModule.Instance.UpdateAllTifName(p_aobReqInfo.faxId, p_aobReqInfo.strLastTifFile))
                    Log_Error_(RESULT.F_DB_ERROR, "전체 TIF파일명을 업데이트하는도중 오류가 발생하였습니다");

				//// 파일사이즈 최대크기 비교 ////
				if (Config.TIF_FILE_MAX_SIZE > 0)
				{	
					long nTifSize = this.GetFileSize_(p_docInfos, p_aobReqInfo);					
					if ( nTifSize > Config.TIF_FILE_MAX_SIZE)
					{
						Log_Error_(RESULT.F_FILE_FAIL_FILE_SIZE_OVERFLOW, string.Format("발송가능한 파일크기를 초과하였습니다. TIF크기/최대크기:{0}/{1}", nTifSize, Config.TIF_FILE_MAX_SIZE));

						//// FAX발송 요청건 대기상태로 전환 - DTL 전체 ////
						if (DbModule.Instance.PassoverDetailSendReq(faxID, -1, P_TYPE.AOB, Config.SYSTEM_PROCESS_ID, p_destProcessType, "Y") <= 0)
						{
							Log_Error_(RESULT.F_DB_ERROR, string.Format("FAX발송 상세건을 {0} 으로 넘기는 도중에 오류가 발생하였습니다.", p_destProcessType));
							return RESULT.F_DBP_FAIL_PASS_OVER_SEND_REQ_DTL;
						}

						//// FAX발송 요청건 대기상태로 전환 - MSTR ////
						if (DbModule.Instance.PassoverMasterSendReq(faxID, P_TYPE.AOB, Config.SYSTEM_PROCESS_ID, p_destProcessType) <= 0)
						{
							Log_Error_(RESULT.F_DB_ERROR, string.Format("FAX발송 요청건을 {0} 으로 넘기는 도중에 오류가 발생하였습니다.", p_destProcessType));
							return RESULT.F_DBP_FAIL_PASS_OVER_SEND_REQ_MSTR;
						}

						// PSC에서 최종 실패처리
						return RESULT.F_FILE_FAIL_FILE_SIZE_OVERFLOW;
					}
				}
            }

            //// FAX발송 요청건 대기상태로 전환 - DTL 전체 ////
            if (DbModule.Instance.PassoverDetailSendReq(faxID, -1, P_TYPE.AOB, Config.SYSTEM_PROCESS_ID, p_destProcessType, "Y") <= 0)
            {
                Log_Error_(RESULT.F_DB_ERROR, string.Format("FAX발송 상세건을 {0} 으로 넘기는 도중에 오류가 발생하였습니다.", p_destProcessType));
                return RESULT.F_DBP_FAIL_PASS_OVER_SEND_REQ_DTL;
            }
            
            //// FAX발송 요청건 대기상태로 전환 - MSTR ////
            if (DbModule.Instance.PassoverMasterSendReq(faxID, P_TYPE.AOB, Config.SYSTEM_PROCESS_ID, p_destProcessType) <= 0)
            {
                Log_Error_(RESULT.F_DB_ERROR, string.Format("FAX발송 요청건을 {0} 으로 넘기는 도중에 오류가 발생하였습니다.", p_destProcessType));
                return RESULT.F_DBP_FAIL_PASS_OVER_SEND_REQ_MSTR;
            }
			
            return RESULT.SUCCESS;
        }

        protected RESULT InsertSendSiteTable_(decimal p_faxID, string p_strTrNo, PacketParser p_parser)
        {
            int i, nColumnCnt;
            string strFieldPath;
            string strColumns, strValues, strValue;
            string strColumnsR;
            List<string> ValuesR = new List<string>();
            bool bReapetFirst, bRepeatFail;
            string strSql;

            if (Config.SST_MAP_header.Count <= 0 && Config.SST_MAP.Count <= 0)
                return RESULT.SUCCESS;
            
            //// 싸이트성 필드값 추출 ////
            strColumns = Config.SEND_SITE_IDFIELD;
            strValues = p_faxID.ToString();
            strColumnsR = "";
            nColumnCnt = 0;

            // 전문 헤더 필드 추출
            if (Config.SST_MAP_header.Count >= 1)
            {
                foreach (KeyValuePair<string, List<DbModule.SST_COLUMN_MAP>> pair in Config.SST_MAP_header)
                {
                    foreach (DbModule.SST_COLUMN_MAP sstColumnMap in pair.Value)
                    {
                        strFieldPath = string.Format("헤더___{0}.{1}", pair.Key, sstColumnMap.strTrField);
                        if (!p_parser.GetFieldValue(strFieldPath, out strValue))
                            break;

                        nColumnCnt++;

                        strColumns += ", " + sstColumnMap.strColumnName;
                        if (sstColumnMap.strColumnType == "S")
                            strValues += ", '" + strValue + "'";
                        else
                            strValues += ", " + strValue;
                    }
                }
            }

            // 전문 필드 추출
            bReapetFirst = true;
            bRepeatFail = false;
            if (Config.SST_MAP.Count >= 1)
            {
                foreach (DbModule.SST_COLUMN_MAP sstColumnMap in Config.SST_MAP[p_strTrNo])
                {
                    if (sstColumnMap.strTrFieldType != "R") // 일반 필드
                    {
                        strFieldPath = string.Format("전문___{0}.{1}", p_strTrNo, sstColumnMap.strTrField);

                        if (!p_parser.GetFieldValue(strFieldPath, out strValue))
                            continue;

                        nColumnCnt++;

                        strColumns += ", " + sstColumnMap.strColumnName;
                        if (sstColumnMap.strColumnType == "S")
                            strValues += ", '" + strValue + "'";
                        else
                            strValues += ", " + strValue;
                    }
                    else  // 반복 필드
                    {
                        if (bRepeatFail)
                            continue;

                        if (bReapetFirst) // 첫번째 반복 필드
                        {
                            for (i = 0; ; ++i)
                            {
                                strFieldPath = string.Format("반복부_{0}[{1}].{2}", sstColumnMap.strTrFieldExtraInfo, i, sstColumnMap.strTrField);
                                if (!p_parser.GetFieldValue(strFieldPath, out strValue))
                                    break;

                                if (sstColumnMap.strColumnType == "S")
                                    ValuesR.Add("'" + strValue + "'");
                                else
                                    ValuesR.Add(strValue);
                            }

                            if (ValuesR.Count > 0)
                            {
                                bReapetFirst = false;
                                strColumnsR = sstColumnMap.strColumnName;
                            }
                        }

                        else // 첫번째 이후 반복 필드
                        {
                            for (i = 0; i < ValuesR.Count; ++i)
                            {
                                strFieldPath = string.Format("반복부_{0}[{1}].{2}", sstColumnMap.strTrFieldExtraInfo, i, sstColumnMap.strTrField);
                                if (!p_parser.GetFieldValue(strFieldPath, out strValue))
                                    break;

                                if (sstColumnMap.strColumnType == "S")
                                    ValuesR[i] += ", '" + strValue + "'";
                                else
                                    ValuesR[i] += ", " + strValue;

                            }

                            if (i > 0)
                            {
                                if (i == ValuesR.Count)
                                    strColumnsR += ", " + sstColumnMap.strColumnName;
                                else
                                {
                                    bRepeatFail = true;
                                    ValuesR.Clear();
                                }
                            }
                        }
                    }
                }
            }
            
            //// 싸이트성 테이블에 INSERT ////
            //// Send Site 테이블 INSERT ( 헤더 필드, 일반 필드 )
            if (nColumnCnt > 0)
            {
                strSql = string.Format("INSERT INTO {0} ( {1} ) VALUES ( {2} )", Config.SEND_SITE_TABLE, strColumns, strValues);

                if (DbModule.Instance.ExecuteSQL(strSql) < 0)
                {
                    Log_Error_(RESULT.F_DB_ERROR, "싸이트성 발송요청건 정보를 INSERT하는 도중 오류가 발생하였습니다.");
                    return RESULT.F_DB_ERROR;
                }
            }

            // Send Site DTL 테이블 INSERT ( 반복 필드 )
            if (ValuesR.Count > 0)
            {
                foreach (string strValuesR in ValuesR)
                {
                    strSql = string.Format("INSERT INTO {0} ( {1}, {2}, {3} ) VALUES ( {4}.NEXTVAL, {5}, {6} )",
                                            Config.SEND_SITE_DTL_TABLE,
                                            Config.SEND_SITE_DTL_SEQFIELD,
                                            Config.SEND_SITE_IDFIELD,
                                            strColumnsR,
                                            Config.SEND_SITE_DTL_SEQ,
                                            p_faxID.ToString(),
                                            strValuesR);

                    if (DbModule.Instance.ExecuteSQL(strSql) < 0)
                        Log_Error_(RESULT.F_DB_ERROR, string.Format("싸이트성 발송요청건 DTL 정보를 INSERT하는 도중 오류가 발생하였습니다. [{0}]", strSql));
                }
            }
            
            return RESULT.SUCCESS;
        }

        private bool CopyToFinishedDir_(List<DbModule.DOC_INFO> p_docInfos, DbModule.SEND_REQUEST_AOB p_sendReq)
        {   
            string strSrc = "";
            string strDest = "";

            DbModule.DOC_INFO docInfo = p_docInfos[0];
            if (docInfo == null)
                return false;

            DbModule.SEND_REQUEST_DTL_AOB dtlReq = p_sendReq.m_lstSendRequestDtlAobInfos[0];
            if (dtlReq == null)
                return false;

            p_sendReq.strLastTifFile = dtlReq.strTiffPath;
            strSrc = String.Format("{0}\\{1}\\{2}.{3}", Config.STG_HOME_PATH, Config.INPUT_DOCS_PATH, docInfo.strDocFile, docInfo.strDocExt);
            strDest = String.Format("{0}\\{1}", Config.FINISHED_TIF_PATH, dtlReq.strTiffPath);

            try 
			{ 
				File.Copy(strSrc, strDest, true);
				Log_Success_(string.Format("src:{0} -> dest{1}", strSrc, strDest ));

				if (Config.DELETE_INPUT_FILE_YN == "Y")
				{
					if (File.Exists(strSrc))
					{
						File.Delete(strSrc);
						Log_Success_(string.Format("Delete Source file. src_file:{0}", strSrc));
					}
				}
			}
            catch 
			{ 
				return false; 
			}

            return true;
        }


		private long GetFileSize_(List<DbModule.DOC_INFO> p_docInfos, DbModule.SEND_REQUEST_AOB p_sendReq)
		{
			string strSrc = "";
			
			DbModule.DOC_INFO docInfo = p_docInfos[0];
			if (docInfo == null)
				return -1;

			DbModule.SEND_REQUEST_DTL_AOB dtlReq = p_sendReq.m_lstSendRequestDtlAobInfos[0];
			if (dtlReq == null)
				return -1;

			p_sendReq.strLastTifFile = dtlReq.strTiffPath;
			strSrc = String.Format("{0}\\{1}\\{2}.{3}", Config.STG_HOME_PATH, Config.INPUT_DOCS_PATH, docInfo.strDocFile, docInfo.strDocExt);
			
			FileInfo fInfo = null;
			try
			{
				fInfo = new FileInfo(strSrc);
				return fInfo.Length;
			}
			catch
			{
				return -1;
			}
		}
                
        private void ClearError_()
        {
            m_result = RESULT.EMPTY;
            
            m_trNo = "NotSet";
            m_indexNo = "NotSet";
            m_faxID = -1;
        }

        protected void SetTrNo_(string p_strTrNo)
        {
            m_trNo = p_strTrNo;
        }

        private void SetIndexNo(string p_strIndexNo)
        {
            m_indexNo = p_strIndexNo.Trim();
        }

        private void SetFaxID(decimal p_faxID)
        {
            m_faxID = p_faxID;
        }

        private void ClearInfos()
        {
            m_remote = null;
            m_trNo = "";
            m_result = RESULT.EMPTY;
            m_faxID = -1;
            m_indexNo = "";
            m_packetXml = "";
            m_docPath = ""; 
        }

        private void Log_Error_(RESULT p_result, string p_strErrorMsg)
        {
            m_result = p_result;
            LogWrite_(LOG_LEVEL.ERR, p_strErrorMsg);
        }

        private void Log_Error_(RESULT p_result, Exception p_ex, string p_strErrorMsg)
        {
            m_result = p_result;
            LogWrite_(LOG_LEVEL.ERR, string.Format("{0}\n{1}\n{2}\n{3}", p_strErrorMsg, p_ex.Source, p_ex.Message, p_ex.StackTrace));
        }

        private void Log_StepMsg_(string p_strMsg)
        {
            LogWrite_(LOG_LEVEL.MSG, p_strMsg);
        }

        private void Log_Success_(string p_strMsg)
        {
            m_result = RESULT.SUCCESS;

            string strMsg;
            if (m_sendRequestInfo.reserveYN == "Y")
            {
                strMsg = string.Format("{0}.  팩스번호:{1}, 수신자이름:{2}, 요청유형:{3}, 발송우선순위:{4}, 승인여부:{5}, 발송예약여부:{6}, 발송예약일시:{7}_{8}.",
                                                    p_strMsg,
                                                    m_sendRequestInfo.strRecipientFaxNo,
                                                    m_sendRequestInfo.strRecipientName,
                                                    m_sendRequestInfo.strReqType,
                                                    m_sendRequestInfo.priority,
                                                    m_sendRequestInfo.approvedReq,
                                                    m_sendRequestInfo.reserveYN,
                                                    m_sendRequestInfo.strReserveDate,
                                                    m_sendRequestInfo.strReserveTime);
            }
            else
            {
                strMsg = string.Format("{0}.  팩스번호:{1}, 수신자이름:{2}, 요청유형:{3}, 발송우선순위:{4}, 승인여부:{5}, 발송예약여부:{6}.",
                                                    p_strMsg,
                                                    m_sendRequestInfo.strRecipientFaxNo,
                                                    m_sendRequestInfo.strRecipientName,
                                                    m_sendRequestInfo.strReqType,
                                                    m_sendRequestInfo.priority,
                                                    m_sendRequestInfo.approvedReq,
                                                    m_sendRequestInfo.reserveYN);
            }
            LogWrite_(LOG_LEVEL.MSG, strMsg);
        }
        
        private void LogWrite_(LOG_LEVEL p_logLevel, string p_strMsg)
        {
            string strRemoteInfo, strLogInfo;

            if (m_remote != null)
                strRemoteInfo = string.Format("[{0}/{1}]", m_remote.Address.ToString(), m_remote.Port);
            else
                strRemoteInfo = "[Not Connected]";
            
            if (m_result != RESULT.EMPTY)
            {
                strLogInfo = string.Format("[TR={0}]  [RESULT={1}] [ID={2}] [ID_SITE={3}]",
                                        m_trNo, m_result, m_faxID, m_indexNo, p_strMsg);
            }
            else
            {
                strLogInfo = string.Format("[TR={0}]", m_trNo);
            }

            string strLogLine = string.Format("{0} {1}  {2}", strRemoteInfo, strLogInfo, p_strMsg);
            if (p_logLevel == LOG_LEVEL.TRC)
            {
                AppLog.Write(p_logLevel, p_strMsg);
            }
            else
            {
                AppLog.Write(p_logLevel, strLogLine);
            }
        }
        #endregion
        
        #region properties
        public bool Running
        {
            get { return running; }
            set { running = value; }
        }
        public Socket Socket
        {
            get { return m_sock; }
            set { m_sock = value; }
        }
        #endregion

        #region fields
        bool running            = false;

        // Socket 
        Socket m_sock           = null;
        protected byte[] m_buffer         = new byte[BUFFER_SIZE];
        byte[] m_responBuffer   = new byte[RESPONSE_BUFFER_SIZE];

        // Info to write log
        IPEndPoint m_remote     = null;
        string m_trNo           = "";
        RESULT m_result         = RESULT.EMPTY;
        decimal m_faxID         = -1;
        string m_indexNo        = "";
        string m_packetXml      = "";
        string m_docPath        = ""; 
        
        // Buisiness
        REQ_TYPE                    m_reqType               = REQ_TYPE.NONE;
        P_TYPE                      m_nextProcessType       = P_TYPE.NONE;
        DbModule.SEND_REQUEST_AOB   m_sendRequestInfo       = new DbModule.SEND_REQUEST_AOB();
        DbModule.SEND_REQ_SITE      m_sendRequestSiteInfo   = new DbModule.SEND_REQ_SITE();
        List<DbModule.AddressInfo>  m_lstAddressInfos       = new List<DbModule.AddressInfo>();
        List<DbModule.DOC_INFO>     m_docInfos              = new List<DbModule.DOC_INFO>();
        
        // Parse packets
        public PacketParser m_parser   = new PacketParser();
        
        // Socket Log
        FileLog m_socketLog     = new FileLog();

        // ConvertableDocs
        readonly string m_convertableDocs = ",PDF,PPT,PPTX,DOC,DOCX,XLS,XLSX,TXT,HWP,HTML,MHT,TIFF,TIF,JPG,JPEG,";
        #endregion
    }
}
