#pragma comment(lib, "ws2_32")
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "happyhttp.h"

void ReceiveResponseData(happyhttp::Connection& conn)
{
	while (conn.outstanding())
	{
		conn.pump();
	}
}

void OnBegin(const happyhttp::Response* response, void* userData)
{
	printf("BEGIN (%d %s)\n", response->getstatus(), response->getreason());
}

void OnData(const happyhttp::Response* response, void* userData, const unsigned char* responseData, int responseDataLength)
{
	fwrite(responseData, 1, responseDataLength, stdout);
}

void OnComplete(const happyhttp::Response* response, void* userData)
{
	std::cout << std::endl;
	std::cout << "Done response data receive" << std::endl;
}

void ExampleMethodGet()
{
	const char* host = "www.postman-echo.com";
	const int32_t port = 80;
	const char* body_data = "fruit=mango&price=3";
	const int32_t body_data_len = (int32_t)strlen(body_data);

	// happyhttp 인스턴스 생성
	happyhttp::Connection conn(host, port);

	// 콜백 함수 등록
	conn.setcallbacks(OnBegin, OnData, OnComplete, 0);

	// 요청
	conn.request(
		"GET",
		"/get",
		NULL,
		(const unsigned char*)body_data,
		body_data_len
	);

	ReceiveResponseData(conn);
}

void ExampleMethodPost()
{
	const char* host = "www.postman-echo.com";
	const int32_t port = 80;
	
	const char* header[] =
	{
		"Connection", "close",
		"Content-type", "application/x-www-form-urlencoded",
		"Accept", "text/plain",
		0
	};
	const char* body_data = "fruit=mango&price=3";
	const int32_t body_data_len = (int)strlen(body_data);

	// happyhttp 인스턴스 생성
	happyhttp::Connection conn(host, port);

	// 콜백 함수 등록
	conn.setcallbacks(OnBegin, OnData, OnComplete, 0);

	// 요청
	conn.request(
		"POST",
		"/post",
		header,
		(const unsigned char*)body_data,
		body_data_len
	);

	ReceiveResponseData(conn);
}

void ExampleMethodPostLowLevelInterface()
{
	const char* host = "www.postman-echo.com";
	const int32_t port = 80;

	const char* body_data = "fruit=mango&price=3";
	const int32_t body_data_len = (int)strlen(body_data);

	// happyhttp 인스턴스 생성
	happyhttp::Connection conn(host, port);

	// 콜백 함수 등록
	conn.setcallbacks(OnBegin, OnData, OnComplete, 0);

	// 헤더 세팅
	conn.putheader("Connection", "close");
	conn.putheader("Content-type", "application/x-www-form-urlencoded");
	conn.putheader("Accept", "text/plain");
	conn.endheaders();

	// 메소드 세팅
	conn.putrequest("POST", "/post");

	// 송신
	conn.send((const unsigned char*)body_data, body_data_len);

	ReceiveResponseData(conn);
}

bool DoJsonRequest (
	const char* host, 
	const int32_t port, 
	const char* url, 
	const char* method, 
	const char* body_data)
{
	const int32_t body_data_len = strlen(body_data);

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

int main()
{
	if (NetInit() == false)
	{
		return 0;
	}

	const char* host = "127.0.0.1";
	const int32_t port = 11502;

	const auto auth_check_body_data = 
		R"(
			{
				"AuthID":"test01",
				"AuthToken":"DUWPQCFN5DQF4P"
			}
		)";


	const auto inapp_check_body_data = 
		R"(
			{
				"Receipt":"WkuOATWDQ909OET9cBjVEXEgI3KqTTbThNFe206bywlkSBiUD1hgrCltj3g1a84d"
			}
		)";

	try
	{
		DoJsonRequest(host, port, "/AuthCheck", "GET", auth_check_body_data);
		DoJsonRequest(host, port, "/AuthCheck", "POST", auth_check_body_data);
		DoJsonRequest(host, port, "/InAppCheck", "POST", inapp_check_body_data);
	}
	catch (happyhttp::Wobbly& e)
	{
		std::cout << "happyhttp Exception: " << e.what() << std::endl;
	}

	WSACleanup();

	return 0;
}