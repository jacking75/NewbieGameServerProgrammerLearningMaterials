# 게임 서버 개발자가 알아야할 TCP/IP Windows 소켓 프로그래밍

저자: 최흥배, Claude AI  

- C++23
- Windows 11
- Visual Studio 2022 이상
  

-----  
# Chapter.11 IOCP 방식의 채팅 서버 
`codes/IocpNetLib` 디렉토리에 코드가 있다.  

## 1. 전체 아키텍처 개요

### 1.1 라이브러리 구조
```
IocpNetLib/
├── IocpNetLib/                 # 핵심 네트워크 라이브러리
├── IocpNetLib_Echo/            # 에코 서버 예제
└── IocpNetLib_IocpChatServer/  # 채팅 서버 예제
```

### 1.2 핵심 설계 원리
이 라이브러리는 **비동기 I/O**와 **멀티스레딩**을 활용한 고성능 서버 아키텍처를 구현한다.

![](./images/221.png)  

각 단계의 역할은 다음과 같다.  
- 클라이언트들: 여러 게임 클라이언트가 TCP 연결을 통해 서버에 접속한다.
- Listen Socket: 새로운 클라이언트 연결을 받아들이는 소켓으로, Accept 처리를 담당한다.
- IOCP Work Threads: Windows의 I/O Completion Port를 사용하여 비동기 I/O를 처리하는 워커 스레드들이다. 패킷 수신과 송신을 효율적으로 처리한다.
- Logic Thread: 게임 로직을 처리하는 단일 스레드로, 패킷을 순차적으로 처리하여 스레드 안전성을 보장한다.
    - 패킷 처리: 실제 게임 상태를 업데이트하고 클라이언트에게 응답을 생성하는 단계이다.
  

## 2. 핵심 컴포넌트 분석

### 2.1 IOCPServerNet 클래스 (메인 서버)

**역할**: 서버의 핵심 관리자 역할
- Listen 소켓 생성 및 관리
- IOCP 핸들 생성 및 워커 스레드 관리
- 클라이언트 연결 풀 관리
- 네트워크 메시지 처리

**주요 메서드**:
```cpp
NetResult Start(NetConfig netConfig)        // 서버 시작
void End()                                   // 서버 종료
bool GetNetworkMessage(...)                  // 네트워크 메시지 수신
void SendPacket(...)                         // 패킷 전송
```

**동작 흐름**:
1. `Start()` 호출 시 Listen 소켓 생성
2. IOCP 핸들 생성 (Work용, Logic용 분리)
3. Connection 객체들을 미리 생성 (객체 풀링)
4. 워커 스레드들 시작
5. `AcceptEx()` 호출로 비동기 연결 대기
 
### 2.2 Connection 클래스 (연결 관리)

**역할**: 개별 클라이언트 연결을 관리하는 핵심 클래스

**주요 구성요소**:
```cpp
class Connection {
private:
    SOCKET m_ClientSocket;           // 클라이언트 소켓
    OVERLAPPED_EX* m_pRecvOverlappedEx;  // 비동기 수신용
    OVERLAPPED_EX* m_pSendOverlappedEx;  // 비동기 송신용
    RecvRingBuffer m_RecvRingBuffer;     // 수신 링버퍼
    SendRingBuffer m_SendRingBuffer;     // 송신 링버퍼
    std::mutex m_MUTEX;                  // 동기화
};
```

**핵심 기능**:
- **비동기 Accept**: `AcceptEx()` 사용
- **비동기 수신**: `WSARecv()` 사용  
- **비동기 송신**: `WSASend()` 사용
- **링버퍼 활용**: 효율적인 데이터 관리

![](./images/222.png)   
  

### 2.3 링버퍼 시스템
**RecvRingBuffer (수신용)**:
```cpp
class RecvRingBuffer {
private:
    char* m_pRingBuffer;    // 순환 버퍼
    int m_ReadPos;          // 읽기 위치
    int m_WritePos;         // 쓰기 위치
};
```

**SendRingBuffer (송신용)**:
```cpp
class SendRingBuffer {
private:
    char* m_pRingBuffer;
    int m_ReadPos;
    int m_WritePos;
    CustomSpinLockCriticalSection m_CS;  // 스핀락 동기화
};
```

**링버퍼의 장점**:
- 메모리 재할당 최소화
- 연속된 메모리 영역 활용
- 효율적인 패킷 조립

### 2.4 메시지 풀링 시스템

**MessagePool 클래스**:
```cpp
class MessagePool {
private:
    concurrency::concurrent_queue<Message*> m_MessagePool;
    int m_MaxMessagePoolCount;
    int m_ExtraMessagePoolCount;
};
```

**핵심 원리**:
- 메시지 객체를 미리 생성하여 풀에 저장
- 동적 할당/해제 오버헤드 제거
- 스레드 안전한 concurrent_queue 사용
  

## 3. IOCP 동작 원리

### 3.1 IOCP 기본 개념
IOCP는 Windows에서 제공하는 고성능 비동기 I/O 메커니즘이다.

**핵심 특징**:
- **비동기 처리**: I/O 작업을 백그라운드에서 처리
- **완료 통지**: 작업 완료 시 완료 포트로 통지
- **스레드 풀**: 효율적인 스레드 관리

### 3.2 이 라이브러리의 IOCP 활용

**두 개의 IOCP 핸들 사용**:
```cpp
HANDLE m_hWorkIOCP;     // 네트워크 I/O 작업용
HANDLE m_hLogicIOCP;    // 로직 처리용
```

**Work IOCP 흐름**:
![](./images/223.png)     
- I/O 작업 완료: AcceptEx, WSARecv, WSASend 등의 비동기 I/O 작업이 완료되면 완료 이벤트가 IOCP로 전달된다.
- IOCP 완료 포트: 모든 완료 이벤트가 하나의 완료 포트로 집중되어 관리된다. GetQueuedCompletionStatus() 함수가 - 대기 중인 완료 이벤트를 반환한다.
- Work Threads: 여러 워커 스레드가 동시에 GetQueuedCompletionStatus()를 호출하여 완료 이벤트를 처리한다. 이를 통해 높은 동시성을 달성할 수 있다.
- 처리 과정: 각 워커 스레드는 완료 이벤트의 종류를 판별하고 적절한 처리를 수행한 후, 다음 I/O 작업을 요청한다.
  
**Logic IOCP 흐름**: 
![](./images/224.png)    
- 계층 분리: 네트워크 I/O를 담당하는 Work Thread 영역과 게임 로직을 처리하는 Logic Thread 영역이 명확히 분리되어 있다.
- 패킷 조립: Work Thread에서 TCP 스트림으로 수신된 불완전한 데이터를 완전한 패킷으로 조립한다. 헤더 검증과 크기 확인을 통해 패킷의 완성도를 보장한다.
- PostQueuedCompletionStatus(): 완전한 패킷이 조립되면 사용자 정의 완료 이벤트를 Logic IOCP로 전송한다. 이는 네트워크 I/O 완료 이벤트와는 다른 게임 로직 전용 이벤트이다.
- 단일 스레드 처리: Logic Thread는 단일 스레드로 모든 게임 로직을 순차적으로 처리하여 동기화 문제를 원천적으로 방지한다.
- 스레드 안전성: 이 구조를 통해 복잡한 락(lock) 없이도 게임 상태의 일관성을 보장할 수 있다.  
  
이 패턴은 고성능과 안정성을 동시에 요구하는 온라인 게임 서버에서 널리 사용되는 아키텍처이다.
  

### 3.3 워커 스레드 동작

```cpp
void WorkThread() {
    while (m_IsRunWorkThread) {
        DWORD ioSize = 0;
        OVERLAPPED_EX* pOverlappedEx = nullptr;
        Connection* pConnection = nullptr;

        auto result = GetQueuedCompletionStatus(
            m_hWorkIOCP, &ioSize, 
            reinterpret_cast<PULONG_PTR>(&pConnection),
            reinterpret_cast<LPOVERLAPPED*>(&pOverlappedEx),
            INFINITE);

        // 작업 타입에 따른 처리
        switch (pOverlappedEx->OverlappedExOperationType) {
            case OperationType::Accept: DoAccept(pOverlappedEx); break;
            case OperationType::Recv:   DoRecv(pOverlappedEx, ioSize); break;
            case OperationType::Send:   DoSend(pOverlappedEx, ioSize); break;
        }
    }
}
```

## 4. 패킷 처리 흐름

### 4.1 패킷 구조
```cpp
struct PACKET_HEADER {
    UINT16 PacketLength;    // 패킷 전체 길이
    UINT16 PacketId;        // 패킷 식별자
    UINT8 Type;             // 패킷 속성
};
```

### 4.2 수신 패킷 처리 과정

**1단계: 비동기 수신**
```cpp
WSARecv(socket, &wsabuf, 1, &recvBytes, &flags, &overlapped, NULL);
```

**2단계: 링버퍼에 데이터 저장**
```cpp
auto [remainByte, pNext] = pConnection->GetReceiveData(ioSize);
```

**3단계: 패킷 조립**
```cpp
while (remainByte >= PACKET_HEADER_LENGTH) {
    short packetSize = *(short*)pBuffer;
    if (remainByte >= packetSize) {
        // 완전한 패킷 발견
        auto pMsg = m_pMsgPool->AllocMsg();
        pMsg->SetMessage(MessageType::OnRecv, pBuffer);
        PostNetMessage(pConnection, pMsg, packetSize);
    }
}
```

**4단계: 로직 스레드로 전달**
```cpp
PostQueuedCompletionStatus(m_hLogicIOCP, packetSize, 
                          reinterpret_cast<ULONG_PTR>(pConnection),
                          reinterpret_cast<LPOVERLAPPED>(pMsg));
```

### 4.3 송신 패킷 처리 과정

**1단계: 송신 버퍼에 데이터 추가**
```cpp
m_SendRingBuffer.AddSendData(sendSize, pSendData);
```

**2단계: 비동기 송신**
```cpp
WSASend(socket, &wsabuf, 1, &sendBytes, flags, &overlapped, NULL);
```

**3단계: 송신 완료 처리**
```cpp
pConnection->SendBufferSendCompleted(ioSize);
pConnection->SetEnableSend();
```
  

## 5. 동기화 및 스레드 안전성

### 5.1 스핀락 활용
```cpp
class CustomSpinLockCriticalSection {
    CRITICAL_SECTION m_CS;
public:
    CustomSpinLockCriticalSection() {
        InitializeCriticalSectionAndSpinCount(&m_CS, SPINLOCK_COUNT);
    }
};
```

**스핀락의 장점**:
- 짧은 임계영역에서 뮤텍스보다 빠름
- 컨텍스트 스위칭 오버헤드 감소

### 5.2 원자적 연산 활용
```cpp
void IncrementRecvIORefCount() { 
    InterlockedIncrement(reinterpret_cast<LPLONG>(&m_RecvIORefCount)); 
}

void SetNetStateConnection() {
    InterlockedExchange(reinterpret_cast<LPLONG>(&m_IsConnect), TRUE);
}
```
  
### 5.3 IO 참조 카운팅
각 Connection은 진행 중인 비동기 I/O 작업 수를 추적한다:
```cpp
DWORD m_SendIORefCount = 0; 
DWORD m_RecvIORefCount = 0; 
std::atomic<short> m_AcceptIORefCount = 0;
```

이를 통해 안전한 연결 종료를 보장한다.
  

## 6. 실제 적용 예제: 채팅 서버

### 6.1 채팅 서버 구조
```
ChatServerLib::Main → PacketManager → UserManager + RoomManager
```

### 6.2 주요 패킷 처리
```cpp
// 로그인 처리
void PacketManager::ProcessLogin(const INT32 connIndex, char* pBuf, INT16 copySize) {
    auto pLoginReqPacket = reinterpret_cast<LOGIN_REQUEST_PACKET*>(pBuf);
    
    if (m_pUserManager->FindUserIndexByID(pLoginReqPacket->UserID) == -1) {
        m_pUserManager->AddUser(pLoginReqPacket->UserID, connIndex);
        // 성공 응답 전송
    } else {
        // 이미 접속중인 사용자 오류 응답
    }
}

// 채팅 메시지 처리
void PacketManager::ProcessRoomChatMessage(INT32 connIndex, char* pBuf, INT16 copySize) {
    auto reqUser = m_pUserManager->GetUserByConnIdx(connIndex);
    auto roomNum = reqUser->GetCurrentRoom();
    auto pRoom = m_pRoomManager->GetRoomByNumber(roomNum);
    
    // 방의 모든 사용자에게 메시지 브로드캐스트
    pRoom->NotifyChat(connIndex, reqUser->GetUserId().c_str(), pMessage);
}
```
  

## 7. 성능 최적화 기법

### 7.1 객체 풀링
- Connection 객체 미리 생성
- Message 객체 풀링
- 동적 할당 최소화

### 7.2 링버퍼 활용
- 메모리 복사 최소화
- 연속된 메모리 영역 활용
- 효율적인 패킷 파싱

### 7.3 비동기 I/O
- 블로킹 없는 네트워크 처리
- 높은 동시 연결 수 지원
- CPU 효율성 극대화

### 7.4 스레드 설계
- I/O 처리와 로직 처리 분리
- 적절한 스레드 풀 크기
- 락 경합 최소화
  

## 8. 에러 처리 및 안정성

### 8.1 연결 상태 관리
```cpp
enum class NetResult : INT16 {
    Success = 0,
    PostRecv_Null_Obj = 61,
    PostRecv_Null_WSABUF = 62,
    PostRecv_Null_Socket_Error = 63,
    // ... 기타 에러 코드들
};
```

### 8.2 크래시 덤프
```cpp
class MiniDump {
    static void Begin();
    static bool CreateDirectories();
    static void End();
};
```

### 8.3 성능 모니터링
```cpp
class Performance {
    void CheckPerformanceThread();
    int IncrementPacketProcessCount();
};
```
  

## 9. 학습 포인트 및 확장 방향

### 9.1 핵심 학습 포인트
1. **IOCP의 동작 원리와 활용법**
2. **비동기 프로그래밍 패턴**
3. **멀티스레드 환경에서의 동기화**
4. **네트워크 프로그래밍 최적화 기법**
5. **서버 아키텍처 설계 원리**

### 9.2 확장 가능한 방향
1. **SSL/TLS 지원 추가**
2. **UDP 프로토콜 지원**
3. **로드 밸런싱 기능**
4. **DB 연동 레이어**
5. **웹소켓 프로토콜 지원**
  

## 10. 실습 및 학습 권장사항

### 10.1 단계별 학습
1. **1단계**: Echo 서버로 기본 동작 이해
2. **2단계**: 채팅 서버로 실제 로직 구현 학습
3. **3단계**: 코드 분석을 통한 최적화 기법 이해
4. **4단계**: 직접 기능 확장 및 커스터마이징

  
## IOCP 서버 아키텍처
  
![](./images/088.png)  
![](./images/089.png)  
![](./images/090.png)  
![](./images/091.png)    


## 주요 코드  

### 1. IOCP 서버 초기화 및 시작 과정
![](./images/225.png)     
  
```
// IOCPServerNet::Start() - 서버 시작의 핵심 흐름
NetResult IOCPServerNet::Start(NetConfig netConfig) {
    m_NetConfig = netConfig;
    
    // 1. Listen 소켓 생성
    auto result = CreateListenSocket();
    if (result != NetResult::Success) return result;
    
    // 2. IOCP 핸들 생성 (Work용, Logic용 분리)
    result = CreateHandleIOCP();
    if (result != NetResult::Success) return result;
    
    // 3. 메시지 풀 생성
    if (!CreateMessageManager()) return NetResult::Fail_Create_Message_Manager;
    
    // 4. Listen 소켓을 Work IOCP에 바인딩
    if (!LinkListenSocketIOCP()) return NetResult::Fail_Link_IOCP;
    
    // 5. Connection 객체들 미리 생성 (객체 풀링)
    if (!CreateConnections()) return NetResult::Fail_Create_Connection;
    
    // 6. 워커 스레드들 생성 및 시작
    if (!CreateWorkThread()) return NetResult::Fail_Create_WorkThread;
    
    return NetResult::Success;
}

// Listen 소켓 생성 과정
NetResult IOCPServerNet::CreateListenSocket() {
    // Winsock 초기화
    WSADATA wsaData;
    auto result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) return NetResult::fail_create_listensocket_startup;
    
    // Overlapped I/O 지원 소켓 생성
    m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, 
                              NULL, 0, WSA_FLAG_OVERLAPPED);
    if (m_ListenSocket == INVALID_SOCKET) {
        return NetResult::fail_create_listensocket_socket;
    }
    
    // 주소 바인딩
    SOCKADDR_IN addr;
    ZeroMemory(&addr, sizeof(SOCKADDR_IN));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_NetConfig.PortNumber);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (::bind(m_ListenSocket, reinterpret_cast<SOCKADDR*>(&addr), 
               sizeof(addr)) == SOCKET_ERROR) {
        return NetResult::fail_create_listensocket_bind;
    }
    
    // Listen 모드로 전환
    if (::listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        return NetResult::fail_create_listensocket_listen;
    }
    
    return NetResult::Success;
}
```  
  
### 2. Connection 클래스의 핵심 동작
  
```
// Connection 초기화 및 AcceptEx 등록
void Connection::Init(const SOCKET listenSocket, const int index, 
                     const ConnectionNetConfig config) {
    m_ListenSocket = listenSocket;
    m_Index = index;
    m_RecvBufSize = config.MaxRecvOverlappedBufferSize;
    m_SendBufSize = config.MaxSendOverlappedBufferSize;
    
    // Overlapped 구조체 생성
    m_pRecvOverlappedEx = new OVERLAPPED_EX(index);
    m_pSendOverlappedEx = new OVERLAPPED_EX(index);
    
    // 링버퍼 생성
    m_RecvRingBuffer.Create(config.MaxRecvBufferSize);
    m_SendRingBuffer.Create(config.MaxSendBufferSize);
    
    // 비동기 Accept 시작
    BindAcceptExSocket();
}

// AcceptEx를 사용한 비동기 연결 수락
NetResult Connection::BindAcceptExSocket() {
    // 클라이언트 소켓 생성
    m_ClientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, 
                              NULL, 0, WSA_FLAG_OVERLAPPED);
    if (m_ClientSocket == INVALID_SOCKET) {
        return NetResult::BindAcceptExSocket_fail_WSASocket;
    }
    
    // AcceptEx용 Overlapped 설정
    ZeroMemory(&m_pRecvOverlappedEx->Overlapped, sizeof(OVERLAPPED));
    m_pRecvOverlappedEx->OverlappedExWsaBuf.buf = m_AddrBuf;
    m_pRecvOverlappedEx->OverlappedExWsaBuf.len = m_RecvBufSize;
    m_pRecvOverlappedEx->OverlappedExOperationType = OperationType::Accept;
    
    IncrementAcceptIORefCount();
    
    // 비동기 Accept 요청
    DWORD acceptByte = 0;
    auto result = AcceptEx(
        m_ListenSocket,           // Listen 소켓
        m_ClientSocket,           // 클라이언트 소켓
        m_pRecvOverlappedEx->OverlappedExWsaBuf.buf,  // 주소 정보 버퍼
        0,                        // 데이터 길이 (주소 정보만 받음)
        sizeof(SOCKADDR_IN) + 16, // 로컬 주소 크기
        sizeof(SOCKADDR_IN) + 16, // 원격 주소 크기
        &acceptByte,              // 받은 바이트 수
        reinterpret_cast<LPOVERLAPPED>(m_pRecvOverlappedEx)  // Overlapped
    );
    
    if (!result && WSAGetLastError() != WSA_IO_PENDING) {
        DecrementAcceptIORefCount();
        return NetResult::BindAcceptExSocket_fail_AcceptEx;
    }
    
    return NetResult::Success;
}

// 비동기 데이터 수신
NetResult Connection::PostRecv() {
    if (m_IsConnect == FALSE || m_pRecvOverlappedEx == nullptr) {
        return NetResult::PostRecv_Null_Obj;
    }
    
    // 수신용 Overlapped 설정
    m_pRecvOverlappedEx->OverlappedExOperationType = OperationType::Recv;
    m_pRecvOverlappedEx->OverlappedExWsaBuf.len = m_RecvBufSize;
    
    // 링버퍼에서 쓰기 가능한 영역 가져오기
    m_pRecvOverlappedEx->OverlappedExWsaBuf.buf = 
        m_RecvRingBuffer.GetWriteBuffer(m_RecvBufSize);
    
    if (m_pRecvOverlappedEx->OverlappedExWsaBuf.buf == nullptr) {
        return NetResult::PostRecv_Null_WSABUF;
    }
    
    ZeroMemory(&m_pRecvOverlappedEx->Overlapped, sizeof(OVERLAPPED));
    IncrementRecvIORefCount();
    
    // 비동기 수신 요청
    DWORD flag = 0;
    DWORD recvByte = 0;
    auto result = WSARecv(
        m_ClientSocket,
        &m_pRecvOverlappedEx->OverlappedExWsaBuf,
        1,                        // 버퍼 개수
        &recvByte,               // 받은 바이트 수
        &flag,                   // 플래그
        &m_pRecvOverlappedEx->Overlapped,  // Overlapped
        NULL                     // 완료 루틴
    );
    
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        DecrementRecvIORefCount();
        return NetResult::PostRecv_Null_Socket_Error;
    }
    
    return NetResult::Success;
}
```  
   
### 3. 워커 스레드의 핵심 동작
![](./images/226.png)     
- 무한 루프 구조: 워커 스레드는 서버가 종료될 때까지 계속해서 IOCP 완료 이벤트를 처리한다.
- 체계적인 에러 처리: 각 단계에서 발생할 수 있는 에러를 체크하고, 문제가 있으면 적절한 처리 후 다음 루프로 continue 한다.
- 작업 타입별 분기: OperationType에 따라 Accept, Recv, Send 처리 함수로 분기하여 각각 다른 로직을 수행한다.
- DoAccept 상세 과정:
    - Connection 객체 획득과 참조 카운트 관리
    - 클라이언트 주소 정보 설정
    - IOCP 바인딩으로 비동기 I/O 준비
    - 연결 상태 변경과 수신 시작
    - Logic Thread에 연결 완료 통지
- 성능 최적화: INFINITE 대기를 통한 CPU 효율성, 즉시 PostRecv 호출로 빠른 수신 준비, switch문을 통한 효율적인 분기 처리 등이 포함되어 있다.
  
    
```
// IOCP 워커 스레드 메인 루프
void IOCPServerNet::WorkThread() {
    while (m_IsRunWorkThread) {
        DWORD ioSize = 0;
        OVERLAPPED_EX* pOverlappedEx = nullptr;
        Connection* pConnection = nullptr;
        
        // IOCP에서 완료된 I/O 작업 가져오기
        auto result = GetQueuedCompletionStatus(
            m_hWorkIOCP,              // IOCP 핸들
            &ioSize,                  // 전송된 바이트 수
            reinterpret_cast<PULONG_PTR>(&pConnection),  // 완료 키 (Connection 포인터)
            reinterpret_cast<LPOVERLAPPED*>(&pOverlappedEx),  // Overlapped
            INFINITE                  // 대기 시간
        );
        
        // 에러 처리
        if (pOverlappedEx == nullptr) {
            if (WSAGetLastError() != 0 && WSAGetLastError() != WSA_IO_PENDING) {
                char logmsg[128] = { 0, };
                sprintf_s(logmsg, "WorkThread - GetQueuedCompletionStatus(). error:%d", 
                         WSAGetLastError());
                LogFuncPtr((int)LogLevel::Error, logmsg);
            }
            continue;
        }
        
        // I/O 실패 또는 연결 종료 처리
        if (!result || (0 == ioSize && OperationType::Accept != 
                       pOverlappedEx->OverlappedExOperationType)) {
            HandleExceptionWorkThread(pConnection, pOverlappedEx);
            continue;
        }
        
        // 작업 타입에 따른 처리
        switch (pOverlappedEx->OverlappedExOperationType) {
            case OperationType::Accept:
                DoAccept(pOverlappedEx);
                break;
            case OperationType::Recv:
                DoRecv(pOverlappedEx, ioSize);
                break;
            case OperationType::Send:
                DoSend(pOverlappedEx, ioSize);
                break;
        }
    }
}

// Accept 완료 처리
void IOCPServerNet::DoAccept(const OVERLAPPED_EX* pOverlappedEx) {
    auto pConnection = GetConnection(pOverlappedEx->ConnectionIndex);
    if (pConnection == nullptr) return;
    
    pConnection->DecrementAcceptIORefCount();
    
    // 클라이언트 주소 정보 설정
    if (pConnection->SetNetAddressInfo() == false) {
        HandleExceptionCloseConnection(pConnection);
        return;
    }
    
    // 클라이언트 소켓을 Work IOCP에 바인딩
    if (!pConnection->BindIOCP(m_hWorkIOCP)) {
        HandleExceptionCloseConnection(pConnection);
        return;
    }
    
    // 연결 상태로 변경
    pConnection->SetNetStateConnection();
    
    // 수신 시작
    auto result = pConnection->PostRecv();
    if (result != NetResult::Success) {
        HandleExceptionCloseConnection(pConnection);
        return;
    }
    
    // Logic Thread에 연결 통지
    PostNetMessage(pConnection, pConnection->GetConnectionMsg());
}
```  
  
### 4. 패킷 조립 및 처리
![](./images/227.png)    
이 다이어그램의 핵심 내용은 다음과 같다.

**메인 흐름 (왼쪽)**:
- DoRecv에서 수신 완료 처리
- 링버퍼에서 데이터 획득
- PacketForwardingLoop로 패킷 조립
- 읽기 포인터 이동으로 처리 완료 표시
- 다음 수신 요청으로 지속적 처리

**PacketForwardingLoop 상세 과정 (가운데)**:
- 무한 루프로 버퍼의 모든 완전한 패킷 처리
- 헤더 크기 체크 (5바이트)
- 패킷 크기 읽기 및 유효성 검증
- 완전한 패킷 확인
- 메시지 풀에서 객체 할당
- Logic IOCP로 메시지 전송

**패킷 구조 (오른쪽 상단)**:
- Length(2) + PacketID(2) + Type(1) + Body(가변)
- 헤더 정보로 패킷 경계 판단

**링버퍼 관리 (오른쪽 중간)**:
- 원형 버퍼 구조로 메모리 효율성
- Read/Write 포인터로 데이터 관리
- 연속 메모리 재할당 없이 처리

**성능 최적화 (오른쪽 하단)**:
- 메시지 객체 풀링으로 메모리 효율성
- TCP 스트림의 정확한 패킷 분리
- 불완전한 패킷의 안전한 처리

이 구조는 TCP 스트림의 특성을 고려하여 안정적이고 효율적으로 패킷을 조립하는 전형적인 고성능 서버 패턴이다. 
  
```
// 수신 완료 처리 및 패킷 조립
void IOCPServerNet::DoRecv(OVERLAPPED_EX* pOverlappedEx, const DWORD ioSize) {
    Connection* pConnection = GetConnection(pOverlappedEx->ConnectionIndex);
    if (pConnection == nullptr) return;
    
    pConnection->DecrementRecvIORefCount();
    
    // 링버퍼에서 수신된 데이터 가져오기
    auto [remainByte, pNext] = pConnection->GetReceiveData(ioSize);
    
    // 패킷 조립 루프
    auto totalReadSize = PacketForwardingLoop(pConnection, remainByte, pNext);
    if (!totalReadSize) return;  // 에러 발생
    
    // 처리된 데이터만큼 읽기 포인터 이동
    pConnection->ReadRecvBuffer(totalReadSize.value());
    
    // 다음 수신 요청
    if (pConnection->PostRecv() != NetResult::Success) {
        if (pConnection->CloseComplete()) {
            HandleExceptionCloseConnection(pConnection);
        }
    }
}

// 패킷 조립 로직
std::optional<int> IOCPServerNet::PacketForwardingLoop(Connection* pConnection, 
                                                       int remainByte, char* pBuffer) {
    const int PACKET_HEADER_LENGTH = 5;  // PacketLength(2) + PacketId(2) + Type(1)
    const int PACKET_SIZE_LENGTH = 2;
    
    int totalReadSize = 0;
    
    while (true) {
        // 헤더 크기만큼 데이터가 있는지 확인
        if (remainByte < PACKET_HEADER_LENGTH) {
            break;  // 헤더가 완전하지 않음
        }
        
        // 패킷 크기 읽기
        short packetSize = 0;
        CopyMemory(&packetSize, pBuffer, PACKET_SIZE_LENGTH);
        
        // 패킷 크기 유효성 검사
        if (0 >= packetSize || packetSize > pConnection->RecvBufferSize()) {
            char logmsg[128] = { 0, };
            sprintf_s(logmsg, "DoRecv. Arrived Wrong Packet. Size:%d", packetSize);
            LogFuncPtr((int)LogLevel::Error, logmsg);
            
            if (pConnection->CloseComplete()) {
                HandleExceptionCloseConnection(pConnection);
            }
            return std::nullopt;  // 에러
        }
        
        // 완전한 패킷이 수신되었는지 확인
        if (remainByte >= packetSize) {
            // 메시지 풀에서 메시지 객체 할당
            auto pMsg = m_pMsgPool->AllocMsg();
            if (pMsg == nullptr) {
                return totalReadSize;  // 풀이 비어있음, 나중에 다시 처리
            }
            
            // 메시지 설정
            pMsg->SetMessage(MessageType::OnRecv, pBuffer);
            
            // Logic IOCP로 전달
            if (PostNetMessage(pConnection, pMsg, packetSize) != NetResult::Success) {
                m_pMsgPool->DeallocMsg(pMsg);  // 실패 시 메시지 반환
                return totalReadSize;
            }
            
            // 다음 패킷 처리를 위한 포인터 이동
            remainByte -= packetSize;
            totalReadSize += packetSize;
            pBuffer += packetSize;
        } else {
            break;  // 패킷이 완전하지 않음, 더 기다림
        }
    }
    
    return totalReadSize;
}
```  
  
### 5. 링버퍼 구현
![](./images/228.png)   
   
![](./images/229.png)     
 
  
### 6. 채팅 서버 패킷 처리 예제
  
```
// 로그인 패킷 처리
void PacketManager::ProcessLogin(const INT32 connIndex, char* pBuf, INT16 copySize) {
    // 패킷 크기 검증
    if (LOGIN_REQUEST_PACKET_SZIE != copySize) {
        return;
    }
    
    auto pLoginReqPacket = reinterpret_cast<LOGIN_REQUEST_PACKET*>(pBuf);
    auto pUserID = pLoginReqPacket->UserID;
    
    // 응답 패킷 준비
    LOGIN_RESPONSE_PACKET loginResPacket;
    loginResPacket.PacketId = (UINT16)PACKET_ID::LOGIN_RESPONSE;
    loginResPacket.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);
    
    // 최대 접속자 수 확인
    if (m_pUserManager->GetCurrentUserCnt() >= m_pUserManager->GetMaxUserCnt()) {
        loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USER_USED_ALL_OBJ;
        SendPacketFunc(connIndex, &loginResPacket, sizeof(LOGIN_RESPONSE_PACKET));
        return;
    }
    
    // 중복 로그인 확인
    if (m_pUserManager->FindUserIndexByID(pUserID) == -1) {
        // 새로운 사용자 등록
        m_pUserManager->AddUser(pUserID, connIndex);
        loginResPacket.Result = (UINT16)ERROR_CODE::NONE;
    } else {
        // 이미 접속 중인 사용자
        loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
    }
    
    // 응답 전송
    SendPacketFunc(connIndex, &loginResPacket, sizeof(LOGIN_RESPONSE_PACKET));
}

// 채팅 메시지 처리
void PacketManager::ProcessRoomChatMessage(INT32 connIndex, char* pBuf, INT16 copySize) {
    auto pRoomChatReqPacket = reinterpret_cast<ROOM_CHAT_REQUEST_PACKET*>(pBuf);
    
    // 응답 패킷 준비
    ROOM_CHAT_RESPONSE_PACKET roomChatResPacket;
    roomChatResPacket.PacketId = (UINT16)PACKET_ID::ROOM_CHAT_RESPONSE;
    roomChatResPacket.PacketLength = sizeof(ROOM_CHAT_RESPONSE_PACKET);
    roomChatResPacket.Result = (INT16)ERROR_CODE::NONE;
    
    // 사용자 정보 조회
    auto reqUser = m_pUserManager->GetUserByConnIdx(connIndex);
    auto roomNum = reqUser->GetCurrentRoom();
    
    // 방 정보 조회
    auto pRoom = m_pRoomManager->GetRoomByNumber(roomNum);
    if (pRoom == nullptr) {
        roomChatResPacket.Result = (INT16)ERROR_CODE::CHAT_ROOM_INVALID_ROOM_NUMBER;
        SendPacketFunc(connIndex, &roomChatResPacket, sizeof(ROOM_CHAT_RESPONSE_PACKET));
        return;
    }
    
    // 성공 응답 전송
    SendPacketFunc(connIndex, &roomChatResPacket, sizeof(ROOM_CHAT_RESPONSE_PACKET));
    
    // 방의 모든 사용자에게 채팅 메시지 브로드캐스트
    pRoom->NotifyChat(connIndex, reqUser->GetUserId().c_str(), 
                     pRoomChatReqPacket->Message);
}  
```  
    

## IocpNetLib

### 주요 코드 흐름 다이어그램  
![](./images/092.png)    
![](./images/093.png)  
![](./images/094.png)  
![](./images/095.png)  
![](./images/096.png)  
![](./images/097.png)  

#### 1. **서버 시작 과정**
- `Start()` → `CreateListenSocket()` → `CreateHandleIOCP()` → `CreateConnections()` → `CreateWorkThread()`
- 모든 Connection 객체를 미리 생성하고 `AcceptEx()` 대기 상태로 설정

#### 2. **비동기 연결 처리**
- `AcceptEx()` 완료 → Work Thread에서 `DoAccept()` → IOCP 바인딩 → `PostRecv()` 시작
- Logic Thread에 CONNECTION 메시지 전달

#### 3. **패킷 처리 분리**
- **Work Thread**: 순수 네트워크 I/O (WSARecv/WSASend)
- **Logic Thread**: 패킷 분석 및 애플리케이션 로직


### 메모리 구조 및 데이터 흐름
![](./images/098.png)   
![](./images/099.png)   
![](./images/100.png)   
![](./images/101.png)  
![](./images/102.png)   
    
#### 1. **메모리 최적화**
- **객체 풀링**: Connection, Message 객체 재사용
- **링버퍼**: 순환 버퍼로 메모리 재할당 최소화
- **Zero-Copy**: 포인터 기반 데이터 처리

#### 2. **스레드 아키텍처**
- **Work IOCP**: 네트워크 I/O 전담 (CPU 효율성)
- **Logic IOCP**: 게임 로직 전담 (확장성)
- **성능 모니터링**: 별도 스레드에서 처리

   
### IOCPServerNet 클래스

**전체 서버의 핵심 관리자 역할**

#### 주요 멤버 변수
```cpp
NetConfig m_NetConfig;                          // 서버 설정
SOCKET m_ListenSocket;                          // 리슨 소켓
std::vector<Connection*> m_Connections;         // 연결 배열
HANDLE m_hWorkIOCP;                            // 워커 IOCP 핸들
HANDLE m_hLogicIOCP;                           // 로직 IOCP 핸들
std::vector<std::unique_ptr<std::thread>> m_WorkThreads;  // 워커 스레드들
std::unique_ptr<MessagePool> m_pMsgPool;       // 메시지 풀
std::unique_ptr<Performance> m_Performance;    // 성능 측정기
```

#### 핵심 동작 과정
1. **서버 시작 (Start)**:
   ```cpp
   NetResult Start(NetConfig netConfig) {
       CreateListenSocket();      // 리슨 소켓 생성
       CreateHandleIOCP();        // IOCP 핸들 생성
       CreateMessageManager();    // 메시지 풀 생성
       LinkListenSocketIOCP();    // 리슨 소켓을 IOCP에 연결
       CreateConnections();       // Connection 객체들 미리 생성
       CreateWorkThread();        // 워커 스레드 생성
   }
   ```

2. **워커 스레드 동작**:
   ```cpp
   void WorkThread() {
       while (m_IsRunWorkThread) {
           GetQueuedCompletionStatus(m_hWorkIOCP, ...);  // IOCP에서 완료 통지 대기
           
           switch (pOverlappedEx->OverlappedExOperationType) {
               case Accept: DoAccept(); break;
               case Recv:   DoRecv();   break;
               case Send:   DoSend();   break;
           }
       }
   }
   ```

3. **패킷 분해 로직**:
   ```cpp
   std::optional<int> PacketForwardingLoop(Connection* pConnection, int remainByte, char* pBuffer) {
       while (true) {
           // 패킷 헤더 크기 확인 (5바이트)
           short packetSize;
           CopyMemory(&packetSize, pBuffer, 2);  // 패킷 크기 읽기
           
           if (remainByte >= packetSize) {
               // 완전한 패킷이 있으면 로직 스레드로 전달
               PostNetMessage(pConnection, pMsg, packetSize);
           }
       }
   }
   ```

### Connection 클래스

**개별 클라이언트 연결의 모든 것을 관리**

#### 주요 멤버 변수
```cpp
int m_Index;                           // 연결 인덱스
SOCKET m_ClientSocket;                 // 클라이언트 소켓
SOCKET m_ListenSocket;                 // 리슨 소켓 참조
OVERLAPPED_EX* m_pRecvOverlappedEx;    // 수신용 OVERLAPPED 구조체
OVERLAPPED_EX* m_pSendOverlappedEx;    // 송신용 OVERLAPPED 구조체
RecvRingBuffer m_RecvRingBuffer;       // 수신 링버퍼
SendRingBuffer m_SendRingBuffer;       // 송신 링버퍼
char m_AddrBuf[MAX_ADDR_LENGTH];       // AcceptEx용 주소 버퍼
BOOL m_IsConnect;                      // 연결 상태
DWORD m_SendIORefCount;                // 송신 I/O 참조 카운트
DWORD m_RecvIORefCount;                // 수신 I/O 참조 카운트
std::atomic<short> m_AcceptIORefCount; // Accept I/O 참조 카운트
```

#### 핵심 기능들

1. **Accept 처리**:
   ```cpp
   NetResult BindAcceptExSocket() {
       m_ClientSocket = WSASocket(...);  // 새 소켓 생성
       
       AcceptEx(m_ListenSocket,          // AcceptEx로 비동기 Accept
                m_ClientSocket,
                m_AddrBuf,               // 클라이언트 주소 정보도 함께 받음
                0,                       // 데이터는 받지 않음
                sizeof(SOCKADDR_IN) + 16,
                sizeof(SOCKADDR_IN) + 16,
                &acceptByte,
                m_pRecvOverlappedEx);
   }
   ```

2. **수신 처리**:
   ```cpp
   NetResult PostRecv() {
       m_pRecvOverlappedEx->OverlappedExWsaBuf.buf = 
           m_RecvRingBuffer.GetWriteBuffer(m_RecvBufSize);  // 링버퍼에서 쓰기 가능 영역 획득
       
       WSARecv(m_ClientSocket,           // 비동기 수신 요청
               &m_pRecvOverlappedEx->OverlappedExWsaBuf,
               1, &recvByte, &flag,
               &m_pRecvOverlappedEx->Overlapped, NULL);
   }
   ```

3. **송신 처리**:
   ```cpp
   bool PostSend() {
       if (InterlockedCompareExchange(&m_IsSendable, FALSE, TRUE)) {  // 송신 중복 방지
           auto [sendSize, pSendData] = m_SendRingBuffer.GetSendAbleData(m_SendBufSize);
           
           WSASend(m_ClientSocket,       // 비동기 송신 요청
                   &m_pSendOverlappedEx->OverlappedExWsaBuf,
                   1, &sendByte, flag,
                   &m_pSendOverlappedEx->Overlapped, NULL);
       }
   }
   ```

4. **연결 종료 처리**:
   ```cpp
   bool CloseComplete() {
       // 모든 I/O 작업이 완료될 때까지 대기
       if (IsConnect() && (m_AcceptIORefCount != 0 || 
                          m_RecvIORefCount != 0 || 
                          m_SendIORefCount != 0)) {
           DisconnectConnection();
           return false;
       }
       // 한 번만 종료 처리
       return InterlockedCompareExchange(&m_IsClosed, TRUE, FALSE) == FALSE;
   }
   ```

### MessagePool 클래스

**메시지 객체의 효율적 관리**

#### 구조
```cpp
class MessagePool {
private:
    concurrency::concurrent_queue<Message*> m_MessagePool;  // 스레드 안전 큐
    int m_MaxMessagePoolCount;     // 기본 풀 크기
    int m_ExtraMessagePoolCount;   // 추가 풀 크기
};
```

#### 동작 방식
1. **초기화 시점에 모든 Message 객체 미리 생성**:
   ```cpp
   bool CreateMessagePool() {
       for (int i = 0; i < m_MaxMessagePoolCount; ++i) {
           Message* pMsg = new Message;
           m_MessagePool.push(pMsg);  // 풀에 추가
       }
   }
   ```

2. **할당/해제**:
   ```cpp
   Message* AllocMsg() {
       Message* pMsg = nullptr;
       if (!m_MessagePool.try_pop(pMsg)) {  // 논블로킹 pop
           return nullptr;  // 풀이 비어있으면 nullptr 반환
       }
       return pMsg;
   }
   
   bool DeallocMsg(Message* pMsg) {
       pMsg->Clear();            // 메시지 내용 초기화
       m_MessagePool.push(pMsg); // 풀로 반환
       return true;
   }
   ```

### RecvRingBuffer 클래스

**수신 데이터의 순환 버퍼 관리**

#### 멤버 변수
```cpp
char* m_pRingBuffer;     // 실제 버퍼
int m_RingBufferSize;    // 버퍼 크기
int m_ReadPos;           // 읽기 위치
int m_WritePos;          // 쓰기 위치
```

#### 핵심 동작

1. **쓰기 버퍼 획득**:
   ```cpp
   char* GetWriteBuffer(const int wantedSize) {
       if (auto size = m_RingBufferSize - m_WritePos; size < wantedSize) {
           // 버퍼 끝에 공간이 부족하면 앞으로 데이터 이동
           auto remain = m_WritePos - m_ReadPos;
           if (remain > 0) {
               CopyMemory(&m_pRingBuffer[0], &m_pRingBuffer[m_ReadPos], remain);
               m_WritePos = remain;
               m_ReadPos = 0;
           }
       }
       return &m_pRingBuffer[m_WritePos];  // 쓰기 가능한 위치 반환
   }
   ```

2. **수신 데이터 처리**:
   ```cpp
   std::tuple<int, char*> GetReceiveData(int recvSize) {
       m_WritePos += recvSize;              // 쓰기 위치 갱신
       auto remain = m_WritePos - m_ReadPos; // 읽을 수 있는 데이터 크기
       return { remain, &m_pRingBuffer[m_ReadPos] };
   }
   ```

### SendRingBuffer 클래스

**송신 데이터의 순환 버퍼 관리 (스레드 안전)**

#### 특징
- RecvRingBuffer와 유사하지만 **SpinLock으로 동기화**
- 여러 스레드에서 동시에 송신 데이터를 추가할 수 있음

#### 핵심 동작
```cpp
bool AddSendData(const int dataSize, const char* pData) {
    SpinLockGuard Lock(&m_CS);  // 스핀락으로 보호
    
    // 버퍼 공간 확인 및 재배치
    if (auto size = m_RingBufferSize - m_WritePos; size < dataSize) {
        auto remain = m_WritePos - m_ReadPos;
        if (remain > 0) {
            CopyMemory(&m_pRingBuffer[0], &m_pRingBuffer[m_ReadPos], remain);
        }
        m_WritePos = remain;
        m_ReadPos = 0;
    }
    
    CopyMemory(&m_pRingBuffer[m_WritePos], pData, dataSize);  // 데이터 복사
    return true;
}
```


### MiniDump 클래스

**크래시 덤프 자동 생성**

#### 핵심 기능
```cpp
inline static LONG WINAPI UnHandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo) {
    if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
        // 스택 오버플로우는 별도 스레드에서 처리
        std::thread overflowThread = std::thread(WriteDump, pExceptionInfo);
        overflowThread.join();
    } else {
        return WriteDump(pExceptionInfo);  // 일반 예외는 바로 처리
    }
}
```

#### 덤프 파일 생성
```cpp
static DWORD WINAPI WriteDump(LPVOID pParam) {
    // DBGHELP.DLL에서 MiniDumpWriteDump 함수 로드
    auto dump = (MINIDUMPWRITEDUMP)GetProcAddress(dllHandle, "MiniDumpWriteDump");
    
    // 현재 시간으로 파일명 생성
    swprintf_s(szDumpPath, L"..\\Bin\\Dumps\\%d-%d-%d %d_%d_%d.dmp", ...);
    
    // 덤프 파일 생성
    dump(GetCurrentProcess(), GetCurrentProcessId(), hFileHandle, 
         MiniDumpNormal, &miniDumpExceptionInfo, NULL, NULL);
}
```

### Lock 클래스 (SpinLock)

**고성능 스핀락 구현**

#### 구조
```cpp
struct CustomSpinLockCriticalSection {
    CRITICAL_SECTION m_CS;
    
    CustomSpinLockCriticalSection() {
        InitializeCriticalSectionAndSpinCount(&m_CS, 1000);  // 1000번 스핀 후 블록
    }
};

class SpinLockGuard {
    CRITICAL_SECTION* m_pSpinCS;
public:
    explicit SpinLockGuard(CustomSpinLockCriticalSection* pCS) : m_pSpinCS(&pCS->m_CS) {
        EnterCriticalSection(m_pSpinCS);
    }
    ~SpinLockGuard() {
        LeaveCriticalSection(m_pSpinCS);
    }
};
```

**사용법**:
```cpp
CustomSpinLockCriticalSection m_CS;

void SomeFunction() {
    SpinLockGuard Lock(&m_CS);  // 생성자에서 락 획득
    // 임계 영역 코드
}  // 소멸자에서 자동으로 락 해제
```

  
## IocpChatServer
  
### IocpChatServer 코드 흐름 다이어그램
![](./images/103.png)   
![](./images/104.png)   
![](./images/105.png)   
![](./images/106.png)   
![](./images/107.png)   
![](./images/108.png)   
![](./images/109.png)   
![](./images/110.png)   
![](./images/111.png)   


### 채팅 서버 패킷 구조 및 실제 시나리오
![](./images/112.png)    
![](./images/113.png)    
![](./images/114.png)    
![](./images/115.png)    
```
void Room::NotifyChat(INT32 connIndex, const char* UserID, const char* Msg) 
{ 
    Common::ROOM_CHAT_NOTIFY_PACKET roomChatNtfyPkt; 
    roomChatNtfyPkt.PacketId = (UINT16)Common::PACKET_ID::ROOM_CHAT_NOTIFY; 
    roomChatNtfyPkt.PacketLength = sizeof(roomChatNtfyPkt); 
    CopyMemory(roomChatNtfyPkt.Msg, Msg, sizeof(roomChatNtfyPkt.Msg)); 
    CopyMemory(roomChatNtfyPkt.UserID, UserID, sizeof(roomChatNtfyPkt.UserID)); 
    
    // 방의 모든 사용자에게 브로드캐스트 
    for (auto pUser : m_UserList) 
    { 
        if (pUser == nullptr) 
        {
            continue; 
        }
        
        SendPacketFunc(pUser->GetNetConnIdx(), &roomChatNtfyPkt, sizeof(roomChatNtfyPkt)); 
    } 
}
```  

![](./images/116.png)    
```
// 확장성을 위한 개선 예시 
class AsyncMessageBroadcaster 
{ 
private: 
    std::queue<BroadcastMessage> m_MessageQueue; 
    std::thread m_BroadcastThread; 
    
public: 
    void QueueBroadcast(const BroadcastMessage& msg) 
    { 
        // 메시지를 큐에 추가하고 별도 스레드에서 처리 
        m_MessageQueue.push(msg); 
    } 
    
    void ProcessBroadcastQueue() 
    { 
        // 배치 처리로 브로드캐스트 성능 최적화 
        while (!m_MessageQueue.empty()) 
        { 
            auto msg = m_MessageQueue.front(); 
            m_MessageQueue.pop(); 
            
            // 실제 브로드캐스트 처리 
        } 
    } 
};
```


#### **계층화된 아키텍처**
- **IocpNetLib**: 재사용 가능한 네트워크 라이브러리
- **ChatServerLib**: 채팅 전용 비즈니스 로직
- **명확한 책임 분리**: 네트워크 ↔ 로직 계층 분리

#### **효율적인 데이터 관리**
```cpp
// O(1) 접근을 위한 최적화된 자료구조
std::vector<User*> UserObjPool;                    // Connection Index로 직접 접근
std::unordered_map<std::string, int> UserDictionary; // UserID → ConnIndex 매핑
std::vector<Room*> m_RoomList;                      // 방 번호로 배열 인덱스 계산
```


### 클래스 다이어그램 및 관계

![](./images/117.png)  
![](./images/118.png)  
![](./images/119.png)  


### 런타임 객체 관계 및 데이터 흐름
  
![](./images/120.png)  
![](./images/121.png)  
![](./images/122.png)  
  

### Main 클래스

**채팅 서버의 최상위 관리자 클래스**

#### 주요 멤버 변수
```cpp
std::unique_ptr<NetLib::IOCPServerNet> m_pIOCPServer;  // 네트워크 서버
std::unique_ptr<UserManager> m_pUserManager;          // 사용자 관리자
std::unique_ptr<PacketManager> m_pPacketManager;      // 패킷 관리자
std::unique_ptr<RoomManager> m_pRoomManager;          // 방 관리자
ChatServerConfig m_Config;                            // 서버 설정
bool m_IsRun;                                         // 실행 상태
```

#### 핵심 동작 과정

1. **서버 초기화**:
   ```cpp
   int Init(ChatServerConfig serverConfig) {
       m_pIOCPServer = std::make_unique<NetLib::IOCPServerNet>();
       
       // 네트워크 서버 시작
       auto ServerStartResult = m_pIOCPServer->Start(netConfig);
       
       // 각 매니저 생성 및 초기화
       m_pPacketManager = std::make_unique<PacketManager>();
       m_pUserManager = std::make_unique<UserManager>();
       m_pRoomManager = std::make_unique<RoomManager>();
       
       // 매니저들 간의 의존성 설정
       auto sendPacketFunc = [&](INT32 connectionIndex, void* pSendPacket, INT16 packetSize) {
           m_pIOCPServer->SendPacket(connectionIndex, pSendPacket, packetSize);
       };
       
       m_pPacketManager->SendPacketFunc = sendPacketFunc;
       m_pRoomManager->SendPacketFunc = sendPacketFunc;
   }
   ```

2. **메인 실행 루프**:
   ```cpp
   void Run() {
       while (m_IsRun) {
           INT8 operationType = 0;
           INT32 connectionIndex = 0;
           INT16 copySize = 0;
           
           // 네트워크 메시지 처리
           if (m_pIOCPServer->ProcessNetworkMessage(operationType, connectionIndex, pBuf, copySize, waitTimeMillisec)) {
               auto msgType = (NetLib::MessageType)operationType;
               
               switch (msgType) {
                   case NetLib::MessageType::Connection:
                       printf("On Connect %d\n", connectionIndex);
                       break;
                   case NetLib::MessageType::Close:
                       m_pPacketManager->ClearConnectionInfo(connectionIndex);  // 연결 정리
                       break;
                   case NetLib::MessageType::OnRecv:
                       m_pPacketManager->ProcessRecvPacket(connectionIndex, pBuf, copySize);  // 패킷 처리
                       break;
               }
           }
       }
   }
   ```

### PacketManager 클래스

**모든 패킷 처리를 담당하는 핵심 클래스**

#### 주요 멤버 변수
```cpp
typedef void(PacketManager::* PROCESS_RECV_PACKET_FUNCTION)(INT32, char*, INT16);
std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION> m_RecvFuntionDictionary;  // 패킷 핸들러 맵
UserManager* m_pUserManager;                          // 사용자 관리자 참조
RoomManager* m_pRoomManager;                          // 방 관리자 참조
std::function<void(INT32, void*, INT16)> SendPacketFunc;  // 패킷 송신 함수
```

#### 초기화 및 패킷 핸들러 등록
```cpp
void Init(UserManager* pUserManager, RoomManager* pRoomManager) {
    m_RecvFuntionDictionary = std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION>();
    
    // 패킷 ID별 핸들러 함수 등록
    m_RecvFuntionDictionary[(int)PACKET_ID::LOGIN_REQUEST] = &PacketManager::ProcessLogin;
    m_RecvFuntionDictionary[(int)PACKET_ID::ROOM_ENTER_REQUEST] = &PacketManager::ProcessEnterRoom;
    m_RecvFuntionDictionary[(int)PACKET_ID::ROOM_LEAVE_REQUEST] = &PacketManager::ProcessLeaveRoom;
    m_RecvFuntionDictionary[(int)PACKET_ID::ROOM_CHAT_REQUEST] = &PacketManager::ProcessRoomChatMessage;
}
```

#### 핵심 패킷 처리 함수들

1. **로그인 처리**:
   ```cpp
   void ProcessLogin(const INT32 connIndex, char* pBuf, INT16 copySize) {
       auto pLoginReqPacket = reinterpret_cast<LOGIN_REQUEST_PACKET*>(pBuf);
       auto pUserID = pLoginReqPacket->UserID;
       
       LOGIN_RESPONSE_PACKET loginResPacket;
       loginResPacket.PacketId = (UINT16)PACKET_ID::LOGIN_RESPONSE;
       loginResPacket.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);
       
       // 최대 접속자 수 체크
       if (m_pUserManager->GetCurrentUserCnt() >= m_pUserManager->GetMaxUserCnt()) {
           loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USER_USED_ALL_OBJ;
           SendPacketFunc(connIndex, &loginResPacket, sizeof(LOGIN_RESPONSE_PACKET));
           return;
       }
       
       // 중복 로그인 체크
       if (m_pUserManager->FindUserIndexByID(pUserID) == -1) {
           m_pUserManager->AddUser(pUserID, connIndex);  // 새 사용자 추가
           loginResPacket.Result = (UINT16)ERROR_CODE::NONE;
       } else {
           loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
       }
       
       SendPacketFunc(connIndex, &loginResPacket, sizeof(LOGIN_RESPONSE_PACKET));
   }
   ```

2. **방 입장 처리**:
   ```cpp
   void ProcessEnterRoom(INT32 connIndex, char* pBuf, INT16 copySize) {
       auto pRoomEnterReqPacket = reinterpret_cast<ROOM_ENTER_REQUEST_PACKET*>(pBuf);
       auto pReqUser = m_pUserManager->GetUserByConnIdx(connIndex);
       
       ROOM_ENTER_RESPONSE_PACKET roomEnterResPacket;
       roomEnterResPacket.Result = m_pRoomManager->EnterUser(pRoomEnterReqPacket->RoomNumber, pReqUser);
       
       SendPacketFunc(connIndex, &roomEnterResPacket, sizeof(ROOM_ENTER_RESPONSE_PACKET));
   }
   ```

3. **채팅 메시지 처리**:
   ```cpp
   void ProcessRoomChatMessage(INT32 connIndex, char* pBuf, INT16 copySize) {
       auto pRoomChatReqPacket = reinterpret_cast<ROOM_CHAT_REQUEST_PACKET*>(pBuf);
       
       auto reqUser = m_pUserManager->GetUserByConnIdx(connIndex);
       auto roomNum = reqUser->GetCurrentRoom();
       auto pRoom = m_pRoomManager->GetRoomByNumber(roomNum);
       
       // 응답 패킷 전송
       ROOM_CHAT_RESPONSE_PACKET roomChatResPacket;
       roomChatResPacket.Result = (INT16)ERROR_CODE::NONE;
       SendPacketFunc(connIndex, &roomChatResPacket, sizeof(ROOM_CHAT_RESPONSE_PACKET));
       
       // 방의 모든 사용자에게 채팅 메시지 브로드캐스트
       pRoom->NotifyChat(connIndex, reqUser->GetUserId().c_str(), pRoomChatReqPacket->Message);
   }
   ```

### UserManager 클래스

**모든 접속 사용자 관리**

#### 주요 멤버 변수
```cpp
INT32 m_MaxUserCnt;                              // 최대 사용자 수
INT32 m_CurrentUserCnt;                          // 현재 사용자 수
std::vector<User*> UserObjPool;                  // 사용자 객체 풀
std::unordered_map<std::string, int> UserDictionary;  // 사용자 ID -> 연결 인덱스 맵
```

#### 핵심 기능

1. **사용자 풀 초기화**:
   ```cpp
   void Init(const INT32 maxUserCount) {
       m_MaxUserCnt = maxUserCount;
       UserObjPool = std::vector<User*>(m_MaxUserCnt);
       
       // 최대 사용자 수만큼 User 객체 미리 생성
       for (auto i = 0; i < m_MaxUserCnt; i++) {
           UserObjPool[i] = new User();
           UserObjPool[i]->Init(i);  // 연결 인덱스로 초기화
       }
   }
   ```

2. **사용자 추가**:
   ```cpp
   ERROR_CODE AddUser(char* userID, int conn_idx) {
       auto user_idx = conn_idx;  // 연결 인덱스를 사용자 인덱스로 사용
       
       UserObjPool[user_idx]->SetLogin(userID);  // 사용자 로그인 설정
       UserDictionary.insert(std::pair<char*, int>(userID, conn_idx));  // ID로 검색 가능하게 등록
       
       return ERROR_CODE::NONE;
   }
   ```

3. **사용자 검색**:
   ```cpp
   INT32 FindUserIndexByID(char* userID) {
       if (auto res = UserDictionary.find(userID); res != UserDictionary.end()) {
           return (*res).second;  // 연결 인덱스 반환
       }
       return -1;  // 찾지 못함
   }
   
   User* GetUserByConnIdx(INT32 conn_idx) {
       return UserObjPool[conn_idx];  // 연결 인덱스로 직접 접근
   }
   ```

### User 클래스

**개별 사용자 정보 관리**

#### 주요 멤버 변수
```cpp
INT32 m_Index;                    // 연결 인덱스 (네트워크 연결과 동일)
INT32 m_RoomIndex;                // 현재 입장한 방 번호
std::string m_UserId;             // 사용자 ID
bool m_IsConfirm;                 // 인증 상태
DOMAIN_STATE m_CurDomainState;    // 현재 도메인 상태
```

#### 도메인 상태 관리
```cpp
enum class DOMAIN_STATE {
    NONE = 0,    // 초기 상태
    LOGIN = 1,   // 로그인 완료
    ROOM = 2     // 방 입장 상태
};
```

#### 핵심 기능

1. **로그인 처리**:
   ```cpp
   int SetLogin(char* login_id) {
       m_CurDomainState = DOMAIN_STATE::LOGIN;
       m_UserId = login_id;
       return 0;
   }
   ```

2. **방 입장 처리**:
   ```cpp
   void EnterRoom(INT32 roomIndex) {
       m_RoomIndex = roomIndex;
       m_CurDomainState = DOMAIN_STATE::ROOM;
   }
   ```

3. **사용자 정보 초기화**:
   ```cpp
   void Clear() {
       m_RoomIndex = -1;
       m_UserId = "";
       m_IsConfirm = false;
       m_CurDomainState = DOMAIN_STATE::NONE;
   }
   ```

### RoomManager 클래스

**모든 채팅방 관리**

#### 주요 멤버 변수
```cpp
std::vector<Room*> m_RoomList;      // 방 객체 배열
INT32 m_BeginRoomNumber;            // 시작 방 번호
INT32 m_EndRoomNumber;              // 끝 방 번호
INT32 m_MaxRoomCount;               // 최대 방 개수
std::function<void(INT32, void*, INT16)> SendPacketFunc;  // 패킷 송신 함수
```

#### 핵심 기능

1. **방 풀 초기화**:
   ```cpp
   void Init(const INT32 beginRoomNumber, const INT32 maxRoomCount, const INT32 maxRoomUserCount) {
       m_BeginRoomNumber = beginRoomNumber;
       m_MaxRoomCount = maxRoomCount;
       m_EndRoomNumber = beginRoomNumber + maxRoomCount;
       
       m_RoomList = std::vector<Room*>(maxRoomCount);
       
       // 모든 방 객체 미리 생성
       for (auto i = 0; i < maxRoomCount; i++) {
           m_RoomList[i] = new Room();
           m_RoomList[i]->SendPacketFunc = SendPacketFunc;  // 송신 함수 설정
           m_RoomList[i]->Init((i + beginRoomNumber), maxRoomUserCount);
       }
   }
   ```

2. **방 입장 처리**:
   ```cpp
   UINT16 EnterUser(INT32 roomNumber, User* pUser) {
       auto pRoom = GetRoomByNumber(roomNumber);
       if (pRoom == nullptr) {
           return (UINT16)ERROR_CODE::ROOM_INVALID_INDEX;
       }
       
       return pRoom->EnterUser(pUser);  // 방에 사용자 입장 처리 위임
   }
   ```

3. **방 번호로 방 객체 획득**:
   ```cpp
   Room* GetRoomByNumber(INT32 number) {
       if (number < m_BeginRoomNumber || number >= m_EndRoomNumber) {
           return nullptr;  // 유효하지 않은 방 번호
       }
       
       auto index = (number - m_BeginRoomNumber);  // 배열 인덱스 계산
       return m_RoomList[index];
   }
   ```

### Room 클래스

**개별 채팅방 관리**

#### 주요 멤버 변수
```cpp
INT32 m_RoomNum;                    // 방 번호
std::list<User*> m_UserList;       // 방 안의 사용자 목록
INT32 m_MaxUserCount;               // 최대 수용 인원
UINT16 m_CurrentUserCount;          // 현재 인원
std::function<void(INT32, void*, INT16)> SendPacketFunc;  // 패킷 송신 함수
```

#### 핵심 기능

1. **사용자 입장**:
   ```cpp
   UINT16 EnterUser(User* pUser) {
       if (m_CurrentUserCount >= m_MaxUserCount) {
           return (UINT16)ERROR_CODE::ENTER_ROOM_FULL_USER;  // 방이 가득 참
       }
       
       m_UserList.push_back(pUser);  // 사용자 목록에 추가
       ++m_CurrentUserCount;
       
       pUser->EnterRoom(m_RoomNum);  // 사용자 객체에 방 정보 설정
       return (UINT16)ERROR_CODE::NONE;
   }
   ```

2. **사용자 퇴장**:
   ```cpp
   void LeaveUser(User* pLeaveUser) {
       // 람다를 사용한 조건부 제거
       m_UserList.remove_if([leaveUserId = pLeaveUser->GetUserId()](User *pUser) {
           return leaveUserId == pUser->GetUserId();
       });
   }
   ```

3. **채팅 메시지 브로드캐스트**:
   ```cpp
   void NotifyChat(INT32 connIndex, const char* UserID, const char* Msg) {
       ROOM_CHAT_NOTIFY_PACKET roomChatNtfyPkt;
       roomChatNtfyPkt.PacketId = (UINT16)PACKET_ID::ROOM_CHAT_NOTIFY;
       roomChatNtfyPkt.PacketLength = sizeof(roomChatNtfyPkt);
       
       CopyMemory(roomChatNtfyPkt.Msg, Msg, sizeof(roomChatNtfyPkt.Msg));
       CopyMemory(roomChatNtfyPkt.UserID, UserID, sizeof(roomChatNtfyPkt.UserID));
       
       SendToAllUser(sizeof(roomChatNtfyPkt), &roomChatNtfyPkt, connIndex, false);
   }
   ```

4. **방 안 모든 사용자에게 메시지 전송**:
   ```cpp
   void SendToAllUser(const UINT16 dataSize, void* pData, const INT32 passUserindex, bool exceptMe) {
       for (auto pUser : m_UserList) {
           if (pUser == nullptr) {
               continue;
           }
           
           if (exceptMe && pUser->GetNetConnIdx() == passUserindex) {
               continue;  // 자신 제외
           }
           
           SendPacketFunc(pUser->GetNetConnIdx(), pData, dataSize);  // 개별 사용자에게 전송
       }
   }
   ```

### 패킷 구조체들

#### 기본 패킷 헤더
```cpp
struct PACKET_HEADER {
    UINT16 PacketLength;  // 패킷 전체 길이
    UINT16 PacketId;      // 패킷 ID
    UINT8 Type;           // 패킷 속성 (압축, 암호화 등)
};
```

#### 주요 패킷들
```cpp
// 로그인 요청/응답
struct LOGIN_REQUEST_PACKET : public PACKET_HEADER {
    char UserID[MAX_USER_ID_LEN+1];
    char UserPW[MAX_USER_PW_LEN+1];
};

// 방 입장 요청/응답
struct ROOM_ENTER_REQUEST_PACKET : public PACKET_HEADER {
    INT32 RoomNumber;
};

// 채팅 메시지
struct ROOM_CHAT_REQUEST_PACKET : public PACKET_HEADER {
    char Message[MAX_CHAT_MSG_SIZE +1];
};

struct ROOM_CHAT_NOTIFY_PACKET : public PACKET_HEADER {
    char UserID[MAX_USER_ID_LEN + 1];
    char Msg[MAX_CHAT_MSG_SIZE + 1];
};
```

### 전체 아키텍처 흐름

```
클라이언트 → Main::Run() → PacketManager → UserManager/RoomManager/Room
                    ↓
              네트워크 송신 ← SendPacketFunc ← 각 Manager/Room
```

**주요 특징**:
1. **객체 풀링**: User, Room 객체들을 미리 생성하여 메모리 할당 오버헤드 제거
2. **함수 포인터 기반 콜백**: SendPacketFunc을 통한 느슨한 결합
3. **패킷 핸들러 맵**: unordered_map으로 패킷 ID별 빠른 핸들러 검색
4. **상태 기반 사용자 관리**: DOMAIN_STATE로 사용자 상태 추적
5. **브로드캐스트 최적화**: 방 단위로 효율적인 메시지 전파

이러한 구조로 확장 가능하고 효율적인 채팅 서버를 구현했다.  
  

