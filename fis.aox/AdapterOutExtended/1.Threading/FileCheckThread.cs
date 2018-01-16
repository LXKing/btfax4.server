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
    /// Name       : FileCheckThread
    /// Content    : File Check to facsimile uploaded tif file in BTFAX Storage.
    /// Writer     : 장동훈
    /// Date       : 2012.09.03
    /// </summary>
    class FileCheckThread : BtfaxThread
    {
        #region constructor && destructor
        public FileCheckThread() { }
        ~FileCheckThread() { }
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
            DbModule.SendRequestMasterInfo reqInfo_mstr = new DbModule.SendRequestMasterInfo();
            DbModule.SendRequestDetailInfo reqInfo_dtl = new DbModule.SendRequestDetailInfo();
            DbModule.SEND_REQ_SITE reqInfo_site = new DbModule.SEND_REQ_SITE();
            P_TYPE destProcessType = P_TYPE.PSC;
            FileInfo fi = null;
            string[] files = null;
            string[] checkFiles = null;
            string[] workFiles = null;
            int successCount = 0;
            string destFileName = null;     // Detail 테이블에 기록될 Unique 파일 이름

            try
            {
                /// 에러로 인해 Process -> Finish 처리가 안된 팩스들 Process 폴더 -> Upload 폴더로 이동
                CheckProcessFolder();
                
                /// Storage 경로 확인
                if (CheckStoragePath() == false) return;

                while (!BtfaxThread.Stop)
                {
                    /// 초기화
                    files = null;
                    checkFiles = null;
                    workFiles = null;
                    successCount = 1;
                    destFileName = null;

                    /// 파일 업로드 여부 확인
                    if (CheckUploadFiles(ref files, ref checkFiles, ref workFiles) == true)
                    {
                        foreach (string f in checkFiles)
                        {
                            /// 한번에 처리할 최대 개수
                            if (successCount++ > Config.FILE_PROCESS_COUNT) break;

                            /// 초기 설정
                            fi = new FileInfo(f);
                            reqInfo_mstr.Clear();
                            reqInfo_dtl.Clear();
                            reqInfo_site.clear();

                            try
                            {
                                Thread.Sleep(100);

                                /// 작업할 파일 작업폴더로 이동
                                if (FileMove(fi, FileMoveLocationEnum.Process, ref destFileName) == false) continue;

                                /// 파일 파싱
                                if (ParseTifFile(fi, reqInfo_mstr, reqInfo_dtl) == false) continue;

                                /// 파싱 성공 후 이미지 폴더로 파일 이동 (+ 이름 변경)
                                if (FileMove(fi, FileMoveLocationEnum.Done, ref destFileName) == false) continue;

                                /// DB 에 저장될 파일명 변경
                                if (FileRenameForDB(reqInfo_dtl, destFileName) == false) continue;

                                /// DB Insert
                                if (InsertToDb(reqInfo_mstr, reqInfo_dtl, reqInfo_site) != RESULT.SUCCESS) continue;

                                /// Pass Over
                                if (PassOverSendRequest(destProcessType, reqInfo_mstr, reqInfo_dtl) != RESULT.SUCCESS) continue;
                            }
                            catch (Exception ex)
                            {
                                AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다.", m_ThreadName));
                                break;
                            }
                        }
                    }

                    /// Wait - Storage polling time
                    /// Sleep 시간이 길 경우 Thread.Join() 도 길어져 Sleep 시간을 줄임
                    for (int i = 0; !BtfaxThread.Stop && (i * 100 < Config.SOTRAGE_POLLING_TIME); i++)
                    {
                        Thread.Sleep(100);
                    }
                }
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("{0} 쓰레드에서 다음과 같은 오류가 발생하였습니다. 쓰레드를 종료합니다.", m_ThreadName));
                //throw ex;
            }
        }

        /// <summary>
        /// 2012.09.04 장동훈 추가.
        /// 공유폴더 확인
        /// 공유가 안되있거나 권한이 없을 경우 false
        /// </summary>
        private bool CheckStoragePath()
        {
            if (Directory.Exists(m_UploadPath) == false)
            {
                AppLog.Write(LOG_LEVEL.ERR, string.Format("업로드 폴더({0})에 연결할 수 없습니다.", m_UploadPath));
                return false;
            }
            else if (Directory.Exists(m_ProcessPath) == false)
            {
                AppLog.Write(LOG_LEVEL.ERR, string.Format("작업 진행 폴더({0})에 연결할 수 없습니다..", m_ProcessPath));
                return false;
            }
            else if (Directory.Exists(m_ExceptionPath) == false)
            {
                AppLog.Write(LOG_LEVEL.ERR, string.Format("오류 파일 처리 폴더({0})에 연결할 수 없습니다.", m_ExceptionPath));
                return false;
            }

            return true;
        }

        /// <summary>
        /// 쓰레드 시작시 Process 폴더에 파일이 존재하면 Upload 폴더로 이동한다.
        /// </summary>
        private void CheckProcessFolder()
        {
            ///// 확인용, 작업용 확장자
            //string[] extensions = new string[] { Config.FILE_CHECK_EXT, Config.FILE_WORK_EXT }.Distinct().ToArray();
            /// Process 폴더의 모든 파일 가져오기
            string[] allFiles = Directory.GetFiles(m_ProcessPath);
            /// TIF, DAT 파일만 선택
            List<string> files = new List<string>();
            /// 파일 정보 변수
            FileInfo fi = null;

            /// 확인용 파일만 선택 (DAT)
            /// 작업용 파일만 선택 (TIF)
            files.AddRange(allFiles.Where((f) => f.Substring(f.LastIndexOf(".") + 1).ToUpper() == Config.FILE_CHECK_EXT ||
                                                 f.Substring(f.LastIndexOf(".") + 1).ToUpper() == Config.FILE_WORK_EXT));

            try
            {
                foreach (string f in files)
                {
                    fi = new FileInfo(f);
                    fi.MoveTo(string.Format("{0}{1}", m_UploadPath, fi.Name));
                }
            }
            catch (Exception ex)
            {   
                AppLog.ExceptionLog(ex, string.Format("Process({0}) 경로의 파일을 Upload({1}) 경로로 이동중 오류가 발생하였지만 계속 진행합니다.", m_ProcessPath, m_ProcessPath));
            }
        }

        private bool CheckUploadFiles(ref string[] files, ref string[] checkFiles, ref string[] workFiles)
        {
            // 모든 파일
            files = Directory.GetFiles(m_UploadPath);
            /// 확인용 파일만 선택
            checkFiles = files.Where((f) => f.Substring(f.LastIndexOf(".") + 1).ToUpper() == Config.FILE_CHECK_EXT).ToArray();
            /// 작업용 파일만 선택
            workFiles = files.Where((f) => f.Substring(f.LastIndexOf(".") + 1).ToUpper() == Config.FILE_WORK_EXT).ToArray();

            /// 파일 존재 여부 확인
            return checkFiles.Length == 0 ? false : true;
        }
        
        private bool ParseTifFile(FileInfo p_fi, DbModule.SendRequestMasterInfo p_reqInfo_mstr, DbModule.SendRequestDetailInfo p_reqInfo_dtl)
        {
            /// 파일 1개씩 파싱
            if (Config.FILE_CHECK_TYPE == FILE_CHECK_TYPE_Enum.F1)
            {
                if (ParseTifFileOnlyName(p_fi, p_reqInfo_mstr, p_reqInfo_dtl) == false) return false;
            }
            else if (Config.FILE_CHECK_TYPE == FILE_CHECK_TYPE_Enum.F2)
            {
                if (ParseTifFileWithInfoFile(p_fi, p_reqInfo_mstr, p_reqInfo_dtl) == false) return false;
            }

            return true;
        }

        private bool ParseTifFileOnlyName(FileInfo fi, DbModule.SendRequestMasterInfo p_reqInfo_mstr, DbModule.SendRequestDetailInfo p_reqInfo_dtl)
        {
            string strSID = "";
            string strFacsimileFaxNo = "";
            string strDestFaxNo = "";
            string strTitle = "";
            string strReqUserName = "";

            string onlyFileName = fi.Name.Substring(0, fi.Name.LastIndexOf(fi.Extension));

            /// 가변 길이 형식 처리
            if (Config.FILE_IsFixOrVariable == "V")
            {
                string[] s = onlyFileName.Split(Convert.ToChar(Config.FILE_DELIMITER));

                /// 파일 이름 유효성 검사
                if (s.Length == 0)
                {
                    FileMoveToException(m_ProcessPath, fi.Name);
                    return false;
                }

                /// 파일 이름과 Tif 파일 이름 포맷 유효성 검사
                if (s.Length != Config.FileTifFileFormatInfo.Count)
                {
                    FileMoveToException(m_ProcessPath, fi.Name);
                    return false;
                }

                try
                {
                    for (int i = 0; i < s.Length; i++)
                    {
                        if (Config.FileTifFileFormatInfo[i].key == "@SID") strSID = s[i].ToString().Trim();
                        else if (Config.FileTifFileFormatInfo[i].key == "@FMFN") strFacsimileFaxNo = s[i].ToString().Trim();
                        else if (Config.FileTifFileFormatInfo[i].key == "@DSFN") strDestFaxNo = s[i].ToString().Trim();
                        else if (Config.FileTifFileFormatInfo[i].key == "@TITL") strTitle = s[i].ToString().Trim();
                        else if (Config.FileTifFileFormatInfo[i].key == "@RQUN") strReqUserName = s[i].ToString().Trim();
                    }
                }
                catch (Exception ex)
                {
                    FileMoveToException(m_ProcessPath, fi.Name);
                    throw ex;
                }
            }
            /// 고정 길이 형식 처리
            else if (Config.FILE_IsFixOrVariable == "F")
            {
                /// 글자수 확인
                if (fi.Name.Length != Config.FILE_TIF_FILE_REAL_FORMAT.Length)
                {
                    FileMoveToException(m_ProcessPath, fi.Name);
                    return false;
                }

                try
                {
                    foreach (FileCheckInfo info in Config.FileTifFileFormatInfo)
                    {
                        string item = onlyFileName.Substring(info.startPosition, info.length).Trim();

                        if (info.key == "@SID") strSID = item;
                        else if (info.key == "@FMFN") strFacsimileFaxNo = item;
                        else if (info.key == "@DSFN") strDestFaxNo = item;
                        else if (info.key == "@TITL") strTitle = item;
                        else if (info.key == "@RQUN") strReqUserName = item;
                    }
                }
                catch (Exception ex)
                {
                    FileMoveToException(m_ProcessPath, fi.Name);
                    throw ex;
                }
            }

            /// 분석된 항목 설정
            p_reqInfo_mstr.DataSetting(strSID, strFacsimileFaxNo, strReqUserName, strTitle);
            //p_reqInfo_dtl.DataSetting(strDestFaxNo, strTitle, string.Format("{0}{1}.{2}", m_RelativePath, onlyFileName, Config.FILE_IMAGE_FILE_EXT));
            p_reqInfo_dtl.DataSetting(strDestFaxNo, strTitle, m_RelativePath);

            return true;
        }

        private bool ParseTifFileWithInfoFile(FileInfo fi, DbModule.SendRequestMasterInfo p_reqInfo_mstr, DbModule.SendRequestDetailInfo p_reqInfo_dtl)
        {
            string strSID = "";
            string strFacsimileFaxNo = "";
            string strDestFaxNo = "";
            string strTitle = "";
            string strReqUserName = "";
            string content = "";

            string onlyFileName = fi.Name.Substring(0, fi.Name.LastIndexOf(fi.Extension));
            string datFile = string.Format("{0}{1}", m_ProcessPath, fi.Name);

            /// 포맷 파일 읽기
            try
            {
                content = File.ReadAllText(datFile, Encoding.Default);
            }
            catch (Exception ex)
            {
                /// 에러 파일 -> Exception 폴더로 이동
                FileMoveToException(fi.DirectoryName, fi.Name);

                AppLog.ExceptionLog(ex, string.Format("{0}을 읽는 중 오류가 발생하였습니다.", fi.Name));
                throw ex;
            }

            /// 포맷 파일 분석
            Regex r = new Regex(@"(?<key>[\w]+)[%s]*=[%s]*(?<value>[\w\d]+)");
            MatchCollection matches = r.Matches(content);

            string key = "";
            string value = "";

            foreach (Match m in matches)
            {
                key = m.Groups["key"].Value;
                value = m.Groups["value"].Value;

                if (key == "DEVICE_ID") strSID = value;
                else if (key == "FACSIMILE_FAXNO") strFacsimileFaxNo = value;
                else if (key == "DEST_FAXNO") strDestFaxNo = value;
                else if (key == "TITLE") strTitle = value;
                else if (key == "REQ_USER_NAME") strReqUserName = value;
            }

            /// 분석된 항목 유효성 검사
            /// 최소 보낼 팩스 번호는 있어야 함.
            if (string.IsNullOrEmpty(strDestFaxNo) == true)
            {
                /// 에러 파일 -> Exception 폴더로 이동
                FileMoveToException(fi.DirectoryName, fi.Name);

                return false;
            }

            /// 분석된 항목 설정
            p_reqInfo_mstr.DataSetting(strSID, strFacsimileFaxNo, strReqUserName, strTitle);
            //p_reqInfo_dtl.DataSetting(strDestFaxNo, strTitle, string.Format("{0}{1}.{2}", m_RelativePath, onlyFileName, Config.FILE_IMAGE_FILE_EXT));
            p_reqInfo_dtl.DataSetting(strDestFaxNo, strTitle, m_RelativePath);

            return true;
        }

        private bool FileMove(FileInfo fi, FileMoveLocationEnum location, ref string destFilename)
        {
            int resultCount = 0;

            /// 확인용, 작업용 확장자
            string[] extensions = new string[] { Config.FILE_CHECK_EXT, Config.FILE_WORK_EXT }.Distinct().ToArray();

            string onlyFileName = fi.Name.Substring(0, fi.Name.LastIndexOf(fi.Extension));
            string onlyFileNameAndExt_forSource = "";
            string onlyFileNameAndExt_forDest = "";
            string sourcePath = "";
            string destPath = "";
            string sourceFile = "";
            string destFile = "";
            string appendFilenameForUnique = "";

            if (location == FileMoveLocationEnum.Process)
            {
                sourcePath = m_UploadPath;
                destPath = m_ProcessPath;
            }
            else if (location == FileMoveLocationEnum.Done)
            {
                //// TIF 가 저장된 오늘일자 경로
                string strTifFullPath = string.Format(@"{0}\{1}", Config.FINISHED_TIF_PATH, m_RelativePath);

                //// 최종 TIFF 일자별 디렉토리 생성 ////
                if (!Directory.Exists(strTifFullPath)) Directory.CreateDirectory(strTifFullPath);

                sourcePath = m_ProcessPath;
                destPath = strTifFullPath;

                appendFilenameForUnique = DateTime.Now.ToString("_yyyyMMddHHmmss.ffffff");
            }
            else if (location == FileMoveLocationEnum.Exception)
            {
                sourcePath = m_UploadPath;
                destPath = m_ProcessPath;
            }


            try
            {
                foreach (string ext in extensions)
                {
                    onlyFileNameAndExt_forSource = string.Format("{0}.{1}", onlyFileName, ext);
                    sourceFile = string.Format("{0}{1}", sourcePath, onlyFileNameAndExt_forSource);

                    /// FInish 폴더로 이동시에는 unique 하기 위해 파일 이름을 변경
                    onlyFileNameAndExt_forDest = string.Format("{0}{1}.{2}", onlyFileName, appendFilenameForUnique, ext);
                    destFile = string.Format("{0}{1}", destPath, onlyFileNameAndExt_forDest);

                    /// DB 이름을 변경할 변수
                    if (ext == Config.FILE_IMAGE_FILE_EXT)
                    {
                        destFilename = onlyFileNameAndExt_forDest;
                    }

                    /// 파일 이동
                    File.Move(sourceFile, destFile);

                    resultCount++;
                }
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, "파일 이동중 오류가 발생하였습니다.");

                /// Process -> Finish 일때만 Exception 으로 진행
                if (location == FileMoveLocationEnum.Done)
                {
                    FileMoveToException(m_ProcessPath, onlyFileNameAndExt_forSource);
                }
                //throw ex;
            }

            if (Config.FILE_CHECK_TYPE == FILE_CHECK_TYPE_Enum.F2)
            {
                return resultCount > 1 ? true : false;
            }
            else
            {
                return resultCount > 0 ? true : false;
            }
            //return Config.FILE_CHECK_TYPE == FILE_CHECK_TYPE_Enum.F1 && resultCount > 0 ? true : false;
        }

        /// <summary>
        /// Exception 폴더로 이동
        /// </summary>
        /// <param name="sourcePath">소스 경로</param>
        /// <param name="sourceOnlyFileAndExt">파일이름.확장자</param>
        /// <param name="moveOnlySelectFile">지정된 파일만 Exception 폴더로 이동할지 여부</param>
        private void FileMoveToException(string sourcePath, string sourceOnlyFileAndExt)
        {
            /// 확인용, 작업용 확장자
            string[] extensions = new string[] { Config.FILE_CHECK_EXT, Config.FILE_WORK_EXT }.Distinct().ToArray();

            string sourceFile = "";
            string destFile = "";
            string onlyFileName = sourceOnlyFileAndExt.Substring(0, sourceOnlyFileAndExt.LastIndexOf("."));
            string onlyFileNameAndExt = "";

            /// 에러 파일 -> Exception 폴더로 이동
            foreach (string ext in extensions)
            {
                try
                {
                    onlyFileNameAndExt = string.Format("{0}.{1}", onlyFileName, ext);
                    sourceFile = string.Format("{0}{1}", sourcePath, onlyFileNameAndExt);
                    destFile = string.Format("{0}{1}", m_ExceptionPath, onlyFileNameAndExt);

                    File.Move(sourceFile, destFile);
                }
                catch { }
            }
        }

        private bool FileRenameForDB(DbModule.SendRequestDetailInfo p_reqInfo_dtl, string destFilename)
        {
            /// TIF 가 저장된 오늘일자 경로
            string strTifFullPath = string.Format(@"{0}\{1}", Config.FINISHED_TIF_PATH, m_RelativePath);

            p_reqInfo_dtl.TIF_FILE += destFilename;

            return true;
        }

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
                return RESULT.F_DB_ERROR;
            }
            p_reqInfo_mstr.OUT_FAX_ID = faxID;
            p_reqInfo_dtl.FAX_ID = faxID;

            /// INSERT - BTF_FAX_SEND_DTL
            decimal faxDtlId = DbModule.Instance.AOX_InsertSendDetail(p_reqInfo_dtl);
            if (faxID < 0)
            {
                return RESULT.F_DB_ERROR;
            }
            p_reqInfo_dtl.OUT_SEQ = faxDtlId;

            /// INSERT - BTF_FAXBOX
            bool bResult = DbModule.Instance.AOX_InsertFAXBOX(p_reqInfo_mstr);
            if ( !bResult )
            {             
                return RESULT.F_DB_ERROR;
            }
            
            #region 사이트별 커스터마이징
            /// FAX발송요청 사이트 정보 INSERT - BTF_FAX_SEND_SITE
            #endregion

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
                return RESULT.F_DB_ERROR;
            }
            // FAX 발송 요청 건 대기상태로 전환 - MSTR ////
            if (DbModule.Instance.PassoverMasterSendReq(p_masterInfo.OUT_FAX_ID, 
                                                        Config.PROCESS_TYPE,
                                                        Config.SYSTEM_PROCESS_ID, p_destProcessType) <= 0)
            { 
                return RESULT.F_DB_ERROR;
            }
            return RESULT.SUCCESS;
        }
        #endregion

        #region properties
        static public FileCheckThread Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new FileCheckThread();
                return s_instance;
            }
        }
        static FileCheckThread s_instance = null;

        /// TIF 가 저장된 오늘일자 경로
        private string m_RelativePath { get { return string.Format(@"{0}\{1}\", DateTime.Now.ToString("yyyy_MM"), DateTime.Now.ToString("dd")); } }

        /// <summary>
        ///  현재 쓰레드 이름
        /// </summary>
        private string m_ThreadName   { get { return "Storage Check"; } }
        public string ThreadName { get { return m_ThreadName; } }
        #endregion

        #region enum
        private enum FileMoveLocationEnum
        {
            Process,
            Exception,
            Done
        }
        #endregion

        #region fields
        private string m_UploadPath = string.Format("{0}{1}", Config.FTP_HOME_PATH, Config.FILE_UPLOAD_PATH);
        private string m_ProcessPath = string.Format("{0}{1}{2}", Config.FTP_HOME_PATH, Config.FILE_UPLOAD_PATH, Config.FILE_PROCESS_PATH);
        private string m_ExceptionPath = string.Format("{0}{1}{2}", Config.FTP_HOME_PATH, Config.FILE_UPLOAD_PATH, Config.FILE_EXCEPTION_PATH);
        #endregion
    }
}
