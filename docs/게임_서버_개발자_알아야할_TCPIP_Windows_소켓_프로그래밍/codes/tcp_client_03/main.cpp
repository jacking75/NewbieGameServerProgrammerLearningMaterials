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


int main() 
{
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