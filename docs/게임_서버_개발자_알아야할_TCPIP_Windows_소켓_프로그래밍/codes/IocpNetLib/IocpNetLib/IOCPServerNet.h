#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <optional>

#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "CommonDefine.h"
#include "Performance.h"
#include "Connection.h"
#include "MessagePool.h"
#include "MiniDump.h"

namespace NetLib
{
	//TODO 너무 크니 적절하게 분해하자

	//TODO 모든 세션을 주기적으로 조사 기능 구현하기
	// - 연결이 끊어졌는데 재 사용하고 있는지 조사하기
	
	struct OVERLAPPED_EX;
	class Connection;
	//class MessagePool;


	class IOCPServerNet
	{
	public:
		IOCPServerNet() {}
		~IOCPServerNet() {}

	public:
		virtual NetResult Start(NetConfig netConfig)
		{
			LogFuncPtr((int)LogLevel::Info, "Start Server Completion");
						
			m_NetConfig = netConfig;
			
			auto result = CreateListenSocket();
			if (result != NetResult::Success)
			{
				return result;
			}

			result = CreateHandleIOCP();
			if (result != NetResult::Success)
			{
				return result;
			}

			if (!CreateMessageManager())
			{
				return NetResult::Fail_Create_Message_Manager;
			}

			if (!LinkListenSocketIOCP())
			{
				return NetResult::Fail_Link_IOCP;
			}

			if (!CreateConnections())
			{
				return NetResult::Fail_Create_Connection;
			}

			if (!CreatePerformance())
			{
				return NetResult::Fail_Create_Performance;
			}

			if (!CreateWorkThread())
			{
				return NetResult::Fail_Create_WorkThread;
			}

			LogFuncPtr((int)LogLevel::Info, "Start Server Completion");
			return NetResult::Success;
		}

		virtual void End()
		{
			LogFuncPtr((int)LogLevel::Info, "IOCPServer::EndServer - Start");

			if (m_hWorkIOCP != INVALID_HANDLE_VALUE)
			{
				m_IsRunWorkThread = false;
				CloseHandle(m_hWorkIOCP);
								
				for (int i = 0; i < m_WorkThreads.size(); ++i)
				{
					if (m_WorkThreads[i].get()->joinable())
					{
						m_WorkThreads[i].get()->join();
					}
				}												
			}
			

			if (m_hLogicIOCP != INVALID_HANDLE_VALUE)
			{
				//PostQueuedCompletionStatus(m_hLogicIOCP, 0, 0, nullptr);
				CloseHandle(m_hLogicIOCP);
			}

			if (m_ListenSocket != INVALID_SOCKET)
			{
				closesocket(m_ListenSocket);
				m_ListenSocket = INVALID_SOCKET;
			}

			WSACleanup();


			DestoryConnections();
						
			LogFuncPtr((int)LogLevel::Info, "IOCPServer::EndServer - Completion");
		}

		
		bool GetNetworkMessage(OUT INT8& msgOperationType, OUT INT32& connectionIndex, char* pBuf, OUT INT16& copySize, const INT32 waitTimeMillisec)
		{
			Message* pMsg = nullptr;
			Connection* pConnection = nullptr;
			DWORD ioSize = 0;
			auto waitTime = waitTimeMillisec;

			if (waitTime == 0)
			{
				waitTime = INFINITE;
			}
			auto result = GetQueuedCompletionStatus(
				m_hLogicIOCP,
				&ioSize,
				reinterpret_cast<PULONG_PTR>(&pConnection),
				reinterpret_cast<LPOVERLAPPED*>(&pMsg),
				waitTime);

			if (result == false)
			{
				return false;
			}

			
			switch (pMsg->Type)
			{
			case MessageType::Connection:
				MakeNetMessageOnConnection(pConnection, pMsg, msgOperationType, connectionIndex);
				break;
			case MessageType::Close:
				//TODO 재 사용에 딜레이를 주도록 한다. 이유는 재 사용으로 가능 도중 IOCP 워크 스레드에서 이 세션이 호출될 수도 있다. 
				MakeNetMessageOnClose(pConnection, pMsg, msgOperationType, connectionIndex);
				break;
			case MessageType::OnRecv:
				MakeNetMessageOnReceive(pConnection, pMsg, msgOperationType, connectionIndex, 
											pBuf, copySize, ioSize);
				m_pMsgPool->DeallocMsg(pMsg);
				break;
			}
						
			return true;
		}

		void SendPacket(const INT32 connectionIndex, const char* pSendPacket, const INT16 packetSize)
		{
			auto pConnection = GetConnection(connectionIndex);
			if (pConnection == nullptr)
			{
				return;
			}
			

			auto result = pConnection->WriteSendData(packetSize, pSendPacket);
			if (result == NetResult::ReservedSendPacketBuffer_Not_Connected)
			{
				return;
			}
			else if (result == NetResult::ReservedSendPacketBuffer_Empty_Buffer)
			{
				if (pConnection->CloseComplete())
				{
					HandleExceptionCloseConnection(pConnection);
				}
				return;
			}
			

			if (pConnection->PostSend() == false)
			{
				if (pConnection->CloseComplete())
				{
					HandleExceptionCloseConnection(pConnection);
				}
			}
		}

		int GetMaxPacketSize() { return m_NetConfig.MaxPacketSize; }
		int GetMaxConnectionCount() { return m_NetConfig.MaxConnectionCount; }
		

	private:		
		NetResult CreateListenSocket()
		{
			WSADATA wsaData;
			auto result = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (result != 0)
			{
				return NetResult::fail_create_listensocket_startup;
			}

			m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (m_ListenSocket == INVALID_SOCKET)
			{
				return NetResult::fail_create_listensocket_socket;
			}

			SOCKADDR_IN	addr;
			ZeroMemory(&addr, sizeof(SOCKADDR_IN));

			addr.sin_family = AF_INET;
			addr.sin_port = htons(m_NetConfig.PortNumber);
			addr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (::bind(m_ListenSocket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR)
			{
				return NetResult::fail_create_listensocket_bind;
			}

			if (::listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR)
			{
				return NetResult::fail_create_listensocket_listen;
			}

			return NetResult::Success;
		}

		NetResult CreateHandleIOCP()
		{
			m_hWorkIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
			if (m_hWorkIOCP == INVALID_HANDLE_VALUE)
			{
				return NetResult::fail_handleiocp_work;
			}

			m_hLogicIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
			if (m_hLogicIOCP == INVALID_HANDLE_VALUE)
			{
				return NetResult::fail_handleiocp_logic;
			}

			return NetResult::Success;
		}

		bool CreateMessageManager()
		{
			m_pMsgPool = std::make_unique<MessagePool>(m_NetConfig.MaxMessagePoolCount, m_NetConfig.ExtraMessagePoolCount); 
			if (!m_pMsgPool->CheckCreate())
			{
				return false;
			}

			return true;
		}

		bool LinkListenSocketIOCP()
		{
			auto hIOCPHandle = CreateIoCompletionPort(
				reinterpret_cast<HANDLE>(m_ListenSocket),
				m_hWorkIOCP,
				0,
				0);

			if (hIOCPHandle == INVALID_HANDLE_VALUE || m_hWorkIOCP != hIOCPHandle)
			{
				return false;
			}

			return true;
		}


		bool CreateConnections()
		{
			ConnectionNetConfig config;
			config.MaxSendBufferSize = m_NetConfig.ConnectionMaxSendBufferSize;
			config.MaxRecvBufferSize = m_NetConfig.ConnectionMaxRecvBufferSize;
			config.MaxRecvOverlappedBufferSize = m_NetConfig.MaxPacketSize;
			config.MaxSendOverlappedBufferSize = m_NetConfig.MaxPacketSize;

			for (int i = 0; i < m_NetConfig.MaxConnectionCount; ++i)
			{
				auto pConnection = new Connection();
				pConnection->Init(m_ListenSocket, i, config);
				m_Connections.push_back(pConnection);
			}

			return true;
		}

		void DestoryConnections()
		{
			for (int i = 0; i < m_NetConfig.MaxConnectionCount; ++i)
			{
				delete m_Connections[i];				
			}
		}

		Connection* GetConnection(const INT32 connectionIndex)
		{
			if (connectionIndex < 0 || connectionIndex >= m_NetConfig.MaxConnectionCount)
			{
				return nullptr;
			}

			return m_Connections[connectionIndex];
		}


		bool CreatePerformance()
		{
			if (m_NetConfig.PerformancePacketMillisecondsTime == INVALID_VALUE)
			{
				return false;
			}

			m_Performance = std::make_unique<Performance>();
			m_Performance->Start(m_NetConfig.PerformancePacketMillisecondsTime);

			return true;
		}

		bool CreateWorkThread()
		{
			if (m_NetConfig.WorkThreadCount == INVALID_VALUE)
			{
				return false;
			}

			for (int i = 0; i < m_NetConfig.WorkThreadCount; ++i)
			{
				m_WorkThreads.push_back(std::make_unique<std::thread>([&]() {WorkThread(); }));
			}

			return true;
		}

		void WorkThread()
		{
			while (m_IsRunWorkThread)
			{
				DWORD ioSize = 0;
				OVERLAPPED_EX* pOverlappedEx = nullptr;
				Connection* pConnection = nullptr;

				//TODO: 모든 GetQueuedCompletionStatus를 GetQueuedCompletionStatusEx 버전을 사용하도록 변경. 
				auto result = GetQueuedCompletionStatus(
					m_hWorkIOCP,
					&ioSize,
					reinterpret_cast<PULONG_PTR>(&pConnection),
					reinterpret_cast<LPOVERLAPPED*>(&pOverlappedEx),
					INFINITE);

				if (pOverlappedEx == nullptr)
				{
					if (WSAGetLastError() != 0 && WSAGetLastError() != WSA_IO_PENDING)
					{
						char logmsg[128] = { 0, };
						sprintf_s(logmsg, "IOCPServer::WorkThread - GetQueuedCompletionStatus(). error:%d", WSAGetLastError());
						LogFuncPtr((int)LogLevel::Error, logmsg);
					}
					continue;
				}

				if (!result || (0 == ioSize && OperationType::Accept != pOverlappedEx->OverlappedExOperationType))
				{
					HandleExceptionWorkThread(pConnection, pOverlappedEx);
					continue;
				}

				switch (pOverlappedEx->OverlappedExOperationType)
				{
				case OperationType::Accept:
					DoAccept(pOverlappedEx);
					break;
				case OperationType::Recv:
					DoRecv(pOverlappedEx, ioSize);
					break;
				case OperationType::Send:
					DoSend(pOverlappedEx, ioSize);
					break;
				}
			}
		}

		NetResult PostNetMessage(Connection* pConnection, Message* pMsg, const DWORD packetSize = 0)
		{
			if (m_hLogicIOCP == INVALID_HANDLE_VALUE || pMsg == nullptr)
			{
				return NetResult::fail_message_null;
			}

			auto result = PostQueuedCompletionStatus(
				m_hLogicIOCP,
				packetSize,
				reinterpret_cast<ULONG_PTR>(pConnection),
				reinterpret_cast<LPOVERLAPPED>(pMsg));

			if (!result)
			{
				char logmsg[256] = { 0, };
				sprintf_s(logmsg, "IOCPServer::PostLogicMsg - PostQueuedCompletionStatus(). error:%d, MsgType:%d", WSAGetLastError(), pMsg->Type);
				LogFuncPtr((int)LogLevel::Error, logmsg);
				return NetResult::fail_pqcs;
			}
			return NetResult::Success;
		}

		void HandleExceptionWorkThread(Connection* pConnection, const OVERLAPPED_EX* pOverlappedEx)
		{
			if (pOverlappedEx == nullptr)
			{
				return;
			}
						
			//Connection 접속 종료 시 남은 IO 처리
			switch (pOverlappedEx->OverlappedExOperationType)
			{
			case OperationType::Accept:
				pConnection->DecrementAcceptIORefCount();
				break;
			case OperationType::Recv:
				pConnection->DecrementRecvIORefCount();
				break;
			case OperationType::Send:
				pConnection->DecrementSendIORefCount();
				break;
			}

			if (pConnection->CloseComplete())
			{
				HandleExceptionCloseConnection(pConnection);
			}

			return;
		}
				
		void HandleExceptionCloseConnection(Connection* pConnection)
		{
			if (pConnection == nullptr)
			{
				return;
			}
									
			pConnection->DisconnectConnection();

			pConnection->SetNetStateDisConnection();
						
			if (PostNetMessage(pConnection, pConnection->GetCloseMsg()) != NetResult::Success)
			{
				pConnection->ResetConnection();
			}
		}

		void DoAccept(const OVERLAPPED_EX* pOverlappedEx)
		{
			auto pConnection = GetConnection(pOverlappedEx->ConnectionIndex);
			if (pConnection == nullptr)
			{
				return;
			}

			pConnection->DecrementAcceptIORefCount();

			if (pConnection->SetNetAddressInfo() == false)
			{
				char logmsg[128] = { 0, };
				sprintf_s(logmsg, "IOCPServer::DoAccept - GetAcceptExSockaddrs(). error:%d", WSAGetLastError());
				LogFuncPtr((int)LogLevel::Error, logmsg);

				if (pConnection->CloseComplete())
				{
					HandleExceptionCloseConnection(pConnection);
				}
				return;
			}
		
			if (!pConnection->BindIOCP(m_hWorkIOCP))
			{
				if (pConnection->CloseComplete())
				{
					HandleExceptionCloseConnection(pConnection);
				}
				return;
			}

			pConnection->SetNetStateConnection();
			
			auto result = pConnection->PostRecv();
			if (result != NetResult::Success)
			{
				char logmsg[128] = { 0, };
				sprintf_s(logmsg, "IOCPServer::PostRecv. Call pConnection->PostRecv. error:%d", WSAGetLastError());
				LogFuncPtr((int)LogLevel::Error, logmsg);
				
				HandleExceptionCloseConnection(pConnection);
				return;
			}

			if (PostNetMessage(pConnection, pConnection->GetConnectionMsg()) != NetResult::Success)
			{
				pConnection->DisconnectConnection();
				pConnection->ResetConnection();
				return;
			}
		}

		void DoRecv(OVERLAPPED_EX* pOverlappedEx, const DWORD ioSize)
		{
			Connection* pConnection = GetConnection(pOverlappedEx->ConnectionIndex);
			if (pConnection == nullptr)
			{
				return;
			}

			pConnection->DecrementRecvIORefCount();
			

			auto [remainByte, pNext] = pConnection->GetReceiveData(ioSize);
			
			auto totalReadSize = PacketForwardingLoop(pConnection, remainByte, pNext);
			if (!totalReadSize)
			{
				return;
			}

			pConnection->ReadRecvBuffer(totalReadSize.value());

			if (pConnection->PostRecv() != NetResult::Success)
			{
				if (pConnection->CloseComplete())
				{
					HandleExceptionCloseConnection(pConnection);
				}
			}


		}

		std::optional<int> PacketForwardingLoop(Connection* pConnection, int remainByte, char* pBuffer)
		{
			//TODO 패킷 분해 부분을 가상 함수로 만들기 

			const int PACKET_HEADER_LENGTH = 5;
			const int PACKET_SIZE_LENGTH = 2;
			const int PACKET_TYPE_LENGTH = 2;
			
			int totalReadSize = 0;

			while (true)
			{
				if (remainByte < PACKET_HEADER_LENGTH)
				{
					break;
				}

				short packetSize = 0;
				CopyMemory(&packetSize, pBuffer, PACKET_SIZE_LENGTH);

				if (0 >= packetSize || packetSize > pConnection->RecvBufferSize())
				{
					char logmsg[128] = { 0, }; sprintf_s(logmsg, "IOCPServer::DoRecv. Arrived Wrong Packet.");
					LogFuncPtr((int)LogLevel::Error, logmsg);

					if (pConnection->CloseComplete())
					{
						HandleExceptionCloseConnection(pConnection);
					}
					return std::nullopt;
				}

				if (remainByte >= packetSize)
				{
					auto pMsg = m_pMsgPool->AllocMsg();
					if (pMsg == nullptr)
					{
						return totalReadSize;
					}

					pMsg->SetMessage(MessageType::OnRecv, pBuffer);
					if (PostNetMessage(pConnection, pMsg, packetSize) != NetResult::Success)
					{
						m_pMsgPool->DeallocMsg(pMsg);
						return totalReadSize;
					}

					remainByte -= packetSize;
					totalReadSize += packetSize;
					pBuffer += packetSize;					
				}
				else
				{
					break;
				}
			}

			return totalReadSize;
		}

		void DoSend(OVERLAPPED_EX* pOverlappedEx, const DWORD ioSize)
		{
			auto pConnection = GetConnection(pOverlappedEx->ConnectionIndex);
			if (pConnection == nullptr)
			{
				return;
			}

			
			pConnection->DecrementSendIORefCount();
			pConnection->SendBufferSendCompleted(ioSize);
			pConnection->SetEnableSend();
			
			if (pConnection->PostSend() == false)
			{
				if (pConnection->CloseComplete())
				{
					HandleExceptionCloseConnection(pConnection);
				}
			}		
		}

		void MakeNetMessageOnConnection(Connection* pConnection, const Message* pMsg, OUT INT8& msgOperationType, OUT INT32& connectionIndex)
		{
			if (pConnection->IsConnect() == false)
			{
				return;
			}

			msgOperationType = (INT8)pMsg->Type;
			connectionIndex = pConnection->GetIndex();

			char logmsg[128] = { 0, };
			sprintf_s(logmsg, "IOCPServer::DoPostConnection. Connect Connection. Index:%d", pConnection->GetIndex());
			LogFuncPtr((int)LogLevel::Debug, logmsg);
		}

		void MakeNetMessageOnClose(Connection* pConnection, const Message* pMsg, OUT INT8& msgOperationType, OUT INT32& connectionIndex)
		{
			msgOperationType = (INT8)pMsg->Type;
			connectionIndex = pConnection->GetIndex();

			char logmsg[128] = { 0, };
			sprintf_s(logmsg, "IOCPServer::DoPostClose. Disconnect Connection. Index:%d", pConnection->GetIndex());
			LogFuncPtr((int)LogLevel::Debug, logmsg);

			pConnection->ResetConnection();
		}

		void MakeNetMessageOnReceive(Connection* pConnection, const Message* pMsg, OUT INT8& msgOperationType, OUT INT32& connectionIndex, char* pBuf, OUT INT16& copySize, const DWORD ioSize)
		{
			if (pMsg->pContents == nullptr)
			{
				return;
			}

			msgOperationType = (INT8)pMsg->Type;
			connectionIndex = pConnection->GetIndex();
			CopyMemory(pBuf, pMsg->pContents, ioSize);

			copySize = static_cast<INT16>(ioSize);

			m_Performance.get()->IncrementPacketProcessCount();
		}


	private:
		NetConfig m_NetConfig;

		SOCKET m_ListenSocket = INVALID_SOCKET;

		std::vector<Connection*> m_Connections;
			
		HANDLE m_hWorkIOCP = INVALID_HANDLE_VALUE;
		HANDLE m_hLogicIOCP = INVALID_HANDLE_VALUE;

		bool m_IsRunWorkThread = true;
		std::vector<std::unique_ptr<std::thread>> m_WorkThreads;

		std::unique_ptr<MessagePool> m_pMsgPool;

		std::unique_ptr<Performance> m_Performance;
	};
}