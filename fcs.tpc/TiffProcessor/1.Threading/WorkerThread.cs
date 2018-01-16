using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.IO;
using Btfax.CommonLib;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Util;
using TiffProcessor.Util;
using TiffProcessor.Db;
using TiffProcessor.TIFProcessing;


namespace TiffProcessor.Threading
{   
    class WorkerThread : BtfaxThread
    {
        #region static
        static WorkerThread s_instance = null;
        static public WorkerThread Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new WorkerThread();
                return s_instance;
            }
        }
        #endregion

        #region constructor
        public WorkerThread() { }
        #endregion

        #region override
        public override bool StartThread()
        {
            return base.StartThread();
        }

        public override bool JoinThread(int p_nTimeout = -1)
        {
            return base.JoinThread(p_nTimeout);
        }

        protected override void ThreadEntry()
        {
            AppLog.Write(LOG_LEVEL.MSG, "### DB폴링 쓰레드 시작 ###");
            Thread.Sleep(Config.INITIAL_SLEEP);
            AppLog.Write(LOG_LEVEL.MSG, "### DB폴링 시작 ###");

            ReadySendReq();

            while (!BtfaxThread.Stop)
            {
                Thread.Sleep(Config.DB_POLLING_SLEEP);
                ClearReqInfo();

                if (!ExistsFaxStorage())
                {
                    AppLog.Write(LOG_LEVEL.ERR, string.Format("팩스스토리지를 찾을수 없습니다. 위치:{0}", Config.STG_HOME_PATH));
                    continue;
                }

                //// TIFF 처리 요청 건 조회 ////
                RESULT result = DbModule.Instance.FetchSendReqToProcessTiff(ref m_lstSendReqTpcInfos);
                switch (result)
                {   
                    case RESULT.EMPTY: 
                    case RESULT.SUCCESS: 
                        break;

                    // 오류
                    default:
                        AppLog.Write(LOG_LEVEL.ERR, "DB 폴링 중 오류가 발생하였습니다");
                        continue;
                }

                LogMessage("DbPolling Check..");

                if (m_lstSendReqTpcInfos.Count <= 0)
                    continue;

                foreach (DbModule.SEND_REQUEST_TPC tpcReqInfo in m_lstSendReqTpcInfos)
                {   
                    //// 병합처리방식 SET ////
                    bool bIsOnce = false;
                    switch (Convert.ToInt32(tpcReqInfo.strReqType))
                    {   
                        case (int)REQ_TYPE.DCV:
                        case (int)REQ_TYPE.TPC:
                        case (int)REQ_TYPE.DCV_TPC:
                            bIsOnce = true;
                            break;

                        case (int)REQ_TYPE.TMK_TPC:
                        case (int)REQ_TYPE.TMK_DCV_TPC:
                            bIsOnce = false;
                            break;

                        default:
                            break;
                    }

                    m_lstSuccSendReqDtlTpcInfos.Clear();
                    string strLogExt;
                    foreach (DbModule.SEND_REQUEST_DTL_TPC tpcDtlReqInfo in tpcReqInfo.m_lstSendRequestDtlTpcInfos)
                    {
                        strLogExt = string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}][REQ_TYPE:{2}][FAX_NO:{3}]", 
                                                tpcReqInfo.faxId
                                                , tpcDtlReqInfo.faxDtlId
                                                , tpcReqInfo.strReqType
                                                , tpcDtlReqInfo.strFaxNo);

                        LogMessage(string.Format("TIF병합건 점유 {0}", strLogExt));


                        //// TIFF 병합 작업 ////
                        result = ProcessingTifs_(tpcReqInfo, tpcDtlReqInfo, tpcReqInfo.m_lstDocInfoAllTpc);
                        if (result != RESULT.SUCCESS)
                        {
                            LogError(string.Format("TIF병합 작업실패 {0}", strLogExt), result);

                            //// 실패 결과 DB 반영 ////
                            if (DbModule.Instance.FailSendReqDetail(tpcReqInfo.faxId, tpcDtlReqInfo.faxDtlId, P_TYPE.TPC, Config.SYSTEM_PROCESS_ID, result) < 0)
                                LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", result);

                            continue;
                        }

                        if (bIsOnce)
                        {   
                            //// 성공 결과전체 DB반영 ////
                            if (DbModule.Instance.SuccessSendReqDetail(tpcReqInfo.faxId, -1, P_TYPE.TPC, Config.SYSTEM_PROCESS_ID) < 0)
                            {
                                LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", result);
                                break;
                            }
                            
                            m_lstSuccSendReqDtlTpcInfos.Add(tpcDtlReqInfo);
                            break;
                        }
                        else
                        {   
                            //// 성공 결과건별 DB반영 ////
                            if (DbModule.Instance.SuccessSendReqDetail(tpcReqInfo.faxId, tpcDtlReqInfo.faxDtlId, P_TYPE.TPC, Config.SYSTEM_PROCESS_ID) < 0)
                            {
                                LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", result);
                                continue;
                            }
                            
                            m_lstSuccSendReqDtlTpcInfos.Add(tpcDtlReqInfo);

                            continue;
                        }

                    }
                    
                    //// 이미지 병합처리 성공건이 존재하지 않을시 처리 ////
                    if (m_lstSuccSendReqDtlTpcInfos.Count <= 0)
                    {
                        LogMessage(String.Format("TIF처리 성공건이 존재하지 않습니다 [FAX_ID:{0}][TR_NO:{1}][REQ_TYPE:{2}][PREVIEW_REQ:{3}][APPROVE_REQ:{4}]",
                          tpcReqInfo.faxId, tpcReqInfo.strTrNo, tpcReqInfo.strReqType, tpcReqInfo.previewYN, tpcReqInfo.approvedReq), RESULT.F_TIFPROCESS_NOT_TIFFFILE);

                        if (DbModule.Instance.FailSendReqMaster(tpcReqInfo.faxId, P_TYPE.TPC, Config.SYSTEM_PROCESS_ID) < 0)
                            LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");

                        continue;
                    }

                    //// TPC유형일경우 DETAIL TABLE 파일명 전체업데이트 ////
                    if(bIsOnce)
                    {
                        if (!DbModule.Instance.UpdateAllTifName(tpcReqInfo.faxId, tpcReqInfo.m_lstSendRequestDtlTpcInfos[0].strTiffPath))
                            LogError("전체 TIF파일명을 업데이트하는 도중 오류가 발생하였습니다", RESULT.F_DB_ERROR);
                    }

                    //// 다음 프로세스로 전달 - BTF_FAX_SEND_DTL ////
                    if (DbModule.Instance.PassoverDetailSendReq(tpcReqInfo.faxId, -1, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, P_TYPE.PSC, "N", R_STATE.PSC_W) <= 0)
                    {
                        LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
                        continue;
                    }

                    //// 다음 프로세스로 전달 - BTF_FAX_SEND_MSTR ////
                    if (DbModule.Instance.PassoverMasterSendReq(tpcReqInfo.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, P_TYPE.PSC) <= 0)
                    {
                        LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
                        continue;
                    }

                    LogMessage("TIF파일 병합처리 완료", RESULT.SUCCESS);
                }
            }

            LogMessage("### 스레드 종료 ###");
            
        }

        /// <summary>
        /// 실행시 자신의 변환 대기건 초기화
        /// </summary>
        protected void ReadySendReq()
        {
            try
            {
                if (!DbModule.Instance.ReadySendRequest(Config.PROCESS_TYPE.ToString(), Config.SYSTEM_PROCESS_ID))
                {
                    // 알람 - Fault
                    AppLog.Write(LOG_LEVEL.WRN, "처리대기건 초기화 실패.");
                }
                else
                {
                    AppLog.Write(LOG_LEVEL.MSG, "처리대기건 초기화 완료.");
                }
            }
            catch (Exception ex)
            {
                // 알람 - Fault
                AppLog.Write(LOG_LEVEL.WRN
                    , string.Format("처리대기건 초기화 중 다음과 같은 오류가 발생하였습니다. 오류:{0}", ex.Message));
            }
        }

        /// <summary>
        /// 팩스 스토리지에 접근 가능한지 시험한다.
        /// </summary>
        /// <returns></returns>
        protected bool ExistsFaxStorage()
        {
            try
            {
                // 팩스스토리지 점검
                if (!Directory.Exists(Config.STG_HOME_PATH))
                    return false;
            }
            catch (Exception ex)
            {
                // 알람 - 장애
                AppLog.Write(LOG_LEVEL.ERR
                    , string.Format("팩스스토리지({0})에 접근중 오류가 발생하였습니다. 오류:{1}", Config.STG_HOME_PATH, ex.Message));

                return false;
            }

            return true;
        }

        #endregion

        #region method

        RESULT ProcessingTifs_(DbModule.SEND_REQUEST_TPC p_tpcInfo, DbModule.SEND_REQUEST_DTL_TPC p_tpcDtlInfo, List<DbModule.DOC_INFO_ALL_TPC> p_lstTpcDocInfos)
        {
            RESULT result = RESULT.EMPTY;

            //// 파일들 정보 SET ////
            p_tpcInfo.m_lstProcessingInfos.Clear();
            foreach (DbModule.DOC_INFO_ALL_TPC tpcDocInfo in p_lstTpcDocInfos)
            {
                string strLocalTifPathFile;

                PROCESSING_INFO info = new PROCESSING_INFO();
                info.exist = false;
                info.seq = tpcDocInfo.seq;
                info.strProcessingMode = tpcDocInfo.strProcessingMode;
                info.strPages = tpcDocInfo.strTiffExtractPages;
                info.processed = false;

                //// 소스 TIF 파일 가져오기 ////
                result = CopySourceTif(p_tpcInfo.faxId, p_tpcDtlInfo.faxDtlId, tpcDocInfo, out strLocalTifPathFile);
                if (result != RESULT.SUCCESS)
                {
                    DbModule.Instance.ResultProcessingReq(info.seq, false);
                    return result;
                }

                info.exist = true;
                info.strFile = strLocalTifPathFile;

                LogMessage(string.Format("TIF파일 다운로드 ({0}\\{1}\\{2})->({3})", Config.CONVERTED_TIF_PATH, tpcDocInfo.strDocPath, tpcDocInfo.strDocFile, strLocalTifPathFile), RESULT.SUCCESS);
                p_tpcInfo.m_lstProcessingInfos.Add(info);
            }

            if (p_tpcInfo.m_lstProcessingInfos.Count() <= 0)
            {
                LogError("존재하는 TIF파일이 0개입니다", RESULT.F_FILE_CNT_ZERO);
                return RESULT.F_FILE_CNT_ZERO;
            }
            
            string strMergedFile = string.Format(@"{0}\{1}_{2}.TIF", Config.TIFF_PROCESSING_PATH, p_tpcInfo.faxId, p_tpcDtlInfo.faxDtlId);
            string strFinishedFile = string.Format(@"{0}\{1}", Config.FINISHED_TIF_PATH, p_tpcDtlInfo.strTiffPath);

            //// TIF파일병합 ////
            result = tifAccess.Merge(strMergedFile, ref p_tpcInfo.m_lstProcessingInfos);

            foreach (PROCESSING_INFO info in p_tpcInfo.m_lstProcessingInfos)
                DbModule.Instance.ResultProcessingReq(info.seq, info.processed);

            if (result != RESULT.SUCCESS)
                return result;

            LogMessage(string.Format("TIF파일 병합완료 ({0})", strMergedFile), RESULT.SUCCESS);

            //// 최종TIF 업로드 - 3회시도 ////
            if (!FnFile.UploadFinishedTif(strMergedFile, strFinishedFile))
                return RESULT.F_FILE_FAIL_TO_UPLOAD;

            LogMessage(string.Format("TIF파일 업로드 완료 ({0})->({1})", strMergedFile, strFinishedFile), RESULT.SUCCESS);

            //// 임시파일 제거 ////
            DeleteFile(strMergedFile);
            DeleteTemporaryTif(p_tpcInfo);

            return RESULT.SUCCESS;
        }

        private void DeleteTemporaryTif(DbModule.SEND_REQUEST_TPC p_tpcInfo)
        {
            foreach (PROCESSING_INFO Info in p_tpcInfo.m_lstProcessingInfos)
            {
                try { File.Delete(Info.strFile); }
                catch { continue; }
            }
        }

        private RESULT CopySourceTif(decimal p_faxId, decimal p_faxDtlId, DbModule.DOC_INFO_ALL p_docInfo, out string p_strLocalTifPathFile)
        {
            p_strLocalTifPathFile = "";

            string strSourceFile = "";

            // 확장자 체크 및 소스파일 SET
            switch(p_docInfo.processingState)
            {
                case "N": // 미작업
                    if (p_docInfo.strProcessingMode == ((int)DOC_PROCESSING_MODE.TPC_NO_EXTRACT).ToString() ||
                        p_docInfo.strProcessingMode == ((int)DOC_PROCESSING_MODE.TPC_EXTRACT).ToString())
                    {
                        string strExtensionUpper = p_docInfo.strDocExt.ToUpper();
                        if (strExtensionUpper != "TIF" && strExtensionUpper != "TIFF")
                        {
                            LogError(string.Format("병합할 TIF문서({0}.{1})는 TIF파일이 아닙니다. 프로세싱모드({2}).", 
                                                        p_docInfo.strDocFile, p_docInfo.strDocExt, p_docInfo.strProcessingMode) ,
                                                        RESULT.F_TIFPROCESS_NOT_TIFFFILE
                                                        );
                            return RESULT.F_TIFPROCESS_NOT_TIFFFILE;
                        }
                    }
                    else
                    {
                        LogError( string.Format("병합할 문서({0}.{1})에 대한 이전작업이 완료되지 않은 상태({2}입니다.",
                                                    p_docInfo.strDocFile, p_docInfo.strDocExt, p_docInfo.processingState),
                                                    RESULT.F_TIFPROCESS_PREJOB_NOTDONE
                                                    );
                        return RESULT.F_TIFPROCESS_PREJOB_NOTDONE;
                    }

                    strSourceFile = string.Format("{0}\\{1}\\{2}.{3}", Config.INPUT_DOCS_PATH,
                                                                        p_docInfo.strDocPath,
                                                                        p_docInfo.strDocFile,
                                                                        p_docInfo.strDocExt);
                    break;

                case "M": // TMK에 의해 생성된 TIF 파일
                    strSourceFile = string.Format("{0}\\{1}_{2}_tifmake.TIF", Config.MADE_TIF_PATH,
                                                                    p_faxId, p_faxDtlId);
                    break;

                case "D": // DCV에 의한 문서 변환된 TIF파일                    
                    strSourceFile = string.Format("{0}\\{1}.TIF", Config.CONVERTED_TIF_PATH,
                                                                        p_docInfo.strDocFile);
                    break;

                default:
                    LogError(string.Format("병합할 문서({0}.{1})의 상태({2}가 올바르지 않습니다.", 
                                               p_docInfo.strDocFile, p_docInfo.strDocExt, p_docInfo.processingState),
                                               RESULT.F_TIFPROCESS_DOC_STATE_INVALID );
                    return RESULT.F_TIFPROCESS_DOC_STATE_INVALID;
            }

            string strDestFile = string.Format("{0}\\{1}_{2}_{3}.TIF", Config.TIFF_PROCESSING_PATH,
                                                                        p_faxId,
                                                                        p_faxDtlId,
                                                                        p_docInfo.strDocFile);

            if (!File.Exists(strSourceFile))
            {
                LogError(string.Format("병합할 TIF문서({0})를 찾을 수 없습니다.", strSourceFile), RESULT.F_FILE_NOT_EXIST);
                return RESULT.F_FILE_NOT_EXIST;
            }

            if (!DownloadTifFile(strSourceFile, strDestFile))
                return RESULT.F_FILE_FAIL_TO_DOWNLOAD;

            p_strLocalTifPathFile = strDestFile;
            return RESULT.SUCCESS;
        }

        private bool DownloadTifFile(string p_strSrcFileName, string p_strDestFileName)
        {
            if (!Directory.Exists(Config.TIFF_PROCESSING_PATH))
                Directory.CreateDirectory(Config.TIFF_PROCESSING_PATH);

            return CopyFile(p_strSrcFileName, p_strDestFileName);
        }

        private bool CopyFile(string p_strSrcFileFullName, string p_strDestFileFullName)
        {
            bool ret = false;
            for (int i = 0; i < 3; i++)
            {
                try
                {
                    File.Copy(p_strSrcFileFullName, p_strDestFileFullName, true);
                    ret = true;
                }
                catch(Exception ex)
                {
                    LogError(string.Format("병합할 TIF문서({0})를 다운로드({1})하는 도중, 다음과 같은 오류가 발생하였습니다. {2}", p_strSrcFileFullName, p_strDestFileFullName, ex.Message),
                            RESULT.F_FILE_FAIL_TO_DOWNLOAD);

                    ret = false;
                    Thread.Sleep(500);
                    continue;
                }
            }

            return ret;
        }

        private bool DeleteFile(string p_strFileFullName)
        {
            bool ret = false;
            for (int i = 0; i < 3; i++)
            {
                try
                {
                    File.Delete(p_strFileFullName);
                    ret = true;
                    break;
                }
                catch
                {
                    ret = false;
                    Thread.Sleep(500);
                    continue;
                }
            }

            return ret;
        }

        void ClearReqInfo()
        {
            m_lstSendReqTpcInfos.Clear();
        }

        private void LogMessage(string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            AppLog.Write(LOG_LEVEL.MSG, string.Format("[RESULT:{0}] {1}", p_result, p_strMsg));
        }

        private void LogError(string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            AppLog.Write(LOG_LEVEL.ERR, string.Format("[RESULT:{0}] {1}", p_result, p_strMsg));
        }
        #endregion

        #region field
        //// TPC처리 요청건 리스트 ////
        List<DbModule.SEND_REQUEST_TPC> m_lstSendReqTpcInfos = new List<DbModule.SEND_REQUEST_TPC>();

        //// 팩스상세건 처리 성공건 리스트 ////
        List<DbModule.SEND_REQUEST_DTL_TPC> m_lstSuccSendReqDtlTpcInfos = new List<DbModule.SEND_REQUEST_DTL_TPC>();

        //// TIF처리 Instance ////
        TifAccess tifAccess = new TifAccess();
        #endregion
    }
}
