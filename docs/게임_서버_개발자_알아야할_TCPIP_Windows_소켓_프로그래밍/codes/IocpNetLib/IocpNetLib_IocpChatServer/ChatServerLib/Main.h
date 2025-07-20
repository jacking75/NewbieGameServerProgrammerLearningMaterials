#pragma once

#include <memory>

#include "../../IocpNetLib/IOCPServerNet.h"
#include "Define.h"
#include "UserManager.h"
#include "RoomManager.h"
#include "PacketManager.h"

//TODO printf 함수로 로그 찍는 것을 정상적인 로그로 변경해야 한다

namespace ChatServerLib 
{
	class Main
	{
	public:
		Main() = default;
		~Main() = default;

		int Init(ChatServerConfig serverConfig);
		
		void Run();
		
		void Stop();
		
		NetLib::IOCPServerNet* GetIOCPServer() { return  m_pIOCPServer.get(); }


	private:
		std::unique_ptr<NetLib::IOCPServerNet> m_pIOCPServer;
		std::unique_ptr<UserManager> m_pUserManager;
		std::unique_ptr<PacketManager> m_pPacketManager;
		std::unique_ptr<RoomManager> m_pRoomManager;
				
		ChatServerConfig m_Config;
				
		bool m_IsRun = false;
	};


}