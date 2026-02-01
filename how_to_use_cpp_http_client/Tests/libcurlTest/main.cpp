#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <curl/curl.h>

#if _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

const bool DoJsonRequest(
	const char* url,
	const char* body_data,
	const bool isPostMethod = false,
	const bool isSSL = false);

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

				DoJsonRequest(server_url.c_str(), body_data, true);
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

				DoJsonRequest(server_url.c_str(), body_data, true);
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

size_t OnResponse(char* ptr, size_t size, size_t nmemb, std::string* stream)
{
	auto realsize = size * nmemb;

	// std::cout << ptr << std::endl;

	InterlockedIncrement(&testset::g_tps);
	InterlockedIncrement64(&testset::g_success_count);

	return realsize;
}

const bool DoJsonRequest(
	const char* url,
	const char* body_data, 
	const bool isPostMethod,
	const bool isSSL)
{
	CURL* curl = curl_easy_init();

	if(curl == NULL)
	{
		std::cout << "Failed intialize curl instance" << std::endl;
		return false;
	}

	const auto body_data_len = strlen(body_data);
	curl_slist* header = nullptr;

	// URL
	curl_easy_setopt(curl, CURLOPT_URL, url);

	// HEADER
	header = curl_slist_append(header, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

	// SSL
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, isSSL);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, isSSL);

	// METHOD
	if (isPostMethod == true)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
	}

	// BODY DATA
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_data);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body_data_len);

	// CALLBACK
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnResponse);

	// SEND
	CURLcode res = curl_easy_perform(curl);

	// CLEANUP
	curl_easy_cleanup(curl);
	curl_slist_free_all(header);

	if (res != CURLE_OK)
	{
		std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		return false;
	}

	return true;
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