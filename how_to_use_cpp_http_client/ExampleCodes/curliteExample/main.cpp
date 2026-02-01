#include <string>
#include <iostream>
#include <cstring>
#include "curl/curl.h"
#include "curlite.hpp"

namespace examples
{
	// 간단한 요청
	void SimpleRequest()
	{
		try
		{
			curlite::Easy handle;

			handle.set(CURLOPT_URL, "http://example.com");
			handle.set(CURLOPT_FOLLOWLOCATION, true);
			std::cout << handle << std::endl;
		}
		catch (std::exception& e)
		{
			std::cerr << "Got an exception: " << e.what() << std::endl;
		}
	}

	// JSON 송/수신하기
	void SendRequestJson(const char* url, const char* body, const bool isPost = false)
	{
		curl_slist* header = NULL;
		header = curl_slist_append(header, "Content-Type: application/json");

		int32_t body_len = (int32_t)strlen(body);

		try
		{
			curlite::Easy handle;
			handle.set(CURLOPT_URL, url);

			if (isPost == true)
			{
				handle.set(CURLOPT_POST, 1L);
			}
			
			handle.set(CURLOPT_HTTPHEADER, header);
			handle.set(CURLOPT_POSTFIELDS, body);
			handle.set(CURLOPT_POSTFIELDSIZE, body_len);
			handle.set(CURLOPT_FOLLOWLOCATION, true);

			std::cout << handle << std::endl;
		}
		catch (std::exception& e)
		{
			std::cerr << "Got an exception: " << e.what() << std::endl;
		}
	}
}

int main()
{
	const char* auth_check_url = "http://127.0.0.1:11502/AuthCheck";
	const char* inapp_check_url = "http://127.0.0.1:11502/InAppCheck";
	const auto auth_check_body_data = R"(
		{
			"AuthID":"test01",
			"AuthToken":"DUWPQCFN5DQF4P"
		}
	)";

	const auto inapp_check_body_data = R"(
		{
			"Receipt":"WkuOATWDQ909OET9cBjVEXEgI3KqTTbThNFe206bywlkSBiUD1hgrCltj3g1a84d"
		}
	)";

	examples::SimpleRequest();

	examples::SendRequestJson(auth_check_url, auth_check_body_data);
	examples::SendRequestJson(auth_check_url, auth_check_body_data, true);
	examples::SendRequestJson(inapp_check_url, inapp_check_body_data, true);

	return 0;
}