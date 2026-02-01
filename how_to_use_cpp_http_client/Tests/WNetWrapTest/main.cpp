#include <iostream>
#include <thread>
#include <vector>
#include "wnetwrap.h"

#if _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

void DoJsonRequestMethodPost(const char* url, const char* body_data);

namespace testset
{
	enum eTestType : int32_t
	{
		None = 0,
		AuthCheck = 1101,
		InAppCheck = 1102,
	};

	bool g_is_running;

	std::string g_server_url;
	int32_t g_test_time_sec;
	int32_t g_worker_count;
	eTestType g_test_type;

	int64_t g_success_count = 0;
	uint32_t g_tps = 0;

	void Worker()
	{
		const std::string server_url = g_server_url;

		while (g_is_running == true)
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

				DoJsonRequestMethodPost(server_url.c_str(), body_data);
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

				DoJsonRequestMethodPost(server_url.c_str(), body_data);
			}
			break;

			default:
				break;
			}

			Sleep(1);
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

void DoJsonRequestMethodPost(const char* url, const char* body_data)
{
	wrap::Response res = wrap::HttpsRequest(
		wrap::Url{ url },
		wrap::Header{
			{"Connection", "close"}, 
			{ "Content-type", "application/json" }, 
			{ "Accept", "text/plain" }
		},
		wrap::Method{ "POST" },
		wrap::Body{ body_data }
	);

	InterlockedIncrement(&testset::g_tps);
	InterlockedIncrement64(&testset::g_success_count);

	//std::cout << res.text << std::endl;
}

// -- *.exe - http://183.111.164.120:9090/[TEST_TYPE] 60 1 1101
int main(int argc, const char* args[])
{
	testset::g_server_url = args[1];
	testset::g_test_time_sec = atoi(args[2]);
	testset::g_worker_count = atoi(args[3]);
	testset::g_test_type = (testset::eTestType)atoi(args[4]);

	testset::Run();

#if _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}