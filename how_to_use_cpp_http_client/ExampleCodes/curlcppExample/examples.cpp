#include <string>
#include <iostream>

#include "curl/curl.h"
#include "curlcpp/curl_easy.h"
#include "curlcpp/curl_form.h"
#include "curlcpp/curl_pair.h"
#include "curlcpp/curl_receiver.h"
#include "curlcpp/curl_exception.h"
#include "curlcpp/curl_sender.h"

namespace examples
{
	// 간단한 요청
	void SimpleRequest()
	{
		curl::curl_easy handle;

		handle.add<CURLOPT_URL>("http://example.com/");
		handle.add<CURLOPT_FOLLOWLOCATION>(1L);

		try 
		{
			handle.perform();
		}
		catch (curl::curl_easy_exception& error) 
		{
			std::cerr << error.what() << std::endl;
		}
	}

	// 세션 정보 출력
	void GetSessionInfos()
	{
		std::ostringstream stream;
		curl::curl_ios<std::ostringstream> ios(stream);
		curl::curl_easy handle(ios);

		handle.add<CURLOPT_URL>("http://example.com/");
		handle.add<CURLOPT_FOLLOWLOCATION>(1L);

		try 
		{
			handle.perform();

			// 출력하고자하는 정보 얻기
			auto x = handle.get_info<CURLINFO_CONTENT_TYPE>();

			// 정보 출력
			std::cout << x.get() << std::endl;

		}
		catch (curl::curl_easy_exception& error) 
		{
			std::cerr << error.what() << std::endl;
			error.print_traceback();
		}
	}

	// 응답 정보 파일로 저장
	void StoreResponseDataFromFile(const std::string& path)
	{
		std::ofstream file;
		file.open(path);

		curl::curl_ios<std::ostream> writer(file);
		curl::curl_easy handle(writer);

		handle.add<CURLOPT_URL>("http://example.com/");
		handle.add<CURLOPT_FOLLOWLOCATION>(1L);

		try 
		{
			handle.perform();
		}
		catch (curl::curl_easy_exception& error) 
		{
			std::cerr << error.what() << std::endl;
			error.print_traceback();
		}

		file.close();
	}

	// 응답 정보 변수에 저장
	void StoreResponseDataFromMemory()
	{
		std::ostringstream mem;

		curl::curl_ios<std::ostream> writer(mem);
		curl::curl_easy handle(writer);

		handle.add<CURLOPT_URL>("http://example.com/");
		handle.add<CURLOPT_FOLLOWLOCATION>(1L);

		try
		{
			handle.perform();

			std::cout << mem.str() << std::endl;
		}
		catch (curl::curl_easy_exception& error)
		{
			std::cerr << error.what() << std::endl;
			error.print_traceback();
		}
	}

	// Receiver / Sender 사용하기
	void UsedReceiverSender()
	{
		std::string request = "GET / HTTP/1.0\r\nHost: example.com\r\n\r\n";
		curl::curl_easy handle;

		// 연결만하기
		try 
		{
			handle.add<CURLOPT_URL>("http://example.com/");
			handle.add<CURLOPT_CONNECT_ONLY>(true);
			handle.perform();
		}
		catch (curl::curl_easy_exception& error) 
		{
			curl::curlcpp_traceback errors = error.get_traceback();
			error.print_traceback();
		}

		// 요청 송신하기
		curl::curl_sender<std::string> sender(handle);
		sender.send(request);

		std::cout << "Sent bytes: " << sender.get_sent_bytes() << std::endl;

		// 응답 수신하기
		for (;;)
		{
			try 
			{
				curl::curl_receiver<char, 1024> receiver;
				receiver.receive(handle);

				const auto received_bytes = receiver.get_received_bytes();
				if (received_bytes == 0)
				{
					break;
				}

				std::cout << "Receiver bytes: " << received_bytes << std::endl;

			}
			catch (curl::curl_easy_exception& error) 
			{
				break;
			}
		}
	}

	// JSON 요청 보내기
	const bool SendRequestJson(const char* url, const char* body)
	{
		curl::curl_easy handle;
		curl_slist* header = nullptr;
		header = curl_slist_append(header, "Content-Type: application/json");

		const auto doby_len = strlen(body);

		handle.add<CURLOPT_URL>(url);
		handle.add<CURLOPT_POST>(1L);
		handle.add<CURLOPT_HTTPHEADER>(header);
		handle.add<CURLOPT_POSTFIELDS>(body);
		handle.add<CURLOPT_POSTFIELDSIZE>(doby_len);

		try 
		{
			handle.perform();
		}
		catch (curl::curl_easy_exception& error) 
		{
			std::cerr << error.what() << std::endl;
			return false;
		}

		return true;
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
	examples::StoreResponseDataFromFile("./output.txt");
	examples::StoreResponseDataFromMemory();
	examples::GetSessionInfos();
	examples::UsedReceiverSender();
	examples::SendRequestJson(auth_check_url, auth_check_body_data);
	examples::SendRequestJson(inapp_check_url, inapp_check_body_data);

	return 0;
}