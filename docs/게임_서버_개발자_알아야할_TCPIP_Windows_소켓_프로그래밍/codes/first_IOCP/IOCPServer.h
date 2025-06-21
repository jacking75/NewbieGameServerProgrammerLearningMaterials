#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <vector>
#include <string>
#include <thread>
#include <memory>
#include <format>

// 라이브러리 링크
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

// 상수 정의
constexpr int BUFFER_SIZE = 4096;

// IOCP 연산 타입 정의
enum class IOType {
    ACCEPT,
    RECV,
    SEND
};

// IOCP에서 사용할 확장 OVERLAPPED 구조체
struct IOContext {
    OVERLAPPED overlapped;
    WSABUF wsaBuf;
    IOType ioType;
    SOCKET socket;
    char buffer[BUFFER_SIZE];

    IOContext(IOType type);
    ~IOContext();
};

// 클라이언트 정보 구조체
struct ClientInfo {
    SOCKET socket;
    SOCKADDR_IN clientAddr;
    std::shared_ptr<IOContext> recvContext;
    std::shared_ptr<IOContext> sendContext;

    ClientInfo();
    ~ClientInfo();
};

// IOCP 서버 클래스
class IOCPServer {
private:
    SOCKET listenSocket;
    HANDLE iocpHandle;
    std::vector<std::thread> workerThreads;
    bool isRunning;
    LPFN_ACCEPTEX lpfnAcceptEx;

public:
    IOCPServer();
    ~IOCPServer();

    bool Initialize(const std::string& port, int workerThreadCount);
    void Start();
    void Stop();

private:
    void WorkerThread();
    void PostAccept();
    void HandleAccept(IOContext* pIOContext);
    void PostRecv(ClientInfo* pClientInfo);
    void HandleRecv(ClientInfo* pClientInfo, DWORD bytesTransferred);
    void PostSend(ClientInfo* pClientInfo, DWORD bytesToSend);
    void HandleSend(ClientInfo* pClientInfo, DWORD bytesTransferred);
};
