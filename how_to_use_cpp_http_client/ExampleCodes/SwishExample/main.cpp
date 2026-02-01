#include <swish/swish.h>

using namespace swish;

int main() {
	auto client = Client();
	auto [response, status] = client.Get("https://github.com");
	if (IsOK(status))
	{
		std::cout << response.body.ToString() << "\n";
	}
	else
	{
		std::cerr << "The following Error occured: " << InterpretStatusCode(status) << std::endl;
	}

	return EXIT_SUCCESS;
}