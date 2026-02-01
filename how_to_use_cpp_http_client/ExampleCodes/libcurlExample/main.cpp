#include <string>
#include <iostream>
#include <cstring>
#include <curl/curl.h>

size_t OnResponse(char* ptr, size_t size, size_t nmemb, std::string* stream)
{
	auto realsize = size * nmemb;

	std::cout << ptr << std::endl;

	return realsize;
}

const bool ExampleMethodGet()
{
	const char* url = "https://www.postman-echo.com/";

	CURL* curl = curl_easy_init();

	if (curl == NULL)
	{
		return false;
	}

	// URL
	curl_easy_setopt(curl, CURLOPT_URL, url);

	// METHOD
	// DEFAULT : GET

	// EXECUTE
	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		return false;
	}

	curl_easy_cleanup(curl);

	// callback 함수가 없으면 stdout 으로 출력된다.

	return true;
}

const bool ExampleMethodPost()
{
	const char* url_post = "https://www.postman-echo.com/";

	CURL* curl = curl_easy_init();

	if (curl == NULL)
	{
		return false;
	}

	curl_slist* header = nullptr; 

	// URL
	curl_easy_setopt(curl, CURLOPT_URL, url_post);

	// METHOD
	curl_easy_setopt(curl, CURLOPT_POST, 1L);

	// HEADERS
	header = curl_slist_append(header, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

	// SSL
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);

	// DATA
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "data");

	// EXECUTE
	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		return false;
	}

	curl_easy_cleanup(curl);

	// callback 함수가 없으면 stdout 으로 출력된다.

	return true;
}

const bool DoJsonRequest(
	const char* url, 
	const char* body_data, 
	const bool isPostMethod = false,
	const bool isSSL = false)
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

int main()
{
	const char* auth_check_url = "http://127.0.0.1:11502/AuthCheck";
	const char* inapp_check_url = "http://127.0.0.1:11502/InAppCheck";

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

	DoJsonRequest(auth_check_url, auth_check_body_data);
	DoJsonRequest(auth_check_url, auth_check_body_data, true);
	DoJsonRequest(inapp_check_url, inapp_check_body_data, true);

	return 0;
}