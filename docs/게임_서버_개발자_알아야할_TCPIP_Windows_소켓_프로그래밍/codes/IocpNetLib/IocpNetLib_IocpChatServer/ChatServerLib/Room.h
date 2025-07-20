#pragma once

#include <functional>
#include "UserManager.h"

namespace ChatServerLib
{
	class Room 
	{
	public:
		Room() = default;
		~Room() = default;

		void Init(const INT32 roomNum, const INT32 maxUserCount);

		INT32 GetMaxUserCount() { return m_MaxUserCount; }
		
		INT32 GetCurrentUserCount() { return m_CurrentUserCount; }
		
		INT32 GetRoomNumber() { return m_RoomNum; }

		UINT16 EnterUser(User* pUser);
		
		void LeaveUser(User* pUser);
						
		void NotifyChat(INT32 connIndex, const char* UserID, const char* Msg);
		
		
		std::function<void(INT32, void*, INT16)> SendPacketFunc;

	private:
		void SendToAllUser(const UINT16 dataSize, void* pData, const INT32 passUserindex, bool exceptMe);


		INT32 m_RoomNum = -1;

		std::list<User*> m_UserList;
		
		INT32 m_MaxUserCount = 0;

		UINT16 m_CurrentUserCount = 0;
	};
}