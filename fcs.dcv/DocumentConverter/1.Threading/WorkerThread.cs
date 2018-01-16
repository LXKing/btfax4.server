using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using Microsoft.Win32;
using Btfax.CommonLib;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Util;
using DocumentConverter.Db;
using DocumentConverter.Util;
using DocumentConverter.Print;

namespace DocumentConverter.Threading
{   
    class WorkerThread : BtfaxThread
    {
        #region static
        static WorkerThread s_instance = null;
        public static WorkerThread Instance
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

                //// 변환 요청 건 조회 ////
                RESULT result = DbModule.Instance.FetchSendReqToConvert(ref m_lstSendReqDcvInfos);

                switch (result)
                {
                    case RESULT.EMPTY:
                    case RESULT.SUCCESS:
                        break;

                    default:
                        AppLog.Write(LOG_LEVEL.ERR, "DB 폴링 중 오류가 발생하였습니다");
                        continue;
                }

                LogMessage("DbPolling Check..");

                if (m_lstSendReqDcvInfos.Count <= 0)
                    continue;

                foreach (DbModule.SEND_REQUEST_DCV dcvReqInfo in m_lstSendReqDcvInfos)
                {
                    string logInfo = string.Format("[FAX_ID:{0}][TR_NO:{1}][REQ_TYPE:{2}]", dcvReqInfo.faxId, dcvReqInfo.strTrNo, dcvReqInfo.strReqType);
                    LogMessage(string.Format("변환요청건 점유 {0}", logInfo));

                    //// 다음 프로세스 유형 SET ////
                    P_TYPE nextProcessType = P_TYPE.NONE;
                    switch (Convert.ToInt32(dcvReqInfo.strReqType))
                    {
                        case (int)REQ_TYPE.DCV:         nextProcessType = P_TYPE.PSC; break;
                        case (int)REQ_TYPE.DCV_TPC:     nextProcessType = P_TYPE.TPC; break;
                        case (int)REQ_TYPE.TMK_DCV_TPC: nextProcessType = P_TYPE.TPC; break;

                        //default
                        default :
                            result = DbModule.Instance.FailSendReqs(dcvReqInfo.faxId, -1, Config.PROCESS_TYPE, Config.PROCESS_ID, RESULT.F_REQ_TYPE_ERROR);
                            if (result != RESULT.SUCCESS)
                                LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", result);
                            
                            continue;
                    }

                    //// 변환 작업 ////
                    result = ConvertDocs(dcvReqInfo, dcvReqInfo.m_lstDocInfoAllDcv, nextProcessType);
                    if (result != RESULT.SUCCESS)
                    {
                        LogError(string.Format("문서변환 실패({0}) {1}", result, logInfo));
                        result = DbModule.Instance.FailSendReqs(dcvReqInfo.faxId, -1, Config.PROCESS_TYPE, Config.PROCESS_ID, RESULT.F_REQ_TYPE_ERROR);
                        if (result != RESULT.SUCCESS)
                            LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", result);

                        continue;
                    }

                    LogMessage(string.Format("문서변환 성공 {0}", logInfo), RESULT.SUCCESS);

                    //// 성공 결과 DB 반영 ////
                    foreach (DbModule.SEND_REQUEST_DTL_DCV dtlReq in dcvReqInfo.m_lstSendRequestDtlDcvInfos)
                    {
                        if (DbModule.Instance.SuccessSendReqDetail(dcvReqInfo.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID) < 0)
                        {
                            LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", RESULT.F_DB_UPDATE_ERROR);
                            continue;
                        }
                    }

                    //// DETAIL건 파일명 전체업데이트 ////
                    if (!DbModule.Instance.UpdateAllTifName(dcvReqInfo.faxId, dcvReqInfo.strLastTifFile))
                    {
                        LogError("전체 TIF파일명을 업데이트하는 도중 오류가 발생하였습니다", RESULT.F_DB_UPDATE_ERROR);
                        continue;
                    }

                    //// DETAIL 전체 Pass over ////
                    if (DbModule.Instance.PassoverDetailSendReq(dcvReqInfo.faxId, -1, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, nextProcessType, "N") <= 0)
                    {
                        LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", RESULT.F_DB_UPDATE_ERROR);
                        continue;
                    }

                    //// MSTR Pass over ////
                    if (DbModule.Instance.PassoverMasterSendReq(dcvReqInfo.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, nextProcessType) <= 0)
                    {
                        LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", RESULT.F_DB_ERROR);
                        continue;
                    }

                    LogMessage(string.Format("변환요청 처리완료 {0}", logInfo), RESULT.SUCCESS);
                }
            }

            AppLog.Write(LOG_LEVEL.MSG, "### 쓰레드 종료 ###");
        }
        #endregion

        #region method
        RESULT ConvertDocs(DbModule.SEND_REQUEST_DCV p_dcvReqInfo, List<DbModule.DOC_INFO_ALL_DCV> p_lstDocInfos, P_TYPE p_destProcessType)
        {
            foreach (DbModule.DOC_INFO_ALL docInfo in p_lstDocInfos)
            {
                if (docInfo.strProcessingMode != ((int)DOC_PROCESSING_MODE.DCV).ToString())
                    continue;

                RESULT result = ConvertDoc(p_dcvReqInfo, p_destProcessType, p_dcvReqInfo.faxId, docInfo.strDocPath, docInfo.strDocFile, docInfo.strDocExt);
                if (result != RESULT.SUCCESS)
                {   
                    if(!DbModule.Instance.ResultProcessingReq(docInfo.seq, false))
                        LogError("문서변환실패 상태를 DB에 반영하는 도중 오류가 발생하였습니다", RESULT.F_DB_ERROR);

                    return result;
                }

                if (!DbModule.Instance.ResultProcessingReq(docInfo.seq, true))
                    LogError("문서변환성공 상태를 DB에 반영하는 도중 오류가 발생하였습니다", RESULT.F_DB_ERROR);
            }

            return RESULT.SUCCESS;
        }


        private bool CopyFile(string p_strSrcFileFullName, string p_strDestFileFullName)
        {
            bool ret = false;
            for (int i = 0; i < 3; i++)
            {
                try
                {
                    int idx = p_strDestFileFullName.LastIndexOf("\\");
                    string path = p_strDestFileFullName.Substring(0, idx);
                    if (!Directory.Exists(path))
                        Directory.CreateDirectory(path);

                    if (!File.Exists(p_strSrcFileFullName))
                    {
                        ret = false;
                        break;
                    }

                    File.Copy(p_strSrcFileFullName, p_strDestFileFullName, true);
                    ret = true;
                }
                catch(Exception ex)
                {
                    LogError(ex.Message);
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

        private bool DownloadDocFile(string p_strSrcFileName, string p_strDestFileName)
        {
            if (!Directory.Exists(Config.TIFF_CONVERTING_PATH))
                Directory.CreateDirectory(Config.TIFF_CONVERTING_PATH);

            return CopyFile(p_strSrcFileName, p_strDestFileName);
        }

        //private bool UploadFinishedTif(string p_strSrcFileName, string p_strDestFileName)
        //{
        //    return CopyFile(p_strSrcFileName, p_strDestFileName);
        //}

        RESULT ConvertDoc(DbModule.SEND_REQUEST_DCV p_sendReqInfo, P_TYPE p_destProcessType, decimal p_faxId, string p_strPath, string p_strFile, string p_strExt)
        {   
            if (p_sendReqInfo.m_lstSendRequestDtlDcvInfos.Count <= 0)
                return RESULT.F_CONV_DOCLIST_NOTEXIST_IN_DB;

            DbModuleBase.SEND_REQUEST_DTL dtlReq = p_sendReqInfo.m_lstSendRequestDtlDcvInfos[0];

            //// 파일 경로/이름 SET ////
            if (p_strPath == ".")
                p_strPath = "";
            else
                p_strPath = p_strPath + "\\";

            string strSourceFile = string.Format("{0}\\{1}{2}.{3}", Config.INPUT_DOCS_PATH, p_strPath, p_strFile, p_strExt);
            string strDestFile = string.Format("{0}\\{1}{2}.{3}", Config.TIFF_CONVERTING_PATH, p_strPath, p_strFile, p_strExt);

            string strFinishFile;
            p_sendReqInfo.strLastTifFile = dtlReq.strTiffPath;
            if (p_destProcessType == P_TYPE.PSC)
                strFinishFile = string.Format("{0}\\{1}", Config.FINISHED_TIF_PATH, dtlReq.strTiffPath);
            else
                strFinishFile = string.Format("{0}\\{1}.TIF", Config.CONVERTED_TIF_PATH, p_strFile);
                
            //// 변환대상 문서 다운로드 ////
            if(!File.Exists(strSourceFile))
                return RESULT.F_FILE_NOT_EXIST;

            if (!DownloadDocFile(strSourceFile, strDestFile))
                return RESULT.F_FILE_FAIL_TO_DOWNLOAD;

            //// 문서포멧 확인 ////
            bool isImgFormat = false;
            string strConvertedFile = "";
            if (PrintingHandler.Instance.IsImageFormat(strDestFile, Config.CONVERT_ABLE_IMG_FORMAT))
                isImgFormat = true;

            strConvertedFile = string.Format("{0}\\{1}.TIF", Config.TIFF_CONVERTING_PATH, p_strFile);

            //// 문서변환 ////
            LogMessage(string.Format("변환처리 ({0}) -> ({1})", strDestFile, strConvertedFile), RESULT.EMPTY);
			
			//// 이미지 포멧일 경우 자체 TIF Conversion ///
			if (isImgFormat)
			{
				if (!PrintingHandler.Instance.Converert(strDestFile, strConvertedFile))
				{
					LogError(string.Format("TIFF파일을 변환하지 못하였습니다.{0}->{1}", strDestFile, strConvertedFile));
					return RESULT.F_CONV;
				}

				LogMessage(string.Format("TIFF파일 변환완료 ({0}) -> ({1})", strDestFile, strConvertedFile), RESULT.SUCCESS);

				if (!CopyFile(strConvertedFile, strFinishFile))
					return RESULT.F_CONV;

				LogMessage(string.Format("변환된 최종TIF 업로드 성공 ({0}) -> ({1})", strConvertedFile, strFinishFile), RESULT.SUCCESS);

				//// 변환이전 로컬 파일 삭제 ////
				if (!DeleteFile(strDestFile))
					LogWarring(string.Format("변환이전 로컬파일({0}) 삭제중 오류가 발생하였습니다.", strDestFile), RESULT.F_CONV);

				//// 변환된 로컬파일 삭제 ////
				if (!DeleteFile(strConvertedFile))
					LogMessage(string.Format("변환된 로컬파일({0})을 삭제중 오류가 발생하였습니다.", strDestFile), RESULT.F_CONV);

				// 임시파일 삭제
				if (Config.DELETE_INPUT_FILE_YN == "Y")
				{
					File.Delete(strSourceFile);
					LogMessage(string.Format("임시파일({0})을 제거하였습니다.", strSourceFile), RESULT.EMPTY);
				}

				//// 파일 업로드 성공인 경우, 함수 종료. ////
				return RESULT.SUCCESS;
			}


			////// 프리터드라이버체크 ////
			if (!PrintingHandler.Instance.ExistsPrintDriver(Config.EXEC_PRINT_DRIVER))
			{
				LogError(string.Format("프린터드라이버({0})가 존재하지 않습니다.", Config.EXEC_PRINT_DRIVER), RESULT.F_CONV);
				return RESULT.F_CONV;
			}

			// 프리터드라이버 -> 기본프린터로 설정 ////
			PrintingHandler.Instance.SetPrinter(Config.EXEC_PRINT_DRIVER);

			//// DCV_CALLBACK.EXE 프로세스가 처리하기 위함. ////
			if (!File.Exists(Config.DCV_CALLBACK_INI_FILE))
			{
				LogError(string.Format("파일이 존재하지 않습니다.{0}", Config.DCV_CALLBACK_INI_FILE));
				return RESULT.F_FILE_NOT_EXIST;
			}

			StringUtil.WritePrivateProfileStringEx(Config.DCV_CALLBACK_INI_FILE, "CONV_INFO", "DCV_OUTPUT", strConvertedFile);
			StringUtil.WritePrivateProfileStringEx(Config.DCV_CALLBACK_INI_FILE, "CONV_INFO", "STATE", "READY");

            if (!PrintingHandler.Instance.Print(strDestFile))
                return RESULT.F_CONV;
            
            bool find = false;
            string state = "";
            int nTurn = Config.CONVERTING_WAIT_TIME / 1000; // 1초마다 파일 존재여부 체크
            for (int i = 0; i < nTurn; i++)
            {
                Thread.Sleep(1000);
                state = StringUtil.GetPrivateProfileStringEx(Config.DCV_CALLBACK_INI_FILE, "CONV_INFO", "STATE");
                if (!state.Contains("COMPLETED"))
                    continue;
                                
                if (!File.Exists(strConvertedFile))
                    continue;

                find = true;

                if (!CopyFile(strConvertedFile, strFinishFile))
                    return RESULT.F_CONV;

                LogMessage(string.Format("변환된 최종TIF 업로드 성공 ({0}) -> ({1})", strConvertedFile, strFinishFile), RESULT.SUCCESS);

                //// 변환이전 로컬 파일 삭제 ////
                if (!DeleteFile(strDestFile))
                    LogWarring(string.Format("변환이전 로컬파일({0}) 삭제중 오류가 발생하였습니다.", strDestFile), RESULT.F_CONV);

                //// 변환된 로컬파일 삭제 ////
                if (!DeleteFile(strConvertedFile))
                    LogMessage(string.Format("변환된 로컬파일({0})을 삭제중 오류가 발생하였습니다.", strDestFile), RESULT.F_CONV);
              
                // 임시파일 삭제
				if (Config.DELETE_INPUT_FILE_YN == "Y")
				{	
					File.Delete(strSourceFile);
					LogMessage(string.Format("임시파일({0})을 제거하였습니다.", strSourceFile), RESULT.EMPTY);
				}

				//// 파일 업로드 성공인 경우, 함수 종료. ////
                return RESULT.SUCCESS;
            }
            
            if (find)
                LogError(string.Format("변환되어진 TIF파일({0})이 대상 경로({1})로 이동되지 못했습니다.", strConvertedFile, strFinishFile), RESULT.F_FILE_FAIL_TO_UPLOAD);

            LogError(string.Format("문서({0})가 TIF파일로 {1}초동안 변환되지 못하였습니다.", strDestFile, Config.CONVERTING_WAIT_TIME / 1000), RESULT.F_CONV_TIMEOUT);
            return RESULT.F_CONV_TIMEOUT;
        }


        private void ClearReqInfo()
        {
            m_lstSendReqDcvInfos.Clear();
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
                {   
                    AppLog.Write(LOG_LEVEL.ERR
                        , string.Format("팩스스토리지를 찾을수 없습니다. 경로:{0}", Config.STG_HOME_PATH));

                    return false;
                }
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


        private void LogMessage(string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            AppLog.Write(LOG_LEVEL.MSG, string.Format("[RESULT:{0}] {1}", p_result, p_strMsg));
        }

        private void LogError(string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            AppLog.Write(LOG_LEVEL.ERR, string.Format("[RESULT:{0}] {1}", p_result, p_strMsg));
        }

        private void LogWarring(string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            AppLog.Write(LOG_LEVEL.WRN, string.Format("[RESULT:{0}] {1}", p_result, p_strMsg));
        }
        #endregion

        #region field
        List<DbModule.SEND_REQUEST_DCV> m_lstSendReqDcvInfos = new List<DbModule.SEND_REQUEST_DCV>();        
        #endregion
    }
}
