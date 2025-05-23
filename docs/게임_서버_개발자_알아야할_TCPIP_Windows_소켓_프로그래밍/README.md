# 게임 서버 개발자 알아야할 TCP/IP Windows 소켓 프로그래밍

저자: 최흥배, Claude AI  

- C++23
- Windows 11
- Visual Studio 2022 이상
  

-----  
# Chapter 01. 네트워크와 소켓 프로그래밍
  
## 01. TCP/IP 프로토콜 개요
TCP/IP(Transmission Control Protocol/Internet Protocol)는 인터넷에서 데이터를 주고받는 데 사용되는 표준 프로토콜 집합입니다. 이 프로토콜은 네트워크에서 데이터가 어떻게 패킷으로 나뉘고, 주소가 지정되고, 전송되고, 라우팅되며, 수신되는지에 대한 규칙을 정의합니다.

### TCP/IP 계층 구조
1. **네트워크 액세스 계층**: 이더넷, Wi-Fi 등 물리적 연결을 담당
2. **인터넷 계층**: IP 프로토콜이 여기에 속하며 패킷 라우팅과 주소 지정 담당
3. **전송 계층**:
   - **TCP**: 연결 지향적 프로토콜로 데이터 전송의 신뢰성 보장
   - **UDP**: 비연결성 프로토콜로 신뢰성보다 속도가 중요한 경우 사용
4. **응용 계층**: HTTP, FTP, SMTP 등 응용 프로그램에 서비스 제공
  
![OSI 7계층, TCP/IP 모델, 게임 서버 관련성](./images/002.png)  
    
  
### 게임 서버 개발 시 고려사항
온라인 게임 서버에서는 보통 TCP와 UDP를 모두 활용합니다. **로그인, 캐릭터 정보, 게임 상태 동기화 등 신뢰성이 중요한 데이터는 TCP**를, **실시간 위치 업데이트나 음성 채팅 등 지연 시간이 중요한 데이터는 UDP**를 사용합니다.
  

## 02. 소켓의 개념  
소켓은 네트워크 통신의 기본 요소로, 두 프로그램이 네트워크를 통해 데이터를 주고받을 수 있게 해주는 양방향 통신 엔드포인트다. 마치 전화기가 두 사람을 연결해주는 것처럼, 소켓은 두 프로그램을 연결해준다.  
  
![Socket 개념도](./images/003.png)    

1. 통신 엔드포인트: 소켓은 네트워크 상의 두 프로그램을 연결하는 끝점이다.
2. 식별자: 각 소켓은 IP 주소와 포트 번호의 조합으로 고유하게 식별된다.
  소켓 = IP 주소 + 포트 번호
3. 소켓 타입:
    - TCP 소켓 (스트림 소켓): 연결 지향적, 신뢰성 있는 데이터 전송
    - UDP 소켓 (데이터그램 소켓): 비연결성, 빠르지만 신뢰성 낮음
4. 클라이언트-서버 모델:
    - 서버 소켓: 특정 포트에서 연결 요청을 대기(listen)
    - 클라이언트 소켓: 서버에 연결 요청을 보냄
  
    
### 소켓의 기본 요소
- **소켓 주소**: IP 주소와 포트 번호의 조합으로 네트워크에서 특정 프로세스를 식별
- **소켓 API**: 운영체제가 제공하는 네트워크 프로그래밍 인터페이스
   

### 소켓의 유형
1. **스트림 소켓(SOCK_STREAM)**: TCP 프로토콜 기반, 연결 지향적, 데이터 신뢰성 보장
2. **데이터그램 소켓(SOCK_DGRAM)**: UDP 프로토콜 기반, 비연결성, 빠른 전송 속도
3. **로우 소켓(SOCK_RAW)**: 하위 수준 프로토콜에 직접 접근 가능
  

![스트림 소켓과 데이터그램 소켓](./images/004.png)     
스트림 소켓과 데이터그램 소켓의 주요 차이점은 다음과 같다:

#### 스트림 소켓 (SOCK_STREAM)
- **프로토콜**: TCP(Transmission Control Protocol) 사용
- **특징**:
  - 연결 지향적 (Connection-oriented)
  - 3-way handshake로 연결 설정
  - 데이터 전송 순서 보장
  - 신뢰성 있는 전송 (패킷 손실 시 재전송)
  - 흐름 제어와 혼잡 제어 제공
  - 바이트 스트림 형태로 데이터 전송

- **적합한 용도**:
  - 파일 전송
  - 웹 브라우징 (HTTP)
  - 이메일 (SMTP)
  - 원격 로그인 (SSH)
  
  
#### 데이터그램 소켓 (SOCK_DGRAM)
- **프로토콜**: UDP(User Datagram Protocol) 사용
- **특징**:
  - 비연결성 (Connectionless)
  - 연결 설정 없이 바로 데이터 전송
  - 데이터 순서 보장 없음
  - 신뢰성 없음 (패킷 손실 가능)
  - 흐름 제어나 혼잡 제어 없음
  - 개별 패킷 단위로 데이터 전송

- **적합한 용도**:
  - 실시간 스트리밍 (비디오, 오디오)
  - 온라인 게임
  - DNS 조회
  - IoT 센서 데이터
  
  
두 소켓 타입의 선택은 응용 프로그램의 요구사항에 따라 달라진다. 데이터 신뢰성이 중요하면 TCP 스트림 소켓을, 속도와 낮은 지연시간이 중요하면 UDP 데이터그램 소켓을 선택한다.  
    
Windows에서는 Winsock(Windows Socket) API를 사용하여 소켓 프로그래밍을 구현합니다.
  

## 03. 소켓의 특징과 구조

### 소켓의 주요 특징
1. **양방향 통신**: 데이터 송수신 모두 가능
2. **프로토콜 독립성**: 다양한 프로토콜에 대해 일관된 인터페이스 제공
3. **다중 연결 처리**: 서버 소켓은 여러 클라이언트 연결 처리 가능
4. **비동기 통신 지원**: 논블로킹 모드와 비동기 I/O 모델 지원
  
### 소켓의 구조
Windows에서 소켓은 `SOCKET` 데이터 타입으로 표현되며, 내부적으로 네트워크 리소스에 대한 핸들입니다. 소켓 주소는 다음과 같은 구조체로 표현됩니다:

```cpp
// IPv4 소켓 주소 구조체
struct sockaddr_in {
    short sin_family;           // 주소 체계 (AF_INET)
    unsigned short sin_port;    // 포트 번호
    struct in_addr sin_addr;    // IP 주소
    char sin_zero[8];           // 패딩
};

// IPv6 소켓 주소 구조체
struct sockaddr_in6 {
    short sin6_family;          // 주소 체계 (AF_INET6)
    unsigned short sin6_port;   // 포트 번호
    unsigned long sin6_flowinfo; // 흐름 정보
    struct in6_addr sin6_addr;  // IPv6 주소
    unsigned long sin6_scope_id; // 범위 ID
};
```
  

### Winsock 초기화
Windows에서 소켓 프로그래밍을 시작하기 전에 WSAStartup 함수를 호출하여 Winsock을 초기화해야 합니다:

```cpp
WSADATA wsaData;
if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    // 초기화 실패 처리
}

// 소켓 프로그래밍 코드...

WSACleanup(); // 프로그램 종료 시 정리
```
  

## 04. 소켓 프로그램 맛보기
이제 C++23 표준을 사용한 간단한 TCP 클라이언트-서버 프로그램 예제를 살펴보겠습니다.  

### TCP 서버 예제:
codes/tcp_server_01 디렉토리  
```cpp
#include <iostream>
#include <string>
#include <format>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup 실패" << endl;
        return 1;
    }

    // 소켓 생성
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "소켓 생성 실패: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // 소켓 주소 설정
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 IP 주소 수신
    serverAddr.sin_port = htons(12345); // 포트 12345 사용

    // 소켓 바인딩
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "바인딩 실패: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // 리스닝 상태로 전환
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "리스닝 실패: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "서버가 시작되었습니다. 클라이언트를 기다리는 중..." << endl;

    // 클라이언트 연결 수락
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
    
    if (clientSocket == INVALID_SOCKET) {
        cerr << "클라이언트 연결 수락 실패: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // 클라이언트 IP 출력
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    cout << format("클라이언트 연결됨: {}:{}\n", clientIP, ntohs(clientAddr.sin_port));

    // 데이터 수신 및 응답
    const int bufferSize = 1024;
    char buffer[bufferSize];
    
    while (true) {
        // 데이터 수신
        int bytesReceived = recv(clientSocket, buffer, bufferSize, 0);
        if (bytesReceived <= 0) {
            if (bytesReceived == 0) {
                cout << "클라이언트 연결 종료" << endl;
            } else {
                cerr << "수신 실패: " << WSAGetLastError() << endl;
            }
            break;
        }

        // 수신된 데이터 처리
        buffer[bytesReceived] = '\0';
        cout << format("클라이언트로부터 수신: {}\n", buffer);

        // 응답 전송
        string response = "메시지 수신 완료: ";
        response += buffer;
        
        if (send(clientSocket, response.c_str(), response.length(), 0) == SOCKET_ERROR) {
            cerr << "전송 실패: " << WSAGetLastError() << endl;
            break;
        }
    }

    // 소켓 닫기
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    
    return 0;
}
```
  
![TCP 서버 구현 흐름도](./images/005.png)  
  

### TCP 클라이언트 예제:  
codes/tcp_client_01 디렉토리  

```cpp
#include <iostream>
#include <string>
#include <format>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup 실패" << endl;
        return 1;
    }

    // 소켓 생성
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "소켓 생성 실패: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // 서버 주소 설정
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // 서버와 동일한 포트 사용
    
    // 서버 IP 주소 설정 (이 예제에서는 로컬호스트)
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        cerr << "잘못된 서버 주소" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // 서버에 연결
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "서버 연결 실패: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "서버에 연결되었습니다." << endl;

    // 메시지 송수신
    const int bufferSize = 1024;
    char buffer[bufferSize];
    string message;

    while (true) {
        // 사용자로부터 메시지 입력 받기
        cout << "서버로 보낼 메시지 (종료하려면 'exit' 입력): ";
        getline(cin, message);

        if (message == "exit") {
            break;
        }

        // 메시지 전송
        if (send(clientSocket, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
            cerr << "전송 실패: " << WSAGetLastError() << endl;
            break;
        }

        // 응답 수신
        int bytesReceived = recv(clientSocket, buffer, bufferSize, 0);
        if (bytesReceived <= 0) {
            if (bytesReceived == 0) {
                cout << "서버 연결 종료" << endl;
            } else {
                cerr << "수신 실패: " << WSAGetLastError() << endl;
            }
            break;
        }

        // 수신된 응답 처리
        buffer[bytesReceived] = '\0';
        cout << format("서버로부터 응답: {}\n", buffer);
    }

    // 소켓 닫기
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
```
  
![TCP 클라이언트 구현 흐름도](./images/006.png)  
 

### 소켓 프로그래밍의 핵심 단계
1. **초기화**: Winsock 라이브러리 초기화 (WSAStartup)
2. **소켓 생성**: socket() 함수로 소켓 생성
3. **서버 측**:
   - 바인딩(bind): 소켓을 특정 주소와 포트에 바인딩
   - 리스닝(listen): 연결 요청 대기 상태로 전환
   - 수락(accept): 클라이언트 연결 요청 수락
4. **클라이언트 측**:
   - 연결(connect): 서버에 연결 요청
5. **데이터 송수신**: send(), recv() 함수 사용
6. **정리**: 소켓 닫기(closesocket), Winsock 정리(WSACleanup)
  

### 게임 서버 개발을 위한 추가 고려사항
1. **비동기 소켓 프로그래밍**: 실제 게임 서버에서는 대부분 비동기 방식을 사용합니다. Windows에서는 WSAEventSelect, WSAAsyncSelect, Overlapped I/O, I/O Completion Port(IOCP) 등의 메커니즘을 제공합니다.

2. **멀티스레딩**: 고성능 서버는 다중 스레드를 활용하여 여러 작업을 동시에 처리합니다. C++23의 std::thread, std::mutex 등을 사용하여 구현할 수 있습니다.

3. **오류 처리**: 네트워크 프로그래밍에서는 다양한 예외 상황이 발생할 수 있으므로 적절한 오류 처리가 중요합니다.

4. **패킷 설계**: 효율적인 데이터 직렬화/역직렬화 방법과 패킷 구조 설계가 필요합니다.

5. **보안**: 온라인 게임 서버는 보안에 특히 주의해야 합니다. SSL/TLS를 이용한 암호화, 패킷 인증 등의 기술을 적용해야 할 수 있습니다.

이러한 기본 개념들을 이해하고 실습하면서, 점진적으로 더 복잡한 게임 서버 구조와 최적화 기법들을 학습해 나가시기 바랍니다.   

 <br>      
   

# Chapter 02. 소켓 시작하기

## 01 오류 처리
Windows 소켓 프로그래밍에서 오류 처리는 안정적인 네트워크 애플리케이션 개발에 필수적인 요소입니다. 특히 온라인 게임 서버와 같이 장시간 실행되며 많은 클라이언트를 처리해야 하는 환경에서는 더욱 중요합니다.

### Winsock 오류 코드 시스템
Winsock API는 함수 호출이 실패할 경우 대부분 `SOCKET_ERROR`(-1) 또는 `INVALID_SOCKET`을 반환합니다. 구체적인 오류 정보는 `WSAGetLastError()` 함수를 통해 확인할 수 있습니다.

```cpp
int result = bind(serverSocket, ...);
if (result == SOCKET_ERROR) {
    int errorCode = WSAGetLastError();
    // 오류 처리
}
```

### 주요 Winsock 오류 코드
게임 서버 개발 시 자주 접하게 될 주요 오류 코드들입니다:

| 오류 코드 | 설명 | 대응 방법 |
|-----------|------|-----------|
| WSAEWOULDBLOCK | 비블로킹 소켓 작업이 즉시 완료될 수 없음 | 비동기 작업 상태로 처리 |
| WSAECONNRESET | 원격 호스트가 강제로 연결 종료 | 클라이언트 연결 정리 및 리소스 해제 |
| WSAETIMEDOUT | 연결 시도 시간 초과 | 재시도 또는 사용자에게 알림 |
| WSAEADDRINUSE | 이미 사용 중인 주소 | 다른 포트 사용 또는 SO_REUSEADDR 옵션 설정 |
| WSAHOST_NOT_FOUND | 호스트를 찾을 수 없음 | DNS 설정 확인 |
  

### C++23을 활용한 오류 처리 패턴
C++23에서는 오류 처리를 위한 새로운 기능들이 추가되었습니다. 특히 `std::expected`를 활용하면 더 명확한 오류 처리가 가능합니다:

```cpp
#include <expected>
#include <string>
#include <format>
#include <WinSock2.h>

// 소켓 작업 결과를 나타내는 타입
using SocketResult = std::expected<SOCKET, int>;

// 소켓 생성 함수
SocketResult createSocket(int family, int type, int protocol) {
    SOCKET sock = socket(family, type, protocol);
    if (sock == INVALID_SOCKET) {
        return std::unexpected(WSAGetLastError());
    }
    return sock;
}

// 사용 예시
void useSocket() {
    auto result = createSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (result) {
        SOCKET sock = *result;
        // 소켓 사용
        closesocket(sock);
    } else {
        int errorCode = result.error();
        std::string errorMsg = std::format("소켓 생성 실패: 오류 코드 {}", errorCode);
        // 오류 로깅 또는 처리
    }
}
```
  

### 게임 서버에서의 오류 처리 전략
온라인 게임 서버에서는 오류 처리가 특히 중요합니다. 몇 가지 핵심 전략을 소개합니다:

1. **세분화된 로깅**: 오류 발생 시 상세 정보(시간, 소켓 정보, 이벤트 컨텍스트 등)를 로그에 기록

2. **장애 복구 메커니즘**: 중요한 소켓 작업 실패 시 자동 재시도 로직 구현

3. **그레이스풀 디그레이드(Graceful Degradation)**: 일부 기능에 장애가 발생해도 핵심 기능은 계속 작동하도록 설계

4. **클라이언트 통보**: 심각한 문제 발생 시 클라이언트에게 적절히 알림 (연결 재시도 유도, 정보 메시지 등)

```cpp
// 게임 서버에서의 오류 처리 예시
void handleNetworkError(int errorCode, const std::string& operation, SOCKET sock) {
    std::string message;
    bool isFatal = false;
    
    switch (errorCode) {
        case WSAECONNRESET:
            message = "클라이언트가 연결을 강제 종료했습니다.";
            // 클라이언트 정리 로직
            break;
            
        case WSAEWOULDBLOCK:
            // 비블로킹 작업에서는 정상적인 상황일 수 있음
            return;
            
        case WSAENETDOWN:
            message = "네트워크 시스템 장애가 발생했습니다.";
            isFatal = true;
            break;
            
        default:
            message = std::format("알 수 없는 네트워크 오류: {}", errorCode);
            break;
    }
    
    // 로그 기록
    Logger::log(isFatal ? LogLevel::ERROR : LogLevel::WARNING, 
               std::format("소켓 {}: {} 작업 중 오류 발생 - {}", 
                          sock, operation, message));
    
    if (isFatal) {
        // 심각한 오류 발생 시 추가 처리
        notifyAdministrator(message);
        initiateRecoveryProcedure();
    }
}
```
  

## 02 소켓 초기화와 종료
Windows에서 소켓 프로그래밍을 시작하기 전에는 Winsock 라이브러리를 초기화해야 하며, 프로그램 종료 시에는 정리 작업이 필요합니다.
  
### WSAStartup 함수
Winsock 라이브러리 초기화를 위해 `WSAStartup` 함수를 호출합니다:

```cpp
int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
```

- **wVersionRequested**: 요청할 Winsock 버전 (일반적으로 2.2 버전 사용)
- **lpWSAData**: WSADATA 구조체 포인터
 

### WSADATA 구조체
`WSADATA` 구조체는 Winsock 구현에 대한 정보를 포함합니다:

```cpp
typedef struct WSAData {
    WORD           wVersion;       // 사용 중인 Winsock 버전
    WORD           wHighVersion;   // 지원되는 최상위 버전
    unsigned short iMaxSockets;    // 최대 소켓 수(deprecated)
    unsigned short iMaxUdpDg;      // 최대 UDP 데이터그램 크기(deprecated)
    char FAR       *lpVendorInfo;  // 벤더 정보(deprecated)
    char           szDescription[WSADESCRIPTION_LEN+1]; // 설명
    char           szSystemStatus[WSASYS_STATUS_LEN+1]; // 상태
} WSADATA;
```

실제 사용 시에는 다음과 같이 초기화합니다:

```cpp
WSADATA wsaData;
int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
if (result != 0) {
    std::cerr << std::format("WSAStartup 실패: 오류 코드 {}\n", result);
    return 1;
}

// 요청한 버전 지원 여부 확인
if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
    std::cerr << "요청한 Winsock 버전 2.2를 사용할 수 없습니다.\n";
    WSACleanup();
    return 1;
}

std::cout << std::format("Winsock 초기화 성공: {}\n", wsaData.szDescription);
```

### WSACleanup 함수
프로그램 종료 시 Winsock을 정리하기 위해 `WSACleanup` 함수를 호출합니다:  

```cpp
int WSACleanup(void);
```

이 함수는 애플리케이션의 Winsock 사용을 종료하고, 관련 리소스를 해제합니다. 모든 소켓을 닫고 Winsock 작업을 완료한 후에 호출해야 합니다.
  

### RAII를 활용한 안전한 초기화 및 종료
C++에서는 RAII(Resource Acquisition Is Initialization) 패턴을 사용하여 Winsock의 초기화와 종료를 안전하게 관리할 수 있습니다:
  
```cpp
class WinsockInit {
public:
    WinsockInit() {
        result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            throw std::runtime_error(
                std::format("WSAStartup 실패: 오류 코드 {}", result));
        }
    }
    
    ~WinsockInit() {
        if (result == 0) {
            WSACleanup();
        }
    }
    
    bool isInitialized() const { return result == 0; }
    
    const WSADATA& getData() const { return wsaData; }
    
private:
    WSADATA wsaData;
    int result;
};

// 사용 예시
int main() {
    try {
        WinsockInit winsock;
        // 소켓 프로그래밍 코드
    }
    catch (const std::exception& e) {
        std::cerr << "오류 발생: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

이 패턴을 사용하면 예외가 발생하더라도 Winsock 리소스가 자동으로 정리됩니다.
  

### 게임 서버에서의 초기화 고려사항
대규모 온라인 게임 서버에서는 Winsock 초기화 시 다음 사항을 고려해야 합니다:

1. **버전 호환성**: 서버 배포 환경에 따라 지원되는 Winsock 버전이 다를 수 있으므로 적절한 버전 확인 필요

2. **스레드 안전성**: 멀티스레드 환경에서는 WSAStartup과 WSACleanup의 호출 타이밍 주의

3. **초기화 검증**: 성공적으로 초기화되었는지 확인하고, 실패 시 적절한 오류 처리

4. **정상 종료**: 게임 서버 종료 시 모든 연결을 정상적으로 종료하고 Winsock 정리
  

## 03 소켓 생성과 닫기
소켓은 네트워크 통신의 기본 단위로, 적절히 생성하고 관리해야 합니다. 여기서는 소켓 생성과 닫기에 대한 상세 내용을 다룹니다.
  
### socket 함수
소켓을 생성하기 위해 `socket` 함수를 사용합니다:

```cpp
SOCKET socket(int af, int type, int protocol);
```

매개변수:
- **af**: 주소 체계(Address Family) - AF_INET(IPv4), AF_INET6(IPv6)
- **type**: 소켓 유형 - SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
- **protocol**: 프로토콜 - IPPROTO_TCP, IPPROTO_UDP

반환값:
- 성공: 소켓 핸들
- 실패: INVALID_SOCKET
  

### 주요 소켓 유형 선택
게임 서버에서 주로 사용하는 소켓 유형:
  
1. **TCP 소켓 (SOCK_STREAM)**
   - 연결 지향적, 신뢰성 있는 데이터 전송
   - 사용 사례: 로그인, 게임 상태 변경, 중요 게임 데이터

   ```cpp
   SOCKET tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   ```

2. **UDP 소켓 (SOCK_DGRAM)**
   - 비연결성, 빠른 데이터 전송, 신뢰성 낮음
   - 사용 사례: 캐릭터 위치 업데이트, 음성 채팅

   ```cpp
   SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   ```

3. **IPv6 지원 소켓**
   - 최신 네트워크 환경에서는 IPv6 지원이 중요

   ```cpp
   SOCKET ipv6Socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
   ```
  

### closesocket 함수
소켓 사용을 마친 후에는 `closesocket` 함수로 소켓을 닫아 리소스를 해제해야 합니다:

```cpp
int closesocket(SOCKET s);
```

성공 시 0을 반환하고, 실패 시 SOCKET_ERROR를 반환합니다.
  

### 소켓 옵션 설정
소켓 생성 후 특정 옵션을 설정하여 동작을 제어할 수 있습니다:

```cpp
int setsockopt(SOCKET s, int level, int optname, const char* optval, int optlen);
```

게임 서버에서 유용한 소켓 옵션들:

1. **SO_REUSEADDR**: 소켓 주소 재사용 허용
   ```cpp
   int reuse = 1;
   setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, 
              reinterpret_cast<const char*>(&reuse), sizeof(reuse));
   ```

2. **TCP_NODELAY**: Nagle 알고리즘 비활성화 (지연 감소)
   ```cpp
   int noDelay = 1;
   setsockopt(gameSocket, IPPROTO_TCP, TCP_NODELAY, 
              reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));
   ```

3. **SO_RCVTIMEO/SO_SNDTIMEO**: 수신/송신 타임아웃 설정
   ```cpp
   DWORD timeout = 5000; // 5초
   setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, 
              reinterpret_cast<const char*>(&timeout), sizeof(timeout));
   ```

4. **SO_SNDBUF/SO_RCVBUF**: 송신/수신 버퍼 크기 설정
   ```cpp
   int bufSize = 64 * 1024; // 64KB
   setsockopt(socket, SOL_SOCKET, SO_RCVBUF, 
              reinterpret_cast<const char*>(&bufSize), sizeof(bufSize));
   ```
  

### 소켓 모드 제어
Windows 소켓은 기본적으로 블로킹 모드로 작동하지만, 비블로킹 모드로 변경할 수 있습니다:  

```cpp
// 비블로킹 모드로 설정
u_long mode = 1;  // 1: 비블로킹, 0: 블로킹
if (ioctlsocket(socket, FIONBIO, &mode) == SOCKET_ERROR) {
    // 오류 처리
}
```
  

### C++23을 활용한 소켓 래퍼 클래스
C++23 기능을 활용하여 소켓 관리를 더 안전하게 할 수 있는 래퍼 클래스 예시입니다:  

```cpp
#include <WinSock2.h>
#include <stdexcept>
#include <format>
#include <string>
#include <utility>
#include <iostream>
#include <expected>

class Socket {
public:
    // 소켓 생성
    static std::expected<Socket, int> create(int af, int type, int protocol) {
        SOCKET sock = socket(af, type, protocol);
        if (sock == INVALID_SOCKET) {
            return std::unexpected(WSAGetLastError());
        }
        return Socket(sock);
    }
    
    // 이동 생성자/대입 연산자
    Socket(Socket&& other) noexcept : sock_(std::exchange(other.sock_, INVALID_SOCKET)) {}
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            close();
            sock_ = std::exchange(other.sock_, INVALID_SOCKET);
        }
        return *this;
    }
    
    // 복사 방지
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    
    // 소멸자에서 자동으로 소켓 닫기
    ~Socket() {
        close();
    }
    
    // 소켓 핸들 반환
    SOCKET get() const { return sock_; }
    
    // 소켓 옵션 설정
    template<typename T>
    bool setOption(int level, int optname, const T& value) {
        int result = setsockopt(sock_, level, optname, 
                               reinterpret_cast<const char*>(&value), sizeof(T));
        return result != SOCKET_ERROR;
    }
    
    // 비블로킹 모드 설정
    bool setNonBlocking(bool nonBlocking = true) {
        u_long mode = nonBlocking ? 1 : 0;
        return ioctlsocket(sock_, FIONBIO, &mode) != SOCKET_ERROR;
    }
    
    // 소켓 명시적 닫기
    void close() {
        if (sock_ != INVALID_SOCKET) {
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
        }
    }
    
    // 유효한 소켓인지 확인
    bool isValid() const { return sock_ != INVALID_SOCKET; }

private:
    explicit Socket(SOCKET sock) : sock_(sock) {}
    SOCKET sock_;
};

// 사용 예시
void useSocketClass() {
    auto result = Socket::create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!result) {
        std::cerr << std::format("소켓 생성 실패: 오류 코드 {}\n", result.error());
        return;
    }
    
    Socket socket = std::move(*result);
    
    // TCP_NODELAY 옵션 설정
    if (!socket.setOption(IPPROTO_TCP, TCP_NODELAY, 1)) {
        std::cerr << "TCP_NODELAY 옵션 설정 실패\n";
    }
    
    // 비블로킹 모드로 설정
    if (!socket.setNonBlocking()) {
        std::cerr << "비블로킹 모드 설정 실패\n";
    }
    
    // 소켓 사용 코드...
    
    // 명시적으로 닫을 필요 없음 (소멸자에서 자동으로 처리)
}
```
  

### 듀얼 IP 스택 지원
최신 게임 서버에서는 IPv4와 IPv6를 모두 지원하는 듀얼 스택 구현이 권장됩니다:

```cpp
// IPv4와 IPv6를 모두 지원하는 소켓 생성
SOCKET createDualStackSocket() {
    // IPv6 소켓 생성
    SOCKET sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    
    // IPv4 매핑 활성화 (듀얼 스택 모드)
    int v6Only = 0;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, 
                  reinterpret_cast<const char*>(&v6Only), sizeof(v6Only)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }
    
    return sock;
}
```
  

### 게임 서버에서의 소켓 관리 전략
대규모 게임 서버에서는 효율적인 소켓 관리가 매우 중요합니다:

1. **소켓 풀링**: 자주 생성/소멸되는 소켓의 성능 개선을 위한 풀링 메커니즘 구현

2. **소켓 모니터링**: 활성 소켓의 상태 추적 및 문제 감지

3. **그레이스풀 셧다운**: 연결 종료 시 `shutdown` 함수를 사용하여 정상적인 종료 처리
   ```cpp
   // 정상적인 연결 종료 (송신만 중단)
   shutdown(socket, SD_SEND);
   // 남은 데이터 수신 후 closesocket 호출
   ```

4. **리소스 한계 관리**: 시스템 리소스 제한에 따른 최대 동시 연결 수 조절
 
이러한 기법을 적용하여 안정적이고 확장 가능한 게임 서버를 구축할 수 있습니다.
  

<br>      
     
# Chapter 03. 소켓 주소 구조체 다루기

## 01 소켓 주소 구조체
소켓 프로그래밍에서는 네트워크 주소를 표현하기 위해 다양한 주소 구조체를 사용합니다. 이러한 구조체들은 IP 주소, 포트 번호, 주소 체계 등의 정보를 포함하고 있으며, Windows 소켓 API 함수에 매개변수로 전달됩니다.

### 기본 소켓 주소 구조체 (sockaddr)
모든 소켓 주소 구조체의 기본이 되는 구조체입니다:

```cpp
struct sockaddr {
    ADDRESS_FAMILY sa_family;    // 주소 체계 (AF_INET, AF_INET6 등)
    CHAR sa_data[14];           // 프로토콜별 주소 정보
};
```

이 기본 구조체는 모든 유형의 소켓 주소를 처리할 수 있도록 설계되었지만, 실제로는 거의 직접 사용하지 않고 프로토콜별 구조체를 사용한 후 필요할 때 형변환합니다.

### IPv4 소켓 주소 구조체 (sockaddr_in)
IPv4 주소 체계에서 사용하는 구조체입니다:

```cpp
struct sockaddr_in {
    ADDRESS_FAMILY sin_family;    // AF_INET
    USHORT sin_port;             // 포트 번호 (네트워크 바이트 순서)
    IN_ADDR sin_addr;            // IPv4 주소
    CHAR sin_zero[8];            // 패딩 (0으로 채움)
};

struct in_addr {
    union {
        struct {
            UCHAR s_b1, s_b2, s_b3, s_b4;  // IPv4 주소 각 바이트
        } S_un_b;
        struct {
            USHORT s_w1, s_w2;  // IPv4 주소를 두 개의 USHORT로 표현
        } S_un_w;
        ULONG S_addr;  // IPv4 주소 32비트 값 (네트워크 바이트 순서)
    } S_un;
};
```

일반적으로 `in_addr` 구조체의 `S_un.S_addr` 필드를 통해 IPv4 주소를 32비트 정수로 처리합니다.

### IPv6 소켓 주소 구조체 (sockaddr_in6)
IPv6 주소 체계에서 사용하는 구조체입니다:

```cpp
struct sockaddr_in6 {
    ADDRESS_FAMILY sin6_family;   // AF_INET6
    USHORT sin6_port;            // 포트 번호 (네트워크 바이트 순서)
    ULONG sin6_flowinfo;         // 흐름 정보
    IN6_ADDR sin6_addr;          // IPv6 주소
    ULONG sin6_scope_id;         // 범위 ID
};

struct in6_addr {
    union {
        UCHAR Byte[16];          // IPv6 주소 (16바이트)
        USHORT Word[8];          // IPv6 주소 (8개의 USHORT)
    } u;
};
```

### 프로토콜 독립적인 소켓 주소 구조체 (sockaddr_storage)
모든 종류의 소켓 주소를 저장할 수 있는 충분히 큰 구조체로, IPv4와 IPv6 모두 처리 가능합니다:

```cpp
struct sockaddr_storage {
    ADDRESS_FAMILY ss_family;     // 주소 체계 (AF_INET, AF_INET6 등)
    CHAR __ss_pad1[8];           // 정렬을 위한 패딩
    LONGLONG __ss_align;         // 8바이트 정렬을 위한 필드
    CHAR __ss_pad2[112];         // 추가 패딩
};
```

이 구조체는 최신 프로그램에서 권장되며, 특히 IPv6 지원이 필요한 경우에 유용합니다.

### 주소 구조체 사용 예시

```cpp
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <format>
#include <span>

// IPv4 주소 구조체 초기화 예시
void initIPv4Address() {
    sockaddr_in serverAddr{};  // C++23의 일관된 초기화 사용
    
    serverAddr.sin_family = AF_INET;  // IPv4 주소 체계
    serverAddr.sin_port = htons(12345);  // 포트 번호 (네트워크 바이트 순서로 변환)
    inet_pton(AF_INET, "192.168.0.1", &serverAddr.sin_addr);  // IP 주소 설정
}

// IPv6 주소 구조체 초기화 예시
void initIPv6Address() {
    sockaddr_in6 serverAddr{};
    
    serverAddr.sin6_family = AF_INET6;  // IPv6 주소 체계
    serverAddr.sin6_port = htons(12345);  // 포트 번호
    serverAddr.sin6_flowinfo = 0;  // 흐름 정보 (일반적으로 0)
    inet_pton(AF_INET6, "2001:db8::1", &serverAddr.sin6_addr);  // IPv6 주소 설정
    serverAddr.sin6_scope_id = 0;  // 범위 ID (일반적으로 0)
}

// 프로토콜 독립적인 프로그래밍 예시
void protocolIndependentExample() {
    sockaddr_storage remoteAddr{};
    int addrLen = sizeof(remoteAddr);
    
    // accept 함수 등에서 사용 예시
    // SOCKET clientSocket = accept(serverSocket, 
    //                      reinterpret_cast<sockaddr*>(&remoteAddr), &addrLen);
    
    // 주소 체계에 따라 처리
    if (remoteAddr.ss_family == AF_INET) {
        // IPv4 주소 처리
        auto* ipv4Addr = reinterpret_cast<sockaddr_in*>(&remoteAddr);
        // ipv4Addr 사용...
    } 
    else if (remoteAddr.ss_family == AF_INET6) {
        // IPv6 주소 처리
        auto* ipv6Addr = reinterpret_cast<sockaddr_in6*>(&remoteAddr);
        // ipv6Addr 사용...
    }
}
```
  

## 02 바이트 정렬 함수
네트워크 프로그래밍에서는 다양한 하드웨어 아키텍처 간의 데이터 교환을 위해 표준화된 바이트 순서를 사용해야 합니다. 여기서 바이트 정렬(Byte Ordering) 함수가 중요한 역할을 합니다.

### 호스트 바이트 순서와 네트워크 바이트 순서
컴퓨터 시스템은 내부적으로 두 가지 방식으로 다중 바이트 값을 저장합니다:

1. **빅 엔디안(Big Endian)**: 가장 중요한(Most Significant) 바이트를 가장 낮은 메모리 주소에 저장
2. **리틀 엔디안(Little Endian)**: 가장 덜 중요한(Least Significant) 바이트를 가장 낮은 메모리 주소에 저장

네트워크 통신에서는 **빅 엔디안** 방식이 표준으로 사용되며, 이를 **네트워크 바이트 순서(Network Byte Order)**라고 합니다. 반면, 호스트 시스템의 내부 표현 방식은 **호스트 바이트 순서(Host Byte Order)**라고 합니다.

### 바이트 정렬 변환 함수
Windows API에서는 다음과 같은 바이트 정렬 함수를 제공합니다:

1. **htons**: Host to Network Short (16비트 값 변환)
2. **ntohs**: Network to Host Short (16비트 값 변환)
3. **htonl**: Host to Network Long (32비트 값 변환)
4. **ntohl**: Network to Host Long (32비트 값 변환)

```cpp
#include <WinSock2.h>
#include <iostream>
#include <format>

void byteOrderExample() {
    // 포트 번호 변환 (16비트)
    u_short hostPort = 12345;
    u_short netPort = htons(hostPort);  // 호스트에서 네트워크 바이트 순서로 변환
    
    std::cout << std::format("호스트 포트: {}, 네트워크 포트: {}\n", 
                            hostPort, netPort);
    
    // IP 주소 변환 (32비트)
    u_long hostAddr = 0x0100007F;  // 127.0.0.1 (호스트 바이트 순서)
    u_long netAddr = htonl(hostAddr);  // 호스트에서 네트워크 바이트 순서로 변환
    
    std::cout << std::format("호스트 주소: 0x{:X}, 네트워크 주소: 0x{:X}\n", 
                            hostAddr, netAddr);
    
    // 네트워크에서 호스트 바이트 순서로 다시 변환
    u_short hostPort2 = ntohs(netPort);
    u_long hostAddr2 = ntohl(netAddr);
    
    std::cout << std::format("변환 후 호스트 포트: {}, 호스트 주소: 0x{:X}\n", 
                            hostPort2, hostAddr2);
}
```

### C++23에서의 바이트 정렬 처리
C++23에서는 `std::byteswap` 함수를 사용하여 바이트 순서를 직접 변환할 수 있습니다:

```cpp
#include <bit>
#include <iostream>
#include <format>

void cppByteswapExample() {
    // 16비트 값 뒤집기 (htons/ntohs 상당)
    std::uint16_t value16 = 0x1234;
    std::uint16_t swapped16 = std::byteswap(value16);
    
    std::cout << std::format("원본 16비트: 0x{:X}, 뒤집힌 16비트: 0x{:X}\n", 
                            value16, swapped16);
    
    // 32비트 값 뒤집기 (htonl/ntohl 상당)
    std::uint32_t value32 = 0x12345678;
    std::uint32_t swapped32 = std::byteswap(value32);
    
    std::cout << std::format("원본 32비트: 0x{:X}, 뒤집힌 32비트: 0x{:X}\n", 
                            value32, swapped32);
}

// 플랫폼 감지를 통한 바이트 정렬 함수 구현
template <typename T>
T toNetworkByteOrder(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(value);
    } else {
        return value;
    }
}

template <typename T>
T toHostByteOrder(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(value);
    } else {
        return value;
    }
}
```

### 게임 서버에서의 바이트 정렬 활용
게임 서버에서는 패킷 구조체를 정의하고 전송할 때 바이트 정렬에 주의해야 합니다:

```cpp
#include <WinSock2.h>
#include <iostream>
#include <array>
#include <bit>
#include <span>

// 게임 패킷 구조체 예시
struct GamePacket {
    uint16_t packetId;   // 패킷 식별자
    uint16_t packetSize; // 패킷 크기
    uint32_t playerID;   // 플레이어 ID
    float positionX;     // X 좌표
    float positionY;     // Y 좌표
    float positionZ;     // Z 좌표
};

// 패킷을 네트워크 바이트 순서로 직렬화
void serializePacket(const GamePacket& packet, std::span<std::byte> buffer) {
    if (buffer.size() < sizeof(GamePacket)) {
        throw std::runtime_error("버퍼 크기가 충분하지 않습니다.");
    }
    
    auto* dest = reinterpret_cast<GamePacket*>(buffer.data());
    
    // 각 멤버를 네트워크 바이트 순서로 변환
    dest->packetId = htons(packet.packetId);
    dest->packetSize = htons(packet.packetSize);
    dest->playerID = htonl(packet.playerID);
    
    // float 값은 바이트 복사 후 필요시 바이트 순서 변환
    // (주: IEEE 754 float 형식에서 바이트 순서 변환은 복잡할 수 있음)
    std::uint32_t x, y, z;
    std::memcpy(&x, &packet.positionX, sizeof(float));
    std::memcpy(&y, &packet.positionY, sizeof(float));
    std::memcpy(&z, &packet.positionZ, sizeof(float));
    
    x = htonl(x);
    y = htonl(y);
    z = htonl(z);
    
    std::memcpy(&dest->positionX, &x, sizeof(float));
    std::memcpy(&dest->positionY, &y, sizeof(float));
    std::memcpy(&dest->positionZ, &z, sizeof(float));
}

// 네트워크에서 받은 패킷을 호스트 바이트 순서로 역직렬화
GamePacket deserializePacket(std::span<const std::byte> buffer) {
    if (buffer.size() < sizeof(GamePacket)) {
        throw std::runtime_error("버퍼 크기가 충분하지 않습니다.");
    }
    
    const auto* src = reinterpret_cast<const GamePacket*>(buffer.data());
    GamePacket packet;
    
    // 각 멤버를 호스트 바이트 순서로 변환
    packet.packetId = ntohs(src->packetId);
    packet.packetSize = ntohs(src->packetSize);
    packet.playerID = ntohl(src->playerID);
    
    // float 값 변환
    std::uint32_t x, y, z;
    std::memcpy(&x, &src->positionX, sizeof(float));
    std::memcpy(&y, &src->positionY, sizeof(float));
    std::memcpy(&z, &src->positionZ, sizeof(float));
    
    x = ntohl(x);
    y = ntohl(y);
    z = ntohl(z);
    
    std::memcpy(&packet.positionX, &x, sizeof(float));
    std::memcpy(&packet.positionY, &y, sizeof(float));
    std::memcpy(&packet.positionZ, &z, sizeof(float));
    
    return packet;
}
```
  

## 03 IP 주소 변환 함수
네트워크 프로그래밍에서는 IP 주소를 다양한 형식으로 표현하고 변환해야 합니다. Win32 API에서는 문자열 형태의 IP 주소와 바이너리 형태의 IP 주소 간 변환을 위한 여러 함수를 제공합니다.

### 구형 함수 vs. 최신 함수
Windows 네트워크 프로그래밍에서는 IP 주소 변환을 위한 다양한 함수가 있습니다:

1. **구형 함수 (레거시)**: `inet_addr`, `inet_ntoa` 등
2. **최신 함수 (권장)**: `inet_pton`, `inet_ntop` 등

최신 함수들은 IPv6를 지원하고 스레드 안전성이 개선되어 있으므로 새로운 프로그램에서는 이들을 사용하는 것이 좋습니다.

### inet_pton 함수 (문자열 -> 숫자)
문자열 형태의 IP 주소를 네트워크 바이트 순서의 바이너리 형태로 변환합니다:

```cpp
int inet_pton(
    INT Family,           // 주소 체계 (AF_INET, AF_INET6)
    PCSTR pszAddrString,  // 변환할 IP 주소 문자열
    PVOID pAddrBuf        // 변환된 주소를 저장할 버퍼
);
```

### inet_ntop 함수 (숫자 -> 문자열)
바이너리 형태의 IP 주소를 문자열 형태로 변환합니다:

```cpp
PCSTR WSAAPI inet_ntop(
    INT Family,           // 주소 체계 (AF_INET, AF_INET6)
    const VOID *pAddr,    // 변환할 바이너리 주소
    PSTR pStringBuf,      // 변환된 문자열을 저장할 버퍼
    size_t StringBufSize  // 버퍼 크기
);
```

### IP 주소 변환 예제

```cpp
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <format>
#include <string>
#include <array>

void ipAddressConversionExample() {
    // IPv4 문자열 주소 -> 바이너리
    in_addr binaryIPv4{};
    const char* ipv4String = "192.168.0.1";
    
    if (inet_pton(AF_INET, ipv4String, &binaryIPv4) != 1) {
        std::cerr << "IPv4 주소 변환 실패\n";
        return;
    }
    
    // 변환된 IPv4 주소 출력 (네트워크 바이트 순서)
    std::cout << std::format("IPv4 바이너리: 0x{:X}\n", binaryIPv4.S_un.S_addr);
    
    // IPv4 바이너리 -> 문자열
    std::array<char, INET_ADDRSTRLEN> ipv4StringBuf{};
    if (inet_ntop(AF_INET, &binaryIPv4, ipv4StringBuf.data(), ipv4StringBuf.size()) == nullptr) {
        std::cerr << "IPv4 주소 문자열 변환 실패\n";
        return;
    }
    
    std::cout << std::format("변환된 IPv4 문자열: {}\n", ipv4StringBuf.data());
    
    // IPv6 문자열 주소 -> 바이너리
    in6_addr binaryIPv6{};
    const char* ipv6String = "2001:db8::1428:57ab";
    
    if (inet_pton(AF_INET6, ipv6String, &binaryIPv6) != 1) {
        std::cerr << "IPv6 주소 변환 실패\n";
        return;
    }
    
    // IPv6 바이너리 -> 문자열
    std::array<char, INET6_ADDRSTRLEN> ipv6StringBuf{};
    if (inet_ntop(AF_INET6, &binaryIPv6, ipv6StringBuf.data(), ipv6StringBuf.size()) == nullptr) {
        std::cerr << "IPv6 주소 문자열 변환 실패\n";
        return;
    }
    
    std::cout << std::format("변환된 IPv6 문자열: {}\n", ipv6StringBuf.data());
}
```

### C++23에서의 IP 주소 처리
C++23에서는 보다 현대적인 방식으로 IP 주소를 다룰 수 있습니다:

```cpp
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <format>
#include <string>
#include <string_view>
#include <span>
#include <expected>

// 오류 코드를 포함한 반환 값
using IPConversionResult = std::expected<void, int>;

// IPv4 주소 변환 함수 (문자열 -> 바이너리)
IPConversionResult convertStringToIPv4(std::string_view ipString, in_addr& result) {
    if (inet_pton(AF_INET, ipString.data(), &result) != 1) {
        return std::unexpected(WSAGetLastError());
    }
    return {};
}

// IPv4 주소 변환 함수 (바이너리 -> 문자열)
std::expected<std::string, int> convertIPv4ToString(const in_addr& addr) {
    std::array<char, INET_ADDRSTRLEN> buffer{};
    
    if (inet_ntop(AF_INET, &addr, buffer.data(), buffer.size()) == nullptr) {
        return std::unexpected(WSAGetLastError());
    }
    
    return std::string(buffer.data());
}

// IPv6 주소 변환 함수 (문자열 -> 바이너리)
IPConversionResult convertStringToIPv6(std::string_view ipString, in6_addr& result) {
    if (inet_pton(AF_INET6, ipString.data(), &result) != 1) {
        return std::unexpected(WSAGetLastError());
    }
    return {};
}

// IPv6 주소 변환 함수 (바이너리 -> 문자열)
std::expected<std::string, int> convertIPv6ToString(const in6_addr& addr) {
    std::array<char, INET6_ADDRSTRLEN> buffer{};
    
    if (inet_ntop(AF_INET6, &addr, buffer.data(), buffer.size()) == nullptr) {
        return std::unexpected(WSAGetLastError());
    }
    
    return std::string(buffer.data());
}

// 사용 예시
void modernIPConversionExample() {
    // IPv4 변환
    in_addr ipv4Addr{};
    auto result1 = convertStringToIPv4("192.168.0.1", ipv4Addr);
    
    if (!result1) {
        std::cerr << std::format("IPv4 문자열 변환 실패: 오류 코드 {}\n", result1.error());
        return;
    }
    
    auto result2 = convertIPv4ToString(ipv4Addr);
    if (!result2) {
        std::cerr << std::format("IPv4 바이너리 변환 실패: 오류 코드 {}\n", result2.error());
        return;
    }
    
    std::cout << std::format("변환된 IPv4: {}\n", *result2);
    
    // IPv6 변환
    in6_addr ipv6Addr{};
    auto result3 = convertStringToIPv6("2001:db8::1", ipv6Addr);
    
    if (!result3) {
        std::cerr << std::format("IPv6 문자열 변환 실패: 오류 코드 {}\n", result3.error());
        return;
    }
    
    auto result4 = convertIPv6ToString(ipv6Addr);
    if (!result4) {
        std::cerr << std::format("IPv6 바이너리 변환 실패: 오류 코드 {}\n", result4.error());
        return;
    }
    
    std::cout << std::format("변환된 IPv6: {}\n", *result4);
}
```

### 게임 서버에서의 IP 주소 처리
게임 서버 개발에서는 클라이언트 연결 관리, 서버 간 통신, 보안 필터링 등을 위해 IP 주소 처리가 중요합니다:

```cpp
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <format>
#include <string>
#include <unordered_map>
#include <chrono>

// 클라이언트 연결 정보 구조체
struct ClientConnection {
    SOCKET socket;
    std::string ipAddress;
    uint16_t port;
    std::chrono::steady_clock::time_point lastActivity;
    // 기타 게임 관련 데이터...
};

// 클라이언트 연결 관리 예시
class ConnectionManager {
public:
    void addClient(SOCKET clientSocket, const sockaddr_storage& addr) {
        ClientConnection client;
        client.socket = clientSocket;
        client.lastActivity = std::chrono::steady_clock::now();
        
        // IP 주소와 포트 추출
        if (addr.ss_family == AF_INET) {
            // IPv4
            auto* ipv4 = reinterpret_cast<const sockaddr_in*>(&addr);
            std::array<char, INET_ADDRSTRLEN> ipStr{};
            
            inet_ntop(AF_INET, &ipv4->sin_addr, ipStr.data(), ipStr.size());
            client.ipAddress = ipStr.data();
            client.port = ntohs(ipv4->sin_port);
        } 
        else if (addr.ss_family == AF_INET6) {
            // IPv6
            auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(&addr);
            std::array<char, INET6_ADDRSTRLEN> ipStr{};
            
            inet_ntop(AF_INET6, &ipv6->sin6_addr, ipStr.data(), ipStr.size());
            client.ipAddress = ipStr.data();
            client.port = ntohs(ipv6->sin6_port);
        }
        
        // 연결 정보 저장
        m_clients[clientSocket] = client;
        
        std::cout << std::format("클라이언트 연결: {}:{}\n", 
                                client.ipAddress, client.port);
        
        // IP 기반 속도 제한 검사
        checkRateLimit(client.ipAddress);
    }
    
    void removeClient(SOCKET clientSocket) {
        auto it = m_clients.find(clientSocket);
        if (it != m_clients.end()) {
            std::cout << std::format("클라이언트 연결 종료: {}:{}\n", 
                                    it->second.ipAddress, it->second.port);
            m_clients.erase(it);
        }
    }
    
private:
    // IP 주소별 연결 속도 제한 검사
    void checkRateLimit(const std::string& ipAddress) {
        auto now = std::chrono::steady_clock::now();
        
        // 현재 시간에서 10초 전 시점
        auto tenSecondsAgo = now - std::chrono::seconds(10);
        
        // 해당 IP의 최근 연결 시도 기록
        auto& attempts = m_connectionAttempts[ipAddress];
        
        // 오래된 기록 제거
        while (!attempts.empty() && attempts.front() < tenSecondsAgo) {
            attempts.pop_front();
        }
        
        // 새 연결 시도 기록 추가
        attempts.push_back(now);
        
        // 10초 내 연결 시도가 5회 이상이면 경고
        if (attempts.size() > 5) {
            std::cout << std::format("경고: IP {}에서 빈번한 연결 시도 감지\n", ipAddress);
            // 필요시 차단 로직 추가
        }
    }
    
    std::unordered_map<SOCKET, ClientConnection> m_clients;
    std::unordered_map<std::string, std::deque<std::chrono::steady_clock::time_point>> 
        m_connectionAttempts;
};
```  
  

## 04 DNS와 이름 변환 함수
온라인 게임에서는 도메인 이름과 IP 주소 간의 변환이 자주 필요합니다. 예를 들어, 게임 서버는 마스터 서버에 접속하거나, 플레이어는 게임 서버에 접속할 때 도메인 이름을 사용할 수 있습니다.

### getaddrinfo 함수
호스트 이름을 IP 주소로 변환하는 현대적인 방법입니다. IPv4와 IPv6를 모두 지원합니다:

```cpp
int getaddrinfo(
    PCSTR pNodeName,               // 호스트 이름 또는 IP 주소 문자열
    PCSTR pServiceName,            // 서비스 이름 또는 포트 번호 문자열
    const ADDRINFOA *pHints,       // 반환 값 제어를 위한 힌트
    PADDRINFOA *ppResult           // 결과 주소 정보 구조체
);
```

### getnameinfo 함수
IP 주소를 호스트 이름으로 변환하는 함수입니다:

```cpp
int getnameinfo(
    const SOCKADDR *pSockaddr,     // 소켓 주소 구조체
    socklen_t SockaddrLength,      // 소켓 주소 구조체 길이
    PCHAR pNodeBuffer,             // 호스트 이름을 저장할 버퍼
    DWORD NodeBufferSize,          // 호스트 이름 버퍼 크기
    PCHAR pServiceBuffer,          // 서비스 이름을 저장할 버퍼
    DWORD ServiceBufferSize,       // 서비스 이름 버퍼 크기
    INT Flags                      // 동작 방식 제어 플래그
);
```

### DNS 조회 예제

```cpp
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <memory>

// getaddrinfo 사용 예제 (호스트 이름 -> IP 주소)
void dnsLookupExample() {
    const char* hostname = "www.example.com";
    const char* port = "80";
    
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;     // IPv4와 IPv6 모두 허용
    hints.ai_socktype = SOCK_STREAM; // TCP 소켓
    
    addrinfo* result = nullptr;
    int error = getaddrinfo(hostname, port, &hints, &result);
    
    if (error != 0) {
        std::cerr << std::format("getaddrinfo 실패: {}\n", gai_strerror(error));
        return;
    }
    
    // 스마트 포인터로 리소스 관리
    auto freeResult = std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>(
        result, &freeaddrinfo);
    
    std::cout << std::format("'{}' 호스트에 대한 DNS 조회 결과:\n", hostname);
    
    // 모든 결과 순회
    for (auto* addr = result; addr != nullptr; addr = addr->ai_next) {
        // 주소 정보 출력
        char ipstr[INET6_ADDRSTRLEN];
        void* addrPtr = nullptr;
        
        if (addr->ai_family == AF_INET) {
            // IPv4
            auto* ipv4 = reinterpret_cast<sockaddr_in*>(addr->ai_addr);
            addrPtr = &ipv4->sin_addr;
        } else if (addr->ai_family == AF_INET6) {
            // IPv6
            auto* ipv6 = reinterpret_cast<sockaddr_in6*>(addr->ai_addr);
            addrPtr = &ipv6->sin6_addr;
        } else {
            continue;  // 지원되지 않는 주소 체계
        }
        
        // IP 주소를 문자열로 변환
        inet_ntop(addr->ai_family, addrPtr, ipstr, sizeof(ipstr));
        
        std::cout << std::format("  {} 주소: {}\n", 
                                 addr->ai_family == AF_INET ? "IPv4" : "IPv6", 
                                 ipstr);
    }
}

// getnameinfo 사용 예제 (IP 주소 -> 호스트 이름)
void reverseDnsLookupExample() {
    // 테스트할 IP 주소 (구글 DNS 서버 8.8.8.8)
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, "8.8.8.8", &sa.sin_addr);
    
    char hostname[NI_MAXHOST];
    char servname[NI_MAXSERV];
    
    int error = getnameinfo(
        reinterpret_cast<sockaddr*>(&sa), sizeof(sa),
        hostname, NI_MAXHOST,
        servname, NI_MAXSERV,
        0);
    
    if (error != 0) {
        std::cerr << std::format("getnameinfo 실패: {}\n", gai_strerror(error));
        return;
    }
    
    std::cout << std::format("IP 주소 8.8.8.8의 호스트 이름: {}\n", hostname);
}
```

### C++23을 활용한 DNS 조회 래퍼
현대적인 C++ 스타일로 DNS 조회 기능을 래핑하는 클래스입니다:

```cpp
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <memory>
#include <expected>
#include <string_view>
#include <span>

// DNS 조회 결과 클래스
class DnsResult {
public:
    DnsResult(addrinfo* result) : m_result(result, &freeaddrinfo) {}
    
    // 결과에서 첫 번째 유효한 주소 반환
    std::expected<sockaddr_storage, int> getFirstAddress() const {
        for (auto* addr = m_result.get(); addr != nullptr; addr = addr->ai_next) {
            if (addr->ai_family == AF_INET || addr->ai_family == AF_INET6) {
                sockaddr_storage storage{};
                std::memcpy(&storage, addr->ai_addr, addr->ai_addrlen);
                return storage;
            }
        }
        return std::unexpected(WSANO_DATA);
    }
    
    // 모든 IP 주소를 문자열로 변환하여 반환
    std::vector<std::string> getAllIpAddresses() const {
        std::vector<std::string> addresses;
        
        for (auto* addr = m_result.get(); addr != nullptr; addr = addr->ai_next) {
            if (addr->ai_family != AF_INET && addr->ai_family != AF_INET6) {
                continue;
            }
            
            char ipstr[INET6_ADDRSTRLEN];
            void* addrPtr = nullptr;
            
            if (addr->ai_family == AF_INET) {
                auto* ipv4 = reinterpret_cast<sockaddr_in*>(addr->ai_addr);
                addrPtr = &ipv4->sin_addr;
            } else {
                auto* ipv6 = reinterpret_cast<sockaddr_in6*>(addr->ai_addr);
                addrPtr = &ipv6->sin6_addr;
            }
            
            inet_ntop(addr->ai_family, addrPtr, ipstr, sizeof(ipstr));
            addresses.emplace_back(ipstr);
        }
        
        return addresses;
    }
    
    // 첫 번째 주소로 소켓 생성
    std::expected<SOCKET, int> createSocket() const {
        for (auto* addr = m_result.get(); addr != nullptr; addr = addr->ai_next) {
            SOCKET sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (sock != INVALID_SOCKET) {
                return sock;
            }
        }
        return std::unexpected(WSAGetLastError());
    }
    
private:
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> m_result;
};

// DNS 조회 클래스
class DnsResolver {
public:
    // 호스트 이름으로 IP 주소 조회
    static std::expected<DnsResult, int> resolve(
            std::string_view hostname, 
            std::string_view service = "",
            int family = AF_UNSPEC,
            int socktype = SOCK_STREAM) {
        
        addrinfo hints{};
        hints.ai_family = family;     // 주소 체계 (AF_INET, AF_INET6, AF_UNSPEC 등)
        hints.ai_socktype = socktype; // 소켓 유형 (SOCK_STREAM, SOCK_DGRAM 등)
        
        addrinfo* result = nullptr;
        int error = getaddrinfo(
            hostname.empty() ? nullptr : hostname.data(),
            service.empty() ? nullptr : service.data(),
            &hints, &result);
        
        if (error != 0) {
            return std::unexpected(error);
        }
        
        return DnsResult(result);
    }
    
    // IP 주소로 호스트 이름 조회
    static std::expected<std::string, int> resolveAddress(
            const sockaddr_storage& addr) {
        
        char hostname[NI_MAXHOST];
        
        int error = getnameinfo(
            reinterpret_cast<const sockaddr*>(&addr),
            (addr.ss_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6),
            hostname, NI_MAXHOST,
            nullptr, 0,
            0);
        
        if (error != 0) {
            return std::unexpected(error);
        }
        
        return std::string(hostname);
    }
};

// 사용 예시
void modernDnsExample() {
    // 호스트 이름으로 IP 주소 조회
    auto result = DnsResolver::resolve("www.example.com", "http");
    
    if (!result) {
        std::cerr << std::format("DNS 조회 실패: {}\n", gai_strerror(result.error()));
        return;
    }
    
    // 모든 IP 주소 출력
    auto addresses = result->getAllIpAddresses();
    std::cout << "www.example.com의 IP 주소:\n";
    for (const auto& addr : addresses) {
        std::cout << std::format("  {}\n", addr);
    }
    
    // 첫 번째 주소로 소켓 생성
    auto socketResult = result->createSocket();
    if (socketResult) {
        SOCKET sock = *socketResult;
        std::cout << "소켓 생성 성공\n";
        closesocket(sock);
    }
    
    // 역방향 DNS 조회 (IP -> 호스트 이름)
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, "8.8.8.8", &sa.sin_addr);
    
    sockaddr_storage storage{};
    std::memcpy(&storage, &sa, sizeof(sa));
    
    auto hostnameResult = DnsResolver::resolveAddress(storage);
    
    if (hostnameResult) {
        std::cout << std::format("8.8.8.8의 호스트 이름: {}\n", *hostnameResult);
    } else {
        std::cerr << std::format("역방향 DNS 조회 실패: {}\n", 
                                gai_strerror(hostnameResult.error()));
    }
}
```
  
### 게임 서버에서의 DNS 활용
게임 서버 개발에서는 다음과 같은 상황에서 DNS 기능을 활용할 수 있습니다:  
게임 서버 간에 Socket으로 연결할 때 IP 주소로 연결을 할 수 있지만 DNS 주소를 사용하여 연결하는 것이 좋습니다.  DNS를 사용하면 IP가 변경 되어도 DNS와 IP 맵핑만 바꾸면 되므로 코드에서 주소를 변경할 필요가 없습니다.   

1. **마스터 서버 찾기**: 게임 서버가 중앙 마스터 서버에 연결할 때
2. **부하 분산**: DNS 라운드 로빈을 통한 서버 부하 분산
3. **서버 간 통신**: 분산 서버 환경에서 다른 서버와 통신할 때
4. **클라이언트 위치 확인**: IP 기반 지역 분류

```cpp
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <format>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <map>

// 게임 서버에서의 DNS 캐시 관리자 예시
class DnsCacheManager {
public:
    // 호스트 이름으로 IP 주소 조회 (캐시 사용)
    std::expected<std::string, int> resolveHostname(const std::string& hostname) {
        {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            
            // 캐시에서 찾기
            auto it = m_dnsCache.find(hostname);
            if (it != m_dnsCache.end()) {
                // 만료 시간 확인
                if (std::chrono::steady_clock::now() < it->second.expiryTime) {
                    return it->second.ipAddress;
                }
                // 만료된 항목 제거
                m_dnsCache.erase(it);
            }
        }
        
        // 캐시에 없으면 실제 DNS 조회 수행
        auto result = DnsResolver::resolve(hostname);
        if (!result) {
            return std::unexpected(result.error());
        }
        
        auto addresses = result->getAllIpAddresses();
        if (addresses.empty()) {
            return std::unexpected(WSANO_DATA);
        }
        
        // 첫 번째 주소 사용
        std::string ipAddress = addresses[0];
        
        // 캐시에 저장 (1시간 유효)
        CacheEntry entry{
            ipAddress,
            std::chrono::steady_clock::now() + std::chrono::hours(1)
        };
        
        {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            m_dnsCache[hostname] = entry;
        }
        
        return ipAddress;
    }
    
    // 캐시 정리 (만료된 항목 제거)
    void cleanupCache() {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        auto now = std::chrono::steady_clock::now();
        
        for (auto it = m_dnsCache.begin(); it != m_dnsCache.end(); ) {
            if (now >= it->second.expiryTime) {
                it = m_dnsCache.erase(it);
            } else {
                ++it;
            }
        }
    }
    
private:
    struct CacheEntry {
        std::string ipAddress;
        std::chrono::steady_clock::time_point expiryTime;
    };
    
    std::mutex m_cacheMutex;
    std::map<std::string, CacheEntry> m_dnsCache;
};

// 마스터 서버 연결 관리자 예시
class MasterServerConnector {
public:
    MasterServerConnector(std::string masterServerDomain)
        : m_masterServerDomain(std::move(masterServerDomain)) {
    }
    
    // 마스터 서버에 연결
    std::expected<SOCKET, int> connectToMasterServer() {
        // 마스터 서버 도메인 이름 해석
        auto ipResult = m_dnsCache.resolveHostname(m_masterServerDomain);
        if (!ipResult) {
            std::cerr << std::format("마스터 서버 DNS 조회 실패: {}\n", 
                                    gai_strerror(ipResult.error()));
            return std::unexpected(ipResult.error());
        }
        
        std::cout << std::format("마스터 서버 IP: {}\n", *ipResult);
        
        // 소켓 생성
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            return std::unexpected(WSAGetLastError());
        }
        
        // 서버 주소 설정
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(12345);  // 마스터 서버 포트
        inet_pton(AF_INET, ipResult->c_str(), &serverAddr.sin_addr);
        
        // 연결
        if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            int error = WSAGetLastError();
            closesocket(sock);
            return std::unexpected(error);
        }
        
        return sock;
    }
    
private:
    std::string m_masterServerDomain;
    DnsCacheManager m_dnsCache;
};

// 사용 예시
void gameMasterServerExample() {
    MasterServerConnector connector("master.mygame.com");
    
    auto socketResult = connector.connectToMasterServer();
    if (socketResult) {
        SOCKET masterSocket = *socketResult;
        std::cout << "마스터 서버 연결 성공\n";
        
        // 마스터 서버와 통신...
        
        closesocket(masterSocket);
    } else {
        std::cerr << std::format("마스터 서버 연결 실패: 오류 코드 {}\n", 
                                socketResult.error());
    }
}
```
  
이러한 DNS 기능들을 활용하면 보다 유연하고 확장 가능한 게임 서버 인프라를 구축할 수 있습니다. 동적인 서버 탐색, 로드 밸런싱, 리전 기반 서버 할당 등 다양한 고급 기능을 구현할 수 있게 됩니다.       
  
      
<br>      
      
# Chapter 04. TCP 서버-클라이언트 
  
## 01 TCP 서버-클라이언트 구조
TCP(Transmission Control Protocol)는 신뢰성 있는 데이터 전송을 보장하는 연결 지향적 프로토콜입니다. 온라인 게임에서는 데이터의 정확한 전달이 중요하기 때문에 TCP가 널리 사용됩니다. 

### TCP의 주요 특징
- **연결 지향적**: 통신 전 연결 설정이 필요합니다
- **신뢰성**: 패킷 손실 시 자동으로 재전송됩니다
- **순서 보장**: 데이터가 보낸 순서대로 도착합니다
- **흐름/혼잡 제어**: 네트워크 상황에 따라 데이터 전송 속도를 조절합니다

### TCP 서버-클라이언트 기본 구조
**서버 측 동작**:
1. 소켓 생성 (socket)
2. 소켓에 주소 바인딩 (bind)
3. 연결 대기 상태로 전환 (listen)
4. 클라이언트 연결 수락 (accept)
5. 데이터 송수신 (send/recv)
6. 연결 종료 (close)

**클라이언트 측 동작**:
1. 소켓 생성 (socket)
2. 서버에 연결 요청 (connect)
3. 데이터 송수신 (send/recv)
4. 연결 종료 (close)

### Windows 네트워크 프로그래밍 기초
Windows에서 네트워크 프로그래밍을 하기 위해 Winsock API를 사용합니다. 기본적인 순서는 다음과 같습니다:

1. WSAStartup 함수로 Winsock 초기화
2. 소켓 생성, 바인딩, 연결 등의 작업 수행
3. 데이터 송수신
4. WSACleanup 함수로 Winsock 종료
  

## 02 TCP 서버-클라이언트 분석

### TCP 서버 구현
![TCP 서버 프로그램의 스레드](./images/010.png)   
  
다음은 C++23과 최신 Win32 API를 사용한 TCP 서버 구현 예시입니다:  
`codes/tcp_server_02`  

```cpp
#include <iostream>
#include <string>
#include <format>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class TCPServer {
private:
    SOCKET listenSocket;
    std::vector<std::thread> clientThreads;
    bool running;
    
    static constexpr int BUFFER_SIZE = 1024;
    static constexpr int DEFAULT_PORT = 27015;

public:
    TCPServer() : listenSocket(INVALID_SOCKET), running(false) {}
    
    ~TCPServer() {
        Stop();
    }
    
    bool Start(int port = DEFAULT_PORT) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << std::format("WSAStartup 실패: {}\n", result);
            return false;
        }
        
        // 소켓 생성
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        // 서버 주소 설정
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;  // 모든 인터페이스에서 접속 허용
        
        // 소켓 바인딩
        result = bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("바인딩 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        // 연결 대기 시작
        result = listen(listenSocket, SOMAXCONN);
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("리슨 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        running = true;
        std::cout << std::format("TCP 서버가 포트 {}에서 시작되었습니다.\n", port);
        
        // 클라이언트 연결 수락 스레드 시작
        std::thread acceptThread(&TCPServer::AcceptClients, this);
        acceptThread.detach();
        
        return true;
    }
    
    void Stop() {
        running = false;
        
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        
        for (auto& thread : clientThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        clientThreads.clear();
        WSACleanup();
        std::cout << "TCP 서버가 중지되었습니다.\n";
    }
    
private:
    void AcceptClients() {
        while (running) {
            // 클라이언트 연결 수락
            sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << std::format("클라이언트 연결 수락 실패: {}\n", WSAGetLastError());
                }
                continue;
            }
            
            // 클라이언트 IP 주소 얻기
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::cout << std::format("새 클라이언트 연결: {}:{}\n", clientIP, ntohs(clientAddr.sin_port));
            
            // 클라이언트 처리 스레드 시작
            clientThreads.emplace_back(&TCPServer::HandleClient, this, clientSocket, std::string(clientIP));
        }
    }
    
    void HandleClient(SOCKET clientSocket, std::string clientIP) {
        char buffer[BUFFER_SIZE];
        
        while (running) {
            // 데이터 수신
            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << std::format("클라이언트 {}가 연결을 종료했습니다.\n", clientIP);
                } else {
                    std::cerr << std::format("recv 실패: {}\n", WSAGetLastError());
                }
                break;
            }
            
            // 수신된 데이터 처리
            buffer[bytesReceived] = '\0';
            std::cout << std::format("{}로부터 수신: {}\n", clientIP, buffer);
            
            // 클라이언트에게 에코 응답
            std::string response = std::format("서버 에코: {}", buffer);
            int bytesSent = send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("send 실패: {}\n", WSAGetLastError());
                break;
            }
        }
        
        // 클라이언트 소켓 닫기
        closesocket(clientSocket);
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    TCPServer server;
    if (server.Start()) {
        std::cout << "서버를 종료하려면 아무 키나 누르세요...\n";
        std::cin.get();
        server.Stop();
    }
    
    return 0;
}
```

### TCP 클라이언트 구현  
`codes/tcp_client_02`  

```cpp
#include <iostream>
#include <string>
#include <format>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class TCPClient {
private:
    SOCKET clientSocket;
    bool connected;
    std::thread receiveThread;
    
    static constexpr int BUFFER_SIZE = 1024;
    static constexpr int DEFAULT_PORT = 27015;

public:
    TCPClient() : clientSocket(INVALID_SOCKET), connected(false) {}
    
    ~TCPClient() {
        Disconnect();
    }
    
    bool Connect(const std::string& serverIP, int port = DEFAULT_PORT) {
        // Winsock 초기화
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << std::format("WSAStartup 실패: {}\n", result);
            return false;
        }
        
        // 소켓 생성
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        // 서버 주소 설정
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
        
        // 서버에 연결
        result = connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("서버 연결 실패: {}\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }
        
        connected = true;
        std::cout << std::format("{}:{}에 연결되었습니다.\n", serverIP, port);
        
        // 수신 스레드 시작
        receiveThread = std::thread(&TCPClient::ReceiveMessages, this);
        
        return true;
    }
    
    void Disconnect() {
        connected = false;
        
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
        
        if (receiveThread.joinable()) {
            receiveThread.join();
        }
        
        WSACleanup();
        std::cout << "서버와의 연결이 종료되었습니다.\n";
    }
    
    bool SendMessage(const std::string& message) {
        if (!connected || clientSocket == INVALID_SOCKET) {
            std::cerr << "연결되지 않았습니다.\n";
            return false;
        }
        
        int bytesSent = send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << std::format("메시지 전송 실패: {}\n", WSAGetLastError());
            return false;
        }
        
        return true;
    }
    
private:
    void ReceiveMessages() {
        char buffer[BUFFER_SIZE];
        
        while (connected) {
            // 데이터 수신
            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << "서버가 연결을 종료했습니다.\n";
                } else {
                    std::cerr << std::format("recv 실패: {}\n", WSAGetLastError());
                }
                connected = false;
                break;
            }
            
            // 수신된 데이터 처리
            buffer[bytesReceived] = '\0';
            std::cout << "서버로부터 수신: " << buffer << std::endl;
        }
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    TCPClient client;
    std::string serverIP;
    
    std::cout << "서버 IP를 입력하세요 (localhost는 127.0.0.1): ";
    std::getline(std::cin, serverIP);
    
    if (client.Connect(serverIP)) {
        std::string message;
        while (true) {
            std::cout << "전송할 메시지 (종료: exit): ";
            std::getline(std::cin, message);
            
            if (message == "exit") {
                break;
            }
            
            client.SendMessage(message);
        }
        
        client.Disconnect();
    }
    
    return 0;
}
```

### 서버-클라이언트 구현 분석

1. **서버 동작 방식**:
   - `Start()` 메서드에서 소켓을 생성하고 바인딩한 후 연결 대기
   - `AcceptClients()` 메서드는 별도 스레드에서 실행되며 클라이언트 연결을 수락
   - 각 클라이언트 연결마다 `HandleClient()` 메서드를 실행하는 새 스레드 생성
   - 에코 서버 방식으로 수신한 메시지를 그대로 클라이언트에게 반환

2. **클라이언트 동작 방식**:
   - `Connect()` 메서드에서 서버에 연결
   - 수신 메시지를 처리하기 위한 별도 스레드 생성 (`ReceiveMessages()`)
   - 메인 스레드는 사용자 입력을 받아 서버로 메시지 전송

3. **멀티스레딩 구현**:
   - C++11부터 도입된 `std::thread`를 사용하여 멀티스레딩 구현
   - 각 클라이언트 연결을 별도 스레드로 처리하여 동시 다중 접속 지원
   - 비동기 메시지 수신을 위해 수신 전용 스레드 사용

4. **에러 처리**:
   - 각 단계마다 상세한 에러 메시지 출력
   - 연결 종료 시 적절한 리소스 정리
  

## 03 TCP 서버-클라이언트(IPv6)
IPv6는 주소 고갈 문제를 해결하기 위해 도입된 차세대 인터넷 프로토콜입니다. 최신 게임 서버는 IPv6 지원이 필수적입니다.

### IPv6 TCP 서버 구현 
`codes/tcp_server_03`   
`codes/tcp_server_02`는 한번에 1개의 클라이언트 접속할 수 있지만 `codes/tcp_server_03`은 thread를 사용하여 동시에 복수의 클라이언트가 접속할 수 있다.          
  
```cpp
#include <iostream>
#include <string>
#include <format>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class TCPServerIPv6 {
private:
    SOCKET listenSocket;
    std::vector<std::thread> clientThreads;
    bool running;
    
    static constexpr int BUFFER_SIZE = 1024;
    static constexpr int DEFAULT_PORT = 27015;

public:
    TCPServerIPv6() : listenSocket(INVALID_SOCKET), running(false) {}
    
    ~TCPServerIPv6() {
        Stop();
    }
    
    bool Start(int port = DEFAULT_PORT) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << std::format("WSAStartup 실패: {}\n", result);
            return false;
        }
        
        // IPv6 소켓 생성
        listenSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        // IPv4 매핑된 IPv6 주소 허용 (듀얼 스택)
        int ipv6Only = 0; // 0: 듀얼 스택(IPv4+IPv6), 1: IPv6만
        result = setsockopt(listenSocket, IPPROTO_IPV6, IPV6_V6ONLY, 
                          reinterpret_cast<char*>(&ipv6Only), sizeof(ipv6Only));
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("IPv6 옵션 설정 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        // 서버 주소 설정 (IPv6)
        sockaddr_in6 serverAddr;
        ZeroMemory(&serverAddr, sizeof(serverAddr));
        serverAddr.sin6_family = AF_INET6;
        serverAddr.sin6_port = htons(port);
        serverAddr.sin6_addr = in6addr_any; // 모든 IPv6 인터페이스에서 접속 허용
        
        // 소켓 바인딩
        result = bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("바인딩 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        // 연결 대기 시작
        result = listen(listenSocket, SOMAXCONN);
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("리슨 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        running = true;
        std::cout << std::format("TCP IPv6 서버가 포트 {}에서 시작되었습니다.\n", port);
        
        // 클라이언트 연결 수락 스레드 시작
        std::thread acceptThread(&TCPServerIPv6::AcceptClients, this);
        acceptThread.detach();
        
        return true;
    }
    
    void Stop() {
        running = false;
        
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        
        for (auto& thread : clientThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        clientThreads.clear();
        WSACleanup();
        std::cout << "TCP IPv6 서버가 중지되었습니다.\n";
    }
    
private:
    void AcceptClients() {
        while (running) {
            // 클라이언트 연결 수락
            sockaddr_in6 clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << std::format("클라이언트 연결 수락 실패: {}\n", WSAGetLastError());
                }
                continue;
            }
            
            // 클라이언트 IP 주소 얻기
            char clientIP[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &clientAddr.sin6_addr, clientIP, INET6_ADDRSTRLEN);
            std::cout << std::format("새 클라이언트 연결: [{}]:{}\n", clientIP, ntohs(clientAddr.sin6_port));
            
            // 클라이언트 처리 스레드 시작
            clientThreads.emplace_back(&TCPServerIPv6::HandleClient, this, clientSocket, std::string(clientIP));
        }
    }
    
    void HandleClient(SOCKET clientSocket, std::string clientIP) {
        char buffer[BUFFER_SIZE];
        
        while (running) {
            // 데이터 수신
            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << std::format("클라이언트 {}가 연결을 종료했습니다.\n", clientIP);
                } else {
                    std::cerr << std::format("recv 실패: {}\n", WSAGetLastError());
                }
                break;
            }
            
            // 수신된 데이터 처리
            buffer[bytesReceived] = '\0';
            std::cout << std::format("{}로부터 수신: {}\n", clientIP, buffer);
            
            // 클라이언트에게 에코 응답
            std::string response = std::format("IPv6 서버 에코: {}", buffer);
            int bytesSent = send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("send 실패: {}\n", WSAGetLastError());
                break;
            }
        }
        
        // 클라이언트 소켓 닫기
        closesocket(clientSocket);
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    TCPServerIPv6 server;
    if (server.Start()) {
        std::cout << "서버를 종료하려면 아무 키나 누르세요...\n";
        std::cin.get();
        server.Stop();
    }
    
    return 0;
}
```

### IPv6 TCP 클라이언트 구현
`codes/tcp_client_03`    

```cpp
#include <iostream>
#include <string>
#include <format>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class TCPClientIPv6 {
private:
    SOCKET clientSocket;
    bool connected;
    std::thread receiveThread;
    
    static constexpr int BUFFER_SIZE = 1024;
    static constexpr int DEFAULT_PORT = 27015;

public:
    TCPClientIPv6() : clientSocket(INVALID_SOCKET), connected(false) {}
    
    ~TCPClientIPv6() {
        Disconnect();
    }
    
    bool Connect(const std::string& serverIP, int port = DEFAULT_PORT) {
        // Winsock 초기화
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << std::format("WSAStartup 실패: {}\n", result);
            return false;
        }
        
        // IPv6 소켓 생성
        clientSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        // 서버 주소 설정 (IPv6)
        sockaddr_in6 serverAddr;
        ZeroMemory(&serverAddr, sizeof(serverAddr));
        serverAddr.sin6_family = AF_INET6;
        serverAddr.sin6_port = htons(port);
        
        // IPv6 주소 변환
        if (inet_pton(AF_INET6, serverIP.c_str(), &serverAddr.sin6_addr) != 1) {
            std::cerr << "잘못된 IPv6 주소 형식입니다.\n";
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }
        
        // 서버에 연결
        result = connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("서버 연결 실패: {}\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }
        
        connected = true;
        std::cout << std::format("[{}]:{}에 연결되었습니다.\n", serverIP, port);
        
        // 수신 스레드 시작
        receiveThread = std::thread(&TCPClientIPv6::ReceiveMessages, this);
        
        return true;
    }
    
    void Disconnect() {
        connected = false;
        
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
        
        if (receiveThread.joinable()) {
            receiveThread.join();
        }
        
        WSACleanup();
        std::cout << "서버와의 연결이 종료되었습니다.\n";
    }
    
    bool SendMessage(const std::string& message) {
        if (!connected || clientSocket == INVALID_SOCKET) {
            std::cerr << "연결되지 않았습니다.\n";
            return false;
        }
        
        int bytesSent = send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << std::format("메시지 전송 실패: {}\n", WSAGetLastError());
            return false;
        }
        
        return true;
    }
    
private:
    void ReceiveMessages() {
        char buffer[BUFFER_SIZE];
        
        while (connected) {
            // 데이터 수신
            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << "서버가 연결을 종료했습니다.\n";
                } else {
                    std::cerr << std::format("recv 실패: {}\n", WSAGetLastError());
                }
                connected = false;
                break;
            }
            
            // 수신된 데이터 처리
            buffer[bytesReceived] = '\0';
            std::cout << "서버로부터 수신: " << buffer << std::endl;
        }
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    TCPClientIPv6 client;
    std::string serverIP;
    
    std::cout << "서버 IPv6 주소를 입력하세요 (localhost는 ::1): ";
    std::getline(std::cin, serverIP);
    
    if (client.Connect(serverIP)) {
        std::string message;
        while (true) {
            std::cout << "전송할 메시지 (종료: exit): ";
            std::getline(std::cin, message);
            
            if (message == "exit") {
                break;
            }
            
            client.SendMessage(message);
        }
        
        client.Disconnect();
    }
    
    return 0;
}
```

### IPv4와 IPv6의 주요 차이점
1. **주소 체계**:
   - IPv4: 32비트 주소 (예: 192.168.0.1)
   - IPv6: 128비트 주소 (예: 2001:0db8:85a3:0000:8a2e:0370:7334)

2. **구현 차이**:
   - 소켓 생성 시 주소 패밀리를 `AF_INET6`로 설정
   - IPv6 주소를 저장하기 위해 `sockaddr_in6` 구조체 사용
   - 듀얼 스택 지원을 위한 `IPV6_V6ONLY` 소켓 옵션 설정

3. **듀얼 스택**:
   - Windows에서는 IPv6 소켓이 IPv4 연결도 수락하도록 설정 가능
   - `IPV6_V6ONLY` 옵션을 0으로 설정하여 구현
   - 하나의 서버로 IPv4와 IPv6 클라이언트 모두 지원 가능

### 게임 서버 개발자를 위한 추가 고려 사항
1. **비동기 I/O 및 성능 최적화**:
   - 실제 게임 서버에서는 비동기 I/O(IOCP, Overlapped I/O)를 활용하여 성능 향상
   - I/O 멀티플렉싱(select, WSAPoll 등) 고려

2. **패킷 설계**:
   - 게임 서버에서는 바이너리 프로토콜을 사용하여 효율적인 데이터 전송
   - 헤더-페이로드 구조의 패킷 설계
   - 직렬화/역직렬화 라이브러리 활용(Protocol Buffers, FlatBuffers 등)

3. **세션 관리**:
   - 클라이언트 연결을 객체 지향적으로 관리하는 세션 시스템 구현
   - 연결/연결 해제 이벤트 처리
   - 타임아웃, 핑-퐁 메커니즘으로 연결 상태 확인

4. **스레드 관리**:
   - 스레드 풀을 사용하여 리소스 효율적 사용
   - 스레드 간 공유 자원 동기화
   - 락 최소화를 통한 성능 향상

5. **보안 고려사항**:
   - 패킷 검증으로 조작 방지
   - TLS/SSL 적용으로 암호화 통신
   - DDoS 방어 전략 수립

이 예제들은 기본적인 TCP 서버-클라이언트 구현을 보여주는 것으로, 실제 게임 서버 개발에는 더 복잡한 아키텍처와 최적화가 필요합니다. 그러나 기본 개념을 이해하는 데 좋은 출발점이 될 것입니다.  

  
<br>      
   



  
<br>      
   




  
<br>      
  



  
<br>      
   
