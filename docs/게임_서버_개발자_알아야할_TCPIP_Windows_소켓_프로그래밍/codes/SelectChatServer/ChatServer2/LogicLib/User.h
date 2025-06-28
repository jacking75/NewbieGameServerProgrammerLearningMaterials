#pragma once
#include <string>

namespace NLogicLib
{
	class User
	{
	public :
		enum class DOMAIN_STATE {
			NONE = 0,
			LOGIN = 1,
			LOBBY = 2,
			ROOM = 3,
		};

	public:
		User() {}
		virtual ~User() {}

		void Init(const short index)
		{
			m_Index = index;
		}

		void Clear()
		{			
			m_SessionIndex = 0;
			m_ID = "";
			m_IsConfirm = false;
			m_CurDomainState = DOMAIN_STATE::NONE;
			m_LobbyIndex = -1;
			m_RoomIndex = -1;
		}

		void Set(const int sessionIndex, const char* pszID)
		{
			m_IsConfirm = true;
			m_CurDomainState = DOMAIN_STATE::LOGIN;

			m_SessionIndex = sessionIndex;
			m_ID = pszID;

		}

		short GetIndex() { return m_Index; }

		int GetSessioIndex() { return m_SessionIndex;  }

		std::string& GetID() { return m_ID;  }

		bool IsConfirm() { return m_IsConfirm;  }

		short GetLobbyIndex() { return m_LobbyIndex; }

		void EnterLobby(const short lobbyIndex)
		{
			m_LobbyIndex = lobbyIndex;
			m_CurDomainState = DOMAIN_STATE::LOBBY;
		}
		
		void LeaveLobby()
		{
			m_CurDomainState = DOMAIN_STATE::LOGIN;
		}

		
		short GetRoomIndex() { return m_RoomIndex; }

		void EnterRoom(const short lobbyIndex, const short roomIndex)
		{
			m_LobbyIndex = lobbyIndex;
			m_RoomIndex = roomIndex;
			m_CurDomainState = DOMAIN_STATE::ROOM;
		}

		bool IsCurDomainInLogIn() {
			return m_CurDomainState == DOMAIN_STATE::LOGIN ? true : false;
		}
		
		bool IsCurDomainInLobby() {
			return m_CurDomainState == DOMAIN_STATE::LOBBY ? true : false;
		}

		bool IsCurDomainInRoom() {
			return m_CurDomainState == DOMAIN_STATE::ROOM ? true : false;
		}

		
	protected:
		short m_Index = -1;
		
		int m_SessionIndex = -1;

		std::string m_ID;
		
		bool m_IsConfirm = false;
		
		DOMAIN_STATE m_CurDomainState = DOMAIN_STATE::NONE;

		short m_LobbyIndex = -1;

		short m_RoomIndex = -1;
	};
}