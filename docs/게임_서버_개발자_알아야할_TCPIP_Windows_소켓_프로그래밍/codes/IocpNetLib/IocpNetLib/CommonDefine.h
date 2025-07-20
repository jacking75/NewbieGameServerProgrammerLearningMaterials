#pragma once

#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

//#define SAFE_DELETE(x) if( x != nullptr ) { delete x; x = nullptr; } 
//#define SAFE_DELETE_ARR(x) if( x != nullptr ) { delete [] x; x = nullptr;}

namespace NetLib
{
	const INT32 INVALID_VALUE = -1;
	
	enum class LogLevel
	{
		Trace = 0,
		Debug,
		Info,
		Warn,
		Error,
		Fetal
	};


	enum class NetResult : INT16
	{
		Success = 0,

		Fail_Make_Directories_Log = 30,
		Fail_Make_Directories_Dump = 31,
		Fail_WorkThread_Info = 32,
		Fail_Server_Info_Port = 33,
		fail_server_info_max_recv_ovelapped_buffer_size = 34,
		fail_server_info_max_send_ovelapped_buffer_size = 35,
		fail_server_info_max_recv_connection_buffer_size = 36,
		fail_server_info_max_send_connection_buffer_size = 37,
		fail_server_info_max_packet_size = 38,
		fail_server_info_max_connection_count = 39,
		fail_server_info_max_message_pool_count = 40,
		fail_server_info_extra_message_pool_count = 41,
		fail_server_info_performance_packet_milliseconds_time = 42,
		fail_server_info_post_messages_threads_count = 43,
		fail_create_listensocket_startup = 45,
		fail_create_listensocket_socket = 46,
		fail_create_listensocket_bind = 47,
		fail_create_listensocket_listen = 48,
		fail_handleiocp_work = 49,
		fail_handleiocp_logic = 50,
		Fail_Create_Message_Manager = 51,
		Fail_Link_IOCP = 52,
		Fail_Create_Connection = 53,
		Fail_Create_Performance = 54,
		Fail_Create_WorkThread = 55,

		BindAcceptExSocket_fail_WSASocket = 56,
		BindAcceptExSocket_fail_AcceptEx = 57,

		fail_message_null = 58,
		fail_pqcs = 59,

		PostRecv_Null_Obj = 61,
		PostRecv_Null_WSABUF = 62,
		PostRecv_Null_Socket_Error = 63,

		ReservedSendPacketBuffer_Not_Connected = 64,
		ReservedSendPacketBuffer_Empty_Buffer = 65,

		FUNCTION_RESULT_END
	};
		
	


	class Helper
	{
		Helper() = default;
		~Helper() = default;

		INT32 SystemCoreCount()
		{
			SYSTEM_INFO systemInfo;
			GetSystemInfo(&systemInfo);
			return (INT32)systemInfo.dwNumberOfProcessors;
		}
	};

	inline void (*LogFuncPtr) (const int, const char* pszLogMsg);

}