#include <iostream>
#include "wnetwrap.h"

void ExampleMethodGet()
{
	wrap::Response res = wrap::HttpsRequest(
		wrap::Url{ "https://www.postman-echo.com/get" },
		wrap::Parameters{{"fruit", "mango"}, { "price","3" }}
	);

	std::cout << res.text << std::endl;
}

void ExampleMethodPost()
{
	wrap::Response res = wrap::HttpsRequest(
		wrap::Url{ "https://www.postman-echo.com/post" },
		wrap::Method{ "POST" },
		wrap::Parameters{
			{"fruit", "mango"}, 
			{ "price","3" }
		}
	);

	std::cout << res.text << std::endl;
}

void DoJsonRequestMethodGet(const char* url, const char* body_data)
{
	wrap::Response res = wrap::HttpsRequest(
		wrap::Url{ url },
		wrap::Header{
			{"Connection", "close"}, 
			{ "Content-type", "application/json" }, 
			{ "Accept", "text/plain" }
		},
		wrap::Body{body_data}
	);

	std::cout << res.text << std::endl;
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

	std::cout << res.text << std::endl;
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

	DoJsonRequestMethodGet(auth_check_url, auth_check_body_data);
	DoJsonRequestMethodPost(auth_check_url, auth_check_body_data);
	DoJsonRequestMethodPost(inapp_check_url, inapp_check_body_data);

	return 0;
}