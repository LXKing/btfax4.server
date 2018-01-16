using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading;

namespace FaxAgent.Server
{    
    internal sealed class SocketAsyncEventArgsPool
    {
        #region constructor
        internal SocketAsyncEventArgsPool(Int32 capacity)
        {
            SocketListener.Log.Write("SocketAsyncEventArgsPool constructor. capacity : " + capacity.ToString());
            
            //this.pool = new Stack<SocketAsyncEventArgs>(capacity);
			this.pool = new Queue<SocketAsyncEventArgs>(capacity);
        }
        #endregion

        #region method
        internal Int32 AssignTokenId()
        {
            Int32 tokenId = Interlocked.Increment(ref nextTokenId);            
            return tokenId;
        }

        internal SocketAsyncEventArgs Pop()
        {
            lock (this.pool)
            {
                //return this.pool.Pop();
				return this.pool.Dequeue();
            }
        }

        internal void Push(SocketAsyncEventArgs item)
        {
            if (item == null) 
            { 
                throw new ArgumentNullException("Items added to a SocketAsyncEventArgsPool cannot be null"); 
            }
            lock (this.pool)
            {
                //this.pool.Push(item);
				this.pool.Enqueue(item);
            }
        }
        #endregion

        #region field
        private Int32 nextTokenId = 0;
        //private Stack<SocketAsyncEventArgs> pool;
		private Queue<SocketAsyncEventArgs> pool;

        internal Int32 Count
        {
            get { return this.pool.Count; }
        }
        #endregion
    }
}
