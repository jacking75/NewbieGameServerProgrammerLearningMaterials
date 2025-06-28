#pragma once
#include <unordered_map>
#include <deque>
#include <string>
#include <vector>

namespace NCommon
{
	enum class ERROR_CODE :short;
}
using ERROR_CODE = NCommon::ERROR_CODE;

namespace NLogicLib
{	
	class User;

	class UserManager
	{
	public:
		UserManager();
		virtual ~UserManager();

		void Init(const int maxUserCount);

		ERROR_CODE AddUser(const int sessionIndex, const char* pszID);
		ERROR_CODE RemoveUser(const int sessionIndex);

		std::tuple<ERROR_CODE,User*> GetUser(const int sessionIndex);

				
	private:
		User* AllocUserObjPoolIndex();
		void ReleaseUserObjPoolIndex(const int index);

		User* FindUser(const int sessionIndex);
		User* FindUser(const char* pszID);
				
	private:
		std::vector<User> m_UserObjPool;
		std::deque<int> m_UserObjPoolIndex;

		std::unordered_map<int, User*> m_UserSessionDic;
		std::unordered_map<const char*, User*> m_UserIDDic; //char*�� key�� ������

	};
}