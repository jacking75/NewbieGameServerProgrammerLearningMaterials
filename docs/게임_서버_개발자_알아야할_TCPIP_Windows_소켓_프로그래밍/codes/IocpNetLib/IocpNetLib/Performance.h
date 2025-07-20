#pragma once

#include <thread>



namespace NetLib
{
	class Performance
	{
	public:
		Performance() = default;
		
		~Performance()
		{
			if (m_IsCheckPerformance)
			{
				SetCheckPerformance(0);
				m_PerformanceThread.join();
			}
		}

	public:

		void Start(int milliseconds = 0)
		{
			m_CheckPerformanceMilliseconds = milliseconds;

			if (m_CheckPerformanceMilliseconds > 0)
			{
				SetCheckPerformance(TRUE);
				m_PerformanceThread = std::thread(&Performance::CheckPerformanceThread, this);
			}
		}

		void CheckPerformanceThread()
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(m_CheckPerformanceMilliseconds));
				if (m_IsCheckPerformance == false)
				{
					return;
				}

				char logmsg[64] = { 0, };
				sprintf_s(logmsg, "Process Packet Count:%d, Per:%d", m_PacketProcessCount, m_CheckPerformanceMilliseconds);
				LogFuncPtr((int)LogLevel::Debug, logmsg);

				ResetPacketProcessCount();
			}
		}

		int IncrementPacketProcessCount() { return m_IsCheckPerformance ? InterlockedIncrement(&m_PacketProcessCount) : 0; }
		
	private:
		void SetCheckPerformance(LONG isCheckPerformance) { InterlockedExchange(&m_IsCheckPerformance, isCheckPerformance); }
		void ResetPacketProcessCount() { InterlockedExchange(&m_PacketProcessCount, 0); }

	private:
		std::thread m_PerformanceThread;
		long m_PacketProcessCount = 0;
		LONG m_IsCheckPerformance = FALSE;
		int m_CheckPerformanceMilliseconds = 0;
	};
}