#include "IOCPServer.h"

// IOContext 구현
IOContext::IOContext(IOType type) : ioType(type), socket(INVALID_SOCKET) {
    ZeroMemory(&overlapped, sizeof(OVERLAPPED));
    ZeroMemory(buffer, BUFFER_SIZE);
    wsaBuf.buf = buffer;
    wsaBuf.len = BUFFER_SIZE;
}

IOContext::~IOContext() {
    if (socket != INVALID_SOCKET) {
        closesocket(socket);
        socket = INVALID_SOCKET;
    }
}

// ClientInfo 구현
ClientInfo::ClientInfo() : socket(INVALID_SOCKET) {
    ZeroMemory(&clientAddr, sizeof(clientAddr));
}

ClientInfo::~ClientInfo() {
    if (socket != INVALID_SOCKET) {
        closesocket(socket);
        socket = INVALID_SOCKET;
    }
}

// IOCPServer 구현
IOCPServer::IOCPServer() : listenSocket(INVALID_SOCKET), iocpHandle(NULL), isRunning(false), lpfnAcceptEx(nullptr) {}

IOCPServer::~IOCPServer() {
    Stop();
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
    }
    if (iocpHandle != NULL) {
        CloseHandle(iocpHandle);
    }
    WSACleanup();
}

bool IOCPServer::Initialize(const std::string& port, int workerThreadCount) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << std::format("WSAStartup 실패: {}\n", result);
        return false;
    }

    listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << std::format("소켓 생성 실패: {}\n", WSAGetLastError());
        WSACleanup();
        return false;
    }

    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(std::stoi(port));

    result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << std::format("bind 실패: {}\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes = 0;
    result = WSAIoctl(
        listenSocket,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidAcceptEx,
        sizeof(guidAcceptEx),
        &lpfnAcceptEx,
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

    iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (iocpHandle == NULL) {
        std::cerr << std::format("IOCP 생성 실패: {}\n", GetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    if (CreateIoCompletionPort((HANDLE)listenSocket, iocpHandle, (ULONG_PTR)nullptr, 0) == NULL) {
        std::cerr << std::format("리슨 소켓을 IOCP에 연결 실패: {}\n", GetLastError());
        CloseHandle(iocpHandle);
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    isRunning = true;
    for (int i = 0; i < workerThreadCount; i++) {
        workerThreads.emplace_back(&IOCPServer::WorkerThread, this);
    }

    return true;
}

void IOCPServer::Start() {
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << std::format("listen 실패: {}\n", WSAGetLastError());
        return;
    }

    std::cout << "서버가 시작되었습니다. 클라이언트 대기 중...\n";

    for (int i = 0; i < 10; i++) {
        PostAccept();
    }
}

void IOCPServer::Stop() {
    isRunning = false;

    for (size_t i = 0; i < workerThreads.size(); i++) {
        PostQueuedCompletionStatus(iocpHandle, 0, 0, NULL);
    }

    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads.clear();
}

void IOCPServer::WorkerThread() {
    while (isRunning) {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        LPOVERLAPPED pOverlapped = nullptr;

        BOOL result = GetQueuedCompletionStatus(
            iocpHandle,
            &bytesTransferred,
            &completionKey,
            &pOverlapped,
            INFINITE
        );

        if (!isRunning) {
            break;
        }

        if (!result || (bytesTransferred == 0 && pOverlapped != nullptr)) {
            if (pOverlapped != nullptr) {
                IOContext* pIOContext = CONTAINING_RECORD(pOverlapped, IOContext, overlapped);

                if (!result) {
                    int error = GetLastError();
                    if (error != ERROR_OPERATION_ABORTED) {
                        std::cerr << std::format("I/O 작업 실패 (GetQueuedCompletionStatus): {}\n", error);
                    }
                }

                ClientInfo* pClientInfo = (ClientInfo*)completionKey;
                if (pClientInfo != nullptr) {
                    std::cout << std::format("클라이언트 연결 종료 또는 오류: 소켓 {}\n", (int)pClientInfo->socket);
                    delete pClientInfo;
                }
                else {
                    if (pIOContext->ioType == IOType::ACCEPT && pIOContext->socket != INVALID_SOCKET) {
                        std::cout << std::format("Accept 작업 관련 소켓 정리: {}\n", (int)pIOContext->socket);
                    }
                }
            }
            continue;
        }

        IOContext* pIOContext = CONTAINING_RECORD(pOverlapped, IOContext, overlapped);
        ClientInfo* pClientInfo = (ClientInfo*)completionKey;

        switch (pIOContext->ioType) {
        case IOType::ACCEPT:
            HandleAccept(pIOContext);
            break;

        case IOType::RECV:
            if (pClientInfo) HandleRecv(pClientInfo, bytesTransferred);
            break;

        case IOType::SEND:
            if (pClientInfo) HandleSend(pClientInfo, bytesTransferred);
            break;
        }
    }
}

void IOCPServer::PostAccept() {
    SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (acceptSocket == INVALID_SOCKET) {
        std::cerr << std::format("AcceptSocket 생성 실패: {}\n", WSAGetLastError());
        return;
    }

    std::shared_ptr<IOContext> acceptContext = std::make_shared<IOContext>(IOType::ACCEPT);
    acceptContext->socket = acceptSocket;

    DWORD bytesReceived = 0;
    int addrLen = sizeof(SOCKADDR_IN) + 16;

    BOOL result = lpfnAcceptEx(
        listenSocket,
        acceptSocket,
        acceptContext->buffer,
        0,
        addrLen,
        addrLen,
        &bytesReceived,
        &acceptContext->overlapped
    );

    if (result == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
        std::cerr << std::format("AcceptEx 실패: {}\n", WSAGetLastError());
        closesocket(acceptSocket);
        return;
    }
}

void IOCPServer::HandleAccept(IOContext* pIOContext) {
    SOCKET clientSocket = pIOContext->socket;

    ClientInfo* pClientInfo = new ClientInfo();
    pClientInfo->socket = clientSocket;
    pIOContext->socket = INVALID_SOCKET;

    if (setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
        (char*)&listenSocket, sizeof(listenSocket)) == SOCKET_ERROR) {
        std::cerr << std::format("setsockopt(SO_UPDATE_ACCEPT_CONTEXT) 실패: {}\n", WSAGetLastError());
        delete pClientInfo;
        closesocket(clientSocket);
        PostAccept();
        return;
    }

    pClientInfo->recvContext = std::make_shared<IOContext>(IOType::RECV);
    pClientInfo->recvContext->socket = clientSocket;
    pClientInfo->sendContext = std::make_shared<IOContext>(IOType::SEND);
    pClientInfo->sendContext->socket = clientSocket;

    if (CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)pClientInfo, 0) == NULL) {
        std::cerr << std::format("클라이언트 소켓을 IOCP에 연결 실패: {}\n", GetLastError());
        delete pClientInfo;
        closesocket(clientSocket);
        PostAccept();
        return;
    }

    int clientAddrLen = sizeof(pClientInfo->clientAddr);
    getpeername(clientSocket, (SOCKADDR*)&pClientInfo->clientAddr, &clientAddrLen);

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(pClientInfo->clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
    unsigned short clientPort = ntohs(pClientInfo->clientAddr.sin_port);

    std::cout << std::format("새 클라이언트 연결: {}:{}\n", clientIP, clientPort);

    PostRecv(pClientInfo);
    PostAccept();
}

void IOCPServer::PostRecv(ClientInfo* pClientInfo) {  
   DWORD flags = 0;  
   DWORD bytesRecvd = 0;  

   char clientIP[INET_ADDRSTRLEN];  
   inet_ntop(AF_INET, &(pClientInfo->clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);  
   unsigned short clientPort = ntohs(pClientInfo->clientAddr.sin_port);  

   int result = WSARecv(  
       pClientInfo->socket,  
       &pClientInfo->recvContext->wsaBuf,  
       1,  
       &bytesRecvd,  
       &flags,  
       &pClientInfo->recvContext->overlapped,  
       NULL  
   );  

   if (result == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {  
       std::cerr << std::format("WSARecv 실패 ({}:{}): {}\n",  
           clientIP, clientPort, WSAGetLastError());  
       delete pClientInfo;  
   }  
}  

void IOCPServer::HandleRecv(ClientInfo* pClientInfo, DWORD bytesTransferred) {  
   if (bytesTransferred == 0) {  
       char clientIP[INET_ADDRSTRLEN];  
       inet_ntop(AF_INET, &(pClientInfo->clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);  
       unsigned short clientPort = ntohs(pClientInfo->clientAddr.sin_port);  

       std::cout << std::format("클라이언트 연결 정상 종료 (Recv 0 byte): {}:{}\n",  
           clientIP, clientPort);  
       delete pClientInfo;  
       return;  
   }  

   pClientInfo->recvContext->buffer[bytesTransferred] = '\0';  
   char clientIP[INET_ADDRSTRLEN];  
   inet_ntop(AF_INET, &(pClientInfo->clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);  
   unsigned short clientPort = ntohs(pClientInfo->clientAddr.sin_port);  

   std::cout << std::format("데이터 수신 ({}:{} {}바이트): {}\n",  
       clientIP, clientPort, bytesTransferred, pClientInfo->recvContext->buffer);  

   memcpy(pClientInfo->sendContext->buffer, pClientInfo->recvContext->buffer, bytesTransferred);  
   pClientInfo->sendContext->wsaBuf.len = bytesTransferred;  

   PostSend(pClientInfo, bytesTransferred);  
}  

void IOCPServer::PostSend(ClientInfo* pClientInfo, DWORD bytesToSend) {  
   DWORD flags = 0;  
   DWORD bytesSent = 0;  

   char clientIP[INET_ADDRSTRLEN];  
   inet_ntop(AF_INET, &(pClientInfo->clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);  
   unsigned short clientPort = ntohs(pClientInfo->clientAddr.sin_port);  

   int result = WSASend(  
       pClientInfo->socket,  
       &pClientInfo->sendContext->wsaBuf,  
       1,  
       &bytesSent,  
       flags,  
       &pClientInfo->sendContext->overlapped,  
       NULL  
   );  

   if (result == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {  
       std::cerr << std::format("WSASend 실패 ({}:{}): {}\n",  
           clientIP, clientPort, WSAGetLastError());  
       delete pClientInfo;  
   }  
}

void IOCPServer::HandleSend(ClientInfo* pClientInfo, DWORD bytesTransferred) {  
   char clientIP[INET_ADDRSTRLEN];  
   inet_ntop(AF_INET, &(pClientInfo->clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);  
   unsigned short clientPort = ntohs(pClientInfo->clientAddr.sin_port);  

   std::cout << std::format("데이터 전송 완료 ({}:{} {}바이트)\n",  
       clientIP, clientPort, bytesTransferred);  

   pClientInfo->recvContext->wsaBuf.len = BUFFER_SIZE;  
   ZeroMemory(pClientInfo->recvContext->buffer, BUFFER_SIZE);  

   PostRecv(pClientInfo);  
}