#pragma once

#include <list>

#include <concurrent_queue.h>

#include "CommonDefine.h"

namespace NetLib
{
	//TODO 크기별 풀로 나누기
	class MessagePool
	{
	public:
		explicit MessagePool(const int maxMsgPoolCount, const int extraMsgPoolCount)
		{
			m_MaxMessagePoolCount = maxMsgPoolCount;
			m_ExtraMessagePoolCount = extraMsgPoolCount;

			CreateMessagePool();
		}
		
		
		~MessagePool()
		{
			DestroyMessagePool();
		}

	public:
		bool CheckCreate()
		{
			if (m_MaxMessagePoolCount == INVALID_VALUE)
			{
				LogFuncPtr((int)LogLevel::Fetal, "Fail MessagePool::CheckCreate");
				return false;
			}

			if (m_ExtraMessagePoolCount == INVALID_VALUE)
			{
				LogFuncPtr((int)LogLevel::Fetal, "Fail MessagePool::CheckCreate | m_ExtraMessagePoolCount == INVALID_VALUE");
				return false;
			}

			return true;
		}
		
		bool DeallocMsg(Message* pMsg)
		{
			if (pMsg == nullptr)
			{
				return false;
			}

			pMsg->Clear();

			m_MessagePool.push(pMsg);
			return true;
		}

		Message* AllocMsg()
		{
			Message* pMsg = nullptr;
			if (!m_MessagePool.try_pop(pMsg))
			{
				return nullptr;
			}

			return pMsg;
		}



	private:
		bool CreateMessagePool()
		{
			Message* pMsg = nullptr;
			for (int i = 0; i < m_MaxMessagePoolCount; ++i)
			{
				pMsg = new Message;
				pMsg->Clear();

				m_MessagePool.push(pMsg);
			}

			for (int i = 0; i < m_ExtraMessagePoolCount; ++i)
			{
				pMsg = new Message;
				pMsg->Clear();

				m_MessagePool.push(pMsg);
			}

			return true;
		}

		void DestroyMessagePool()
		{
			while (auto pData = AllocMsg() )
			{
				if (pData == nullptr)
				{
					break;
				}

				delete pData;
			}
		}

	private:
		concurrency::concurrent_queue<Message*> m_MessagePool;
		
		int m_MaxMessagePoolCount = INVALID_VALUE;
		int m_ExtraMessagePoolCount = INVALID_VALUE;
	};
}