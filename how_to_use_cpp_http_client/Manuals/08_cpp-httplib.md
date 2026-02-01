# cpp-httplib

- 예제 경로 : `~/ExampleCodes/cpp-httplibExample`
- 원본 코드 : https://github.com/yhirose/cpp-httplib
	- Last Commit : 2023/06/10
	- Latest Release Version : 0.12.6

## 들어가기전에

- 해당 문서에서는 `Windows` 환경에서의 설치 방법만 설명한다.

## 라이브러리 특징

- 직관적인 API
- 크로스 플랫폼 지원
- **`C++ 11` 이상** 필요
- 모든 요청은 **동기로 동작**. (*비동기 지원 X*)
- **Header-Only** 라이브러리
- 서버 기능도 지원 (*해당 문서에서는 다루지 않는다.*)

## 라이브러리 설치하기

1. [GitHub](https://github.com/yhirose/cpp-httplib)에서 프로젝트를 다운로드한다.

2. 다운로드 한 프로젝트 폴더 내부의 `httplib.h`파일을 자신의 프로젝트로 옮긴다.

![install_02](../Images/cpp-httplib/install_02.png)

3. 해당 파일을 자신의 프로젝트에 포함시킨다.

![install_03](../Images/cpp-httplib/install_03.png)

## 예제 코드

### 예제1. 간단한 요청

```cpp
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
```

### 예제2. JSON 송/수신
```cpp
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
```

### 사용 예시
```cpp
#include <iostream>
#include "httplib.h"

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
```