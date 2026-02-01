# curlite

- 예제 코드 : `~/ExampleCodes/curliteExample`
- 원본 코드 : https://github.com/grynko/curlite
	- Last Commit : 2019/04/12
	- Latest Release Version : None

## 들어가기전에

- 해당 문서에서는 `Windows` 환경에서의 설치 방법만 설명한다.

- 해당 문서에서 설명할 `curlite` 라이브러리는 [libcurl](https://github.com/curl/curl)을 `C++`로 랩핑한 라이브러리다. 따라서 `curlite` 라이브러리를 적용하고자 하는 프로젝트에는 최소 **7.32 버전 이상의 `libcurl` 라이브러리가 설치되어 있어야**한다. ([`libcurl` 설치 방법](~/Manuals/libcurl.md))

## 라이브러리 특징

- `libcurl` 필요
- 직관적인 API
- **`C++ 11` 이상** 필요.
- **멀티스레드로 사용 불가능.**
- **현재 개발중인 라이브러리**

## 라이브러리 설치하기

1. [GitHub](https://github.com/grynko/curlite)에서 프로젝트를 다운로드한다.

2. 다운로드 한 프로젝트 폴더 내부의 다음 파일들을 자신의 프로젝트로 옮긴다.

![install_02_01](../Images/curlite/install_02.png)

3. 해당 파일들을 자신의 프로젝트에 포함시킨다.

![install_03](../Images/curlite/install_03.png)

## 예제 코드

### 예제 1. 간단한 요청
```cpp
void SimpleRequest()
{
	try
	{
		curlite::Easy handle;

		handle.set(CURLOPT_URL, "http://example.com");
		handle.set(CURLOPT_FOLLOWLOCATION, true);
		std::cout << handle;
	}
	catch (std::exception& e)
	{
		std::cerr << "Got an exception: " << e.what() << std::endl;
	}
}
```

### 예제2. JSON 송/수신하기

```cpp
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
```

### 사용 예시
```cpp
#include <string>
#include <iostream>
#include <cstring>
#include "curl/curl.h"
#include "curlite.hpp"

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
```