#include "Defines.h"
#include "MixedLengthServer.h"



int main() 
{
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
        
    MixedLengthServer server;
    if (server.Start()) {
        std::cout << "서버가 시작되었습니다. 종료하려면 아무 키나 누르세요.\n";
        std::cin.ignore();
        std::cin.get();
        server.Stop();
    }
    

    return 0;
}
