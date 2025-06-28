#pragma once

#ifdef _WIN32
#define FD_SETSIZE 5096 // http://blog.naver.com/znfgkro1/220175848048
#else
// 리눅스에서는 1024까지만  https://qiita.com/fujinochan/items/2337ce48a998cf67966b
#endif

#ifdef _WIN32
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif

#include <vector>
#include <deque>
#include <unordered_map>
#include "ITcpNetwork.h"


namespace NServerNetLib
{
	class TcpNetwork : public ITcpNetwork
	{
	public:
		TcpNetwork();
		virtual ~TcpNetwork();

		NET_ERROR_CODE Init(const ServerConfig* pConfig, ILog* pLogger) override;
		
		NET_ERROR_CODE SendData(const int sessionIndex, const short packetId, const short size, const char* pMsg) override;
		
		void Run() override;
		
		RecvPacketInfo GetPacketInfo() override;

		void Release() override;

		int ClientSessionPoolSize() override { return (int)m_ClientSessionPool.size(); }

		void ForcingClose(const int sessionIndex) override;


	protected:
		NET_ERROR_CODE InitServerSocket();
		NET_ERROR_CODE BindListen(short port, int backlogCount);
		
		int AllocClientSessionIndex();
		void ReleaseSessionIndex(const int index);

		int CreateSessionPool(const int maxClientCount);
		NET_ERROR_CODE NewSession();
		void SetSockOption(const SOCKET fd);
		void ConnectedSession(const int sessionIndex, const SOCKET fd, const char* pIP);
		
		void CloseSession(const SOCKET_CLOSE_CASE closeCase, const SOCKET sockFD, const int sessionIndex);
		
		NET_ERROR_CODE RecvSocket(const int sessionIndex);
		NET_ERROR_CODE RecvBufferProcess(const int sessionIndex);
		void AddPacketQueue(const int sessionIndex, const short pktId, const short bodySize, char* pDataPos);
		
		void RunProcessWrite(const int sessionIndex, const SOCKET fd, fd_set& write_set);
		NetError FlushSendBuff(const int sessionIndex);
		NetError SendSocket(const SOCKET fd, const char* pMsg, const int size);

		bool RunCheckSelectResult(const int result);
		void RunCheckSelectClients(fd_set& read_set, fd_set& write_set);
		bool RunProcessReceive(const int sessionIndex, const SOCKET fd, fd_set& read_set);

		NET_ERROR_CODE SetNonBlockSocket(const SOCKET sock);
		
	protected:
		ServerConfig m_Config;
				
		SOCKET m_ServerSockfd;
        SOCKET m_MaxSockFD = 0;

		fd_set m_Readfds;
		size_t m_ConnectedSessionCount = 0;
		
		int64_t m_ConnectSeq = 0;
		
		std::vector<ClientSession> m_ClientSessionPool;
		std::deque<int> m_ClientSessionPoolIndex;
		
		std::deque<RecvPacketInfo> m_PacketQueue;

		ILog* m_pRefLogger;
	};
}
