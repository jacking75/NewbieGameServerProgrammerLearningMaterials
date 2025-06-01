#pragma once

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <format>
#include <vector>
#include <thread>
#include <string>
#include <map>

#include "Defines.h"

#pragma comment(lib, "ws2_32.lib")


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
                }
                else {
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
                    }
                    else {
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
        }
        else {
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