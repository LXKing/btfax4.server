using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text; //for testing

namespace FaxAgent.Server
{
    class DataHoldingUserToken
    {
        #region constructor
        private DataHoldingUserToken()
        {
        }

        internal DataHoldingUserToken(SocketAsyncEventArgs e, Int32 rOffset, Int32 sOffset, Int32 receivePrefixLength, Int32 sendPrefixLength, Int32 identifier)
        {
            this.idOfThisObject = identifier;

			this.BufferOffsetRecv = rOffset;
            this.bufferOffsetReceive = rOffset;

			this.BufferOffsetSend = sOffset;
            this.bufferOffsetSend = sOffset;
            this.receivePrefixLength = receivePrefixLength;
            this.sendPrefixLength = sendPrefixLength;
            this.receiveMessageOffset = rOffset + receivePrefixLength;
            this.permanentReceiveMessageOffset = this.receiveMessageOffset;
        }
        #endregion

        #region method
        internal void CreateNewDataHolder()
        {
            dataToReceive = null;
        }

        internal void CreateSessionId()
        {	
            sessionId = Interlocked.Increment(ref SocketListener.MainSessionId);
        }

        internal void Reset()
        {
            this.receivedPrefixBytesDoneCount = 0;
            this.receivedMessageBytesDoneCount = 0;
            this.recPrefixBytesDoneThisOp = 0;
            this.receiveMessageOffset = this.permanentReceiveMessageOffset;
        }
        #endregion

        #region field
        internal Byte[] dataToReceive;

        internal Int32 socketHandleNumber;

        internal readonly Int32 bufferOffsetReceive;
        internal readonly Int32 permanentReceiveMessageOffset;
        internal readonly Int32 bufferOffsetSend;

        private Int32 idOfThisObject;           //for testing only        

        internal Int32 lengthOfCurrentIncomingMessage;

        internal Int32 receiveMessageOffset;
        internal Byte[] byteArrayForPrefix;
        internal readonly Int32 receivePrefixLength;
        internal Int32 receivedPrefixBytesDoneCount = 0;
        internal Int32 receivedMessageBytesDoneCount = 0;
        
        internal Int32 recPrefixBytesDoneThisOp = 0;

        internal Int32 sendBytesRemainingCount;
        internal readonly Int32 sendPrefixLength;
        internal Byte[] dataToSend;
        internal Int32 bytesSentAlreadyCount;
        
        private Int32 sessionId;

        internal Int32 TokenId { get { return this.idOfThisObject; } }

        internal Int32 SessionId { get { return this.sessionId; } }

		public string RemoteIp { get; set; }
		public int RemotePort { get; set; }

		public IPEndPoint IpEndPt { get; set; }

		public int BufferOffsetRecv { get; set; }
		public int BufferOffsetSend { get; set; }
		
        #endregion
    }
}
