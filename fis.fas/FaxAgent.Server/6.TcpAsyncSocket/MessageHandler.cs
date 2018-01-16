using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;

namespace FaxAgent.Server
{
    class MessageHandler
    {
        internal bool HandleMessage(SocketAsyncEventArgs receiveSendEventArgs, DataHoldingUserToken receiveSendToken, Int32 remainingBytesToProcess)
        {	
			bool incomingTcpMessageIsReady = false;
			lock (this)
			{

				if (receiveSendToken.receivedMessageBytesDoneCount == 0)
				{
					//SocketListener.Log.Write("MessageHandler, creating receive array " + receiveSendToken.TokenId);

					receiveSendToken.dataToReceive = new Byte[receiveSendToken.lengthOfCurrentIncomingMessage];
				}

				if (remainingBytesToProcess + receiveSendToken.receivedMessageBytesDoneCount == receiveSendToken.lengthOfCurrentIncomingMessage)
				{
					//SocketListener.Log.Write("MessageHandler, length is right for " + receiveSendToken.TokenId);

					Buffer.BlockCopy(receiveSendEventArgs.Buffer,
									 receiveSendToken.receiveMessageOffset - receiveSendToken.receivePrefixLength,
									 receiveSendToken.dataToReceive,
									 receiveSendToken.receivedMessageBytesDoneCount,
									 remainingBytesToProcess);

					incomingTcpMessageIsReady = true;
				}
				else
				{
					try
					{
						Buffer.BlockCopy(receiveSendEventArgs.Buffer,
										 receiveSendToken.receiveMessageOffset,
										 receiveSendToken.dataToReceive,
										 receiveSendToken.receivedMessageBytesDoneCount,
										 remainingBytesToProcess);

					}
					catch (Exception ex)
					{
						SocketListener.Log.WRN("MessageHandler, length is short for " + receiveSendToken.TokenId);
						SocketListener.Log.WRN(string.Format("receiveMessageOffset:{0}, receivedMessageBytesDoneCount:{1}, remainingBytesToProcess:{2}"
																									, receiveSendToken.receiveMessageOffset
																									, receiveSendToken.receivedMessageBytesDoneCount
																									, remainingBytesToProcess));
						SocketListener.Log.WRN(ex.Message);

					}

					receiveSendToken.receiveMessageOffset = receiveSendToken.receiveMessageOffset - receiveSendToken.recPrefixBytesDoneThisOp;
					receiveSendToken.receivedMessageBytesDoneCount += remainingBytesToProcess;
				}

				//else if (remainingBytesToProcess + receiveSendToken.receivedMessageBytesDoneCount < receiveSendToken.lengthOfCurrentIncomingMessage)
				//{
				//    SocketListener.Log.Write("MessageHandler, length is short for " + receiveSendToken.TokenId);

				//    try
				//    {
				//        Buffer.BlockCopy(receiveSendEventArgs.Buffer,
				//                         receiveSendToken.receiveMessageOffset,
				//                         receiveSendToken.dataToReceive,
				//                         receiveSendToken.receivedMessageBytesDoneCount,
				//                         remainingBytesToProcess);

				//    }
				//    catch (Exception ex)
				//    {
				//        SocketListener.Log.Write("MessageHandler, length is short for " + receiveSendToken.TokenId +
				//            ", " + ex.ToString());
				//    }

				//    receiveSendToken.receiveMessageOffset = receiveSendToken.receiveMessageOffset - receiveSendToken.recPrefixBytesDoneThisOp;
				//    receiveSendToken.receivedMessageBytesDoneCount += remainingBytesToProcess;

				//    incomingTcpMessageIsReady = true;
				//}
				//else
				//{
				//    SocketListener.Log.Write("MessageHandler, length is large for " + receiveSendToken.TokenId);

				//    try
				//    {
				//        Buffer.BlockCopy(receiveSendEventArgs.Buffer,
				//                        receiveSendToken.receiveMessageOffset - receiveSendToken.receivePrefixLength,
				//                        receiveSendToken.dataToReceive,
				//                        receiveSendToken.receivedMessageBytesDoneCount,
				//                        receiveSendToken.lengthOfCurrentIncomingMessage);
				//    }
				//    catch (Exception ex)
				//    {
				//        SocketListener.Log.Write("MessageHandler, length is large for " + receiveSendToken.TokenId +
				//            ", " + ex.ToString());
				//    }

				//    incomingTcpMessageIsReady = true;
				//}
			}

            return incomingTcpMessageIsReady;
        }
    }
}
