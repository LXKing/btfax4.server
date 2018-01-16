using System;
using System.Collections.Generic;
using System.Net;
using System.Text;

namespace FaxAgent.Server
{
    class SocketListenerSettings
    {
        #region constructor
        private SocketListenerSettings()
        {
        }

        internal SocketListenerSettings(Int32 maxConnections,
                                        Int32 backlog,
                                        Int32 maxSimultaneousAcceptOps,
                                        Int32 receiveBufferSize,
                                        Int32 listenPort)
        {
            Int32 excessSaeaObjectsInPool = 1;      // SocketAsyncEventArgs 예비 개수
            Int32 receivePrefixLength = 1 + 5;      // 요청전문-파싱하기위한 최소 바이트수 (STX1자리 + 전문길이5자리)
            Int32 sendPrefixLength = 1 + 5;         // 응답전문-파싱하기위한 최소 바이트수 (STX1자리 + 전문길이5자리)
            Int32 opsToPreAlloc = 2;                // 소켓 버퍼 종류 (1 for receive, 1 for send)

            this.MaxConnections = maxConnections;
            this.NumberOfSaeaForRecSend = maxConnections + excessSaeaObjectsInPool;
            this.Backlog = backlog;
            this.MaxAcceptOps = maxSimultaneousAcceptOps;
            this.ReceivePrefixLength = receivePrefixLength;
            this.BufferSize = receiveBufferSize;
            this.SendPrefixLength = sendPrefixLength;
            this.OpsToPreAllocate = opsToPreAlloc;
            this.LocalEndPoint = new IPEndPoint(IPAddress.Any, listenPort);
        }
        #endregion

        #region field
        internal Int32 MaxConnections
        {
            get;
            private set;
        }

        // SAEA 객체 수
        internal Int32 NumberOfSaeaForRecSend
        {
            get;
            private set;
        }

        // listen queue
        internal Int32 Backlog
        {
            get;
            private set;
        }

        // Accept SAEA 객체 수
        internal Int32 MaxAcceptOps
        {
            get;
            private set;
        }

        // 소켓 버퍼 크기
        internal Int32 BufferSize
        {
            get;
            private set;
        }

        // 전문 파싱 전 받을 최소 크기
        internal Int32 ReceivePrefixLength
        {
            get;
            private set;
        }

        // 전문 파싱 전 받을 최소 크기
        internal Int32 SendPrefixLength
        {
            get;
            private set;
        }

        // 버퍼 종류 수
        internal Int32 OpsToPreAllocate
        {
            get;
            private set;
        }

        // 로컬 주소
        internal IPEndPoint LocalEndPoint
        {
            get;
            private set;
        }
        #endregion
    }    
}
