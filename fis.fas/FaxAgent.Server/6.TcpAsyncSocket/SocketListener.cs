using System;
using System.IO;
using System.Collections.Generic; //for testing
using System.Net.Sockets;
using System.Threading; //for Semaphore and Interlocked
using System.Net;
using System.Text; //for testing
using System.Diagnostics; //for testing
using Btfax.CommonLib.Log;
using FaxAgent.Common.Socket;

namespace FaxAgent.Server
{
	class SocketListener
    {
        #region constructor
        internal SocketListener()
        {
            Log = new SocketListenerLog(Btfax.CommonLib.Log.LOG_LEVEL.MSG);    // SocketListener ���� �α�

            theProcess = Process.GetCurrentProcess();

            //SocketListener.Log.Write("SocketListener constructor");
            LogWriteWithThreadInfo("SocketListener constructor");

            /// ���ڵ� ����
            m_Encoding = Encoding.Default;

            this.socketListenerSettings = new SocketListenerSettings(Config.MAX_CONENCTION,
                                                                     Config.BACK_LOG,
                                                                     Config.MAX_ACCEPT_OPERATION,
                                                                     Config.BUFFER_SIZE,
                                                                     Config.LISTEN_PORT);
            this.numberOfAcceptedSockets = 0;

            this.prefixHandler = new PrefixHandler();
            this.messageHandler = new MessageHandler();

            /// ���� ����
            this.theBufferManager = new BufferManager(this.socketListenerSettings.BufferSize * this.socketListenerSettings.OpsToPreAllocate * this.socketListenerSettings.NumberOfSaeaForRecSend,
                                                      this.socketListenerSettings.BufferSize);

            this.poolOfAcceptEventArgs = new SocketAsyncEventArgsPool(this.socketListenerSettings.MaxAcceptOps);
            this.poolOfRecvEventArgs = new SocketAsyncEventArgsPool(this.socketListenerSettings.NumberOfSaeaForRecSend);
            this.poolOfSendEventArgs = new SocketAsyncEventArgsPool(this.socketListenerSettings.NumberOfSaeaForRecSend);

            /// ���� ���� ����
            this.theMaxConnectionsEnforcer = new Semaphore(this.socketListenerSettings.MaxConnections,
                                                           this.socketListenerSettings.MaxConnections);

            /// ���� �м� - ó��
            this.messageParsing = new MessageParsing();

            /// �ʱ�ȭ
            this.Init();
            this.StartListen();

            /// �ߺ� �α��� �߻� ó��
            messageParsing.OnLoginDuplicated += new MessageParsing.StringEventHandler(messageParsing_OnLoginDuplicated);

            /// SERVER_LIMIT_RESPONSE_TIME ��ŭ alive ó��
            AliveCheckSocketsThread = new Thread(this.AliveCheckSockets_ThreadEntry);
            AliveCheckSocketsThread.IsBackground = true;
            AliveCheckSocketsThread.Start();
        }
        #endregion

        #region constructor - Init
        private void Init()
        {
            LogWriteWithThreadInfo("SocketListener.Init()");

            /// Accept SAEA ��ü ����
            SocketListener.Log.Write("Starting creation of accept SocketAsyncEventArgs pool:");
            for (Int32 i = 0; i < this.socketListenerSettings.MaxAcceptOps; i++)
            {
                this.poolOfAcceptEventArgs.Push(CreateNewSaeaForAccept(poolOfAcceptEventArgs));
            }
            SocketListener.Log.Write("Finishing creation of accept SocketAsyncEventArgs pool");

            /// Receive, Send SAEA ��ü ����
            SocketListener.Log.Write("Starting creation of receive/send SocketAsyncEventArgs pool:");
            theBufferManager.InitBuffer();     /// ���۸� ũ�� �ϳ��� �Ҵ� (�޸� ����ȭ ���� �ذ�)
            CreateNewSaeaForRecvSend();
            SocketListener.Log.Write("Finishing creation of receive/send SocketAsyncEventArgs pool");
        }

        private void StartListen()
        {
            LogWriteWithThreadInfo("SocketListener.StartListen()");

            listenSocket = new Socket(this.socketListenerSettings.LocalEndPoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
            listenSocket.Bind(this.socketListenerSettings.LocalEndPoint);
            listenSocket.Listen(this.socketListenerSettings.Backlog);

            this.StartAccept();
        }

        /// <summary>
        /// Accept SAEA ��ü ����
        /// </summary>
        private SocketAsyncEventArgs CreateNewSaeaForAccept(SocketAsyncEventArgsPool pool)
        {
            SocketAsyncEventArgs acceptEventArg = new SocketAsyncEventArgs();

            acceptEventArg.Completed += new EventHandler<SocketAsyncEventArgs>(AcceptEventArg_Completed);

            AcceptOpUserToken theAcceptOpToken = new AcceptOpUserToken(pool.AssignTokenId() + 10000);
            acceptEventArg.UserToken = theAcceptOpToken;

            return acceptEventArg;
        }

		private void CreateNewSaeaForRecvSendOne()
		{
			Int32 tokenId;
			SocketAsyncEventArgs recvSAEAObjectForPool;
			SocketAsyncEventArgs sendSAEAObjectForPool;
			recvSAEAObjectForPool = new SocketAsyncEventArgs();
			sendSAEAObjectForPool = new SocketAsyncEventArgs();

			this.theBufferManager.SetBuffer(recvSAEAObjectForPool, sendSAEAObjectForPool);
			

			recvSAEAObjectForPool.Completed += new EventHandler<SocketAsyncEventArgs>(IO_Completed);
			sendSAEAObjectForPool.Completed += new EventHandler<SocketAsyncEventArgs>(IO_Completed);

			poolOfSendEventArgs.AssignTokenId();
			tokenId = poolOfRecvEventArgs.AssignTokenId() + 1000000;

			DataHoldingUserToken theTempRecvUserToken = new DataHoldingUserToken(recvSAEAObjectForPool,
																				 recvSAEAObjectForPool.Offset,
																				 sendSAEAObjectForPool.Offset,
																				 this.socketListenerSettings.ReceivePrefixLength,
																				 this.socketListenerSettings.SendPrefixLength,
																				 tokenId);
			theTempRecvUserToken.CreateNewDataHolder();
			recvSAEAObjectForPool.UserToken = theTempRecvUserToken;
			sendSAEAObjectForPool.UserToken = theTempRecvUserToken;

			this.poolOfRecvEventArgs.Push(recvSAEAObjectForPool);
			this.poolOfSendEventArgs.Push(sendSAEAObjectForPool);
		}

        /// <summary>
        /// Receive, Send SAEA ��ü ����
        /// </summary>
        private void CreateNewSaeaForRecvSend()
        {
			//Int32 tokenId;
			//SocketAsyncEventArgs recvSAEAObjectForPool;
			//SocketAsyncEventArgs sendSAEAObjectForPool;

            for (Int32 i = 0; i < this.socketListenerSettings.NumberOfSaeaForRecSend; i++)
            {
				CreateNewSaeaForRecvSendOne();
            }
            SocketListener.Log.Write("SocketListener.CreateNewSaeaForRecvSend() CreateNewSaea count : " + this.socketListenerSettings.NumberOfSaeaForRecSend + " * 2");
        }
        #endregion

        #region method - TCP Operation - Accept
        private void StartAccept()
        {
			try
			{
				Thread.Sleep(1);
				SocketAsyncEventArgs acceptEventArg;

				// ����� 1���� ���ܵ�
				if (this.poolOfAcceptEventArgs.Count > 1)
				{
					try
					{
						acceptEventArg = this.poolOfAcceptEventArgs.Pop();						
					}
					catch
					{
						acceptEventArg = CreateNewSaeaForAccept(poolOfAcceptEventArgs);
					}
				}
				else
				{
					acceptEventArg = CreateNewSaeaForAccept(poolOfAcceptEventArgs);
				}

				/// �ִ� ������ ����
				this.theMaxConnectionsEnforcer.WaitOne();

				/// Accept ����
				/// true  : Accept �񵿱� ������
				/// false : Accept ���� ó���� (Accept ��)
				SocketListener.Log.MSG("Start AcceptAsync !!");
				bool willRaiseEvent = listenSocket.AcceptAsync(acceptEventArg);
				if (!willRaiseEvent)
				{
					SocketListener.Log.WRN("StartAccept in if (!willRaiseEvent), accept token id " + (acceptEventArg.UserToken as AcceptOpUserToken).TokenId);
					ProcessAccept(acceptEventArg);
				}
			}
			catch(Exception ex)
			{
				SocketListener.Log.ERR(string.Format("StartAccept - {0}", ex.Message));
				this.StartAccept();
			}
        }

        /// <summary>
        /// Accept �񵿱� �Ϸ�� ȣ��
        /// </summary>
        private void AcceptEventArg_Completed(object sender, SocketAsyncEventArgs e)
        {
			if (!ValidationCheckSocketAsyncEventArgs("AcceptEventArg_Completed", e))
			{
				SocketListener.Log.WRN("AcceptEventArg_Completed : ValidationCheckSocketAsyncEventArgs fail.");
				this.HandleBadAccept(e);
				this.StartAccept();
				return;
			}

			IPEndPoint remoteIpEndPoint = e.AcceptSocket.RemoteEndPoint as IPEndPoint;
			if(remoteIpEndPoint == null)
			{
				SocketListener.Log.ERR("AcceptEventArg_Completed : SocketAsyncEventArgs invalid remoteIpEndPoint.");
				this.HandleBadAccept(e);
				this.StartAccept();
				return;
			}

			string strRemoteIp = remoteIpEndPoint.Address.ToString();
			int nReportPort = remoteIpEndPoint.Port;

			AcceptOpUserToken userToken = e.UserToken as AcceptOpUserToken;
			userToken.RemoteIp = remoteIpEndPoint.Address.ToString();
			userToken.RemotePort = remoteIpEndPoint.Port;
			userToken.IpEndPt = remoteIpEndPoint;

			SocketListener.Log.MSG(e, "AcceptEvent completed.");
            ProcessAccept(e);
        }

		private bool ValidationCheckSocketAsyncEventArgs(string strFuncName, SocketAsyncEventArgs e)
		{
			string strRemoteIp = "";
			int nRemotePort = 0;
			int nToken = 0;

			DataHoldingUserToken dToken = e.UserToken as DataHoldingUserToken;
			if (dToken != null)
			{
				strRemoteIp = dToken.RemoteIp;
				nRemotePort = dToken.RemotePort;
				nToken = dToken.TokenId;
			}
			else
			{
				AcceptOpUserToken aToken = e.UserToken as AcceptOpUserToken;
				strRemoteIp = aToken.RemoteIp;
				nRemotePort = aToken.RemotePort;
				nToken = aToken.TokenId;
			}

			if (e.AcceptSocket == null)
			{
				SocketListener.Log.ERR(string.Format("{0,-15}:{1}/{2}|{3} - AcceptSocket is null."
										, strRemoteIp
										, nRemotePort
										, nToken
										, strFuncName
										));
				return false;
			}

			//IPEndPoint remoteIpEndPoint = e.AcceptSocket.RemoteEndPoint as IPEndPoint;
			//if (remoteIpEndPoint == null)
			//{
			//    SocketListener.Log.ERR(string.Format("{0,-15}:{1}/{2}|{3} - RemoteIpEndPoint is null."
			//                            , strRemoteIp
			//                            , nRemotePort
			//                            , nToken
			//                            , strFuncName
			//                            ));
			//    return false;
			//}

			SocketError sockErr = e.SocketError;
			if (sockErr != SocketError.Success)
			{
				SocketListener.Log.ERR(string.Format("{0,-15}:{1}/{2}|{3} - Socket error : {4}."
										, strRemoteIp
										, nRemotePort
										, nToken
										, strFuncName
										, sockErr
										));
				
				return false;
			}

			return true;
		}

		private void InitSocketAsyncEventArgs(ref SocketAsyncEventArgs args)
		{
			args.AcceptSocket = null;
			//args.BufferList = null;
			//args.DisconnectReuseSocket = false;
			args.RemoteEndPoint = null;
			args.SocketError = SocketError.Success;
			//args.SocketFlags = SocketFlags.None;
		}

        private void ProcessAccept(SocketAsyncEventArgs acceptEventArgs)
        {
			// �̺�Ʈ �˻�
			if (!ValidationCheckSocketAsyncEventArgs("ProcessAccept", acceptEventArgs))
			{
				this.StartAccept();
				this.HandleBadAccept(acceptEventArgs);
				return;
			}

			// �ִ� ������ �� ����
			Int32 max = this.maxSimultaneousClientsThatWereConnected;
			Int32 numberOfConnectedSockets = Interlocked.Increment(ref this.numberOfAcceptedSockets);
			if (numberOfConnectedSockets > max)
			{
			    Interlocked.Increment(ref this.maxSimultaneousClientsThatWereConnected);
			}

			// ���� ���� �̺�Ʈ ��� ó�� : acceptEventArgs -> recvEventArgs, sendEventArgs
			StartAccept();

			SocketListener.Log.MSG(acceptEventArgs, string.Format("Start accept processing !! poolOfAcceptEventArgs:{0}, poolOfRecvEventArgs:{1}, poolOfSendEventArgs:{2}"
																, this.poolOfAcceptEventArgs.Count
																, this.poolOfRecvEventArgs.Count
																, this.poolOfSendEventArgs.Count));

			try
			{
				// Accept �� Client �� �߰� : acceptEventArgs -> recvEventArgs, sendEventArgs
				if (this.poolOfRecvEventArgs.Count <= 1 || this.poolOfSendEventArgs.Count <= 1)
					this.CreateNewSaeaForRecvSendOne();

				AcceptOpUserToken aToken = acceptEventArgs.UserToken as AcceptOpUserToken;
				SocketAsyncEventArgs recvEventArgs = null;
				SocketAsyncEventArgs sendEventArgs = null;

				while (true)
				{
					recvEventArgs = this.poolOfRecvEventArgs.Pop();
					sendEventArgs = this.poolOfSendEventArgs.Pop();
					
					if (recvEventArgs.Buffer == null || sendEventArgs.Buffer == null)
					{	
						DataHoldingUserToken dToken = recvEventArgs.UserToken as DataHoldingUserToken;
						if (dToken != null)
						{
							this.theBufferManager.SetBuffer(dToken.BufferOffsetRecv, recvEventArgs);
							SocketListener.Log.WRN(string.Format("Receive SocketAsyncEventArgs buffer is null. reset buffer. Reuse later!! token:{0}, token_offset:{1}, recv_offset:{2}", dToken.TokenId, dToken.BufferOffsetRecv, recvEventArgs.Offset));
						}
						
						dToken = sendEventArgs.UserToken as DataHoldingUserToken;
						if (dToken != null)
						{
							this.theBufferManager.SetBuffer(dToken.BufferOffsetSend, sendEventArgs);
							SocketListener.Log.WRN(string.Format("Send SocketAsyncEventArgs buffer is null. reset buffer. Reuse later!! token:{0}, token_offset:{1}, send_offset:{2}", dToken.TokenId, dToken.BufferOffsetSend, sendEventArgs.Offset));
						}
						
						this.poolOfRecvEventArgs.Push(recvEventArgs);
						this.poolOfSendEventArgs.Push(sendEventArgs);
						continue;
					}

					break;
				}

				InitSocketAsyncEventArgs(ref recvEventArgs);
				InitSocketAsyncEventArgs(ref sendEventArgs);

				SocketListener.Log.MSG(string.Format("Start accept processing !! SocketAsyncEventArgs buffer offset. recv_offset:{0}, send_offset:{1}", recvEventArgs.Offset, sendEventArgs.Offset));

				(recvEventArgs.UserToken as DataHoldingUserToken).CreateSessionId();
				(recvEventArgs.UserToken as DataHoldingUserToken).RemoteIp = aToken.RemoteIp;
				(recvEventArgs.UserToken as DataHoldingUserToken).RemotePort = aToken.RemotePort;
				(recvEventArgs.UserToken as DataHoldingUserToken).IpEndPt = aToken.IpEndPt;
				
				(sendEventArgs.UserToken as DataHoldingUserToken).CreateSessionId();
				(sendEventArgs.UserToken as DataHoldingUserToken).RemoteIp = aToken.RemoteIp;
				(sendEventArgs.UserToken as DataHoldingUserToken).RemotePort = aToken.RemotePort;
				(sendEventArgs.UserToken as DataHoldingUserToken).IpEndPt = aToken.IpEndPt;
				

				recvEventArgs.AcceptSocket = acceptEventArgs.AcceptSocket;
				//recvEventArgs.SocketError = acceptEventArgs.SocketError;

				sendEventArgs.AcceptSocket = acceptEventArgs.AcceptSocket;
				//sendEventArgs.SocketError = sendEventArgs.SocketError;

				(recvEventArgs.UserToken as DataHoldingUserToken).socketHandleNumber = (Int32)recvEventArgs.AcceptSocket.Handle;
				(sendEventArgs.UserToken as DataHoldingUserToken).socketHandleNumber = (Int32)sendEventArgs.AcceptSocket.Handle;

				// �̺�Ʈ �ݳ�
				aToken.RemoteIp = "";
				aToken.RemotePort = 0;
				aToken.IpEndPt = null;
				acceptEventArgs.AcceptSocket = null;
				this.poolOfAcceptEventArgs.Push(acceptEventArgs);

				string strMsg;
				FACContainer.AddEx(recvEventArgs.AcceptSocket, recvEventArgs, sendEventArgs, out strMsg);
				if (!string.IsNullOrEmpty(strMsg))
					SocketListener.Log.WRN(recvEventArgs, string.Format("FACContainer AddEx - {0}!!", strMsg));

				this.DisplayConnectionInfo();

				/// Accept �˸� ������
				SendMessageToClient(sendEventArgs, MessagePacketNameEnum.ACCPT);
				
				/// ���� ���
				StartReceive(recvEventArgs);
			}
			catch(Exception ex)
			{
				// REMOVE - KIMCG : 20140827
				// Exception �߻��� ��� ȣ������� Accept pool ���� �̺�Ʈ�� ������. -> ���� �̺�Ʈ �������� �۾��Ұ���.
				// StartAccept();
				// HandleBadAccept(acceptEventArgs);
				// REMOVE - END
				SocketListener.Log.ERR(string.Format("ProcessAccept - message:{0}\r\nstac_trace:{1}\r\nsource:{2}",ex.Message, ex.StackTrace, ex.Source));
			}
        }
        #endregion

        #region method - TCP Operation - Receive, Send
        /// <summary>
        /// Receive, Send �񵿱� �Ϸ�� ȣ��
        /// </summary>
        private void IO_Completed(object sender, SocketAsyncEventArgs e)
		{	
			try
			{
				// �̺�Ʈ �˻�
				if (!ValidationCheckSocketAsyncEventArgs("IO_Completed", e))
				{
					// heartbit �������� ���� �Ǿ������� �̹� Close ��.
					this.CloseClientSocket(e);
					return;
				}

				switch (e.LastOperation)
				{
					case SocketAsyncOperation.Receive:
						SocketListener.Log.TRC(e, "method in Receive");
						ProcessReceive(e);
						break;

					case SocketAsyncOperation.Send:
						SocketListener.Log.TRC(e, "method in send");
						ProcessSend(e);
						break;

					default:
						SocketListener.Log.MSG(e, "Not defined SocketAsyncOperation");
						break;
				}
			}
			catch(Exception ex)
			{	
				SocketListener.Log.ERR(string.Format("IO_Completed : {0}\r\n{1}\r\n{2}", ex.Message, ex.Source, ex.StackTrace));
			}
		}

        private void StartReceive(SocketAsyncEventArgs recvEventArgs)
        {	
			// �̺�Ʈ �˻�
			if (!ValidationCheckSocketAsyncEventArgs("StartReceive", recvEventArgs))
				return;

			try
			{
				DataHoldingUserToken recvToken = recvEventArgs.UserToken as DataHoldingUserToken;
				recvEventArgs.SetBuffer(recvToken.bufferOffsetReceive, this.socketListenerSettings.BufferSize);

				/// Receive ����
				/// true  : Receive �񵿱� ������
				/// false : Receive ���� ó���� (Receive ��)
				bool willRaiseEvent = recvEventArgs.AcceptSocket.ReceiveAsync(recvEventArgs);
				if (!willRaiseEvent)
				{
					SocketListener.Log.WRN(recvEventArgs, "StartReceive in if (!willRaiseEvent).");
					//Thread.Sleep(50);
					//ProcessReceive(recvEventArgs);
				}
			}
			catch(Exception ex)
			{
				SocketListener.Log.ERR(recvEventArgs, string.Format("StartReceive : {0} ", ex.Message));
				SocketAsyncOperation lastOper = recvEventArgs.LastOperation;
				SocketListener.Log.ERR(recvEventArgs, string.Format("SocketAsyncOperation - {0}!!", recvEventArgs.LastOperation.ToString()));
				SocketListener.Log.ERR(recvEventArgs, string.Format("SocketFlags - {0}!!", recvEventArgs.SocketFlags.ToString()));

				//Thread.Sleep(50);
				//StartReceive(recvEventArgs);
			}
        }

        private void ProcessReceive(SocketAsyncEventArgs recvEventArgs)
        {
			DataHoldingUserToken recvToken = recvEventArgs.UserToken as DataHoldingUserToken;

			// �̺�Ʈ �˻�
			if (!ValidationCheckSocketAsyncEventArgs("ProcessReceive", recvEventArgs))
			{
				recvToken.Reset();
				this.CloseClientSocket(recvEventArgs);
				return;
			}

			try
			{
				if (recvEventArgs.BytesTransferred == 0)
				{
					SocketListener.Log.WRN(recvEventArgs, "RecvEventArgs.BytesTransferred is zero. !!");
					recvToken.Reset();
					this.CloseClientSocket(recvEventArgs);
					return;
				}

				FACInfo facInfo = FACContainer.FindFACInfo(recvEventArgs);
				if (facInfo == null)
				{
					SocketListener.Log.ERR(recvEventArgs, "Not found facinfo.");
					recvToken.Reset();
					return;
				}

				facInfo.LastRequestTime = DateTime.Now;

				Int32 remainingBytesToProcess = recvEventArgs.BytesTransferred;

				if (recvToken.receivedPrefixBytesDoneCount < this.socketListenerSettings.ReceivePrefixLength)
				{
					/// ���� ���� ù �÷� (��������) �б�
					remainingBytesToProcess = prefixHandler.HandlePrefix(recvEventArgs, recvToken, remainingBytesToProcess);
					if (remainingBytesToProcess == 0)
					{
						SocketListener.Log.WRN(recvEventArgs, "RemainingBytesToProcess is zero. !!");
						StartReceive(recvEventArgs);
						return;
					}
				}

				// �������̸�ŭ ���� �б�
				bool incomingTcpMessageIsReady = messageHandler.HandleMessage(recvEventArgs, recvToken, remainingBytesToProcess);
				if (incomingTcpMessageIsReady == true)
				{
					SocketAsyncEventArgs sendEventArgs = facInfo.sendEventArgs;
					if (recvEventArgs == null)
					{
						recvToken.Reset();
						//CloseClientSocket(recvEventArgs);
						return;
					}

					LOG_LEVEL logLv = LOG_LEVEL.MSG;
					string packetName = m_Encoding.GetString(recvToken.dataToReceive, 34, 5);
					if (packetName == MessagePacketNameEnum.ALIVE.ToString())
						logLv = LOG_LEVEL.TRC;
					else
						logLv = LOG_LEVEL.MSG;

					// ��Ŷ �α�
					SocketListener.Log.LogWrite(logLv,
												"RECEIVE ",
												recvEventArgs.AcceptSocket.RemoteEndPoint.ToString(),
												recvEventArgs.AcceptSocket.Handle.ToInt32(),
												recvToken.TokenId,
												recvToken.dataToReceive.Length,
												m_Encoding.GetString(recvToken.dataToReceive));

					// ���� ���� ������
					this.SendMessageToClient(sendEventArgs);
				}
				else
				{
					recvToken.receiveMessageOffset = recvToken.bufferOffsetReceive;
					recvToken.recPrefixBytesDoneThisOp = 0;
				}

				StartReceive(recvEventArgs);
			}
			catch(Exception ex)
			{
				SocketListener.Log.ERR(recvEventArgs, string.Format("ProcessReceive : {0}", ex.Message));
				recvToken.Reset();				
			}
        }

        private void StartSend(SocketAsyncEventArgs sendEventArgs)
        {	
			try
			{
				// �̺�Ʈ �˻�
				if (!ValidationCheckSocketAsyncEventArgs("StartSend", sendEventArgs))
					return;

				int nRemainingCnt = 0;
				DataHoldingUserToken sendToken = sendEventArgs.UserToken as DataHoldingUserToken;
				nRemainingCnt = sendToken.sendBytesRemainingCount;
				if (nRemainingCnt <= 0)
				{
					SocketListener.Log.TRC(sendEventArgs, string.Format("StartSend : Invalid DataHoldingUserToken sendBytesRemainingCount count({0}). ", sendToken.sendBytesRemainingCount));
					return;
				}

				if (nRemainingCnt <= this.socketListenerSettings.BufferSize)
					nRemainingCnt = sendToken.sendBytesRemainingCount;
				else
					nRemainingCnt = this.socketListenerSettings.BufferSize;

				sendEventArgs.SetBuffer(sendToken.bufferOffsetSend, nRemainingCnt);

				Buffer.BlockCopy(sendToken.dataToSend,
								 sendToken.bytesSentAlreadyCount,
								 sendEventArgs.Buffer,
								 sendToken.bufferOffsetSend,
								 nRemainingCnt);

				// ��ȯ ��:
				//     I/O �۾��� ���� ���� ��� true�� ��ȯ�մϴ�.�۾��� �Ϸ�Ǹ� e �Ű� ������ ���� System.Net.Sockets.SocketAsyncEventArgs.Completed
				//     �̺�Ʈ�� �߻��մϴ�.I/O �۾��� ���������� �Ϸ�� ��� false�� ��ȯ�մϴ�.�� ��쿡�� e �Ű� �������� System.Net.Sockets.SocketAsyncEventArgs.Completed
				//     �̺�Ʈ�� �߻����� ������, �޼��� ȣ���� ��ȯ�� ���� �Ű� ������ ���޵� e ��ü�� �˻��Ͽ� �۾� ����� �˻��� �� �ֽ��ϴ�.
				bool willRaiseEvent = sendEventArgs.AcceptSocket.SendAsync(sendEventArgs);
				if (!willRaiseEvent)
				{	
					SocketListener.Log.WRN(sendEventArgs, "StartSend in if (!willRaiseEvent).");
					//Thread.Sleep(50);
					//ProcessSend(sendEventArgs);
				}
			}
			catch(Exception ex)
			{
				SocketListener.Log.ERR(sendEventArgs, string.Format("StartSend : {0}\r\n{1}. ", ex.Message, ex.StackTrace));
			}
        }

        private void ProcessSend(SocketAsyncEventArgs sendEventArgs)
        {	
			int nRemainingCnt = 0;

			// �̺�Ʈ �˻�
			if (!ValidationCheckSocketAsyncEventArgs("ProcessSend", sendEventArgs))
				return;

			DataHoldingUserToken sendToken = sendEventArgs.UserToken as DataHoldingUserToken;			
			nRemainingCnt = sendToken.sendBytesRemainingCount;
			if (nRemainingCnt <= 0)
			{
				SocketListener.Log.TRC(sendEventArgs, string.Format("ProcessSend : Invalid DataHoldingUserToken sendBytesRemainingCount count({0}).", sendToken.sendBytesRemainingCount));
				return;
			}

			FACInfo facInfo = FACContainer.FindFACInfo(sendEventArgs);
			if (facInfo == null)
			{
				SocketListener.Log.WRN(sendEventArgs, "ProcessSend : Not found FaxAgentInfo.");
				return;
			}

			facInfo.LastResponseTime = DateTime.Now;

			LOG_LEVEL logLv = LOG_LEVEL.MSG;
			string packetName = m_Encoding.GetString(sendToken.dataToSend, 34, 5);
			if (packetName == MessagePacketNameEnum.ALIVE.ToString())
				logLv = LOG_LEVEL.TRC;
			else
				logLv = LOG_LEVEL.MSG;

			// ��Ŷ �α�
			SocketListener.Log.LogWrite(logLv,
										"SEND    ",
										sendEventArgs.AcceptSocket.RemoteEndPoint.ToString(),
										sendEventArgs.AcceptSocket.Handle.ToInt32(),
										sendToken.TokenId,
										sendToken.dataToSend.Length,
										m_Encoding.GetString(sendToken.dataToSend));

			sendToken.sendBytesRemainingCount = sendToken.sendBytesRemainingCount - sendEventArgs.BytesTransferred;

			sendToken.bytesSentAlreadyCount += sendEventArgs.BytesTransferred;
		
			// send async
			StartSend(sendEventArgs);

			if (packetName == MessagePacketNameEnum.LOGIN.ToString())
				this.DisplayConnectionInfo();
        }

        #endregion

        #region method - TCP Operation - Close
        /// <summary>
        /// Shutdown -> Close
        /// </summary>
        private bool CloseClientSocket(SocketAsyncEventArgs e)
		{
			try
			{
				SocketListener.Log.MSG(e, "Start Close FaxAgent.");
				DataHoldingUserToken receiveSendToken = e.UserToken as DataHoldingUserToken;
				if (receiveSendToken.dataToReceive != null)
					receiveSendToken.CreateNewDataHolder();

				// �α׾ƿ� ó��
				FACInfo facInfo = FACContainer.FindFACInfo(e);
				if (facInfo != null)
				{
					// DB Logout ó��
					if (FACContainer.LoginedList.Count > 0 && facInfo.USER_ID != "")
					{
						if (DbModule.Instance.FAS_LogoutAgentClient(facInfo.USER_ID) != Btfax.CommonLib.RESULT.SUCCESS)
							SocketListener.Log.WRN(e, "Database Logout process failure");
						else
							SocketListener.Log.MSG(e, "Database Logout process success");
					}
				}

				// socket close
				try	{ e.AcceptSocket.Shutdown(SocketShutdown.Both); }
				catch { }

				if (e.AcceptSocket != null)
				{
					try { 
						e.AcceptSocket.Close();
						e.AcceptSocket = null;
					}
					catch { };
				}

				if (facInfo != null)
				{	
					this.poolOfRecvEventArgs.Push(facInfo.recvEventArgs);
					this.poolOfSendEventArgs.Push(facInfo.sendEventArgs);
				}
				else
				{
					this.CreateNewSaeaForRecvSendOne();
					SocketListener.Log.MSG(string.Format("CreateNewSaeaForRecvSendOne this.poolOfRecvEventArgs:{0}, this.poolOfSendEventArgs.Push{1}"
											, this.poolOfRecvEventArgs.Count
											, this.poolOfSendEventArgs.Count));
				}
							
				FACContainer.Remove(e);

				// Accept count ����
				Interlocked.Decrement(ref this.numberOfAcceptedSockets);
				this.theMaxConnectionsEnforcer.Release();
				
				SocketListener.Log.MSG(e, string.Format("Close FaxAgent success."));
				this.DisplayConnectionInfo();
				return true;
			}
			catch (Exception ex)
			{
				// Accept count ����
				Interlocked.Decrement(ref this.numberOfAcceptedSockets);
				this.theMaxConnectionsEnforcer.Release();

				SocketListener.Log.ERR(string.Format("CloseClientSocket : {0}\r\n{1}", ex.Message, ex.StackTrace));
				return false;
			}
        }

        private void HandleBadAccept(SocketAsyncEventArgs acceptEventArgs)
        {
			if (acceptEventArgs == null)
				return;

            AcceptOpUserToken acceptOpToken = acceptEventArgs.UserToken as AcceptOpUserToken;
			if (acceptOpToken == null)
				return;

            SocketListener.Log.Write("Closing socket of accept id " + acceptOpToken.TokenId);

            // ���� Close, managed and unmanaged �ڿ� ����
            // (���������� Dispose ȣ����)
			if(acceptEventArgs.AcceptSocket != null)
				acceptEventArgs.AcceptSocket.Close();

            poolOfAcceptEventArgs.Push(acceptEventArgs);
        }

        /// <summary>
        /// Accept �� Client ��� ����
        /// </summary>
        private void CloseAllFaxClient()
        {
            Dictionary<EndPoint, FACInfo> acceptList = new Dictionary<EndPoint, FACInfo>(FACContainer.AcceptedList);
            foreach (KeyValuePair<EndPoint, FACInfo> kvPair in acceptList)
            {
                (kvPair.Value.recvEventArgs.UserToken as DataHoldingUserToken).Reset();
                CloseClientSocket(kvPair.Value.recvEventArgs);
            }
        }

        private void DisposeAllSaeaObjects()
        {
            SocketAsyncEventArgs eventArgs;
            while (this.poolOfAcceptEventArgs.Count > 0)
            {
                eventArgs = poolOfAcceptEventArgs.Pop();
                eventArgs.Dispose();
            }
            while (this.poolOfRecvEventArgs.Count > 0)
            {
                eventArgs = poolOfRecvEventArgs.Pop();
                eventArgs.Dispose();
            }
            while (this.poolOfSendEventArgs.Count > 0)
            {
                eventArgs = poolOfSendEventArgs.Pop();
                eventArgs.Dispose();
            }
        }
        #endregion

        #region method - other
        /// <summary>
        /// �ߺ� �α��� ó��
        /// </summary>
        /// <param name="userId"></param>
        private void messageParsing_OnLoginDuplicated(FACInfo loginedFacInfo)
        {
            SocketAsyncEventArgs sendEventArgs = loginedFacInfo.sendEventArgs;
            if (sendEventArgs == null) return;

            this.SendMessageToClient(sendEventArgs, MessagePacketNameEnum.CLOSE, loginedFacInfo.USER_ID);
        }

        /// <summary>
        /// �����ð����� Client ��û�� ������ ���� ���� ó��
        /// </summary>
        private void AliveCheckSockets_ThreadEntry()
        {
			Dictionary<EndPoint, FACInfo> acceptedList;
			//bool isalive = true;
			while (true)
			{
				Thread.Sleep(1000 * 10);

				lock (FACContainer.LockThis)
				{
					acceptedList = new Dictionary<EndPoint, FACInfo>(FACContainer.AcceptedList);
				}

				foreach (KeyValuePair<EndPoint, FACInfo> kvPair in acceptedList)
				{
					try
					{
						FACInfo info = FACContainer.FindFACInfo(kvPair.Key);
						if (info == null)
							continue;

						if (Config.SERVER_LIMIT_RESPONSE_TIME > 0 &&
							DateTime.Now.Subtract(info.LastRequestTime).TotalSeconds > Config.SERVER_LIMIT_RESPONSE_TIME)
						{
							// ���� ó��
							SocketListener.Log.WRN(string.Format("AliveCheckSockets_ThreadEntry() : Arrive timeout. {0}/{1}"
													, DateTime.Now.Subtract(info.LastRequestTime).TotalSeconds
													, Config.SERVER_LIMIT_RESPONSE_TIME));

							// �̹� ���� �Ǿ����� Exception �߻�
							if (info.Socket != null)
							{
								SocketListener.Log.WRN(string.Format("{0}|Socket Disconnect.", kvPair.Key.ToString()));
								try { info.Socket.Disconnect(false); }
								catch { }
							}

							FACContainer.Remove(info.recvEventArgs);
							FACContainer.AcceptedList.Remove(kvPair.Key);
						}
					}
					catch (Exception ex)
					{
						SocketListener.Log.ERR(string.Format("{0}|AliveCheckSockets_ThreadEntry() {1}\r\n{2}", kvPair.Key.ToString(), ex.Message, ex.StackTrace));
						continue;
					}
				}

			}
        }
        #endregion

        #region method - for debug
        private void LogWriteWithThreadInfo(string methodName)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(string.Format("{0}\tThreadId:{1,5}", methodName, Thread.CurrentThread.ManagedThreadId));
            sb.Append(DealWithNewThreads());
            SocketListener.Log.Write(sb.ToString());
        }

        private string DealWithNewThreads()
        {
            StringBuilder sb = new StringBuilder();
            bool newThreadChecker = false;
            lock (this.lockerForThreadHashSet)
            {
                if (managedThreadIds.Add(Thread.CurrentThread.ManagedThreadId) == true)
                {
                    /// ���ο� �����尡 �߰���
                    managedThreads.Add(Thread.CurrentThread);
                    newThreadChecker = true;
                }
            }
            if (newThreadChecker == true)
            {
                sb.Append("\r\n****\r\n\tNew managed thread.\r\n\tSystem thread numbers: ");
                arrayOfLiveThreadsInThisProcess = theProcess.Threads; //for testing only
                
                foreach (ProcessThread theNativeThread in arrayOfLiveThreadsInThisProcess)
                {
                    sb.Append(theNativeThread.Id.ToString() + ", ");
                }

                sb.Append("\r\n\tManaged threads that have been used: ");
                foreach (Int32 theManagedThreadId in managedThreadIds)
                {
                    sb.Append(theManagedThreadId.ToString() + ", ");
                }

                sb.Append("\r\n\tManagedthread.IsAlive true: ");
                foreach (Thread theManagedThread in managedThreads)
                {
                    if (theManagedThread.IsAlive == true)
                    {
                        sb.Append(theManagedThread.ManagedThreadId.ToString() + ", ");
                    }
                }
                sb.Append("\r\n****");
            }
            return sb.ToString();
        }
        #endregion

        #region method - for interface - �ܺη� ���� or �ܺο��� ���
        /// <summary>
        /// UDP �� ���޹��� �޼����� Client �� �˸�
        /// </summary>
        /// <param name="userId">�˸� ���� UserID</param>
        /// <param name="message">�˸� ���� �޼���</param>
        internal void SendMessageNotiftyToClient(byte[] messageBytes, string userId)
        {	
            FACInfo facInfo = FACContainer.FindFACInfo(userId);
            if (facInfo == null)
                return;

            SocketAsyncEventArgs sendEventArgs = facInfo.sendEventArgs;
            if (sendEventArgs == null)
                return;

            /// ���� By Pass
            
            /// ���۷� ����
            DataHoldingUserToken sendToken = (DataHoldingUserToken)sendEventArgs.UserToken;
            sendToken.dataToSend = new byte[messageBytes.Length];
            Buffer.BlockCopy(messageBytes, 0, sendToken.dataToSend, 0, messageBytes.Length);

            this.SendMessage(sendEventArgs);
        }

        /// <summary>
        /// Client �� ���� ������
        /// </summary>
        /// <param name="sendEventArgs">���� SAEA ��ü</param>
        /// <param name="packetName">�ɼ� : Ư������ ����</param>
        /// <param name="userId">�ɼ� : ����� ID</param>
        /// <param name="message">�ɼ� : �޼���</param>
        internal void SendMessageToClient(SocketAsyncEventArgs sendEventArgs, MessagePacketNameEnum packetName = MessagePacketNameEnum.EMPTY, string userId = "")
        {
            DataHoldingUserToken sendToken = (DataHoldingUserToken)sendEventArgs.UserToken;
            MessageStream response = null;

            /// ���� ����
            response = messageParsing.GetResponse(sendEventArgs, packetName, userId);
			if (response == null)
				return;

            /// ���۷� ����
            int readByteAll = response.ReadAll(ref sendToken.dataToSend, this.socketListenerSettings.BufferSize);
			
            this.SendMessage(sendEventArgs);
        }

        private void SendMessage(SocketAsyncEventArgs sendEventArgs)
        {
            DataHoldingUserToken sendToken = (DataHoldingUserToken)sendEventArgs.UserToken;
            sendToken.sendBytesRemainingCount = sendToken.dataToSend.Length;
			sendToken.bytesSentAlreadyCount = 0;

            sendToken.CreateNewDataHolder();
            sendToken.Reset();

            this.StartSend(sendEventArgs);
        }

        internal void CleanUpOnExit()
        {
            CloseAllFaxClient();

            DisposeAllSaeaObjects();

			if(listenSocket != null)
				this.listenSocket.Close();
        }

		public void DisplayConnectionInfo()
		{
			SocketListener.Log.MSG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
			SocketListener.Log.MSG(string.Format(">> Connection Count:{0}, Login Count:{1}", FACContainer.AcceptedList.Count, FACContainer.LoginedList.Count));
			SocketListener.Log.MSG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		}

        /// <summary>
        /// �ִ� ������ �� (Accept)
        /// </summary>
        internal int GetMaxConnected { get { lock (this.LockThis) { return maxSimultaneousClientsThatWereConnected; } } }

        /// <summary>
        /// ���� ������ �� (Accept)
        /// </summary>
        internal int GetTotalConnection { get { lock (this.LockThis) { return numberOfAcceptedSockets; } } }

        /// <summary>
        /// ���� �α��� ����� ��
        /// </summary>
        internal int GetTotalLogins { get { lock (LockThis) { return FACContainer.LoginedCount; } } }
        #endregion

        #region static - fields
        //internal static Int32 MainTransMissionId = 10000;             // 
        //internal static Int32 StartingTid = 0;                        // �׻� 0���� ����
        internal static Int32 MainSessionId = 1000000000;               // 

        internal static SocketListenerLog Log;                          // SocketListener Ŭ���� ���� �α�
        #endregion

        #region field
        // �׽�Ʈ�� ---------------------------------------------------------------
        private Process theProcess;                                         //for testing only
        private ProcessThreadCollection arrayOfLiveThreadsInThisProcess;    //for testing
        private HashSet<int> managedThreadIds = new HashSet<int>();         //for testing
        private HashSet<Thread> managedThreads = new HashSet<Thread>();     //for testing        

        private object lockerForThreadHashSet = new object();
        // �׽�Ʈ�� ---------------------------------------------------------------

        private Encoding m_Encoding;                                // ���� ���ڵ�

        private BufferManager theBufferManager;                     // ���� ���� (Receive, Send ����)
        private Socket listenSocket;                                // ���� ����
        private Semaphore theMaxConnectionsEnforcer;                // ���� ���� ����
        private SocketListenerSettings socketListenerSettings;      // SocketListener ����

        private PrefixHandler prefixHandler;                        //
        private MessageHandler messageHandler;                      // 

        private SocketAsyncEventArgsPool poolOfAcceptEventArgs;     // SAEA Pool - Accept �뵵
        private SocketAsyncEventArgsPool poolOfRecvEventArgs;       // SAEA Pool - Receive �뵵
        private SocketAsyncEventArgsPool poolOfSendEventArgs;       // SAEA Pool - Send �뵵

        private Int32 numberOfAcceptedSockets = 0;                  // Accept �� Socket ��
        private Int32 maxSimultaneousClientsThatWereConnected = 0;  // �ִ� ���� �����ڼ�

        private MessageParsing messageParsing;                      // ���� �м� - ó��
        private Thread AliveCheckSocketsThread;                     // ���� ����/���� üũ

        private object LockThis = new object();
        #endregion
    }    
}
