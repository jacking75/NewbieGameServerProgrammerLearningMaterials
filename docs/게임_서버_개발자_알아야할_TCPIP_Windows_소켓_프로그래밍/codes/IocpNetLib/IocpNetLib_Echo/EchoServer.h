#pragma once

#include "../IocpNetLib/NetDefine.h"
#include "../IocpNetLib/IOCPServerNet.h"

struct ServerAppConfig : NetLib::NetConfig
{
	int PostMessagesThreadsCount = 1;
	
	NetLib::NetConfig GetNetConfig()
	{
		NetLib::NetConfig netConfig;

		netConfig.PortNumber = PortNumber;
		netConfig.WorkThreadCount = WorkThreadCount;
		netConfig.ConnectionMaxRecvBufferSize = ConnectionMaxRecvBufferSize;
		netConfig.ConnectionMaxSendBufferSize = ConnectionMaxSendBufferSize;
		netConfig.MaxPacketSize = MaxPacketSize;
		netConfig.MaxConnectionCount = MaxConnectionCount;
		netConfig.MaxMessagePoolCount = MaxMessagePoolCount;
		netConfig.ExtraMessagePoolCount = ExtraMessagePoolCount;
		netConfig.PerformancePacketMillisecondsTime = PerformancePacketMillisecondsTime;

		return netConfig;
	}
};


class EchoServer
{
public:
	EchoServer() = default;
	~EchoServer() = default;

	int Init(ServerAppConfig serverConfig)
	{
		m_pIOCPServer = std::make_unique<NetLib::IOCPServerNet>();

		m_Config = serverConfig;
		auto netConfig = serverConfig.GetNetConfig();

		auto ServerStartResult = m_pIOCPServer->Start(netConfig);
		if (ServerStartResult != NetLib::NetResult::Success) {
			printf("ServerStartError! ErrorCode: %d\n", (int)ServerStartResult);
			return -1;
		}
		
		return 0;
	}

	void Run()
	{
		m_IsRun = true;

		auto MaxPacketSize = m_Config.MaxPacketSize + 1;
		auto pBuf = new char[MaxPacketSize];
		ZeroMemory(pBuf, sizeof(char) * MaxPacketSize);
		INT32 waitTimeMillisec = 1;

		while (m_IsRun)
		{
			INT8 operationType = 0;
			INT32 connectionIndex = 0;
			INT16 copySize = 0;

			//여기 함수 개선하기(위의 동적할당 계속 되어서)

			//WorkThread의 함수들을 불러와서 처리한다.

			if (m_pIOCPServer->GetNetworkMessage(operationType, connectionIndex, pBuf, copySize, waitTimeMillisec) == false)
			{
				continue;
			}

			auto msgType = (NetLib::MessageType)operationType;

			switch (msgType)
			{
			case NetLib::MessageType::Connection:
				printf("On Connect %d\n", connectionIndex);
				break;
			case NetLib::MessageType::Close:
				printf("On DisConnect %d\n", connectionIndex);
				break;
			case NetLib::MessageType::OnRecv:
				{
					//PACKET_HEADER* pHeader = reinterpret_cast<PACKET_HEADER*>(pBuf);
					m_pIOCPServer->SendPacket(connectionIndex, pBuf, copySize);
				}
				break;
			}
		}
	}

	void Stop()
	{
		m_IsRun = false;

		m_pIOCPServer->End();
	}

	NetLib::IOCPServerNet* GetIOCPServer() { return  m_pIOCPServer.get(); }


private:
	std::unique_ptr<NetLib::IOCPServerNet> m_pIOCPServer;
	

	ServerAppConfig m_Config;

	bool m_IsRun = false;
};