# Description
- 원본 소스 출처: https://github.com/shawn246/redis_client
- Windows, Visual Studio 지원 추가

- Only two files included. One hpp file and one cpp file.
- Depend on hiredis.
- Support both single node mode and cluster mode.
- Support pipeline.
- Use connection pool.
- Thread safe.
- Reconnect automatically.
    
    
# TODO
- Optimize the reconnect function.
- Support pub/sub and transaction.
- Support scan in an unsafe way.
  
  
# License
This program was written by Shawn XU and is released under the BSD license.
  
  
# Usage
```c++
#include "RedisClient.hpp"

int main()
{
    CRedisClient redisCli;
    if (!redisCli.Initialize("127.0.0.1", 6379, 2, 10))
    {
        std::cout << "connect to redis failed" << std::endl;
        return -1;
    }

    std::string strKey = "key_1";
    std::string strVal;
    if (redisCli.Get(strKey, &strVal) == RC_SUCCESS)
    {
        std::cout << "key_1 has value " << strVal << std::endl;
        return 0;
    }
    else
    {
        std::cout << "request failed" << std::endl;
        return -1;
    }
}
```
  
You can view the test file if you want to know more using details about this client.



