﻿using System;
using System.Collections.Generic;
using System.Threading;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using Btfax.CommonLib;
using Btfax.CommonLib.Db;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib.Alarm;
using PostFcs.Util;
using PostFcs.DB;

namespace PostFcs.Threading
{
    class DbPollingThread : BtfaxThread
    {
        #region static
        private static DbPollingThread s_instance = null;
        public static DbPollingThread Instance
        {
            get 
            {
                if (s_instance == null)
                    s_instance = new DbPollingThread();
                return s_instance;
            }
        }
        #endregion
        
        #region method
		//private void StartPostProcessingThreads()
		//{
		//    for (int i = 0; i < Config.THREAD_CNT; i++)
		//    {
		//        BtfaxThread thread = new PostProcessingThread();
		//        thread.StartThread();

		//        m_lstPostProcessingThreads.Add(thread);
		//    }
		//}
                
		//private void JoinPostProcessingThreads()
		//{
		//    foreach (BtfaxThread thread in m_lstPostProcessingThreads)
		//    {
		//        if (!thread.JoinThread())
		//            AppLog.Write(LOG_LEVEL.MSG, "쓰레드 종료중 오류가 발생하였습니다.");
		//    }
		//    m_lstPostProcessingThreads.Clear();
		//}        
        #endregion

        #region override
        public override bool StartThread()
        {   
            //StartPostProcessingThreads();
            return base.StartThread();
        }

        public override bool JoinThread(int p_nTimeout = -1)
        {   
            //JoinPostProcessingThreads();
            if (!base.JoinThread(p_nTimeout))
                return false;
            return true;
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

                if (!ExistsFaxStorage())
                {
                    AppLog.Write(LOG_LEVEL.ERR, string.Format("팩스스토리지를 찾을수 없습니다. 위치:{0}", Config.STG_HOME_PATH));
                    continue;
                }

                if (m_lstPostProcessingReqs.Count > 0)
                    m_lstPostProcessingReqs.Clear();

                //// 처리건 조회 ////                    
                RESULT ret = DbModule.Instance.FetchPostProcessingRequest(Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, Config.FETCH_CNT, ref m_lstPostProcessingReqs);
                switch (ret)
                {
                    case RESULT.EMPTY:
                    case RESULT.SUCCESS:
                        break;

                    default:                        
                        AppLog.Write(LOG_LEVEL.ERR, "DB 폴링 중 오류가 발생하였습니다");
                        continue;
                }

				AppLog.Write(LOG_LEVEL.MSG, "DB Polling check OK.");
                
                // 후처리 시작
				foreach (DbModule.SEND_REQUEST_PSC req in m_lstPostProcessingReqs)
				{	
					PostProcessing(req);
				}
            }
            AppLog.Write(LOG_LEVEL.MSG, "### DB폴링 종료 ###");
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


		protected RESULT PostProcessing(DbModule.SEND_REQUEST_PSC p_pscReq)
		{	
			AppLog.Write(LOG_LEVEL.MSG, String.Format("[RESULT:{0}] 팩스요청건점유 [FAX_ID:{1}][TR_NO:{2}][REQ_TYPE:{3}][PREVIEW_REQ:{4}][APPROVE_REQ:{5}]",
											   RESULT.EMPTY,
											   p_pscReq.faxId,
											   p_pscReq.strTrNo,
											   p_pscReq.strReqType,
											   p_pscReq.previewYN,
											   p_pscReq.approvedReq
											   ));

			RESULT result = RESULT.EMPTY;
			m_lstSuccReqDtls.Clear();
			if (p_pscReq.strState == ((int)R_STATE.PSC_R).ToString())
			{
				result = HandleBeforeWeb(p_pscReq);
			}
			else if (p_pscReq.strState == ((int)R_STATE.PSC_RS).ToString())
			{
				result = HandleAfterWeb(p_pscReq);
			}
			else
			{
				result = RESULT.F_STATE_ERROR;
			}

			if (result != RESULT.SUCCESS)
			{
				/// 에러처리
				if (DbModule.Instance.FailSendReqMaster(p_pscReq.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID) < 0)
				{
					//AlarmAPI.Instance.SendFaultAlarm((UInt32)RESULT.F_DB_UPDATE_ERROR);
					LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
				}

				LogMessage(String.Format("[RESULT:{0}] MASTER 실패건 처리완료 [FAX_ID:{1}][TR_NO:{2}][REQ_TYPE:{3}][PREVIEW_REQ:{4}][APPROVE_REQ:{5}]",
											RESULT.SUCCESS,
											p_pscReq.faxId,
											p_pscReq.strTrNo,
											p_pscReq.strReqType,
											p_pscReq.previewYN,
											p_pscReq.approvedReq
											));

				//AlarmAPI.Instance.SendFaultAlarm((UInt32)result);
			}
			
			return result;
		}
		
		protected RESULT HandleBeforeWeb(DbModule.SEND_REQUEST_PSC p_pscReq)
		{
			P_TYPE destProcessType = P_TYPE.NONE;
			R_STATE state = R_STATE.INIT;
			//int nState = 0;


			//// Beffore처리 1. 문서필터링 처리 ////
			if (PostFcs.Util.Config.DOC_FILTERING_YN == "Y")
			{
				#region TO - DO

				#endregion
			}

			//// Before처리 2. TIF 크기, 페이지 SET ////
			long lTiffFileSize = -1;
			int nTiffPageCnt = -1;

			foreach (DbModule.SEND_REQUEST_DTL_PSC dtlReq in p_pscReq.m_lstSendRequestDtlPscInfos)
			{
				string strLocalTiffFullName = "";
				string logExtMsg = String.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}][FAX_NO:{2}]",
									dtlReq.faxId
									, dtlReq.faxDtlId
									, dtlReq.strFaxNo
									);

				if (!DownloadTiffFile(dtlReq, out strLocalTiffFullName))
				{
					LogError(String.Format("TIF다운로드 실패 [RESULT:{0}]{1}", RESULT.F_FILE_NOT_EXIST, logExtMsg));
					continue;
				}

				LogMessage(String.Format("TIF파일 다운로드 완료 [RESULT:{0}]{1}", RESULT.SUCCESS, logExtMsg));

				if (!GetTiffFileNames(dtlReq, strLocalTiffFullName))
				{
					dtlReq.strStateEach = R_STATE.PSC_F.ToString();
					LogError(String.Format("TIF파일명 정보를 알수없습니다 [RESULT:{0}]{1}", RESULT.F_FILE_NOT_EXIST, logExtMsg));

					if (DbModule.Instance.FailSendReqDetail(p_pscReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, RESULT.F_FILE_NOT_EXIST) < 0)
						LogError(String.Format("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 [RESULT:{0}]{1}", RESULT.F_DB_ERROR, logExtMsg));

					continue;
				}

				lTiffFileSize = GetFileSize(dtlReq.strTiffFullName);
				if (lTiffFileSize < 0)
				{
					dtlReq.strStateEach = R_STATE.PSC_F.ToString();
					LogError(String.Format("TIF사이즈 정보를 알수없습니다 [RESULT:{0}]{1}", RESULT.F_FILE_FAIL_GET_FILE_SIZE, logExtMsg));

					if (DbModule.Instance.FailSendReqDetail(p_pscReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, RESULT.F_FILE_FAIL_GET_FILE_SIZE) < 0)
						LogError(String.Format("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 [RESULT:{0}]{1}", RESULT.F_DB_ERROR, logExtMsg));

					continue;
				}

				LogMessage(String.Format("TIF파일 사이즈 > {0} [RESULT:{1}]{2}", lTiffFileSize, RESULT.SUCCESS, logExtMsg));

				nTiffPageCnt = GetPageCount(dtlReq.strTiffFullName);
				if (nTiffPageCnt < 0)
				{
					dtlReq.strStateEach = R_STATE.PSC_F.ToString();
					LogError(String.Format("TIF페이지수 정보를 알수없습니다 [RESULT:{0}]{1}", RESULT.F_FILE_FAIL_GET_FILE_SIZE, logExtMsg));

					if (DbModule.Instance.FailSendReqDetail(p_pscReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, RESULT.F_FILE_FAIL_GET_FILE_SIZE) < 0)
						LogError(String.Format("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 [RESULT:{0}]{1}", RESULT.F_DB_ERROR, logExtMsg));

					continue;
				}

				LogMessage(String.Format("TIF파일 페이지수 > {0} [RESULT:{1}]{2}", nTiffPageCnt, RESULT.SUCCESS, logExtMsg));

				if (!DbModule.Instance.UpdateTifInfo(p_pscReq.faxId, dtlReq.faxDtlId, dtlReq.strTiffPath, (int)lTiffFileSize, nTiffPageCnt))
				{
					LogError(String.Format("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 [RESULT:{0}]{1}", RESULT.F_DB_ERROR, logExtMsg));
					continue;
				}

				//// 파일사이즈 제한 ////
				if (lTiffFileSize > Config.TIF_FILE_MAX_SIZE)
				{
					LogError(String.Format("발송가능한 파일크기를 초과하였습니다. TIF크기/최대크기:{0}/{1}", lTiffFileSize, Config.TIF_FILE_MAX_SIZE));
					if (DbModule.Instance.FailSendReqDetail(p_pscReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, RESULT.F_FILE_FAIL_FILE_SIZE_OVERFLOW) < 0)
						LogError(String.Format("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 [RESULT:{0}]{1}", RESULT.F_FILE_FAIL_FILE_SIZE_OVERFLOW, logExtMsg));

					continue;
				}

				//// DownloadTIFF 파일 삭제 ////
				if (File.Exists(strLocalTiffFullName))
					File.Delete(strLocalTiffFullName);

				m_lstSuccReqDtls.Add(dtlReq);
			}

			LogMessage(String.Format("변환후 처리 [전체:{0}] [성공:{1}] [실패:{2}]",
											p_pscReq.m_lstSendRequestDtlPscInfos.Count,
											m_lstSuccReqDtls.Count,
											(p_pscReq.m_lstSendRequestDtlPscInfos.Count - m_lstSuccReqDtls.Count)
											));

			if (m_lstSuccReqDtls.Count <= 0)
			{
				if (DbModule.Instance.FailSendReqMaster(p_pscReq.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID) < 0)
					LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");

				LogMessage(String.Format("[RESULT:{0}] MSTER 실패건 처리완료 [FAX_ID:{1}][TR_NO:{2}][REQ_TYPE:{3}][PREVIEW_REQ:{4}][APPROVE_REQ:{5}]",
											RESULT.SUCCESS,
											p_pscReq.faxId,
											p_pscReq.strTrNo,
											p_pscReq.strReqType,
											p_pscReq.previewYN,
											p_pscReq.approvedReq
											));
				return RESULT.SUCCESS;
			}


			//// 미리보기, 승인 건이 아닌경우, AfterWEB 에서 처리 ////
			if (p_pscReq.previewYN.ToUpper() != "Y" && p_pscReq.approvedReq.ToUpper() != "Y")
			{
				//예약발송이면 PSC후처리대기상태로 가도록 처리
				if (p_pscReq.reserveYN.ToUpper() == "Y")
				{
					destProcessType = P_TYPE.WEB;
					state = R_STATE.PSC_WS;

					foreach (DbModule.SEND_REQUEST_DTL dtlReq in m_lstSuccReqDtls)
					{
						if (DbModule.Instance.PassoverDetailSendReq(p_pscReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, destProcessType, "N", state) <= 0)
						{
							LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
							continue;
						}
					}
					if (DbModule.Instance.PassoverMasterSendReq(p_pscReq.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, destProcessType, state) <= 0)
					{
						LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
						return RESULT.F_DBP_FAIL_PASS_OVER_SEND_REQ_MSTR;
					}

					LogMessage(String.Format("[RESULT:{0}] 전처리 작업 완료 [FAX_ID:{1}][TR_NO:{2}][REQ_TYPE:{3}][PREVIEW_REQ:{4}][APPROVE_REQ:{5}]",
													RESULT.SUCCESS,
													p_pscReq.faxId,
													p_pscReq.strTrNo,
													p_pscReq.strReqType,
													p_pscReq.previewYN,
													p_pscReq.approvedReq
													));
					return RESULT.SUCCESS;
				}

				// 중요 !!! //
				return HandleAfterWeb(p_pscReq);
			}

			//승인 대기Y && 미리보기N 건인 경우 FAXBOX로 Insert
			if (p_pscReq.approvedReq.ToUpper() == "Y" && p_pscReq.previewYN.ToUpper() == "N")
			{
				if (!DbModule.Instance.InsertFaxbox(p_pscReq.faxId))
				{
					//실패처리
					if (DbModule.Instance.FailSendReqMaster(p_pscReq.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID) < 0)
						LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");

					LogMessage(String.Format("[RESULT:{0}] 승인대기건 FAXBOX로 이동중 실패건 처리완료 [FAX_ID:{1}][TR_NO:{2}][REQ_TYPE:{3}][PREVIEW_REQ:{4}][APPROVE_REQ:{5}]",
							RESULT.SUCCESS,
							p_pscReq.faxId,
							p_pscReq.strTrNo,
							p_pscReq.strReqType,
							p_pscReq.previewYN,
							p_pscReq.approvedReq
							));
					return RESULT.SUCCESS;
				}
			}


			//// 미리보기, 승인 건인 경우, WEB 으로 전달 ////
			destProcessType = P_TYPE.WEB;
			state = (p_pscReq.previewYN == "Y") ? R_STATE.PSC_WP : R_STATE.PSC_WA;

			foreach (DbModule.SEND_REQUEST_DTL dtlReq in m_lstSuccReqDtls)
			{
				if (DbModule.Instance.PassoverDetailSendReq(p_pscReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, destProcessType, "N", state) <= 0)
				{
					LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
					continue;
				}
			}
			if (DbModule.Instance.PassoverMasterSendReq(p_pscReq.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, destProcessType, state) <= 0)
			{
				LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
				return RESULT.F_DBP_FAIL_PASS_OVER_SEND_REQ_MSTR;
			}

			LogMessage(String.Format("[RESULT:{0}] 후처리 작업 완료 [FAX_ID:{1}][TR_NO:{2}][REQ_TYPE:{3}][PREVIEW_REQ:{4}][APPROVE_REQ:{5}]",
											RESULT.SUCCESS,
											p_pscReq.faxId,
											p_pscReq.strTrNo,
											p_pscReq.strReqType,
											p_pscReq.previewYN,
											p_pscReq.approvedReq
											));

			return RESULT.SUCCESS;
		}


		protected RESULT HandleAfterWeb(DbModule.SEND_REQUEST_PSC p_pscReq)
        {
            List<DbModule.SEND_REQUEST_DTL_PSC> outInwardList = new List<DbModule.SEND_REQUEST_DTL_PSC>();
            P_TYPE destProcessType = P_TYPE.NONE;
            R_STATE state = R_STATE.INIT;
            string strDid="";

            //미리보기Y && 승인대기Y 이면, 미리보기N으로 UPDATE 후에 PSC대기상태(71)로 변경.
            if (p_pscReq.previewYN.ToUpper() == "Y" && p_pscReq.approvedReq.ToUpper() == "Y")
            {
                if (!DbModule.Instance.UpdatePreviewInfo(p_pscReq.faxId, "N"))
                {
                    LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
                    return RESULT.F_DB_UPDATE_ERROR;
                }

                destProcessType = P_TYPE.WEB;
                state = R_STATE.PSC_W;

                foreach (DbModule.SEND_REQUEST_DTL dtlReq in p_pscReq.m_lstSendRequestDtlPscInfos)
                {
					if (DbModule.Instance.PassoverDetailSendReq(p_pscReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, destProcessType, "N", state) <= 0)
                    {
                        LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
                        continue;
                    }
                }
                if (DbModule.Instance.PassoverMasterSendReq(p_pscReq.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, destProcessType, state) <= 0)
                {
                    LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
                    return RESULT.F_DBP_FAIL_PASS_OVER_SEND_REQ_MSTR;
                }

                LogMessage(String.Format("변환후처리 작업 완료 [FAX_ID:{0}][TR_NO:{1}][REQ_TYPE:{2}][PREVIEW_REQ:{3}][APPROVE_REQ:{4}]",
                                            p_pscReq.faxId,
                                            p_pscReq.strTrNo,
                                            p_pscReq.strReqType,
                                            p_pscReq.previewYN,
                                            p_pscReq.approvedReq
                                            ));

                return RESULT.SUCCESS;

            }

            //// 결과 성공 처리 ////
            if (DbModule.Instance.SuccessSendReqDetail(p_pscReq.faxId, -1, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID) < 0)
            {
                LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
                return RESULT.F_DB_UPDATE_ERROR;
            }

            //// 내부 발송 처리 ////
            foreach (DbModule.SEND_REQUEST_DTL_PSC dtlReq in p_pscReq.m_lstSendRequestDtlPscInfos)
            {
                //팩스 번호에서 '-' 제거
                dtlReq.strFaxNo = dtlReq.strFaxNo.Replace("-", "");

                strDid = "";

				// ADD - KIMCG : 20150914
				// LOOP_FAXNO 일경우 FOD로 전달.
				if (dtlReq.strFaxNo == Config.LOOP_FAXNO)
				{
					LogMessage(String.Format("[RESULT:{0}] LOOP FAX [FAX_ID:{1}][TR_NO:{2}][REQ_TYPE:{3}][PREVIEW_REQ:{4}][APPROVE_REQ:{5}]",
								RESULT.EMPTY,
								p_pscReq.faxId,
								p_pscReq.strTrNo,
								p_pscReq.strReqType,
								p_pscReq.previewYN,
								p_pscReq.approvedReq
								));
					continue;
				}
				// ADD - END

                // 내부 팩스 번호가 아니면, SKIP.
                if (!DbModule.Instance.IsInboundFaxNo(dtlReq.strFaxNo, ref strDid))
                    continue;

                // 내부 발송 건 추가
                outInwardList.Add(dtlReq);

                // 내부 발송 처리
				RESULT result = ProcessInbound(p_pscReq, dtlReq, strDid);

                // 발송 MSTR/DTL DATA 테이블로 이동
				if (!DbModule.Instance.FinishIBSendDtl(Config.SYSTEM_NO, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, p_pscReq.strBroadcastYN, dtlReq, result))
                {
                    LogError("인바운드 케이스를 Data테이블로 이동하는 도중 오류가 발생하였습니다");
                    return RESULT.F_DB_UPDATE_ERROR;
                }
            }

            // 내부 발송 처리 완료 건, 제외
            foreach (DbModule.SEND_REQUEST_DTL_PSC dtlReq in outInwardList)
                p_pscReq.m_lstSendRequestDtlPscInfos.Remove(dtlReq);
            
            // 모두 내부 발송이면, 처리 종료.
            if (p_pscReq.m_lstSendRequestDtlPscInfos.Count <= 0)
            {
                LogMessage(String.Format("[RESULT:{0}] 후처리 작업 완료 [FAX_ID:{1}][TR_NO:{2}][REQ_TYPE:{3}][PREVIEW_REQ:{4}][APPROVE_REQ:{5}]",
                                RESULT.SUCCESS,
                                p_pscReq.faxId,
                                p_pscReq.strTrNo,
                                p_pscReq.strReqType,
                                p_pscReq.previewYN,
                                p_pscReq.approvedReq
                                ));
                return RESULT.SUCCESS;
            }

            ////외부 발송 처리 : FOD로 전달 ////
            destProcessType = P_TYPE.FOD;
            state = R_STATE.FOD_W;

            foreach (DbModule.SEND_REQUEST_DTL dtlReq in p_pscReq.m_lstSendRequestDtlPscInfos)
            {
                if (DbModule.Instance.PassoverDetailSendReq(p_pscReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, destProcessType, "N", state) <= 0)
                {
                    LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
                    continue;
                }
            }
            if (DbModule.Instance.PassoverMasterSendReq(p_pscReq.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, destProcessType, state) <= 0)
            {
                LogError("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다");
                return RESULT.F_DBP_FAIL_PASS_OVER_SEND_REQ_MSTR;
            }

            LogMessage(String.Format("변환후처리 작업 완료 [FAX_ID:{0}][TR_NO:{1}][REQ_TYPE:{2}][PREVIEW_REQ:{3}][APPROVE_REQ:{4}]",
                                            p_pscReq.faxId,
                                            p_pscReq.strTrNo,
                                            p_pscReq.strReqType,
                                            p_pscReq.previewYN,
                                            p_pscReq.approvedReq
                                            ));
            return RESULT.SUCCESS;
        }


        private RESULT ProcessInbound(DbModule.SEND_REQUEST_PSC p_pscReq, PostFcs.DB.DbModule.SEND_REQUEST_DTL_PSC dtlReq, string p_strDid)
        {   
            string strIBTiffName = "";
            string strLocalTiffFullName = "";
            long lTiffFileSize = -1;
            int nTiffPageCnt = -1;

            string logExtMsg = String.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}][FAX_NO:{2}][DID:{3}][RECIPIENT_NAME:{4}][TIF_NAME:{5}]",
                                dtlReq.faxId
                                , dtlReq.faxDtlId
                                , dtlReq.strFaxNo
                                , p_strDid
                                , dtlReq.strRecipientName
                                , dtlReq.strTiffFullName
                                );

            LogMessage(String.Format("내부발송처리 [RESULT:{0}]{1}", RESULT.EMPTY, logExtMsg));

            if (!DownloadTiffFile(dtlReq, out strLocalTiffFullName))
            {
                LogError(String.Format("TIF다운로드 실패 [RESULT:{0}]{1}", RESULT.F_FILE_NOT_EXIST, logExtMsg));
                return RESULT.F_FILE_NOT_EXIST;
            }

            LogMessage(String.Format("TIF파일 다운로드 완료 [RESULT:{0}]{1}", RESULT.SUCCESS, logExtMsg));

            if (!GetTiffFileNames(dtlReq, strLocalTiffFullName))
            {
                dtlReq.strStateEach = R_STATE.PSC_F.ToString();
                LogError(String.Format("TIF파일명 정보를 알수없습니다 [RESULT:{0}]{1}", RESULT.F_FILE_NOT_EXIST, logExtMsg));

				if (DbModule.Instance.FailSendReqDetail(dtlReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, RESULT.F_FILE_NOT_EXIST) < 0)
                    LogError(String.Format("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 [RESULT:{0}]{1}", RESULT.F_DB_ERROR, logExtMsg));

                return RESULT.F_FILE_NOT_EXIST;
            }

            lTiffFileSize = GetFileSize(dtlReq.strTiffFullName);
            if (lTiffFileSize < 0)
            {
                dtlReq.strStateEach = R_STATE.PSC_F.ToString();
                LogError(String.Format("TIF사이즈 정보를 알수없습니다 [RESULT:{0}]{1}", RESULT.F_FILE_FAIL_GET_FILE_SIZE, logExtMsg));

                if (DbModule.Instance.FailSendReqDetail(dtlReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, RESULT.F_FILE_FAIL_GET_FILE_SIZE) < 0)
                    LogError(String.Format("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 [RESULT:{0}]{1}", RESULT.F_DB_ERROR, logExtMsg));

                return RESULT.F_FILE_FAIL_GET_FILE_SIZE;
            }

            LogMessage(String.Format("TIF파일 사이즈 > {0} [RESULT:{1}]{2}", lTiffFileSize, RESULT.SUCCESS, logExtMsg));

            nTiffPageCnt = GetPageCount(dtlReq.strTiffFullName);
            if (nTiffPageCnt < 0)
            {
                dtlReq.strStateEach = R_STATE.PSC_F.ToString();
                LogError(String.Format("TIF페이지수 정보를 알수없습니다 [RESULT:{0}]{1}", RESULT.F_FILE_FAIL_GET_FILE_SIZE, logExtMsg));

                if (DbModule.Instance.FailSendReqDetail(dtlReq.faxId, dtlReq.faxDtlId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, RESULT.F_FILE_FAIL_GET_FILE_SIZE) < 0)
                    LogError(String.Format("작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다 [RESULT:{0}]{1}", RESULT.F_DB_ERROR, logExtMsg));

                return RESULT.F_FILE_FAIL_GET_FILE_SIZE;
            }

            LogMessage(String.Format("TIF파일 페이지수 > {0} [RESULT:{1}]{2}", nTiffPageCnt, RESULT.SUCCESS, logExtMsg));


            //Out -> In 디렉토리로 Tif파일 복사 후 삭제.
            strIBTiffName = string.Format("{0}\\{1}", Config.INBOUND_TIF_PATH, dtlReq.strTiffPath);

            string strDirectory = strIBTiffName.Substring(0, strIBTiffName.LastIndexOf("\\"));
            if (!Directory.Exists(strDirectory))
                Directory.CreateDirectory(strDirectory);

            try
            {
                LogMessage(String.Format("TIF파일 Out -> In 디렉토리로 파일 복사 > {0} -> {1} [RESULT:{2}]{3}", dtlReq.strTiffFullName, strIBTiffName, RESULT.SUCCESS, logExtMsg));
                File.Copy(dtlReq.strTiffFullName, strIBTiffName, true);
                File.Delete(dtlReq.strTiffFullName);
            }
            catch(Exception ex)
            {
                LogError(ex.Message);
            }
			

            //수신함으로 Insert
			decimal recvFaxId = DbModule.Instance.InsertRecvMaster(Config.SYSTEM_NO, Config.PROCESS_NO, lTiffFileSize, nTiffPageCnt, p_pscReq.strReqUserTelNo, p_strDid, dtlReq);
            if (recvFaxId < 0)
            {
                LogError("인바운드 케이스를 수신함에 Insert하는 도중 오류가 발생하였습니다");
                return RESULT.F_DBP_FAIL_MOVE_TO_DATA;
            }

            //수신함으로 Running state
            if (!DbModule.Instance.UpdateRecvStateRunning(recvFaxId))
            {
                LogError("인바운드 케이스의 수신함 상태를 변경하는 도중 오류가 발생하였습니다");
                return RESULT.F_DB_UPDATE_ERROR;
            }

            if (!DbModule.Instance.UpdateRecvStateFinished(recvFaxId, (int)RESULT.SUCCESS, (int)RESULT.SUCCESS, dtlReq.strTiffPath, lTiffFileSize, nTiffPageCnt))
            {
                LogError("인바운드 케이스의 수신함 상태를 완료 상태로 변경하는 도중 오류가 발생하였습니다");
                return RESULT.F_DB_UPDATE_ERROR;
            }

            return RESULT.SUCCESS;
        }






















		private bool DownloadTiffFile(DbModule.SEND_REQUEST_DTL p_dtlReq, out string p_downTiffFullName)
		{
			p_downTiffFullName = "";

			bool ret = false;
			for (int cnt = 0; cnt < 3; cnt++)
			{
				try
				{
					string localPath = string.Format("{0}\\tiff", System.Windows.Forms.Application.StartupPath);
					if (!Directory.Exists(localPath))
						Directory.CreateDirectory(localPath);

					int idx = p_dtlReq.strTiffPath.LastIndexOf("\\");
					if (idx < 0)
						continue;

					string fileName = p_dtlReq.strTiffPath.Substring(idx + 1);
					string localFileFullName = string.Format("{0}\\{1}", localPath, fileName);
					string srcTiffFullName = String.Format("{0}\\{1}", Config.FINISHED_TIF_PATH, p_dtlReq.strTiffPath);

					LogMessage(String.Format("TIF파일 다운로드({0}회) : {1} > {2}", cnt + 1, srcTiffFullName, localFileFullName));
					if (!File.Exists(srcTiffFullName))
					{
						LogError(String.Format("TIF파일 존재하지 않습니다. TIF파일 : {0}", srcTiffFullName));
						continue;
					}

					File.Copy(srcTiffFullName, localFileFullName, true);

					p_downTiffFullName = localFileFullName;
					ret = true;

					break;
				}
				catch (Exception ex)
				{
					LogError(String.Format("TIF파일을 다운로드중 다음과 같은 오류가 발생하였습니다 {0}", ex.Message));
					ret = false;

					continue;
				}
			}

			return ret;
		}

		private bool GetTiffFileNames(DbModule.SEND_REQUEST_DTL p_dtlReq, string p_localTiffFullName)
		{
			try
			{
				FileInfo fInfo = new FileInfo(p_localTiffFullName);
				p_dtlReq.strTiffName = fInfo.Name;
				p_dtlReq.strTiffFullName = fInfo.FullName;

				return true;
			}
			catch
			{
				return false;
			}
		}

		private bool GetTiffFileNames(DbModule.SEND_REQUEST_DTL p_dtlReq)
		{
			string strTiffFullName = "";
			strTiffFullName = String.Format("{0}\\{1}", Config.FINISHED_TIF_PATH, p_dtlReq.strTiffPath);
			if (!File.Exists(strTiffFullName))
				return false;
			try
			{
				FileInfo fInfo = new FileInfo(strTiffFullName);
				p_dtlReq.strTiffName = fInfo.Name;
				p_dtlReq.strTiffFullName = strTiffFullName;
				return true;
			}
			catch
			{
				return false;
			}
		}

		private long GetFileSize(string p_fileFullName)
		{
			long len = -1;

			FileInfo fInfo = null;
			for (int i = 0; i < 3; i++)
			{
				try
				{
					fInfo = new FileInfo(p_fileFullName);
					if (fInfo == null)
						continue;

					len = fInfo.Length;
					if (len > 0)
						break;

					Thread.Sleep(10);
				}
				catch
				{
					fInfo = null;
					len = -1;
				}
			}

			return len;
		}

		private int GetPageCount(string p_fileFullName)
		{
			int nCnt = -1;
			for (int i = 0; i < 3; i++)
			{
				try
				{

					Bitmap img = Bitmap.FromFile(p_fileFullName) as Bitmap;
					if (img == null)
					{
						Thread.Sleep(10);
						continue;
					}

					nCnt = img.GetFrameCount(FrameDimension.Page);
					if (nCnt > 0)
					{
						img.Dispose();
						break;
					}

					img.Dispose();
				}
				catch (Exception ex)
				{
					LogError(ex.Message);
					nCnt = -1;
				}
			}

			return nCnt;
		}


































		private void LogWarring(string p_strMsg)
		{
			LogWrite(LOG_LEVEL.WRN, p_strMsg);
		}

		private void LogError(string p_strMsg)
		{
			LogWrite(LOG_LEVEL.ERR, p_strMsg);
		}

		private void LogMessage(string p_strMsg)
		{
			LogWrite(LOG_LEVEL.MSG, p_strMsg);
		}

		private void LogWrite(LOG_LEVEL p_logLevel, string p_strMsg)
		{
			p_strMsg = String.Format("[TH:{0:D02}]{1}", base.ThreadNo, p_strMsg);
			AppLog.Write(p_logLevel, p_strMsg);
		}




        #endregion

        #region delegate
        public delegate void delegate_DbPollingCheck(string p_strDot);
        #endregion
        
        #region field
        private List<DbModule.SEND_REQUEST_PSC> m_lstPostProcessingReqs = new List<DbModule.SEND_REQUEST_PSC>();

		//// 팩스상세건 처리 성공건 리스트 ////
		List<DbModule.SEND_REQUEST_DTL_PSC> m_lstSuccReqDtls = new List<DbModule.SEND_REQUEST_DTL_PSC>();
        #endregion
    }
}
