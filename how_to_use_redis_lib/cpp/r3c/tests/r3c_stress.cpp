// Writed by yijian (eyjian@qq.com)
#include "r3c.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>

// 可用于测试异常时接口的正确性，
// 比如集群中节点挂掉，或节点挂起等异常情况

// argv[1] redis nodes
// argv[2] number of keys
// argv[3] number of cycles
int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <redis nodes> <number of keys> <number of cycles>\n", argv[0]);
        fprintf(stderr, "Example: %s 192.168.1.61:6379,192.168.1.62:6379 100 1000000\n", argv[0]);
        exit(1);
    }
    else
    {
        const int num_retries = r3c::NUM_RETRIES;
        const uint32_t expired_seconds = 3600*24;
        const std::string& redis_nodes = argv[1];
        const int num_keys = atoi(argv[2]);
        const int num_cycles = atoi(argv[3]);
        r3c::CRedisClient redis(redis_nodes);
        std::vector<std::string> keys(num_keys);

        try
        {
            for (std::vector<std::string>::size_type j=0; j<keys.size(); ++j)
            {
                char key_buf[64];
                snprintf(key_buf, sizeof(key_buf), "KEY_%zd", j);
                keys[j] = key_buf;
                redis.del(keys[j]);
            }
            for (int i=0; i<num_cycles; ++i)
            {
                if ((i > 0) && (0 == i % 10000))
                {
                    fprintf(stdout, "%d\n", i);
                }
                for (std::vector<std::string>::size_type j=0; j<keys.size(); ++j)
                {
                    const std::string& key = keys[j];
                    redis.incrby(key, 1, expired_seconds, NULL, num_retries);
                }
            }

            for (std::vector<std::string>::size_type j=0; j<keys.size(); ++j)
            {
                const std::string& key = keys[j];
                std::string value;
                if (redis.get(key, &value))
                {
                    fprintf(stdout, "%s => %s\n", key.c_str(), value.c_str());
                }
                else
                {
                    fprintf(stdout, "%s => %s\n", key.c_str(), "NONE");
                }
            }
            return 0;
        }
        catch (r3c::CRedisException& ex)
        {
            fprintf(stderr, "%s\n", ex.str().c_str());
            exit(1);
        }
    }
}
