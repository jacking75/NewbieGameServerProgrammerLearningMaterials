# 게임 서버 개발자가 알아야할 TCP/IP Windows 소켓 프로그래밍

저자: 최흥배, Claude AI  

- C++23
- Windows 11
- Visual Studio 2022 이상
  

-----  
# Chapter.14 지연된 처리 및 배치 처리
게임 서버에서 성능 최적화의 핵심은 **작은 패킷들을 효율적으로 처리**하는 것이다. 개별적으로 처리하면 비효율적인 작은 I/O 작업들을 모아서 한 번에 처리하는 방법을 알아보자.

## 1. 지연된 처리(Deferred Processing)의 개념
지연된 처리는 즉시 처리하지 않고 **일정 시간 대기하거나 특정 조건이 될 때까지 모아서 처리**하는 방식이다.

### 게임 서버에서의 필요성
```cpp
// 비효율적인 즉시 처리 방식
void SendToClient(int clientId, const char* data, int size) {
    WSASend(clientSocket, &wsaBuf, 1, &bytesSent, 0, &overlapped, nullptr);
    // 매번 시스템 콜 발생 → 오버헤드 큼
}

// 게임에서 초당 수십~수백 개의 작은 패킷이 발생
SendPositionUpdate(clientId, x, y, z);     // 12바이트
SendHealthUpdate(clientId, hp);            // 4바이트  
SendInventoryUpdate(clientId, item, count); // 8바이트
// → 각각 개별 송신 시 시스템 콜 오버헤드 발생
```
  

## 2. 배치 처리 구현

### 기본 배치 버퍼 클래스
```cpp
class BatchBuffer {
private:
    static const int BATCH_SIZE = 8192;  // 8KB 배치 버퍼
    char buffer[BATCH_SIZE];
    int currentSize;
    DWORD lastFlushTime;
    static const DWORD FLUSH_INTERVAL = 10; // 10ms 후 강제 플러시

public:
    BatchBuffer() : currentSize(0), lastFlushTime(GetTickCount()) {}
    
    bool AddData(const char* data, int size) {
        // 버퍼 오버플로우 체크
        if (currentSize + size > BATCH_SIZE) {
            return false;  // 배치 플러시 필요
        }
        
        memcpy(buffer + currentSize, data, size);
        currentSize += size;
        return true;
    }
    
    bool ShouldFlush() const {
        DWORD currentTime = GetTickCount();
        return (currentSize > 0) && 
               ((currentSize >= BATCH_SIZE * 0.8) ||  // 80% 찼을 때
                (currentTime - lastFlushTime >= FLUSH_INTERVAL)); // 시간 초과
    }
    
    void Flush(SOCKET clientSocket) {
        if (currentSize == 0) return;
        
        WSABUF wsaBuf;
        wsaBuf.buf = buffer;
        wsaBuf.len = currentSize;
        
        DWORD bytesSent;
        OVERLAPPED* overlapped = new OVERLAPPED{0};
        
        WSASend(clientSocket, &wsaBuf, 1, &bytesSent, 0, overlapped, nullptr);
        
        currentSize = 0;
        lastFlushTime = GetTickCount();
    }
};
```

### 클라이언트별 배치 관리자
```cpp
class ClientBatchManager {
private:
    std::unordered_map<int, std::unique_ptr<BatchBuffer>> clientBuffers;
    std::mutex bufferMutex;
    
public:
    void SendData(int clientId, const char* data, int size) {
        std::lock_guard<std::mutex> lock(bufferMutex);
        
        auto& buffer = clientBuffers[clientId];
        if (!buffer) {
            buffer = std::make_unique<BatchBuffer>();
        }
        
        // 버퍼에 추가 시도
        if (!buffer->AddData(data, size)) {
            // 버퍼가 가득 참 → 먼저 플러시
            buffer->Flush(GetClientSocket(clientId));
            buffer->AddData(data, size);  // 다시 추가
        }
        
        // 플러시 조건 체크
        if (buffer->ShouldFlush()) {
            buffer->Flush(GetClientSocket(clientId));
        }
    }
    
    // 주기적으로 호출하여 대기 중인 데이터 플러시
    void FlushAll() {
        std::lock_guard<std::mutex> lock(bufferMutex);
        for (auto& [clientId, buffer] : clientBuffers) {
            if (buffer && buffer->ShouldFlush()) {
                buffer->Flush(GetClientSocket(clientId));
            }
        }
    }
};
```

## 3. 나글 알고리즘과 TCP_NODELAY

### 나글 알고리즘의 동작
나글 알고리즘은 **작은 패킷들을 자동으로 모아서 전송**하는 TCP의 기본 기능이다.

```cpp
// 나글 알고리즘이 활성화된 상태 (기본값)
send(socket, "A", 1, 0);      // 즉시 전송되지 않음
send(socket, "B", 1, 0);      // A와 합쳐져서 "AB"로 전송
send(socket, "C", 1, 0);      // 이전 ACK 대기 중이면 대기
```

### TCP_NODELAY 설정
```cpp
void ConfigureSocket(SOCKET sock, bool enableNodelay) {
    if (enableNodelay) {
        // 나글 알고리즘 비활성화 → 즉시 전송
        int flag = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, 
                  (char*)&flag, sizeof(flag));
    }
    
    // 추가 최적화 옵션들
    int sendBufferSize = 64 * 1024;  // 64KB 송신 버퍼
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, 
              (char*)&sendBufferSize, sizeof(sendBufferSize));
}
```

## 4. 균형점 찾기: 하이브리드 접근법

### 패킷 타입별 전략
```cpp
enum class PacketPriority {
    IMMEDIATE,    // 즉시 전송 (입력, 채팅)
    BATCHABLE,    // 배치 가능 (위치, 상태)
    PERIODIC      // 주기적 전송 (통계, 동기화)
};

class SmartBatchManager {
private:
    ClientBatchManager batchManager;
    std::thread flushThread;
    std::atomic<bool> running;
    
public:
    void SendPacket(int clientId, const char* data, int size, 
                   PacketPriority priority) {
        switch (priority) {
        case PacketPriority::IMMEDIATE:
            // TCP_NODELAY 소켓으로 즉시 전송
            ImmediateSend(clientId, data, size);
            break;
            
        case PacketPriority::BATCHABLE:
            // 배치 버퍼에 추가
            batchManager.SendData(clientId, data, size);
            break;
            
        case PacketPriority::PERIODIC:
            // 주기적 배치에 추가 (더 긴 대기시간)
            AddToPeriodicBatch(clientId, data, size);
            break;
        }
    }
    
private:
    void FlushWorker() {
        const int FLUSH_INTERVAL_MS = 5;  // 5ms마다 체크
        
        while (running) {
            batchManager.FlushAll();
            ProcessPeriodicBatch();
            std::this_thread::sleep_for(
                std::chrono::milliseconds(FLUSH_INTERVAL_MS));
        }
    }
};
```

### 게임별 최적화 전략
```cpp
// FPS 게임: 낮은 지연시간 우선
void ConfigureFPSGame(SmartBatchManager& manager) {
    // 입력, 샷 → 즉시 전송
    manager.SendPacket(clientId, inputData, size, PacketPriority::IMMEDIATE);
    
    // 위치 업데이트 → 짧은 배치 (2-5ms)
    manager.SendPacket(clientId, posData, size, PacketPriority::BATCHABLE);
}

// MMO 게임: 대역폭 효율성 우선  
void ConfigureMMOGame(SmartBatchManager& manager) {
    // 채팅, 중요 이벤트 → 즉시 전송
    manager.SendPacket(clientId, chatData, size, PacketPriority::IMMEDIATE);
    
    // 상태 변화 → 중간 배치 (10-20ms)
    manager.SendPacket(clientId, statusData, size, PacketPriority::BATCHABLE);
    
    // 주변 오브젝트 정보 → 긴 배치 (50-100ms)
    manager.SendPacket(clientId, objectData, size, PacketPriority::PERIODIC);
}
```

## 5. 성능 측정 및 모니터링

```cpp
class BatchMetrics {
public:
    struct Stats {
        uint64_t totalPacketsSent;
        uint64_t totalBytesSent;
        uint64_t batchCount;
        double avgBatchSize;
        double avgLatency;
    };
    
    void RecordBatch(int packetCount, int totalBytes, double latency) {
        stats.batchCount++;
        stats.totalPacketsSent += packetCount;
        stats.totalBytesSent += totalBytes;
        
        // 이동 평균으로 통계 업데이트
        stats.avgBatchSize = stats.avgBatchSize * 0.9 + 
                            (double)totalBytes * 0.1;
        stats.avgLatency = stats.avgLatency * 0.9 + latency * 0.1;
    }
    
    void PrintStats() {
        printf("배치 통계:\n");
        printf("- 총 배치: %llu개\n", stats.batchCount);
        printf("- 평균 배치 크기: %.1f바이트\n", stats.avgBatchSize);
        printf("- 평균 지연시간: %.2fms\n", stats.avgLatency);
        printf("- 압축률: %.1f%%\n", GetCompressionRatio());
    }
};
```

## 핵심 포인트

1. **배치 크기와 지연시간의 트레이드오프**: 큰 배치는 효율적이지만 지연시간 증가
2. **게임 장르별 최적화**: FPS는 지연시간, MMO는 대역폭 효율성 우선
3. **하이브리드 접근**: 패킷 중요도에 따라 즉시/배치 전송 선택
4. **모니터링**: 실시간 성능 측정으로 파라미터 조정

이런 방식으로 구현하면 **네트워크 처리량을 2-5배 향상**시킬 수 있으며, 특히 동접자가 많은 게임 서버에서 큰 효과를 볼 수 있다.
  




  

