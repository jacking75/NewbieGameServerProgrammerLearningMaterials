#pragma comment(lib, "ws2_32")
#include <iostream>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "happyhttp.h"

#if _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

bool DoJsonRequest(
	const char* host,
	const int32_t port,
	const char* url,
	const char* method,
	const char* body_data);

namespace testset
{
	enum eTestType : int32_t
	{
		None = 0,
		AuthCheck = 1101,
		InAppCheck = 1102,
	};

	bool g_is_running;

	std::string g_server_ip;
	int32_t g_server_port;
	int32_t g_test_time_sec;
	int32_t g_worker_count;
	eTestType g_test_type;

	int64_t g_success_count = 0;
	uint32_t g_tps = 0;

	void Worker()
	{
		const std::string server_ip = g_server_ip;
		const int32_t server_port = g_server_port;

		while (g_is_running == true)
		{
			try
			{
				switch (g_test_type)
				{
				case eTestType::AuthCheck:
				{
					const auto body_data =
						R"(
					{
						"AuthID":"test01",
						"AuthToken":"DUWPQCFN5DQF4P"
					}
				)";

					DoJsonRequest(server_ip.c_str(), server_port, "/AuthCheck", "POST", body_data);
				}

				break;

				case eTestType::InAppCheck:
				{
					const auto body_data =
						R"(
					{
						"Receipt":"WkuOATWDQ909OET9cBjVEXEgI3KqTTbThNFe206bywlkSBiUD1hgrCltj3g1a84d"
					}
				)";

					DoJsonRequest(server_ip.c_str(), server_port, "/InAppCheck", "POST", body_data);
				}
				break;

				default:
					break;
				}

				Sleep(1);
			}
			catch (happyhttp::Wobbly& e)
			{
				std::cout << "happyhttp Exception: " << e.what() << std::endl;
			}
		}
	}

	void Monitor()
	{
		while (g_is_running == true)
		{
			Sleep(999);
			printf("[Monitor] TPS: %d\n", g_tps);

			g_tps = 0;
		}
	}

	void Timer()
	{
		Sleep(g_test_time_sec * 1000);
		g_is_running = false;
	}

	void Run()
	{
		printf("[Start] WorkerCount: %d, TestTime(Sec): %d\n", g_worker_count, g_test_time_sec);

		g_is_running = true;

		std::thread timer = std::thread(Timer);

		std::thread monitor = std::thread(Monitor);

		std::vector<std::thread> threads;
		for (int32_t i = 0; i < g_worker_count; i++)
		{
			threads.emplace_back(std::thread(Worker));
		}

		timer.join();

		monitor.join();

		auto iter = threads.begin();
		for (iter; iter != threads.end();)
		{
			iter->join();
			iter = threads.erase(iter);
		}

		printf("[Done] SuccessCount: %lld\n", g_success_count);
	}
}

void ReceiveResponseData(happyhttp::Connection& conn)
{
	while (conn.outstanding())
	{
		conn.pump();
	}
}

void OnBegin(const happyhttp::Response* response, void* userData)
{
}

void OnData(const happyhttp::Response* response, void* userData, const unsigned char* responseData, int responseDataLength)
{
}

void OnComplete(const happyhttp::Response* response, void* userData)
{
	InterlockedIncrement(&testset::g_tps);
	InterlockedIncrement64(&testset::g_success_count);
}

bool DoJsonRequest (
	const char* host, 
	const int32_t port, 
	const char* url, 
	const char* method, 
	const char* body_data)
{
	const int32_t body_data_len = (int32_t)strlen(body_data);

	if (body_data_len <= 0)
	{
		return false;
	}

	const char* header[] = {
		"Connection", "close",
		"Content-type", "application/json",
		"Accept", "text/plain",
		0
	};

	happyhttp::Connection conn(host, port);
	conn.setcallbacks(OnBegin, OnData, OnComplete, 0);

	conn.request(
		method,
		url,
		header,
		(const unsigned char*)body_data,
		body_data_len
	);

	ReceiveResponseData(conn);

	conn.close();

	return true;
}

bool NetInit()
{
	WSAData wsaData;
	int32_t errcode = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errcode != NO_ERROR)
	{
		std::cout << "WSAStartup failed with error: " << errcode << std::endl;
		return false;
	}
	return true;
}

// -- *.exe - 183.111.164.120 9090 60 1 1101
int main(int argc, const char* args[])
{
	testset::g_server_ip = args[1];
	testset::g_server_port = atoi(args[2]);
	testset::g_test_time_sec = atoi(args[3]);
	testset::g_worker_count = atoi(args[4]);
	testset::g_test_type = (testset::eTestType)atoi(args[5]);

	if (NetInit() == false)
	{
		return 0;
	}

	testset::Run();

	WSACleanup();

#if _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}