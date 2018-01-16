using System;
using System.Collections.Generic;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Threading;
using System.Data;
using DevExpress.XtraPrinting;
using DevExpress.XtraReports.UI;
using DevExpress.XtraReports.Btfax;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using Btfax.CommonLib;
using TiffMaker.Db;
using TiffMaker.Util;

namespace TiffMaker.Threading
{
    class MakingTiffThread : BtfaxThread
    {
        #region constant
        static readonly int QUEUE_POLLING_SLEEP = 500; // 0.5초
        #endregion

        #region constructor
        public MakingTiffThread()
        {   
            this.exportOption = new ImageExportOptions();
            this.exportOption.Format = ImageFormat.Tiff;
            this.exportOption.ExportMode = ImageExportMode.SingleFilePageByPage;
            this.exportOption.Resolution = 196;
        }
        #endregion

        #region override
        public override bool StartThread()
        {
            LogMessage("TIFF생성 스레드 시작");
            return base.StartThread();
        }

        public override bool JoinThread(int p_nTimeout = -1)
        {
            LogMessage("TIFF생성 스레드 종료");
            return base.JoinThread(p_nTimeout);
        }

        protected override void ThreadEntry()
        {
            while (!BtfaxThread.Stop)
            {
                //// 작업 요청 건 큐 폴링 ////
                ClearTmkSendReqInfo();
                m_tmkReqInfo = OutFaxQueue.Instance.Dequeue();
                if (m_tmkReqInfo == null || m_tmkReqInfo.faxId < 0)
                {   
                    System.Threading.Thread.Sleep(QUEUE_POLLING_SLEEP);
                    continue;
                }

                //// 다음 프로세스 유형 SET ////
                P_TYPE nextProcessType = P_TYPE.NONE;
                switch (Convert.ToInt32(m_tmkReqInfo.strReqType))
                {
                    case (int)REQ_TYPE.TMK:         nextProcessType = P_TYPE.PSC; break;
                    case (int)REQ_TYPE.TMK_TPC:     nextProcessType = P_TYPE.TPC; break;
                    case (int)REQ_TYPE.TMK_DCV_TPC: nextProcessType = P_TYPE.DCV; break;
                }

                LogMessage(string.Format("[FAX_ID:{0}] 팩스요청건점유", m_tmkReqInfo.faxId), RESULT.SUCCESS);
                
                //// TIF 생성정보 GET ////
                RESULT result = DbModule.Instance.GetTiffMakingInfo(ref m_tmkReqInfo);
                if (result != RESULT.SUCCESS)
                {   
                    LogError(string.Format("[FAX_ID:{0}] TIF생성 정보를 얻어오지 못하였습니다", m_tmkReqInfo.faxId), result);

                    //// MSTR 실패처리 ////
                    if (DbModule.Instance.FailSendReqMaster(m_tmkReqInfo.faxId, P_TYPE.TMK, Config.SYSTEM_PROCESS_ID) < 0)
                        LogError(string.Format("[FAX_ID:{0}] 작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId));
                    
                    //// DTL 전체실패처리 ////
                    if (DbModule.Instance.FailSendReqDetail(m_tmkReqInfo.faxId, -1, P_TYPE.TMK, Config.SYSTEM_PROCESS_ID, result) < 0)
                        LogError(string.Format("[FAX_ID:{0}] 작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId));

                    continue;
                }

                bool docInfoExistInDb = true;
                DbModule.DOC_INFO_ALL_TMK docInfoAll = m_tmkReqInfo.m_tmkDocInfoAll;
                if (docInfoAll == null)
                {
                    docInfoExistInDb = false;

                    m_tmkDocInfoAll.Clear();
                    docInfoAll = m_tmkDocInfoAll;
                    docInfoAll.strProcessingMode = ((int)DOC_PROCESSING_MODE.TMK).ToString();
                    docInfoAll.strTiffExtractPages = "all";
                }
                docInfoAll.strDocExt = "TIF";
                docInfoAll.strDocPath = ".";

                string strSrcFaxFormFullName = string.Format("{0}\\{1}{2}\\{3}", Config.STG_HOME_PATH
                                                                            , Config.FAXFORM_PATH
                                                                            , m_tmkReqInfo.m_strFaxFormPath
                                                                            , m_tmkReqInfo.m_strFaxFormFileName);
                
                string strLocalFaxFormFullName = string.Format("{0}\\{1}.fax", Config.TIFF_MAKING_PATH
                                                                            , m_tmkReqInfo.faxId);
                
                //// 팩스폼 다운로드 ////
                result = DownloadFaxForm(strSrcFaxFormFullName, strLocalFaxFormFullName);
                if (result != RESULT.SUCCESS)
                {   
                    //// MSTR 실패처리 ////
                    if (DbModule.Instance.FailSendReqMaster(m_tmkReqInfo.faxId, P_TYPE.TMK, Config.SYSTEM_PROCESS_ID) < 0)
                        LogError(string.Format("[FAX_ID:{0}] 작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId));

                    //// DTL 전체실패처리 ////
                    if (DbModule.Instance.FailSendReqDetail(m_tmkReqInfo.faxId, -1, P_TYPE.TMK, Config.SYSTEM_PROCESS_ID, result) < 0)
                        LogError(string.Format("[FAX_ID:{0}] 작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId));

                    continue;
                }

                LogMessage(string.Format("[FAX_ID:{0}] 팩스폼다운로드성공 (파일:{1} -> {2})", m_tmkReqInfo.faxId, strSrcFaxFormFullName, strLocalFaxFormFullName), result);

                foreach (DbModule.SEND_REQUEST_DTL_TMK tmkDtlReqInfo in m_tmkReqInfo.m_lstTmkSendRequestDtlInfos)
                {   
                    //// TIFF생성 - 실패시 3회처리 ////
                    result = RESULT.EMPTY;
                    for (int i = 0; i < 3; i++)
                    {
                        result = MakingTiff(m_tmkReqInfo.faxId,
                                            m_tmkReqInfo.strTrNo,
                                            m_tmkReqInfo.m_strPacket,
                                            strLocalFaxFormFullName,
                                            strSrcFaxFormFullName,
                                            tmkDtlReqInfo,
                                            docInfoAll,
                                            nextProcessType);

                        if (result == RESULT.SUCCESS)
                            break;

                        LogError(string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}] TIF파일 생성실패 (횟수:{2})", m_tmkReqInfo.faxId, tmkDtlReqInfo.faxDtlId, i + 1), result);
                        Thread.Sleep(500);
                    }

                    if (result != RESULT.SUCCESS)
                    {   
                        if (DbModule.Instance.FailSendReqDetail(tmkDtlReqInfo.faxId, tmkDtlReqInfo.faxDtlId, P_TYPE.TMK, Config.SYSTEM_PROCESS_ID, result) < 0)
                            LogError(string.Format("[FAX_ID:{0}] 작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId), RESULT.F_DB_UPDATE_ERROR);

                        continue;
                    }

                    //// DTL성공 처리 ////
                    if (DbModule.Instance.SuccessSendReqDetail(m_tmkReqInfo.faxId, tmkDtlReqInfo.faxDtlId, P_TYPE.TMK, Config.SYSTEM_PROCESS_ID) < 0)
                    {
                        LogError(string.Format("[FAX_ID:{0}] 작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId), RESULT.F_DB_UPDATE_ERROR);
                        continue;
                    }

                    //// 성공건 추가 ////
                    m_lstSuccessTmkDtlReqs.Add(tmkDtlReqInfo);

                    Thread.Sleep(100);
                }

                
                File.Delete(strLocalFaxFormFullName);

                if (m_lstSuccessTmkDtlReqs.Count <= 0)
                {
                    
                    result = RESULT.F_MAKE_IMAGEFILE_NOT_EXIST;
                    LogError(string.Format("[FAX_ID:{0}] TIF생성 성공건이 존재하지 않습니다", m_tmkReqInfo.faxId), result);

                    if (DbModule.Instance.FailSendReqMaster(m_tmkReqInfo.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID) < 0)
                        LogError(string.Format("[FAX_ID:{0}] 작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId));

                    continue;
                }

                if (docInfoExistInDb)
                {
                    //// 생성 TIFF DB 에 업데이트 ////
                    if (!DbModule.Instance.FinishProcessingDoc(Config.PROCESS_TYPE, docInfoAll.seq))
                    {
                        LogError(string.Format("[FAX_ID:{0}] TIF파일 생성정보를 DB에 UPDATE 하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId), RESULT.F_DB_UPDATE_ERROR);
                        continue;
                    }
                }
                else
                {
                    //// 생성 TIFF DB 에 추가 ////
                    if (!DbModule.Instance.InsertSendDoc(m_tmkReqInfo.faxId, Config.PROCESS_TYPE, ".", docInfoAll))
                    {
                        LogError("생성TIF 정보를 DB에 INSERT 하는 도중 오류가 발생하였습니다", RESULT.F_DB_UPDATE_ERROR);
                        continue;
                    }
                }

                //// 다음 프로세스로 전달 - BTF_FAX_SEND_DTL ////
                if (DbModule.Instance.PassoverDetailSendReq(m_tmkReqInfo.faxId, -1, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, nextProcessType, "N") <= 0)
                {
                    LogError(string.Format("[FAX_ID:{0}] 작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId), RESULT.F_DB_UPDATE_ERROR);
                    continue;
                }

                //// 다음 프로세스로 전달 - BTF_FAX_SEND_MSTR ////
                if (DbModule.Instance.PassoverMasterSendReq(m_tmkReqInfo.faxId, Config.PROCESS_TYPE, Config.SYSTEM_PROCESS_ID, nextProcessType) <= 0)
                {
                    LogError(string.Format("[FAX_ID:{0}] 작업완료 상태를 DB에 반영하는 도중 오류가 발생하였습니다", m_tmkReqInfo.faxId), RESULT.F_DB_UPDATE_ERROR);
                    continue;
                }

                //// 로그 기록 ////
                LogMessage(string.Format("[FAX_ID:{0}] TIF파일 생성작업 완료", m_tmkReqInfo.faxId, strSrcFaxFormFullName, strLocalFaxFormFullName), RESULT.SUCCESS);

                Thread.Sleep(QUEUE_POLLING_SLEEP);
            }
        }
        #endregion

        private RESULT DownloadFaxForm(string p_strSrc, string p_strDest)
        {            
            RESULT ret = RESULT.EMPTY;
            for (int i = 0; i < 3; i++)
            {
                try
                {
                    if (!File.Exists(p_strSrc))
                    {   
                        LogError(string.Format("팩스폼 파일을 찾을수 없습니다. 파일:{0}", p_strSrc), RESULT.F_FILE_FAIL_TO_DOWNLOAD);
                        Thread.Sleep(200);
                        continue;
                    }

                    File.Copy(p_strSrc, p_strDest, true);
                    ret = RESULT.SUCCESS;
                    break;
                }
                catch(Exception ex)
                {   
                    ret = RESULT.F_FILE_FAIL_TO_DOWNLOAD;
                    LogError(string.Format("팩스폼 파일을 다운로드중 다음과 같은 오류가 발생하였습니다. 파일:{0} 오류:{1}", p_strSrc, ex.Message), ret);
                    
                    LogError(ex.Message);                    
                    Thread.Sleep(200);
                }
            }

            return ret;
        }

        private bool UploadFinishedTif(string p_strSrc, string p_strDest)
        {
            int idx = p_strDest.LastIndexOf("\\");
            if (idx < 0)
                return false;

            string strFinishedPath = p_strDest.Substring(0, idx + 1);

            bool ret = false;
            for (int i = 0; i < 3 ; i++)
            {
                try
                {
                    //// FINISHED 디렉터리 생성 YYYY_MM\DD ////
                    if (!Directory.Exists(strFinishedPath))
                        Directory.CreateDirectory(strFinishedPath);

                    File.Copy(p_strSrc, p_strDest, true);
                    ret = true;
                    break;
                }
                catch (Exception ex)
                {
                    ret = false;
                    LogError(ex.Message);
                    Thread.Sleep(500);
                    continue;
                }
            }

            return ret;
        }

        private RESULT MakingTiff(decimal p_faxId, 
                                    string p_strTrNo, 
                                    string p_strPacket, 
                                    string p_strFaxFormLocal,
                                    string p_strFaxFormFullPath,
                                    DbModule.SEND_REQUEST_DTL_TMK p_tmkDtlInfo, 
                                    DbModule.DOC_INFO_ALL_TMK p_docInfo,
                                    P_TYPE p_destProcessType)
        {   
            RESULT result = RESULT.EMPTY;

			// ADD - KIMCG : 20140903
			// 팩스번호 복호화 처리 
			if (Config.ENCRYPT_FIELD_YN == "Y")
			{
				string strDecrytedFaxNo = "";
				if (EncryptApi.Decrypt(p_tmkDtlInfo.strFaxNo, out strDecrytedFaxNo))
					p_tmkDtlInfo.strFaxNo = strDecrytedFaxNo;
				else
					LogWrite(LOG_LEVEL.WRN, "팩스번호 복호화 처리 실패", RESULT.EMPTY);
			}
			// ADD - END
				
            if (p_destProcessType == P_TYPE.PSC)
                p_docInfo.strDocFile = string.Format("{0}_{1}", p_faxId, p_tmkDtlInfo.faxDtlId);
            else
                p_docInfo.strDocFile = string.Format("{0}_{1}_tifmake", p_faxId, p_tmkDtlInfo.faxDtlId);

            string strFileName = string.Format("{0}.{1}", p_docInfo.strDocFile, p_docInfo.strDocExt);

            //// 파일경로이름 SET ////
            string strFinishFile = "";
            string strMakingFile = string.Format(@"{0}\{1}", Config.TIFF_MAKING_PATH, strFileName);
            if (p_destProcessType == P_TYPE.PSC)
                strFinishFile = string.Format(@"{0}\{1}", Config.FINISHED_TIF_PATH, p_tmkDtlInfo.strTiffPath);
            else
                strFinishFile = string.Format(@"{0}\{1}", Config.MADE_TIF_PATH, strFileName);
            string strPacketXmlFile = string.Format(@"{0}\{1}_{2}.xml", Config.TIFF_MAKING_PATH, p_faxId, p_tmkDtlInfo.faxDtlId);

            //// TIFF 생성 ////
            try
            {
                //// 팩스폼 로딩 ////
                if (!File.Exists(p_strFaxFormLocal))
                {
                    LogError(string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}] 팩스폼파일을 찾을수 없습니다.(파일:{2})", p_faxId, p_tmkDtlInfo.faxDtlId, p_strFaxFormLocal), RESULT.F_MAKE_NOT_EXIST_FAXFORMFILE);
                    return RESULT.F_MAKE_NOT_EXIST_FAXFORMFILE;
                }

                //// Page fault 처리 - 스트림 생성 ////
                using (MemoryStream faxFormStream = new MemoryStream())
                {
                    using (Stream localFaxFormStream = File.OpenRead(p_strFaxFormLocal))
                    {
                        localFaxFormStream.CopyTo(faxFormStream);
                        localFaxFormStream.Close();
                    }

                    using (XtraReport_Btfax report = new XtraReport_Btfax())
                    {
                        report.LoadLayout(p_strFaxFormLocal);
                        LogMessage(string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}] 팩스폼파일 로딩성공 (파일:{2})", p_faxId,p_tmkDtlInfo.faxDtlId, p_strFaxFormLocal), RESULT.SUCCESS);
                        
                        //// 전문 XML 로딩 ////
                        report.LoadPacketXml(p_strPacket, p_strTrNo, false, true);

                        //// 레포트 데이터셋에 팩스번호, 수신인명 바인딩 처리 ////
                        DataSet reportDs = report.DataSource as DataSet;
                        if (reportDs == null)
                        {
                            LogError(string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}] 팩스폼에 데이터셋이 존재하지 않습니다 (파일:{2})", p_faxId, p_tmkDtlInfo.faxDtlId, p_strFaxFormLocal), RESULT.F_MAKE);                            
                            return RESULT.F_MAKE;
                        }

                        DataTable reportDt = reportDs.Tables["헤더___HEADER_O_COMMON"];
                        if (reportDt == null)
                        {
                            LogError(string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}] 팩스폼에 데이터셋이 존재하지 않습니다 (파일:{2})", p_faxId, p_tmkDtlInfo.faxDtlId, p_strFaxFormLocal), RESULT.F_MAKE);                            
                            return RESULT.F_MAKE;
                        }

                        bool bFaxNumBinded = false;
                        bool bFaxRecipientNameBinded = false;
                        foreach (DataColumn col in reportDt.Columns)
                        {
                            foreach (DataRow row in reportDt.Rows)
                            {
                                if (bFaxNumBinded && bFaxRecipientNameBinded)
                                    break;

                                if (col.ColumnName == "수신자팩스번호")
                                {
                                    row[col.ColumnName] = p_tmkDtlInfo.strFaxNo;
                                    bFaxNumBinded = true;
                                }
                                if (col.ColumnName == "수신자이름")
                                {
                                    row[col.ColumnName] = p_tmkDtlInfo.strRecipientName;
                                    bFaxRecipientNameBinded = true;
                                }
                            }
                        }

                        //// 컨트롤 바인딩 처리 ////
                        foreach (Band band in report.Bands)
                        {
                            foreach (XRControl control in band.Controls)
                            {
                                //// 이미지 외, 컨트롤 바인딩 유효성 체크 ////
                                if (control is XRLabel || control is XRCheckBox || control is XRTableCell)
                                {
                                    result = ValidateBinding(report, control);
                                    if (result != RESULT.SUCCESS)
                                        return result;
                                }

                                //// 이미지 바인딩 유효성 체크 및 동적 바인딩 ////
                                else if (control is XRPictureBox)
                                {
                                    XRPictureBox xrPictureBox = control as XRPictureBox;

                                    result = BindImageBox(report, xrPictureBox);
                                    if (result != RESULT.SUCCESS)
                                        throw new Exception();
                                }
                            }
                        }

                        //// Export ///
                        //report.ExportToImage(strMakingFile, this.exportOption);
                        if (!report.ExportImageForFax(Config.TIFF_MAKING_PATH, strFileName))
                        {
                            LogError(string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}] TIF파일 생성실패 (파일:{2})", p_faxId, p_tmkDtlInfo.faxDtlId, strMakingFile), RESULT.F_MAKE);
                            return RESULT.F_MAKE;
                        }

                        LogMessage(string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}] TIF파일 생성성공 (파일:{2})", p_faxId, p_tmkDtlInfo.faxDtlId, strMakingFile), RESULT.SUCCESS);

                        //// 파일 업로드 ////
                        if (!UploadFinishedTif(strMakingFile, strFinishFile))
                        {   
                            LogError(string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}] TIF파일 업로드실패 (파일:{2}->{3})", p_faxId, p_tmkDtlInfo.faxDtlId, strMakingFile, strFinishFile), RESULT.F_FILE_FAIL_TO_UPLOAD);
                            return RESULT.F_FILE_FAIL_TO_UPLOAD;
                        }

                        LogMessage(string.Format("[FAX_ID:{0}][FAX_DTL_ID:{1}] TIF파일 업로드성공 (파일:{2}->{3})", p_faxId, p_tmkDtlInfo.faxDtlId, strMakingFile, strFinishFile), RESULT.SUCCESS);

                        if (reportDt != null)
                            reportDt.Dispose();

                        if (reportDs != null)
                            reportDs.Dispose();

                        // LHOPE
                        File.Delete(strMakingFile);
                    }

                    faxFormStream.Close();
                    return RESULT.SUCCESS;
                }
            }
            catch (Exception ex)
            {
                LogError( string.Format("팩스 발송 요청에 대하여 팩스폼[{0}]으로 TIFF 파일[{1}]을 생성하는 도중., 다음과 같은 오류가 발생하였습니다. {2}",
                            p_strFaxFormFullPath, strMakingFile, ex.Message), RESULT.F_MAKE);

                return RESULT.F_MAKE;
            }
        }

        protected RESULT ValidateBinding(XtraReport_Btfax p_report, XRControl p_xrControl)
        {
            if(p_xrControl.DataBindings.Count <= 0)
                return RESULT.SUCCESS;

            XRBinding xrBinding = p_xrControl.DataBindings[0];

            //// 바인딩 이름이 사용자 필드 존재여부 확인 ////
            foreach (CalculatedField calculateField in p_report.CalculatedFields)
            {
                string strBindingName = string.Format("{0}.{1}", calculateField.DataMember, calculateField.Name);
                if (strBindingName == xrBinding.DataMember)
                    return RESULT.SUCCESS;
            }

            //// 바인딩 이름이 전문필드 존재여부 확인 ////
            string strBindingData = p_report.GetPacketFieldData(xrBinding.DataMember);
            if (strBindingData != null)
                return RESULT.SUCCESS;
            
            //// 로그기록 - 바인딩 데이터 없음 ////
            LogError(string.Format("{0} 팩스폼에서 [{1}] 컨트롤의 바인딩정보[{2}]에 해당하는 전문필드를 얻지 못하였습니다.",
                    p_report.Name, p_xrControl.Name, xrBinding.DataMember), RESULT.F_MAKE_FAIL_TO_BINDING_DATA);

            return RESULT.F_MAKE_FAIL_TO_BINDING_DATA;
        }

        protected RESULT BindImageBox(XtraReport_Btfax p_report, XRPictureBox p_xrPictureBox)
        {
            if (p_xrPictureBox.DataBindings.Count <= 0)
                return RESULT.SUCCESS;

            XRBinding xrBinding = p_xrPictureBox.DataBindings[0];
            string strImageFile = p_report.GetPacketFieldData(xrBinding.DataMember);
            if (strImageFile == null)
            {
                LogError(string.Format("전문 XML 에서 이미지파일정보[{0}]을 얻지 못하였습니다.", xrBinding.DataMember), RESULT.F_MAKE_IMAGEFILE_INFO_NOT_IN_PACKET);
                return RESULT.F_MAKE_IMAGEFILE_INFO_NOT_IN_PACKET;
            }
            string strImagFullPath = string.Format("{0}\\{1}", Config.INPUT_DOCS_PATH, strImageFile);
            if (!File.Exists(strImagFullPath))
            {
                LogError(string.Format("[{0}] 파일이 존재하지 않습니다.", strImagFullPath), RESULT.F_MAKE_IMAGEFILE_NOT_EXIST);
                return RESULT.F_MAKE_IMAGEFILE_NOT_EXIST;
            }

            try
            {
                p_xrPictureBox.DataBindings.Clear();
                Image img = Image.FromFile(strImagFullPath);
                p_xrPictureBox.Image = img;
                int angle = p_xrPictureBox.Angle;
                p_xrPictureBox.Angle = 0;
                p_xrPictureBox.Angle = angle;
            }
            catch (Exception ex)
            {
                LogError(string.Format("이미지파일[{0}:{1}]을 열지 못하였습니다. {2}", xrBinding.DataMember, strImagFullPath, ex), RESULT.F_MAKE);
                return RESULT.F_MAKE_IMAGEFILE_NOT_EXIST;
            }

            return RESULT.SUCCESS;
        }

        private void ClearTmkSendReqInfo()
        {
            m_lstSuccessTmkDtlReqs.Clear();
            if (m_tmkReqInfo != null)
                m_tmkReqInfo.Clear();
            m_tmkReqInfo = null;
        }

        private void LogError(string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            LogWrite(LOG_LEVEL.ERR, p_strMsg, p_result);
        }

        private void LogMessage(string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            LogWrite(LOG_LEVEL.MSG, p_strMsg, p_result);
        }

        private void LogWrite(LOG_LEVEL p_logLevel, string p_strMsg, RESULT p_result = RESULT.EMPTY)
        {
            p_strMsg = String.Format("[TH:{0:D02}][RESULT:{1}] {2}", base.ThreadNo, p_result, p_strMsg);
            AppLog.Write(p_logLevel, p_strMsg);
        }

        #region fields
        protected List<DbModule.SEND_REQUEST_DTL_TMK> m_lstSuccessTmkDtlReqs = new List<DbModule.SEND_REQUEST_DTL_TMK>();
        DbModule.SEND_REQUEST_TMK m_tmkReqInfo = new DbModule.SEND_REQUEST_TMK();
        DbModule.DOC_INFO_ALL_TMK m_tmkDocInfoAll = new DbModule.DOC_INFO_ALL_TMK();
        ImageExportOptions exportOption = null;
        #endregion
    }
}
