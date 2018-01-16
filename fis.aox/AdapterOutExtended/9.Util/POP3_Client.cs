using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Security;
using System.Net.Sockets;
using System.Web;
using System.Windows.Forms;
using System.Text.RegularExpressions;
using System.IO;
using Btfax.CommonLib.Log;

namespace AdapterOutExtended
{
    class POP3_Client
    {
        TcpClient m_tcpClient;
        SslStream m_sslStream;
        int m_mailCount;
        byte[] m_buffer = new byte[8172];

        public bool IsConnected { get { return m_tcpClient == null ? false : m_tcpClient.Connected; } }

        public bool IsLogin = false;

        public POP3_Client()
        {
            m_mailCount = -1;
        }

        /// <summary>
        /// Connect to pop3 server "host" using "port" and auth SSL as client.
        /// </summary>
        /// <param name="host"></param>
        /// <param name="port"></param>
        /// <returns></returns>
        public bool Connect(string host, int port)
        {
            try
            {
                m_tcpClient = new TcpClient(host, port);
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, "Connect()");
                return false;
            }

            try
            {
                //m_tcpClient.Connect(host, port);
                m_sslStream = new SslStream(m_tcpClient.GetStream());
                m_sslStream.AuthenticateAsClient(host);
                // Read the stream to make sure we are connected
                int bytes = m_sslStream.Read(m_buffer, 0, m_buffer.Length);
                return (Encoding.ASCII.GetString(m_buffer, 0, bytes).Contains("+OK"));
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("Connect()에서 다음과 같은 오류가 발생하였습니다."));                
                return false;
            }
            
        }
        /// <summary>
        /// Closes SSL and TCP connections.
        /// </summary>
        public void Disconnect()
        {
            if (m_tcpClient.Connected)
                m_sslStream.Write(Encoding.ASCII.GetBytes("QUIT\r\n"));
            m_sslStream.Close();
            m_tcpClient.Close();

            this.IsLogin = false;
        }
        /// <summary>
        /// Logs into the pop3 server (when connected.)
        /// </summary>
        /// <param name="username"></param>
        /// <param name="password"></param>
        /// <returns></returns>
        public bool Login(string username, string password)
        {
            if (!m_tcpClient.Connected)
                return false;
            if (string.IsNullOrEmpty(username) || string.IsNullOrEmpty(password))
                throw new ArgumentException("Username or Password was empty.");
            int bytesRead = -1;
            //Send the users login details
            m_sslStream.Write(Encoding.ASCII.GetBytes("USER " + username + "\r\n"));
            bytesRead = m_sslStream.Read(m_buffer, 0, m_buffer.Length);
            if (!Encoding.ASCII.GetString(m_buffer, 0, bytesRead).Contains("+OK"))
                return false;
            //Send the password                        
            m_sslStream.Write(Encoding.ASCII.GetBytes("PASS " + password + "\r\n"));
            bytesRead = m_sslStream.Read(m_buffer, 0, m_buffer.Length);
            if (!Encoding.ASCII.GetString(m_buffer, 0, bytesRead).Contains("+OK"))
                return false;

            this.IsLogin = true;
            return true;
        }

        public bool DeleteMessage(int messageNumber)
        {
            m_sslStream.Write(Encoding.ASCII.GetBytes("DELE " + messageNumber.ToString() + "\r\n"));
            int bytesRead = m_sslStream.Read(m_buffer, 0, m_buffer.Length);
            if (!Encoding.ASCII.GetString(m_buffer, 0, bytesRead).Contains("+OK"))
                return false;
            return true;
        }

        public bool ResetDeleteMessage()
        {
            m_sslStream.Write(Encoding.ASCII.GetBytes("RSET \r\n"));
            int bytesRead = m_sslStream.Read(m_buffer, 0, m_buffer.Length);
            if (!Encoding.ASCII.GetString(m_buffer, 0, bytesRead).Contains("+OK"))
                return false;
            return true;
        }

        /// <summary>
        /// Returns the number of emails in the inbox.
        /// </summary>
        /// <returns></returns>
        public int GetMailCount()
        {
            m_sslStream.Write(Encoding.ASCII.GetBytes("STAT\r\n"));
            int bytesRead = m_sslStream.Read(m_buffer, 0, m_buffer.Length);
            string data = Encoding.ASCII.GetString(m_buffer, 0, bytesRead);
            //char [] str = Encoding.ASCII.GetChars(m_buffer);
            //string data = str.ToString();
            if (data.Contains("+OK"))
            {
                data = data.Remove(0, 4);
                string[] temp = data.Split(' ');
                int r;
                if (Int32.TryParse(temp[0], out r))
                {
                    m_mailCount = r;
                    return r;
                }
            }
            return -1;
        }

        /// <summary>
        /// Returns the message data as a string.
        /// </summary>
        /// <param name="messageNumber"></param>
        /// <returns></returns>
        private string RetrieveMessage(int messageNumber)
        {
            string ret = "";
            m_sslStream.Write(Encoding.ASCII.GetBytes("RETR " + messageNumber.ToString() + "\r\n"));
            int bytesRead = -1;
            while (bytesRead != 0 && !ret.Contains("\r\n.\r\n"))
            {
                bytesRead = m_sslStream.Read(m_buffer, 0, m_buffer.Length);
                ret += Encoding.ASCII.GetString(m_buffer, 0, bytesRead);
            }
            ret = ret.Remove(0, ret.IndexOf("\r\n") + 2);
            return ret;
        }

        public POP3_Parsing GetMessage(int messageNumber)
        {
            POP3_Parsing parse = new POP3_Parsing();
            try
            {
                parse.MailMessage(RetrieveMessage(messageNumber));

                parse.ParseHeader();
                parse.ParseBody();
                parse.ConvertStringBoundaryBody();

                parse.IsParseSuccess = true;

                AppLog.Write(LOG_LEVEL.MSG, string.Format("[ Subject: {0} ] 메일을 수신 완료하였습니다.", parse.Subject));

                #region For Debug
                //AppLog.Write(LOG_LEVEL.TRC, "----------------------------------------------------------------------------------------------------");
                //AppLog.Write(LOG_LEVEL.TRC, parse.Subject);

                //foreach (POP3_BoundaryInfo info in parse.BoundaryInfoList)
                //{
                //    AppLog.IsDisplay = false;
                //    AppLog.Write(LOG_LEVEL.TRC, "----------------------------------------------------------------------------------------------------");
                //    AppLog.Write(LOG_LEVEL.TRC, "Convert Body                     : " + info.ContentType);
                //    AppLog.Write(LOG_LEVEL.TRC, "CharSet                          : " + info.ContentTypeCharSet);
                //    AppLog.Write(LOG_LEVEL.TRC, "ContentTransferEncoding          : " + info.ContentTransferEncoding);
                //    AppLog.Write(LOG_LEVEL.TRC, "ConvertBody                      : " + info.ConvertBody);
                //    AppLog.Write(LOG_LEVEL.TRC, "ContentTypeName                  : " + info.ContentTypeName);
                //    AppLog.Write(LOG_LEVEL.TRC, "ContentDispositionAttachFileName : " + info.ContentDispositionAttachFileName);
                //    AppLog.IsDisplay = true;
                //}
                //AppLog.Write(LOG_LEVEL.TRC, " ");
                //AppLog.Write(LOG_LEVEL.TRC, " ");
                //AppLog.Write(LOG_LEVEL.TRC, " ");
                //AppLog.Write(LOG_LEVEL.TRC, " ");
                //AppLog.Write(LOG_LEVEL.TRC, " ");
                //AppLog.Write(LOG_LEVEL.TRC, " ");
                #endregion
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, string.Format("[ Subject:{0} ] 메일을 수신 실패하였습니다.", parse.Subject));
            }
            return parse;
        }
    }
}