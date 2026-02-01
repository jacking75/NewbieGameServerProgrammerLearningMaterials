# HappyHTTP

- 예제 코드 : `~/ExampleCodes/HappyHTTPExample`
- 원본 코드 : https://github.com/mingodad/HappyHTTP
	- Last Commit : 2013/07/27
	- Latest Release Version : 0.1

## 들어가기전에

해당 문서의 예제 코드(`~/ExampleCodes/HappyHTTPExample`)에서 사용되는 라이브러리 코드는 **원본 빌드 시 발생하는 컴파일 오류 및 에러 사항을 수정한 코드**다.

## 라이브러리 특징

- 직관적인 API
- 크로스 플랫폼 (`Windows`, `Linux`, `OSX`) 지원
- 다른 라이브러리 **종속성 없음**
- 모든 요청은 **비동기로 동작** (*응답 대기 불필요, 여러 번 요청 가능*)
- 응답 데이터는 **Folling** 방식으로 수신.
- 응답 데이터는 콜백 함수로 핸들링


## 라이브러리 설치하기

1. `~/ExampleCodes/HappyHTTPExample`에서 `happyhttp.cpp`와 `happyhttp.h`를 자신의 프로젝트 폴더 복사한다.

![install](../Images/HappyHTTP/install_01.png)

2. 복사한 소스 코드를 프로젝트에 포함시킨다.

![install](../Images/HappyHTTP/install_02.png)

## 라이브러리 사용하기

### 0. `winsock` 초기화

`HappyHTTP`는 `winsock` 기반의 라이브러리이며, 라이브러리 기능 사용 전에 `winsock`을 초기화 해야한다.

```cpp
WSAData wsaData;
int32_t errcode = WSAStartup(MAKEWORD(2, 2), &wsaData);
if (errcode != NO_ERROR)
{
	std::cout << "WSAStartup failed with error: " << errcode << std::endl;
	return false;
}
return true;
```

### 1. Folling 함수 선언 및 정의

- 요청 패킷 송신 후 응답 데이터 수신을 위해 다음과 같이 `outstanding()` 함수와 `pump()` 함수를 호출한다.

```cpp
void ReceiveResponseData(happyhttp::Connection* p_conn)
{
	while (p_conn->outstanding()) // 응답 데이터가 남아있는가?
	{
		// 응답 데이터를 계속 수신한다.
		p_conn->pump();
	}
}
```

### 2. 응답 데이터 콜백 함수 선언 및 정의

#### `OnBegin`

- 응답 데이터 수신 시 **한 번 호출**
- 매개 변수 `happyhttp::Response*`를 통해 응답 상태 확인 가능.

##### 예제 코드 
```cpp
void OnBegin(const happyhttp::Response* response, void* userData)
{
	// 응답 상태를 출력한다.
	printf("BEGIN (%d %s)\n", response->getstatus(), response->getreason());
}
```

#### `OnData`
- 응답 데이터 수신을 위한 **Folling 중 (*아래 설명*) 여러 번 호출**
- 매개 변수 `happyhttp::Response*`를 통해 응답 상태 확인 가능.
- 매개 변수 `responseData`와 `responseDataLength`로 응답 데이터 접근 가능

##### 예제 코드
```cpp
void OnData(const happyhttp::Response* response, void* userData, const unsigned char* responseData, int responseDataLength)
{
	// 표준 출력 장치에 현재 수신 받은 응답 데이터 출력
	fwrite(responseData, 1, responseDataLength, stdout);
}
```

#### `OnComplete`
- 모든 응답 데이터 수신 시 **한 번 호출**

## 예제 코드
```cpp
void OnComplete(const happyhttp::Response* response, void* userData)
{
	std::cout << std::endl;
	std::cout << "Done response data receive" << std::endl;
}
```

### 예제 1. 간단한 GET 요청
```cpp
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
```

### 예제 2. 간단한 POST 요청
```cpp
void ExampleMethodPost()
{
	const char* url = "www.postman-echo.com";
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
	happyhttp::Connection conn(url, port);

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
```

### 예제 3. 간단한 POST 요청 (Low-Level Interface)
```cpp
void ExampleMethodPostLowLevelInterface()
{
	const char* url = "www.postman-echo.com";
	const int32_t port = 80;

	const char* body_data = "fruit=mango&price=3";
	const int32_t body_data_len = (int)strlen(body_data);

	// happyhttp 인스턴스 생성
	happyhttp::Connection conn(url, port);

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
```

### 예제 4. JSON 송/수신
```cpp
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
		"Content-type", "application/json",	// JSON으로 설정한다.
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
```

### 사용 예시
```cpp
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
```