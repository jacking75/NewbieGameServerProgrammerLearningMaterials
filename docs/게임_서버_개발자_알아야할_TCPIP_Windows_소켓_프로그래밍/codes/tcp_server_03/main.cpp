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


int main() 
{
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