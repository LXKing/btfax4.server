using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Mail;
using System.Net;
using PostFid;
using Btfax.CommonLib.Log;

namespace PostFid.Util
{
    class MailHelper
    {
        public static bool SendMail(string p_emailTO)
        {
            bool result = false;

            using (MailMessage mail = new MailMessage())
            {
                mail.From = new MailAddress(Config.SMTP_FROM);     // BTFAX 관리자 이메일 주소
                mail.To.Add(new MailAddress(p_emailTO));           // BTF_FAX_USER_MSTR.EMAIL_NOTIFY 컬럼의 이메일 주소

                mail.Subject = Config.SMTP_SUBJECT;                // 제목
                mail.Body = Config.SMTP_SUBJECT;                   // 수신메세지
                mail.SubjectEncoding = Encoding.UTF8;
                mail.BodyEncoding = Encoding.UTF8;

                using (SmtpClient smtpClient = new SmtpClient())
                {
                    smtpClient.Host = Config.SMTP_IP;           // SMTP Server IP
                    smtpClient.Port = Config.SMTP_PORT;         // SMTP Server Port

                    smtpClient.UseDefaultCredentials = Config.SMTP_USEDEFAULTCREDENTIALS;   // 기본인증 사용 여부
                    smtpClient.EnableSsl = Config.SMTP_ENABLESSL;                           // 보안 연결 (SSL) 사용 여부
                    smtpClient.DeliveryMethod = SmtpDeliveryMethod.Network;                 // 
                    //smtpClient.Credentials = new NetworkCredential(id, pw);               // SMTP Server 인증 ID / PW
                    smtpClient.Timeout = Config.SMTP_TIMEOUT;                               // 연결 제한시간

                    try
                    {
                        smtpClient.Send(mail);
                        result = true;
                    }
                    catch (Exception ex)
                    {
                        AppLog.ExceptionLog(ex, string.Join("\t", "MailHelper()", "SendMail()"));
                    }
                }
            }

            return result;
        }
    }
}
