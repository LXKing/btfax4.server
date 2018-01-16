using System;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace FaxAgent.Server
{
    class AcceptOpUserToken
    {
        #region constructor
        private AcceptOpUserToken()
        {
        }

        internal AcceptOpUserToken(Int32 identifier)
        {
            TokenId = identifier;

            SocketListener.Log.Write("AcceptOpUserToken constructor, TokenId " + TokenId);
        }
        #endregion

        #region field
        //internal Int32 socketHandleNumber;

        internal Int32 TokenId
        {
            get;
            set;
        }

		internal string RemoteIp
		{
			get;
			set;
		}

		internal int RemotePort
		{
			get;
			set;
		}

		public IPEndPoint IpEndPt { get; set; }
        #endregion
    }
}
