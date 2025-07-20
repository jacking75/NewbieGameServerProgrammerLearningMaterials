#pragma once
#include <string>

#include "Packet.h"



namespace ChatServerLib
{
	class User
	{
	public:
		enum class DOMAIN_STATE 
		{
			NONE = 0,
			LOGIN = 1,
			ROOM = 2
		};


		User() = default;
		~User() = default;

		void Init(const INT32 index)
		{
			m_Index = index;
		}

		void Clear()
		{
			m_RoomIndex = -1;
			m_UserId = "";
			m_IsConfirm = false;
			m_CurDomainState = DOMAIN_STATE::NONE;
		}

		int SetLogin(char* login_id)
		{
			m_CurDomainState = DOMAIN_STATE::LOGIN;
			m_UserId = login_id;

			return 0;
		}
		
		void EnterRoom(INT32 roomIndex)
		{
			m_RoomIndex = roomIndex;
			m_CurDomainState = DOMAIN_STATE::ROOM;
		}
		
		void SetDomainState(DOMAIN_STATE val) { m_CurDomainState = val; }

		INT32 GetCurrentRoom() 
		{
			return m_RoomIndex;
		}

		INT32 GetNetConnIdx() 
		{
			return m_Index;
		}

		std::string GetUserId() const
		{
			return  m_UserId;
		}

		DOMAIN_STATE GetDomainState() 
		{
			return m_CurDomainState;
		}
				

	private:
		//TODO type을 window에서 정의된것으로 (INT16 INT32 INT64) 이 형식이 없으면 header에 window.h
		//이것은 cross 플랫폼이 아니라 windows에서만 사용하기에 쓰는 type. 만일 크로스플랫폼을 쓴다면 c++에서 정의된 타입을 씀

		INT32 m_RoomIndex = -1;

		std::string m_UserId;
		bool m_IsConfirm = false;
		std::string AuthToken;

		INT32 m_Index = -1;

		DOMAIN_STATE m_CurDomainState = DOMAIN_STATE::NONE;		
	};

}