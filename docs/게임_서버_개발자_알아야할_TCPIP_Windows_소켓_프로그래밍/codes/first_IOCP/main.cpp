#include <iostream>
#include <string>
#include "IOCPServer.h"

int main() {
    IOCPServer server;

    std::string port = "8080";
    int workerThreadCount = 4;

    std::cout << "IOCP Echo 서버 시작\n";
    std::cout << "포트: " << port << "\n";
    std::cout << "워커 스레드 수: " << workerThreadCount << "\n\n";

    // 서버 초기화
    if (!server.Initialize(port, workerThreadCount)) {
        std::cerr << "서버 초기화 실패\n";
        return -1;
    }

    // 서버 시작
    server.Start();

    // 사용자 입력 대기 (서버 종료를 위해)
    std::cout << "서버 종료를 원하시면 'q'를 입력하고 Enter를 누르세요.\n";
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "q" || input == "Q") {
            break;
        }
        std::cout << "서버 종료를 원하시면 'q'를 입력하고 Enter를 누르세요.\n";
    }

    // 서버 중지
    std::cout << "서버를 종료합니다...\n";
    server.Stop();
    std::cout << "서버 종료 완료\n";

    return 0;
}