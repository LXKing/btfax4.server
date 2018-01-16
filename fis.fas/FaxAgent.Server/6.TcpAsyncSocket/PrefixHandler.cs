using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;

namespace FaxAgent.Server
{
    class PrefixHandler
    {
        internal Int32 HandlePrefix(SocketAsyncEventArgs e, DataHoldingUserToken receiveSendToken, Int32 remainingBytesToProcess)
        {
			lock (this)
			{
				if (receiveSendToken.receivedPrefixBytesDoneCount == 0)
					receiveSendToken.byteArrayForPrefix = new Byte[receiveSendToken.receivePrefixLength];

				if (remainingBytesToProcess >= receiveSendToken.receivePrefixLength - receiveSendToken.receivedPrefixBytesDoneCount)
				{
					//SocketListener.SocketListenerFileLog.WriteLine("PrefixHandler, enough for prefix " + receiveSendToken.TokenId + ". remainingBytesToProcess = " + remainingBytesToProcess);

					Buffer.BlockCopy(e.Buffer,
									 receiveSendToken.receiveMessageOffset - receiveSendToken.receivePrefixLength + receiveSendToken.receivedPrefixBytesDoneCount,
									 receiveSendToken.byteArrayForPrefix,
									 receiveSendToken.receivedPrefixBytesDoneCount,
									 receiveSendToken.receivePrefixLength - receiveSendToken.receivedPrefixBytesDoneCount);

					receiveSendToken.recPrefixBytesDoneThisOp = receiveSendToken.receivePrefixLength - receiveSendToken.receivedPrefixBytesDoneCount;

					receiveSendToken.receivedPrefixBytesDoneCount = receiveSendToken.receivePrefixLength;

					receiveSendToken.lengthOfCurrentIncomingMessage = int.Parse(Encoding.Default.GetString(receiveSendToken.byteArrayForPrefix, 1, receiveSendToken.receivePrefixLength - 1));
				}
				else
				{
					//SocketListener.SocketListenerFileLog.WriteLine("PrefixHandler, NOT all of prefix " + receiveSendToken.TokenId + ". remainingBytesToProcess = " + remainingBytesToProcess);

					Buffer.BlockCopy(e.Buffer,
									 receiveSendToken.receiveMessageOffset - receiveSendToken.receivePrefixLength + receiveSendToken.receivedPrefixBytesDoneCount,
									 receiveSendToken.byteArrayForPrefix,
									 receiveSendToken.receivedPrefixBytesDoneCount,
									 remainingBytesToProcess);

					receiveSendToken.recPrefixBytesDoneThisOp = remainingBytesToProcess;
					receiveSendToken.receivedPrefixBytesDoneCount += remainingBytesToProcess;
					remainingBytesToProcess = 0;
				}

				if (remainingBytesToProcess == 0)
				{
					receiveSendToken.receiveMessageOffset = receiveSendToken.receiveMessageOffset - receiveSendToken.recPrefixBytesDoneThisOp;
					receiveSendToken.recPrefixBytesDoneThisOp = 0;
				}
			}
            return remainingBytesToProcess;
        }
    }
}
