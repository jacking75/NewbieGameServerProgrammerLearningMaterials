// Connect.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include "../../src/CRedisConn.h"

int main()
{
	RedisCpp::CRedisConn con;
	
	std::string value;
	uint32_t retval = 0;

	if (!con.connect("127.0.0.1", 6379))
	{
		std::cout << "connect error " << con.getErrorStr() << std::endl;
		return -1;
	}
	else
	{
		std::cout << "connect success !!!" << std::endl;
	}


	if (con.get("test02", value))
	{
		printf("test02 - %s", value.c_str());
		con.del("test02", retval);
	}
	else
	{
		std::cout << "test02 - ???" << std::endl;		
		con.set("test02", "llll", retval);
	}

	return 0;
}

