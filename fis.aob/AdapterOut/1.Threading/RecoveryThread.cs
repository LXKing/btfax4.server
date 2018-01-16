using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading;
using Btfax.CommonLib;
using Btfax.CommonLib.Util;
using Btfax.CommonLib.Threading;
using Btfax.CommonLib.Log;
using AdapterOut.Util;
using AdapterOut.PacketParsing;
using AdapterOut.Db;

namespace AdapterOut.Threading
{
    class RecoveryThread : TcpSessionThread
    {
        #region static
        private static bool s_isRunning = false;

        private static RecoveryThread s_instance = null;
        public static RecoveryThread Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new RecoveryThread();

                return s_instance;
            }
        }
        #endregion

        #region overriding
        public override bool StartThread()
        {
            //// 리커버리 스레드 처리중 ////
            if (s_isRunning)
            {
                WriteMsg("현재 리커버리 작업을 수행중입니다");
                return true;
            }

            s_isRunning = true;
            m_raiseSystemErrorTime = DateTime.Now;
            WriteMsg("시스템장애로인해 리커버리 작업을 시작합니다");
            return base.StartThread();
        }

        public override bool JoinThread(int p_nTimeout = -1)
        {
            s_isRunning = false;
            bool ret = base.JoinThread(1000);
            WriteMsg("리커버리 작업을 종료합니다.");

            return ret;
        }

        protected override void ThreadEntry()
        {
            while (s_isRunning)
            {
                Thread.Sleep(1000 * 10);
                
                //// NFS 체크 : 패킷 레이아웃 파일 검증 ////
                if (!base.m_parser.Load())
                {   
                    WriteErr(string.Format("{0}에 접근할 수 없습니다. 메시지:{1}", m_parser.LayoutPathFile, m_parser.ReasonMsg));
                    continue;
                }

                //// DB 체크 ////
                if (!DbModule.Instance.ReOpen())
                {
                    WriteErr(string.Format("데이터베이스에 접근할 수 없습니다. 메시지:{0}", Config.DB_CONNECTION_STRING));
                    continue;
                }

                //// 요청건 수집 ////
                string recoveryPath = string.Format("{0}\\recovery\\", System.Windows.Forms.Application.StartupPath);
                string[] fileNames = Directory.GetFiles(recoveryPath, "*.recovery");
                
                WriteMsg(string.Format("리커버리 작업을 수행합니다. 리커버리 건수:{0}", fileNames.Length));
                if (fileNames.Length > 0)
                {
                    //// 팩스 발송 요청 ////
                    for (int i = 0; i < fileNames.Length; i++)
                    {
                        Thread.Sleep(1000);

                        string fileName = fileNames[i];
                        if (!File.Exists(fileName))
                        {
                            WriteErr(string.Format("리커버리 파일이 존재하지 않습니다. 파일명:{0}", fileName));
                            continue;
                        }

                        WriteMsg(string.Format("리커버리({0})에 대한 팩스요청을 진행합니다. ", fileName));
                        string workFileFullName = fileName + ".open";
                        string succFileFullName = fileName + ".succ";
                        string failFileFullName = fileName + ".fail";
                        string overFileFullName = fileName + ".over";

                        //// 장애 발생시점 부터 10분(컨피그) 내외의 파일들에 한에서만 진행함 ////
                        FileInfo srcFInfo = new FileInfo(fileName);
                        if (srcFInfo.CreationTime > m_raiseSystemErrorTime.AddMinutes(Config.RECOVERY_BASE_TIME))
                        {
                            WriteMsg(string.Format("리커버리 시간({0}분)이 종료된 파일입니다. 파일명:{1} ", Config.RECOVERY_BASE_TIME, fileName));
                            ReplaceFile(fileName, overFileFullName, true);
                            continue;
                        }

                        ReplaceFile(fileName, workFileFullName, true);

                        string strPacket = "";
                        using (FileStream fs = File.OpenRead(workFileFullName))
                        {
                            using (StreamReader reader = new StreamReader(fs, Encoding.Default))
                            {
                                strPacket = reader.ReadToEnd();
                                reader.Close();
                            }
                        }

                        if (string.IsNullOrEmpty(strPacket))
                        {
                            WriteErr(string.Format("리커버리 파일에 데이터가 존재하지 않습니다. 파일명:{0}", workFileFullName));
                            ReplaceFile(workFileFullName, failFileFullName, true);
                            continue;
                        }

                        //// 전문 길이 얻기 ////
                        int packetLen = 0;
                        string strPacketLen = strPacket.Substring(Config.SIZE_FIELD_POS, Config.SIZE_FIELD_LEN);
                        if (!Int32.TryParse(strPacketLen, out packetLen))
                        {
                            WriteErr(string.Format("전문길이 필드 Convert 오류 size:{0}", strPacketLen));
                            ReplaceFile(workFileFullName, failFileFullName, true);
                            continue;
                        }

                        //// 버퍼에 데이터복사 ////
                        base.m_buffer = Encoding.Default.GetBytes(strPacket);

                        //// SET - 전문번호 ////
                        string strTrNo = Encoding.Default.GetString(base.m_buffer, Config.TRNO_FIELD_POS, Config.TRNO_FIELD_LEN);
                        strTrNo = strTrNo.TrimEnd(" ".ToCharArray());
                        if (string.IsNullOrEmpty(strTrNo))
                        {
                            WriteErr(string.Format("리커버리 파일에 전문번호 필드를 찾을수 없습니다. 파일명:{0}", workFileFullName));
                            ReplaceFile(workFileFullName, failFileFullName, true);
                            continue;
                        }

                        base.SetTrNo_(strTrNo);

                        //// 전문처리 ////
                        RESULT ret = base.ProcessPacket_(strTrNo, base.m_buffer);
                        if (ret != RESULT.SUCCESS)
                        {
                            WriteErr(string.Format("리커버리 파일 팩스요청중 오류가 발생하였습니다. 파일명:{0} 오류:{1}", workFileFullName, ret));
                            ReplaceFile(workFileFullName, failFileFullName, true);
                            continue;
                        }

                        File.Delete(workFileFullName);
                        //ReplaceFile(workFileFullName, succFileFullName, true);
                        WriteMsg(string.Format("리커버리({0})에 대한 팩스요청을 완료하였습니다.", fileName));
                    }
                }

                break;
            }

            JoinThread();
        }

        #endregion

        private void ReplaceFile(string p_srcFileFullName, string p_destFileFullName, bool p_deleteSource)
        {
            try
            {
                File.Copy(p_srcFileFullName, p_destFileFullName, true);
                if (p_deleteSource)
                    File.Delete(p_srcFileFullName);
            }
            catch(Exception ex)
            {
                WriteErr(ex.Message);
            }
        }

        private void WriteMsg(string p_msg)
        {
            AppLog.Write(LOG_LEVEL.MSG, string.Format("[RECOVERY] {0}", p_msg));
        }

        private void WriteErr(string p_msg)
        {
            AppLog.Write(LOG_LEVEL.ERR, string.Format("[RECOVERY] {0}", p_msg));
        }

        #region fields
        private DateTime m_raiseSystemErrorTime;
        #endregion
    }
}
