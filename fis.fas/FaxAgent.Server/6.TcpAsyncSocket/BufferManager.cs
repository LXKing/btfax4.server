using System;
using System.Collections.Generic;
using System.Net.Sockets;

namespace FaxAgent.Server
{   
    class BufferManager
    {
        #region constructor
        private BufferManager()
        {
        }

        internal BufferManager(Int32 totalBytes, Int32 totalBufferBytesInEachSaeaObject)
        {
            totalBytesInBufferBlock = totalBytes;
            this.currentIndex = 0;
            this.bufferBytesAllocatedForEachSaea = totalBufferBytesInEachSaeaObject;
            //this.freeIndexPool = new Stack<int>();
        }
        #endregion

        #region method
        // 버퍼 생성
        internal void InitBuffer()
        {
            this.bufferBlock = new byte[totalBytesInBufferBlock];
            SocketListener.Log.Write("BufferManager.InitBuffer() Allocation bytes : " + totalBytesInBufferBlock);
        }

        // 버퍼 할당
        internal bool SetBuffer(SocketAsyncEventArgs recvArgs, SocketAsyncEventArgs sendArgs)
        {
            //if (this.freeIndexPool.Count > 0)
            //{
            //    int freeIndex = this.freeIndexPool.Pop();

            //    /// 수신 SAEA 객체 버퍼 할당
            //    recvArgs.SetBuffer(this.bufferBlock, freeIndex, this.bufferBytesAllocatedForEachSaea);
            //    /// 송신 SAEA 객체 버퍼 당할
            //    sendArgs.SetBuffer(this.bufferBlock, freeIndex + this.bufferBytesAllocatedForEachSaea, this.bufferBytesAllocatedForEachSaea);
            //}
            //else
            //{

			/// 수신 SAEA 객체 버퍼 할당
			int nBuffSize = 4096;
			recvArgs.SetBuffer(new byte[nBuffSize], 0, nBuffSize);

			/// 송신 SAEA 객체 버퍼 할당
			sendArgs.SetBuffer(new byte[nBuffSize], 0, nBuffSize);



			//if ((totalBytesInBufferBlock - this.bufferBytesAllocatedForEachSaea) < this.currentIndex)
			//{
			//    return false;
			//}
			
			///// 수신 SAEA 객체 버퍼 할당
			//recvArgs.SetBuffer(this.bufferBlock, this.currentIndex, this.bufferBytesAllocatedForEachSaea);
			
			///// 송신 SAEA 객체 버퍼 할당
			//sendArgs.SetBuffer(this.bufferBlock, this.currentIndex + this.bufferBytesAllocatedForEachSaea, this.bufferBytesAllocatedForEachSaea);
			
			///// 다음 할당할 버퍼 Offset
			//this.currentIndex += this.bufferBytesAllocatedForEachSaea * 2;
            
            return true;
        }

		// 버퍼 할당
		internal bool SetBuffer(int nOffset, SocketAsyncEventArgs args)
		{
			if (nOffset < 0)
				return false;

			int nBuffSize = 4096;
			args.SetBuffer(new byte[nBuffSize], 0, nBuffSize);

			//args.SetBuffer(this.bufferBlock, nOffset + this.bufferBytesAllocatedForEachSaea, this.bufferBytesAllocatedForEachSaea);
			//args.SetBuffer(this.bufferBlock, nOffset + this.bufferBytesAllocatedForEachSaea, this.bufferBytesAllocatedForEachSaea);
			return true;
		}


		//// 버퍼 할당
		//internal bool ResetBuffer(SocketAsyncEventArgs recvArgs, SocketAsyncEventArgs sendArgs)
		//{
		//    /// 수신 SAEA 객체 버퍼 할당
		//    recvArgs.SetBuffer(this.bufferBlock, recvArgs.Offset + this.bufferBytesAllocatedForEachSaea, this.bufferBytesAllocatedForEachSaea);
		//    /// 송신 SAEA 객체 버퍼 할당
		//    sendArgs.SetBuffer(this.bufferBlock, sendArgs.Offset + this.bufferBytesAllocatedForEachSaea, this.bufferBytesAllocatedForEachSaea);
		//    return true;
		//}


        //internal void FreeBuffer(SocketAsyncEventArgs args)
        //{
        //    this.freeIndexPool.Push(args.Offset);
        //    args.SetBuffer(null, 0, 0);
        //}
        #endregion

        #region field
        private Int32 totalBytesInBufferBlock;          // 버퍼 전체 크기

        private byte[] bufferBlock;
        //private Stack<int> freeIndexPool;
        private Int32 currentIndex;
        private Int32 bufferBytesAllocatedForEachSaea;  // SAEA 객체 1개가 사용할 크기 (2048 * 2)
        #endregion
    }
}
