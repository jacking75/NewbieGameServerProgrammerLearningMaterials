# 게임 서버 개발자가 알아야할 TCP/IP Windows 소켓 프로그래밍

저자: 최흥배, Claude AI  

- C++23
- Windows 11
- Visual Studio 2022 이상
  

-----  
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
  
