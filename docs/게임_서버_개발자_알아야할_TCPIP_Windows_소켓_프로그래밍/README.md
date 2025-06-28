# 게임 서버 개발자가 알아야할 TCP/IP Windows 소켓 프로그래밍

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
   
# Chapter.05 데이터 전송하기

## 01 응용 프로그램 프로토콜과 데이터 전송
온라인 게임에서 서버와 클라이언트 간의 데이터 전송은 게임의 핵심입니다. 이 데이터 교환을 위해서는 양측이 이해할 수 있는 규약, 즉 **응용 프로그램 프로토콜**이 필요합니다.

### 응용 프로그램 프로토콜이란?
응용 프로그램 프로토콜은 **두 프로그램이 서로 통신할 때 사용하는 데이터 형식과 규칙을 정의**한 것입니다.   
게임에서는 이 프로토콜을 통해 캐릭터 움직임, 공격, 채팅 등 다양한 정보를 주고받습니다.  

### 바이트 순서(Endianness) 
네트워크 통신에서 가장 기본적인 문제 중 하나는 바이트 순서입니다.

```cpp
// 네트워크 바이트 순서(빅 엔디안)로 변환
uint16_t hostToNetwork16(uint16_t value) {
    return htons(value); // host to network short
}

uint32_t hostToNetwork32(uint32_t value) {
    return htonl(value); // host to network long
}

// 호스트 바이트 순서로 변환
uint16_t networkToHost16(uint16_t value) {
    return ntohs(value); // network to host short
}

uint32_t networkToHost32(uint32_t value) {
    return ntohl(value); // network to host long
}
```
  
C++로 만든 서버와 클라이언트 푸로그램이 x86_64 CPU 아키텍처에서 실행된다면 바이트 순서 문제는 대체로 고려하지 않아도 됩니다. x86_64 CPU 아키텍처는 `리틀 엔디안` 입니다. 또 C#을 사용하는 경우 CPU 아키텍처와 상관 없이 .NET 플랫폼은 `리틀 엔디안` 입니다.
  
![](./images/027.png)   
![](./images/028.png)   


프로그래밍 언어별 바이트 순서 처리  
  
C/C++ - 플랫폼 의존적  
```
#include <arpa/inet.h>  // Linux/Unix
#include <winsock2.h>   // Windows

// 엔디안 확인
bool is_little_endian() {
    uint32_t test = 1;
    return *(uint8_t*)&test == 1;
}

// 네트워크 바이트 순서 변환
uint32_t host_to_network(uint32_t host_value) {
    return htonl(host_value);  // Host TO Network Long
}

uint32_t network_to_host(uint32_t network_value) {
    return ntohl(network_value);  // Network TO Host Long
}

// 수동 바이트 순서 변환
uint32_t swap_bytes(uint32_t value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x000000FF) << 24);
}
```  
  
Rust - 명시적 엔디안 제어  
```
// 바이트 순서 변환
let value: u32 = 0x12345678;

// 네트워크 바이트 순서 (Big Endian)
let big_endian = value.to_be();
let from_big = u32::from_be(big_endian);

// 호스트 바이트 순서 (Little Endian)
let little_endian = value.to_le();
let from_little = u32::from_le(little_endian);

// 바이트 배열로 직렬화
let bytes_be = value.to_be_bytes();  // [0x12, 0x34, 0x56, 0x78]
let bytes_le = value.to_le_bytes();  // [0x78, 0x56, 0x34, 0x12]

// 바이트 배열에서 복원
let restored_be = u32::from_be_bytes([0x12, 0x34, 0x56, 0x78]);
let restored_le = u32::from_le_bytes([0x78, 0x56, 0x34, 0x12]);
Go - 내장 바이너리 패키지
import (
    "encoding/binary"
    "bytes"
)

func endianExample() {
    value := uint32(0x12345678)
    buf := new(bytes.Buffer)
    
    // Big Endian 쓰기
    binary.Write(buf, binary.BigEndian, value)
    // 결과: [0x12, 0x34, 0x56, 0x78]
    
    // Little Endian 쓰기
    buf.Reset()
    binary.Write(buf, binary.LittleEndian, value)
    // 결과: [0x78, 0x56, 0x34, 0x12]
    
    // 읽기
    var result uint32
    binary.Read(buf, binary.LittleEndian, &result)
}
```  
  
Java - DataInputStream/DataOutputStream  
```
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

// Java는 기본적으로 Big Endian
public class EndianExample {
    public static void main(String[] args) throws IOException {
        int value = 0x12345678;
        
        // Big Endian (Java 기본값)
        ByteBuffer bigBuffer = ByteBuffer.allocate(4);
        bigBuffer.putInt(value);
        byte[] bigBytes = bigBuffer.array();
        // 결과: [0x12, 0x34, 0x56, 0x78]
        
        // Little Endian
        ByteBuffer littleBuffer = ByteBuffer.allocate(4);
        littleBuffer.order(ByteOrder.LITTLE_ENDIAN);
        littleBuffer.putInt(value);
        byte[] littleBytes = littleBuffer.array();
        // 결과: [0x78, 0x56, 0x34, 0x12]
    }
}
```  
  
Python - struct 모듈
```  
import struct
import socket

value = 0x12345678

# Big Endian 패킹
big_endian = struct.pack('>I', value)     # '>' = Big Endian
print(f"Big Endian: {big_endian}")
# 결과: b'\x12\x34\x56\x78'

# Little Endian 패킹  
little_endian = struct.pack('<I', value)  # '<' = Little Endian
print(f"Little Endian: {little_endian}")
# 결과: b'xV4\x12'

# 언패킹 예제
unpacked_big = struct.unpack('>I', big_endian)[0]
unpacked_little = struct.unpack('<I', little_endian)[0]

print(f"Original value: 0x{value:08x}")
print(f"Unpacked from big endian: 0x{unpacked_big:08x}")
print(f"Unpacked from little endian: 0x{unpacked_little:08x}")

# 네트워크 바이트 순서 (항상 big endian)
network_order = socket.htonl(value)  # host to network long
print(f"Network order: 0x{network_order:08x}")
```  
  
    
### 바이트 정렬
  
![](./images/023.png)   
![](./images/024.png)   
 
C++ 구조체에 패딩이 필요한 이유는 **메모리 정렬(memory alignment)** 때문이다.

#### 메모리 정렬이 필요한 이유
CPU는 데이터를 특정 경계에서 읽을 때 가장 효율적으로 동작한다. 예를 들어 4바이트 정수는 4의 배수 주소에서, 8바이트 double은 8의 배수 주소에서 읽는 것이 빠르다.

##### 패딩 없이 구조체를 배치하면?

```cpp
struct Example {
    char a;     // 1바이트
    int b;      // 4바이트  
    char c;     // 1바이트
    double d;   // 8바이트
};
```

**패딩 없는 메모리 배치:**
```
주소:  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
데이터: a  b  b  b  b  c  d  d  d  d  d  d  d  d
```

이 경우 문제점들:
- `int b`가 주소 1에서 시작 → 4바이트 경계에 정렬되지 않음
- `double d`가 주소 6에서 시작 → 8바이트 경계에 정렬되지 않음

#### 패딩을 추가한 실제 메모리 배치:

```
주소:  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
데이터: a  P  P  P  b  b  b  b  c  P  P  P  P  P  P  P  d  d  d  d  d  d  d  d
```

여기서 `P`는 패딩(빈 공간)이다.

**실제 구조체 크기와 오프셋:**
```cpp
struct Example {
    char a;     // 오프셋 0, 크기 1
    // 3바이트 패딩
    int b;      // 오프셋 4, 크기 4  
    char c;     // 오프셋 8, 크기 1
    // 7바이트 패딩
    double d;   // 오프셋 16, 크기 8
};
// 전체 크기: 24바이트
```

#### 시각적 비교

**패딩 없음 (비효율적):**
```
┌─┬─────┬─┬───────┐
│a│ b b │c│ d d d │  ← 정렬되지 않음
└─┴─────┴─┴───────┘
```

**패딩 있음 (효율적):**
```
┌─┬───┬─────┬─┬───────┬───────────┐
│a│PAD│ b b │c│  PAD  │  d d d d  │  ← 모든 데이터가 올바르게 정렬됨
└─┴───┴─────┴─┴───────┴───────────┘
```

#### 패딩의 장점
1. **성능 향상**: CPU가 한 번의 메모리 접근으로 데이터를 읽을 수 있다
2. **안정성**: 일부 CPU는 정렬되지 않은 접근 시 오류를 발생시킨다
3. **캐시 효율성**: 메모리 캐시 라인과 잘 맞아떨어진다

따라서 컴파일러는 자동으로 패딩을 추가해서 모든 멤버가 적절한 경계에 정렬되도록 한다.    


### 직렬화와 역직렬화
직렬화(Serialization)는 메모리 내의 데이터 구조를 바이트 스트림으로 변환하는 과정이고, 역직렬화(Deserialization)는 그 반대 과정입니다.
  
C++에서는 `std::memcpy` 또는 구조체 패킹을 통해 간단한 직렬화를 구현할 수 있지만, 복잡한 데이터 구조에는 한계가 있습니다.  


#### 구조체 패킹
- `#pragma pack(1)`으로 메모리 정렬 제어
- 구조체를 통째로 바이너리 파일에 저장/로드
- 배열 데이터 처리와 메모리 직렬화 예제 포함
- 고정 크기 데이터에 최적화
  
```    
#include <iostream>
#include <fstream>
#include <cstring>

// 구조체 패킹을 사용한 직렬화
#pragma pack(push, 1)  // 1바이트 정렬로 패딩 제거
struct PackedData {
    int id;
    float value;
    char name[16];
    bool active;
    double score;
};
#pragma pack(pop)

// 복합 데이터 구조체
#pragma pack(push, 1)
struct GamePlayer {
    int playerId;
    char playerName[32];
    float health;
    float mana;
    int level;
    bool isOnline;
};
#pragma pack(pop)

int main() {
    std::cout << "=== 구조체 패킹 직렬화 예제 ===" << std::endl;
    
    // 단일 구조체 직렬화
    PackedData data1 = {123, 45.67f, "TestData", true, 98.5};
    
    std::cout << "원본 데이터:" << std::endl;
    std::cout << "ID: " << data1.id << std::endl;
    std::cout << "Value: " << data1.value << std::endl;
    std::cout << "Name: " << data1.name << std::endl;
    std::cout << "Active: " << data1.active << std::endl;
    std::cout << "Score: " << data1.score << std::endl;
    
        
    // 메모리 직렬화 (바이트 배열로)
    std::cout << "\n=== 메모리 직렬화 ===" << std::endl;
    
    char buffer[sizeof(PackedData)];
    std::memcpy(buffer, &data1, sizeof(PackedData));
    
    PackedData data3;
    std::memcpy(&data3, buffer, sizeof(PackedData));
    
    std::cout << "메모리에서 복사된 데이터: " << data3.name << std::endl;
    
    // 구조체 크기 정보
    std::cout << "\n=== 구조체 크기 정보 ===" << std::endl;
    std::cout << "PackedData 크기: " << sizeof(PackedData) << " bytes" << std::endl;
    
    return 0;
}
```  


#### std::memcpy: 직렬화/역직렬화 클래스
- 유연한 직렬화/역직렬화 클래스 구현
- 가변 길이 문자열과 배열 처리 가능
- 복합 데이터 구조체를 위한 직렬화 메서드 제공
- 메모리 간 직접 복사도 지원
  
```
#include <iostream>
#include <cstring>
#include <vector>
#include <string>

// memcpy를 사용한 직렬화 클래스
class BinarySerializer {
private:
    std::vector<char> buffer;
    size_t writeOffset;

public:
    BinarySerializer() : writeOffset(0) {}
    
    // 기본 데이터 타입 쓰기
    template<typename T>
    void write(const T& data) {
        size_t size = sizeof(T);
        buffer.resize(buffer.size() + size);
        std::memcpy(buffer.data() + writeOffset, &data, size);
        writeOffset += size;
    }
    
    // 문자열 쓰기 (길이 + 데이터)
    void writeString(const std::string& str) {
        size_t len = str.length();
        write(len);  // 먼저 길이 저장
        buffer.resize(buffer.size() + len);
        std::memcpy(buffer.data() + writeOffset, str.c_str(), len);
        writeOffset += len;
    }
    
    // 배열 쓰기
    template<typename T>
    void writeArray(const T* arr, size_t count) {
        write(count);  // 배열 크기 저장
        size_t totalSize = sizeof(T) * count;
        buffer.resize(buffer.size() + totalSize);
        std::memcpy(buffer.data() + writeOffset, arr, totalSize);
        writeOffset += totalSize;
    }
    
    // 버퍼 반환
    const std::vector<char>& getBuffer() const { return buffer; }
    size_t getSize() const { return buffer.size(); }
    
    // 버퍼 초기화
    void clear() {
        buffer.clear();
        writeOffset = 0;
    }
};

// 역직렬화 클래스
class BinaryDeserializer {
private:
    std::vector<char> buffer;
    size_t readOffset;

public:
    BinaryDeserializer() : readOffset(0) {}
    
    // 버퍼에서 직접 로드
    void loadFromBuffer(const std::vector<char>& data) {
        buffer = data;
        readOffset = 0;
    }
    
    // 기본 데이터 타입 읽기
    template<typename T>
    T read() {
        T data;
        std::memcpy(&data, buffer.data() + readOffset, sizeof(T));
        readOffset += sizeof(T);
        return data;
    }
    
    // 문자열 읽기
    std::string readString() {
        size_t len = read<size_t>();
        std::string str(buffer.data() + readOffset, len);
        readOffset += len;
        return str;
    }
    
    // 배열 읽기
    template<typename T>
    std::vector<T> readArray() {
        size_t count = read<size_t>();
        std::vector<T> arr(count);
        size_t totalSize = sizeof(T) * count;
        std::memcpy(arr.data(), buffer.data() + readOffset, totalSize);
        readOffset += totalSize;
        return arr;
    }
    
    // 읽기 위치 초기화
    void reset() { readOffset = 0; }
    bool hasMore() const { return readOffset < buffer.size(); }
    size_t remaining() const { return buffer.size() - readOffset; }
};

// 복합 데이터 구조체
struct PlayerData {
    int id;
    std::string name;
    float health;
    std::vector<int> inventory;
    bool isActive;
    
    // 직렬화 메서드
    void serialize(BinarySerializer& serializer) const {
        serializer.write(id);
        serializer.writeString(name);
        serializer.write(health);
        serializer.writeArray(inventory.data(), inventory.size());
        serializer.write(isActive);
    }
    
    // 역직렬화 메서드
    void deserialize(BinaryDeserializer& deserializer) {
        id = deserializer.read<int>();
        name = deserializer.readString();
        health = deserializer.read<float>();
        inventory = deserializer.readArray<int>();
        isActive = deserializer.read<bool>();
    }
};

int main() 
{
    std::cout << "=== memcpy 기반 직렬화 예제 ===" << std::endl;
    
    // 기본 데이터 직렬화
    BinarySerializer serializer;
    
    int number = 42;
    float pi = 3.14159f;
    std::string message = "Hello, Serialization!";
    bool flag = true;
    double precision = 123.456789;
    
    serializer.write(number);
    serializer.write(pi);
    serializer.writeString(message);
    serializer.write(flag);
    serializer.write(precision);
    
    std::cout << "직렬화 완료, 버퍼 크기: " << serializer.getSize() << " bytes" << std::endl;
    
    // 메모리에서 역직렬화
    BinaryDeserializer deserializer;
    deserializer.loadFromBuffer(serializer.getBuffer());
    
    int readNumber = deserializer.read<int>();
    float readPi = deserializer.read<float>();
    std::string readMessage = deserializer.readString();
    bool readFlag = deserializer.read<bool>();
    double readPrecision = deserializer.read<double>();
    
    std::cout << "\n역직렬화 결과:" << std::endl;
    std::cout << "Number: " << readNumber << std::endl;
    std::cout << "Pi: " << readPi << std::endl;
    std::cout << "Message: " << readMessage << std::endl;
    std::cout << "Flag: " << (readFlag ? "true" : "false") << std::endl;
    std::cout << "Precision: " << readPrecision << std::endl;
    
    std::cout << "\n=== 복합 데이터 직렬화 ===" << std::endl;
    
    // 플레이어 데이터 생성
    PlayerData player1;
    player1.id = 1001;
    player1.name = "SuperPlayer";
    player1.health = 85.5f;
    player1.inventory = {1, 5, 3, 12, 8};  // 아이템 ID들
    player1.isActive = true;
    
    // 복합 데이터 직렬화
    BinarySerializer playerSerializer;
    player1.serialize(playerSerializer);
    
    std::cout << "플레이어 데이터 직렬화 완료" << std::endl;
    std::cout << "원본 플레이어: " << player1.name 
              << " (ID: " << player1.id 
              << ", HP: " << player1.health 
              << ", Items: " << player1.inventory.size() << "개)" << std::endl;
    
    // 복합 데이터 역직렬화
    PlayerData player2;
    BinaryDeserializer playerDeserializer;
    playerDeserializer.loadFromBuffer(playerSerializer.getBuffer());
    player2.deserialize(playerDeserializer);
    
    std::cout << "\n로드된 플레이어: " << player2.name 
              << " (ID: " << player2.id 
              << ", HP: " << player2.health 
              << ", Active: " << (player2.isActive ? "Yes" : "No") << ")" << std::endl;
    
    std::cout << "인벤토리: ";
    for (size_t i = 0; i < player2.inventory.size(); i++) {
        std::cout << player2.inventory[i];
        if (i < player2.inventory.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    
    std::cout << "\n=== 메모리 간 직렬화 ===" << std::endl;
    
    // 메모리에서 메모리로 직접 복사
    BinaryDeserializer memoryDeserializer;
    memoryDeserializer.loadFromBuffer(playerSerializer.getBuffer());
    
    PlayerData player3;
    player3.deserialize(memoryDeserializer);
    
    std::cout << "메모리 복사된 플레이어: " << player3.name 
              << " (ID: " << player3.id << ")" << std::endl;
    
    return 0;
}
```    
  
![](./images/030.png)  
![](./images/031.png)   
![](./images/032.png)   
![](./images/033.png)     

      
#### 오픈 소스 라이브러리
[Google Protocol Buffer (Protobuf)](https://velog.io/@scarleter99/C%EA%B2%8C%EC%9E%84%EC%84%9C%EB%B2%84-5-3.-%ED%8C%A8%ED%82%B7-%EC%A7%81%EB%A0%AC%ED%99%94-Google-Protocol-Buffer-Protobuf )    
[Protocol Buffers Documentation](https://protobuf.dev/ )  
  


## 02 다양한 데이터 전송 방식

### 1. 송수신 모드
**블로킹 I/O vs 논블로킹 I/O**
- 블로킹 I/O: 작업이 완료될 때까지 스레드가 대기
- 논블로킹 I/O: 즉시 반환되며, 작업 완료 여부를 반환값으로 확인
  
![블로킹 I/O vs 논블로킹 I/O](./images/007.png) 
  

**동기 I/O vs 비동기 I/O**
- 동기 I/O: 작업 완료 시점에 결과를 받음
- 비동기 I/O: 작업을 요청만 하고 완료 시 콜백 또는 이벤트로 통지받음  
  
![동기 I/O vs 비동기 I/O](./images/008.png)   
  
  
### 2. 데이터 패킷 구조
게임 서버에서 일반적으로 사용되는 패킷 구조:  

1. **고정 길이 패킷**: 항상 같은 크기의 데이터
  
2. **가변 길이 패킷**: 데이터 크기가 가변적
  
3. **헤더-페이로드 구조**: 고정 길이 헤더 + 가변 길이 페이로드  
    ```cpp
    // 패킷 헤더 예시
    #pragma pack(push, 1)  // 구조체 패딩 제거
    struct PacketHeader {
        uint16_t size;      // 전체 패킷 크기
        uint16_t type;      // 패킷 유형
    };
    #pragma pack(pop)
    ```   
  
![패킷 구조](./images/009.png)   


### 3. 데이터 경계 처리
TCP는 스트림 지향 프로토콜이므로 메시지 경계를 보존하지 않습니다.    
![TCP 스트림과 메시지 경계](./images/034.png) 
  
경계 처리 방법으로는:
1. **고정 길이 패킷**: 항상 같은 크기로 송수신
2. **길이 필드 추가**: 데이터 앞에 길이 정보 추가
3. **구분자 사용**: 특별한 시퀀스로 메시지 끝 표시
4. **헤더-페이로드 구조**: 헤더에 페이로드 크기 정보 포함
  
![](./images/035.png)  
![](./images/036.png)   
![](./images/037.png)   
![](./images/038.png)      


## 실습: 고정 길이 데이터 전송 연습
고정 길이 데이터는 구현이 간단하지만 유연성이 떨어집니다. 다음은 플레이어 위치 정보를 고정 길이로 전송하는 예제입니다.    
  
<details>  
<summary>FixedLengthServer Code</summary>     
  
```cpp
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <format>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

// 고정 길이 데이터 구조
#pragma pack(push, 1)
struct PlayerPosition {
    uint32_t playerId;
    float x;
    float y;
    float z;
    float rotation;
};
#pragma pack(pop)

// 서버 클래스
class FixedLengthServer {
private:
    SOCKET listenSocket;
    std::vector<std::thread> clientThreads;
    bool running;

public:
    FixedLengthServer() : listenSocket(INVALID_SOCKET), running(false) {}
    
    ~FixedLengthServer() {
        Stop();
    }
    
    bool Start(int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("바인딩 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << std::format("리슨 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        running = true;
        std::cout << std::format("고정 길이 데이터 서버가 포트 {}에서 시작됨\n", port);
        
        // 클라이언트 연결 수락 스레드 시작
        std::thread acceptThread(&FixedLengthServer::AcceptClients, this);
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
        std::cout << "서버가 중지됨\n";
    }
    
private:
    void AcceptClients() {
        while (running) {
            sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << std::format("클라이언트 연결 수락 실패: {}\n", WSAGetLastError());
                }
                continue;
            }
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::cout << std::format("새 클라이언트 연결: {}:{}\n", clientIP, ntohs(clientAddr.sin_port));
            
            clientThreads.emplace_back(&FixedLengthServer::HandleClient, this, clientSocket);
        }
    }
    
    void HandleClient(SOCKET clientSocket) {
        PlayerPosition position;
        
        while (running) {
            // 고정 길이 데이터 수신 (한 번에 모두 받지 못할 수도 있음)
            int totalBytesReceived = 0;
            int bytesToReceive = sizeof(PlayerPosition);
            char* buffer = reinterpret_cast<char*>(&position);
            
            // 데이터를 완전히 수신할 때까지 반복
            while (totalBytesReceived < bytesToReceive) {
                int bytesReceived = recv(clientSocket, 
                                         buffer + totalBytesReceived, 
                                         bytesToReceive - totalBytesReceived, 
                                         0);
                
                if (bytesReceived <= 0) {
                    if (bytesReceived == 0) {
                        std::cout << "클라이언트 연결 종료\n";
                    } else {
                        std::cerr << std::format("recv 실패: {}\n", WSAGetLastError());
                    }
                    closesocket(clientSocket);
                    return;
                }
                
                totalBytesReceived += bytesReceived;
            }
            
            // 수신된 위치 정보 출력
            std::cout << std::format("플레이어 ID: {}, 위치: ({:.2f}, {:.2f}, {:.2f}), 회전: {:.2f}\n",
                                    position.playerId, position.x, position.y, position.z, position.rotation);
            
            // 응답으로 같은 데이터 전송
            int totalBytesSent = 0;
            int bytesToSend = sizeof(PlayerPosition);
            const char* sendBuffer = reinterpret_cast<const char*>(&position);
            
            while (totalBytesSent < bytesToSend) {
                int bytesSent = send(clientSocket, 
                                    sendBuffer + totalBytesSent, 
                                    bytesToSend - totalBytesSent, 
                                    0);
                
                if (bytesSent == SOCKET_ERROR) {
                    std::cerr << std::format("send 실패: {}\n", WSAGetLastError());
                    closesocket(clientSocket);
                    return;
                }
                
                totalBytesSent += bytesSent;
            }
        }
        
        closesocket(clientSocket);
    }
};

// 클라이언트 클래스
class FixedLengthClient {
private:
    SOCKET clientSocket;
    bool connected;
    
public:
    FixedLengthClient() : clientSocket(INVALID_SOCKET), connected(false) {}
    
    ~FixedLengthClient() {
        Disconnect();
    }
    
    bool Connect(const std::string& serverIP, int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
        
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("서버 연결 실패: {}\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }
        
        connected = true;
        std::cout << std::format("서버 {}:{}에 연결됨\n", serverIP, port);
        return true;
    }
    
    void Disconnect() {
        if (connected && clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
            connected = false;
            WSACleanup();
            std::cout << "서버와 연결 종료\n";
        }
    }
    
    bool SendPlayerPosition(uint32_t playerId, float x, float y, float z, float rotation) {
        if (!connected || clientSocket == INVALID_SOCKET) {
            std::cerr << "서버에 연결되지 않음\n";
            return false;
        }
        
        PlayerPosition position;
        position.playerId = playerId;
        position.x = x;
        position.y = y;
        position.z = z;
        position.rotation = rotation;
        
        // 고정 길이 데이터 전송
        int totalBytesSent = 0;
        int bytesToSend = sizeof(PlayerPosition);
        const char* buffer = reinterpret_cast<const char*>(&position);
        
        while (totalBytesSent < bytesToSend) {
            int bytesSent = send(clientSocket, 
                                buffer + totalBytesSent, 
                                bytesToSend - totalBytesSent, 
                                0);
            
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("send 실패: {}\n", WSAGetLastError());
                return false;
            }
            
            totalBytesSent += bytesSent;
        }
        
        // 서버로부터 응답 수신
        PlayerPosition response;
        int totalBytesReceived = 0;
        int bytesToReceive = sizeof(PlayerPosition);
        char* recvBuffer = reinterpret_cast<char*>(&response);
        
        while (totalBytesReceived < bytesToReceive) {
            int bytesReceived = recv(clientSocket, 
                                    recvBuffer + totalBytesReceived, 
                                    bytesToReceive - totalBytesReceived, 
                                    0);
            
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << "서버 연결 종료\n";
                } else {
                    std::cerr << std::format("recv 실패: {}\n", WSAGetLastError());
                }
                return false;
            }
            
            totalBytesReceived += bytesReceived;
        }
        
        std::cout << "서버로부터 응답 수신: ";
        std::cout << std::format("플레이어 ID: {}, 위치: ({:.2f}, {:.2f}, {:.2f}), 회전: {:.2f}\n",
                                response.playerId, response.x, response.y, response.z, response.rotation);
        
        return true;
    }
};

// 테스트용 메인 함수 (서버)
int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "1: 서버 모드, 2: 클라이언트 모드 - 선택: ";
    int mode;
    std::cin >> mode;
    
    if (mode == 1) {
        FixedLengthServer server;
        if (server.Start()) {
            std::cout << "서버가 시작되었습니다. 종료하려면 아무 키나 누르세요.\n";
            std::cin.ignore();
            std::cin.get();
            server.Stop();
        }
    } else if (mode == 2) {
        FixedLengthClient client;
        std::string serverIP;
        
        std::cout << "서버 IP를 입력하세요: ";
        std::cin.ignore();
        std::getline(std::cin, serverIP);
        
        if (client.Connect(serverIP)) {
            for (int i = 0; i < 5; i++) {
                // 랜덤 위치 데이터 생성 및 전송
                float x = static_cast<float>(rand() % 100);
                float y = static_cast<float>(rand() % 100);
                float z = static_cast<float>(rand() % 100);
                float rotation = static_cast<float>(rand() % 360);
                
                std::cout << std::format("위치 데이터 전송: ({:.2f}, {:.2f}, {:.2f}), 회전: {:.2f}\n", 
                                        x, y, z, rotation);
                
                client.SendPlayerPosition(1, x, y, z, rotation);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            
            client.Disconnect();
        }
    }
    
    return 0;
}
```  
</details>   
  

### C++ 소켓 통신 프로그램 분석
이 코드는 Windows 환경에서 동작하는 고정 길이 데이터 구조를 사용한 TCP 소켓 통신 프로그램입니다. 서버와 클라이언트 간에 플레이어의 위치 정보를 주고받는 기능을 구현하고 있습니다.

#### 주요 구성 요소
1. **PlayerPosition 구조체**: 플레이어 위치 데이터를 담는 고정 길이 구조체
2. **FixedLengthServer 클래스**: TCP 서버 기능 담당
3. **FixedLengthClient 클래스**: TCP 클라이언트 기능 담당
4. **메인 함수**: 서버/클라이언트 모드 선택 실행
  

#### 동작 원리 시각화  
![동작 원리](./images/011.png)  
  
  
#### 주요 특징 설명

##### 1. 고정 길이 데이터 구조
```cpp
#pragma pack(push, 1)
struct PlayerPosition {
    uint32_t playerId;
    float x;
    float y;
    float z;
    float rotation;
};
#pragma pack(pop)
```
- `#pragma pack(1)`을 사용하여 메모리 정렬 없이 1바이트 단위로 패킹
- 네트워크를 통해 바이너리 형태로 직접 전송 가능한 고정 길이 구조체
- 총 20바이트: playerId(4) + x(4) + y(4) + z(4) + rotation(4)
  
##### 2. 완전한 데이터 송수신 보장
```cpp
// 데이터를 완전히 수신할 때까지 반복
while (totalBytesReceived < bytesToReceive) {
    int bytesReceived = recv(clientSocket, 
                            buffer + totalBytesReceived, 
                            bytesToReceive - totalBytesReceived, 
                            0);
    // 오류 처리...
    totalBytesReceived += bytesReceived;
}
```
- TCP는 스트림 기반 프로토콜로 한 번에 모든 데이터가 오지 않을 수 있음
- 위 코드는 모든 데이터를 완전히 수신할 때까지 반복하여 데이터 무결성 보장

##### 3. 멀티스레딩 처리
- 서버는 클라이언트 연결 수락을 위한 별도 스레드 사용
- 각 클라이언트 연결마다 별도 스레드를 생성하여 동시에 여러 클라이언트 처리
- `clientThreads` 벡터로 모든 클라이언트 스레드 관리

##### 4. 서버-클라이언트 데이터 흐름
1. 클라이언트가 서버에 연결
2. 클라이언트가 PlayerPosition 데이터 생성 및 전송
3. 서버는 데이터 수신 후 내용 출력
4. 서버는 동일한 데이터를 클라이언트에게 응답으로 전송
5. 클라이언트는 응답 수신 후 내용 출력

#### 결론
이 코드는 게임 서버와 같은 환경에서 플레이어 위치 정보를 효율적으로 교환하기 위한 기본 구조를 제공합니다. 고정 길이 데이터 구조와 완전한 데이터 송수신 보장, 멀티스레딩을 통한 다중 클라이언트 처리가 주요 특징입니다.
  

## 실습: 가변 길이 데이터 전송 연습
가변 길이 데이터 전송은 길이 정보를 함께 전송하거나 특별한 종료 마커를 사용해야 합니다. 다음은 채팅 메시지를 가변 길이로 전송하는 예제입니다.  
패킷을 두 부분으로 나누어서 보냅니다. 
**고정 길이인 메시지의 Header를 먼저 보내고, 가변 길이가 되는 Body 데이터를 보냅니다**.
  
<details>
<summary>VariableLengthServer 코드</summary> 
  
```cpp
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <format>
#include <vector>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")

// 채팅 메시지 헤더
#pragma pack(push, 1)
struct ChatMessageHeader {
    uint32_t messageLength;  // 메시지 내용 길이 (헤더 제외)
    uint32_t userId;         // 발신자 ID
};
#pragma pack(pop)

// 서버 클래스
class VariableLengthServer {
private:
    SOCKET listenSocket;
    std::vector<std::thread> clientThreads;
    bool running;

public:
    VariableLengthServer() : listenSocket(INVALID_SOCKET), running(false) {}
    
    ~VariableLengthServer() {
        Stop();
    }
    
    bool Start(int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("바인딩 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << std::format("리슨 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        running = true;
        std::cout << std::format("가변 길이 데이터 서버가 포트 {}에서 시작됨\n", port);
        
        // 클라이언트 연결 수락 스레드 시작
        std::thread acceptThread(&VariableLengthServer::AcceptClients, this);
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
        std::cout << "서버가 중지됨\n";
    }
    
private:
    void AcceptClients() {
        while (running) {
            sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << std::format("클라이언트 연결 수락 실패: {}\n", WSAGetLastError());
                }
                continue;
            }
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::cout << std::format("새 클라이언트 연결: {}:{}\n", clientIP, ntohs(clientAddr.sin_port));
            
            clientThreads.emplace_back(&VariableLengthServer::HandleClient, this, clientSocket);
        }
    }
    
    void HandleClient(SOCKET clientSocket) {
        // 헤더 버퍼
        ChatMessageHeader header;
        
        while (running) {
            // 1. 헤더 수신 (고정 길이)
            int totalBytesReceived = 0;
            int bytesToReceive = sizeof(ChatMessageHeader);
            char* headerBuffer = reinterpret_cast<char*>(&header);
            
            // 헤더를 완전히 수신할 때까지 반복
            while (totalBytesReceived < bytesToReceive) {
                int bytesReceived = recv(clientSocket, 
                                        headerBuffer + totalBytesReceived, 
                                        bytesToReceive - totalBytesReceived, 
                                        0);
                
                if (bytesReceived <= 0) {
                    if (bytesReceived == 0) {
                        std::cout << "클라이언트 연결 종료\n";
                    } else {
                        std::cerr << std::format("헤더 수신 실패: {}\n", WSAGetLastError());
                    }
                    closesocket(clientSocket);
                    return;
                }
                
                totalBytesReceived += bytesReceived;
            }
            
            // 메시지 길이 및 사용자 ID 확인
            uint32_t messageLength = header.messageLength;
            uint32_t userId = header.userId;
            
            // 길이 검증 (최대 크기 제한)
            const uint32_t MAX_MESSAGE_LENGTH = 8192;
            if (messageLength == 0 || messageLength > MAX_MESSAGE_LENGTH) {
                std::cerr << std::format("잘못된 메시지 길이: {}\n", messageLength);
                closesocket(clientSocket);
                return;
            }
            
            // 2. 메시지 내용 수신 (가변 길이)
            std::vector<char> messageBuffer(messageLength + 1); // +1 for null terminator
            totalBytesReceived = 0;
            
            // 메시지를 완전히 수신할 때까지 반복
            while (totalBytesReceived < messageLength) {
                int bytesReceived = recv(clientSocket, 
                                        messageBuffer.data() + totalBytesReceived, 
                                        messageLength - totalBytesReceived, 
                                        0);
                
                if (bytesReceived <= 0) {
                    if (bytesReceived == 0) {
                        std::cout << "클라이언트 연결 종료\n";
                    } else {
                        std::cerr << std::format("메시지 내용 수신 실패: {}\n", WSAGetLastError());
                    }
                    closesocket(clientSocket);
                    return;
                }
                
                totalBytesReceived += bytesReceived;
            }
            
            // 문자열 종료 널 문자 추가
            messageBuffer[messageLength] = '\0';
            
            // 수신된 메시지 출력
            std::cout << std::format("사용자 ID {}: {}\n", userId, messageBuffer.data());
            
            // 3. 응답 메시지 생성 (에코)
            std::string responseMessage = std::format("서버 응답: {}", messageBuffer.data());
            
            // 4. 응답 헤더 준비
            ChatMessageHeader responseHeader;
            responseHeader.messageLength = static_cast<uint32_t>(responseMessage.length());
            responseHeader.userId = 0; // 서버 ID는 0
            
            // 5. 응답 헤더 전송
            int totalBytesSent = 0;
            int bytesToSend = sizeof(ChatMessageHeader);
            const char* sendHeaderBuffer = reinterpret_cast<const char*>(&responseHeader);
            
            while (totalBytesSent < bytesToSend) {
                int bytesSent = send(clientSocket, 
                                    sendHeaderBuffer + totalBytesSent, 
                                    bytesToSend - totalBytesSent, 
                                    0);
                
                if (bytesSent == SOCKET_ERROR) {
                    std::cerr << std::format("응답 헤더 전송 실패: {}\n", WSAGetLastError());
                    closesocket(clientSocket);
                    return;
                }
                
                totalBytesSent += bytesSent;
            }
            
            // 6. 응답 메시지 내용 전송
            totalBytesSent = 0;
            bytesToSend = static_cast<int>(responseMessage.length());
            
            while (totalBytesSent < bytesToSend) {
                int bytesSent = send(clientSocket, 
                                    responseMessage.c_str() + totalBytesSent, 
                                    bytesToSend - totalBytesSent, 
                                    0);
                
                if (bytesSent == SOCKET_ERROR) {
                    std::cerr << std::format("응답 메시지 전송 실패: {}\n", WSAGetLastError());
                    closesocket(clientSocket);
                    return;
                }
                
                totalBytesSent += bytesSent;
            }
        }
        
        closesocket(clientSocket);
    }
};

// 클라이언트 클래스
class VariableLengthClient {
private:
    SOCKET clientSocket;
    bool connected;
    
public:
    VariableLengthClient() : clientSocket(INVALID_SOCKET), connected(false) {}
    
    ~VariableLengthClient() {
        Disconnect();
    }
    
    bool Connect(const std::string& serverIP, int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
        
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("서버 연결 실패: {}\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }
        
        connected = true;
        std::cout << std::format("서버 {}:{}에 연결됨\n", serverIP, port);
        return true;
    }
    
    void Disconnect() {
        if (connected && clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
            connected = false;
            WSACleanup();
            std::cout << "서버와 연결 종료\n";
        }
    }
    
    bool SendChatMessage(uint32_t userId, const std::string& message) {
        if (!connected || clientSocket == INVALID_SOCKET) {
            std::cerr << "서버에 연결되지 않음\n";
            return false;
        }
        
        // 1. 헤더 준비
        ChatMessageHeader header;
        header.messageLength = static_cast<uint32_t>(message.length());
        header.userId = userId;
        
        // 2. 헤더 전송
        int totalBytesSent = 0;
        int bytesToSend = sizeof(ChatMessageHeader);
        const char* headerBuffer = reinterpret_cast<const char*>(&header);
        
        while (totalBytesSent < bytesToSend) {
            int bytesSent = send(clientSocket, 
                                headerBuffer + totalBytesSent, 
                                bytesToSend - totalBytesSent, 
                                0);
            
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("헤더 전송 실패: {}\n", WSAGetLastError());
                return false;
            }
            
            totalBytesSent += bytesSent;
        }
        
        // 3. 메시지 내용 전송
        totalBytesSent = 0;
        bytesToSend = static_cast<int>(message.length());
        
        while (totalBytesSent < bytesToSend) {
            int bytesSent = send(clientSocket, 
                                message.c_str() + totalBytesSent, 
                                bytesToSend - totalBytesSent, 
                                0);
            
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("메시지 내용 전송 실패: {}\n", WSAGetLastError());
                return false;
            }
            
            totalBytesSent += bytesSent;
        }
        
        // 4. 응답 헤더 수신
        ChatMessageHeader responseHeader;
        int totalBytesReceived = 0;
        int bytesToReceive = sizeof(ChatMessageHeader);
        char* responseHeaderBuffer = reinterpret_cast<char*>(&responseHeader);
        
        while (totalBytesReceived < bytesToReceive) {
            int bytesReceived = recv(clientSocket, 
                                    responseHeaderBuffer + totalBytesReceived, 
                                    bytesToReceive - totalBytesReceived, 
                                    0);
            
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << "서버 연결 종료\n";
                } else {
                    std::cerr << std::format("응답 헤더 수신 실패: {}\n", WSAGetLastError());
                }
                return false;
            }
            
            totalBytesReceived += bytesReceived;
        }
        
        // 5. 응답 메시지 내용 수신
        uint32_t responseMessageLength = responseHeader.messageLength;
        
        // 길이 검증
        const uint32_t MAX_MESSAGE_LENGTH = 8192;
        if (responseMessageLength == 0 || responseMessageLength > MAX_MESSAGE_LENGTH) {
            std::cerr << std::format("잘못된 응답 메시지 길이: {}\n", responseMessageLength);
            return false;
        }
        
        std::vector<char> responseBuffer(responseMessageLength + 1); // +1 for null terminator
        totalBytesReceived = 0;
        
        while (totalBytesReceived < responseMessageLength) {
            int bytesReceived = recv(clientSocket, 
                                    responseBuffer.data() + totalBytesReceived, 
                                    responseMessageLength - totalBytesReceived, 
                                    0);
            
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << "서버 연결 종료\n";
                } else {
                    std::cerr << std::format("응답 메시지 수신 실패: {}\n", WSAGetLastError());
                }
                return false;
            }
            
            totalBytesReceived += bytesReceived;
        }
        
        // 문자열 종료 널 문자 추가
        responseBuffer[responseMessageLength] = '\0';
        
        // 응답 메시지 출력
        std::cout << "서버로부터 응답: " << responseBuffer.data() << std::endl;
        
        return true;
    }
};

// 테스트용 메인 함수
int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "1: 서버 모드, 2: 클라이언트 모드 - 선택: ";
    int mode;
    std::cin >> mode;
    
    if (mode == 1) {
        VariableLengthServer server;
        if (server.Start()) {
            std::cout << "서버가 시작되었습니다. 종료하려면 아무 키나 누르세요.\n";
            std::cin.ignore();
            std::cin.get();
            server.Stop();
        }
    } else if (mode == 2) {
        VariableLengthClient client;
        std::string serverIP;
        
        std::cout << "서버 IP를 입력하세요: ";
        std::cin.ignore();
        std::getline(std::cin, serverIP);
        
        if (client.Connect(serverIP)) {
            uint32_t userId = 1001;
            
            while (true) {
                std::string message;
                std::cout << "전송할 메시지 (종료: exit): ";
                std::getline(std::cin, message);
                
                if (message == "exit") {
                    break;
                }
                
                client.SendChatMessage(userId, message);
            }
            
            client.Disconnect();
        }
    }
    
    return 0;
}
```   
</details>  
    
이 코드는 **가변 길이 데이터를 처리하는 TCP 클라이언트-서버 시스템**이다. 채팅 메시지 형태로 헤더 + 데이터 구조를 사용하여 안전하게 가변 길이 데이터를 주고받는다.

### 핵심 구조

#### 1. 메시지 프로토콜 구조

```
[헤더 8바이트] + [메시지 내용 (가변길이)]
```    
![채팅 메시지 프로토콜 구조](./images/012.png)    
  

#### 2. 핵심 클래스들
**VariableLengthServer**: 다중 클라이언트를 처리하는 서버
- 각 클라이언트마다 별도 스레드로 처리
- 가변 길이 메시지를 안전하게 수신/송신

**VariableLengthClient**: 서버와 통신하는 클라이언트
- 메시지 전송 후 응답 대기
- 에코 서버 형태로 서버 응답을 출력

### 주요 동작 과정
![클라이언트-서버 통신 과정](./images/013.png)    

### 중요한 기술적 특징들

#### 1. **완전한 데이터 수신/송신 보장**
```cpp
// 헤더를 완전히 수신할 때까지 반복
while (totalBytesReceived < bytesToReceive) {
    int bytesReceived = recv(clientSocket, 
                            headerBuffer + totalBytesReceived, 
                            bytesToReceive - totalBytesReceived, 
                            0);
    // ...
    totalBytesReceived += bytesReceived;
}
```

TCP는 스트림 방식이므로 한 번의 recv/send로 모든 데이터가 전송되지 않을 수 있다. 따라서 루프를 통해 완전한 전송을 보장한다.  

#### 2. **메모리 정렬과 헤더 구조**
```cpp
#pragma pack(push, 1)
struct ChatMessageHeader {
    uint32_t messageLength;  // 4바이트
    uint32_t userId;         // 4바이트
};
#pragma pack(pop)
```

`#pragma pack(1)`으로 구조체 패딩을 제거하여 정확히 8바이트 크기를 보장한다.

#### 3. **에러 처리 및 검증**
```cpp
const uint32_t MAX_MESSAGE_LENGTH = 8192;
if (messageLength == 0 || messageLength > MAX_MESSAGE_LENGTH) {
    std::cerr << std::format("잘못된 메시지 길이: {}\n", messageLength);
    closesocket(clientSocket);
    return;
}
```

메시지 길이 검증을 통해 비정상적인 데이터나 공격을 방지한다.

#### 4. **멀티스레딩 처리**
```cpp
std::thread acceptThread(&VariableLengthServer::AcceptClients, this);
acceptThread.detach();

// 클라이언트별 스레드 생성
clientThreads.emplace_back(&VariableLengthServer::HandleClient, this, clientSocket);
```

서버는 여러 클라이언트를 동시에 처리할 수 있도록 각 클라이언트마다 별도 스레드를 생성한다.

### 전체 동작 플로우
![전체 시스템 아키텍처](./images/014.png) 
  
### 실행 방법
1. **서버 실행**: 프로그램 실행 후 `1` 선택
2. **클라이언트 실행**: 다른 터미널에서 프로그램 실행 후 `2` 선택
3. **통신**: 클라이언트에서 메시지 입력하면 서버가 에코 응답

이 코드는 네트워크 프로그래밍에서 **가변 길이 데이터를 안전하게 처리하는 표준적인 방법**을 잘 보여준다. 특히 TCP의 스트림 특성을 고려한 완전한 송수신 보장과 헤더 기반 프로토콜 설계가 핵심이다.    
  
  

## 실습: 고정 길이 + 가변 길이 데이터 전송 연습
온라인 게임에서는 고정 길이 헤더와 가변 길이 페이로드 구조가 일반적입니다. 아이템 거래 시스템을 예로 구현해 보겠습니다.  
아래의 코드는 Server와 Client 코드 같이 있어서 실행할 때 어느쪽으로 실행할지 선택할 수 있다.  

코드를 보기 편하고, 동작을 쉽게 하기 위해서 두 개의 코드로 나누었고 아래 디렉토리에 있다.   
`codes/tcp_MixedLengthServer` , `codes/tcp_MixedLengthClient`      


<details>
<summary>MixedLength Server-Client 코드</summary>  
  
```cpp
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <format>
#include <vector>
#include <thread>
#include <string>
#include <map>

#pragma comment(lib, "ws2_32.lib")

// 패킷 유형 정의
enum PacketType {
    PT_TRADE_REQUEST = 1,
    PT_TRADE_RESPONSE,
    PT_CHAT_MESSAGE,
    PT_PLAYER_MOVE
};

// 패킷 헤더 (고정 길이)
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t totalSize;     // 헤더 포함 전체 패킷 크기
    uint16_t packetType;    // 패킷 유형 (PacketType enum)
};

// 아이템 구조체 (고정 길이)
struct Item {
    uint32_t itemId;        // 아이템 고유 ID
    uint16_t quantity;      // 수량
    uint16_t category;      // 카테고리 (무기, 방어구 등)
};

// 거래 요청 패킷 (고정 길이 + 가변 길이)
struct TradeRequestPacket {
    uint32_t requesterId;   // 요청자 ID
    uint32_t targetId;      // 대상자 ID
    uint16_t itemCount;     // 아이템 수
    // 이후에 Item 배열과 추가 메시지(문자열)이 가변 길이로 옴
};
#pragma pack(pop)

// 서버 클래스
class MixedLengthServer {
private:
    SOCKET listenSocket;
    std::vector<std::thread> clientThreads;
    bool running;
    
    // 클라이언트 ID와 소켓 매핑
    std::map<uint32_t, SOCKET> clientSockets;

public:
    MixedLengthServer() : listenSocket(INVALID_SOCKET), running(false) {}
    
    ~MixedLengthServer() {
        Stop();
    }
    
    bool Start(int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("바인딩 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << std::format("리슨 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        running = true;
        std::cout << std::format("혼합 길이 데이터 서버가 포트 {}에서 시작됨\n", port);
        
        // 클라이언트 연결 수락 스레드 시작
        std::thread acceptThread(&MixedLengthServer::AcceptClients, this);
        acceptThread.detach();
        
        return true;
    }
    
    void Stop() {
        running = false;
        
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        
        // 모든 클라이언트 소켓 닫기
        for (const auto& [userId, socket] : clientSockets) {
            closesocket(socket);
        }
        clientSockets.clear();
        
        for (auto& thread : clientThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        clientThreads.clear();
        WSACleanup();
        std::cout << "서버가 중지됨\n";
    }
    
private:
    void AcceptClients() {
        while (running) {
            sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << std::format("클라이언트 연결 수락 실패: {}\n", WSAGetLastError());
                }
                continue;
            }
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::cout << std::format("새 클라이언트 연결: {}:{}\n", clientIP, ntohs(clientAddr.sin_port));
            
            // 임시 사용자 ID 생성 (실제 게임에서는 로그인 과정에서 처리)
            static uint32_t nextUserId = 1000;
            uint32_t userId = nextUserId++;
            
            // 클라이언트 소켓 저장
            clientSockets[userId] = clientSocket;
            
            // 클라이언트 처리 스레드 시작
            clientThreads.emplace_back(&MixedLengthServer::HandleClient, this, clientSocket, userId);
        }
    }
    
    void HandleClient(SOCKET clientSocket, uint32_t userId) {
        std::cout << std::format("클라이언트 ID {}가 연결됨\n", userId);
        
        // 패킷 처리 루프
        while (running) {
            // 1. 패킷 헤더 수신
            PacketHeader header;
            int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&header), sizeof(header), 0);
            
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << std::format("클라이언트 ID {}가 연결 종료\n", userId);
                } else {
                    std::cerr << std::format("헤더 수신 실패: {}\n", WSAGetLastError());
                }
                break;
            }
            
            if (bytesReceived != sizeof(header)) {
                std::cerr << "불완전한 헤더 수신됨\n";
                break;
            }
            
            // 2. 패킷 크기 검증
            uint16_t totalSize = header.totalSize;
            uint16_t dataSize = totalSize - sizeof(header);
            
            if (totalSize < sizeof(header) || dataSize > 8192) {
                std::cerr << std::format("잘못된 패킷 크기: {}\n", totalSize);
                break;
            }
            
            // 3. 패킷 데이터 수신
            std::vector<char> packetData(dataSize);
            int totalBytesReceived = 0;
            
            while (totalBytesReceived < dataSize) {
                bytesReceived = recv(clientSocket, 
                                    packetData.data() + totalBytesReceived, 
                                    dataSize - totalBytesReceived, 
                                    0);
                
                if (bytesReceived <= 0) {
                    if (bytesReceived == 0) {
                        std::cout << "클라이언트 연결 종료\n";
                    } else {
                        std::cerr << std::format("데이터 수신 실패: {}\n", WSAGetLastError());
                    }
                    closesocket(clientSocket);
                    clientSockets.erase(userId);
                    return;
                }
                
                totalBytesReceived += bytesReceived;
            }
            
            // 4. 패킷 유형에 따른 처리
            switch (header.packetType) {
                case PT_TRADE_REQUEST:
                    HandleTradeRequest(clientSocket, userId, packetData.data(), dataSize);
                    break;
                    
                case PT_CHAT_MESSAGE:
                    // 채팅 메시지 처리 (구현 생략)
                    std::cout << "채팅 메시지 패킷 수신됨\n";
                    break;
                    
                case PT_PLAYER_MOVE:
                    // 플레이어 이동 처리 (구현 생략)
                    std::cout << "플레이어 이동 패킷 수신됨\n";
                    break;
                    
                default:
                    std::cerr << std::format("알 수 없는 패킷 유형: {}\n", header.packetType);
                    break;
            }
        }
        
        // 연결 종료 처리
        closesocket(clientSocket);
        clientSockets.erase(userId);
    }
    
    void HandleTradeRequest(SOCKET clientSocket, uint32_t senderId, const char* data, uint16_t dataSize) {
        // 거래 요청 패킷 파싱
        if (dataSize < sizeof(TradeRequestPacket)) {
            std::cerr << "거래 요청 패킷이 너무 작음\n";
            return;
        }
        
        const TradeRequestPacket* tradeRequest = reinterpret_cast<const TradeRequestPacket*>(data);
        
        // 아이템 목록의 시작 위치
        const char* itemsData = data + sizeof(TradeRequestPacket);
        uint16_t itemsDataSize = sizeof(Item) * tradeRequest->itemCount;
        
        // 아이템 데이터 크기 검증
        if (sizeof(TradeRequestPacket) + itemsDataSize > dataSize) {
            std::cerr << "아이템 데이터가 패킷보다 큼\n";
            return;
        }
        
        // 아이템 목록 파싱
        std::vector<Item> items;
        for (uint16_t i = 0; i < tradeRequest->itemCount; i++) {
            const Item* item = reinterpret_cast<const Item*>(itemsData + i * sizeof(Item));
            items.push_back(*item);
        }
        
        // 추가 메시지 파싱 (있는 경우)
        std::string message;
        if (sizeof(TradeRequestPacket) + itemsDataSize < dataSize) {
            const char* messageData = itemsData + itemsDataSize;
            uint16_t messageSize = dataSize - sizeof(TradeRequestPacket) - itemsDataSize;
            message = std::string(messageData, messageSize);
        }
        
        // 거래 요청 정보 출력
        std::cout << std::format("거래 요청 수신: 요청자={}, 대상자={}, 아이템 수={}\n", 
                             tradeRequest->requesterId, tradeRequest->targetId, tradeRequest->itemCount);
        
        for (const auto& item : items) {
            std::cout << std::format("  아이템 ID: {}, 수량: {}, 카테고리: {}\n", 
                                 item.itemId, item.quantity, item.category);
        }
        
        if (!message.empty()) {
            std::cout << std::format("  메시지: {}\n", message);
        }
        
        // 대상 클라이언트에게 거래 요청 전달 (실제 구현 생략)
        uint32_t targetId = tradeRequest->targetId;
        if (clientSockets.find(targetId) != clientSockets.end()) {
            std::cout << std::format("대상 클라이언트 ID {}에게 거래 요청 전달\n", targetId);
            
            // 여기서 대상 클라이언트에게 패킷 전달 로직 구현 필요
        } else {
            std::cout << std::format("대상 클라이언트 ID {}가 연결되어 있지 않음\n", targetId);
            
            // 요청자에게 실패 응답 보내기
            SendTradeResponse(clientSocket, senderId, tradeRequest->requesterId, false, "대상 플레이어가 접속 중이 아닙니다.");
        }
    }
    
    void SendTradeResponse(SOCKET clientSocket, uint32_t senderId, uint32_t targetId, bool accepted, const std::string& message) {
        // 1. 응답 패킷 헤더 준비
        PacketHeader header;
        header.packetType = PT_TRADE_RESPONSE;
        
        // 2. 응답 데이터 준비
        struct TradeResponseData {
            uint32_t senderId;
            uint32_t targetId;
            uint8_t accepted;
        };
        
        TradeResponseData responseData;
        responseData.senderId = senderId;
        responseData.targetId = targetId;
        responseData.accepted = accepted ? 1 : 0;
        
        // 3. 전체 패킷 크기 계산
        uint16_t totalSize = sizeof(header) + sizeof(responseData) + static_cast<uint16_t>(message.length());
        header.totalSize = totalSize;
        
        // 4. 패킷 버퍼 생성 및 데이터 복사
        std::vector<char> packetBuffer(totalSize);
        char* bufferPtr = packetBuffer.data();
        
        // 헤더 복사
        memcpy(bufferPtr, &header, sizeof(header));
        bufferPtr += sizeof(header);
        
        // 응답 데이터 복사
        memcpy(bufferPtr, &responseData, sizeof(responseData));
        bufferPtr += sizeof(responseData);
        
        // 메시지 복사
        memcpy(bufferPtr, message.c_str(), message.length());
        
        // 5. 패킷 전송
        int totalBytesSent = 0;
        while (totalBytesSent < totalSize) {
            int bytesSent = send(clientSocket, 
                                packetBuffer.data() + totalBytesSent, 
                                totalSize - totalBytesSent, 
                                0);
            
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("거래 응답 전송 실패: {}\n", WSAGetLastError());
                return;
            }
            
            totalBytesSent += bytesSent;
        }
        
        std::cout << std::format("클라이언트 ID {}에게 거래 응답 전송됨. 수락: {}\n", targetId, accepted ? "예" : "아니오");
    }
};

// 클라이언트 클래스
class MixedLengthClient {
private:
    SOCKET clientSocket;
    bool connected;
    uint32_t userId;
    
public:
    MixedLengthClient() : clientSocket(INVALID_SOCKET), connected(false), userId(0) {}
    
    ~MixedLengthClient() {
        Disconnect();
    }
    
    bool Connect(const std::string& serverIP, int port = 27015, uint32_t userIdParam = 0) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
        
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("서버 연결 실패: {}\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }
        
        connected = true;
        userId = userIdParam;
        std::cout << std::format("서버 {}:{}에 연결됨\n", serverIP, port);
        return true;
    }
    
    void Disconnect() {
        if (connected && clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
            connected = false;
            WSACleanup();
            std::cout << "서버와 연결 종료\n";
        }
    }
    
    bool SendTradeRequest(uint32_t targetId, const std::vector<Item>& items, const std::string& message) {
        if (!connected || clientSocket == INVALID_SOCKET) {
            std::cerr << "서버에 연결되지 않음\n";
            return false;
        }
        
        // 1. 패킷 크기 계산
        uint16_t headerSize = sizeof(PacketHeader);
        uint16_t tradeRequestSize = sizeof(TradeRequestPacket);
        uint16_t itemsSize = static_cast<uint16_t>(items.size() * sizeof(Item));
        uint16_t messageSize = static_cast<uint16_t>(message.length());
        uint16_t totalSize = headerSize + tradeRequestSize + itemsSize + messageSize;
        
        // 2. 패킷 버퍼 생성
        std::vector<char> packetBuffer(totalSize);
        char* bufferPtr = packetBuffer.data();
        
        // 3. 헤더 설정
        PacketHeader header;
        header.totalSize = totalSize;
        header.packetType = PT_TRADE_REQUEST;
        memcpy(bufferPtr, &header, headerSize);
        bufferPtr += headerSize;
        
        // 4. 거래 요청 정보 설정
        TradeRequestPacket tradeRequest;
        tradeRequest.requesterId = userId;
        tradeRequest.targetId = targetId;
        tradeRequest.itemCount = static_cast<uint16_t>(items.size());
        memcpy(bufferPtr, &tradeRequest, tradeRequestSize);
        bufferPtr += tradeRequestSize;
        
        // 5. 아이템 정보 설정
        for (const auto& item : items) {
            memcpy(bufferPtr, &item, sizeof(Item));
            bufferPtr += sizeof(Item);
        }
        
        // 6. 메시지 추가 (있는 경우)
        if (!message.empty()) {
            memcpy(bufferPtr, message.c_str(), messageSize);
        }
        
        // 7. 패킷 전송
        int totalBytesSent = 0;
        while (totalBytesSent < totalSize) {
            int bytesSent = send(clientSocket, 
                               packetBuffer.data() + totalBytesSent, 
                               totalSize - totalBytesSent, 
                               0);
            
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("거래 요청 전송 실패: {}\n", WSAGetLastError());
                return false;
            }
            
            totalBytesSent += bytesSent;
        }
        
        std::cout << std::format("플레이어 ID {}에게 거래 요청 전송됨. 아이템 {}개, 메시지: {}\n", 
                             targetId, items.size(), message);
        
        // 8. 응답 수신 (별도 스레드나 콜백으로 처리할 수 있음)
        // 여기서는 간단하게 동기적으로 응답 기다림
        PacketHeader responseHeader;
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&responseHeader), sizeof(responseHeader), 0);
        
        if (bytesReceived != sizeof(responseHeader)) {
            std::cerr << "응답 헤더 수신 실패\n";
            return false;
        }
        
        if (responseHeader.packetType != PT_TRADE_RESPONSE) {
            std::cerr << std::format("예상치 못한 응답 유형: {}\n", responseHeader.packetType);
            return false;
        }
        
        uint16_t responseDataSize = responseHeader.totalSize - sizeof(responseHeader);
        std::vector<char> responseBuffer(responseDataSize);
        
        bytesReceived = recv(clientSocket, responseBuffer.data(), responseDataSize, 0);
        if (bytesReceived != responseDataSize) {
            std::cerr << "응답 데이터 수신 실패\n";
            return false;
        }
        
        // 응답 데이터 파싱 (예시)
        if (responseDataSize >= 9) {  // 최소 응답 크기 (senderId(4) + targetId(4) + accepted(1))
            uint32_t responderId = *reinterpret_cast<uint32_t*>(responseBuffer.data());
            uint32_t respTargetId = *reinterpret_cast<uint32_t*>(responseBuffer.data() + 4);
            uint8_t accepted = responseBuffer[8];
            
            std::string responseMessage;
            if (responseDataSize > 9) {
                responseMessage = std::string(responseBuffer.data() + 9, responseDataSize - 9);
            }
            
            std::cout << std::format("거래 응답 수신: 발신자={}, 수신자={}, 수락={}, 메시지={}\n", 
                                 responderId, respTargetId, accepted ? "예" : "아니오", responseMessage);
        }
        
        return true;
    }
};

// 테스트용 메인 함수
int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "1: 서버 모드, 2: 클라이언트 모드 - 선택: ";
    int mode;
    std::cin >> mode;
    
    if (mode == 1) {
        MixedLengthServer server;
        if (server.Start()) {
            std::cout << "서버가 시작되었습니다. 종료하려면 아무 키나 누르세요.\n";
            std::cin.ignore();
            std::cin.get();
            server.Stop();
        }
    } else if (mode == 2) {
        MixedLengthClient client;
        std::string serverIP;
        uint32_t userId, targetId;
        
        std::cout << "서버 IP를 입력하세요: ";
        std::cin.ignore();
        std::getline(std::cin, serverIP);
        
        std::cout << "당신의 사용자 ID를 입력하세요: ";
        std::cin >> userId;
        
        if (client.Connect(serverIP, 27015, userId)) {
            while (true) {
                std::cout << "\n1: 거래 요청 보내기, 2: 종료 - 선택: ";
                int choice;
                std::cin >> choice;
                
                if (choice == 2) {
                    break;
                } else if (choice == 1) {
                    std::cout << "거래할 대상 ID를 입력하세요: ";
                    std::cin >> targetId;
                    
                    // 아이템 목록 생성
                    std::vector<Item> items;
                    int itemCount;
                    std::cout << "보낼 아이템 수를 입력하세요: ";
                    std::cin >> itemCount;
                    
                    for (int i = 0; i < itemCount; i++) {
                        Item item;
                        std::cout << std::format("아이템 #{}:\n", i+1);
                        std::cout << "아이템 ID: ";
                        std::cin >> item.itemId;
                        std::cout << "수량: ";
                        std::cin >> item.quantity;
                        std::cout << "카테고리(0-무기, 1-방어구, 2-소모품): ";
                        std::cin >> item.category;
                        
                        items.push_back(item);
                    }
                    
                    std::string message;
                    std::cout << "거래 메시지를 입력하세요: ";
                    std::cin.ignore();
                    std::getline(std::cin, message);
                    
                    client.SendTradeRequest(targetId, items, message);
                }
            }
            
            client.Disconnect();
        }
    }
    
    return 0;
}
```  
</details>  
    
이 코드는 **게임에서 사용하는 복합 패킷 시스템**으로, 고정 길이와 가변 길이 데이터를 조합한 거래 요청 시스템을 구현한다. 온라인 게임의 아이템 거래 기능을 예시로 한 네트워크 프로그래밍이다.

### 패킷 구조 및 설계
    
#### 1. 기본 패킷 구조
![복합 패킷 구조 (Trade Request Packet)](./images/015.png)    
  
#### 2. 패킷 타입과 구조체

```cpp
enum PacketType {
    PT_TRADE_REQUEST = 1,   // 거래 요청
    PT_TRADE_RESPONSE,      // 거래 응답
    PT_CHAT_MESSAGE,        // 채팅 메시지
    PT_PLAYER_MOVE          // 플레이어 이동
};
```

각 패킷 타입별로 다른 데이터 구조를 가지며, 헤더의 `packetType` 필드로 구분한다.
  

### 핵심 클래스 구조  
![시스템 아키텍처 및 데이터 흐름](./images/016.png)   

### 핵심 기술적 특징

#### 1. **혼합형 데이터 구조**
```cpp
// 고정 길이 + 가변 길이 조합
struct TradeRequestPacket {
    uint32_t requesterId;   // 고정 4바이트
    uint32_t targetId;      // 고정 4바이트  
    uint16_t itemCount;     // 고정 2바이트
    // 이후 가변 길이: Item 배열 + 메시지 문자열
};
```

고정 길이 헤더로 기본 정보를 제공하고, 가변 길이 부분은 동적으로 처리한다.

#### 2. **패킷 크기 사전 계산**
```cpp
// 클라이언트에서 패킷 전송 전 크기 계산
uint16_t headerSize = sizeof(PacketHeader);
uint16_t tradeRequestSize = sizeof(TradeRequestPacket);
uint16_t itemsSize = static_cast<uint16_t>(items.size() * sizeof(Item));
uint16_t messageSize = static_cast<uint16_t>(message.length());
uint16_t totalSize = headerSize + tradeRequestSize + itemsSize + messageSize;
```

전체 패킷 크기를 미리 계산하여 헤더에 포함시킨다. 수신측에서 정확한 크기만큼만 읽을 수 있다.

#### 3. **메모리 직렬화 기법**
```cpp
// 패킷 데이터를 연속된 메모리에 직렬화
std::vector<char> packetBuffer(totalSize);
char* bufferPtr = packetBuffer.data();

memcpy(bufferPtr, &header, headerSize);
bufferPtr += headerSize;

memcpy(bufferPtr, &tradeRequest, tradeRequestSize);  
bufferPtr += tradeRequestSize;

// 아이템 배열 복사
for (const auto& item : items) {
    memcpy(bufferPtr, &item, sizeof(Item));
    bufferPtr += sizeof(Item);
}
```

구조체와 배열을 연속된 메모리에 순차적으로 복사하여 네트워크 전송용 버퍼를 만든다.

#### 4. **패킷 파싱 과정**
![서버의 거래 요청 패킷 파싱 과정](./images/017.png)    

#### 5. **클라이언트 매핑 테이블**
```cpp
std::map<uint32_t, SOCKET> clientSockets;
```

서버는 사용자 ID를 키로 하는 매핑 테이블을 유지한다. 거래 요청시 대상 클라이언트를 빠르게 찾을 수 있다.

#### 6. **에러 처리 및 검증**
```cpp
// 패킷 크기 검증
if (totalSize < sizeof(header) || dataSize > 8192) {
    std::cerr << std::format("잘못된 패킷 크기: {}\n", totalSize);
    break;
}

// 아이템 데이터 크기 검증  
if (sizeof(TradeRequestPacket) + itemsDataSize > dataSize) {
    std::cerr << "아이템 데이터가 패킷보다 큼\n";
    return;
}
```

패킷 크기와 내부 데이터 구조를 다중으로 검증하여 비정상 패킷을 방지한다.
  

### 실행 시나리오 예시  
![거래 요청 시나리오](./images/018.png) 
   
### 주요 개선점과 장점

#### 1. **타입 안전성**
```cpp
enum PacketType {
    PT_TRADE_REQUEST = 1,
    PT_TRADE_RESPONSE,
    PT_CHAT_MESSAGE,
    PT_PLAYER_MOVE
};
```
열거형을 사용하여 패킷 타입을 명확히 정의하고, switch문으로 안전하게 분기 처리한다.

#### 2. **메모리 정렬 보장**
```cpp
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t totalSize;
    uint16_t packetType;
};
#pragma pack(pop)
```
구조체 패딩을 제거하여 플랫폼 간 호환성을 보장한다.

#### 3. **효율적인 클라이언트 관리**
```cpp
std::map<uint32_t, SOCKET> clientSockets;
```
해시 맵을 사용하여 O(log n) 시간에 클라이언트 검색이 가능하다.

#### 4. **확장 가능한 설계**
새로운 패킷 타입을 추가할 때 enum에 타입 추가 후 switch문에 처리 로직만 추가하면 된다.

### 실제 사용법
**서버 실행:**
1. 프로그램 실행 후 `1` 선택
2. 다중 클라이언트 연결 대기

**클라이언트 실행:**
1. 프로그램 실행 후 `2` 선택  
2. 서버 IP와 사용자 ID 입력
3. 거래할 대상 ID, 아이템 정보, 메시지 입력
4. 자동으로 거래 요청 패킷 생성 및 전송

이 시스템은 **온라인 게임의 실시간 거래 시스템**을 구현한 것으로, 복잡한 데이터 구조를 안전하게 네트워크로 전송하는 좋은 예시다. 고정 길이와 가변 길이를 적절히 조합하여 메모리 효율성과 확장성을 동시에 확보했다.  
  
  
## 실습: 데이터 전송 후 종료 연습
네트워크 연결을 안전하게 종료하는 방법을 배우는 것은 중요합니다. 이 예제에서는 `shutdown`을 사용한 안전한 연결 종료 방법을 알아봅니다.   
아래 코드의 핵심적인 부분은 `GracefulDisconnect()` 함수이다.  

<details>
<summary>GracefulShutdownServer 코드</summary>  
  
```cpp
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <format>
#include <string>
#include <thread>
#include <vector>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

// 파일 전송 패킷 헤더
#pragma pack(push, 1)
struct FileHeader {
    uint32_t fileSize;      // 파일 전체 크기
    uint16_t nameLength;    // 파일 이름 길이
    // 이후에 파일 이름과 파일 데이터가 옴
};
#pragma pack(pop)

// 안전한 종료를 구현한 서버 클래스
class GracefulShutdownServer {
private:
    SOCKET listenSocket;
    std::vector<std::thread> clientThreads;
    bool running;

public:
    GracefulShutdownServer() : listenSocket(INVALID_SOCKET), running(false) {}
    
    ~GracefulShutdownServer() {
        Stop();
    }
    
    bool Start(int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("바인딩 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << std::format("리슨 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        running = true;
        std::cout << std::format("안전한 종료 서버가 포트 {}에서 시작됨\n", port);
        
        // 클라이언트 연결 수락 스레드 시작
        std::thread acceptThread(&GracefulShutdownServer::AcceptClients, this);
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
        std::cout << "서버가 중지됨\n";
    }
    
private:
    void AcceptClients() {
        while (running) {
            sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << std::format("클라이언트 연결 수락 실패: {}\n", WSAGetLastError());
                }
                continue;
            }
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::cout << std::format("새 클라이언트 연결: {}:{}\n", clientIP, ntohs(clientAddr.sin_port));
            
            clientThreads.emplace_back(&GracefulShutdownServer::HandleClient, this, clientSocket, std::string(clientIP));
        }
    }
    
    void HandleClient(SOCKET clientSocket, std::string clientIP) {
        // 파일 헤더 수신
        FileHeader header;
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&header), sizeof(header), 0);
        
        if (bytesReceived != sizeof(header)) {
            std::cerr << "파일 헤더 수신 실패\n";
            closesocket(clientSocket);
            return;
        }
        
        // 파일 이름 수신
        std::vector<char> fileNameBuffer(header.nameLength + 1, 0);
        bytesReceived = recv(clientSocket, fileNameBuffer.data(), header.nameLength, 0);
        
        if (bytesReceived != header.nameLength) {
            std::cerr << "파일 이름 수신 실패\n";
            closesocket(clientSocket);
            return;
        }
        
        std::string fileName(fileNameBuffer.data());
        std::cout << std::format("파일 수신 시작: {}, 크기: {} 바이트\n", fileName, header.fileSize);
        
        // 파일 데이터 수신
        std::vector<char> fileBuffer(header.fileSize);
        int totalBytesReceived = 0;
        
        // 진행 상황 업데이트 시간 추적
        auto lastUpdateTime = std::chrono::steady_clock::now();
        
        while (totalBytesReceived < header.fileSize) {
            bytesReceived = recv(clientSocket, 
                              fileBuffer.data() + totalBytesReceived,
                              header.fileSize - totalBytesReceived, 
                              0);
            
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << "클라이언트가 연결을 정상적으로 종료했습니다.\n";
                } else {
                    std::cerr << std::format("파일 데이터 수신 실패: {}\n", WSAGetLastError());
                }
                break;
            }
            
            totalBytesReceived += bytesReceived;
            
            // 1초마다 진행 상황 업데이트
            auto currentTime = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastUpdateTime).count() >= 1) {
                float progress = static_cast<float>(totalBytesReceived) / header.fileSize * 100.0f;
                std::cout << std::format("파일 수신 중: {:.1f}% 완료\n", progress);
                lastUpdateTime = currentTime;
            }
        }
        
        // 파일 수신 완료
        if (totalBytesReceived == header.fileSize) {
            std::cout << std::format("파일 '{}' 수신 완료 ({} 바이트)\n", fileName, totalBytesReceived);
            
            // 여기서 파일을 저장하거나 처리할 수 있음
            
            // 수신 완료 응답 보내기
            std::string response = "파일 수신 완료";
            send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
            
            // 안전하게 연결 종료 (서버 측에서 먼저 종료)
            std::cout << "연결 종료 시작 (서버 측)...\n";
            
            // 송신 방향 종료 (더 이상 데이터를 보내지 않을 것임을 알림)
            shutdown(clientSocket, SD_SEND);
            
            // 클라이언트가 보낸 데이터가 있다면 모두 수신
            char buffer[1024];
            while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
                std::cout << std::format("연결 종료 중 추가 데이터 {} 바이트 수신됨\n", bytesReceived);
            }
            
            std::cout << "연결 안전하게 종료됨\n";
        } else {
            std::cerr << std::format("파일 수신 불완전: 예상 {}, 실제 {} 바이트\n", header.fileSize, totalBytesReceived);
        }
        
        // 소켓 닫기
        closesocket(clientSocket);
    }
};

// 안전한 종료를 구현한 클라이언트 클래스
class GracefulShutdownClient {
private:
    SOCKET clientSocket;
    bool connected;
    
public:
    GracefulShutdownClient() : clientSocket(INVALID_SOCKET), connected(false) {}
    
    ~GracefulShutdownClient() {
        Disconnect();
    }
    
    bool Connect(const std::string& serverIP, int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
        
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("서버 연결 실패: {}\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }
        
        connected = true;
        std::cout << std::format("서버 {}:{}에 연결됨\n", serverIP, port);
        return true;
    }
    
    void Disconnect() {
        if (connected && clientSocket != INVALID_SOCKET) {
            // 안전한 연결 종료 시도
            GracefulDisconnect();
            
            // 소켓 닫기
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
            connected = false;
            WSACleanup();
        }
    }
    
    bool SendFile(const std::string& fileName, const std::vector<char>& fileData) {
        if (!connected || clientSocket == INVALID_SOCKET) {
            std::cerr << "서버에 연결되지 않음\n";
            return false;
        }
        
        // 1. 파일 헤더 준비
        FileHeader header;
        header.fileSize = static_cast<uint32_t>(fileData.size());
        header.nameLength = static_cast<uint16_t>(fileName.length());
        
        // 2. 헤더 전송
        int bytesSent = send(clientSocket, reinterpret_cast<char*>(&header), sizeof(header), 0);
        if (bytesSent != sizeof(header)) {
            std::cerr << "파일 헤더 전송 실패\n";
            return false;
        }
        
        // 3. 파일 이름 전송
        bytesSent = send(clientSocket, fileName.c_str(), header.nameLength, 0);
        if (bytesSent != header.nameLength) {
            std::cerr << "파일 이름 전송 실패\n";
            return false;
        }
        
        // 4. 파일 데이터 전송
        int totalBytesSent = 0;
        const int CHUNK_SIZE = 4096;  // 한 번에 4KB씩 전송
        
        // 진행 상황 업데이트 시간 추적
        auto lastUpdateTime = std::chrono::steady_clock::now();
        
        while (totalBytesSent < header.fileSize) {
            int bytesToSend = std::min(CHUNK_SIZE, static_cast<int>(header.fileSize - totalBytesSent));
            
            bytesSent = send(clientSocket, 
                           fileData.data() + totalBytesSent, 
                           bytesToSend, 
                           0);
            
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("파일 데이터 전송 실패: {}\n", WSAGetLastError());
                return false;
            }
            
            totalBytesSent += bytesSent;
            
            // 진행 상황 업데이트 (1초마다)
            auto currentTime = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastUpdateTime).count() >= 1) {
                float progress = static_cast<float>(totalBytesSent) / header.fileSize * 100.0f;
                std::cout << std::format("파일 전송 중: {:.1f}% 완료\n", progress);
                lastUpdateTime = currentTime;
            }
            
            // 전송 속도 제어 (필요한 경우)
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::cout << std::format("파일 '{}' 전송 완료 ({} 바이트)\n", fileName, totalBytesSent);
        
        // 5. 서버 응답 수신
        char responseBuffer[1024] = {0};
        int bytesReceived = recv(clientSocket, responseBuffer, sizeof(responseBuffer) - 1, 0);
        
        if (bytesReceived > 0) {
            std::cout << "서버 응답: " << responseBuffer << std::endl;
        } else {
            std::cerr << "서버 응답 수신 실패\n";
            return false;
        }
        
        // 6. 안전한 연결 종료
        return GracefulDisconnect();
    }
    
private:
    bool GracefulDisconnect() {
        if (!connected || clientSocket == INVALID_SOCKET) {
            return false;
        }
        
        std::cout << "안전한 연결 종료 시작...\n";
        
        // 1. 송신 방향 종료 (더 이상 데이터를 보내지 않음)
        if (shutdown(clientSocket, SD_SEND) == SOCKET_ERROR) {
            std::cerr << std::format("shutdown 실패: {}\n", WSAGetLastError());
            return false;
        }
        
        // 2. 서버가 보낸 모든 데이터 수신
        char buffer[1024];
        int bytesReceived;
        
        while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << std::format("연결 종료 중 추가 데이터 수신: {}\n", buffer);
        }
        
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << std::format("recv 실패: {}\n", WSAGetLastError());
            return false;
        }
        
        std::cout << "안전한 연결 종료 완료\n";
        return true;
    }
};

// 테스트용 메인 함수
int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "1: 서버 모드, 2: 클라이언트 모드 - 선택: ";
    int mode;
    std::cin >> mode;
    
    if (mode == 1) {
        GracefulShutdownServer server;
        if (server.Start()) {
            std::cout << "서버가 시작되었습니다. 종료하려면 아무 키나 누르세요.\n";
            std::cin.ignore();
            std::cin.get();
            server.Stop();
        }
    } else if (mode == 2) {
        GracefulShutdownClient client;
        std::string serverIP;
        
        std::cout << "서버 IP를 입력하세요: ";
        std::cin.ignore();
        std::getline(std::cin, serverIP);
        
        if (client.Connect(serverIP)) {
            // 가상의 파일 데이터 생성 (실제로는 파일에서 읽어올 수 있음)
            std::string fileName = "test_data.bin";
            size_t fileSize;
            
            std::cout << "전송할 파일 크기(바이트)를 입력하세요: ";
            std::cin >> fileSize;
            
            std::vector<char> fileData(fileSize);
            
            // 파일 데이터 생성 (간단한 패턴으로 채움)
            for (size_t i = 0; i < fileSize; i++) {
                fileData[i] = static_cast<char>(i % 256);
            }
            
            // 파일 전송 및 안전한 종료
            client.SendFile(fileName, fileData);
            
            // 명시적으로 연결 종료
            client.Disconnect();
        }
    }
    
    return 0;
}
```  
</details>  
  
   
## 정리: 데이터 전송의 주요 고려사항

1. **응용 프로그램 프로토콜 설계**
   - 패킷 구조(헤더, 페이로드)
   - 바이트 정렬 처리
   - 데이터 경계 설정 방법

2. **데이터 송수신 전략**
   - 고정 길이: 구현 간단, 공간 낭비 가능성
   - 가변 길이: 효율적 공간 사용, 경계 처리 필요
   - 혼합 구조: 헤더는 고정, 페이로드는 가변 길이

3. **신뢰성 확보**
   - 패킷 크기 검증
   - 데이터 무결성 검사
   - 완전 수신 보장 (부분 수신 대응)

4. **연결 종료**
   - `shutdown` 함수로 안전한 종료
   - 송신과 수신을 독립적으로 제어
   - 모든 데이터 교환 완료 확인

온라인 게임 서버 개발에서는 이러한 기본적인 데이터 전송 방식을 바탕으로, 게임의 특성에 맞는 최적화된 프로토콜을 설계하고 구현하는 것이 중요합니다. 실시간성이 중요한 게임에서는 데이터 압축, 패킷 최적화, 우선순위 처리 등의 고급 기법도 함께 적용해 볼 수 있습니다.  

  
<br>        
     
# Chapter.06 멀티스레드 프로그래밍
  
## 01 스레드 기초
스레드(Thread)는 프로세스 내에서 실행되는 독립적인 실행 흐름으로, 온라인 게임 서버에서 매우 중요한 개념입니다. 대규모 온라인 게임은 수많은 클라이언트를 동시에 처리해야 하므로 효율적인 멀티스레딩이 필수적입니다.

### 스레드의 개념
프로세스는 운영체제로부터 할당받은 자원의 단위이며, 각 프로세스는 독립된 메모리 공간을 가집니다. 반면, 스레드는 하나의 프로세스 내에서 여러 개 생성될 수 있으며, 같은 프로세스 내의 스레드들은 메모리 공간을 공유합니다.
  

### 프로세스와 스레드의 차이점

| 구분 | 프로세스 | 스레드 |
|------|---------|--------|
| 정의 | 실행 중인 프로그램 | 프로세스 내 실행 흐름 |
| 자원 소유 | O | X (프로세스의 자원 공유) |
| 메모리 공간 | 독립적 | 공유 (스택 제외) |
| 통신 비용 | 높음 (IPC 필요) | 낮음 (직접 메모리 접근) |
| 문맥 전환 비용 | 높음 | 낮음 |
| 안정성 | 한 프로세스 중단 시 다른 프로세스는 영향 없음 | 한 스레드 중단 시 전체 프로세스 영향 |
  

### 스레드 사용의 장점
1. **병렬 처리**: 다중 코어 CPU에서 여러 작업을 동시에 실행하여 성능 향상
2. **응답성 향상**: UI 스레드와 작업 스레드를 분리하여 사용자 반응성 유지
3. **자원 공유**: 같은 메모리 공간을 공유하여 통신 비용 감소
4. **효율성**: 프로세스 생성보다 스레드 생성이 더 빠르고 경제적
  

### 스레드 사용의 단점
1. **동기화 문제**: 공유 자원 접근 시 레이스 컨디션(Race Condition) 발생 가능
2. **디버깅 어려움**: 동시성 관련 버그는 발견과 수정이 어려움
3. **교착 상태(Deadlock)**: 잘못된 락 획득 순서로 인한 상호 대기 상태
4. **기아 상태(Starvation)**: 특정 스레드가 필요한 자원을 계속 얻지 못하는 상태
  

### 게임 서버에서의 스레드 활용
온라인 게임 서버에서 멀티스레딩은 다음과 같은 용도로 활용됩니다:

1. **클라이언트 연결 관리**: 연결 수락 및 각 클라이언트 통신 처리
2. **게임 로직 처리**: 게임 상태 업데이트, AI, 물리 계산 등
3. **데이터베이스 작업**: 비동기 데이터 저장 및 로드
4. **주기적 작업**: 타이머 이벤트, 세션 관리, 자원 정리 등
  
![](./images/039.png)    
   
**방식 1 (I/O와 처리 분리)이 적합한 경우:**
- 처리량이 중요한 게임 
- 게임 콘텐츠 중 공유 객체에 lock을 걸어야 한다.
- 패킷 순서가 크게 중요하지 않은 상황
```
// 방식 1: 네트워크 I/O와 패킷 처리 분리
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

// Thread-Safe 패킷 큐
class PacketQueue {
private:
    std::queue<Packet> packets;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(const Packet& packet) {
        std::lock_guard<std::mutex> lock(mtx);
        packets.push(packet);
        cv.notify_one();
    }
    
    Packet pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !packets.empty(); });
        Packet packet = packets.front();
        packets.pop();
        return packet;
    }
};

class GameServer_Method1 {
private:
    PacketQueue packetQueue;
    std::vector<std::thread> ioThreads;
    std::vector<std::thread> processingThreads;
    
public:
    void start() {
        // 네트워크 I/O 스레드들 시작
        for (int i = 0; i < 4; ++i) {
            ioThreads.emplace_back([this]() {
                networkIOLoop();
            });
        }
        
        // 패킷 처리 스레드들 시작
        for (int i = 0; i < 8; ++i) {
            processingThreads.emplace_back([this]() {
                packetProcessingLoop();
            });
        }
    }
    
private:
    void networkIOLoop() {
        while (running) {
            // 소켓에서 데이터 수신
            Packet packet = receiveFromSocket();
            if (packet.isValid()) {
                // 큐에 패킷 추가 (스레드 안전)
                packetQueue.push(packet);
            }
        }
    }
    
    void packetProcessingLoop() {
        while (running) {
            // 큐에서 패킷 가져와서 처리
            Packet packet = packetQueue.pop();
            processPacket(packet);
        }
    }
};
```    


**방식 2 (기능별 분리)가 적합한 경우:**
- 패킷 처리 순서 보장 (단일 스레드)
- 게임 콘텐츠 중 공유 객체에 lock을 걸지 않아도 된다
- DB 작업이 많고 복잡한 게임
- 성능 튜닝과 모니터링이 중요한 상용 서비스     
```
// 방식 2: 기능별 스레드 분리 (전문화)
class GameServer_Method2 {
private:
    // 네트워크 스레드들 (2개 이상)
    std::vector<std::thread> networkThreads;
    
    // 패킷 처리 스레드 (1개 - 순서 보장)
    std::thread packetProcessThread;
    
    // DB 스레드들 (2개 이상)
    std::vector<std::thread> dbThreads;
    
    // 스레드 간 통신을 위한 큐들
    ThreadSafeQueue<Packet> networkToPacketQueue;
    ThreadSafeQueue<DBRequest> packetToDBQueue;
    ThreadSafeQueue<DBResponse> dbToPacketQueue;
    
public:
    void start() {
        // 네트워크 스레드들 시작 (최소 2개)
        for (int i = 0; i < 3; ++i) {
            networkThreads.emplace_back([this, i]() {
                networkThreadLoop(i);
            });
        }
        
        // 패킷 처리 스레드 시작 (1개만)
        packetProcessThread = std::thread([this]() {
            packetProcessingLoop();
        });
        
        // DB 스레드들 시작 (최소 2개)
        for (int i = 0; i < 4; ++i) {
            dbThreads.emplace_back([this, i]() {
                dbThreadLoop(i);
            });
        }
    }
    
private:
    // 네트워크 전담 스레드
    void networkThreadLoop(int threadId) {
        while (running) {
            // 각 스레드가 일부 클라이언트 담당
            auto clients = getAssignedClients(threadId);
            
            for (auto& client : clients) {
                Packet packet = receiveFromClient(client);
                if (packet.isValid()) {
                    // 패킷 처리 스레드로 전달
                    networkToPacketQueue.push(packet);
                }
            }
        }
    }
    
    // 패킷 처리 전담 스레드 (1개로 순서 보장)
    void packetProcessingLoop() {
        while (running) {
            // 네트워크에서 받은 패킷 처리
            if (!networkToPacketQueue.empty()) {
                Packet packet = networkToPacketQueue.pop();
                
                // 게임 로직 처리
                GameEvent event = processGameLogic(packet);
                
                // DB 작업이 필요하면 DB 스레드에 요청
                if (event.needsDB()) {
                    DBRequest dbReq = createDBRequest(event);
                    packetToDBQueue.push(dbReq);
                }
            }
            
            // DB에서 완료된 작업 처리
            if (!dbToPacketQueue.empty()) {
                DBResponse response = dbToPacketQueue.pop();
                handleDBResponse(response);
            }
        }
    }
    
    // DB 전담 스레드들
    void dbThreadLoop(int threadId) {
        // 각 스레드가 다른 DB 연결 사용
        DatabaseConnection db = createDBConnection(threadId);
        
        while (running) {
            if (!packetToDBQueue.empty()) {
                DBRequest request = packetToDBQueue.pop();
                
                // DB 작업 수행 (시간이 오래 걸릴 수 있음)
                DBResponse response = executeDBQuery(db, request);
                
                // 결과를 패킷 처리 스레드로 전달
                dbToPacketQueue.push(response);
            }
        }
    }
};
```



## 02 스레드 API
C++11부터 표준 라이브러리에서 스레드 지원이 추가되어 플랫폼 독립적인 멀티스레딩이 가능해졌습니다. 본 내용에서는 Win32 API 대신 C++의 표준 스레드 API를 중심으로 설명하겠습니다.

### C++ 스레드 라이브러리 vs Win32 스레드 API

| C++ 스레드 라이브러리 | Win32 스레드 API |
|---------------------|-----------------|
| std::thread | CreateThread |
| std::mutex | CRITICAL_SECTION |
| std::condition_variable | CONDITION_VARIABLE |
| std::async | - |
| std::future/promise | - |
  

### 스레드 생성과 관리

```cpp
#include <iostream>
#include <thread>
#include <format>

// 스레드 함수
void threadFunction() {
    std::cout << std::format("스레드 ID: {}\n", std::this_thread::get_id());
    for (int i = 0; i < 5; ++i) {
        std::cout << std::format("스레드에서 카운트: {}\n", i);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << std::format("메인 스레드 ID: {}\n", std::this_thread::get_id());
    
    // 스레드 생성
    std::thread t(threadFunction);
    
    // 메인 스레드 작업
    for (int i = 0; i < 3; ++i) {
        std::cout << std::format("메인에서 카운트: {}\n", i);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    // 스레드 종료 대기
    t.join();
    
    std::cout << "모든 스레드 종료\n";
    
    return 0;
}
```

### 스레드에 인수 전달

```cpp
#include <iostream>
#include <thread>
#include <string>
#include <format>

// 값에 의한 전달
void threadFunction(int id, std::string name) {
    std::cout << std::format("스레드 {}({}): 실행 시작\n", id, name);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << std::format("스레드 {}({}): 실행 완료\n", id, name);
}

// 참조에 의한 전달
void threadFunctionRef(int id, const std::string& name, int& result) {
    std::cout << std::format("스레드 {}({}): 계산 시작\n", id, name);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 결과값 계산 및 참조로 반환
    result = id * 10;
    
    std::cout << std::format("스레드 {}({}): 계산 완료 (결과: {})\n", id, name, result);
}

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    // 1. 값에 의한 전달
    std::thread t1(threadFunction, 1, "작업 스레드");
    
    // 2. 참조로 결과 반환
    int result = 0;
    std::thread t2(threadFunctionRef, 2, "계산 스레드", std::ref(result));
    
    // 3. 람다 함수로 스레드 생성
    std::thread t3([](int id) {
        std::cout << std::format("람다 스레드 {}: 실행\n", id);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << std::format("람다 스레드 {}: 완료\n", id);
    }, 3);
    
    // 모든 스레드 종료 대기
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << std::format("계산 결과: {}\n", result);
    std::cout << "모든 스레드 종료\n";
    
    return 0;
}
```

### std::async와 std::future
`std::async`는 비동기 작업을 쉽게 생성하고 `std::future`를 통해 결과를 받을 수 있는 편리한 방법입니다.

```cpp
#include <iostream>
#include <future>
#include <chrono>
#include <format>

// 결과를 반환하는 함수
int calculateResult(int value) {
    std::cout << std::format("계산 시작 (입력: {})\n", value);
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 무거운 작업 시뮬레이션
    return value * value;
}

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "비동기 작업 시작\n";
    
    // std::async로 비동기 작업 시작
    std::future<int> result1 = std::async(std::launch::async, calculateResult, 10);
    std::future<int> result2 = std::async(std::launch::async, calculateResult, 20);
    
    std::cout << "메인 스레드에서 다른 작업 수행 중...\n";
    
    // 결과가 준비될 때까지 블로킹
    int value1 = result1.get();
    int value2 = result2.get();
    
    std::cout << std::format("결과 1: {}\n", value1);
    std::cout << std::format("결과 2: {}\n", value2);
    std::cout << std::format("합계: {}\n", value1 + value2);
    
    return 0;
}
```
  

## 03 멀티스레드 TCP 서버
멀티스레드 TCP 서버는 다수의 클라이언트 연결을 효율적으로 처리하기 위한 구조입니다. 일반적으로 다음과 같은 모델을 사용합니다:

1. **Accept 스레드**: 새로운 클라이언트 연결을 수락하는 전용 스레드
2. **Worker 스레드 풀**: 클라이언트 요청을 처리하는 스레드 집합
3. **연결 관리**: 각 클라이언트 연결을 스레드에 할당하는 방식

### 스레드 풀 기반 TCP 서버 구현
다음은 스레드 풀을 사용한 간단한 멀티스레드 에코 서버 예제입니다.

<details>
<summary>ThreadPool Echo Server 코드</summary>  

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <format>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this, i] {
                std::cout << std::format("워커 스레드 {} 시작\n", i);
                
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        
                        // 작업이 있거나 중단 신호가 올 때까지 대기
                        this->condition.wait(lock, [this] { 
                            return this->stop || !this->tasks.empty(); 
                        });
                        
                        // 중단 신호가 왔고 작업이 없으면 종료
                        if (this->stop && this->tasks.empty()) {
                            std::cout << std::format("워커 스레드 {} 종료\n", i);
                            return;
                        }
                        
                        // 작업 가져오기
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    
                    // 작업 실행
                    task();
                }
            });
        }
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        
        condition.notify_all();
        
        for (std::thread &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    // 작업 추가
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) {
                throw std::runtime_error("스레드 풀 중단 후 작업 추가 시도");
            }
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
};

class TCPServer {
private:
    SOCKET listenSocket;
    ThreadPool threadPool;
    std::atomic<bool> running;
    std::mutex consoleMutex; // 콘솔 출력용 뮤텍스

public:
    TCPServer(size_t numThreads) : threadPool(numThreads), running(false), listenSocket(INVALID_SOCKET) {}
    
    ~TCPServer() {
        Stop();
    }
    
    bool Start(int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("바인딩 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << std::format("리슨 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        running = true;
        
        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << std::format("TCP 서버가 포트 {}에서 시작됨\n", port);
        }
        
        // Accept 스레드 시작
        std::thread acceptThread(&TCPServer::AcceptConnections, this);
        acceptThread.detach();
        
        return true;
    }
    
    void Stop() {
        running = false;
        
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        
        WSACleanup();
        
        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << "서버가 중지됨\n";
        }
    }
    
private:
    void AcceptConnections() {
        while (running) {
            sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << std::format("클라이언트 연결 수락 실패: {}\n", WSAGetLastError());
                }
                continue;
            }
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            
            {
                std::lock_guard<std::mutex> lock(consoleMutex);
                std::cout << std::format("새 클라이언트 연결: {}:{}\n", clientIP, ntohs(clientAddr.sin_port));
            }
            
            // 클라이언트 처리 작업을 스레드 풀에 추가
            threadPool.enqueue([this, clientSocket, clientIP]() {
                this->HandleClient(clientSocket, std::string(clientIP));
            });
        }
    }
    
    void HandleClient(SOCKET clientSocket, const std::string& clientIP) {
        const int BUFFER_SIZE = 1024;
        char buffer[BUFFER_SIZE];
        
        while (running) {
            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << std::format("클라이언트 {}가 연결을 종료함\n", clientIP);
                } else {
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cerr << std::format("recv 실패: {}\n", WSAGetLastError());
                }
                break;
            }
            
            buffer[bytesReceived] = '\0';
            
            {
                std::lock_guard<std::mutex> lock(consoleMutex);
                std::cout << std::format("{}로부터 수신: {}\n", clientIP, buffer);
            }
            
            // 에코 응답
            int bytesSent = send(clientSocket, buffer, bytesReceived, 0);
            if (bytesSent == SOCKET_ERROR) {
                std::lock_guard<std::mutex> lock(consoleMutex);
                std::cerr << std::format("send 실패: {}\n", WSAGetLastError());
                break;
            }
        }
        
        closesocket(clientSocket);
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    // 하드웨어 스레드 수에 기반한 스레드 풀 크기 계산
    size_t numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // 감지 실패 시 기본값
    
    std::cout << std::format("스레드 풀 크기: {}\n", numThreads);
    
    TCPServer server(numThreads);
    if (server.Start()) {
        std::cout << "서버가 시작되었습니다. 종료하려면 아무 키나 누르세요.\n";
        std::cin.get();
        server.Stop();
    }
    
    return 0;
}
```  
</details>  
  
![TCP 서버 with 스레드 풀 아키텍처](./images/020.png)     
  
**주요 구성 요소:**
1. **메인 스레드**: 서버를 초기화하고 스레드 풀을 생성한다
2. **Accept 스레드**: 클라이언트 연결을 지속적으로 수락하고, 각 클라이언트 처리 작업을 스레드 풀의 큐에 추가한다
3. **스레드 풀**: 고정된 개수의 워커 스레드들이 작업 큐에서 클라이언트 처리 작업을 가져와 실행한다
  
**동작 흐름:**
1. 클라이언트가 연결을 요청하면 Accept 스레드가 수락
2. Accept 스레드가 `HandleClient` 작업을 큐에 등록 
3. 대기 중인 워커 스레드가 작업을 가져와 클라이언트와 통신
4. recv/send 루프를 통해 에코 서버 역할 수행
  
**동기화 메커니즘:**
- `queueMutex`: 작업 큐의 동시 접근을 방지
- `condition_variable`: 워커 스레드가 효율적으로 작업을 대기
- `atomic<bool> stop`: 안전한 서버 종료를 위한 플래그
- `consoleMutex`: 콘솔 출력의 동기화

이 설계의 장점은 스레드 생성/소멸 오버헤드 없이 다수의 클라이언트를 동시에 처리할 수 있다는 것이다. 워커 스레드 개수는 하드웨어 스레드 수에 맞춰 자동 조정된다.  
  

### 스레드 풀 설계의 장점
1. **자원 효율성**: 미리 생성된 스레드를 재사용하여 스레드 생성/소멸 비용 절감
2. **부하 분산**: 여러 스레드에 작업을 고르게 분산 가능
3. **시스템 안정성**: 동시 실행 스레드 수를 제한하여 시스템 과부하 방지
4. **확장성**: 필요에 따라 스레드 수를 조정 가능
  
  
## 04 스레드 동기화
멀티스레드 프로그래밍에서 가장 중요한 문제 중 하나는 공유 자원에 대한 접근을 동기화하는 것입니다. 잘못된 동기화는 데이터 경쟁(Data Race), 교착 상태(Deadlock), 기아 상태(Starvation) 등의 문제를 일으킬 수 있습니다.  
    
![채팅서버 게임방 멀티스레드 접근 및 Lock](./images/040.png)       
  
### 뮤텍스(Mutex)
뮤텍스는 상호 배제(Mutual Exclusion)를 구현하는 동기화 기법으로, 공유 자원에 대한 접근을 한 번에 하나의 스레드로 제한합니다.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <format>

class Counter {
private:
    int value = 0;
    std::mutex mutex;

public:
    void increment() {
        std::lock_guard<std::mutex> lock(mutex);
        ++value;
    }
    
    int getValue() {
        std::lock_guard<std::mutex> lock(mutex);
        return value;
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    Counter counter;
    std::vector<std::thread> threads;
    
    const int NUM_THREADS = 10;
    const int NUM_INCREMENTS = 100000;
    
    // 여러 스레드에서 동시에 카운터 증가
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&counter, i, NUM_INCREMENTS]{
            for (int j = 0; j < NUM_INCREMENTS; ++j) {
                counter.increment();
            }
            std::cout << std::format("스레드 {} 완료\n", i);
        });
    }
    
    // 모든 스레드 종료 대기
    for (auto& t : threads) {
        t.join();
    }
    
    // 최종 결과 출력
    std::cout << std::format("예상 값: {}\n", NUM_THREADS * NUM_INCREMENTS);
    std::cout << std::format("실제 값: {}\n", counter.getValue());
    
    return 0;
}
```

### 락 가드와 유니크 락
C++에서는 RAII(Resource Acquisition Is Initialization) 원칙에 따라 뮤텍스를 자동으로 잠그고 해제하는 여러 유틸리티를 제공합니다.

1. **std::lock_guard**: 생성 시 락을 획득하고 소멸 시 자동으로 해제
2. **std::unique_lock**: lock_guard보다 유연하며, 수동으로 잠금/해제 가능
3. **std::shared_lock**: 공유 뮤텍스와 함께 사용하여 읽기 공유 락 구현
  
  
### 데드락(Deadlock) 방지
데드락은 두 개 이상의 스레드가 서로 상대방이 점유한 자원을 기다리며 무한히 대기하는 상황입니다.  
  
![Resource 전송 방법별 Lock 동작 비교](./images/041.png)   

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <format>

class Resource {
private:
    std::mutex mutex;
    int value = 0;
    
public:
    Resource(int initialValue) : value(initialValue) {}
    
    // 안전하지 않은 전송 방법 (데드락 가능성)
    void transferUnsafe(Resource& other, int amount) {
        // 내 리소스 락 획득
        std::lock_guard<std::mutex> lockThis(mutex);
        
        // 작업 지연 시뮬레이션
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 상대 리소스 락 획득 (이미 다른 스레드가 잠갔다면 데드락!)
        std::lock_guard<std::mutex> lockOther(other.mutex);
        
        if (value >= amount) {
            value -= amount;
            other.value += amount;
            std::cout << std::format("전송 성공: {} 단위\n", amount);
        } else {
            std::cout << "전송 실패: 잔액 부족\n";
        }
    }
    
    // 안전한 전송 방법 (std::lock 사용)
    void transferSafe(Resource& other, int amount) {
        // 두 뮤텍스를 한 번에 안전하게 락
        std::unique_lock<std::mutex> lockThis(mutex, std::defer_lock);
        std::unique_lock<std::mutex> lockOther(other.mutex, std::defer_lock);
        
        // 데드락 없이 두 뮤텍스 모두 획득
        std::lock(lockThis, lockOther);
        
        if (value >= amount) {
            value -= amount;
            other.value += amount;
            std::cout << std::format("전송 성공: {} 단위\n", amount);
        } else {
            std::cout << "전송 실패: 잔액 부족\n";
        }
    }
    
    // C++17의 scoped_lock을 사용한 더 간단한 방법
    void transferSafeScoped(Resource& other, int amount) {
        // 한 줄로 여러 뮤텍스를 안전하게 락
        std::scoped_lock lock(mutex, other.mutex);
        
        if (value >= amount) {
            value -= amount;
            other.value += amount;
            std::cout << std::format("전송 성공: {} 단위\n", amount);
        } else {
            std::cout << "전송 실패: 잔액 부족\n";
        }
    }
    
    int getValue() const {
        std::lock_guard<std::mutex> lock(mutex);
        return value;
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    Resource resource1(1000);
    Resource resource2(1000);
    
    // 안전하지 않은 방법 - 데드락 가능성 있음
    /*
    std::thread t1([&resource1, &resource2]() {
        for (int i = 0; i < 5; ++i) {
            resource1.transferUnsafe(resource2, 100);
        }
    });
    
    std::thread t2([&resource1, &resource2]() {
        for (int i = 0; i < 5; ++i) {
            resource2.transferUnsafe(resource1, 50);
        }
    });
    */
    
    // 안전한 방법 - 데드락 방지
    std::thread t1([&resource1, &resource2]() {
        for (int i = 0; i < 5; ++i) {
            resource1.transferSafeScoped(resource2, 100);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    std::thread t2([&resource1, &resource2]() {
        for (int i = 0; i < 5; ++i) {
            resource2.transferSafeScoped(resource1, 50);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    t1.join();
    t2.join();
    
    std::cout << std::format("최종 상태 - 리소스 1: {}, 리소스 2: {}\n", 
                        resource1.getValue(), resource2.getValue());
    
    return 0;
}
```

### 조건 변수(Condition Variable)
조건 변수는 스레드 간 신호를 주고받기 위한 동기화 기법으로, 특정 조건이 만족될 때까지 스레드를 대기시키는 데 사용됩니다.  
  
![스레드 안정한 큐 (Producer-Consumer 패턴)](./images/022.png)     

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <format>

// 스레드 안전한 큐 구현
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable condVar;
    
public:
    // 아이템 추가
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(std::move(item));
        }
        condVar.notify_one();  // 대기 중인 스레드에 신호
    }
    
    // 아이템 가져오기 (비어있으면 대기)
    T pop() {
        std::unique_lock<std::mutex> lock(mutex);
        
        // 큐가 비어있지 않을 때까지 대기
        condVar.wait(lock, [this]{ return !queue.empty(); });
        
        T item = std::move(queue.front());
        queue.pop();
        return item;
    }
    
    // 비어있는지 확인
    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }
    
    // 크기 확인
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    ThreadSafeQueue<int> queue;
    
    // 생산자 스레드
    std::thread producer([&queue]() {
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::cout << std::format("생산: {}\n", i);
            queue.push(i);
        }
    });
    
    // 소비자 스레드
    std::thread consumer([&queue]() {
        for (int i = 0; i < 10; ++i) {
            int value = queue.pop();  // 아이템이 없으면 대기
            std::cout << std::format("소비: {}\n", value);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "모든 작업 완료\n";
    
    return 0;
}
```
  

### 원자적 연산(Atomic Operations)
락 대신 원자적 연산을 사용하면 성능을 향상시킬 수 있습니다. C++에서는 `std::atomic` 타입을 제공합니다.

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <format>

class AtomicCounter {
private:
    std::atomic<int> value{0};

public:
    void increment() {
        ++value;  // 원자적 증가 연산
    }
    
    int getValue() const {
        return value.load();  // 원자적 읽기 연산
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    AtomicCounter counter;
    std::vector<std::thread> threads;
    
    const int NUM_THREADS = 10;
    const int NUM_INCREMENTS = 100000;
    
    // 여러 스레드에서 동시에 카운터 증가
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&counter, i, NUM_INCREMENTS]{
            for (int j = 0; j < NUM_INCREMENTS; ++j) {
                counter.increment();
            }
            std::cout << std::format("스레드 {} 완료\n", i);
        });
    }
    
    // 모든 스레드 종료 대기
    for (auto& t : threads) {
        t.join();
    }
    
    // 최종 결과 출력
    std::cout << std::format("예상 값: {}\n", NUM_THREADS * NUM_INCREMENTS);
    std::cout << std::format("실제 값: {}\n", counter.getValue());
    
    return 0;
}
```
  

## 실습: 스레드 생성과 종료, 인수 전달 연습
다양한 방식으로 스레드를 생성하고 인수를 전달하는 방법을 연습해 봅시다.

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <functional>
#include <format>
#include <Windows.h>

// 일반 함수
void threadFunction(int id) {
    std::cout << std::format("일반 함수 스레드 {}: 시작\n", id);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << std::format("일반 함수 스레드 {}: 종료\n", id);
}

// 여러 매개변수를 받는 함수
void parameterizedFunction(int id, std::string name, bool flag) {
    std::cout << std::format("스레드 {}({}): 시작, 플래그={}\n", id, name, flag ? "true" : "false");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << std::format("스레드 {}({}): 종료\n", id, name);
}

// 참조 매개변수가 있는 함수
void referenceFunction(int id, std::vector<int>& values) {
    std::cout << std::format("참조 스레드 {}: 시작\n", id);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 벡터 수정 (참조로 전달된 벡터가 원본에 반영됨)
    for (int i = 0; i < 5; ++i) {
        values.push_back(id * 10 + i);
    }
    
    std::cout << std::format("참조 스레드 {}: 종료\n", id);
}

// 함수 객체 (Functor)
class ThreadFunctor {
private:
    int id;
    
public:
    ThreadFunctor(int id) : id(id) {}
    
    void operator()() {
        std::cout << std::format("함수 객체 스레드 {}: 시작\n", id);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << std::format("함수 객체 스레드 {}: 종료\n", id);
    }
};

// 멤버 함수를 스레드에서 실행하는 클래스
class ThreadTask {
private:
    int id;
    
public:
    ThreadTask(int id) : id(id) {}
    
    void task() {
        std::cout << std::format("멤버 함수 스레드 {}: 시작\n", id);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << std::format("멤버 함수 스레드 {}: 종료\n", id);
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "===== 스레드 생성 및 인수 전달 연습 =====\n";
    
    // 1. 일반 함수로 스레드 생성
    std::thread t1(threadFunction, 1);
    
    // 2. 여러 인수를 받는 함수로 스레드 생성
    std::thread t2(parameterizedFunction, 2, "테스트 스레드", true);
    
    // 3. 참조 전달 (std::ref 필요)
    std::vector<int> shared_data;
    std::thread t3(referenceFunction, 3, std::ref(shared_data));
    
    // 4. 함수 객체(Functor)로 스레드 생성
    ThreadFunctor functor(4);
    std::thread t4(functor);
    
    // 5. 람다 표현식으로 스레드 생성
    std::thread t5([](int id) {
        std::cout << std::format("람다 스레드 {}: 시작\n", id);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << std::format("람다 스레드 {}: 종료\n", id);
    }, 5);
    
    // 6. 클래스 멤버 함수로 스레드 생성
    ThreadTask task(6);
    std::thread t6(&ThreadTask::task, &task);
    
    // 모든 스레드 종료 대기
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    
    // 공유 데이터 확인
    std::cout << "공유 데이터 내용: ";
    for (int value : shared_data) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    std::cout << "모든 스레드가 종료되었습니다.\n";
    
    return 0;
}
```

## 실습: 스레드 실행 제어와 종료 기다리기 연습

스레드의 실행을 제어하고 안전하게 종료하는 방법을 연습해 봅시다.

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <format>

class WorkerThread {
private:
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> stopRequested{false};
    std::atomic<bool> pauseRequested{false};
    std::atomic<bool> isRunning{false};
    
public:
    WorkerThread(int id) {
        thread = std::thread([this, id]() {
            std::cout << std::format("작업자 스레드 {} 시작\n", id);
            isRunning = true;
            
            int count = 0;
            while (!stopRequested) {
                // 일시 중지 요청 처리
                if (pauseRequested) {
                    std::unique_lock<std::mutex> lock(mutex);
                    std::cout << std::format("작업자 스레드 {} 일시 중지됨\n", id);
                    
                    // 재개 신호나 종료 신호를 기다림
                    cv.wait(lock, [this]() {
                        return !pauseRequested || stopRequested;
                    });
                    
                    if (!pauseRequested) {
                        std::cout << std::format("작업자 스레드 {} 재개됨\n", id);
                    }
                }
                
                if (stopRequested) break;
                
                // 작업 시뮬레이션
                std::cout << std::format("작업자 스레드 {}: 카운트 {}\n", id, count++);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            
            std::cout << std::format("작업자 스레드 {} 종료\n", id);
            isRunning = false;
        });
    }
    
    ~WorkerThread() {
        if (thread.joinable()) {
            requestStop();
            thread.join();
        }
    }
    
    // 스레드 일시 중지
    void pause() {
        if (isRunning && !pauseRequested) {
            pauseRequested = true;
        }
    }
    
    // 스레드 재개
    void resume() {
        if (isRunning && pauseRequested) {
            pauseRequested = false;
            cv.notify_one();
        }
    }
    
    // 스레드 중지 요청
    void requestStop() {
        stopRequested = true;
        // 일시 중지 상태일 수 있으므로 조건 변수에 신호
        cv.notify_one();
    }
    
    // 스레드가 실행 중인지 확인
    bool running() const {
        return isRunning;
    }
    
    // 스레드가 일시 중지 상태인지 확인
    bool paused() const {
        return pauseRequested;
    }
    
    // 스레드 종료 대기
    void join() {
        if (thread.joinable()) {
            thread.join();
        }
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "===== 스레드 실행 제어 연습 =====\n";
    
    // 작업자 스레드 생성
    WorkerThread worker(1);
    
    // 잠시 실행
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 스레드 일시 중지
    std::cout << "메인: 스레드 일시 중지 요청\n";
    worker.pause();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 스레드 재개
    std::cout << "메인: 스레드 재개 요청\n";
    worker.resume();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 스레드 중지
    std::cout << "메인: 스레드 중지 요청\n";
    worker.requestStop();
    
    // 스레드 종료 대기
    std::cout << "메인: 스레드 종료 대기\n";
    worker.join();
    
    std::cout << "메인: 모든 작업 완료\n";
    
    return 0;
}
```
  

## 실습: 멀티스레드 TCP 서버 작성과 테스트
게임 서버와 유사한 구조의 고급 멀티스레드 TCP 서버를 구현해 봅시다. 이 서버는 각 클라이언트를 세션으로 관리하고, 간단한 명령어를 처리합니다.  
  
![게임 서버 스레드 구조](./images/042.png)   

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>
#include <sstream>
#include <chrono>
#include <atomic>
#include <memory>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <format>

#pragma comment(lib, "ws2_32.lib")

// 스레드 풀 클래스
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<int> activeThreads{0};

public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this, i] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        
                        this->condition.wait(lock, [this] { 
                            return this->stop || !this->tasks.empty(); 
                        });
                        
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    
                    activeThreads++;
                    task();
                    activeThreads--;
                }
            });
        }
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        
        condition.notify_all();
        
        for (std::thread &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) {
                throw std::runtime_error("스레드 풀 중단 후 작업 추가 시도");
            }
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
    
    size_t getTaskCount() {
        std::unique_lock<std::mutex> lock(queueMutex);
        return tasks.size();
    }
    
    int getActiveThreadCount() {
        return activeThreads;
    }
    
    size_t getThreadCount() {
        return workers.size();
    }
};

// 클라이언트 세션 클래스
class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    using Pointer = std::shared_ptr<ClientSession>;
    
private:
    SOCKET socket;
    std::string address;
    uint16_t port;
    std::atomic<bool> connected{false};
    
    std::vector<char> receiveBuffer;
    std::mutex sendMutex;
    
    // 세션에 붙은 플레이어 데이터 (실제 게임에서는 더 복잡할 것)
    struct PlayerData {
        std::string name;
        int x = 0;
        int y = 0;
        int hp = 100;
    } player;

public:
    ClientSession(SOCKET socket, const std::string& address, uint16_t port)
        : socket(socket), address(address), port(port), receiveBuffer(1024) {
        connected = true;
    }
    
    ~ClientSession() {
        close();
    }
    
    void start(ThreadPool& threadPool) {
        auto self = shared_from_this();
        threadPool.enqueue([self]() {
            self->readData();
        });
    }
    
    void close() {
        if (connected) {
            connected = false;
            closesocket(socket);
            socket = INVALID_SOCKET;
        }
    }
    
    bool isConnected() const {
        return connected;
    }
    
    std::string getAddress() const {
        return address;
    }
    
    uint16_t getPort() const {
        return port;
    }
    
    std::string getPlayerName() const {
        return player.name;
    }
    
    void sendData(const std::string& data) {
        if (!connected) return;
        
        std::lock_guard<std::mutex> lock(sendMutex);
        
        int totalSent = 0;
        int remaining = static_cast<int>(data.size());
        const char* buffer = data.c_str();
        
        while (totalSent < remaining) {
            int sent = send(socket, buffer + totalSent, remaining - totalSent, 0);
            if (sent == SOCKET_ERROR) {
                std::cerr << std::format("데이터 전송 실패: {}\n", WSAGetLastError());
                close();
                return;
            }
            totalSent += sent;
        }
    }

private:
    void readData() {
        if (!connected) return;
        
        int bytesReceived = recv(socket, receiveBuffer.data(), static_cast<int>(receiveBuffer.size()) - 1, 0);
        
        if (bytesReceived <= 0) {
            if (bytesReceived == 0) {
                std::cout << std::format("클라이언트 {}:{} 연결 종료\n", address, port);
            } else {
                std::cerr << std::format("데이터 수신 실패: {}\n", WSAGetLastError());
            }
            close();
            return;
        }
        
        receiveBuffer[bytesReceived] = '\0';
        std::string data(receiveBuffer.data(), bytesReceived);
        
        processCommand(data);
        
        if (connected) {
            auto self = shared_from_this();
            // 다시 읽기 작업 예약 (재귀적으로 호출하지 않고 예약)
            std::thread([self]() {
                self->readData();
            }).detach();
        }
    }
    
    void processCommand(const std::string& data) {
        std::istringstream iss(data);
        std::string command;
        iss >> command;
        
        if (command == "NAME") {
            std::string name;
            iss >> name;
            player.name = name;
            sendData(std::format("OK 이름이 {}(으)로 설정되었습니다.\n", name));
        }
        else if (command == "MOVE") {
            int x, y;
            iss >> x >> y;
            player.x = x;
            player.y = y;
            sendData(std::format("OK 위치가 ({}, {})로 이동했습니다.\n", x, y));
        }
        else if (command == "ATTACK") {
            std::string target;
            iss >> target;
            sendData(std::format("OK {}을(를) 공격했습니다.\n", target));
        }
        else if (command == "WHERE") {
            sendData(std::format("현재 위치: ({}, {})\n", player.x, player.y));
        }
        else if (command == "STATS") {
            sendData(std::format("플레이어: {}, 위치: ({}, {}), HP: {}\n", 
                             player.name, player.x, player.y, player.hp));
        }
        else if (command == "HELP") {
            sendData("사용 가능한 명령어:\n"
                     "NAME <이름> - 플레이어 이름 설정\n"
                     "MOVE <x> <y> - 지정된 좌표로 이동\n"
                     "ATTACK <대상> - 대상 공격\n"
                     "WHERE - 현재 위치 확인\n"
                     "STATS - 플레이어 상태 확인\n"
                     "HELP - 도움말 표시\n"
                     "QUIT - 접속 종료\n");
        }
        else if (command == "QUIT") {
            sendData("서버와의 연결을 종료합니다. 안녕히 가세요!\n");
            close();
        }
        else {
            sendData(std::format("알 수 없는 명령어: {}. 'HELP'를 입력하여 도움말을 확인하세요.\n", command));
        }
    }
};

// 게임 서버 클래스
class GameServer {
private:
    SOCKET listenSocket;
    ThreadPool threadPool;
    std::atomic<bool> running;
    
    std::map<SOCKET, ClientSession::Pointer> sessions;
    std::mutex sessionsMutex;
    
    std::thread maintenanceThread;

public:
    GameServer(size_t numThreads) 
        : threadPool(numThreads), running(false), listenSocket(INVALID_SOCKET) {}
    
    ~GameServer() {
        stop();
    }
    
    bool start(int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("바인딩 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << std::format("리슨 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        running = true;
        
        std::cout << std::format("게임 서버가 포트 {}에서 시작됨\n", port);
        
        // Accept 스레드 시작
        std::thread acceptThread(&GameServer::acceptClients, this);
        acceptThread.detach();
        
        // 유지보수 스레드 시작
        maintenanceThread = std::thread(&GameServer::maintenanceTask, this);
        
        return true;
    }
    
    void stop() {
        running = false;
        
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        
        // 모든 세션 종료
        {
            std::lock_guard<std::mutex> lock(sessionsMutex);
            for (auto& [_, session] : sessions) {
                session->close();
            }
            sessions.clear();
        }
        
        if (maintenanceThread.joinable()) {
            maintenanceThread.join();
        }
        
        WSACleanup();
        
        std::cout << "서버가 중지됨\n";
    }
    
    // 서버 상태 보고
    void printStatus() {
        size_t sessionCount;
        {
            std::lock_guard<std::mutex> lock(sessionsMutex);
            sessionCount = sessions.size();
        }
        
        std::cout << std::format("=== 서버 상태 ===\n");
        std::cout << std::format("활성 연결: {}\n", sessionCount);
        std::cout << std::format("스레드 풀: {}/{} 스레드 활성화, {} 작업 대기 중\n", 
                             threadPool.getActiveThreadCount(), 
                             threadPool.getThreadCount(),
                             threadPool.getTaskCount());
        
        // 연결된 클라이언트 정보 출력
        {
            std::lock_guard<std::mutex> lock(sessionsMutex);
            if (!sessions.empty()) {
                std::cout << "연결된 클라이언트:\n";
                for (const auto& [_, session] : sessions) {
                    std::cout << std::format("  {}:{} - {}\n", 
                                         session->getAddress(), 
                                         session->getPort(),
                                         session->getPlayerName().empty() ? "(이름 없음)" : session->getPlayerName());
                }
            }
        }
        
        std::cout << std::format("==================\n");
    }
    
private:
    void acceptClients() {
        while (running) {
            sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << std::format("클라이언트 연결 수락 실패: {}\n", WSAGetLastError());
                }
                continue;
            }
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            uint16_t clientPort = ntohs(clientAddr.sin_port);
            
            std::cout << std::format("새 클라이언트 연결: {}:{}\n", clientIP, clientPort);
            
            // 클라이언트 세션 생성 및 관리
            auto session = std::make_shared<ClientSession>(clientSocket, clientIP, clientPort);
            
            {
                std::lock_guard<std::mutex> lock(sessionsMutex);
                sessions[clientSocket] = session;
            }
            
            // 환영 메시지 전송
            session->sendData("게임 서버에 연결되었습니다. 'HELP'를 입력하여 사용 가능한 명령어를 확인하세요.\n");
            
            // 세션 처리 시작
            session->start(threadPool);
        }
    }
    
    void maintenanceTask() {
        while (running) {
            // 5초마다 끊어진 세션 정리
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            std::lock_guard<std::mutex> lock(sessionsMutex);
            for (auto it = sessions.begin(); it != sessions.end();) {
                if (!it->second->isConnected()) {
                    std::cout << std::format("세션 정리: {}:{}\n", 
                                         it->second->getAddress(), 
                                         it->second->getPort());
                    it = sessions.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
};

// 테스트용 클라이언트
class GameClient {
private:
    SOCKET socket;
    bool connected;
    std::thread receiveThread;
    std::atomic<bool> running;

public:
    GameClient() : socket(INVALID_SOCKET), connected(false), running(false) {}
    
    ~GameClient() {
        disconnect();
    }
    
    bool connect(const std::string& serverIP, int port = 27015) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup 실패\n";
            return false;
        }
        
        socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
        
        if (::connect(socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << std::format("서버 연결 실패: {}\n", WSAGetLastError());
            closesocket(socket);
            WSACleanup();
            return false;
        }
        
        connected = true;
        running = true;
        
        // 수신 스레드 시작
        receiveThread = std::thread(&GameClient::receiveData, this);
        
        std::cout << std::format("서버 {}:{}에 연결됨\n", serverIP, port);
        return true;
    }
    
    void disconnect() {
        running = false;
        
        if (connected) {
            closesocket(socket);
            socket = INVALID_SOCKET;
            connected = false;
        }
        
        if (receiveThread.joinable()) {
            receiveThread.join();
        }
        
        WSACleanup();
    }
    
    bool sendCommand(const std::string& command) {
        if (!connected) {
            std::cerr << "서버에 연결되어 있지 않음\n";
            return false;
        }
        
        int result = send(socket, command.c_str(), static_cast<int>(command.length()), 0);
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("명령 전송 실패: {}\n", WSAGetLastError());
            disconnect();
            return false;
        }
        
        return true;
    }
    
private:
    void receiveData() {
        std::vector<char> buffer(1024);
        
        while (running) {
            if (!connected) break;
            
            int bytesReceived = recv(socket, buffer.data(), static_cast<int>(buffer.size()) - 1, 0);
            
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {
                    std::cout << "서버가 연결을 종료함\n";
                } else {
                    std::cerr << std::format("데이터 수신 실패: {}\n", WSAGetLastError());
                }
                break;
            }
            
            buffer[bytesReceived] = '\0';
            std::cout << buffer.data();
        }
        
        connected = false;
    }
};

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "1: 서버 모드, 2: 클라이언트 모드 - 선택: ";
    int mode;
    std::cin >> mode;
    std::cin.ignore(); // 버퍼 비우기
    
    if (mode == 1) {
        // 하드웨어 스레드 수에 기반한 스레드 풀 크기 계산
        size_t numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 4; // 감지 실패 시 기본값
        
        std::cout << std::format("스레드 풀 크기: {}\n", numThreads);
        
        GameServer server(numThreads);
        if (server.start()) {
            std::cout << "서버가 시작되었습니다.\n";
            std::cout << "명령어 입력 (status: 상태 확인, exit: 종료): ";
            
            std::string command;
            while (true) {
                std::getline(std::cin, command);
                
                if (command == "exit") {
                    break;
                } else if (command == "status") {
                    server.printStatus();
                } else {
                    std::cout << "알 수 없는 명령어. 사용 가능: status, exit\n";
                }
                
                std::cout << "> ";
            }
            
            server.stop();
        }
    } else if (mode == 2) {
        GameClient client;
        std::string serverIP;
        
        std::cout << "서버 IP를 입력하세요: ";
        std::getline(std::cin, serverIP);
        
        if (client.connect(serverIP)) {
            std::cout << "서버에 연결되었습니다. 명령어를 입력하세요 (종료: QUIT):\n";
            
            std::string command;
            while (true) {
                std::cout << "> ";
                std::getline(std::cin, command);
                
                if (command == "QUIT") {
                    client.sendCommand(command);
                    break;
                }
                
                if (!client.sendCommand(command)) {
                    std::cout << "서버 연결이 끊겼습니다.\n";
                    break;
                }
                
                // 잠시 대기하여 응답 출력이 명령 입력보다 먼저 표시되도록 함
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            client.disconnect();
        }
    }
    
    return 0;
}
```

## 실습 임계 영역 연습

여러 스레드가 공유 데이터에 안전하게 접근하는 임계 영역(Critical Section) 처리 방법을 연습해 봅시다.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <chrono>
#include <atomic>
#include <format>

// 임계 영역이 없는 클래스 (스레드 안전하지 않음)
class UnsafeCounter {
private:
    int value = 0;
    
public:
    void increment() {
        ++value;  // 스레드 안전하지 않은 연산
    }
    
    int getValue() const {
        return value;
    }
};

// std::mutex를 사용하는 스레드 안전 클래스
class ThreadSafeCounter {
private:
    int value = 0;
    mutable std::mutex mutex;
    
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mutex);
        ++value;
    }
    
    int getValue() const {
        std::lock_guard<std::mutex> lock(mutex);
        return value;
    }
};

// std::atomic을 사용하는 스레드 안전 클래스
class AtomicCounter {
private:
    std::atomic<int> value{0};
    
public:
    void increment() {
        ++value;  // 원자적 연산
    }
    
    int getValue() const {
        return value.load();
    }
};

// 읽기-쓰기 락을 사용하는 스레드 안전 클래스
class RWLockCounter {
private:
    int value = 0;
    mutable std::shared_mutex rwMutex;
    
public:
    void increment() {
        // 쓰기 락 (독점적)
        std::unique_lock<std::shared_mutex> lock(rwMutex);
        ++value;
    }
    
    int getValue() const {
        // 읽기 락 (공유 가능)
        std::shared_lock<std::shared_mutex> lock(rwMutex);
        return value;
    }
};

template<typename Counter>
void testCounter(const std::string& counterName) {
    const int NUM_THREADS = 10;
    const int NUM_INCREMENTS = 100000;
    
    Counter counter;
    std::vector<std::thread> threads;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 여러 스레드에서 동시에 카운터 증가
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&counter, NUM_INCREMENTS]{
            for (int j = 0; j < NUM_INCREMENTS; ++j) {
                counter.increment();
            }
        });
    }
    
    // 모든 스레드 종료 대기
    for (auto& t : threads) {
        t.join();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // 결과 확인
    std::cout << std::format("{} 결과:\n", counterName);
    std::cout << std::format("  예상 값: {}\n", NUM_THREADS * NUM_INCREMENTS);
    std::cout << std::format("  실제 값: {}\n", counter.getValue());
    std::cout << std::format("  실행 시간: {} ms\n", duration.count());
    std::cout << std::endl;
}

// 데드락 시뮬레이션
void demonstrateDeadlock() {
    std::mutex mutex1, mutex2;
    
    auto thread1 = std::thread([&mutex1, &mutex2]{
        std::cout << "스레드 1: mutex1 잠금 시도\n";
        std::lock_guard<std::mutex> lock1(mutex1);
        std::cout << "스레드 1: mutex1 잠금 성공\n";
        
        // 약간의 지연 추가
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "스레드 1: mutex2 잠금 시도\n";
        std::lock_guard<std::mutex> lock2(mutex2);  // 데드락 가능성
        std::cout << "스레드 1: mutex2 잠금 성공\n";
        
        std::cout << "스레드 1: 작업 완료\n";
    });
    
    auto thread2 = std::thread([&mutex1, &mutex2]{
        std::cout << "스레드 2: mutex2 잠금 시도\n";
        std::lock_guard<std::mutex> lock2(mutex2);
        std::cout << "스레드 2: mutex2 잠금 성공\n";
        
        // 약간의 지연 추가
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "스레드 2: mutex1 잠금 시도\n";
        std::lock_guard<std::mutex> lock1(mutex1);  // 데드락 가능성
        std::cout << "스레드 2: mutex1 잠금 성공\n";
        
        std::cout << "스레드 2: 작업 완료\n";
    });
    
    // 스레드 분리하여 데드락 시뮬레이션 (실제로는 join 필요)
    thread1.detach();
    thread2.detach();
    
    // 데드락 시뮬레이션을 위한 대기
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "데드락이 발생했을 가능성이 있습니다.\n";
}

// 데드락 방지 시연
void demonstrateDeadlockPrevention() {
    std::mutex mutex1, mutex2;
    
    auto thread1 = std::thread([&mutex1, &mutex2]{
        std::cout << "스레드 1: 두 뮤텍스 잠금 시도 (안전하게)\n";
        
        // std::lock을 사용한 데드락 방지
        std::unique_lock<std::mutex> lock1(mutex1, std::defer_lock);
        std::unique_lock<std::mutex> lock2(mutex2, std::defer_lock);
        std::lock(lock1, lock2);  // 원자적으로 두 락 모두 획득
        
        std::cout << "스레드 1: 두 뮤텍스 잠금 성공\n";
        
        // 작업 시뮬레이션
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "스레드 1: 작업 완료\n";
        // 락은 스코프 종료 시 자동으로 해제됨
    });
    
    auto thread2 = std::thread([&mutex1, &mutex2]{
        std::cout << "스레드 2: 두 뮤텍스 잠금 시도 (안전하게)\n";
        
        // C++17의 scoped_lock 사용 (더 간단한 방법)
        std::scoped_lock lock(mutex1, mutex2);  // 원자적으로 모든 락 획득
        
        std::cout << "스레드 2: 두 뮤텍스 잠금 성공\n";
        
        // 작업 시뮬레이션
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "스레드 2: 작업 완료\n";
        // 락은 스코프 종료 시 자동으로 해제됨
    });
    
    thread1.join();
    thread2.join();
    
    std::cout << "두 스레드가 안전하게 완료되었습니다 (데드락 없음).\n";
}

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "===== 임계 영역 사용 연습 =====\n\n";
    
    // 각 카운터 클래스 테스트
    std::cout << "각 카운터 구현의 성능 및 정확성 비교:\n";
    testCounter<UnsafeCounter>("안전하지 않은 카운터");
    testCounter<ThreadSafeCounter>("mutex를 사용한 카운터");
    testCounter<AtomicCounter>("atomic을 사용한 카운터");
    testCounter<RWLockCounter>("읽기-쓰기 락을 사용한 카운터");
    
    std::cout << "데드락 시뮬레이션 (3초 후 중단됨):\n";
    demonstrateDeadlock();
    
    std::cout << "\n데드락 방지 시연:\n";
    demonstrateDeadlockPrevention();
    
    return 0;
}
```

## 실습: 이벤트 연습
스레드 간 이벤트 통지를 위한 `std::condition_variable`의 사용 방법을 연습해 봅시다. 생산자-소비자 패턴을 구현합니다.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <random>
#include <atomic>
#include <format>

// 스레드 안전한 큐
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable dataCondition;
    std::condition_variable spaceCondition;
    size_t capacity;
    
public:
    ThreadSafeQueue(size_t maxCapacity = SIZE_MAX) : capacity(maxCapacity) {}
    
    // 아이템 추가 (큐가 꽉 찬 경우 대기)
    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex);
        
        // 큐가 꽉 찼으면 공간이 생길 때까지 대기
        spaceCondition.wait(lock, [this]{ return queue.size() < capacity; });
        
        queue.push(std::move(item));
        
        // 대기 중인 소비자에게 신호
        dataCondition.notify_one();
    }
    
    // 아이템 가져오기 (큐가 비어있는 경우 대기)
    T pop() {
        std::unique_lock<std::mutex> lock(mutex);
        
        // 큐가 비어있으면 데이터가 들어올 때까지 대기
        dataCondition.wait(lock, [this]{ return !queue.empty(); });
        
        T item = std::move(queue.front());
        queue.pop();
        
        // 대기 중인 생산자에게 신호
        spaceCondition.notify_one();
        
        return item;
    }
    
    // 타임아웃이 있는 팝 메서드
    bool tryPopFor(T& item, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        
        // 큐가 비어있으면 타임아웃까지 데이터가 들어올 때까지 대기
        bool dataAvailable = dataCondition.wait_for(lock, timeout, [this]{ return !queue.empty(); });
        
        if (!dataAvailable) {
            return false;  // 타임아웃
        }
        
        item = std::move(queue.front());
        queue.pop();
        
        // 대기 중인 생산자에게 신호
        spaceCondition.notify_one();
        
        return true;
    }
    
    // 현재 큐 크기
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }
    
    // 비어있는지 확인
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }
};

// 이벤트 관리자 클래스
class EventManager {
private:
    std::mutex mutex;
    std::condition_variable eventCondition;
    std::atomic<bool> eventTriggered{false};
    
public:
    // 이벤트 대기
    void waitForEvent() {
        std::unique_lock<std::mutex> lock(mutex);
        eventCondition.wait(lock, [this]{ return eventTriggered.load(); });
    }
    
    // 타임아웃이 있는 이벤트 대기
    bool waitForEventWithTimeout(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return eventCondition.wait_for(lock, timeout, [this]{ return eventTriggered.load(); });
    }
    
    // 이벤트 발생 (하나의 대기 스레드에 신호)
    void triggerEvent() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            eventTriggered = true;
        }
        eventCondition.notify_one();
    }
    
    // 이벤트 발생 (모든 대기 스레드에 신호)
    void triggerEventForAll() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            eventTriggered = true;
        }
        eventCondition.notify_all();
    }
    
    // 이벤트 재설정
    void reset() {
        std::lock_guard<std::mutex> lock(mutex);
        eventTriggered = false;
    }
};

// 생산자-소비자 패턴 데모
void producerConsumerDemo() {
    const int MAX_QUEUE_SIZE = 5;           // 최대 큐 크기
    const int NUM_ITEMS = 20;               // 생산할 총 아이템 수
    const int NUM_CONSUMERS = 3;            // 소비자 스레드 수
    
    ThreadSafeQueue<int> queue(MAX_QUEUE_SIZE);
    std::atomic<int> itemsConsumed{0};
    std::atomic<bool> done{false};
    
    // 생산자 스레드
    std::thread producer([&queue, &done, NUM_ITEMS]() {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(100, 1000);
        
        for (int i = 0; i < NUM_ITEMS; ++i) {
            int sleepTime = dist(rng);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime / 5));
            
            int item = i + 1;
            std::cout << std::format("생산자: 아이템 {} 생산\n", item);
            queue.push(item);
        }
        
        std::cout << "생산자: 모든 아이템 생산 완료\n";
        done = true;
    });
    
    // 소비자 스레드들
    std::vector<std::thread> consumers;
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.emplace_back([&queue, &itemsConsumed, &done, i]() {
            std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<int> dist(200, 2000);
            
            while (!done || !queue.empty()) {
                int item;
                if (queue.tryPopFor(item, std::chrono::milliseconds(100))) {
                    int sleepTime = dist(rng);
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime / 2));
                    
                    std::cout << std::format("소비자 {}: 아이템 {} 소비\n", i, item);
                    itemsConsumed++;
                }
            }
            
            std::cout << std::format("소비자 {}: 종료\n", i);
        });
    }
    
    producer.join();
    for (auto& consumer : consumers) {
        consumer.join();
    }
    
    std::cout << std::format("생산자-소비자 데모 완료. 총 {} 아이템 처리됨.\n", itemsConsumed.load());
}

// 이벤트 기반 동기화 데모
void eventBasedSyncDemo() {
    EventManager eventManager;
    
    // 이벤트를 기다릴 스레드들
    std::vector<std::thread> waiters;
    for (int i = 0; i < 5; ++i) {
        waiters.emplace_back([&eventManager, i]() {
            std::cout << std::format("대기자 {}: 이벤트 대기 시작\n", i);
            
            if (i % 2 == 0) {
                // 일반 대기
                eventManager.waitForEvent();
                std::cout << std::format("대기자 {}: 이벤트 수신\n", i);
            } else {
                // 타임아웃 있는 대기
                auto timeout = std::chrono::seconds(10);
                bool received = eventManager.waitForEventWithTimeout(timeout);
                if (received) {
                    std::cout << std::format("대기자 {}: 이벤트 수신 (타임아웃 전)\n", i);
                } else {
                    std::cout << std::format("대기자 {}: 타임아웃 발생\n", i);
                }
            }
        });
    }
    
    // 이벤트 발생 전 잠시 대기
    std::cout << "메인 스레드: 3초 후 이벤트 발생 예정\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 모든 대기자에게 이벤트 발생
    std::cout << "메인 스레드: 이벤트 발생!\n";
    eventManager.triggerEventForAll();
    
    // 모든 스레드 종료 대기
    for (auto& waiter : waiters) {
        waiter.join();
    }
    
    std::cout << "이벤트 기반 동기화 데모 완료.\n";
}

int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    std::cout << "===== 이벤트 및 조건 변수 연습 =====\n\n";
    
    std::cout << "1. 생산자-소비자 패턴 데모\n";
    producerConsumerDemo();
    
    std::cout << "\n2. 이벤트 기반 동기화 데모\n";
    eventBasedSyncDemo();
    
    return 0;
}
```

이 코드 예제들을 통해 Windows 환경에서의 멀티스레드 프로그래밍과 동기화 기법을 효과적으로 학습할 수 있습니다. 온라인 게임 서버 개발자가 되기 위해 이러한 멀티스레딩 개념은 매우 중요하며, 실제 게임 서버에서는 여기서 다룬 기법들이 더 복잡하게 조합되어 사용됩니다.  
  
    
<br>      
  
# Chapter.07 UDP 네트워크 프로그래밍
  
## 01. UDP 특징
UDP(User Datagram Protocol)는 TCP와 달리 연결 지향적이지 않은 프로토콜로, 온라인 게임에서 실시간성이 중요한 데이터 전송에 자주 사용됩니다.

### UDP의 특징
- **비연결성**: 연결 설정 과정 없이 바로 데이터 전송
- **비신뢰성**: 패킷 전달 보장 없음, 순서 보장 없음
- **낮은 오버헤드**: TCP보다 헤더 크기가 작고 처리 과정이 단순함
- **빠른 속도**: 연결 관리, 흐름 제어, 혼잡 제어 없음
  
![](./images/201.png)  
  

### UDP 서버-클라이언트 기본 구조
![](./images/202.png)   
![](./images/203.png)  
![](./images/204.png)   
  
1. **서버 측**:
   - 소켓 생성 (SOCK_DGRAM)
   - 특정 포트에 바인딩
   - 클라이언트로부터 데이터 수신
   - 클라이언트에게 데이터 전송

2. **클라이언트 측**:
   - 소켓 생성 (SOCK_DGRAM)
   - 서버에게 데이터 전송
   - 서버로부터 데이터 수신 

### UDP의 게임 서버 활용
- 실시간 위치 정보, 플레이어 동작 등 지연이 중요한 데이터
- 음성 채팅 같은 실시간 미디어 전송
- 패킷 손실이 있어도 게임 플레이에 큰 영향이 없는 정보
  

## 02. UDP 프로그래밍

### Winsock2 초기화
Windows에서 네트워크 프로그래밍을 하기 위해서는 Winsock 라이브러리를 초기화해야 합니다.

```cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <format>

#pragma comment(lib, "ws2_32.lib")

bool InitializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << std::format("WSAStartup 실패: {}\n", result);
        return false;
    }
    return true;
}
```

### UDP 소켓 생성
UDP 소켓은 SOCK_DGRAM 타입으로 생성합니다.

```cpp
SOCKET CreateUdpSocket() {
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }
    return udpSocket;
}
```

### 서버 소켓 바인딩
UDP 서버는 특정 주소와 포트에 바인딩되어야 합니다.

```cpp
bool BindSocket(SOCKET socket, const char* ip, unsigned short port) {
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    // IP 주소 변환
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);
    
    // 소켓을 주소와 포트에 바인딩
    int result = bind(socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << std::format("바인드 실패: {}\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return false;
    }
    
    return true;
}
```

### 데이터 송수신
UDP에서는 `sendto()`, `recvfrom()` 함수를 사용하여 데이터를 송수신합니다.

```cpp
// 데이터 전송
bool SendData(SOCKET socket, const char* data, int dataLength, const sockaddr_in& destination) {
    int bytesSent = sendto(socket, data, dataLength, 0, 
                         reinterpret_cast<const sockaddr*>(&destination), 
                         sizeof(destination));
    
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << std::format("sendto 실패: {}\n", WSAGetLastError());
        return false;
    }
    
    return true;
}

// 데이터 수신
int ReceiveData(SOCKET socket, char* buffer, int bufferSize, sockaddr_in& sender) {
    int senderAddrSize = sizeof(sender);
    int bytesReceived = recvfrom(socket, buffer, bufferSize, 0,
                               reinterpret_cast<sockaddr*>(&sender),
                               &senderAddrSize);
    
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << std::format("recvfrom 실패: {}\n", WSAGetLastError());
        return -1;
    }
    
    return bytesReceived;
}
```
  

## 03. UDP IPv6
IPv6는 128비트 주소 체계로, 기존 IPv4의 주소 고갈 문제를 해결하기 위해 개발되었습니다.

### IPv6 소켓 생성 및 바인딩

```cpp
SOCKET CreateUdpSocketIPv6() {
    SOCKET udpSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << std::format("IPv6 소켓 생성 실패: {}\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }
    return udpSocket;
}

bool BindSocketIPv6(SOCKET socket, const char* ip, unsigned short port) {
    sockaddr_in6 serverAddr{};
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_port = htons(port);
    
    // IPv6 주소 변환
    inet_pton(AF_INET6, ip, &serverAddr.sin6_addr);
    
    // 소켓을 주소와 포트에 바인딩
    int result = bind(socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << std::format("IPv6 바인드 실패: {}\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return false;
    }
    
    return true;
}
```

### IPv6 데이터 송수신

```cpp
// IPv6 데이터 전송
bool SendDataIPv6(SOCKET socket, const char* data, int dataLength, const sockaddr_in6& destination) {
    int bytesSent = sendto(socket, data, dataLength, 0, 
                         reinterpret_cast<const sockaddr*>(&destination), 
                         sizeof(destination));
    
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << std::format("IPv6 sendto 실패: {}\n", WSAGetLastError());
        return false;
    }
    
    return true;
}

// IPv6 데이터 수신
int ReceiveDataIPv6(SOCKET socket, char* buffer, int bufferSize, sockaddr_in6& sender) {
    int senderAddrSize = sizeof(sender);
    int bytesReceived = recvfrom(socket, buffer, bufferSize, 0,
                               reinterpret_cast<sockaddr*>(&sender),
                               &senderAddrSize);
    
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << std::format("IPv6 recvfrom 실패: {}\n", WSAGetLastError());
        return -1;
    }
    
    return bytesReceived;
}
```
  

## 실습: UDP 서버-클라이언트 작성과 테스트

### UDP 서버 구현  
![](./images/205.png)  
![](./images/206.png)  
![](./images/207.png)  
![](./images/208.png)  
![](./images/209.png)  

```cpp
// UdpServer.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <format>
#include <thread>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

std::atomic<bool> g_running = true;

int main() {
    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패\n";
        return 1;
    }
    
    // UDP 소켓 생성
    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // 서버 주소 설정 및 바인딩
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);  // 포트 9000 사용
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // 모든 인터페이스에서 수신
    
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << std::format("바인드 실패: {}\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "UDP 서버가 시작되었습니다. 포트 9000에서 대기 중...\n";
    
    // 클라이언트로부터 메시지 수신 및 응답
    char buffer[1024];
    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    
    // 별도 스레드에서 콘솔 입력을 처리하여 서버를 종료할 수 있게 함
    std::thread inputThread([]() {
        std::string input;
        while (g_running) {
            std::getline(std::cin, input);
            if (input == "quit" || input == "exit") {
                g_running = false;
                break;
            }
        }
    });
    
    // 비동기 모드로 설정
    u_long nonBlocking = 1;
    ioctlsocket(serverSocket, FIONBIO, &nonBlocking);
    
    while (g_running) {
        // 데이터 수신 시도
        int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer) - 1, 0,
                                  reinterpret_cast<sockaddr*>(&clientAddr), 
                                  &clientAddrSize);
        
        if (bytesReceived > 0) {
            // 수신한 메시지 처리
            buffer[bytesReceived] = '\0';
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            
            std::cout << std::format("클라이언트({}:{})로부터 메시지 수신: {}\n", 
                                     clientIP, ntohs(clientAddr.sin_port), buffer);
            
            // 응답 메시지
            std::string response = std::format("메시지 '{}'를 수신했습니다.", buffer);
            
            // 클라이언트에게 응답 전송
            int bytesSent = sendto(serverSocket, response.c_str(), response.length(), 0,
                                reinterpret_cast<sockaddr*>(&clientAddr), 
                                clientAddrSize);
            
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("sendto 실패: {}\n", WSAGetLastError());
            }
        } else if (bytesReceived == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                std::cerr << std::format("recvfrom 실패: {}\n", error);
            }
        }
        
        // CPU 사용률을 낮추기 위한 짧은 지연
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    inputThread.join();
    closesocket(serverSocket);
    WSACleanup();
    std::cout << "서버가 종료되었습니다.\n";
    
    return 0;
}
```

### UDP 클라이언트 구현

```cpp
// UdpClient.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <format>

#pragma comment(lib, "ws2_32.lib")

int main() {
    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패\n";
        return 1;
    }
    
    // UDP 소켓 생성
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // 서버 주소 설정
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);  // 서버 포트 9000
    
    // 서버 IP 주소 설정 (로컬호스트)
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    
    std::cout << "UDP 클라이언트가 시작되었습니다. 서버(127.0.0.1:9000)에 연결 중...\n";
    std::cout << "메시지를 입력하세요 ('quit' 또는 'exit'를 입력하면 종료):\n";
    
    std::string message;
    char buffer[1024];
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);
        
        if (message == "quit" || message == "exit") {
            break;
        }
        
        // 서버로 메시지 전송
        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0,
                            reinterpret_cast<sockaddr*>(&serverAddr), 
                            sizeof(serverAddr));
        
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << std::format("sendto 실패: {}\n", WSAGetLastError());
            continue;
        }
        
        // 서버로부터 응답 수신
        sockaddr_in fromAddr{};
        int fromAddrSize = sizeof(fromAddr);
        
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer) - 1, 0,
                                   reinterpret_cast<sockaddr*>(&fromAddr), 
                                   &fromAddrSize);
        
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << std::format("recvfrom 실패: {}\n", WSAGetLastError());
            continue;
        }
        
        buffer[bytesReceived] = '\0';
        std::cout << "서버 응답: " << buffer << "\n";
    }
    
    closesocket(clientSocket);
    WSACleanup();
    std::cout << "클라이언트가 종료되었습니다.\n";
    
    return 0;
}
```

### 코드 컴파일 및 실행 방법
1. Visual Studio에서 두 개의 별도 프로젝트를 생성합니다.
2. 서버 코드와 클라이언트 코드를 각 프로젝트에 추가합니다.
3. 컴파일하고 실행합니다.
4. 먼저 서버를 실행한 다음 클라이언트를 실행합니다.
5. 클라이언트에서 메시지를 입력하면 서버로 전송되고 서버에서 응답을 받습니다.
  

## 실습: UDP 서버-클라이언트(IPv6) 작성과 테스트

### IPv6 UDP 서버 구현

```cpp
// UdpServerIPv6.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <format>
#include <thread>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

std::atomic<bool> g_running = true;

int main() {
    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패\n";
        return 1;
    }
    
    // IPv6 UDP 소켓 생성
    SOCKET serverSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // IPv4-매핑된 IPv6 주소를 허용하지 않음 (순수 IPv6 모드)
    // 이 옵션을 변경하면 IPv4 클라이언트도 접속 가능
    DWORD v6Only = 1;
    if (setsockopt(serverSocket, IPPROTO_IPV6, IPV6_V6ONLY, 
                   reinterpret_cast<char*>(&v6Only), sizeof(v6Only)) == SOCKET_ERROR) {
        std::cerr << std::format("setsockopt IPV6_V6ONLY 실패: {}\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    // 서버 주소 설정 및 바인딩
    sockaddr_in6 serverAddr{};
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_port = htons(9000);  // 포트 9000 사용
    serverAddr.sin6_addr = in6addr_any;  // 모든 인터페이스에서 수신
    
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << std::format("바인드 실패: {}\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "IPv6 UDP 서버가 시작되었습니다. 포트 9000에서 대기 중...\n";
    
    // 클라이언트로부터 메시지 수신 및 응답
    char buffer[1024];
    sockaddr_in6 clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    
    // 별도 스레드에서 콘솔 입력을 처리하여 서버를 종료할 수 있게 함
    std::thread inputThread([]() {
        std::string input;
        while (g_running) {
            std::getline(std::cin, input);
            if (input == "quit" || input == "exit") {
                g_running = false;
                break;
            }
        }
    });
    
    // 비동기 모드로 설정
    u_long nonBlocking = 1;
    ioctlsocket(serverSocket, FIONBIO, &nonBlocking);
    
    while (g_running) {
        // 데이터 수신 시도
        int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer) - 1, 0,
                                  reinterpret_cast<sockaddr*>(&clientAddr), 
                                  &clientAddrSize);
        
        if (bytesReceived > 0) {
            // 수신한 메시지 처리
            buffer[bytesReceived] = '\0';
            
            char clientIP[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &clientAddr.sin6_addr, clientIP, INET6_ADDRSTRLEN);
            
            std::cout << std::format("클라이언트([{}]:{})로부터 메시지 수신: {}\n", 
                                     clientIP, ntohs(clientAddr.sin6_port), buffer);
            
            // 응답 메시지
            std::string response = std::format("IPv6 메시지 '{}'를 수신했습니다.", buffer);
            
            // 클라이언트에게 응답 전송
            int bytesSent = sendto(serverSocket, response.c_str(), response.length(), 0,
                                reinterpret_cast<sockaddr*>(&clientAddr), 
                                clientAddrSize);
            
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << std::format("sendto 실패: {}\n", WSAGetLastError());
            }
        } else if (bytesReceived == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                std::cerr << std::format("recvfrom 실패: {}\n", error);
            }
        }
        
        // CPU 사용률을 낮추기 위한 짧은 지연
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    inputThread.join();
    closesocket(serverSocket);
    WSACleanup();
    std::cout << "서버가 종료되었습니다.\n";
    
    return 0;
}
```

### IPv6 UDP 클라이언트 구현

```cpp
// UdpClientIPv6.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <format>

#pragma comment(lib, "ws2_32.lib")

int main() {
    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패\n";
        return 1;
    }
    
    // IPv6 UDP 소켓 생성
    SOCKET clientSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // 서버 주소 설정
    sockaddr_in6 serverAddr{};
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_port = htons(9000);  // 서버 포트 9000
    
    // 서버 IPv6 주소 설정 (로컬호스트 - ::1)
    inet_pton(AF_INET6, "::1", &serverAddr.sin6_addr);
    
    std::cout << "IPv6 UDP 클라이언트가 시작되었습니다. 서버([::1]:9000)에 연결 중...\n";
    std::cout << "메시지를 입력하세요 ('quit' 또는 'exit'를 입력하면 종료):\n";
    
    std::string message;
    char buffer[1024];
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);
        
        if (message == "quit" || message == "exit") {
            break;
        }
        
        // 서버로 메시지 전송
        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0,
                            reinterpret_cast<sockaddr*>(&serverAddr), 
                            sizeof(serverAddr));
        
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << std::format("sendto 실패: {}\n", WSAGetLastError());
            continue;
        }
        
        // 서버로부터 응답 수신
        sockaddr_in6 fromAddr{};
        int fromAddrSize = sizeof(fromAddr);
        
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer) - 1, 0,
                                   reinterpret_cast<sockaddr*>(&fromAddr), 
                                   &fromAddrSize);
        
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << std::format("recvfrom 실패: {}\n", WSAGetLastError());
            continue;
        }
        
        buffer[bytesReceived] = '\0';
        std::cout << "서버 응답: " << buffer << "\n";
    }
    
    closesocket(clientSocket);
    WSACleanup();
    std::cout << "클라이언트가 종료되었습니다.\n";
    
    return 0;
}
```

### IPv6 코드 실행 방법
1. IPv6가 활성화된 시스템에서 코드를 컴파일하고 실행합니다.
2. 먼저 IPv6 서버를 실행한 다음 IPv6 클라이언트를 실행합니다.
3. `::1`은 IPv6 루프백 주소로, IPv4의 127.0.0.1과 동일한 역할을 합니다.

### IPv6 테스트 시 주의 사항
1. Windows에서 IPv6가 활성화되어 있는지 확인하세요.
2. 방화벽 설정이 IPv6 트래픽을 허용하는지 확인하세요.
3. 네트워크 어댑터에서 IPv6 프로토콜이 활성화되어 있는지 확인하세요.
  

-----  
이 UDP 서버-클라이언트 코드는 온라인 게임 서버 개발을 위한 기초적인 구현입니다. 실제 게임 서버 개발 시에는 다음과 같은 기능을 추가로 구현해야 할 수 있습니다:

1. 패킷 손실 처리 및 재전송 메커니즘
2. 패킷 순서 보장 시스템
3. 패킷 세그먼테이션 및 재조립
4. 패킷 인증 및 암호화
5. 게임 로직과의 통합
6. 성능 최적화 및 부하 테스트

UDP는 빠른 속도와 낮은 오버헤드로 실시간 게임에 적합하지만, 신뢰성이 떨어지는 점은 애플리케이션 레벨에서 보완해야 합니다.   
  

<br>      
   
# Chapter.08 소켓 옵션
   
## 01. 소켓 옵션의 종류와 관련 함수
소켓 옵션은 소켓의 동작 방식을 세부적으로 제어할 수 있게 해주는 매개변수들입니다. 온라인 게임 서버 개발 시 이러한 옵션들을 적절히 설정하면 성능 향상과 안정성을 높일 수 있습니다.  

### 소켓 옵션 설정/조회 함수
Windows에서는 다음 두 함수를 통해 소켓 옵션을 설정하고 조회할 수 있습니다:

```cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <format>

#pragma comment(lib, "ws2_32.lib")

// 소켓 옵션 설정 함수
int setsockopt(
    SOCKET s,               // 대상 소켓
    int level,              // 옵션 레벨
    int optname,            // 옵션 이름
    const char* optval,     // 옵션 값
    int optlen              // 옵션 값의 크기
);

// 소켓 옵션 조회 함수
int getsockopt(
    SOCKET s,               // 대상 소켓
    int level,              // 옵션 레벨
    int optname,            // 옵션 이름
    char* optval,           // 옵션 값을 저장할 버퍼
    int* optlen             // 버퍼 크기 및 반환된 옵션 값 크기
);
```

### 소켓 옵션 레벨
소켓 옵션은 다양한 프로토콜 레벨에서 적용됩니다:

1. **SOL_SOCKET**: 소켓 자체에 적용되는 일반적인 옵션
2. **IPPROTO_IP**: IPv4 프로토콜에 적용되는 옵션
3. **IPPROTO_IPV6**: IPv6 프로토콜에 적용되는 옵션
4. **IPPROTO_TCP**: TCP 프로토콜에 적용되는 옵션
5. **IPPROTO_UDP**: UDP 프로토콜에 적용되는 옵션

### 소켓 옵션 헬퍼 함수
C++23에서는 템플릿을 활용하여 소켓 옵션 설정을 편리하게 할 수 있는 헬퍼 함수를 만들 수 있습니다:

```cpp
template <typename T>
bool SetSockOpt(SOCKET sock, int level, int optname, const T& value) {
    return setsockopt(sock, level, optname, 
                     reinterpret_cast<const char*>(&value), 
                     sizeof(T)) != SOCKET_ERROR;
}

template <typename T>
bool GetSockOpt(SOCKET sock, int level, int optname, T& value) {
    int len = sizeof(T);
    return getsockopt(sock, level, optname, 
                     reinterpret_cast<char*>(&value), 
                     &len) != SOCKET_ERROR;
}
```

![Socket Options 템플릿 함수 사용 예시](./images/043.png)       
  
  
## 02. SOL_SOCKET 레벨 옵션
SOL_SOCKET 레벨 옵션은 모든 소켓 유형에 적용되는 일반적인 옵션입니다.

### SO_REUSEADDR
서버 재시작 시 이전에 사용했던 주소와 포트를 즉시 재사용할 수 있게 합니다.

```cpp
BOOL optVal = TRUE;
if (!SetSockOpt(serverSocket, SOL_SOCKET, SO_REUSEADDR, optVal)) {
    std::cerr << std::format("SO_REUSEADDR 설정 실패: {}\n", WSAGetLastError());
}
```
  
**게임 서버에서는 서버 다운 후 빠른 재시작을 위해 중요합니다.**
  
### SO_RCVBUF와 SO_SNDBUF
수신 및 송신 버퍼의 크기를 설정합니다.

```cpp
int recvBufSize = 64 * 1024; // 64KB
int sendBufSize = 64 * 1024; // 64KB

if (!SetSockOpt(gameSocket, SOL_SOCKET, SO_RCVBUF, recvBufSize)) {
    std::cerr << std::format("SO_RCVBUF 설정 실패: {}\n", WSAGetLastError());
}

if (!SetSockOpt(gameSocket, SOL_SOCKET, SO_SNDBUF, sendBufSize)) {
    std::cerr << std::format("SO_SNDBUF 설정 실패: {}\n", WSAGetLastError());
}
```

게임 서버에서는 대량의 데이터 처리를 위해 버퍼 크기를 조정하는 것이 중요합니다.  

그러나 **최신 OS에서는 소켓 버퍼 크기가 자동으로 조정된다**. 이를 **TCP 자동 튜닝(Auto-tuning)**이라고 한다.

#### 주요 특징
**Linux 커널 2.6.17 이후**: 기본적으로 4MB 최대 버퍼 크기를 가진 전체 자동 튜닝 기능이 제공된다. /proc/sys/net/ipv4/tcp_moderate_rcvbuf가 1로 설정되면 자동 튜닝이 활성화되고 버퍼 크기가 동적으로 조정된다.  
**Windows Vista 이후**: Microsoft도 수신측 버퍼 자동 튜닝을 도입했다.  
**FreeBSD 7.0**: 버퍼 자동 튜닝 기능을 포함하여 출시되었다.  
  
#### 동작 원리
![](./images/046.png)   
![](./images/047.png)   
  
**Linux의 경우**: 커널이 로컬 애플리케이션이 수신 큐에서 데이터를 읽는 속도를 추적하고 세션 RTT도 알고 있어서, 애플리케이션 레이어나 네트워크 병목 링크가 처리량의 제약이 될 때까지 자동으로 버퍼와 수신 윈도우를 증가시킨다.  
**FreeBSD의 경우**: 작은 버퍼로 시작해서 TCP 혼잡 윈도우와 병렬로 빠르게 성장시켜 실제 네트워크 조건에 맞춘다.  
**Windows의 경우**: [Windows TCP 자동 튜닝 설정 가이드](https://claude.ai/public/artifacts/624c04c3-96df-491b-aeb1-00ec290c0a47 )  
  

#### 실제 예시 코드

```c
// 자동 튜닝 상태 확인 (Linux)
#include <sys/socket.h>
#include <netinet/tcp.h>

int check_autotuning() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 현재 수신 버퍼 크기 확인
    int rcvbuf_size;
    socklen_t len = sizeof(rcvbuf_size);
    getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, &len);
    
    printf("Current receive buffer: %d bytes\n", rcvbuf_size);
    
    // 자동 튜닝이 동작 중이면 이 값이 동적으로 변한다
    return rcvbuf_size;
}
```

#### 성능 효과
기본 시스템 설정 대비 큰 대역폭 지연 경로에서 성능이 대폭 개선되고, 수동 튜닝된 연결 대비 메모리 사용량이 현저히 줄어들어 서버가 최소 2배 더 많은 동시 연결을 지원할 수 있다.  
대부분의 최신 OS들이 TCP 버퍼를 잘 자동 튜닝하지만, 10G 네트워크의 경우 기본 최대 TCP 버퍼 크기가 여전히 작을 수 있다는 점은 유의해야 한다.  
  

### SO_LINGER
소켓 종료 시 전송 대기 중인 데이터 처리 방식을 설정합니다.

```cpp
linger lingerOpt{};
lingerOpt.l_onoff = 1;    // linger 사용
lingerOpt.l_linger = 10;  // 10초 동안 대기

if (!SetSockOpt(clientSocket, SOL_SOCKET, SO_LINGER, lingerOpt)) {
    std::cerr << std::format("SO_LINGER 설정 실패: {}\n", WSAGetLastError());
}
```

**게임 서버에서는 클라이언트 연결 종료 시 미전송 데이터를 어떻게 처리할지 결정하는 데 사용됩니다.**

### SO_KEEPALIVE
TCP 연결이 여전히 활성 상태인지 주기적으로 확인합니다.

```cpp
BOOL keepAlive = TRUE;
if (!SetSockOpt(playerSocket, SOL_SOCKET, SO_KEEPALIVE, keepAlive)) {
    std::cerr << std::format("SO_KEEPALIVE 설정 실패: {}\n", WSAGetLastError());
}
```

게임 서버에서는 플레이어 연결이 갑자기 끊겼는지 감지하는 데 유용합니다. (잘 사용하지 않음)    
![게임 서버: Heartbeat vs TCP Keepalive](./images/044.png) 
    
  
### SO_BROADCAST
UDP 소켓에서 브로드캐스트 메시지 전송을 허용합니다.

```cpp
BOOL broadcast = TRUE;
if (!SetSockOpt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, broadcast)) {
    std::cerr << std::format("SO_BROADCAST 설정 실패: {}\n", WSAGetLastError());
}
```

  
### SO_ERROR
소켓의 마지막 에러 코드를 가져옵니다.

```cpp
int errorCode;
if (!GetSockOpt(socket, SOL_SOCKET, SO_ERROR, errorCode)) {
    std::cerr << std::format("SO_ERROR 조회 실패: {}\n", WSAGetLastError());
} else {
    if (errorCode != 0) {
        std::cerr << std::format("소켓 에러 발생: {}\n", errorCode);
    }
}
```

비동기 작업 후 오류 확인에 유용합니다.

### SO_DONTLINGER
SO_LINGER의 반대 동작을 설정합니다.

```cpp
BOOL dontLinger = TRUE;
if (!SetSockOpt(socket, SOL_SOCKET, SO_DONTLINGER, dontLinger)) {
    std::cerr << std::format("SO_DONTLINGER 설정 실패: {}\n", WSAGetLastError());
}
```

closesocket() 호출 즉시 반환되게 하여 게임 종료 시 지연이 없도록 합니다.
  

## 03. IPPROTO_IP, IPPROTO_IPV6 레벨 옵션
이 레벨 옵션들은 IP 프로토콜 관련 설정을 제어합니다.

### IP_TTL (Time To Live)
패킷의 생존 시간을 설정합니다.

```cpp
int ttl = 64;  // 홉 수
if (!SetSockOpt(socket, IPPROTO_IP, IP_TTL, ttl)) {
    std::cerr << std::format("IP_TTL 설정 실패: {}\n", WSAGetLastError());
}
```

게임 서버에서는 패킷이 네트워크에서 너무 오래 떠돌지 않도록 제한하는 데 사용됩니다.
  
### IP_MULTICAST_TTL
멀티캐스트 패킷의 TTL 값을 설정합니다.

```cpp
int multicastTTL = 8;  // 멀티캐스트 범위 제한
if (!SetSockOpt(multicastSocket, IPPROTO_IP, IP_MULTICAST_TTL, multicastTTL)) {
    std::cerr << std::format("IP_MULTICAST_TTL 설정 실패: {}\n", WSAGetLastError());
}
```

온라인 게임에서 로컬 네트워크 내의 멀티캐스트를 제어하는 데 유용합니다.

### IP_MULTICAST_LOOP
멀티캐스트 패킷이 송신자에게도 수신되는지 여부를 설정합니다.

```cpp
BOOL loopback = FALSE;  // 송신자에게 수신되지 않음
if (!SetSockOpt(multicastSocket, IPPROTO_IP, IP_MULTICAST_LOOP, loopback)) {
    std::cerr << std::format("IP_MULTICAST_LOOP 설정 실패: {}\n", WSAGetLastError());
}
```

게임에서 자신이 보낸 메시지는 받지 않도록 설정하는 데 사용됩니다.

### IP_ADD_MEMBERSHIP / IP_DROP_MEMBERSHIP
멀티캐스트 그룹에 가입하거나 탈퇴합니다.

```cpp
ip_mreq multicastRequest{};
inet_pton(AF_INET, "239.255.0.1", &multicastRequest.imr_multiaddr);
multicastRequest.imr_interface.s_addr = INADDR_ANY;

if (!SetSockOpt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, multicastRequest)) {
    std::cerr << std::format("IP_ADD_MEMBERSHIP 설정 실패: {}\n", WSAGetLastError());
}
```

게임 방 검색이나 지역 기반 게임 이벤트 알림 등에 활용됩니다.

### IPV6_V6ONLY
IPv6 소켓에서 IPv4 매핑된 주소를 허용할지 설정합니다.

```cpp
BOOL v6Only = TRUE;
if (!SetSockOpt(socket, IPPROTO_IPV6, IPV6_V6ONLY, v6Only)) {
    std::cerr << std::format("IPV6_V6ONLY 설정 실패: {}\n", WSAGetLastError());
}
```

IPv6 전용 게임 서버 구현 시 사용됩니다.
  

## 04. IPPROTO_TCP 레벨 옵션
TCP 프로토콜 관련 설정을 제어하는 옵션들입니다.

### TCP_NODELAY (Nagle 알고리즘 비활성화)
가장 중요한 TCP 옵션 중 하나로, 작은 패킷들을 모아서 전송하는 Nagle 알고리즘을 비활성화합니다.

```cpp
BOOL noDelay = TRUE;
if (!SetSockOpt(playerSocket, IPPROTO_TCP, TCP_NODELAY, noDelay)) {
    std::cerr << std::format("TCP_NODELAY 설정 실패: {}\n", WSAGetLastError());
}
```

**게임 서버에서 매우 중요**: 아주 빠른 실시간 게임(예, FPS)에서는 패킷 지연이 게임 경험에 직접적인 영향을 미치므로, 대부분의 게임 서버에서는 이 옵션을 활성화하여 지연을 최소화합니다.  
   
![온라인 게임에서 TCP_NODELAY 사용 케이스](./images/045.png)   
완료했다! 수정된 다이어그램에서 핵심 포인트는:

#### **턴제 게임에서 TCP_NODELAY가 불필요한 이유**

**Nagle 알고리즘 대기 조건:**
1. 보낼 데이터가 MSS(Maximum Segment Size)보다 작음
2. 아직 ACK를 받지 못한 데이터가 존재

**턴제 게임의 특별함:**
- **요청-응답 패턴**: 플레이어가 수를 두면 → 서버가 응답 → 다음 요청
- 서버 응답이 이전 요청에 대한 **ACK 역할**을 함
- 따라서 다음 요청을 보낼 때는 이미 ACK를 받은 상태
- **Nagle 대기 조건이 충족되지 않아** 즉시 전송됨

**실제 시나리오 비교:**
- **FPS 게임**: MOVE → SHOOT → AIM → MOVE (연속 전송, 응답 대기 안함) → **지연 발생**
- **바둑 게임**: MOVE → (서버 응답 대기) → MOVE → (서버 응답 대기) → **지연 없음**

결론적으로 턴제 게임은 통신 패턴 자체가 Nagle 알고리즘과 충돌하지 않기 때문에 TCP_NODELAY를 설정할 필요가 없고, 오히려 불필요한 네트워크 오버헤드만 증가시킬 수 있다.
  

### TCP_KEEPIDLE
TCP keepalive 패킷을 보내기 전에 대기하는 시간을 설정합니다.

```cpp
DWORD keepIdle = 60; // 60초
if (!SetSockOpt(clientSocket, IPPROTO_TCP, TCP_KEEPIDLE, keepIdle)) {
    std::cerr << std::format("TCP_KEEPIDLE 설정 실패: {}\n", WSAGetLastError());
}
```

게임 서버에서는 비활성 클라이언트 검출 타이밍을 조정하는 데 사용됩니다.

### TCP_KEEPINTVL
keepalive 프로브 재전송 간격을 설정합니다.

```cpp
DWORD keepInterval = 5; // 5초
if (!SetSockOpt(clientSocket, IPPROTO_TCP, TCP_KEEPINTVL, keepInterval)) {
    std::cerr << std::format("TCP_KEEPINTVL 설정 실패: {}\n", WSAGetLastError());
}
```

클라이언트 연결 상태 확인 빈도를 조정하는 데 사용됩니다.

### TCP_MAXRT
TCP 재전송 최대 시간을 설정합니다.

```cpp
DWORD maxRt = 5000; // 5초
if (!SetSockOpt(socket, IPPROTO_TCP, TCP_MAXRT, maxRt)) {
    std::cerr << std::format("TCP_MAXRT 설정 실패: {}\n", WSAGetLastError());
}
```

불안정한 네트워크 환경에서 게임 연결이 끊어지는 시간을 조절하는 데 유용합니다.
  

## 실습: SO_REUSEADDR 옵션 테스트
SO_REUSEADDR 옵션은 서버 프로그램이 종료 후 재시작할 때 "Address already in use" 오류를 방지하기 위해 중요합니다. 이 실습에서는 이 옵션의 효과를 테스트해보겠습니다.

### 테스트 프로그램 1: SO_REUSEADDR 없이 실행

```cpp
// ReuseAddrTest1.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <format>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

int main() {
    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패\n";
        return 1;
    }
    
    // TCP 소켓 생성
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // 서버 주소 설정
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);  // 포트 9000 사용
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    // SO_REUSEADDR 옵션 없이 바인딩
    std::cout << "SO_REUSEADDR 옵션 없이 바인딩 시도...\n";
    
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << std::format("바인드 실패: {}\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "바인딩 성공!\n";
    
    // 5초 대기 후 종료 (TIME_WAIT 상태로 만들기 위함)
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << std::format("listen 실패: {}\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "서버 소켓이 9000 포트에서 리스닝 중입니다.\n";
    std::cout << "5초 후 종료됩니다...\n";
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 소켓 닫기
    closesocket(serverSocket);
    WSACleanup();
    
    std::cout << "프로그램이 종료되었습니다. 즉시 다시 실행해보세요.\n";
    return 0;
}
```

### 테스트 프로그램 2: SO_REUSEADDR 사용

```cpp
// ReuseAddrTest2.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <format>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

template <typename T>
bool SetSockOpt(SOCKET sock, int level, int optname, const T& value) {
    return setsockopt(sock, level, optname, 
                     reinterpret_cast<const char*>(&value), 
                     sizeof(T)) != SOCKET_ERROR;
}

int main() {
    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패\n";
        return 1;
    }
    
    // TCP 소켓 생성
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // SO_REUSEADDR 옵션 설정
    std::cout << "SO_REUSEADDR 옵션 설정 중...\n";
    BOOL reuseAddr = TRUE;
    if (!SetSockOpt(serverSocket, SOL_SOCKET, SO_REUSEADDR, reuseAddr)) {
        std::cerr << std::format("SO_REUSEADDR 설정 실패: {}\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    // 서버 주소 설정
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);  // 포트 9000 사용
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    // SO_REUSEADDR 옵션으로 바인딩
    std::cout << "SO_REUSEADDR 옵션으로 바인딩 시도...\n";
    
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << std::format("바인드 실패: {}\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "바인딩 성공!\n";
    
    // 5초 대기 후 종료
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << std::format("listen 실패: {}\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "서버 소켓이 9000 포트에서 리스닝 중입니다.\n";
    std::cout << "5초 후 종료됩니다...\n";
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 소켓 닫기
    closesocket(serverSocket);
    WSACleanup();
    
    std::cout << "프로그램이 종료되었습니다.\n";
    return 0;
}
```

### 테스트 방법
1. 먼저 첫 번째 프로그램(ReuseAddrTest1)을 컴파일하고 실행합니다.
2. 프로그램이 종료된 직후(5초 이내) 다시 같은 프로그램을 실행합니다. "Address already in use" 오류가 발생할 것입니다.
3. 이제 두 번째 프로그램(ReuseAddrTest2)을 컴파일하고 실행합니다.
4. 프로그램이 종료된 직후(5초 이내) 다시 같은 프로그램을 실행합니다. SO_REUSEADDR 옵션 덕분에 오류 없이 실행됩니다.

### 테스트 결과 분석
**SO_REUSEADDR 없이 실행 시**:
첫 번째 프로그램이 종료된 후에도 TCP 연결은 TIME_WAIT 상태로 일정 시간 유지됩니다. 이 상태에서 같은 포트에 바인딩하려고 하면 "Address already in use" 오류가 발생합니다.

**SO_REUSEADDR 사용 시**:
두 번째 프로그램은 SO_REUSEADDR 옵션을 설정했기 때문에 TIME_WAIT 상태의 포트에도 바인딩이 가능합니다. 이는 게임 서버를 재시작할 때 기존 포트를 즉시 재사용할 수 있음을 의미합니다.

### 게임 서버 개발에서의 중요성
온라인 게임 서버 개발에서 SO_REUSEADDR 옵션은 다음과 같은 이유로 중요합니다:

1. **빠른 서버 재시작**: 서버 크래시나 업데이트 후 즉시 재시작 가능
2. **다운타임 최소화**: 포트 대기 시간 없이 서비스 재개 가능
3. **멀티프로세스 서버**: 여러 프로세스가 같은 포트를 공유할 수 있음

그러나 주의할 점은 SO_REUSEADDR 옵션이 보안상의 위험을 증가시킬 수 있다는 것입니다. 다른 프로그램이 같은 포트를 사용할 가능성이 있으므로, 게임 서버 환경에서는 추가적인 보안 조치를 고려해야 합니다.

### 최적의 서버 옵션 설정 예제
다음은 게임 서버에서 일반적으로 사용되는 소켓 옵션 조합입니다:

```cpp
bool ConfigureGameServerSocket(SOCKET serverSocket) {
    // 주소 재사용 허용
    BOOL reuseAddr = TRUE;
    if (!SetSockOpt(serverSocket, SOL_SOCKET, SO_REUSEADDR, reuseAddr)) {
        std::cerr << std::format("SO_REUSEADDR 설정 실패: {}\n", WSAGetLastError());
        return false;
    }
    
    // 수신 버퍼 크기 설정
    int recvBufSize = 256 * 1024; // 256KB
    if (!SetSockOpt(serverSocket, SOL_SOCKET, SO_RCVBUF, recvBufSize)) {
        std::cerr << std::format("SO_RCVBUF 설정 실패: {}\n", WSAGetLastError());
        return false;
    }
    
    // 송신 버퍼 크기 설정
    int sendBufSize = 256 * 1024; // 256KB
    if (!SetSockOpt(serverSocket, SOL_SOCKET, SO_SNDBUF, sendBufSize)) {
        std::cerr << std::format("SO_SNDBUF 설정 실패: {}\n", WSAGetLastError());
        return false;
    }
    
    // Nagle 알고리즘 비활성화 (실시간 게임에 중요)
    BOOL noDelay = TRUE;
    if (!SetSockOpt(serverSocket, IPPROTO_TCP, TCP_NODELAY, noDelay)) {
        std::cerr << std::format("TCP_NODELAY 설정 실패: {}\n", WSAGetLastError());
        return false;
    }
    
    // keepalive 활성화
    BOOL keepAlive = TRUE;
    if (!SetSockOpt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, keepAlive)) {
        std::cerr << std::format("SO_KEEPALIVE 설정 실패: {}\n", WSAGetLastError());
        return false;
    }
    
    // 모든 옵션이 성공적으로 설정됨
    return true;
}
```

이러한 설정을 통해 게임 서버의 성능과 안정성을 최적화할 수 있습니다. 특히 TCP_NODELAY 옵션은 실시간 게임에서 지연 시간을 최소화하는 데 매우 중요합니다.

-----  
소켓 옵션은 게임 서버의 성능, 안정성, 그리고 사용자 경험에 직접적인 영향을 미칩니다. 각 게임의 특성과 요구사항에 맞게 적절한 옵션을 선택하고 설정하는 것이 중요합니다. 실제 서버 개발 시에는 다양한 부하 테스트를 통해 최적의 설정을 찾는 과정이 필요합니다.   
    
  
<br>      
   
# Chapter.9 소켓 입출력 모델

## 01 소켓 입출력 모델 개요
윈도우 환경에서의 네트워크 프로그래밍을 위한 다양한 소켓 입출력 모델에 대해 알아보겠습니다. 온라인 게임 서버와 같은 고성능 네트워크 애플리케이션 개발에 있어 적절한 입출력 모델의 선택은 매우 중요합니다.

### 윈도우에서 지원하는 소켓 입출력 모델
1. **블로킹(Blocking) 소켓**
   - 가장 기본적인 모델로, 작업이 완료될 때까지 대기
   - 구현이 간단하지만 다중 클라이언트 처리에 비효율적
   - 단일 클라이언트 처리나 멀티스레드 환경에서 활용

2. **넌블로킹(Non-blocking) 소켓**
   - 작업 완료 여부와 관계없이 즉시 반환
   - 반환 값과 오류 코드를 통해 작업 완료 여부 확인
   - 폴링(polling) 방식으로 지속적인 확인 필요

3. **I/O 멀티플렉싱(Multiplexing)**
   - **select**: 다수의 소켓을 모니터링하는 기본적인 방식
   - WSAPoll: select 모델의 확장 버전
   - WSAAsyncSelect: 윈도우 메시지를 활용한 비동기 처리

4. **비동기 I/O**
   - WSAEventSelect: 이벤트 객체를 활용한 비동기 모델
   - Overlapped I/O: 중첩 I/O 모델
   - **IOCP(I/O Completion Port)**: 고성능 서버를 위한 완성 포트 모델
  
![윈도우 소켓 입출력 모델](./images/210.png)   


### 🔌 소켓 입출력 모델 동작 다이어그램    
  
![](./images/211.png)   
![](./images/212.png)   
![](./images/213.png)   
![](./images/214.png)   
   
   
### 소켓 입출력 모델 선택 시 고려사항
- 처리해야 할 동시 연결 수
- 응답 시간 요구사항
- 개발 복잡도
- 리소스 사용량
- 확장성 요구사항

온라인 게임 서버와 같은 고성능 서버는 일반적으로 **IOCP** 모델을 활용하는 것이 권장됩니다.
  
  

## 02. 넌블로킹 소켓
넌블로킹 소켓을 사용한 기본적인 에코 서버를 구현해보겠습니다.  

![](./images/057.png)   
![](./images/058.png)   
![](./images/059.png)  

### 🎯 주요 특징
1. **Single Thread**: 모든 소켓을 하나의 스레드에서 순차적으로 처리
2. **Non-Blocking**: `ioctlsocket(FIONBIO)`로 소켓을 즉시 반환하도록 설정
3. **Polling 방식**: 메인 루프에서 지속적으로 모든 소켓 상태 확인
4. **WSAEWOULDBLOCK**: 데이터가 없을 때 발생하는 정상적인 에러
 

### ⚙️ 작동 흐름
1. **연결 수락**: `accept()`가 즉시 반환, 새 클라이언트는 vector에 추가
2. **데이터 처리**: 모든 클라이언트 소켓에 `recv()` 시도
3. **에코 응답**: 받은 데이터를 `send()`로 즉시 전송
4. **CPU 절약**: `Sleep(1)`로 과도한 CPU 사용 방지
  

### 📊 장단점
- **장점**: 구현 단순, 적은 메모리 사용, 많은 동시 연결 가능
- **단점**: 높은 CPU 사용률(폴링), IOCP보다 낮은 성능



```cpp
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <format>

#pragma comment(lib, "ws2_32.lib")

class NonBlockingServer {
private:
    SOCKET listenSocket;
    std::vector<SOCKET> clientSockets;
    
public:
    NonBlockingServer() : listenSocket(INVALID_SOCKET) {}
    
    ~NonBlockingServer() {
        for (auto& socket : clientSockets) {
            closesocket(socket);
        }
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
        }
        WSACleanup();
    }
    
    bool Initialize(const std::string& port) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << std::format("WSAStartup 실패: {}\n", result);
            return false;
        }
        
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;
        
        addrinfo* addrResult = nullptr;
        result = getaddrinfo(nullptr, port.c_str(), &hints, &addrResult);
        if (result != 0) {
            std::cerr << std::format("getaddrinfo 실패: {}\n", gai_strerrorA(result));
            WSACleanup();
            return false;
        }
        
        listenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            freeaddrinfo(addrResult);
            WSACleanup();
            return false;
        }
        
        // 소켓을 넌블로킹 모드로 설정
        u_long nonBlocking = 1;
        result = ioctlsocket(listenSocket, FIONBIO, &nonBlocking);
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("ioctlsocket 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return false;
        }
        
        result = bind(listenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
        freeaddrinfo(addrResult);
        
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("bind 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        return true;
    }
    
    void Run() {
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << std::format("listen 실패: {}\n", WSAGetLastError());
            return;
        }
        
        std::cout << "서버가 시작되었습니다. 클라이언트 대기 중...\n";
        
        while (true) {
            // 새 클라이언트 연결 확인
            SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
            if (clientSocket != INVALID_SOCKET) {
                // 클라이언트 소켓도 넌블로킹으로 설정
                u_long nonBlocking = 1;
                int result = ioctlsocket(clientSocket, FIONBIO, &nonBlocking);
                if (result == SOCKET_ERROR) {
                    std::cerr << std::format("클라이언트 ioctlsocket 실패: {}\n", WSAGetLastError());
                    closesocket(clientSocket);
                }
                else {
                    clientSockets.push_back(clientSocket);
                    std::cout << std::format("새 클라이언트 연결: {}\n", clientSocket);
                }
            }
            else {
                // WSAEWOULDBLOCK 에러는 넌블로킹 소켓에서 정상적인 상황
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK) {
                    std::cerr << std::format("accept 실패: {}\n", error);
                }
            }
            
            // 모든 클라이언트 소켓 처리
            auto it = clientSockets.begin();
            while (it != clientSockets.end()) {
                SOCKET socket = *it;
                char buffer[4096];
                
                // 데이터 수신 시도
                int bytesReceived = recv(socket, buffer, sizeof(buffer), 0);
                
                if (bytesReceived > 0) {
                    // 데이터 수신 성공
                    buffer[bytesReceived] = '\0';
                    std::cout << std::format("데이터 수신 ({}바이트): {}\n", bytesReceived, buffer);
                    
                    // 에코: 받은 데이터 다시 전송
                    int bytesSent = 0;
                    while (bytesSent < bytesReceived) {
                        int result = send(socket, buffer + bytesSent, bytesReceived - bytesSent, 0);
                        if (result > 0) {
                            bytesSent += result;
                        }
                        else if (result == SOCKET_ERROR) {
                            int error = WSAGetLastError();
                            if (error != WSAEWOULDBLOCK) {
                                std::cerr << std::format("send 실패: {}\n", error);
                                break;
                            }
                        }
                    }
                    
                    ++it;
                }
                else if (bytesReceived == 0) {
                    // 클라이언트 연결 종료
                    std::cout << std::format("클라이언트 연결 종료: {}\n", socket);
                    closesocket(socket);
                    it = clientSockets.erase(it);
                }
                else {
                    // 에러 발생
                    int error = WSAGetLastError();
                    if (error == WSAEWOULDBLOCK) {
                        // 데이터가 아직 없음 - 정상
                        ++it;
                    }
                    else {
                        // 다른 에러 - 연결 종료
                        std::cerr << std::format("recv 실패: {}\n", error);
                        closesocket(socket);
                        it = clientSockets.erase(it);
                    }
                }
            }
            
            // CPU 사용률 절약을 위한 짧은 슬립
            Sleep(1);
        }
    }
};

int main() {
    NonBlockingServer server;
    
    if (server.Initialize("12345")) {
        server.Run();
    }
    
    return 0;
}
```
  

## 03 Select 모델
Select 모델은 가장 기본적인 I/O 멀티플렉싱 방식으로, 단일 스레드에서 여러 소켓을 모니터링할 수 있습니다.

### Select 모델의 기본 원리
Select 함수는 지정된 소켓 집합에서 읽기, 쓰기, 예외 상태를 확인하여 준비된 소켓만 반환합니다.  
  
![Windows Select 모델](./images/048.png)  
![Windows Select API](./images/049.png)   
    

### 에제코드: SelectServer
     
![](./images/050.png)  
![](./images/051.png)  
![](./images/052.png)  
  
**핵심 동작 흐름:**
1. **초기화**: WSAStartup → 소켓 생성 → 바인딩
2. **리슨 시작**: listen() 호출 후 masterSet에 리슨 소켓 등록
3. **메인 루프**: 
   - `fd_set readSet = masterSet` (매번 복사)
   - `select()` 호출로 I/O 가능한 소켓들 대기
   - 준비된 소켓들을 순회하며 처리
  
**두 가지 주요 이벤트 처리:**
- **새 연결**: `socket == listenSocket`인 경우 → `accept()` → masterSet 추가
- **데이터 수신**: 기존 클라이언트 소켓 → `recv()` → `send()`로 에코
  
**중요한 구현 세부사항:**
- `std::vector<SOCKET> clientSockets`로 클라이언트 소켓 별도 관리
- 연결 종료 시 masterSet에서 `FD_CLR()`, 벡터에서도 제거
- C++20의 `std::format` 사용으로 현대적인 코드 스타일
- RAII 패턴으로 소멸자에서 자동 리소스 정리
  
  
```cpp
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <format>

#pragma comment(lib, "ws2_32.lib")

class SelectServer {
private:
    SOCKET listenSocket;
    fd_set masterSet;
    std::vector<SOCKET> clientSockets;
    
public:
    SelectServer() : listenSocket(INVALID_SOCKET) {
        FD_ZERO(&masterSet);
    }
    
    ~SelectServer() {
        for (auto& socket : clientSockets) {
            closesocket(socket);
        }
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
        }
        WSACleanup();
    }
    
    bool Initialize(const std::string& port) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << std::format("WSAStartup 실패: {}\n", result);
            return false;
        }
        
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;
        
        addrinfo* addrResult = nullptr;
        result = getaddrinfo(nullptr, port.c_str(), &hints, &addrResult);
        if (result != 0) {
            std::cerr << std::format("getaddrinfo 실패: {}\n", gai_strerrorA(result));
            WSACleanup();
            return false;
        }
        
        listenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            freeaddrinfo(addrResult);
            WSACleanup();
            return false;
        }
        
        result = bind(listenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
        freeaddrinfo(addrResult);
        
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("bind 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
        
        return true;
    }
    
    void Run() {
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << std::format("listen 실패: {}\n", WSAGetLastError());
            return;
        }
        
        // 리슨 소켓을 마스터 세트에 추가
        FD_SET(listenSocket, &masterSet);
        
        std::cout << "서버가 시작되었습니다. 클라이언트 대기 중...\n";
        
        while (true) {
            fd_set readSet = masterSet;
            
            // select 함수로 소켓 상태 모니터링
            int socketCount = select(0, &readSet, nullptr, nullptr, nullptr);
            
            if (socketCount == SOCKET_ERROR) {
                std::cerr << std::format("select 실패: {}\n", WSAGetLastError());
                break;
            }
            
            // 모든 소켓 확인
            for (int i = 0; i < readSet.fd_count; i++) {
                SOCKET socket = readSet.fd_array[i];
                
                if (socket == listenSocket) {
                    // 새 연결 수락
                    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
                    if (clientSocket == INVALID_SOCKET) {
                        std::cerr << std::format("accept 실패: {}\n", WSAGetLastError());
                        continue;
                    }
                    
                    // 새 클라이언트 소켓을 마스터 세트에 추가
                    FD_SET(clientSocket, &masterSet);
                    clientSockets.push_back(clientSocket);
                    
                    std::cout << std::format("새 클라이언트 연결: {}\n", clientSocket);
                }
                else {
                    // 기존 클라이언트로부터 데이터 수신
                    char buffer[4096];
                    int bytesReceived = recv(socket, buffer, sizeof(buffer), 0);
                    
                    if (bytesReceived <= 0) {
                        // 연결 종료 또는 오류
                        if (bytesReceived == 0) {
                            std::cout << std::format("클라이언트 연결 종료: {}\n", socket);
                        }
                        else {
                            std::cerr << std::format("recv 실패: {}\n", WSAGetLastError());
                        }
                        
                        // 소켓 정리
                        closesocket(socket);
                        FD_CLR(socket, &masterSet);
                        
                        // clientSockets 벡터에서 제거
                        auto it = std::find(clientSockets.begin(), clientSockets.end(), socket);
                        if (it != clientSockets.end()) {
                            clientSockets.erase(it);
                        }
                    }
                    else {
                        // 에코 서버: 받은 데이터를 다시 전송
                        buffer[bytesReceived] = '\0';
                        std::cout << std::format("데이터 수신 ({}바이트): {}\n", bytesReceived, buffer);
                        
                        int bytesSent = send(socket, buffer, bytesReceived, 0);
                        if (bytesSent == SOCKET_ERROR) {
                            std::cerr << std::format("send 실패: {}\n", WSAGetLastError());
                        }
                    }
                }
            }
        }
    }
};
```

### Select 모델의 장단점
**장점:**
- 구현이 비교적 간단함
- 단일 스레드에서 여러 클라이언트 처리 가능
- 크로스 플랫폼 호환성이 좋음

**단점:**
- 소켓 수가 많아지면 성능이 급격히 저하됨 (O(n) 복잡도)
- Windows에서 최대 64개의 소켓만 처리 가능 (FD_SETSIZE 제한)
- 대규모 연결에는 적합하지 않음
    
 
### FD_SETSIZE 의 최대 크기

#### Windows
- 기본값: **64개**
- 컴파일 시 `#define FD_SETSIZE 128` 같은 방식으로 쉽게 변경 가능
- 실제로는 64K까지도 설정 가능하다고 함
  
```cpp
// winsock2.h 포함 전에 정의하면 됨
#define FD_SETSIZE 1024  // 64 → 1024로 증가
#include <winsock2.h>
```
  
#### Linux
**기본값**: 1024 (대부분의 리눅스 배포판)
**이론적 최대값**: 일반적으로 65536 (64K)까지 확장 가능하지만, 실제로는 더 제한적입니다.

##### 확장 방법과 한계

###### 1. 컴파일 타임 확장
```c
#define FD_SETSIZE 4096
#include <sys/select.h>
```

###### 2. 실제 한계 요인들

**메모리 제약**:
- fd_set 구조체 크기 = FD_SETSIZE / 8 바이트
- FD_SETSIZE 65536 → fd_set 크기 8KB
- select() 호출마다 3개의 fd_set (read, write, except) 복사 필요

**성능 저하**:
- FD_SETSIZE가 클수록 select() 성능 급격히 저하
- O(n) 복잡도로 모든 FD를 순회해야 함
- 일반적으로 1024~4096 이상에서는 성능 문제 발생

###### 3. 시스템 한계 확인 방법

```bash
# 프로세스당 최대 파일 디스크립터 수 확인
ulimit -n

# 시스템 전체 최대 파일 디스크립터 수
cat /proc/sys/fs/file-max

# 현재 사용 중인 파일 디스크립터 수
cat /proc/sys/fs/file-nr
```

###### 권장 최대값
- **일반적인 경우**: 4096 이하
- **고성능이 필요한 경우**: 1024 이하 또는 다른 모델 사용

###### 대안 솔루션
FD_SETSIZE를 크게 늘리는 대신 더 효율적인 I/O 모델 사용을 권장합니다:

1. **epoll** (리눅스 고유)
   - 최대 약 100만 개의 FD 처리 가능
   - O(1) 성능

2. **poll**
   - FD_SETSIZE 제한 없음
   - 하지만 여전히 O(n) 성능

3. **io_uring** (리눅스 5.1+)
   - 최신 비동기 I/O 인터페이스
   - 최고 성능

###### 결론
기술적으로는 FD_SETSIZE를 65536까지 늘릴 수 있지만, **실용적으로는 4096 이하를 권장**합니다. 더 많은 연결을 처리해야 한다면 `epoll`이나 `io_uring` 같은 현대적인 I/O 모델을 사용하는 것이 훨씬 효율적입니다.

  

## 비동기 I/O  
  
![네트워크 프로그래밍에서 비동기 I/O 개념](./images/054.png)   

### 비동기 I/O 실생활 비유 다이어그램
  
![](./images/215.png)   
![](./images/216.png)   
![](./images/217.png)   
![](./images/218.png)   
![](./images/219.png)     
![](./images/220.png)     


### 비동기 I/O가 게임 서버 성능에 미치는 영향
  
![I/O 모델에 따른 동시 접속자 처리 능력 비교](./images/053.png)     
  
  
## 04 IOCP 모델
**"IOCP는 Input/Output Completion Port의 줄임말로, 윈도우에서 제공하는 고성능 비동기 I/O 모델입니다."**

**핵심 특징 3가지:**
- **완료 기반(Completion-based)**: I/O 작업이 완료되면 알려줌
- **커널 레벨 큐**: 운영체제가 직접 관리하는 효율적인 큐 시스템
- **워커 스레드 풀**: 제한된 수의 스레드로 많은 연결을 처리

### 2단계: 동작 원리 설명 

```
1. I/O 요청 단계
   - WSARecv(), WSASend() 등으로 비동기 I/O 요청
   - 함수는 즉시 반환 (블록되지 않음)

2. 백그라운드 처리
   - 커널이 백그라운드에서 실제 I/O 작업 수행
   - 애플리케이션은 다른 작업 계속 가능

3. 완료 통지
   - I/O 완료 시 결과를 Completion Port 큐에 삽입
   - 성공/실패, 전송된 바이트 수 등 정보 포함

4. 워커 스레드 처리
   - GetQueuedCompletionStatus()로 완료된 I/O 결과 획득
   - 워커 스레드가 후속 비즈니스 로직 처리
```

### 3단계: 왜 뛰어난지 설명

#### **성능 면에서:**
- **O(1) 복잡도**: select는 O(n), IOCP는 상수 시간
- **커널 최적화**: 운영체제 레벨에서 최적화된 구조
- **메모리 효율성**: fd_set 복사 불필요, 큐 기반 처리

#### **확장성 면에서:**
- **동시 연결 수**: 수만~수십만 연결 처리 가능
- **스레드 효율성**: 연결 수와 무관하게 고정된 워커 스레드 수
- **CPU 코어 활용**: 멀티코어 시스템에서 최적의 성능

#### **게임 서버에 최적인 이유:**
- **낮은 지연시간**: 실시간 게임에 필수
- **높은 처리량**: 대규모 동시 사용자 지원
- **안정성**: 커널 레벨 보장으로 안정적


### Q&A 
 
#### Q: "IOCP의 단점은 없나요?"
**A:** "네, 몇 가지 제약이 있습니다."
- **윈도우 전용**: 플랫폼 종속성 (리눅스는 epoll 사용)
- **복잡성**: 구현이 select보다 복잡함
- **디버깅**: 비동기 특성상 디버깅이 어려움
- **오버헤드**: 적은 연결 수에서는 오히려 비효율적일 수 있음

#### Q: "워커 스레드는 몇 개가 적당한가요?"
**A:** "일반적으로 **CPU 코어 수 × 2** 정도가 권장됩니다."
- CPU 집약적 작업: CPU 코어 수와 동일
- I/O 집약적 작업: CPU 코어 수 × 2~4
- 실제로는 성능 테스트를 통해 최적값 찾아야 함

#### Q: "다른 플랫폼에서는 뭘 사용하나요?"
**A:**
- **리눅스**: epoll, io_uring
- **macOS**: kqueue
- **크로스 플랫폼**: libevent, libuv 같은 라이브러리


### 스레드 효율성: 연결 수와 무관하게 고정된 워커 스레드 수

#### **기존 모델의 문제점**
**Thread-per-Connection 모델:**
```
클라이언트 1 ←→ 스레드 1
클라이언트 2 ←→ 스레드 2  
클라이언트 3 ←→ 스레드 3
...
클라이언트 10,000 ←→ 스레드 10,000
```

**문제점:**
- **메모리 폭증**: 스레드 하나당 스택 메모리 1~8MB 필요
  - 10,000 연결 = 10GB~80GB 메모리 사용!
- **컨텍스트 스위칭 오버헤드**: 스레드 전환 비용 급증
- **스케줄링 부담**: OS가 관리해야 할 스레드 수 폭증

#### **IOCP 모델의 해결**
**고정된 워커 스레드 풀:**
```
워커 스레드 1 ┐
워커 스레드 2 ├─→ IOCP 큐 ←─ 클라이언트 1~10,000
워커 스레드 3 ┘
```

**핵심 아이디어:**
```c++
// 전통적인 방식 (BAD)
for(int i = 0; i < clientCount; i++) {
    CreateThread(HandleClient, clients[i]);  // 클라이언트 수만큼 스레드 생성
}

// IOCP 방식 (GOOD)
const int WORKER_THREADS = GetSystemInfo().dwNumberOfProcessors * 2;
for(int i = 0; i < WORKER_THREADS; i++) {
    CreateThread(WorkerThread, completionPort);  // 고정된 수의 워커 스레드
}
```

#### **실제 숫자로 보는 효율성**

| 연결 수 | Thread-per-Connection | IOCP |
|---------|----------------------|------|
| 1,000명 | 1,000개 스레드 (1~8GB) | 8개 스레드 (8~64MB) |
| 10,000명 | 10,000개 스레드 (10~80GB) | 8개 스레드 (8~64MB) |
| 100,000명 | 불가능 (메모리 부족) | 8개 스레드 (8~64MB) |


### CPU 코어 활용: 멀티코어 시스템에서 최적의 성능

#### IOCP의 스마트한 작업 분산
**자동 로드 밸런싱:**
```
CPU Core 1: 워커 스레드 1, 2
CPU Core 2: 워커 스레드 3, 4  
CPU Core 3: 워커 스레드 5, 6
CPU Core 4: 워커 스레드 7, 8

IOCP 큐에서 완료된 I/O를 가장 여유로운 워커 스레드가 자동으로 처리
```

#### **GetQueuedCompletionStatus()의 스마트함**

```c++
// 여러 워커 스레드가 동시에 대기
DWORD WorkerThread(LPVOID param) {
    while(true) {
        DWORD bytesTransferred;
        OVERLAPPED* overlapped;
        SOCKET clientSocket;
        
        // 이 함수가 스마트하게 작업을 분산함
        BOOL result = GetQueuedCompletionStatus(
            completionPort,           // IOCP 핸들
            &bytesTransferred,        // 전송된 바이트
            (PULONG_PTR)&clientSocket,// 완료된 소켓
            &overlapped,              // 완료된 작업
            INFINITE                  // 무한 대기
        );
        
        if(result) {
            // 이 워커 스레드가 이 작업을 처리
            ProcessCompletedIO(clientSocket, bytesTransferred, overlapped);
        }
    }
}
```

#### **왜 최적의 성능일까?**
**1. 워크 스틸링 (Work Stealing) 효과**
```
상황: Core 1은 바쁘고, Core 2는 여유로움

기존 방식:
Core 1: [작업1][작업2][작업3] ← 대기열 길어짐
Core 2: [      비어있음      ] ← 놀고 있음

IOCP 방식:
Core 1: [작업1] ← IOCP 큐에서 가져옴
Core 2: [작업2] ← IOCP 큐에서 가져옴 (자동 분산!)
```

**2. NUMA (Non-Uniform Memory Access) 최적화**
```
대용량 서버에서:
CPU 소켓 1 (Core 1-8) ←→ 메모리 뱅크 1
CPU 소켓 2 (Core 9-16) ←→ 메모리 뱅크 2

IOCP는 같은 소켓의 코어와 메모리를 함께 사용하도록 최적화
→ 메모리 접근 속도 향상
```

    
### IOCP (I/O Completion Port) 아키텍처  
  
```
┌─────────────────────────────────────────────────────────────────┐
│                      응용 프로그램                                │
│                                                                 │
│  스레드1    스레드2    스레드3    스레드4    ... 워커 스레드 풀       │
│    │         │         │         │                              │
│    └─────────┼─────────┼─────────┼─────────────┐                │
└──────────────┼─────────┼─────────┼─────────────┼────────────────┘
               │         │         │             │
               ▼         ▼         ▼             ▼
┌─────────────────────────────────────────────────────────────────┐
│            GetQueuedCompletionStatus() 호출                      │
│                     (블로킹 대기)                                 │
└─────────────────────────────────────────────────────────────────┘
               ▲
               │ 완료 통지
               │
┌─────────────────────────────────────────────────────────────────┐
│                   I/O Completion Port                           │
│                      (커널 객체)                                 │
│                                                                 │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │   완료된 I/O     │  │   완료된 I/O     │  │   완료된 I/O     │ │
│  │   작업 큐        │  │   작업 큐        │  │   작업 큐        │ │
│  │                 │  │                 │  │                 │ │
│  │ [소켓A-수신완료]  │  │ [소켓B-송신완료]  │  │ [소켓C-연결완료]  │ │
│  │ [소켓D-수신완료]  │  │ [소켓E-송신완료]  │  │ [소켓F-오류발생]  │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
               ▲
               │ I/O 완료 통지
               │
┌─────────────────────────────────────────────────────────────────┐
│                      커널 영역                                   │
│                                                                 │
│  소켓1 ◄──── 비동기 수신 작업 진행중                                │
│  소켓2 ◄──── 비동기 송신 작업 진행중                                │
│  소켓3 ◄──── 비동기 연결 작업 진행중                                │
│  소켓4 ◄──── 비동기 수신 작업 진행중                                │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**IOCP 동작 과정:**

**1단계 - IOCP 생성 및 소켓 연결**
```
CreateIoCompletionPort() 호출
         │
         ▼
┌─────────────────┐
│   IOCP 객체      │ ◄─── 소켓들을 IOCP에 연결
│    생성          │      CreateIoCompletionPort(소켓, IOCP, ...)
└─────────────────┘
```

**2단계 - 비동기 I/O 작업 시작**
```
┌─────────────────────────────────────────────────────────────┐
│                비동기 I/O 작업 시작                           │
│                                                             │
│  WSARecv()     WSASend()     AcceptEx()     ConnectEx()     │
│     │             │             │             │             │
│     ▼             ▼             ▼             ▼             │
│  즉시 반환      즉시 반환      즉시 반환      즉시 반환          │
│ (WSA_IO_       (WSA_IO_       (WSA_IO_       (WSA_IO_      │
│  PENDING)       PENDING)       PENDING)       PENDING)     │
└─────────────────────────────────────────────────────────────┘
```

**3단계 - 워커 스레드의 완료 대기**
```
워커 스레드 풀
┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐
│스레드 1  │  │스레드 2  │  │스레드 3  │  │스레드 4  │
└────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘
     │            │            │            │
     └────────────┼────────────┼────────────┘
                  │            │
                  ▼            ▼
         GetQueuedCompletionStatus()
              (블로킹 대기)
```

**4단계 - I/O 완료 처리**
```
I/O 작업 완료!
┌─────────────────────────────────────────────────────────────┐
│                    커널에서 IOCP로                            │
│                                                             │
│  완료된 작업 정보:                                            │
│  • 전송된 바이트 수                                           │
│  • 오버랩 구조체 포인터                                        │
│  • 완료 키 (Completion Key)                                  │
│  • 오류 코드                                                 │
│                                                             │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│              워커 스레드가 깨어남                              │
│                                                             │
│  GetQueuedCompletionStatus() 반환                            │
│  • dwNumberOfBytes: 전송 바이트                               │
│  • lpOverlapped: 오버랩 구조체                                │
│  • lpCompletionKey: 완료 키                                   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**IOCP의 주요 특징:**

**확장성 (Scalability)**
```
        클라이언트 수
             ▲
             │     IOCP
             │    ┌─────
             │   ╱
             │  ╱
             │ ╱
             │╱     Select 모델
             └─────────────────► 성능
            낮은 성능          높은 성능
```

**비동기 처리 방식**
```
동기 방식 (Select):
응용프로그램 ──► I/O 요청 ──► 대기 ──► 완료 ──► 처리

비동기 방식 (IOCP):
응용프로그램 ──► I/O 요청 ──► 즉시 반환
      │                        ▲
      ▼                        │
   다른 작업 수행 ──────────► 완료 통지 받음
```
  

### IOCP 프로그래밍 기초 

#### 1. 핵심 IOCP API 개요

```cpp
// 주요 IOCP API들
HANDLE CreateIoCompletionPort(
    HANDLE FileHandle,           // 소켓 핸들
    HANDLE ExistingCompletionPort, // 기존 IOCP 핸들
    ULONG_PTR CompletionKey,     // 완료 키
    DWORD NumberOfConcurrentThreads // 동시 실행 스레드 수
);

BOOL GetQueuedCompletionStatus(
    HANDLE CompletionPort,       // IOCP 핸들
    LPDWORD lpNumberOfBytes,     // 전송된 바이트 수
    PULONG_PTR lpCompletionKey,  // 완료 키
    LPOVERLAPPED* lpOverlapped,  // 오버랩 구조체
    DWORD dwMilliseconds         // 타임아웃
);

// 비동기 소켓 API들
int WSARecv(...);
int WSASend(...);
BOOL AcceptEx(...);
BOOL ConnectEx(...);
```

#### 2. 단계별 IOCP 구현

##### 2.1 IOCP 객체 생성 및 초기화

```cpp
class IOCPServer {
private:
    HANDLE m_hIOCP;              // IOCP 핸들
    std::vector<HANDLE> m_workerThreads;  // 워커 스레드들
    
public:
    bool InitializeIOCP() {
        // 1. IOCP 객체 생성
        m_hIOCP = CreateIoCompletionPort(
            INVALID_HANDLE_VALUE,    // 새 IOCP 생성
            NULL,                    // 기존 IOCP 없음
            0,                       // 완료 키 (나중에 설정)
            0                        // 시스템이 자동으로 스레드 수 결정
        );
        
        if (m_hIOCP == NULL) {
            printf("CreateIoCompletionPort failed: %d\n", GetLastError());
            return false;
        }
        
        // 2. 워커 스레드 생성
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        int threadCount = sysInfo.dwNumberOfProcessors * 2;
        
        for (int i = 0; i < threadCount; ++i) {
            HANDLE hThread = CreateThread(
                NULL, 0, WorkerThread, this, 0, NULL
            );
            m_workerThreads.push_back(hThread);
        }
        
        return true;
    }
};
```  

![IOCP 큐 생성 과정](./images/134.png)   
 

##### 2.2 소켓을 IOCP에 연결

```cpp
// 클라이언트 정보를 담는 구조체
struct ClientInfo {
    SOCKET socket;
    SOCKADDR_IN addr;
    char recvBuffer[1024];
    WSAOVERLAPPED overlapped;
    WSABUF wsaBuf;
    DWORD recvBytes;
    DWORD flags;
};

bool AssociateWithIOCP(SOCKET clientSocket) {
    // 클라이언트 정보 객체 생성
    ClientInfo* pClientInfo = new ClientInfo;
    ZeroMemory(pClientInfo, sizeof(ClientInfo));
    pClientInfo->socket = clientSocket;
    
    // 소켓을 IOCP에 연결
    HANDLE hResult = CreateIoCompletionPort(
        (HANDLE)clientSocket,        // 소켓 핸들
        m_hIOCP,                     // 기존 IOCP 핸들
        (ULONG_PTR)pClientInfo,      // 완료 키 (클라이언트 정보)
        0                            // 스레드 수 (이미 설정됨)
    );
    
    if (hResult != m_hIOCP) {
        printf("소켓을 IOCP에 연결 실패: %d\n", GetLastError());
        delete pClientInfo;
        return false;
    }
    
    // 첫 번째 비동기 수신 작업 시작
    return PostRecv(pClientInfo);
}
```

##### 2.3 비동기 수신 작업 시작

```cpp
bool PostRecv(ClientInfo* pClientInfo) {
    // 오버랩 구조체 초기화
    ZeroMemory(&pClientInfo->overlapped, sizeof(WSAOVERLAPPED));
    
    // WSABUF 설정
    pClientInfo->wsaBuf.buf = pClientInfo->recvBuffer;
    pClientInfo->wsaBuf.len = sizeof(pClientInfo->recvBuffer);
    
    // 플래그 설정
    pClientInfo->flags = 0;
    
    // 비동기 수신 시작
    int result = WSARecv(
        pClientInfo->socket,         // 소켓
        &pClientInfo->wsaBuf,        // 버퍼 정보
        1,                           // 버퍼 개수
        &pClientInfo->recvBytes,     // 수신된 바이트 수
        &pClientInfo->flags,         // 플래그
        &pClientInfo->overlapped,    // 오버랩 구조체
        NULL                         // 완료 루틴 (IOCP 사용시 NULL)
    );
    
    // WSARecv는 즉시 완료되거나 WSA_IO_PENDING 반환
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING) {
            printf("WSARecv 실패: %d\n", error);
            return false;
        }
    }
    
    return true;
}
```

##### 2.4 비동기 송신 작업

```cpp
bool PostSend(ClientInfo* pClientInfo, const char* data, int dataLen) {
    // 송신용 오버랩 구조체 (별도로 할당)
    WSAOVERLAPPED* pSendOverlapped = new WSAOVERLAPPED;
    ZeroMemory(pSendOverlapped, sizeof(WSAOVERLAPPED));
    
    // 송신 데이터 복사 (비동기이므로 데이터 보존 필요)
    char* pSendBuffer = new char[dataLen];
    memcpy(pSendBuffer, data, dataLen);
    
    WSABUF sendBuf;
    sendBuf.buf = pSendBuffer;
    sendBuf.len = dataLen;
    
    DWORD sendBytes = 0;
    
    int result = WSASend(
        pClientInfo->socket,
        &sendBuf,
        1,
        &sendBytes,
        0,
        pSendOverlapped,
        NULL
    );
    
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING) {
            printf("WSASend 실패: %d\n", error);
            delete[] pSendBuffer;
            delete pSendOverlapped;
            return false;
        }
    }
    
    return true;
}
```

##### 2.5 워커 스레드 구현

```cpp
static DWORD WINAPI WorkerThread(LPVOID lpParam) {
    IOCPServer* pServer = (IOCPServer*)lpParam;
    HANDLE hIOCP = pServer->m_hIOCP;
    
    DWORD bytesTransferred = 0;
    ClientInfo* pClientInfo = nullptr;
    WSAOVERLAPPED* pOverlapped = nullptr;
    
    while (true) {
        // 완료된 I/O 작업 대기
        BOOL result = GetQueuedCompletionStatus(
            hIOCP,                           // IOCP 핸들
            &bytesTransferred,               // 전송된 바이트 수
            (PULONG_PTR)&pClientInfo,        // 완료 키 (클라이언트 정보)
            &pOverlapped,                    // 오버랩 구조체
            INFINITE                         // 무한 대기
        );
        
        // 서버 종료 신호 처리
        if (pClientInfo == nullptr) {
            break;
        }
        
        // I/O 작업 실패 처리
        if (!result || bytesTransferred == 0) {
            printf("클라이언트 연결 종료\n");
            pServer->DisconnectClient(pClientInfo);
            continue;
        }
        
        // 수신 작업 완료 처리
        if (pOverlapped == &pClientInfo->overlapped) {
            pServer->ProcessRecv(pClientInfo, bytesTransferred);
        }
        // 송신 작업 완료 처리 (별도 오버랩 구조체)
        else {
            pServer->ProcessSend(pOverlapped);
        }
    }
    
    return 0;
}
```  
[IOCP 워커 스레드 동작 과정](https://poe.com/preview/Kpot06Ia8Gxsget1hf94 )  
  

##### 2.6 I/O 완료 처리

```cpp
void ProcessRecv(ClientInfo* pClientInfo, DWORD bytesTransferred) {
    // 수신된 데이터 처리
    pClientInfo->recvBuffer[bytesTransferred] = '\0';
    printf("수신: %s\n", pClientInfo->recvBuffer);
    
    // 에코 서버 예제: 받은 데이터를 그대로 송신
    PostSend(pClientInfo, pClientInfo->recvBuffer, bytesTransferred);
    
    // 다음 수신 작업 시작
    PostRecv(pClientInfo);
}

void ProcessSend(WSAOVERLAPPED* pOverlapped) {
    // 송신 완료 후 메모리 정리
    // 오버랩 구조체에서 버퍼 포인터를 얻어야 함
    // (실제로는 커스텀 오버랩 구조체 사용 권장)
    
    delete pOverlapped;
    // 송신 버퍼도 함께 해제해야 함 (별도 관리 필요)
}
```

##### 2.7 AcceptEx를 이용한 비동기 Accept

```cpp
bool PostAccept() {
    SOCKET acceptSocket = WSASocket(
        AF_INET, SOCK_STREAM, IPPROTO_TCP, 
        NULL, 0, WSA_FLAG_OVERLAPPED
    );
    
    if (acceptSocket == INVALID_SOCKET) {
        return false;
    }
    
    // AcceptEx용 오버랩 구조체
    WSAOVERLAPPED* pAcceptOverlapped = new WSAOVERLAPPED;
    ZeroMemory(pAcceptOverlapped, sizeof(WSAOVERLAPPED));
    
    // AcceptEx용 버퍼 (주소 정보 저장용)
    char* pAcceptBuffer = new char[1024];
    
    DWORD bytesReceived = 0;
    BOOL result = AcceptEx(
        m_listenSocket,              // 리슨 소켓
        acceptSocket,                // 새 클라이언트 소켓
        pAcceptBuffer,               // 주소 정보 버퍼
        0,                           // 데이터 수신 길이 (0 = 연결만)
        sizeof(SOCKADDR_IN) + 16,    // 로컬 주소 크기
        sizeof(SOCKADDR_IN) + 16,    // 원격 주소 크기
        &bytesReceived,              // 수신된 바이트
        pAcceptOverlapped            // 오버랩 구조체
    );
    
    if (!result) {
        int error = WSAGetLastError();
        if (error != WSA_IO_PENDING) {
            printf("AcceptEx 실패: %d\n", error);
            closesocket(acceptSocket);
            delete pAcceptOverlapped;
            delete[] pAcceptBuffer;
            return false;
        }
    }
    
    return true;
}
```

#### 3. 고급 활용 팁

##### 3.1 커스텀 오버랩 구조체

```cpp
// 확장된 오버랩 구조체
struct ExtendedOverlapped {
    WSAOVERLAPPED overlapped;
    enum IOType { IO_RECV, IO_SEND, IO_ACCEPT } ioType;
    char* buffer;
    int bufferLen;
    ClientInfo* pClientInfo;
};
```

##### 3.2 메모리 풀 사용

```cpp
class MemoryPool {
private:
    std::queue<ClientInfo*> m_clientPool;
    std::mutex m_mutex;
    
public:
    ClientInfo* Allocate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_clientPool.empty()) {
            return new ClientInfo;
        }
        ClientInfo* pClient = m_clientPool.front();
        m_clientPool.pop();
        return pClient;
    }
    
    void Deallocate(ClientInfo* pClient) {
        std::lock_guard<std::mutex> lock(m_mutex);
        ZeroMemory(pClient, sizeof(ClientInfo));
        m_clientPool.push(pClient);
    }
};
```

##### 3.3 PostQueuedCompletionStatus 활용

```cpp
// 서버 종료시 워커 스레드들에게 종료 신호 전송
void Shutdown() {
    for (size_t i = 0; i < m_workerThreads.size(); ++i) {
        PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
    }
    
    // 모든 워커 스레드 종료 대기
    WaitForMultipleObjects(
        m_workerThreads.size(), 
        m_workerThreads.data(), 
        TRUE, 
        INFINITE
    );
}
```


#### IOCP 예제 코드
  
![](./images/055.png)  
![](./images/056.png)

`codes/first_IOCP`에는 아래와 코드는 동일하지만 헤더와 cpp로 파일이 분리 되어 있습니다.  

```cpp
#include <iostream>
#include <winsock2.h> // 윈도우 소켓 프로그래밍 기본 헤더
#include <ws2tcpip.h> // TCP/IP 프로토콜 관련 헤더
#include <mswsock.h>  // AcceptEx 등 확장 함수 헤더
#include <windows.h>  // 윈도우 API 기본 헤더
#include <vector>     // 동적 배열 사용
#include <string>     // 문자열 사용
#include <thread>     // 스레드 사용
#include <memory>     // 스마트 포인터 사용 (shared_ptr)
#include <format>     // C++20 문자열 포매팅

// 라이브러리 링크
#pragma comment(lib, "ws2_32.lib") // Winsock2 라이브러리
#pragma comment(lib, "mswsock.lib") // Mswsock 라이브러리 (AcceptEx 등)

// 상수 정의
constexpr int BUFFER_SIZE = 4096; // I/O 작업 시 사용할 버퍼 크기

// IOCP 연산 타입 정의 (어떤 종류의 I/O 작업인지 구분)
enum class IOType {
    ACCEPT, // 클라이언트 접속 요청 수락
    RECV,   // 데이터 수신
    SEND    // 데이터 송신
};

// IOCP에서 사용할 확장 OVERLAPPED 구조체
// 비동기 I/O 작업의 상태와 정보를 담음
struct IOContext {
    OVERLAPPED overlapped;      // 비동기 I/O 작업 상태를 저장하는 구조체 (필수)
    WSABUF wsaBuf;              // 데이터 버퍼 정보 (버퍼 포인터, 길이)
    IOType ioType;              // I/O 작업 종류 (ACCEPT, RECV, SEND)
    SOCKET socket;              // 작업과 관련된 소켓 (주로 Accept 작업 시 사용)
    char buffer[BUFFER_SIZE];   // 실제 데이터가 저장되는 버퍼

    // 생성자: I/O 타입과 소켓 초기화, 메모리 초기화
    IOContext(IOType type) : ioType(type), socket(INVALID_SOCKET) {
        ZeroMemory(&overlapped, sizeof(OVERLAPPED)); // overlapped 구조체 0으로 초기화
        ZeroMemory(buffer, BUFFER_SIZE);            // 데이터 버퍼 0으로 초기화
        wsaBuf.buf = buffer;                        // wsaBuf의 버퍼 포인터를 내부 버퍼로 설정
        wsaBuf.len = BUFFER_SIZE;                   // wsaBuf의 길이를 버퍼 크기로 설정
    }

    // 소멸자: 소켓이 유효하면 닫음
    ~IOContext() {
        if (socket != INVALID_SOCKET) {
            closesocket(socket);
            socket = INVALID_SOCKET;
        }
    }
};

// 클라이언트 정보 구조체
// 연결된 각 클라이언트의 정보를 관리
struct ClientInfo {
    SOCKET socket;                          // 클라이언트와 통신하는 소켓
    SOCKADDR_IN clientAddr;                 // 클라이언트 주소 정보
    std::shared_ptr<IOContext> recvContext; // 데이터 수신용 IOContext
    std::shared_ptr<IOContext> sendContext; // 데이터 송신용 IOContext

    // 생성자: 소켓 및 주소 정보 초기화
    ClientInfo() : socket(INVALID_SOCKET) {
        ZeroMemory(&clientAddr, sizeof(clientAddr));
    }

    // 소멸자: 소켓이 유효하면 닫음
    ~ClientInfo() {
        if (socket != INVALID_SOCKET) {
            closesocket(socket);
            socket = INVALID_SOCKET;
        }
    }
};

// IOCP 서버 클래스
class IOCPServer {
private:
    SOCKET listenSocket;                   // 클라이언트 연결 요청을 받는 리슨 소켓
    HANDLE iocpHandle;                     // IOCP 커널 객체 핸들
    std::vector<std::thread> workerThreads; // I/O 작업을 처리하는 워커 스레드들
    bool isRunning;                        // 서버 실행 상태 플래그
    LPFN_ACCEPTEX lpfnAcceptEx;             // AcceptEx 함수 포인터 (동적으로 로드)

public:
    // 생성자: 멤버 변수 초기화
    IOCPServer() : listenSocket(INVALID_SOCKET), iocpHandle(NULL), isRunning(false), lpfnAcceptEx(nullptr) {}

    // 소멸자: 서버 중지 및 자원 해제
    ~IOCPServer() {
        Stop(); // 서버 실행 중지
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket); // 리슨 소켓 닫기
        }
        if (iocpHandle != NULL) {
            CloseHandle(iocpHandle); // IOCP 핸들 닫기
        }
        WSACleanup(); // Winsock 라이브러리 정리
    }

    // 서버 초기화 함수
    bool Initialize(const std::string& port, int workerThreadCount) {
        WSADATA wsaData;
        // Winsock 라이브러리 초기화 (버전 2.2)
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << std::format("WSAStartup 실패: {}\n", result);
            return false;
        }

        // 리슨 소켓 생성 (TCP, Overlapped I/O 사용)
        listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
            WSACleanup();
            return false;
        }

        // 서버 주소 설정 (IP: 모든 주소, Port: 입력받은 포트)
        SOCKADDR_IN serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 IP 주소에서 오는 연결 허용
        serverAddr.sin_port = htons(std::stoi(port));   // 문자열 포트를 정수로 변환 후 네트워크 바이트 순서로 변경

        // 소켓에 주소 바인딩
        result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("bind 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }

        // AcceptEx 함수 포인터 가져오기 (확장 함수이므로 동적으로 로드)
        GUID guidAcceptEx = WSAID_ACCEPTEX;
        DWORD dwBytes = 0;
        result = WSAIoctl(
            listenSocket,
            SIO_GET_EXTENSION_FUNCTION_POINTER, // 확장 함수 포인터를 가져오는 제어 코드
            &guidAcceptEx,                      // AcceptEx 함수의 GUID
            sizeof(guidAcceptEx),
            &lpfnAcceptEx,                      // AcceptEx 함수 포인터를 저장할 변수
            sizeof(lpfnAcceptEx),
            &dwBytes,
            NULL,
            NULL
        );
        if (result == SOCKET_ERROR) {
            std::cerr << std::format("AcceptEx 함수 포인터 얻기 실패: {}\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }

        // IOCP 커널 객체 생성
        // 첫 번째 인자: INVALID_HANDLE_VALUE (새 IOCP 생성)
        // 세 번째 인자: CompletionKey (여기서는 0)
        // 네 번째 인자: 동시 실행 가능한 스레드 수 (0이면 CPU 코어 수만큼)
        iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if (iocpHandle == NULL) {
            std::cerr << std::format("IOCP 생성 실패: {}\n", GetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }

        // 리슨 소켓을 IOCP에 연결 (등록)
        // 이렇게 하면 리슨 소켓에서 발생하는 I/O 완료 이벤트도 IOCP 큐로 들어감
        // 세 번째 인자 (CompletionKey): 여기서는 리슨 소켓을 구분할 필요가 없으므로 nullptr 또는 0 사용
        if (CreateIoCompletionPort((HANDLE)listenSocket, iocpHandle, (ULONG_PTR)nullptr, 0) == NULL) {
            std::cerr << std::format("리슨 소켓을 IOCP에 연결 실패: {}\n", GetLastError());
            CloseHandle(iocpHandle);
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }

        // 워커 스레드 생성 및 시작
        isRunning = true;
        for (int i = 0; i < workerThreadCount; i++) {
            // 각 스레드는 WorkerThread 함수를 실행
            workerThreads.emplace_back(&IOCPServer::WorkerThread, this);
        }

        return true;
    }

    // 서버 시작 함수
    void Start() {
        // 리슨 소켓을 통해 클라이언트 연결 대기 시작
        // SOMAXCONN: 시스템이 허용하는 최대 연결 대기 큐 크기
        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << std::format("listen 실패: {}\n", WSAGetLastError());
            return;
        }

        std::cout << "서버가 시작되었습니다. 클라이언트 대기 중...\n";

        // 초기 Accept 작업 등록 (여러 개의 Accept 요청을 미리 걸어둠)
        // 서버 시작 시 미리 여러 개의 클라이언트 연결을 받을 준비를 해두어 성능 향상
        for (int i = 0; i < 10; i++) { // 예시로 10개
            PostAccept();
        }
    }

    // 서버 중지 함수
    void Stop() {
        isRunning = false; // 워커 스레드 루프 종료 조건 변경

        // 모든 워커 스레드에 종료 신호 전송
        // PostQueuedCompletionStatus를 사용하여 워커 스레드의 GetQueuedCompletionStatus 함수를 깨움
        // CompletionKey를 0으로 설정하여 종료 신호임을 알림 (사용자 정의)
        for (size_t i = 0; i < workerThreads.size(); i++) {
            PostQueuedCompletionStatus(iocpHandle, 0, 0, NULL);
        }

        // 모든 워커 스레드가 종료될 때까지 대기
        for (auto& thread : workerThreads) {
            if (thread.joinable()) { // 스레드가 아직 실행 중이고 join 가능하면
                thread.join();       // 스레드 종료 대기
            }
        }
        workerThreads.clear(); // 스레드 벡터 비우기
    }

private:
    // 워커 스레드 함수 (IOCP에서 완료된 I/O 작업을 처리)
    void WorkerThread() {
        while (isRunning) { // 서버 실행 중인 동안 반복
            DWORD bytesTransferred = 0;   // 전송된 바이트 수
            ULONG_PTR completionKey = 0;  // 완료된 I/O 작업과 관련된 키 (여기서는 ClientInfo 포인터 또는 0)
            LPOVERLAPPED pOverlapped = nullptr; // 완료된 I/O 작업의 OVERLAPPED 구조체 포인터

            // IOCP 큐에서 완료된 작업 가져오기 (블로킹 함수, 작업이 완료될 때까지 대기)
            // INFINITE: 완료된 작업이 있을 때까지 무한정 대기
            BOOL result = GetQueuedCompletionStatus(
                iocpHandle,
                &bytesTransferred,
                &completionKey,
                &pOverlapped,
                INFINITE
            );

            // 서버 종료 신호 확인 (Stop 함수에서 PostQueuedCompletionStatus로 보낸 신호)
            if (!isRunning) {
                break; // 루프 탈출하여 스레드 종료
            }

            // 오류 또는 클라이언트 연결 종료 확인
            // result가 FALSE이거나, bytesTransferred가 0이면 I/O 오류 또는 정상적인 연결 종료
            if (!result || (bytesTransferred == 0 && pOverlapped != nullptr)) {
                if (pOverlapped != nullptr) {
                    // 클라이언트 연결 종료 또는 오류 처리
                    IOContext* pIOContext = CONTAINING_RECORD(pOverlapped, IOContext, overlapped);

                    if (!result) { // I/O 작업 자체에서 오류 발생
                        int error = GetLastError();
                        // ERROR_OPERATION_ABORTED (995)는 소켓이 닫힐 때 흔히 발생하므로,
                        // 필요에 따라 특정 오류 코드는 무시할 수 있음
                        if (error != ERROR_OPERATION_ABORTED) {
                           std::cerr << std::format("I/O 작업 실패 (GetQueuedCompletionStatus): {}\n", error);
                        }
                    }
                     // bytesTransferred == 0 이면 정상적인 연결 종료 (FIN 패킷 수신)

                    // 클라이언트 정보 정리
                    // completionKey에는 ClientInfo 포인터가 저장되어 있음 (Accept 성공 시)
                    ClientInfo* pClientInfo = (ClientInfo*)completionKey;
                    if (pClientInfo != nullptr) {
                        std::cout << std::format("클라이언트 연결 종료 또는 오류: 소켓 {}\n", (int)pClientInfo->socket);
                        delete pClientInfo; // ClientInfo 객체 메모리 해제
                                            // ClientInfo 소멸자에서 소켓 자동 닫힘
                    } else {
                         // completionKey가 nullptr인 경우 (예: 리슨 소켓 관련 이벤트, 현재 코드에서는 특별히 처리 안 함)
                         // 또는 Accept 실패 후 정리 과정에서 pIOContext만 있는 경우
                        if(pIOContext->ioType == IOType::ACCEPT && pIOContext->socket != INVALID_SOCKET) {
                            // AcceptEx에 사용된 소켓은 여기서 닫아줘야 할 수 있음.
                            // 하지만通常은 pClientInfo가 nullptr이면서 pOverlapped가 유효한 경우는 드묾.
                            // (AcceptEx 실패 시 PostAccept에서 이미 closesocket 처리)
                            std::cout << std::format("Accept 작업 관련 소켓 정리: {}\n", (int)pIOContext->socket);
                            // closesocket(pIOContext->socket); // 이미 IOContext 소멸자에서 처리될 수 있음
                        }
                    }
                    // IOContext 객체는 shared_ptr로 관리되므로, 해당 포인터를 직접 delete하지 않음.
                    // 만약 IOContext가 new로 할당되었다면 여기서 delete 필요. (현재 코드는 shared_ptr)
                }
                continue; // 다음 완료된 작업 처리
            }

            // I/O 컨텍스트 및 클라이언트 정보 가져오기
            // pOverlapped 포인터로부터 IOContext 구조체의 시작 주소를 계산
            IOContext* pIOContext = CONTAINING_RECORD(pOverlapped, IOContext, overlapped);
            ClientInfo* pClientInfo = (ClientInfo*)completionKey; // Accept 성공 시 ClientInfo 포인터

            // I/O 작업 타입에 따른 처리
            switch (pIOContext->ioType) {
                case IOType::ACCEPT: // 클라이언트 연결 수락 완료
                    HandleAccept(pIOContext);
                    break;

                case IOType::RECV:   // 데이터 수신 완료
                    if (pClientInfo) HandleRecv(pClientInfo, bytesTransferred);
                    break;

                case IOType::SEND:   // 데이터 송신 완료
                    if (pClientInfo) HandleSend(pClientInfo, bytesTransferred);
                    break;
            }
        }
    }

    // Accept 작업을 IOCP에 등록하는 함수
    void PostAccept() {
        // 새 클라이언트 연결을 위한 소켓 생성 (Overlapped I/O)
        SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (acceptSocket == INVALID_SOCKET) {
            std::cerr << std::format("AcceptSocket 생성 실패: {}\n", WSAGetLastError());
            return;
        }

        // Accept 작업을 위한 IOContext 생성 (스마트 포인터로 관리)
        // IOContext는 AcceptEx 호출 시 OVERLAPPED 구조체로 사용됨
        // 이 IOContext는 Accept 작업이 완료될 때까지 살아 있어야 함.
        std::shared_ptr<IOContext> acceptContext = std::make_shared<IOContext>(IOType::ACCEPT);
        acceptContext->socket = acceptSocket; // 생성된 acceptSocket을 IOContext에 저장

        DWORD bytesReceived = 0; // AcceptEx에서는 사용되지 않지만, 형식상 필요
        // 로컬 및 원격 주소 정보를 받기 위한 버퍼 공간.
        // AcceptEx는 첫 번째 데이터도 함께 받을 수 있지만, 여기서는 주소 정보만 받도록 설정 (dwReceiveDataLength = 0).
        // 로컬 주소 길이 + 원격 주소 길이 + 약간의 여유 공간
        int addrLen = sizeof(SOCKADDR_IN) + 16;

        // 비동기 AcceptEx 호출
        // dwReceiveDataLength: 연결 수락 시 함께 받을 데이터 크기 (0이면 데이터 안 받음)
        // lpOutputBuffer: 로컬 주소, 원격 주소, (선택적) 초기 데이터가 저장될 버퍼
        // overlapped: 비동기 작업을 위한 OVERLAPPED 구조체 포인터
        BOOL result = lpfnAcceptEx(
            listenSocket,                      // 리슨 소켓
            acceptSocket,                      // 새로 연결될 클라이언트 소켓
            acceptContext->buffer,             // 주소 정보를 받을 버퍼 (IOContext 내부 버퍼 사용)
            0,                                 // 초기 수신 데이터 크기 (0으로 설정)
            addrLen,                           // 로컬 주소 버퍼 크기
            addrLen,                           // 원격 주소 버퍼 크기
            &bytesReceived,                    // 실제로 수신된 바이트 (여기서는 의미 없음)
            &acceptContext->overlapped         // 비동기 I/O를 위한 OVERLAPPED 구조체
        );

        if (result == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
            // ERROR_IO_PENDING: 작업이 비동기적으로 진행 중임을 의미 (정상)
            // 그 외의 오류는 AcceptEx 호출 실패
            std::cerr << std::format("AcceptEx 실패: {}\n", WSAGetLastError());
            closesocket(acceptSocket); // 실패했으므로 accept 소켓 닫기
            // acceptContext는 shared_ptr이므로 스코프 벗어나면 자동 해제
            return;
        }
        // 성공 또는 ERROR_IO_PENDING이면 작업이 IOCP에 등록된 것임.
        // 완료되면 WorkerThread의 GetQueuedCompletionStatus에서 감지됨.
        // 이때 CompletionKey는 리슨 소켓에 연결된 것이므로 ClientInfo가 아님 (여기서는 nullptr).
        // HandleAccept에서 새로운 ClientInfo를 생성하고 IOCP에 등록할 때 ClientInfo 포인터를 CompletionKey로 사용.
    }

    // 클라이언트 연결 수락 완료 처리 함수
    void HandleAccept(IOContext* pIOContext) {
        // pIOContext->socket에 새로 연결된 클라이언트 소켓이 들어있음.
        SOCKET clientSocket = pIOContext->socket;

        // 새로운 클라이언트 정보 생성
        ClientInfo* pClientInfo = new ClientInfo(); // 나중에 delete 필요
        pClientInfo->socket = clientSocket;
        // pIOContext의 socket은 이제 ClientInfo가 관리하므로, IOContext에서는 더 이상 관리하지 않도록 INVALID_SOCKET으로 설정.
        // 이렇게 하지 않으면 IOContext 소멸 시 clientSocket이 또 닫힐 수 있음.
        pIOContext->socket = INVALID_SOCKET;


        // 소켓 옵션 설정 (SO_UPDATE_ACCEPT_CONTEXT)
        // AcceptEx로 생성된 소켓은 리슨 소켓의 속성을 상속받으므로,
        // 해당 소켓을 독립적으로 사용하기 위해 컨텍스트 업데이트 필요
        if (setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                       (char*)&listenSocket, sizeof(listenSocket)) == SOCKET_ERROR) {
            std::cerr << std::format("setsockopt(SO_UPDATE_ACCEPT_CONTEXT) 실패: {}\n", WSAGetLastError());
            delete pClientInfo; // 실패 시 ClientInfo 메모리 해제
            closesocket(clientSocket); // 클라이언트 소켓 닫기

            PostAccept(); // 다음 연결 요청을 위해 새 Accept 작업 등록
            return;
        }

        // 클라이언트 정보 초기화 (Recv, Send를 위한 IOContext 생성)
        pClientInfo->recvContext = std::make_shared<IOContext>(IOType::RECV);
        pClientInfo->recvContext->socket = clientSocket; // Recv 작업용 소켓 설정
        pClientInfo->sendContext = std::make_shared<IOContext>(IOType::SEND);
        pClientInfo->sendContext->socket = clientSocket; // Send 작업용 소켓 설정

        // 새로 연결된 클라이언트 소켓을 IOCP에 연결 (등록)
        // 세 번째 인자 (CompletionKey): 이 클라이언트 소켓과 관련된 I/O 작업 완료 시
        // GetQueuedCompletionStatus에서 반환될 값. ClientInfo 객체의 포인터를 전달하여
        // 어떤 클라이언트의 작업인지 식별할 수 있도록 함.
        if (CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)pClientInfo, 0) == NULL) {
            std::cerr << std::format("클라이언트 소켓을 IOCP에 연결 실패: {}\n", GetLastError());
            delete pClientInfo;
            closesocket(clientSocket);

            PostAccept();
            return;
        }

        // 새 클라이언트 연결 정보 출력
        int clientAddrLen = sizeof(pClientInfo->clientAddr);
        // 연결된 클라이언트의 주소 정보 가져오기
        getpeername(clientSocket, (SOCKADDR*)&pClientInfo->clientAddr, &clientAddrLen);

        char clientIP[INET_ADDRSTRLEN]; // IPv4 주소 문자열 저장 버퍼
        // 숫자 형태의 IP 주소를 문자열 형태로 변환
        inet_ntop(AF_INET, &(pClientInfo->clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        unsigned short clientPort = ntohs(pClientInfo->clientAddr.sin_port); // 네트워크 바이트 순서를 호스트 바이트 순서로

        std::cout << std::format("새 클라이언트 연결: {}:{}\n", clientIP, clientPort);

        // 첫 데이터 수신 작업 등록
        PostRecv(pClientInfo);

        // 다음 클라이언트 연결 요청을 받기 위해 또 다른 Accept 작업 등록
        PostAccept();
    }

    // 데이터 수신 작업을 IOCP에 등록하는 함수
    void PostRecv(ClientInfo* pClientInfo) {
        DWORD flags = 0;         // 수신 작업 플래그 (보통 0)
        DWORD bytesRecvd = 0;    // 실제로 수신된 바이트 수 (비동기 호출 시에는 의미 없음)

        // WSARecv 호출하여 비동기 데이터 수신 시작
        // pClientInfo->recvContext->wsaBuf: 수신 데이터를 저장할 버퍼 정보
        // &pClientInfo->recvContext->overlapped: 비동기 작업을 위한 OVERLAPPED 구조체
        int result = WSARecv(
            pClientInfo->socket,
            &pClientInfo->recvContext->wsaBuf, // WSABUF 배열 (여기서는 1개)
            1,                                 // WSABUF 배열의 크기
            &bytesRecvd,                       // 수신된 바이트 수 (비동기 완료 시 채워짐)
            &flags,                            // 플래그 (MSG_PARTIAL 등)
            &pClientInfo->recvContext->overlapped, // OVERLAPPED 구조체
            NULL                               // 완료 루틴 (여기서는 사용 안 함)
        );

        if (result == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
            // ERROR_IO_PENDING: 작업이 비동기적으로 진행 중 (정상)
            // 그 외의 오류는 WSARecv 호출 실패
            std::cerr << std::format("WSARecv 실패 ({}:{}): {}\n",
                inet_ntoa(pClientInfo->clientAddr.sin_addr), ntohs(pClientInfo->clientAddr.sin_port), WSAGetLastError());
            // 연결 종료 처리 (ClientInfo 객체 삭제 -> 소켓 자동 닫힘)
            delete pClientInfo;
        }
        // 성공 또는 ERROR_IO_PENDING이면 작업이 IOCP에 등록됨.
        // 완료되면 WorkerThread에서 감지. CompletionKey는 pClientInfo.
    }

    // 데이터 수신 완료 처리 함수
    void HandleRecv(ClientInfo* pClientInfo, DWORD bytesTransferred) {
        if (bytesTransferred == 0) {
            // 클라이언트가 정상적으로 연결을 종료함 (FIN 패킷 수신)
            std::cout << std::format("클라이언트 연결 정상 종료 (Recv 0 byte): {}:{}\n",
                inet_ntoa(pClientInfo->clientAddr.sin_addr), ntohs(pClientInfo->clientAddr.sin_port));
            delete pClientInfo; // ClientInfo 정리
            return;
        }

        // 수신된 데이터 처리 (버퍼 끝에 NULL 문자 추가하여 문자열로 만듦)
        pClientInfo->recvContext->buffer[bytesTransferred] = '\0';
        std::cout << std::format("데이터 수신 ({}:{} {}바이트): {}\n",
            inet_ntoa(pClientInfo->clientAddr.sin_addr), ntohs(pClientInfo->clientAddr.sin_port),
            bytesTransferred, pClientInfo->recvContext->buffer);

        // 에코 서버: 받은 데이터 그대로 전송하기 위해 send 버퍼에 복사
        memcpy(pClientInfo->sendContext->buffer, pClientInfo->recvContext->buffer, bytesTransferred);
        pClientInfo->sendContext->wsaBuf.len = bytesTransferred; // 전송할 데이터 길이 설정

        // 데이터 송신 작업 등록
        PostSend(pClientInfo, bytesTransferred);
    }

    // 데이터 송신 작업을 IOCP에 등록하는 함수
    void PostSend(ClientInfo* pClientInfo, DWORD bytesToSend) {
        DWORD flags = 0;         // 송신 작업 플래그 (보통 0)
        DWORD bytesSent = 0;     // 실제로 송신된 바이트 수 (비동기 호출 시에는 의미 없음)

        // WSASend 호출하여 비동기 데이터 송신 시작
        // pClientInfo->sendContext->wsaBuf: 송신할 데이터가 담긴 버퍼 정보
        // &pClientInfo->sendContext->overlapped: 비동기 작업을 위한 OVERLAPPED 구조체
        int result = WSASend(
            pClientInfo->socket,
            &pClientInfo->sendContext->wsaBuf, // WSABUF 배열 (여기서는 1개)
            1,                                 // WSABUF 배열의 크기
            &bytesSent,                        // 송신된 바이트 수 (비동기 완료 시 채워짐)
            flags,                             // 플래그
            &pClientInfo->sendContext->overlapped, // OVERLAPPED 구조체
            NULL                               // 완료 루틴 (여기서는 사용 안 함)
        );

        if (result == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
            // ERROR_IO_PENDING: 작업이 비동기적으로 진행 중 (정상)
            // 그 외의 오류는 WSASend 호출 실패
            std::cerr << std::format("WSASend 실패 ({}:{}): {}\n",
                inet_ntoa(pClientInfo->clientAddr.sin_addr), ntohs(pClientInfo->clientAddr.sin_port), WSAGetLastError());
            // 연결 종료 처리
            delete pClientInfo;
        }
        // 성공 또는 ERROR_IO_PENDING이면 작업이 IOCP에 등록됨.
        // 완료되면 WorkerThread에서 감지. CompletionKey는 pClientInfo.
    }

    // 데이터 송신 완료 처리 함수
    void HandleSend(ClientInfo* pClientInfo, DWORD bytesTransferred) {
        std::cout << std::format("데이터 전송 완료 ({}:{} {}바이트)\n",
            inet_ntoa(pClientInfo->clientAddr.sin_addr), ntohs(pClientInfo->clientAddr.sin_port),
            bytesTransferred);

        // 중요: Send 완료 후 Recv 버퍼를 다시 사용할 수 있도록 초기화
        pClientInfo->recvContext->wsaBuf.len = BUFFER_SIZE;
        ZeroMemory(pClientInfo->recvContext->buffer, BUFFER_SIZE);

        // 다음 데이터 수신 작업 등록 (에코 서버이므로 계속해서 수신 대기)
        PostRecv(pClientInfo);
    }
};
```  
  
#### IOCP의 장단점
**장점:**
- 매우 높은 확장성과 성능
- 효율적인 스레드 관리 (CPU 코어 수에 맞게 스레드 풀 운영)
- 다양한 I/O 모델과 통합 가능
- 대규모 동시 연결 처리에 적합

**단점:**
- 구현이 복잡함
- 윈도우 플랫폼에서만 사용 가능
- 디버깅이 상대적으로 어려움
  

  
#### 단계 별로 IOCP 실습하기
간단한 코드를 시작으로 채팅 서버까지 만들어보는 학습 자료를 소개한다.  
- [GitHub](https://github.com/jacking75/edu_cpp_IOCP )  
    - [YOUTUBE](https://www.youtube.com/watch?v=RMRsvll7hrM&list=PLW_xyUw4fSdbYjgwC-JCCFWznhayrZv77 )
    - [학습가이드](https://github.com/jacking75/edu_cpp_IOCP/blob/master/iocp_learning_guide.md  )  
    - [DeepWiki](https://deepwiki.com/jacking75/edu_cpp_IOCP/1-overview )


## 04 소켓 입출력 모델 비교
다양한 소켓 입출력 모델의 특성을 비교해보겠습니다.

### 성능 및 확장성 비교

| 모델 | 확장성 | CPU 사용률 | 메모리 사용량 | 구현 복잡도 |
|------|--------|------------|--------------|------------|
| 블로킹 | 매우 낮음 | 높음 (스레드 다수) | 높음 | 매우 낮음 |
| 넌블로킹 | 낮음 | 매우 높음 (폴링) | 낮음 | 낮음 |
| Select | 중간 (64개 제한) | 중간 | 낮음 | 중간 |
| WSAAsyncSelect | 중간 | 중간 | 낮음 | 중간 |
| IOCP | 매우 높음 | 낮음 | 중간 | 높음 |

### 적합한 사용 시나리오
- **블로킹 소켓**: 간단한 클라이언트, 저사양 서버
- **넌블로킹 소켓**: 단일 스레드에서 소수의 연결 처리
- **Select 모델**: 중소규모 서버, 크로스 플랫폼 필요 시
- **IOCP 모델**: 고성능 온라인 게임 서버, 대규모 웹 서버
  
 

<br>      
  
# Chapter.10 select 방식의 채팅 서버
- [GitHub](https://github.com/heungbae-com2us/SelectChatServer )  
- [DeepWiki](https://deepwiki.com/heungbae-com2us/SelectChatServer/1-overview )  
- `code/SelectChatServer` 에도 소스가 있다.
- YOUTUBE 
    - [1] (https://www.youtube.com/watch?v=OJZmDPO6FIA&list=PLW_xyUw4fSdYJuxJaMx3k32xTN3Ou8aDy&index=4 )   
    - [2](https://www.youtube.com/watch?v=q9hsy_RK6Ss&list=PLW_xyUw4fSdYJuxJaMx3k32xTN3Ou8aDy&index=8 )
    - [C++ 채팅서버 클라이언트(C#,Winform) 코드 설명](https://www.youtube.com/watch?v=V1Lpned5NvA&list=PLW_xyUw4fSdYJuxJaMx3k32xTN3Ou8aDy )
           

## 1. 전체 아키텍처 개요
이 프로젝트는 **select 기반의 TCP 네트워크**를 사용하는 멀티플레이어 채팅 서버입니다. 게임 서버 개발의 기본 개념들을 학습할 수 있도록 설계되었습니다.

### 1.1 프로젝트 구조

```
SelectChatServer-main/
├── ChatServer2/           # Chat 서버 코드
│   ├── ConsoleHost/       # 서버 실행 진입점
│   ├── LogicLib/          # 게임 로직 라이브러리  
│   └── ServerNetLib/      # 네트워크 라이브러리
├── ChatClient/            # C# 클라이언트
├── Common/                # 공통 헤더 (패킷, 에러코드)
└── Bin/                   # 설정 파일
```

### 1.2 시스템 아키텍처
  
![SelectChatServer 아키텍처](./images/060.png)  
  
  
## 2. 서버 구조 상세 분석

### 2.1 Main 클래스 - 서버 진입점

```cpp
// ChatServer2/LogicLib/Main.cpp
class Main
{
private:
    std::unique_ptr<NServerNetLib::ITcpNetwork> m_pNetwork;    // 네트워크 계층
    std::unique_ptr<PacketProcess> m_pPacketProc;              // 패킷 처리
    std::unique_ptr<UserManager> m_pUserMgr;                   // 유저 관리
    std::unique_ptr<LobbyManager> m_pLobbyMgr;                 // 로비 관리
};
```

**Main 클래스의 역할:**
- 서버 초기화 및 설정 로드
- 각 매니저 클래스들의 생명주기 관리
- 메인 게임 루프 실행

**초기화 과정:**
1. 설정 파일 로드 (포트, 최대 사용자 수 등)
2. 네트워크 라이브러리 초기화
3. 매니저 클래스들 초기화
4. 패킷 처리기 설정
   

#### 1. Main 클래스 다이어그램
![Main 클래스 다이어그램](./images/067.png)  

#### 2. Main 클래스 상세 분석

##### 2.1 멤버 변수 분석

```cpp
class Main
{
private:
    bool m_IsRun = false;  // 서버 실행 상태 플래그
    
    // 핵심 컴포넌트들 (스마트 포인터로 관리)
    std::unique_ptr<NServerNetLib::ServerConfig> m_pServerConfig;   // 서버 설정
    std::unique_ptr<NServerNetLib::ILog> m_pLogger;                 // 로깅 시스템
    std::unique_ptr<NServerNetLib::ITcpNetwork> m_pNetwork;         // 네트워크 계층
    std::unique_ptr<PacketProcess> m_pPacketProc;                   // 패킷 처리기
    std::unique_ptr<UserManager> m_pUserMgr;                        // 사용자 관리자
    std::unique_ptr<LobbyManager> m_pLobbyMgr;                      // 로비 관리자
};
```

**스마트 포인터 사용 이유:**
- **자동 메모리 관리**: 객체의 생명주기 자동 관리
- **예외 안전성**: 예외 발생 시 자동으로 메모리 해제
- **소유권 명확화**: unique_ptr로 소유권을 명확히 표현

##### 2.2 초기화 과정 (Init 메서드)

```cpp
ERROR_CODE Main::Init()
{
    // 1. 로거 초기화
    m_pLogger = std::make_unique<ConsoleLog>();
    
    // 2. 설정 파일 로드
    LoadConfig();
    
    // 3. 네트워크 계층 초기화
    m_pNetwork = std::make_unique<NServerNetLib::TcpNetwork>();
    auto result = m_pNetwork->Init(m_pServerConfig.get(), m_pLogger.get());
    if (result != NET_ERROR_CODE::NONE) {
        return ERROR_CODE::MAIN_INIT_NETWORK_INIT_FAIL;
    }
    
    // 4. 사용자 관리자 초기화
    m_pUserMgr = std::make_unique<UserManager>();
    m_pUserMgr->Init(m_pServerConfig->MaxClientCount);
    
    // 5. 로비 관리자 초기화
    m_pLobbyMgr = std::make_unique<LobbyManager>();
    m_pLobbyMgr->Init({
        m_pServerConfig->MaxLobbyCount,
        m_pServerConfig->MaxLobbyUserCount,
        m_pServerConfig->MaxRoomCountByLobby,
        m_pServerConfig->MaxRoomUserCount
    }, m_pNetwork.get(), m_pLogger.get());
    
    // 6. 패킷 처리기 초기화
    m_pPacketProc = std::make_unique<PacketProcess>();
    m_pPacketProc->Init(m_pNetwork.get(), m_pUserMgr.get(), 
                        m_pLobbyMgr.get(), m_pServerConfig.get(), m_pLogger.get());
    
    m_IsRun = true;
    return ERROR_CODE::NONE;
}
```

**초기화 순서:**  
![초기화 순서도](./images/068.png)  

  
##### 2.3 설정 로드 (LoadConfig 메서드)

```cpp
ERROR_CODE Main::LoadConfig()
{
    m_pServerConfig = std::make_unique<NServerNetLib::ServerConfig>();
    
    // 하드코딩된 설정값들 (실제로는 파일에서 읽어야 함)
    m_pServerConfig->Port = 32452;                              // 서버 포트
    m_pServerConfig->BackLogCount = 128;                        // 백로그 큐 크기
    m_pServerConfig->MaxClientCount = 1000;                     // 최대 클라이언트 수
    
    // 버퍼 크기 설정
    m_pServerConfig->MaxClientSockOptRecvBufferSize = 10240;    // 소켓 수신 버퍼
    m_pServerConfig->MaxClientSockOptSendBufferSize = 10240;    // 소켓 송신 버퍼
    m_pServerConfig->MaxClientRecvBufferSize = 8192;            // 어플리케이션 수신 버퍼
    m_pServerConfig->MaxClientSendBufferSize = 8192;            // 어플리케이션 송신 버퍼
    
    // 로그인 체크 설정
    m_pServerConfig->IsLoginCheck = false;                      // 로그인 타임아웃 체크 여부
    
    // 로비/방 설정
    m_pServerConfig->ExtraClientCount = 64;                     // 여분 클라이언트 수
    m_pServerConfig->MaxLobbyCount = 2;                         // 최대 로비 수
    m_pServerConfig->MaxRoomCountByLobby = 20;                  // 로비당 최대 방 수
    m_pServerConfig->MaxRoomUserCount = 4;                      // 방당 최대 유저 수
    
    return ERROR_CODE::NONE;
}
```

**서버 설정 상세:**   
![서버 설정 상세](./images/069.png)    
  
  
##### 2.4 메인 루프 (Run 메서드)

```cpp
void Main::Run()
{
    while (m_IsRun)
    {
        // 1. 네트워크 이벤트 처리 (select 호출)
        m_pNetwork->Run();
        
        // 2. 수신된 패킷들 처리
        while (true)
        {
            auto packetInfo = m_pNetwork->GetPacketInfo();
            
            if (packetInfo.PacketId == 0) {
                break;  // 처리할 패킷이 없음
            }
            else {
                m_pPacketProc->Process(packetInfo);  // 패킷 처리
            }
        }
        
        // 3. 주기적 상태 체크 (로그인 타임아웃 등)
        m_pPacketProc->StateCheck();
    }
}
```

**메인 루프 처리 흐름:**   
![메인 루프 흐름도](./images/070.png)    
  
 
##### 2.5 종료 처리

```cpp
void Main::Stop()
{
    m_IsRun = false;  // 메인 루프 종료 신호
}

void Main::Release() 
{
    if (m_pNetwork) {
        m_pNetwork->Release();  // 네트워크 리소스 정리
    }
    // unique_ptr들은 자동으로 소멸됨
}

Main::~Main()
{
    Release();  // 소멸자에서 리소스 정리
}
```
  

#### 3. Main 클래스의 설계 특징

##### 3.1 RAII (Resource Acquisition Is Initialization) 패턴

```cpp
// 스마트 포인터를 사용한 자동 리소스 관리
std::unique_ptr<NServerNetLib::ITcpNetwork> m_pNetwork;
std::unique_ptr<PacketProcess> m_pPacketProc;
// ... 기타 등등
```

**장점:**
- 예외 발생 시에도 자동으로 메모리 해제
- 명시적 delete 호출 불필요
- 메모리 누수 방지

##### 3.2 의존성 주입 (Dependency Injection) 패턴

```cpp
// PacketProcess 초기화 시 필요한 의존성들을 주입
m_pPacketProc->Init(
    m_pNetwork.get(),     // 네트워크 계층 주입
    m_pUserMgr.get(),     // 사용자 관리자 주입
    m_pLobbyMgr.get(),    // 로비 관리자 주입
    m_pServerConfig.get(), // 설정 정보 주입
    m_pLogger.get()       // 로거 주입
);
```

**장점:**
- 결합도 감소
- 테스트 용이성 향상
- 모듈 간 독립성 증대

##### 3.3 단일 책임 원칙 (Single Responsibility Principle)

Main 클래스는 다음과 같은 단일 책임을 가집니다:
- **서버 생명주기 관리**: 초기화, 실행, 종료
- **컴포넌트 조합**: 각 하위 시스템들을 조합하여 완전한 서버 구성

각각의 세부 기능은 전문화된 클래스들에게 위임:
- 네트워크 처리 → TcpNetwork
- 사용자 관리 → UserManager  
- 로비 관리 → LobbyManager
- 패킷 처리 → PacketProcess
  
  
### 2.2 네트워크 계층 (ServerNetLib)

#### TcpNetwork 클래스

```cpp
class TcpNetwork : public ITcpNetwork
{
private:
    SOCKET m_ServerSockfd;                              // 서버 소켓
    fd_set m_Readfds;                                   // select용 읽기 소켓 집합
    std::vector<ClientSession> m_ClientSessionPool;     // 클라이언트 세션 풀
    std::deque<RecvPacketInfo> m_PacketQueue;          // 수신 패킷 큐
};
```

**select 기반 네트워크 처리 흐름:**
![Select 기반 네트워크 처리](./images/061.png)  
      
```cpp
void TcpNetwork::Run()
{
    auto read_set = m_Readfds;
    auto write_set = m_Readfds;
    
    // select 호출 - 1ms 타임아웃
    auto selectResult = select(0, &read_set, &write_set, 0, &timeout);
    
    if (selectResult > 0) {
        // 새로운 연결 처리
        if (FD_ISSET(m_ServerSockfd, &read_set)) {
            NewSession();
        }
        
        // 기존 클라이언트들 처리
        RunCheckSelectClients(read_set, write_set);
    }
}
```
  
[TcpNetwork 클래스 다이어그램]
![TcpNetwork 클래스 다이어그램](./images/071.png)    

  
##### 클래스 초기화 (Init 메서드)

```cpp
NET_ERROR_CODE TcpNetwork::Init(const ServerConfig* pConfig, ILog* pLogger)
{
    // 1. 설정 정보 복사
    std::memcpy(&m_Config, pConfig, sizeof(ServerConfig));
    m_pRefLogger = pLogger;

    // 2. 서버 소켓 초기화
    auto initRet = InitServerSocket();
    if (initRet != NET_ERROR_CODE::NONE) {
        return initRet;
    }
    
    // 3. 바인드 및 리슨
    auto bindListenRet = BindListen(pConfig->Port, pConfig->BackLogCount);
    if (bindListenRet != NET_ERROR_CODE::NONE) {
        return bindListenRet;
    }

    // 4. fd_set 초기화
    FD_ZERO(&m_Readfds);
    FD_SET(m_ServerSockfd, &m_Readfds);
    
    // 5. 클라이언트 세션 풀 생성
    auto sessionPoolSize = CreateSessionPool(pConfig->MaxClientCount + pConfig->ExtraClientCount);
    
    return NET_ERROR_CODE::NONE;
}
```

**초기화 과정 상세:**   
![TcpNetwork 초기화 과정](./images/072.png)  
  
##### 메인 네트워크 루프 (Run 메서드)

```cpp
void TcpNetwork::Run()
{
    auto read_set = m_Readfds;   // 읽기 이벤트 감지용
    auto write_set = m_Readfds;  // 쓰기 이벤트 감지용
    
    timeval timeout{ 0, 1000 }; // 1ms 타임아웃
    
    // select 호출
#ifdef _WIN32
    auto selectResult = select(0, &read_set, &write_set, 0, &timeout);
#else
    auto selectResult = select(m_MaxSockFD+1, &read_set, &write_set, 0, &timeout);
#endif
    
    // select 결과 확인
    auto isFDSetChanged = RunCheckSelectResult(selectResult);
    if (isFDSetChanged == false) {
        return;  // 이벤트가 없거나 에러 발생
    }

    // 새로운 클라이언트 연결 처리
    if (FD_ISSET(m_ServerSockfd, &read_set)) {
        NewSession();
    }
    
    // 기존 클라이언트들의 I/O 이벤트 처리
    RunCheckSelectClients(read_set, write_set);
}
```

##### 세션 풀 관리

```cpp
// 세션 풀 생성
int TcpNetwork::CreateSessionPool(const int maxClientCount)
{
    for (int i = 0; i < maxClientCount; ++i)
    {
        ClientSession session;
        session.Clear();
        session.Index = i;
        
        // 각 세션마다 독립적인 버퍼 할당
        session.pRecvBuffer = new char[m_Config.MaxClientRecvBufferSize];
        session.pSendBuffer = new char[m_Config.MaxClientSendBufferSize];
        
        m_ClientSessionPool.push_back(session);
        m_ClientSessionPoolIndex.push_back(session.Index);			
    }
    
    return maxClientCount;
}

// 세션 할당
int TcpNetwork::AllocClientSessionIndex()
{
    if (m_ClientSessionPoolIndex.empty()) {
        return -1;  // 사용 가능한 세션이 없음
    }

    int index = m_ClientSessionPoolIndex.front();
    m_ClientSessionPoolIndex.pop_front();
    return index;
}

// 세션 해제
void TcpNetwork::ReleaseSessionIndex(const int index)
{
    m_ClientSessionPoolIndex.push_back(index);
    m_ClientSessionPool[index].Clear();
}
```

**세션 풀 구조:**   
![세션 풀 구조](./images/073.png)  
  

##### 2.4 패킷 수신 및 처리

```cpp
NET_ERROR_CODE TcpNetwork::RecvSocket(const int sessionIndex)
{
    auto& session = m_ClientSessionPool[sessionIndex];
    auto fd = static_cast<SOCKET>(session.SocketFD);

    if (session.IsConnected() == false) {
        return NET_ERROR_CODE::RECV_PROCESS_NOT_CONNECTED;
    }

    int recvPos = 0;
    
    // 이전에 남은 데이터가 있다면 앞으로 이동
    if (session.RemainingDataSize > 0) {
        memcpy(session.pRecvBuffer, 
               &session.pRecvBuffer[session.PrevReadPosInRecvBuffer], 
               session.RemainingDataSize);
        recvPos += session.RemainingDataSize;
    }

    // 새로운 데이터 수신
    auto recvSize = recv(fd, &session.pRecvBuffer[recvPos], (MAX_PACKET_BODY_SIZE * 2), 0);
    
    if (recvSize == 0) {
        return NET_ERROR_CODE::RECV_REMOTE_CLOSE;  // 상대방이 연결 종료
    }
    
    if (recvSize < 0) {
        auto netError = WSAGetLastError();  // Windows
        if (netError != WSAEWOULDBLOCK) {
            return NET_ERROR_CODE::RECV_API_ERROR;
        }
        return NET_ERROR_CODE::NONE;  // WOULDBLOCK은 정상 (논블로킹)
    }

    session.RemainingDataSize += recvSize;
    return NET_ERROR_CODE::NONE;
}
```

```cpp
NET_ERROR_CODE TcpNetwork::RecvBufferProcess(const int sessionIndex)
{
    auto& session = m_ClientSessionPool[sessionIndex];
    
    auto readPos = 0;
    const auto dataSize = session.RemainingDataSize;
    PacketHeader* pPktHeader;
    
    // 버퍼에서 완성된 패킷들을 찾아서 처리
    while ((dataSize - readPos) >= PACKET_HEADER_SIZE)
    {
        pPktHeader = (PacketHeader*)&session.pRecvBuffer[readPos];
        readPos += PACKET_HEADER_SIZE;
        auto bodySize = (int16_t)(pPktHeader->TotalSize - PACKET_HEADER_SIZE);

        if (bodySize > 0) {
            // 패킷 바디가 완전히 도착했는지 확인
            if (bodySize > (dataSize - readPos)) {
                readPos -= PACKET_HEADER_SIZE;  // 헤더 위치로 되돌리기
                break;  // 불완전한 패킷이므로 다음에 처리
            }

            // 패킷 크기 검증
            if (bodySize > MAX_PACKET_BODY_SIZE) {
                return NET_ERROR_CODE::RECV_CLIENT_MAX_PACKET;
            }
        }

        // 완성된 패킷을 큐에 추가
        AddPacketQueue(sessionIndex, pPktHeader->Id, bodySize, &session.pRecvBuffer[readPos]);
        readPos += bodySize;
    }
    
    // 처리된 데이터 정리
    session.RemainingDataSize -= readPos;
    session.PrevReadPosInRecvBuffer = readPos;
    
    return NET_ERROR_CODE::NONE;
}
```

**패킷 처리 과정:**   
![패킷 수신 및 처리 과정](./images/074.png)  
    
  
##### 패킷 송신

```cpp
NET_ERROR_CODE TcpNetwork::SendData(const int sessionIndex, const short packetId, 
                                   const short bodySize, const char* pMsg)
{
    auto& session = m_ClientSessionPool[sessionIndex];
    
    auto pos = session.SendSize;
    auto totalSize = (int16_t)(bodySize + PACKET_HEADER_SIZE);

    // 송신 버퍼 공간 확인
    if ((pos + totalSize) > m_Config.MaxClientSendBufferSize) {
        return NET_ERROR_CODE::CLIENT_SEND_BUFFER_FULL;
    }
    
    // 패킷 헤더 구성
    PacketHeader pktHeader{ totalSize, packetId, (uint8_t)0 };
    memcpy(&session.pSendBuffer[pos], (char*)&pktHeader, PACKET_HEADER_SIZE);

    // 패킷 바디 복사
    if (bodySize > 0) {
        memcpy(&session.pSendBuffer[pos + PACKET_HEADER_SIZE], pMsg, bodySize);
    }

    session.SendSize += totalSize;
    return NET_ERROR_CODE::NONE;
}
```

```cpp
NetError TcpNetwork::FlushSendBuff(const int sessionIndex)
{
    auto& session = m_ClientSessionPool[sessionIndex];
    auto fd = static_cast<SOCKET>(session.SocketFD);

    if (session.IsConnected() == false) {
        return NetError(NET_ERROR_CODE::CLIENT_FLUSH_SEND_BUFF_REMOTE_CLOSE);
    }

    // 실제 소켓으로 데이터 전송
    auto result = SendSocket(fd, session.pSendBuffer, session.SendSize);
    if (result.Error != NET_ERROR_CODE::NONE) {
        return result;
    }

    auto sendSize = result.Value;
    if (sendSize < session.SendSize) {
        // 부분 전송 - 나머지 데이터를 버퍼 앞쪽으로 이동
        memmove(&session.pSendBuffer[0],
                &session.pSendBuffer[sendSize],
                session.SendSize - sendSize);
        session.SendSize -= sendSize;
    } else {
        // 전체 전송 완료
        session.SendSize = 0;
    }
    
    return result;
}
```

##### TcpNetwork의 설계 특징 및 성능 고려사항

###### 객체 풀 패턴 적용

**장점:**
- 런타임 메모리 할당 없음
- 메모리 단편화 방지
- 예측 가능한 성능

**단점:**
- 초기 메모리 사용량이 큼
- 최대 동접자 수 제한

###### Select 모델의 특성

**플랫폼별 차이점:**
```cpp
#ifdef _WIN32
    // Windows: 첫 번째 인자 무시
    select(0, &read_set, &write_set, 0, &timeout);
#else
    // Linux: 최대 fd + 1 전달
    select(m_MaxSockFD+1, &read_set, &write_set, 0, &timeout);
#endif
```

**성능 특성:**
- O(n) 시간 복잡도 (n = 연결된 소켓 수)
- FD_SETSIZE 제한 (Windows: 64, Linux: 1024)
- 1ms 타임아웃으로 반응성과 CPU 사용률의 균형

###### 에러 처리 및 복구
코드 전반에 걸쳐 다양한 네트워크 에러 상황을 고려한 처리가 구현되어 있으며, 각 상황별로 적절한 에러 코드를 반환하여 상위 계층에서 대응할 수 있도록 설계되었습니다.
이러한 TcpNetwork 클래스는 select 기반 네트워크 프로그래밍의 전형적인 구현 예시이며, 게임 서버 개발의 기초를 학습하는 데 매우 유용한 참고 자료입니다.  

  
#### 세션 관리

```cpp
struct ClientSession
{
    int Index = 0;                    // 세션 인덱스
    SOCKET SocketFD = 0;             // 소켓 파일 디스크립터
    char IP[MAX_IP_LEN] = { 0, };    // 클라이언트 IP
    
    char* pRecvBuffer = nullptr;      // 수신 버퍼
    int RemainingDataSize = 0;        // 남은 데이터 크기
    
    char* pSendBuffer = nullptr;      // 송신 버퍼
    int SendSize = 0;                 // 송신 데이터 크기
};
```

**세션 풀 관리 방식:**
- 미리 고정된 수의 세션 객체를 생성
- 연결 시 풀에서 할당, 연결 해제 시 풀로 반환
- 메모리 할당/해제 오버헤드 최소화
  

### 2.3 로직 계층 (LogicLib)

#### PacketProcess 클래스

```cpp
class PacketProcess
{
private:
    // 패킷 ID별 처리 함수 배열
    typedef ERROR_CODE(PacketProcess::*PacketFunc)(PacketInfo);
    PacketFunc PacketFuncArray[(int)NCommon::PACKET_ID::MAX];
    
    UserManager* m_pRefUserMgr;
    LobbyManager* m_pRefLobbyMgr;
};
```

**패킷 처리 흐름:**  
![패킷 처리 흐름도](./images/062.png)   
  
![PacketProcess 클래스 다이어그램](./images/075.png)    


##### 패킷 함수 포인터 배열

```cpp
class PacketProcess
{
    // 패킷 처리 함수 포인터 타입 정의
    typedef ERROR_CODE(PacketProcess::*PacketFunc)(PacketInfo);
    
    // 패킷 ID별 처리 함수 배열
    PacketFunc PacketFuncArray[(int)NCommon::PACKET_ID::MAX];
    
public:
    void Process(PacketInfo packetInfo)
    {
        auto packetId = packetInfo.PacketId;
        
        // 등록된 처리 함수가 있는지 확인
        if (PacketFuncArray[packetId] == nullptr) {
            return;
        }

        // 해당 패킷 처리 함수 호출
        (this->*PacketFuncArray[packetId])(packetInfo);
    }
};
```
  
##### 초기화 과정 (Init 메서드)

```cpp
void PacketProcess::Init(TcpNet* pNetwork, UserManager* pUserMgr, LobbyManager* pLobbyMgr, 
                        ServerConfig* pConfig, ILog* pLogger)
{
    // 참조 저장
    m_pRefLogger = pLogger;
    m_pRefNetwork = pNetwork;
    m_pRefUserMgr = pUserMgr;
    m_pRefLobbyMgr = pLobbyMgr;

    // 연결 사용자 관리자 초기화
    m_pConnectedUserManager = std::make_unique<ConnectedUserManager>();
    m_pConnectedUserManager->Init(pNetwork->ClientSessionPoolSize(), pNetwork, pConfig, pLogger);

    // 함수 포인터 배열 초기화
    using netLibPacketId = NServerNetLib::PACKET_ID;
    using commonPacketId = NCommon::PACKET_ID;
    
    // 모든 항목을 nullptr로 초기화
    for (int i = 0; i < (int)commonPacketId::MAX; ++i) {
        PacketFuncArray[i] = nullptr;
    }

    // 시스템 패킷 등록
    PacketFuncArray[(int)netLibPacketId::NTF_SYS_CONNECT_SESSION] = &PacketProcess::NtfSysConnctSession;
    PacketFuncArray[(int)netLibPacketId::NTF_SYS_CLOSE_SESSION] = &PacketProcess::NtfSysCloseSession;
    
    // 게임 로직 패킷 등록
    PacketFuncArray[(int)commonPacketId::LOGIN_IN_REQ] = &PacketProcess::Login;
    PacketFuncArray[(int)commonPacketId::LOBBY_LIST_REQ] = &PacketProcess::LobbyList;
    PacketFuncArray[(int)commonPacketId::LOBBY_ENTER_REQ] = &PacketProcess::LobbyEnter;
    PacketFuncArray[(int)commonPacketId::LOBBY_LEAVE_REQ] = &PacketProcess::LobbyLeave;
    PacketFuncArray[(int)commonPacketId::ROOM_ENTER_REQ] = &PacketProcess::RoomEnter;
    PacketFuncArray[(int)commonPacketId::ROOM_LEAVE_REQ] = &PacketProcess::RoomLeave;
    PacketFuncArray[(int)commonPacketId::ROOM_CHAT_REQ] = &PacketProcess::RoomChat;
    PacketFuncArray[(int)commonPacketId::ROOM_MASTER_GAME_START_REQ] = &PacketProcess::RoomMasterGameStart;
    PacketFuncArray[(int)commonPacketId::ROOM_GAME_START_REQ] = &PacketProcess::RoomGameStart;

    // 개발용 패킷 등록
    PacketFuncArray[(int)commonPacketId::DEV_ECHO_REQ] = &PacketProcess::DevEcho;
}
```

**패킷 핸들러 등록 과정:**  
1단계: 배열 초기화    
```
// 모든 패킷 핸들러를 nullptr로 초기화
for (int i = 0; i < (int)commonPacketId::MAX; ++i) {
    PacketFuncArray[i] = nullptr;
}
```  
  
![2단계: 패킷 핸들러 등록](./images/076.png)      
  
3단계: 등록 코드 예시  
```
// 함수 포인터 등록 방식
PacketFuncArray[(int)PACKET_ID::LOGIN_IN_REQ] = &PacketProcess::Login;

// 실제 호출 시
if (PacketFuncArray[packetId] != nullptr) {
    ERROR_CODE result = (this->*PacketFuncArray[packetId])(packetInfo);
}
```  
  
##### 로그인 처리

```cpp
ERROR_CODE PacketProcess::Login(PacketInfo packetInfo)
{
    // 패킷 데이터 파싱
    auto reqPkt = (NCommon::PktLogInReq*)packetInfo.pRefData;
    NCommon::PktLogInRes resPkt;

    // 사용자 추가 시도
    auto addRet = m_pRefUserMgr->AddUser(packetInfo.SessionIndex, reqPkt->szID);

    if (addRet != ERROR_CODE::NONE) {
        // 실패 시 에러 응답
        resPkt.SetError(addRet);
        m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOGIN_IN_RES, 
                               sizeof(NCommon::PktLogInRes), (char*)&resPkt);
        return addRet;
    }

    // 로그인 성공 처리
    m_pConnectedUserManager->SetLogin(packetInfo.SessionIndex);

    // 성공 응답 전송
    resPkt.ErrorCode = (short)ERROR_CODE::NONE;
    m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOGIN_IN_RES, 
                           sizeof(NCommon::PktLogInRes), (char*)&resPkt);

    return ERROR_CODE::NONE;
}
```

##### 방 입장 처리

```cpp
ERROR_CODE PacketProcess::RoomEnter(PacketInfo packetInfo)
{
    auto reqPkt = (NCommon::PktRoomEnterReq*)packetInfo.pRefData;
    NCommon::PktRoomEnterRes resPkt;

    // 1. 사용자 유효성 검증
    auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
    if (errorCode != ERROR_CODE::NONE) {
        resPkt.SetError(errorCode);
        m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, 
                               sizeof(resPkt), (char*)&resPkt);
        return errorCode;
    }

    // 2. 사용자 상태 검증 (로비에 있어야 함)
    if (pUser->IsCurDomainInLobby() == false) {
        resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_DOMAIN);
        m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, 
                               sizeof(resPkt), (char*)&resPkt);
        return ERROR_CODE::ROOM_ENTER_INVALID_DOMAIN;
    }

    // 3. 로비 참조 획득
    auto lobbyIndex = pUser->GetLobbyIndex();
    auto pLobby = m_pRefLobbyMgr->GetLobby(lobbyIndex);
    if (pLobby == nullptr) {
        resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_LOBBY_INDEX);
        m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, 
                               sizeof(resPkt), (char*)&resPkt);
        return ERROR_CODE::ROOM_ENTER_INVALID_LOBBY_INDEX;
    }

    Room* pRoom = nullptr;
    
    // 4. 방 생성 또는 기존 방 입장
    if (reqPkt->IsCreate) {
        // 새 방 생성
        pRoom = pLobby->CreateRoom();
        if (pRoom == nullptr) {
            resPkt.SetError(ERROR_CODE::ROOM_ENTER_EMPTY_ROOM);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, 
                                   sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_ENTER_EMPTY_ROOM;
        }

        auto ret = pRoom->CreateRoom(reqPkt->RoomTitle);
        if (ret != ERROR_CODE::NONE) {
            resPkt.SetError(ret);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, 
                                   sizeof(resPkt), (char*)&resPkt);
            return ret;
        }
    } else {
        // 기존 방 입장
        pRoom = pLobby->GetRoom(reqPkt->RoomIndex);
        if (pRoom == nullptr) {
            resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, 
                                   sizeof(resPkt), (char*)&resPkt);
            return ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX;
        }
    }

    // 5. 방에 사용자 추가
    auto enterRet = pRoom->EnterUser(pUser);
    if (enterRet != ERROR_CODE::NONE) {
        resPkt.SetError(enterRet);
        m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, 
                               sizeof(resPkt), (char*)&resPkt);
        return enterRet;
    }

    // 6. 사용자 상태 업데이트
    pUser->EnterRoom(lobbyIndex, pRoom->GetIndex());

    // 7. 다른 사용자들에게 알림
    pRoom->NotifyEnterUserInfo(pUser->GetIndex(), pUser->GetID().c_str());

    // 8. 성공 응답 전송
    m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_ENTER_RES, 
                           sizeof(resPkt), (char*)&resPkt);
    return ERROR_CODE::NONE;
}
```

**방 입장 처리 흐름:**   
![방 입장 처리 흐름](./images/077.png)  
  
##### 방 채팅 처리

```cpp
ERROR_CODE PacketProcess::RoomChat(PacketInfo packetInfo)
{
    auto reqPkt = (NCommon::PktRoomChatReq*)packetInfo.pRefData;
    NCommon::PktRoomChatRes resPkt;

    // 1. 사용자 유효성 검증
    auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
    if (errorCode != ERROR_CODE::NONE) {
        resPkt.SetError(errorCode);
        m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, 
                               sizeof(resPkt), (char*)&resPkt);
        return errorCode;
    }

    // 2. 사용자가 방에 있는지 확인
    if (pUser->IsCurDomainInRoom() == false) {
        resPkt.SetError(ERROR_CODE::ROOM_CHAT_INVALID_DOMAIN);
        m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, 
                               sizeof(resPkt), (char*)&resPkt);
        return ERROR_CODE::ROOM_CHAT_INVALID_DOMAIN;
    }

    // 3. 로비 및 방 객체 조회
    auto lobbyIndex = pUser->GetLobbyIndex();
    auto pLobby = m_pRefLobbyMgr->GetLobby(lobbyIndex);
    if (pLobby == nullptr) {
        resPkt.SetError(ERROR_CODE::ROOM_CHAT_INVALID_LOBBY_INDEX);
        m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, 
                               sizeof(resPkt), (char*)&resPkt);
        return ERROR_CODE::ROOM_CHAT_INVALID_LOBBY_INDEX;
    }

    auto pRoom = pLobby->GetRoom(pUser->GetRoomIndex());
    if (pRoom == nullptr) {
        resPkt.SetError(ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX);
        m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, 
                               sizeof(resPkt), (char*)&resPkt);
        return ERROR_CODE::ROOM_ENTER_INVALID_ROOM_INDEX;
    }

    // 4. 방의 모든 사용자에게 채팅 메시지 브로드캐스트
    pRoom->NotifyChat(pUser->GetSessioIndex(), pUser->GetID().c_str(), reqPkt->Msg);

    // 5. 요청자에게 성공 응답
    m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::ROOM_CHAT_RES, 
                           sizeof(resPkt), (char*)&resPkt);
    return ERROR_CODE::NONE;
}
```

##### 2.4.1 클라이언트 연결/해제 처리

```cpp
ERROR_CODE PacketProcess::NtfSysConnctSession(PacketInfo packetInfo)
{
    // 새로운 연결에 대한 관리 시작
    m_pConnectedUserManager->SetConnectSession(packetInfo.SessionIndex);
    return ERROR_CODE::NONE;
}

ERROR_CODE PacketProcess::NtfSysCloseSession(PacketInfo packetInfo)
{
    // 연결 종료된 사용자 정리
    auto pUser = std::get<1>(m_pRefUserMgr->GetUser(packetInfo.SessionIndex));

    if (pUser) {
        auto pLobby = m_pRefLobbyMgr->GetLobby(pUser->GetLobbyIndex());
        if (pLobby) {
            auto pRoom = pLobby->GetRoom(pUser->GetRoomIndex());

            // 방에 있었다면 방에서 제거
            if (pRoom) {
                pRoom->LeaveUser(pUser->GetIndex());
                pRoom->NotifyLeaveUserInfo(pUser->GetID().c_str());
                
                m_pRefLogger->Write(LOG_TYPE::L_INFO, "%s | NtfSysCloseSesson. sessionIndex(%d). Room Out", 
                                   __FUNCTION__, packetInfo.SessionIndex);
            }

            // 로비에서 제거
            pLobby->LeaveUser(pUser->GetIndex());
            m_pRefLogger->Write(LOG_TYPE::L_INFO, "%s | NtfSysCloseSesson. sessionIndex(%d). Lobby Out", 
                               __FUNCTION__, packetInfo.SessionIndex);
        }
        
        // 사용자 관리자에서 제거
        m_pRefUserMgr->RemoveUser(packetInfo.SessionIndex);
    }
    
    // 연결 관리자에서 제거
    m_pConnectedUserManager->SetDisConnectSession(packetInfo.SessionIndex);

    m_pRefLogger->Write(LOG_TYPE::L_INFO, "%s | NtfSysCloseSesson. sessionIndex(%d)", 
                       __FUNCTION__, packetInfo.SessionIndex);
    return ERROR_CODE::NONE;
}
```

##### 2.5 상태 체크 (StateCheck 메서드)

```cpp
void PacketProcess::StateCheck()
{
    // 연결 상태 및 로그인 타임아웃 체크
    m_pConnectedUserManager->LoginCheck();
}
```

##### PacketProcess 설계 특징

###### 명령 패턴 (Command Pattern) 적용  
  
```
// 명령 인터페이스 (함수 포인터)
typedef ERROR_CODE(PacketProcess::*PacketFunc)(PacketInfo);

// 명령 등록 (Invoker)
PacketFuncArray[PACKET_ID::LOGIN_REQ] = &PacketProcess::Login;

// 명령 실행
(this->*PacketFuncArray[packetId])(packetInfo);
```  
  

🔥 패턴 적용의 이점:    
✅ 확장성: 새로운 패킷 타입 쉽게 추가  
✅ 유지보수성: 각 패킷 처리 로직이 독립적  
✅ 성능: O(1) 시간에 처리 함수 호출  
✅ 동적 등록: 런타임에 핸들러 변경 가능  
✅ 테스트 용이성: 개별 핸들러 단위 테스트 가능
  
![](./images/078.png)  
  

###### 상태 기반 검증
각 패킷 처리 함수는 사용자의 현재 상태를 확인하여 유효한 요청인지 검증합니다:

- **로그인 상태 검증**: 로비 목록 요청 시 로그인 완료 여부 확인
- **도메인 상태 검증**: 방 입장 시 로비에 있는지 확인
- **권한 검증**: 방장 게임 시작 시 실제 방장인지 확인

###### 에러 처리 일관성
모든 패킷 처리 함수는 동일한 패턴을 따릅니다:

1. **입력 검증**: 패킷 데이터 및 사용자 상태 확인
2. **비즈니스 로직 실행**: 실제 게임 로직 처리
3. **응답 전송**: 성공/실패 결과를 클라이언트에게 전송
4. **에러 코드 반환**: 상위 계층에서 추가 처리할 수 있도록 에러 코드 반환

이러한 구조를 통해 PacketProcess 클래스는 게임 서버의 핵심 로직을 안정적이고 확장 가능하게 처리할 수 있습니다.

  
#### 사용자 관리 시스템

```cpp
class UserManager
{
private:
    std::vector<User> m_UserObjPool;                    // 사용자 객체 풀
    std::deque<int> m_UserObjPoolIndex;                 // 사용 가능한 인덱스 큐
    std::unordered_map<int, User*> m_UserSessionDic;    // 세션 -> 사용자 매핑
    std::unordered_map<const char*, User*> m_UserIDDic; // ID -> 사용자 매핑
};
```

**사용자 상태 관리:**

```cpp
class User
{
    enum class DOMAIN_STATE {
        NONE = 0,    // 미연결
        LOGIN = 1,   // 로그인 완료
        LOBBY = 2,   // 로비 입장
        ROOM = 3,    // 방 입장
    };
};
```

### 2.4 로비 및 방 시스템

#### 로비 시스템 구조
![로비 시스템 구조](./images/063.png)  
  
![LobbyManager & Lobby 클래스 다이어그램](./images/084.png)     
  

##### LobbyManager 초기화

```cpp
class LobbyManager
{
private:
    ILog* m_pRefLogger;
    TcpNet* m_pRefNetwork;
    std::vector<Lobby> m_LobbyList;  // 로비들을 값으로 저장 (복사)

public:
    void Init(const LobbyManagerConfig config, TcpNet* pNetwork, ILog* pLogger)
    {
        m_pRefLogger = pLogger;
        m_pRefNetwork = pNetwork;

        // 설정된 수만큼 로비 생성
        for (int i = 0; i < config.MaxLobbyCount; ++i)
        {
            Lobby lobby;
            lobby.Init((short)i,                           // 로비 인덱스
                      (short)config.MaxLobbyUserCount,     // 로비 최대 사용자 수
                      (short)config.MaxRoomCountByLobby,   // 로비당 방 수
                      (short)config.MaxRoomUserCount);     // 방당 최대 사용자 수
                      
            lobby.SetNetwork(m_pRefNetwork, m_pRefLogger); // 네트워크 참조 설정

            m_LobbyList.push_back(lobby);  // 벡터에 로비 추가
        }
    }

    Lobby* GetLobby(short lobbyId)
    {
        if (lobbyId < 0 || lobbyId >= (short)m_LobbyList.size()) {
            return nullptr;
        }
        return &m_LobbyList[lobbyId];  // 벡터 원소의 주소 반환
    }
};
```

**LobbyManagerConfig 구조체:**

```cpp
struct LobbyManagerConfig
{
    int MaxLobbyCount;        // 서버 전체 로비 수 (예: 2개)
    int MaxLobbyUserCount;    // 각 로비 최대 사용자 수 (예: 50명)
    int MaxRoomCountByLobby;  // 로비당 방 수 (예: 20개)
    int MaxRoomUserCount;     // 방당 최대 사용자 수 (예: 4명)
};
```

##### 로비 목록 정보 전송

```cpp
void LobbyManager::SendLobbyListInfo(const int sessionIndex)
{
    NCommon::PktLobbyListRes resPkt;
    resPkt.ErrorCode = (short)ERROR_CODE::NONE;
    resPkt.LobbyCount = static_cast<short>(m_LobbyList.size());

    int index = 0;
    for (auto& lobby : m_LobbyList)
    {
        resPkt.LobbyList[index].LobbyId = lobby.GetIndex();
        resPkt.LobbyList[index].LobbyUserCount = lobby.GetUserCount();
        resPkt.LobbyList[index].LobbyMaxUserCount = lobby.MaxUserCount();
        ++index;
    }

    // 패킷 전송
    m_pRefNetwork->SendData(sessionIndex, (short)PACKET_ID::LOBBY_LIST_RES, 
                           sizeof(resPkt), (char*)&resPkt);
}
```

**로비 목록 응답 패킷 구조:**  
![로비 목록 응답 패킷 구조](./images/085.png)  

  
##### Lobby 초기화 및 구조

```cpp
class Lobby
{
private:
    ILog* m_pRefLogger;
    TcpNet* m_pRefNetwork;

    short m_LobbyIndex = 0;
    short m_MaxUserCount = 0;

    // 사용자 관리를 위한 3가지 자료구조
    std::vector<LobbyUser> m_UserList;                    // 슬롯 기반 사용자 목록
    std::unordered_map<int, User*> m_UserIndexDic;       // 인덱스 기반 빠른 검색
    std::unordered_map<const char*, User*> m_UserIDDic;  // ID 기반 빠른 검색

    std::vector<Room*> m_RoomList;                        // 방 목록 (포인터 저장)
};

void Lobby::Init(const short lobbyIndex, const short maxLobbyUserCount, 
                const short maxRoomCountByLobby, const short maxRoomUserCount)
{
    m_LobbyIndex = lobbyIndex;
    m_MaxUserCount = maxLobbyUserCount;

    // 사용자 슬롯 초기화
    for (int i = 0; i < maxLobbyUserCount; ++i)
    {
        LobbyUser lobbyUser;
        lobbyUser.Index = (short)i;
        lobbyUser.pUser = nullptr;  // 빈 슬롯

        m_UserList.push_back(lobbyUser);
    }

    // 방 객체들 생성
    for (int i = 0; i < maxRoomCountByLobby; ++i)
    {
        m_RoomList.emplace_back(new Room());
        m_RoomList[i]->Init((short)i, maxRoomUserCount);
    }
}
```

**LobbyUser 구조체:**

```cpp
struct LobbyUser
{
    short Index = 0;    // 슬롯 인덱스
    User* pUser = nullptr;  // 실제 사용자 객체 포인터 (null이면 빈 슬롯)
};
```

##### 사용자 입장 처리

```cpp
ERROR_CODE Lobby::EnterUser(User* pUser)
{
    // 1. 로비 정원 확인
    if (m_UserIndexDic.size() >= m_MaxUserCount) {
        return ERROR_CODE::LOBBY_ENTER_MAX_USER_COUNT;
    }

    // 2. 중복 사용자 확인
    if (FindUser(pUser->GetIndex()) != nullptr) {
        return ERROR_CODE::LOBBY_ENTER_USER_DUPLICATION;
    }

    // 3. 빈 슬롯에 사용자 추가
    auto addRet = AddUser(pUser);
    if (addRet != ERROR_CODE::NONE) {
        return addRet;
    }

    // 4. 사용자 상태 변경
    pUser->EnterLobby(m_LobbyIndex);

    // 5. 빠른 검색을 위한 맵에 추가
    m_UserIndexDic.insert({ pUser->GetIndex(), pUser });
    m_UserIDDic.insert({ pUser->GetID().c_str(), pUser });

    return ERROR_CODE::NONE;
}

ERROR_CODE Lobby::AddUser(User* pUser)
{
    // std::find_if를 사용하여 빈 슬롯 찾기
    auto findIter = std::find_if(std::begin(m_UserList), std::end(m_UserList), 
                                [](auto& lobbyUser) { 
                                    return lobbyUser.pUser == nullptr; 
                                });
    
    if (findIter == std::end(m_UserList)) {
        return ERROR_CODE::LOBBY_ENTER_EMPTY_USER_LIST;
    }

    findIter->pUser = pUser;  // 빈 슬롯에 사용자 할당
    return ERROR_CODE::NONE;
}
```

**사용자 입장 과정 시각화:**   
![사용자 입장 과정 시각화](./images/086.png)  
   
##### 사용자 퇴장 처리

```cpp
ERROR_CODE Lobby::LeaveUser(const int userIndex)
{
    // 1. 슬롯에서 사용자 제거
    RemoveUser(userIndex);

    // 2. 사용자 객체 찾기
    auto pUser = FindUser(userIndex);
    if (pUser == nullptr) {
        return ERROR_CODE::LOBBY_LEAVE_USER_NVALID_UNIQUEINDEX;
    }

    // 3. 사용자 상태 변경
    pUser->LeaveLobby();

    // 4. 검색 맵에서 제거
    m_UserIndexDic.erase(pUser->GetIndex());
    m_UserIDDic.erase(pUser->GetID().c_str());
    
    return ERROR_CODE::NONE;
}

void Lobby::RemoveUser(const int userIndex)
{
    auto findIter = std::find_if(std::begin(m_UserList), std::end(m_UserList), 
                                [userIndex](auto& lobbyUser) { 
                                    return lobbyUser.pUser != nullptr && 
                                           lobbyUser.pUser->GetIndex() == userIndex; 
                                });

    if (findIter == std::end(m_UserList)) {
        return;
    }

    findIter->pUser = nullptr;  // 슬롯을 빈 상태로 만듦
}

User* Lobby::FindUser(const int userIndex)
{
    auto findIter = m_UserIndexDic.find(userIndex);
    if (findIter == m_UserIndexDic.end()) {
        return nullptr;
    }
    return (User*)findIter->second;
}
```

##### 방 관리 시스템

```cpp
Room* Lobby::CreateRoom()
{
    // 사용하지 않는 방 찾기
    for (int i = 0; i < (int)m_RoomList.size(); ++i)
    {
        if (m_RoomList[i]->IsUsed() == false) {
            return m_RoomList[i];  // 빈 방 반환
        }
    }
    return nullptr;  // 사용 가능한 방이 없음
}

Room* Lobby::GetRoom(const short roomIndex)
{
    if (roomIndex < 0 || roomIndex >= m_RoomList.size()) {
        return nullptr;
    }
    return m_RoomList[roomIndex];
}

void Lobby::Release()
{
    // 방 객체들 메모리 해제
    for (int i = 0; i < (int)m_RoomList.size(); ++i)
    {
        delete m_RoomList[i];
    }
    m_RoomList.clear();
}
```

##### 설계 특징 및 성능 고려사항

###### 다층 자료구조 설계
![다층 자료구조 설계](./images/087.png)    

  
###### 확장성 고려사항  
1. **로비 수 확장**: LobbyManager가 동적으로 로비 추가/제거 가능
2. **방 관리 확장**: 각 로비별로 독립적인 방 관리
3. **사용자 검색 최적화**: 인덱스/ID 기반 O(1) 검색 지원
4. **메모리 효율성**: 포인터 기반 참조로 메모리 사용량 최소화

###### 동시성 고려사항
현재 구현은 단일 스레드를 가정하고 있으나, 멀티스레드 환경으로 확장 시 다음 사항들을 고려해야 합니다:

- **읽기/쓰기 락**: 사용자 목록 수정 시 동기화
- **원자적 연산**: 사용자 상태 변경의 원자성 보장
- **락 프리 자료구조**: 성능 향상을 위한 락 프리 큐 활용

이러한 LobbyManager와 Lobby 클래스의 설계는 대규모 멀티플레이어 게임 서버의 핵심 컴포넌트로서, 효율적인 사용자 관리와 방 시스템을 제공합니다.  

  
#### Room 클래스 구조

```cpp
class Room
{
private:
    short m_Index = -1;                    // 방 인덱스
    bool m_IsUsed = false;                 // 사용 중 여부
    std::wstring m_Title;                  // 방 제목
    std::vector<User*> m_UserList;         // 방 사용자 목록
    Game* m_pGame = nullptr;               // 게임 객체
    
public:
    ERROR_CODE CreateRoom(const wchar_t* pRoomTitle);
    ERROR_CODE EnterUser(User* pUser);
    ERROR_CODE LeaveUser(const short userIndex);
    void NotifyChat(const int sessionIndex, const char* pszUserID, const wchar_t* pszMsg);
};
```
  
![Room 클래스 다이어그램](./images/079.png)   
  

##### 클래스 초기화 및 설정

```cpp
class Room
{
private:
    ILog* m_pRefLogger;                    // 로깅 시스템 참조
    TcpNet* m_pRefNetwork;                 // 네트워크 계층 참조

    short m_Index = -1;                    // 방 인덱스 (로비 내에서 고유)
    short m_MaxUserCount;                  // 최대 수용 인원
    
    bool m_IsUsed = false;                 // 방 사용 중 여부
    std::wstring m_Title;                  // 방 제목 (유니코드 지원)
    std::vector<User*> m_UserList;         // 방에 있는 사용자들 (포인터 저장)

    Game* m_pGame = nullptr;               // 게임 객체 (오목, 가위바위보 등)
};

void Room::Init(const short index, const short maxUserCount)
{
    m_Index = index;
    m_MaxUserCount = maxUserCount;
    
    // 게임 객체 생성
    m_pGame = new Game;
}

void Room::SetNetwork(TcpNet* pNetwork, ILog* pLogger)
{
    m_pRefLogger = pLogger;
    m_pRefNetwork = pNetwork;
}
```

##### 방 생성 및 관리

```cpp
ERROR_CODE Room::CreateRoom(const wchar_t* pRoomTitle)
{
    // 이미 사용 중인 방인지 확인
    if (m_IsUsed) {
        return ERROR_CODE::ROOM_ENTER_CREATE_FAIL;
    }

    // 방 활성화 및 제목 설정
    m_IsUsed = true;
    m_Title = pRoomTitle;

    return ERROR_CODE::NONE;
}

void Room::Clear()
{
    m_IsUsed = false;      // 방 비활성화
    m_Title = L"";         // 제목 초기화
    m_UserList.clear();    // 사용자 목록 초기화 (포인터만 제거, 실제 User 객체는 유지)
}
```

**방 생성 과정:**  
로비에서 빈 방 슬롯 찾기:  LobbyManager가 사용 가능한 Room 객체 탐색  
```
Room* Lobby::CreateRoom() {
    for (auto& room : m_RoomList) {
        if (room->IsUsed() == false) {
            return room;  // 빈 방 반환
        }
    }
    return nullptr;  // 사용 가능한 방이 없음
}
```  
   
![방 상태 변경](./images/080.png)    
  
  
##### 사용자 입장/퇴장 관리

```cpp
ERROR_CODE Room::EnterUser(User* pUser)
{
    // 방이 생성되지 않은 상태 확인
    if (m_IsUsed == false) {
        return ERROR_CODE::ROOM_ENTER_NOT_CREATED;
    }

    // 방 정원 확인
    if (m_UserList.size() == m_MaxUserCount) {
        return ERROR_CODE::ROOM_ENTER_MEMBER_FULL;
    }

    // 사용자를 방에 추가
    m_UserList.push_back(pUser);
    return ERROR_CODE::NONE;
}

ERROR_CODE Room::LeaveUser(const short userIndex)
{
    if (m_IsUsed == false) {
        return ERROR_CODE::ROOM_ENTER_NOT_CREATED;
    }

    // 사용자 찾기 및 제거
    auto iter = std::find_if(std::begin(m_UserList), std::end(m_UserList), 
                            [userIndex](auto pUser) { 
                                return pUser->GetIndex() == userIndex; 
                            });
    
    if (iter == std::end(m_UserList)) {
        return ERROR_CODE::ROOM_LEAVE_NOT_MEMBER;
    }
    
    // 사용자 제거
    m_UserList.erase(iter);

    // 방이 비었다면 방 정리
    if (m_UserList.empty()) {
        Clear();
    }

    return ERROR_CODE::NONE;
}

bool Room::IsMaster(const short userIndex)
{
    // 첫 번째 사용자가 방장
    return m_UserList[0]->GetIndex() == userIndex ? true : false;
}
```

**사용자 입장/퇴장 흐름:**  
![사용자 입장/퇴장 흐름](./images/081.png)   
  
  
##### 방 내 통신 시스템

```cpp
// 방의 모든 사용자에게 패킷 전송
void Room::SendToAllUser(const short packetId, const short dataSize, char* pData, const int passUserindex)
{
    for (auto pUser : m_UserList)
    {
        // 특정 사용자 제외 (자신에게는 보내지 않음)
        if (pUser->GetIndex() == passUserindex) {
            continue;
        }

        // 네트워크 계층을 통해 패킷 전송
        m_pRefNetwork->SendData(pUser->GetSessioIndex(), packetId, dataSize, pData);
    }
}

// 새 사용자 입장 알림
void Room::NotifyEnterUserInfo(const int userIndex, const char* pszUserID)
{
    NCommon::PktRoomEnterUserInfoNtf pkt;
    
    // 안전한 문자열 복사 (플랫폼별 처리)
#ifdef _WIN32
    strncpy_s(pkt.UserID, (NCommon::MAX_USER_ID_SIZE+1), pszUserID, NCommon::MAX_USER_ID_SIZE);
#else
    std::strncpy(pkt.UserID, pszUserID, NCommon::MAX_USER_ID_SIZE);
#endif

    // 새로 들어온 사용자를 제외한 모든 사용자에게 알림
    SendToAllUser((short)PACKET_ID::ROOM_ENTER_NEW_USER_NTF, sizeof(pkt), (char*)&pkt, userIndex);
}

// 사용자 퇴장 알림
void Room::NotifyLeaveUserInfo(const char* pszUserID)
{
    if (m_IsUsed == false) {
        return;  // 방이 비활성화된 상태면 알림 불필요
    }

    NCommon::PktRoomLeaveUserInfoNtf pkt;
#ifdef _WIN32
    strncpy_s(pkt.UserID, (NCommon::MAX_USER_ID_SIZE+1), pszUserID, NCommon::MAX_USER_ID_SIZE);
#else
    std::strncpy(pkt.UserID, pszUserID, NCommon::MAX_USER_ID_SIZE);
#endif
    
    // 모든 사용자에게 알림
    SendToAllUser((short)PACKET_ID::ROOM_LEAVE_USER_NTF, sizeof(pkt), (char*)&pkt);
}

// 채팅 메시지 브로드캐스트
void Room::NotifyChat(const int sessionIndex, const char* pszUserID, const wchar_t* pszMsg)
{
    NCommon::PktRoomChatNtf pkt;
    
    // 사용자 ID와 메시지 복사
#ifdef _WIN32
    strncpy_s(pkt.UserID, (NCommon::MAX_USER_ID_SIZE+1), pszUserID, NCommon::MAX_USER_ID_SIZE);
    wcsncpy_s(pkt.Msg, NCommon::MAX_ROOM_CHAT_MSG_SIZE + 1, pszMsg, NCommon::MAX_ROOM_CHAT_MSG_SIZE);
#else
    std::strncpy(pkt.UserID, pszUserID, NCommon::MAX_USER_ID_SIZE);
    std::wcsncpy(pkt.Msg, pszMsg, NCommon::MAX_ROOM_CHAT_MSG_SIZE);
#endif
    
    // 발신자를 제외한 모든 사용자에게 전송
    SendToAllUser((short)PACKET_ID::ROOM_CHAT_NTF, sizeof(pkt), (char*)&pkt, sessionIndex);
}
```

**방 내 통신 시스템 구조:**  
![방 내 통신 시스템 구조](./images/082.png)  
  

##### 2.5 게임 상태 관리

```cpp
Game* Room::GetGameObj()
{
    return m_pGame;
}

void Room::Update()
{
    // 게임이 진행 중일 때 주기적 업데이트
    if (m_pGame->GetState() == GameState::ING)
    {
        // 게임 시간 제한 체크 (예: 선택 제한 시간)
        if (m_pGame->CheckSelectTime())
        {
            // 시간 초과 시 게임 로직 처리
            // 예: 선택하지 않은 플레이어가 지도록 처리
        }
    }
}
```

##### Room 클래스 설계 특징

###### 책임 분리
Room 클래스는 다음과 같은 명확한 책임을 가집니다:  
![Room 클래스 책임 분리](./images/083.png)    
    

###### 메모리 관리 방식
Room 클래스는 효율적인 메모리 관리를 위해 다음과 같은 전략을 사용합니다:

- **User 포인터 저장**: 실제 User 객체를 복사하지 않고 포인터만 저장
- **Game 객체 소유**: Room이 Game 객체의 생명주기를 직접 관리
- **문자열 최적화**: std::wstring 사용으로 유니코드 지원과 효율성 확보

###### 확장성 고려
현재 구조는 다양한 게임 타입으로 확장할 수 있도록 설계되었습니다:

- **Game 인터페이스**: 다양한 게임 구현체로 교체 가능
- **패킷 시스템**: 새로운 방 기능 추가 시 패킷만 추가하면 됨
- **상태 관리**: GameState enum으로 다양한 게임 진행 상태 지원

이러한 Room 클래스는 멀티플레이어 게임의 핵심 컴포넌트로서, 사용자 간의 상호작용과 게임 진행을 안정적으로 관리하는 역할을 수행합니다.


## 3. 클라이언트 구조 (C#)

### 3.1 메인 폼 구조

```csharp
public partial class mainForm : Form
{
    ClientSimpleTcp Network = new ClientSimpleTcp();                    // TCP 클라이언트
    PacketBufferManager PacketBuffer = new PacketBufferManager();      // 패킷 버퍼 관리
    Queue<PacketData> RecvPacketQueue = new Queue<PacketData>();       // 수신 패킷 큐
    Queue<byte[]> SendPacketQueue = new Queue<byte[]>();               // 송신 패킷 큐
    
    Dictionary<PACKET_ID, Action<byte[]>> PacketFuncDic;               // 패킷 처리 함수 딕셔너리
}
```

### 3.2 네트워크 처리 구조

**멀티 스레드 구조:**  
![클라이언트 스레드 구조](./images/064.png)    
  
  
### 3.3 패킷 버퍼 관리

```csharp
class PacketBufferManager
{
    int BufferSize = 0;           // 버퍼 크기
    int ReadPos = 0;              // 읽기 위치
    int WritePos = 0;             // 쓰기 위치
    byte[] PacketData;            // 패킷 데이터 버퍼
    
    public bool Write(byte[] data, int pos, int size)
    {
        // 수신된 데이터를 버퍼에 쓰기
        Buffer.BlockCopy(data, pos, PacketData, WritePos, size);
        WritePos += size;
        
        if (NextFree() == false) {
            BufferRelocate();  // 버퍼 정리
        }
        return true;
    }
    
    public ArraySegment<byte> Read()
    {
        // 완성된 패킷이 있는지 확인하고 반환
        var packetDataSize = BitConverter.ToInt16(PacketData, ReadPos);
        if (enableReadSize < packetDataSize) {
            return new ArraySegment<byte>();
        }
        
        var completePacketData = new ArraySegment<byte>(PacketData, ReadPos, packetDataSize);
        ReadPos += packetDataSize;
        return completePacketData;
    }
}
```
  

## 4. 통신 프로토콜 분석

### 4.1 패킷 구조

```cpp
#pragma pack(push, 1)
struct PacketHeader
{
    short TotalSize;    // 전체 패킷 크기 (헤더 + 바디)
    short Id;           // 패킷 ID
    unsigned char Reserve; // 예약 필드
};
#pragma pack(pop)

const int PACKET_HEADER_SIZE = sizeof(PacketHeader); // 5바이트
```

### 4.2 주요 패킷 종류
![패킷 종류 및 흐름](./images/065.png)    
  
  
### 4.3 통신 시나리오 예제

**로그인 → 로비 입장 → 방 생성 → 채팅 흐름:**  
![통신 시나리오](./images/066.png)    
  
  
## 5. 핵심 기술 개념

### 5.1 Select 모델의 장단점

**장점:**
- 플랫폼 호환성 (Windows/Linux 공통)
- 구현 복잡도가 상대적으로 낮음
- 작은 규모의 서버에 적합

**단점:**
- FD_SETSIZE 제한 (Windows: 64, Linux: 1024)
- O(n) 시간 복잡도
- 대규모 동접자 처리에 한계
  

### 5.2 객체 풀 패턴

```cpp
// 사용자 객체 풀 구현 예제
class UserManager
{
private:
    std::vector<User> m_UserObjPool;        // 미리 할당된 객체들
    std::deque<int> m_UserObjPoolIndex;     // 사용 가능한 인덱스
    
public:
    User* AllocUserObjPoolIndex()
    {
        if (m_UserObjPoolIndex.empty()) return nullptr;
        
        int index = m_UserObjPoolIndex.front();
        m_UserObjPoolIndex.pop_front();
        return &m_UserObjPool[index];
    }
    
    void ReleaseUserObjPoolIndex(const int index)
    {
        m_UserObjPoolIndex.push_back(index);
        m_UserObjPool[index].Clear();
    }
};
```

**객체 풀의 이점:**
- 런타임 메모리 할당/해제 최소화
- 가비지 컬렉션 압박 감소
- 예측 가능한 메모리 사용량

### 5.3 상태 기반 사용자 관리

```cpp
enum class DOMAIN_STATE {
    NONE = 0,    // 연결되지 않음
    LOGIN = 1,   // 로그인 완료
    LOBBY = 2,   // 로비에 있음  
    ROOM = 3,    // 방에 있음
};
```

상태 전이 규칙을 통해 잘못된 요청을 사전에 차단하여 서버 안정성을 높입니다.
  

## 6. 확장 가능성 및 학습 포인트

### 6.1 현재 구조의 한계

1. **동기식 I/O**: select는 동기식이므로 대규모 확장성에 한계
2. **싱글 스레드**: 패킷 처리가 단일 스레드에서 진행
3. **메모리 기반**: 데이터베이스 연동이 없어 영속성 부족

### 6.2 개선 방향

1. **비동기 I/O 모델**: IOCP(Windows), epoll(Linux) 적용
2. **워커 스레드 풀**: 패킷 처리를 멀티스레드로 분산
3. **데이터베이스 연동**: 사용자 정보 영속화
4. **로드 밸런싱**: 다중 서버 구조로 확장

### 6.3 학습 가치

이 프로젝트는 다음과 같은 게임 서버 핵심 개념들을 학습할 수 있습니다:

- **네트워크 프로그래밍**: TCP 소켓, select 모델
- **메모리 관리**: 객체 풀, 버퍼 관리
- **상태 관리**: 사용자 상태, 방 상태
- **패킷 처리**: 직렬화/역직렬화, 프로토콜 설계
- **멀티스레드**: 생산자-소비자 패턴
- **아키텍처 설계**: 계층 분리, 모듈화



<br>       
  