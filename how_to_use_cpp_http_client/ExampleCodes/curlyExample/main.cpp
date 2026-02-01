#include <string>
#include <iostream>
#include "curl/curl.h"
#include "curly.hpp"

namespace examples
{
	// 간단한 요청
	void SimpleRequest()
	{
		auto request = curly_hpp::request_builder()
			.method(curly_hpp::http_method::GET)
			.url("http://example.com")
			.send();

		request.wait();

		if (request.is_done())
		{
			auto response = request.take();

			std::cout << "Status code: " << response.http_code() << std::endl;
			std::cout << "Body content: " << response.content.as_string_view() << std::endl;
			std::cout << "Content Length: " << response.headers["content-length"] << std::endl << std::endl;
		}
		else
		{
			std::cout << "Error message: " << request.get_error() << std::endl;
		}
	}

	// JSON 송/수신
	void SendRequestJson(const char* url, const char* body, const curly_hpp::http_method method)
	{
		// 헤더 세팅
		curly_hpp::header_ilist_t headers = {
			{"Content-Type", "application/json"}
		};

		// 요청 정보 세팅 후 바로 송신
		auto request = curly_hpp::request_builder()
			.method(method)
			.url(url)
			.headers(headers)
			.content(body)
			.send();

		// 응답 대기 (내부에서 상태 변수를 락걸고 변경한다.)
		request.wait();

		// 응답 처리 (내부에서 상태 변수를 락걸고 변경한다. 즉, 락을 획득했다는 건 응답 데이터를 받았다는 뜻이다.)
		if (request.is_done())
		{
			// 응답 데이터를 가져온다.
			auto response = request.take();

			std::cout << "Status code: " << response.http_code() << std::endl;
			std::cout << "Body content: " << response.content.as_string_view() << std::endl;
			std::cout << "Content Length: " << response.headers["content-length"] << std::endl << std::endl;
		}
		else
		{
			std::cout << "Error message: " << request.get_error() << std::endl;
		}
	}
}

void Worker(const char* url, const char* body, const curly_hpp::http_method method)
{
	for (int i = 0; i < 100; i++)
	{
		examples::SendRequestJson(url, body, method);
		Sleep(1);
	}
}

int main()
{
	// 송신 담당 인스턴스 선언
	curly_hpp::performer performer;

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

	// 싱글 스레드로 보내기
	examples::SendRequestJson(auth_check_url, auth_check_body_data, curly_hpp::http_method::POST);
	examples::SendRequestJson(inapp_check_url, inapp_check_body_data, curly_hpp::http_method::POST);

	// 멀티 스레드로 보내기
	std::vector<std::thread> workers;
	for (int i = 0; i < 5; i++)
	{
		workers.push_back(std::thread(Worker, auth_check_url, auth_check_body_data, curly_hpp::http_method::POST));
	}

	for (int i = 0; i < 5; i++)
	{
		workers.push_back(std::thread(Worker, inapp_check_url, inapp_check_body_data, curly_hpp::http_method::POST));
	}

	for (int i = 0; i < workers.size(); i++)
	{
		workers[i].join();
	}

	return 0;
}