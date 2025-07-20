// TGS_ConsoleHost.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

#include <iostream>

#include "flags.h"
#include "EchoServer.h"

ServerAppConfig ParseConfig(int argc, char* argv[]);

int main(int argc, char* argv[])
{		
	auto serverConfig = ParseConfig(argc, argv);
	
	NetLib::LogFuncPtr = [](const int eLogInfoType, const char* szLogMsg) {
		UNREFERENCED_PARAMETER(eLogInfoType);
		printf("%s\r\n", szLogMsg);
	};

	EchoServer main;
	main.Init(serverConfig);

	std::thread logicThread([&]() {
		main.Run(); }
	);

	
	std::cout << "Server quits when you press (quit) !!!" << std::endl;

	while (true)
	{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);
		
		if (inputCmd == "quit")
		{
			break;
		}
	}

	main.Stop();
	logicThread.join();
}


ServerAppConfig ParseConfig(int argc, char* argv[])
{
	//--port = 11021 --WorkThreadCount = 4 --MaxRecvOverlappedBufferSize = 600 --MaxSendOverlappedBufferSize = 600 --MaxRecvConnectionBufferCount = 1024 --MaxSendConnectionBufferCount = 1024  --MaxPacketSize = 1024 --MaxConnectionCount = 1024 --MaxMessagePoolCount = 1024 --ExtraMessagePoolCount = 128
	auto serverConfig = ServerAppConfig();

	const flags::args args(argc, argv);
	const auto port = args.get<UINT16>("port"); 
	if (!port) { 
		std::cerr << "No Port. :(" << std::endl;
		return serverConfig;
	} 
	std::cout << "Port: " << *port << std::endl;


	const auto WorkThreadCount = args.get<INT32>("WorkThreadCount");
	if (!WorkThreadCount) {
		std::cerr << "No WorkThreadCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "WorkThreadCount: " << *WorkThreadCount << std::endl;
			
	const auto MaxRecvConnectionBufferSize = args.get<INT32>("MaxRecvConnectionBufferSize");
	if (!MaxRecvConnectionBufferSize) {
		std::cerr << "No MaxRecvConnectionBufferSize :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxRecvConnectionBufferCount: " << *MaxRecvConnectionBufferSize << std::endl;


	const auto MaxSendConnectionBufferSize = args.get<INT32>("MaxSendConnectionBufferSize");
	if (!MaxSendConnectionBufferSize) {
		std::cerr << "No MaxSendConnectionBufferSize :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxSendConnectionBufferCount: " << *MaxSendConnectionBufferSize << std::endl;


	const auto MaxPacketSize = args.get<INT32>("MaxPacketSize");
	if (!MaxPacketSize) {
		std::cerr << "No MaxPacketSize. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxSendConnectionBufferCount: " << *MaxSendConnectionBufferSize << std::endl;


	const auto MaxConnectionCount = args.get<INT32>("MaxConnectionCount");
	if (!MaxSendConnectionBufferSize) {
		std::cerr << "No MaxConnectionCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxConnectionCount: " << *MaxConnectionCount << std::endl;


	const auto MaxMessagePoolCount = args.get<INT32>("MaxMessagePoolCount");
	if (!MaxMessagePoolCount) {
		std::cerr << "No MaxMessagePoolCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxMessagePoolCount: " << *MaxMessagePoolCount << std::endl;


	const auto ExtraMessagePoolCount = args.get<INT32>("ExtraMessagePoolCount");
	if (!ExtraMessagePoolCount) {
		std::cerr << "No ExtraMessagePoolCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "ExtraMessagePoolCount: " << *ExtraMessagePoolCount << std::endl;

		
	serverConfig.PortNumber = *port;
	serverConfig.WorkThreadCount = *WorkThreadCount;
	serverConfig.ConnectionMaxRecvBufferSize = *MaxRecvConnectionBufferSize;
	serverConfig.ConnectionMaxSendBufferSize = *MaxSendConnectionBufferSize;
	serverConfig.MaxPacketSize = *MaxPacketSize;
	serverConfig.MaxConnectionCount = *MaxConnectionCount;
	serverConfig.MaxMessagePoolCount = *MaxMessagePoolCount;
	serverConfig.ExtraMessagePoolCount = *ExtraMessagePoolCount;
	serverConfig.PerformancePacketMillisecondsTime = 0;

	serverConfig.PostMessagesThreadsCount = 1;
	
	return serverConfig;
}



