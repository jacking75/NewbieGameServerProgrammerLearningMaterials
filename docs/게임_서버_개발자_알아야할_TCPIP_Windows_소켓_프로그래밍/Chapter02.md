# 게임 서버 개발자가 알아야할 TCP/IP Windows 소켓 프로그래밍

저자: 최흥배, Claude AI  

- C++23
- Windows 11
- Visual Studio 2022 이상
  

-----  
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

