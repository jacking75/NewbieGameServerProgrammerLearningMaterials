#include <iostream>
#include "httplib.h"

namespace examples
{
	void SimpleRequest()
	{
		httplib::Client connection("example.com", 80);
		httplib::Result result = connection.Post("/");

		if (result == nullptr)
		{
			std::cout << "error code: " << result.error() << std::endl;
			return;
		}

		std::cout << result->status << std::endl;
		std::cout << result->body << std::endl;
	}

	void SendJsonRequest(const std::string& host, const int port, const std::string& path, const char* body) 
	{
		httplib::Headers headers = {
			{"Content-Type", "application/json"}
		};

		httplib::Client connection(host, port);
		httplib::Result result = connection.Post(path, headers, body, "application/json");

		if (result == nullptr)
		{
			std::cout << "error code: " << result.error() << std::endl;
			return;
		}

		std::cout << result->status << std::endl;
		std::cout << result->body << std::endl;
	}
}

int main() 
{
	const auto auth_check_body = R"(
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
	examples::SendJsonRequest("127.0.0.1", 11502, "/AuthCheck", auth_check_body);
	examples::SendJsonRequest("127.0.0.1", 11502, "/InAppCheck", inapp_check_body_data);

	return 0;
}