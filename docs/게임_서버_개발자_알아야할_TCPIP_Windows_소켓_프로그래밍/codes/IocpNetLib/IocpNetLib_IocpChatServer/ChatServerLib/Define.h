#pragma once


#include "../../IocpNetLib/NetDefine.h"


namespace ChatServerLib
{
	struct ChatServerConfig : NetLib::NetConfig
	{
		int PostMessagesThreadsCount = 1;
		int StartRoomNummber = 0;
		int MaxRoomCount = 0;
		int MaxRoomUserCount = 0;

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

}