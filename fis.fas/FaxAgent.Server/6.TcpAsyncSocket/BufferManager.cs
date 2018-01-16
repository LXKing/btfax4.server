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
        // ���� ����
        internal void InitBuffer()
        {
            this.bufferBlock = new byte[totalBytesInBufferBlock];
            SocketListener.Log.Write("BufferManager.InitBuffer() Allocation bytes : " + totalBytesInBufferBlock);
        }

        // ���� �Ҵ�
        internal bool SetBuffer(SocketAsyncEventArgs recvArgs, SocketAsyncEventArgs sendArgs)
        {
            //if (this.freeIndexPool.Count > 0)
            //{
            //    int freeIndex = this.freeIndexPool.Pop();

            //    /// ���� SAEA ��ü ���� �Ҵ�
            //    recvArgs.SetBuffer(this.bufferBlock, freeIndex, this.bufferBytesAllocatedForEachSaea);
            //    /// �۽� SAEA ��ü ���� ����
            //    sendArgs.SetBuffer(this.bufferBlock, freeIndex + this.bufferBytesAllocatedForEachSaea, this.bufferBytesAllocatedForEachSaea);
            //}
            //else
            //{

			/// ���� SAEA ��ü ���� �Ҵ�
			int nBuffSize = 4096;
			recvArgs.SetBuffer(new byte[nBuffSize], 0, nBuffSize);

			/// �۽� SAEA ��ü ���� �Ҵ�
			sendArgs.SetBuffer(new byte[nBuffSize], 0, nBuffSize);



			//if ((totalBytesInBufferBlock - this.bufferBytesAllocatedForEachSaea) < this.currentIndex)
			//{
			//    return false;
			//}
			
			///// ���� SAEA ��ü ���� �Ҵ�
			//recvArgs.SetBuffer(this.bufferBlock, this.currentIndex, this.bufferBytesAllocatedForEachSaea);
			
			///// �۽� SAEA ��ü ���� �Ҵ�
			//sendArgs.SetBuffer(this.bufferBlock, this.currentIndex + this.bufferBytesAllocatedForEachSaea, this.bufferBytesAllocatedForEachSaea);
			
			///// ���� �Ҵ��� ���� Offset
			//this.currentIndex += this.bufferBytesAllocatedForEachSaea * 2;
            
            return true;
        }

		// ���� �Ҵ�
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


		//// ���� �Ҵ�
		//internal bool ResetBuffer(SocketAsyncEventArgs recvArgs, SocketAsyncEventArgs sendArgs)
		//{
		//    /// ���� SAEA ��ü ���� �Ҵ�
		//    recvArgs.SetBuffer(this.bufferBlock, recvArgs.Offset + this.bufferBytesAllocatedForEachSaea, this.bufferBytesAllocatedForEachSaea);
		//    /// �۽� SAEA ��ü ���� �Ҵ�
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
        private Int32 totalBytesInBufferBlock;          // ���� ��ü ũ��

        private byte[] bufferBlock;
        //private Stack<int> freeIndexPool;
        private Int32 currentIndex;
        private Int32 bufferBytesAllocatedForEachSaea;  // SAEA ��ü 1���� ����� ũ�� (2048 * 2)
        #endregion
    }
}
