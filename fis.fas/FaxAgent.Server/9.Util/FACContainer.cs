using System;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;

namespace FaxAgent.Server
{
    class FACContainer
    {
        #region static method
        /// <summary>
        /// 접속 유지되고 있는 client 리스트
        /// </summary>
        public static Dictionary<EndPoint, FACInfo> AcceptedList { get { return FACContainer.Instance.FacList; } }

        /// <summary>
        /// 로그인된 client 리스트
        /// </summary>
        public static Dictionary<string, List<EndPoint>> LoginedList { get { return FACContainer.Instance.FacMapList; } }
        
        /// <summary>
        /// 로그인된 client 수
        /// </summary>
        public static int LoginedCount { get { return FACContainer.Instance.FacMapList.Count; } }

        /// <summary>
        /// SAEA 객체로 접속한 client 찾기
        /// </summary>
        /// <param name="e">Receive, Send SAEA 객체</param>
        /// <returns>FACInfo</returns>
        public static FACInfo FindFACInfo(SocketAsyncEventArgs e)
        {
			try
			{
				DataHoldingUserToken dToken = e.UserToken as DataHoldingUserToken;
				if (dToken == null)
				{	
					return null;
				}

				EndPoint endPoint = dToken.IpEndPt;
				return FACContainer.FindFACInfo(endPoint);
			}
			catch
			{
				return null;
			}

			////EndPoint endPoint = e.AcceptSocket.RemoteEndPoint;
			////return FACContainer.FindFACInfo(endPoint);

			////FACInfo fInfo = null;
			////IPEndPoint ep = null;
			//try
			//{
			//    DataHoldingUserToken dToken = e.UserToken as DataHoldingUserToken;
			//    if (dToken != null)
			//    {
			//        ep = dToken.IpEndPt;
			//    }
			//    else
			//    {
			//        AcceptOpUserToken aToken = e.UserToken as AcceptOpUserToken;
			//        ep = aToken.IpEndPt;
			//    }

			//    EndPoint endPoint = ep;//e.AcceptSocket.RemoteEndPoint;
			//    fInfo = FACContainer.FindFACInfo(endPoint);
			//}
			//catch
			//{
			//    fInfo = null;
			//}

			//return fInfo;
        }

        /// <summary>
        /// UserID 로 접속한 client 찾기
        /// </summary>
        /// <param name="e">UserID</param>
        /// <returns>FACInfo</returns>
        public static FACInfo FindFACInfo(string userId)
        {
            lock (FACContainer.LockThis)
            {
                if (FACContainer.Instance.FacMapList.ContainsKey(userId) == false)
                    return null;

                // 첫번째 항목을 가져온다
                return FACContainer.FindFACInfo(FACContainer.Instance.FacMapList[userId][0]);
            }
        }

        /// <summary>
        /// client IP 정보로 client 찾기
        /// </summary>
        /// <param name="endPoint">client IP 정보</param>
        /// <returns>FACInfo</returns>
        public static FACInfo FindFACInfo(EndPoint endPoint)
        {
			FACInfo info = null;
            lock (FACContainer.LockThis)
            {	
				if (!FACContainer.Instance.FacList.TryGetValue(endPoint, out info))
					info = null;
            }

			return info;
        }
		
        /// <summary>
        /// Accept 한 client 추가 (로그인 아님)
        /// </summary>
        /// <param name="client">client socket</param>
        /// <param name="recvEventArgs">등록할 Receive SAEA 객체</param>
        /// <param name="sendEventArgs">등록할 Send SAEA 객체</param>
        public static bool Add(Socket client, SocketAsyncEventArgs recvEventArgs, SocketAsyncEventArgs sendEventArgs)
        {
            lock (FACContainer.LockThis)
            {
                FACInfo fac = new FACInfo();
                fac.Socket = client;
                fac.recvEventArgs = recvEventArgs;
                fac.sendEventArgs = sendEventArgs;

				// MODIFY : KIMCG - 20140802
				// fac가 이미 존재시 계속 로그인 실패하기 때문에 Remove가 필요함.
				//if (FACContainer.Instance.FacList.ContainsKey(client.RemoteEndPoint) == true)
				//{
				//    return false;
				//}
				//else
				//{
				//    FACContainer.Instance.FacList.Add(client.RemoteEndPoint, fac);
				//    return true;
				//}

                // Dictionary 추가
				if (FACContainer.Instance.FacList.ContainsKey(client.RemoteEndPoint))
					FACContainer.Instance.FacList.Remove(client.RemoteEndPoint);

				FACContainer.Instance.FacList.Add(client.RemoteEndPoint, fac);
				return true;
            }
        }

		/// <summary>
		/// Accept 한 client 추가 (로그인 아님)
		/// </summary>
		/// <param name="client">client socket</param>
		/// <param name="recvEventArgs">등록할 Receive SAEA 객체</param>
		/// <param name="sendEventArgs">등록할 Send SAEA 객체</param>
		public static bool AddEx(Socket client, SocketAsyncEventArgs recvEventArgs, SocketAsyncEventArgs sendEventArgs, out string strOutMsg)
		{
			strOutMsg = "";
			lock (FACContainer.LockThis)
			{
				FACInfo fac = new FACInfo();
				fac.Socket = client;
				fac.recvEventArgs = recvEventArgs;
				fac.sendEventArgs = sendEventArgs;

				if (FACContainer.Instance.FacList.ContainsKey(client.RemoteEndPoint))
				{
					strOutMsg = "Already Exists FaxAgentCleint. remove old data";
					FACContainer.Instance.FacList.Remove(client.RemoteEndPoint);
				}

				FACContainer.Instance.FacList.Add(client.RemoteEndPoint, fac);
				return true;
			}
		}

        /// <summary>
        /// Login 된 client 정보로 수정
        /// </summary>
        /// <param name="client">client socket (검색용)</param>
        /// <param name="userId">등록할 UserID</param>
        /// <param name="userName">등록할 User 이름</param>
        public static bool Update(Socket client, string userId, string userName)
        {
            lock (FACContainer.LockThis)
            {
                /// 유효성 검사
                EndPoint endPoint = client.RemoteEndPoint;
                FACInfo facInfo = FACContainer.FindFACInfo(endPoint);
                if (facInfo == null)
                    return false;

                /// 정보 수정
                facInfo.USER_ID = userId;
                facInfo.USER_NAME = userName;
                
                // Dictionary Map 추가 (검색용)
                if (FACContainer.Instance.FacMapList.ContainsKey(userId) == false)
                {
                    List<EndPoint> mapEndPoint = new List<EndPoint>();
                    mapEndPoint.Add(endPoint);
                    FACContainer.Instance.FacMapList.Add(userId, mapEndPoint);
                }
                else
                {
                    FACContainer.Instance.FacMapList[userId].Add(client.RemoteEndPoint);
                }
                return true;
            }
        }

        /// <summary>
        /// Accept, Login 된 client 정보 제거
        /// </summary>
        /// <param name="e">SAEA 객체</param>
        public static bool Remove(SocketAsyncEventArgs e)
        {
			lock (FACContainer.LockThis)
			{
				IPEndPoint ep = null;
				DataHoldingUserToken dToken = e.UserToken as DataHoldingUserToken;
				ep = dToken.IpEndPt;

				//IPEndPoint ep = e.RemoteEndPoint as IPEndPoint;
				//if (dToken != null)
				//{
				//    ep = dToken.IpEndPt;
				//}
				//else
				//{
				//    AcceptOpUserToken aToken = e.UserToken as AcceptOpUserToken;
				//    ep = aToken.IpEndPt;
				//}

				//if(e.AcceptSocket == null)
				//	return false;

				//EndPoint endPoint = e.AcceptSocket.RemoteEndPoint;

				//DataHoldingUserToken receiveSendToken = (e.UserToken as DataHoldingUserToken);
				//EndPoint endPoint = e.AcceptSocket.RemoteEndPoint;
				if (ep == null)
					return false;

				FACInfo facInfo = null;
				if (!FACContainer.Instance.FacList.TryGetValue(ep, out facInfo))
				{	
					return false;
				}

				// 사용자맵 정보 제거
				List<EndPoint> mapEndPoint;
				if (FACContainer.Instance.FacMapList.TryGetValue(facInfo.USER_ID, out mapEndPoint))
				{
					if (mapEndPoint.Count == 1)
						FACContainer.Instance.FacMapList.Remove(facInfo.USER_ID);
					else
						FACContainer.Instance.FacMapList[facInfo.USER_ID].Remove(ep);
				}

				// FAC 정보 제거
				if (FACContainer.Instance.FacList.ContainsKey(ep))
					FACContainer.Instance.FacList.Remove(ep);

                return true;
            }
        }
        #endregion

        #region static - fields
        private static FACContainer s_instance = null;
        public static FACContainer Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new FACContainer();

                return s_instance;
            }
        }
        public static object LockThis = new object();
        #endregion

        #region constructor
        private FACContainer()
        {
            this.FacList = new Dictionary<EndPoint, FACInfo>();
            this.FacMapList = new Dictionary<string, List<EndPoint>>();
        }
        #endregion

        #region field
        /// <summary>
        /// FAC Accept 정보
        /// Key   : Client IP 정보
        /// Value : FACInfo 클래스
        /// </summary>
        private Dictionary<EndPoint, FACInfo> FacList
        {
            get;
            set;
        }

        /// <summary>
        /// FAC Login 정보
        /// Key   : client LoginID
        /// Value : client IP 정보 (중복 로그인된 것 포함)
        /// </summary>
        private Dictionary<string, List<EndPoint>> FacMapList
        {
            get;
            set;
        }
        #endregion
    }
}
