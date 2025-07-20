#pragma once

#include <thread>

#include <DbgHelp.h> 

//사용법
/*
초기화
MiniDump::Begin();

if (MiniDump::CreateDirectories() == false)
{
	return FUNCTION_RESULT_FAIL_MAKE_DIRECTORIES_DUMP;
}

종료
MiniDump::End();
*/
namespace NetLib
{
	const WCHAR DUMP_CHECK_PATH[] = L"..\\Bin\\Dumps";
	const WCHAR DUMP_PATH[] = L"..\\Bin\\Dumps\\%d-%d-%d %d_%d_%d.dmp";
	const WCHAR DUMP_DLL_NAME[] = L"DBGHELP.DLL";
	const char DUMP_FUNCTION_NAME[] = "MiniDumpWriteDump";


	typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(
		HANDLE hProcess,
		DWORD dwPid,
		HANDLE hFile,
		MINIDUMP_TYPE DumpType,
		CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

	
	inline static DWORD WINAPI WriteDump(LPVOID pParam)
	{
		auto pExceptionInfo = reinterpret_cast<PEXCEPTION_POINTERS>(pParam);

		auto dllHandle = LoadLibrary(DUMP_DLL_NAME);
		if (dllHandle != nullptr)
		{
			MINIDUMPWRITEDUMP dump = (MINIDUMPWRITEDUMP)GetProcAddress(dllHandle, DUMP_FUNCTION_NAME);
			if (dump != nullptr)
			{
				WCHAR szDumpPath[MAX_PATH] = { 0, };
				SYSTEMTIME systemTime;
				GetLocalTime(&systemTime);

				swprintf_s(szDumpPath,
					MAX_PATH,
					DUMP_PATH,
					systemTime.wYear,
					systemTime.wMonth,
					systemTime.wDay,
					systemTime.wHour,
					systemTime.wMinute,
					systemTime.wSecond);

				auto hFileHandle = CreateFile(
					szDumpPath,
					GENERIC_WRITE,
					FILE_SHARE_WRITE,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

				if (hFileHandle != INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION miniDumpExceptionInfo;

					miniDumpExceptionInfo.ThreadId = GetCurrentThreadId();
					miniDumpExceptionInfo.ExceptionPointers = pExceptionInfo;
					miniDumpExceptionInfo.ClientPointers = NULL;

					auto isSuccess = dump(
						GetCurrentProcess(),
						GetCurrentProcessId(),
						hFileHandle,
						MiniDumpNormal,
						&miniDumpExceptionInfo,
						NULL,
						NULL);

					if (isSuccess == TRUE)
					{
						CloseHandle(hFileHandle);

						return EXCEPTION_EXECUTE_HANDLER;
					}
				}
				CloseHandle(hFileHandle);
			}
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}

	inline static LONG WINAPI UnHandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
	{
		if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		{
			std::thread overflowThread = std::thread(WriteDump, pExceptionInfo);
			overflowThread.join();
		}
		else
		{
			return WriteDump(pExceptionInfo);
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	class MiniDump
	{
		inline static LPTOP_LEVEL_EXCEPTION_FILTER g_previousExceptionFilter = nullptr;
		
	public:
		static void Begin()
		{
			SetErrorMode(SEM_FAILCRITICALERRORS);
			g_previousExceptionFilter = SetUnhandledExceptionFilter(UnHandledExceptionFilter);
		}
		static void End()
		{
			SetUnhandledExceptionFilter(g_previousExceptionFilter);
		}

		static bool CreateDirectories()
		{
			auto result = _waccess_s(DUMP_CHECK_PATH, 0);
			if (result != 0)
			{
				auto isSuccess = CreateDirectoryW(DUMP_CHECK_PATH, nullptr);
				if (isSuccess == FALSE)
				{
					return false;
				}
			}

			return true;
		}
	};


}