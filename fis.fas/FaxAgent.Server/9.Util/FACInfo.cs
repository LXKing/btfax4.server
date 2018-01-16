using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace FaxAgent.Server
{
    class FACInfo
    {
        public Exception LastException;     // 마지막 예외
        public DateTime LastRequestTime;    // 마지막 요청시간
        public DateTime LastResponseTime;   // 마지막 응답시간

        //public DateTime LastUdpRequestTime;
        //public DateTime LastUdpResponseTime;
        
        public Socket Socket;               // Accept Socket
        public EndPoint LocalEndPoint { get { return this.Socket.LocalEndPoint; } }
        public EndPoint RemoteEndPoint { get { return this.Socket.RemoteEndPoint; } }

        public string USER_ID;
        public string USER_NAME;

        public SocketAsyncEventArgs recvEventArgs;
        public SocketAsyncEventArgs sendEventArgs;

        public FACInfo()
        {
            Socket = null;
            USER_ID = "";
            USER_NAME = "";
            recvEventArgs = null;
            sendEventArgs = null;

            LastException = null;
            LastRequestTime = DateTime.Now;
            LastResponseTime = DateTime.Now;
        }
    }
}
