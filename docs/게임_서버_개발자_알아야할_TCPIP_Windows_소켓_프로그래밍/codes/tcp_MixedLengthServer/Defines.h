#pragma once


#include <cstdint>

// 패킷 유형 정의
enum PacketType {
    PT_TRADE_REQUEST = 1,
    PT_TRADE_RESPONSE,
    PT_CHAT_MESSAGE,
    PT_PLAYER_MOVE
};

// 패킷 헤더 (고정 길이)
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t totalSize;     // 헤더 포함 전체 패킷 크기
    uint16_t packetType;    // 패킷 유형 (PacketType enum)
};

// 아이템 구조체 (고정 길이)
struct Item {
    uint32_t itemId;        // 아이템 고유 ID
    uint16_t quantity;      // 수량
    uint16_t category;      // 카테고리 (무기, 방어구 등)
};

// 거래 요청 패킷 (고정 길이 + 가변 길이)
struct TradeRequestPacket {
    uint32_t requesterId;   // 요청자 ID
    uint32_t targetId;      // 대상자 ID
    uint16_t itemCount;     // 아이템 수
    // 이후에 Item 배열과 추가 메시지(문자열)이 가변 길이로 옴
};
#pragma pack(pop)