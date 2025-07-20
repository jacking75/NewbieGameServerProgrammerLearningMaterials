#pragma once

#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

namespace NetLib
{
	const int SPINLOCK_COUNT = 1000;

	struct CustomSpinLockCriticalSection
	{
		CRITICAL_SECTION m_CS;

		CustomSpinLockCriticalSection()
		{
			(void)InitializeCriticalSectionAndSpinCount(&m_CS, SPINLOCK_COUNT);
		}

		~CustomSpinLockCriticalSection()
		{
			DeleteCriticalSection(&m_CS);
		}
	};


	class SpinLockGuard
	{
	public:
		explicit SpinLockGuard(CustomSpinLockCriticalSection* pCS)
			: m_pSpinCS(&pCS->m_CS)
		{
			EnterCriticalSection(m_pSpinCS);
		}

		~SpinLockGuard()
		{
			LeaveCriticalSection(m_pSpinCS);
		}

	private:
		CRITICAL_SECTION* m_pSpinCS = nullptr;
	};
}