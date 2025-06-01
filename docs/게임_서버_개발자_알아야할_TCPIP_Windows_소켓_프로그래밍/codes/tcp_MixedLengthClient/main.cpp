#include "Defines.h"
#include "MixedLengthClient.h"


int main() {
    // 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);

    
    MixedLengthClient client;
    std::string serverIP;
    uint32_t userId, targetId;

    std::cout << "서버 IP를 입력하세요: ";
    std::cin.ignore();
    std::getline(std::cin, serverIP);

    std::cout << "당신의 사용자 ID를 입력하세요: ";
    std::cin >> userId;

    if (client.Connect(serverIP, 27015, userId)) {
        while (true) {
            std::cout << "\n1: 거래 요청 보내기, 2: 종료 - 선택: ";
            int choice;
            std::cin >> choice;

            if (choice == 2) {
                break;
            }
            else if (choice == 1) {
                std::cout << "거래할 대상 ID를 입력하세요: ";
                std::cin >> targetId;

                // 아이템 목록 생성
                std::vector<Item> items;
                int itemCount;
                std::cout << "보낼 아이템 수를 입력하세요: ";
                std::cin >> itemCount;

                for (int i = 0; i < itemCount; i++) {
                    Item item;
                    std::cout << std::format("아이템 #{}:\n", i + 1);
                    std::cout << "아이템 ID: ";
                    std::cin >> item.itemId;
                    std::cout << "수량: ";
                    std::cin >> item.quantity;
                    std::cout << "카테고리(0-무기, 1-방어구, 2-소모품): ";
                    std::cin >> item.category;

                    items.push_back(item);
                }

                std::string message;
                std::cout << "거래 메시지를 입력하세요: ";
                std::cin.ignore();
                std::getline(std::cin, message);

                client.SendTradeRequest(targetId, items, message);
            }
        }

        client.Disconnect();
        
    }

    return 0;
}
