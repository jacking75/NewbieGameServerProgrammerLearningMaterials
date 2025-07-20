#include "Room.h"
#include "Packet.h"
#define MAX_USER_ID_LEN  20

namespace ChatServerLib
{
	void Room::Init(const INT32 roomNum, const INT32 maxUserCount)
	{
		m_RoomNum = roomNum;
		m_MaxUserCount = maxUserCount;
	}

	UINT16 Room::EnterUser(User* pUser) 
	{
		if (m_CurrentUserCount >= m_MaxUserCount)
		{
			return (UINT16)Common::ERROR_CODE::ENTER_ROOM_FULL_USER;
		}
				
		m_UserList.push_back(pUser);
		++m_CurrentUserCount;

		pUser->EnterRoom(m_RoomNum);
		return (UINT16)Common::ERROR_CODE::NONE;
	}


	void Room::LeaveUser(User* pLeaveUser) 
	{
		m_UserList.remove_if([leaveUserId = pLeaveUser->GetUserId()](User *pUser) {
									return leaveUserId == pUser->GetUserId();
								});
	}
			
	void Room::NotifyChat(INT32 connIndex, const char* UserID, const char* Msg) 
	{
		Common::ROOM_CHAT_NOTIFY_PACKET roomChatNtfyPkt;
		roomChatNtfyPkt.PacketId = (UINT16)Common::PACKET_ID::ROOM_CHAT_NOTIFY;
		roomChatNtfyPkt.PacketLength = sizeof(roomChatNtfyPkt);

		CopyMemory(roomChatNtfyPkt.Msg, Msg, sizeof(roomChatNtfyPkt.Msg));
		CopyMemory(roomChatNtfyPkt.UserID, UserID, sizeof(roomChatNtfyPkt.UserID));
		SendToAllUser(sizeof(roomChatNtfyPkt), &roomChatNtfyPkt, connIndex, false);
	}


	void Room::SendToAllUser(const UINT16 dataSize, void* pData, const INT32 passUserindex, bool exceptMe) 
	{

		for (auto pUser : m_UserList)
		{
			if (pUser == nullptr) {
				continue;
			}

			if (exceptMe && pUser->GetNetConnIdx() == passUserindex) {
				continue;
			}
						
			SendPacketFunc(pUser->GetNetConnIdx(), pData, dataSize);
		}
	}
}