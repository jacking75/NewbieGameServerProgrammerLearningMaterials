#include <thread>
#include <chrono>

#include "../ServerNetLib/ServerNetErrorCode.h"
#include "../ServerNetLib/Define.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "ConsoleLogger.h"
#include "LobbyManager.h"
#include "PacketProcess.h"
#include "UserManager.h"
#include "Main.h"

using LOG_TYPE = NServerNetLib::LOG_TYPE;
using NET_ERROR_CODE = NServerNetLib::NET_ERROR_CODE;

namespace NLogicLib
{
	Main::Main()
	{
	}

	Main::~Main()
	{
		Release();
	}

	ERROR_CODE Main::Init()
	{
		m_pLogger = std::make_unique<ConsoleLog>();

		LoadConfig();

		m_pNetwork = std::make_unique<NServerNetLib::TcpNetwork>();
		auto result = m_pNetwork->Init(m_pServerConfig.get(), m_pLogger.get());
			
		if (result != NET_ERROR_CODE::NONE)
		{
			m_pLogger->Write(LOG_TYPE::L_ERROR, "%s | Init Fail. NetErrorCode(%s)", __FUNCTION__, (short)result);
			return ERROR_CODE::MAIN_INIT_NETWORK_INIT_FAIL;
		}

		
		m_pUserMgr = std::make_unique<UserManager>();
		m_pUserMgr->Init(m_pServerConfig->MaxClientCount);

		m_pLobbyMgr = std::make_unique<LobbyManager>();
		m_pLobbyMgr->Init({ m_pServerConfig->MaxLobbyCount, 
							m_pServerConfig->MaxLobbyUserCount,
							m_pServerConfig->MaxRoomCountByLobby, 
							m_pServerConfig->MaxRoomUserCount },
						m_pNetwork.get(), m_pLogger.get());

		m_pPacketProc = std::make_unique<PacketProcess>();
		m_pPacketProc->Init(m_pNetwork.get(), m_pUserMgr.get(), m_pLobbyMgr.get(), m_pServerConfig.get(), m_pLogger.get());

		m_IsRun = true;

		m_pLogger->Write(LOG_TYPE::L_INFO, "%s | Init Success. Server Run", __FUNCTION__);
		return ERROR_CODE::NONE;
	}

	void Main::Release() 
	{
		if (m_pNetwork) {
			m_pNetwork->Release();
		}
	}

	void Main::Stop()
	{
		m_IsRun = false;
	}

	void Main::Run()
	{
		while (m_IsRun)
		{
			m_pNetwork->Run();

			while (true)
			{				
				auto packetInfo = m_pNetwork->GetPacketInfo();

				if (packetInfo.PacketId == 0)
				{
					break;
				}
				else
				{
					m_pPacketProc->Process(packetInfo);
				}
			}

			m_pPacketProc->StateCheck();
		}
	}

	ERROR_CODE Main::LoadConfig()
	{
		//TODO argument로 설정 정보 받기

		m_pServerConfig = std::make_unique<NServerNetLib::ServerConfig>();

		m_pServerConfig->Port = 32452;
		m_pServerConfig->BackLogCount = 128;
		m_pServerConfig->MaxClientCount = 1000;

		m_pServerConfig->MaxClientSockOptRecvBufferSize = 10240;
		m_pServerConfig->MaxClientSockOptSendBufferSize = 10240;
		m_pServerConfig->MaxClientRecvBufferSize = 8192;
		m_pServerConfig->MaxClientSendBufferSize = 8192;

		m_pServerConfig->IsLoginCheck = false;

		m_pServerConfig->ExtraClientCount = 64;
		m_pServerConfig->MaxLobbyCount = 2;
		//m_pServerConfig->MaxLobbyUserCount = 50;
		m_pServerConfig->MaxRoomCountByLobby = 20;
		m_pServerConfig->MaxRoomUserCount = 4;
	    
		m_pLogger->Write(NServerNetLib::LOG_TYPE::L_INFO, "%s | Port(%d), Backlog(%d)", __FUNCTION__, m_pServerConfig->Port, m_pServerConfig->BackLogCount);
		m_pLogger->Write(NServerNetLib::LOG_TYPE::L_INFO, "%s | IsLoginCheck(%d)", __FUNCTION__, m_pServerConfig->IsLoginCheck);
		return ERROR_CODE::NONE;
	}
		
}