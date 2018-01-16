using System;
using System.IO;
using System.Linq;
using System.Xml.Linq;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using Btfax.CommonLib;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;

namespace AdapterOutExtended
{
    /// <summary>
    /// Name       : EmailCheckThread
    /// Content    : Email Check.
    /// Writer     : 장동훈
    /// Date       : 2012.09.03
    /// </summary>
    class EMailCheckThread : BtfaxThread
    {
        #region constructor && destructor
        public EMailCheckThread() { }
        ~EMailCheckThread() { }
        #endregion

        #region IMPLEMENT
        /// <summary>
        /// Name       : ThreadEntry
        /// Parameters : Empty
        /// Content    : 쓰레드 진입점
        /// Return     : Empty
        /// Writer     : 장동훈
        /// Date       : 2012.09.03
        /// </summary>
        protected override void ThreadEntry()
        {
            AppLog.Write(LOG_LEVEL.MSG, string.Format("### {0} 쓰레드 시작 ###", m_ThreadName));
            PollingLoop_();
            AppLog.Write(LOG_LEVEL.MSG, string.Format("### {0} 쓰레드 종료 ###", m_ThreadName));
        }

        /// <summary>
        /// Name       : PollingLoop_
        /// Parameters : Empty
        /// Content    : 폴링 루프
        /// Return     : Empty
        /// Writer     : 장동훈
        /// Date       : 2012.09.03
        /// </summary>
        private void PollingLoop_()
        {
            POP3_Client mail = new POP3_Client();
            DbModule.SendRequestMasterInfo reqInfo_mstr = null;
            DbModule.SendRequestDetailInfo reqInfo_dtl = null;
            DbModule.SEND_REQ_SITE reqInfo_site = null;
            List<DbModule.SendRequestDocInfo> reqInfo_doc_List = null;
            P_TYPE destProcessType = P_TYPE.PSC;
            int retryConnectCount = 0;      // POP3 접속 실패시 3회 재시도
            int retryLoginCount = 0;        // POP3 로그인 실패시 3회 재시도

            /// 마지막 실행시간 (Sleep 실행의 기준시간)
            /// 첫 Sleep 실행시 바로 실행 되도록 시간 수정
            DateTime lastExecuteTime = DateTime.Now.AddMilliseconds(Config.EMAIL_POLLING_TIME * -1);
            
            try
            {
                while (!BtfaxThread.Stop)
                {
                    if (DateTime.Now.Subtract(lastExecuteTime).TotalMilliseconds < Config.EMAIL_POLLING_TIME)
                    {
                        Thread.Sleep(1000);
                        continue;
                    }
                    lastExecuteTime = DateTime.Now;

                    try
                    {   
                        if (mail.IsConnected == false &&
                            !mail.Connect(Config.EMAIL_POP_SERVER, Config.EMAIL_POP3_SERVER_PORT))
                        {
                            if (retryConnectCount++ < 3)
                            {
                                AppLog.Write(LOG_LEVEL.ERR, string.Format("IP: {0}, PORT: {1} 로 접속중 오류가 발생하였습니다. ({2} 회 접속 시도)", Config.EMAIL_POP_SERVER, Config.EMAIL_POP3_SERVER_PORT, retryConnectCount));

                                /// 0.5 초 대기
                                for (int i = 0; i < 5; i++) Thread.Sleep(100);

                                mail = new POP3_Client();
                                continue;
                            }
                            else
                            {
                                AppLog.Write(LOG_LEVEL.ERR, "해당 서버로 접속 연결이 되지 않습니다.");
                                throw new Exception("접속 에러");
                            }
                        }
                        if (mail.IsLogin == false &&
                            !mail.Login(Config.EMAIL_ID, Config.EMAIL_PW))
                        {
                            if (retryLoginCount++ < 3)
                            {
                                AppLog.Write(LOG_LEVEL.ERR, string.Format("ID: {0}, PASSWORD: {1} 로 로그인중 오류가 발생하였습니다. ({2} 회 로그인 시도)", Config.EMAIL_ID, string.Empty.PadRight(Config.EMAIL_PW.Length, '*'), retryLoginCount));

                                /// 0.5 초 대기
                                for (int i = 0; i < 5; i++) Thread.Sleep(100);

                                continue;
                            }
                            else
                            {
                                AppLog.Write(LOG_LEVEL.ERR, "해당 서버로 로그인이 되지 않습니다.");
                                throw new Exception("로그인 에러");
                            }
                        }

                        /// 초기화
                        retryConnectCount = 0;
                        retryLoginCount = 0;

                        int messageCount = mail.GetMailCount();
                        for (int i = 1; i <= messageCount; i++)
                        {
                            POP3_Parsing parseMail = mail.GetMessage(i);
                            
                            if (parseMail.IsParseSuccess == false) continue;

                            if (ParseEmailDocument(parseMail,
                                                   ref reqInfo_mstr,
                                                   ref reqInfo_dtl,
                                                   ref reqInfo_site,
                                                   ref reqInfo_doc_List,
                                                   ref destProcessType) != RESULT.SUCCESS) continue;

                            /// Master, Detail DB Insert
                            if (InsertToDb(reqInfo_mstr, reqInfo_dtl, reqInfo_site) != RESULT.SUCCESS) continue;

                            /// Doc DB Insert
                            if (InsertToDbDoc(reqInfo_mstr.OUT_FAX_ID, reqInfo_doc_List) != RESULT.SUCCESS) continue;

                            // FAXBOX Insert - 20130124
                            if (InsertToDbFaxbox(reqInfo_mstr) != RESULT.SUCCESS) continue;

                            /// Pass Over
                            if (PassOverSendRequest(destProcessType, reqInfo_mstr, reqInfo_dtl) != RESULT.SUCCESS) continue;

                            break;
                        }

                        /// 메일함 Update
                        mail.Disconnect();

                        lastExecuteTime = DateTime.Now;
                    }
                    catch (Exception ex)
                    {
                        lastExecuteTime = DateTime.Now;

                        AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다.-1", m_ThreadName));
                        if (mail.IsConnected == true) mail.ResetDeleteMessage();
                        AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다.-2", m_ThreadName));
                        //throw ex;
                    }
                }   // while
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다. 쓰레드를 종료합니다.-1", m_ThreadName));
                if (mail.IsConnected == true) mail.ResetDeleteMessage();
                AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다. 쓰레드를 종료합니다.-2", m_ThreadName));
                throw ex;
            }
        }

        private RESULT ParseEmailDocument(POP3_Parsing p_parseMail,
                                          ref DbModule.SendRequestMasterInfo p_reqInfo_mstr,
                                          ref DbModule.SendRequestDetailInfo p_reqInfo_dtl,
                                          ref DbModule.SEND_REQ_SITE p_reqInfo_site,
                                          ref List<DbModule.SendRequestDocInfo> p_reqInfo_doc_List,
                                          ref P_TYPE p_destProcessType)
        {
            p_reqInfo_mstr = new DbModule.SendRequestMasterInfo(m_ThreadName);
            p_reqInfo_dtl = new DbModule.SendRequestDetailInfo();
            p_reqInfo_site = new DbModule.SEND_REQ_SITE();
            p_reqInfo_doc_List = new List<DbModule.SendRequestDocInfo>();

            try
            {
                /// 첨부파일 Boundary 추출
                List<POP3_BoundaryInfo> attachFileList = GetAttachFileList(p_parseMail.BoundaryInfoList);

                if (attachFileList.Count == 0) return RESULT.F_FILE_CNT_ZERO;

                /// 메일 제목 분석
                if (ParseEmailTitle(p_parseMail.Subject, ref p_reqInfo_mstr, ref p_reqInfo_dtl) == false)
                {
                    AppLog.Write(LOG_LEVEL.ERR, string.Format("Email 제목이 형식에 맞지 않습니다. ({0} / {1})", Config.EMAIL_TITLE_FORMAT, p_parseMail.Subject));
                    //continue;
                    return RESULT.F_PARSE_ERROR;
                }

                /// REQ_TYPE, Dest Process Type 결정
                SetReqType(attachFileList, ref p_reqInfo_mstr, ref p_reqInfo_dtl, ref p_destProcessType);

                /// PSC 는 tif 1개로 DCV, TPC 필요없음.
                if (p_destProcessType == P_TYPE.PSC)
                {
                    POP3_BoundaryInfo bi = attachFileList[0];

                    /// Email AttachFile -> Local File
                    string filePath = p_parseMail.ConvertBase64ToFile(m_DownloadEmailAttachFileLocalPath, bi);
                    if (filePath.Length == 0) return RESULT.F_FILE_NOT_EXIST;

                    /// File name is must unique. (File Rename)
                    FileInfo fi = new FileInfo(filePath);
                    do
                    {
                        fi.MoveTo(string.Format(@"{0}\{1}_{2}.{3}", fi.Directory, bi.RepresentationFilenameOnly, DateTime.Now.ToString("yyyyMMddHHmmss.fffffff"), bi.RepresentationFilenameExtension));
                    //} while (File.Exists(Path.Combine(m_AoxInputDocPath, fi.Name)));
                    } while (File.Exists(Path.Combine(Config.FINISHED_TIF_PATH, m_RelativePath, fi.Name)));
                    /// Local -> Destination
                    //fi.MoveTo(Path.Combine(m_AoxInputDocPath, fi.Name));
                    fi.MoveTo(Path.Combine(Config.FINISHED_TIF_PATH, m_RelativePath, fi.Name));

                    //p_reqInfo_dtl.TIF_FILE = Path.Combine(m_RelativePath, attachFileList[0].RepresentationFilename);
                    p_reqInfo_dtl.TIF_FILE = Path.Combine(m_RelativePath, fi.Name);
                }
                else
                {
                    /// 첨부파일 AOX_INPUT_DOC_PATH 로 이동
                    foreach (POP3_BoundaryInfo bi in attachFileList)
                    {
                        /// Email AttachFile -> Local File
                        string filePath = p_parseMail.ConvertBase64ToFile(m_DownloadEmailAttachFileLocalPath, bi);
                        if (filePath.Length == 0) continue;

                        /// File name is must unique.
                        FileInfo fi = new FileInfo(filePath);
                        do
                        {
                            fi.MoveTo(string.Format(@"{0}\{1}_{2}.{3}", fi.Directory, bi.RepresentationFilenameOnly, DateTime.Now.ToString("yyyyMMddHHmmss.fffffff"), bi.RepresentationFilenameExtension));
                        } while (File.Exists(Path.Combine(m_AoxInputDocPath, fi.Name)));
                        /// Local -> Destination
                        fi.MoveTo(Path.Combine(m_AoxInputDocPath, fi.Name));

                        /// DOC info
                        DbModule.SendRequestDocInfo doc = new DbModule.SendRequestDocInfo();
                        doc.DOC_PATH = Config.AOX_INPUT_DOCS_PATH;
                        doc.strDocFile = fi.Name.Substring(0, fi.Name.LastIndexOf("."));
                        doc.strDocExt = fi.Name.Substring(fi.Name.LastIndexOf(".") + 1);

                        p_reqInfo_doc_List.Add(doc);
                    }
                }

                AppLog.Write(LOG_LEVEL.MSG, string.Format("[ Subject: {0} ] 메일을 분석 완료하였습니다.", p_parseMail.Subject));
                return RESULT.SUCCESS;

            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, "");

                return RESULT.F_PARSE_ERROR;
            }
        }
        
        private List<POP3_BoundaryInfo> GetAttachFileList(List<POP3_BoundaryInfo> biList)
        {
            return biList.Where((bi) => bi.IsFileAttach == true).ToList();
        }

        private void SetReqType(List<POP3_BoundaryInfo> biList,
                                ref DbModule.SendRequestMasterInfo p_reqInfo_mstr,
                                ref DbModule.SendRequestDetailInfo p_reqInfo_dtl,
                                ref P_TYPE destProcessType)
        {
            if (biList.Count == 1)
            {
                p_reqInfo_mstr.REQ_TYPE = "02";
                destProcessType = P_TYPE.DCV;
            }
            else if (biList.Count > 1)
            {
                p_reqInfo_mstr.REQ_TYPE = "12";
                destProcessType = P_TYPE.DCV;
            }
        }

        private bool ParseEmailTitle(string emailTitle, ref DbModule.SendRequestMasterInfo p_reqInfo_mstr, ref DbModule.SendRequestDetailInfo p_reqInfo_dtl)
        {
            string strSID = "";
            string strFacsimileFaxNo = "";
            string strDestFaxNo = "";
            string strTitle = "";
            string strReqUserName = "";

            /// 가변 길이 형식 처리
            if (Config.EMAIL_IsFixOrVariable == "V")
            {
                string[] s = emailTitle.Split(Convert.ToChar(Config.FILE_DELIMITER));

                /// 파일 이름 유효성 검사
                if (s.Length == 0) return false;

                /// 파일 이름과 Tif 파일 이름 포맷 유효성 검사
                if (s.Length != Config.EmailTitleFormatInfo.Count) return false;

                try
                {
                    for (int i = 0; i < s.Length; i++)
                    {
                        if (Config.EmailTitleFormatInfo[i].key == "@SID") strSID = s[i].ToString().Trim();
                        else if (Config.EmailTitleFormatInfo[i].key == "@FMFN") strFacsimileFaxNo = s[i].ToString().Trim();
                        else if (Config.EmailTitleFormatInfo[i].key == "@DSFN") strDestFaxNo = s[i].ToString().Trim();
                        else if (Config.EmailTitleFormatInfo[i].key == "@TITL") strTitle = s[i].ToString().Trim();
                        else if (Config.EmailTitleFormatInfo[i].key == "@RQUN") strReqUserName = s[i].ToString().Trim();
                    }
                }
                catch (Exception ex)
                {
                    AppLog.ExceptionLog(ex, "");
                    return false;
                }
            }
            /// 고정 길이 형식 처리
            else if (Config.EMAIL_IsFixOrVariable == "F")
            {
                /// 글자수 확인
                if (emailTitle.Length != Config.EMAIL_TITLE_REAL_FORMAT.Length) return false;

                try
                {
                    foreach (FileCheckInfo info in Config.EmailTitleFormatInfo)
                    {
                        string item = emailTitle.Substring(info.startPosition, info.length).Trim();

                        if (info.key == "@SID") strSID = item;
                        else if (info.key == "@FMFN") strFacsimileFaxNo = item;
                        else if (info.key == "@DSFN") strDestFaxNo = item;
                        else if (info.key == "@TITL") strTitle = item;
                        else if (info.key == "@RQUN") strReqUserName = item;
                    }
                }
                catch (Exception ex)
                {
                    AppLog.ExceptionLog(ex, "");
                    return false;
                }
            }

            /// 분석된 항목 설정
            p_reqInfo_mstr.DataSetting(strSID, strFacsimileFaxNo, strReqUserName, strTitle);
            //p_reqInfo_dtl.DataSetting(strDestFaxNo, strTitle, attachFile.Length > 0 ? string.Format("{0}{1}", m_RelativePath, attachFile) : "");
            p_reqInfo_dtl.DataSetting(strDestFaxNo, strTitle, p_reqInfo_dtl.TIF_FILE);

            return true;
        }
        #region 디비 작업
        private RESULT InsertToDb(DbModule.SendRequestMasterInfo p_reqInfo_mstr,
                                  DbModule.SendRequestDetailInfo p_reqInfo_dtl,
                                  DbModule.SEND_REQ_SITE p_reqInfo_site)
        {
            p_reqInfo_mstr.REQ_DATE = DateTime.Now;
            p_reqInfo_dtl.DATE_TO_SEND = DateTime.Now;

            /// INSERT - BTF_FAX_SEND_MSTR
            decimal faxID = DbModule.Instance.AOX_InsertSendMaster(p_reqInfo_mstr);
            if (faxID < 0)
            {
                AppLog.Write(LOG_LEVEL.ERR, string.Format("Send Master 테이블에 입력 실패하였습니다.     REQ_USER_ID: {0}", p_reqInfo_mstr.REQ_USER_ID));
                return RESULT.F_DB_ERROR;
            }
            p_reqInfo_mstr.OUT_FAX_ID = faxID;
            p_reqInfo_dtl.FAX_ID = faxID;

            /// INSERT - BTF_FAX_SEND_DTL
            decimal faxDtlId = DbModule.Instance.AOX_InsertSendDetail(p_reqInfo_dtl);
            if (faxDtlId < 0)
            {
                AppLog.Write(LOG_LEVEL.ERR, string.Format("Send Detail 테이블에 입력 실패하였습니다.     FAX_ID: {0}", faxID));
                return RESULT.F_DB_ERROR;
            }
            p_reqInfo_dtl.OUT_SEQ = faxDtlId;

            #region 사이트별 커스터마이징
            /// FAX발송요청 사이트 정보 INSERT - BTF_FAX_SEND_SITE
            #endregion

            return RESULT.SUCCESS;
        }

        private RESULT InsertToDbDoc(decimal p_FAX_ID, List<DbModule.SendRequestDocInfo> p_reqInfo_doc)
        {
            foreach (DbModule.SendRequestDocInfo doc in p_reqInfo_doc)
            {
                if (DbModule.Instance.AOX_InsertSendDoc(p_FAX_ID, doc) == false)
                {
                    AppLog.Write(LOG_LEVEL.ERR, "변환문서 정보를 DB에 INSERT하는 도중 오류가 발생하였습니다");
                    return RESULT.F_DB_ERROR;
                }
            }
            return RESULT.SUCCESS;
        }


        private RESULT InsertToDbFaxbox( DbModule.SendRequestMasterInfo p_reqInfo_mstr )
        {
            /// INSERT - BTF_FAXBOX
            bool bResult = DbModule.Instance.AOX_InsertFAXBOX(p_reqInfo_mstr);
            if (!bResult)
            {
                return RESULT.F_DB_ERROR;
            }

            return RESULT.SUCCESS;
        }
        private RESULT PassOverSendRequest(P_TYPE p_destProcessType,
                                           DbModule.SendRequestMasterInfo p_masterInfo,
                                           DbModule.SendRequestDetailInfo p_detailInfo)
        {
            // FAX 발송 요청 건 대기상태로 전환 - DTL ////
            if (DbModule.Instance.PassoverDetailSendReq(p_detailInfo.FAX_ID,
                                                        p_detailInfo.OUT_SEQ,
                                                        Config.PROCESS_TYPE,
                                                        Config.SYSTEM_PROCESS_ID,
                                                        p_destProcessType,
                                                        "Y") <= 0)
            {
                AppLog.Write(LOG_LEVEL.ERR, string.Format("Send Detail 테이블의 Pass Over가 실패하였습니다.     FAX_ID: {0}, FAX_DTL_ID: {1}", p_detailInfo.FAX_ID, p_detailInfo.OUT_SEQ));
                return RESULT.F_DB_ERROR;
            }
            // FAX 발송 요청 건 대기상태로 전환 - MSTR ////
            if (DbModule.Instance.PassoverMasterSendReq(p_masterInfo.OUT_FAX_ID,
                                                        Config.PROCESS_TYPE,
                                                        Config.SYSTEM_PROCESS_ID,
                                                        p_destProcessType) <= 0)
            {
                AppLog.Write(LOG_LEVEL.ERR, string.Format("Send Master 테이블의 Pass Over가 실패하였습니다.     FAX_ID: {0}", p_masterInfo.OUT_FAX_ID));
                return RESULT.F_DB_ERROR;
            }
            return RESULT.SUCCESS;
        }
        #endregion
        #endregion

        #region properties
        static public EMailCheckThread Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new EMailCheckThread();
                return s_instance;
            }
        }
        static EMailCheckThread s_instance = null;

        /// TIF 가 저장된 오늘일자 경로
        private string m_RelativePath { get { return string.Format(@"{0}\{1}\", DateTime.Now.ToString("yyyy_MM"), DateTime.Now.ToString("dd")); } }

        /// 변환될 TIF, DOCX, XLSX 등의 파일이 저장될 경로
        private string m_AoxInputDocPath { get { return Path.Combine(Config.FTP_HOME_PATH, Config.INPUT_DOCS_PATH, Config.AOX_INPUT_DOCS_PATH); } }

        /// 첨부파일 다운로드 로컬 경로
        private string m_DownloadEmailAttachFileLocalPath = @"Email\";

        /// <summary>
        ///  현재 쓰레드 이름
        /// </summary>
        private string m_ThreadName { get { return "E-Mail Check"; } }
        public string ThreadName { get { return m_ThreadName; } }
        #endregion

        #region enum
        #endregion

        #region fields
        #endregion
    }
}
