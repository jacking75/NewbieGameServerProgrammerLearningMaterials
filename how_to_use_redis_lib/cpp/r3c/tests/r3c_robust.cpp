// Writed by yijian (eyjian@qq.com)
#include "r3c.h"
#include "utils.h"
#include <libgen.h> // basename
#include <stdlib.h> // atoi
#include <string.h> // strdup
#include <sys/sysinfo.h>
#if __cplusplus >= 201103L
#include <thread>
#endif // __cplusplus >= 201103L

typedef void (*F)(r3c::CRedisClient& redis);
static void fill_f(F f[]);

int main(int argc, char* argv[])
{
    try
    {
        F f[2019];
        char* s = strdup(argv[0]);
        char* b = basename(s);

        r3c::set_error_log_write(r3c::r3c_log_write);
        for (size_t i=0; i<sizeof(f)/sizeof(f[0]); ++i)
            f[i] = NULL;
        fill_f(f);
        if (argc != 3)
        {
            fprintf(stderr, "Usage: %s index redis_nodes, example: %s 1 127.0.0.1:6379\n", b, b);
            free(s);
            exit(1);
        }
        else
        {
            const int n = atoi(argv[1]);
            const char* redis_nodes = argv[2];

            if (n<0 || n>=static_cast<int>(sizeof(f)/sizeof(f[0])))
            {
                fprintf(stderr, "Usage: %s index redis_nodes, example: %s 1 127.0.0.1:6379\n", b, b);
                free(s);
                exit(1);
            }
            else
            {
                free(s);
                if (NULL == f[n])
                {
                    fprintf(stderr, "Not implemented\n");
                }
                else
                {
                    r3c::CRedisClient redis(redis_nodes);
                    (*(f[n]))(redis);
                }
                return 0;
            }
        }
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, "%s\n", ex.str().c_str());
        exit(1);
    }
}

// 可用来观察：
// 1) 运行中，master异常
// 2) 运行中，replica异常
// 3) 运行中，master和replica都异常
void f0(r3c::CRedisClient& redis)
{
    const std::string k1 = "K1";
    const std::string k2 = "{K1}K2";
    std::string v;
    r3c::Node which;

    // k1
    redis.setex(k1, "11", 1800);
    redis.setex(k2, "22", 1800);
    for (int i=0; i<600; ++i)
    {
        try
        {
            // k1
            redis.get(k1, &v, &which);
            fprintf(stdout, "[%s][slot://%d][key://%s][redis://%s] %s => %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), r3c::get_key_slot(&k1),
                    k1.c_str(), r3c::node2string(which).c_str(), k1.c_str(), v.c_str());

            // k2
            redis.get(k2, &v, &which);
            fprintf(stdout, "[%s][slot://%d][key://%s][redis://%s] %s => %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), r3c::get_key_slot(&k2),
                    k2.c_str(), r3c::node2string(which).c_str(), k2.c_str(), v.c_str());
        }
        catch (r3c::CRedisException& ex)
        {
            fprintf(stderr, "[%s] `GET` failed: %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), ex.str().c_str());
        }

        r3c::millisleep(1000);
    }
}

// 观察eval在异常时的情况
void f1(r3c::CRedisClient& redis)
{
    const std::string lua_scripts = "local v=redis.call('get',KEYS[1]);return v;";
    const std::string k1 = "K1";
    const std::string k2 = "{K1}K2";
    r3c::Node which;

    // k1
    redis.setex(k1, "11", 1800);
    redis.setex(k2, "22", 1800);
    for (int i=0; i<1800; ++i)
    {
        try
        {
            // k1
            redis.eval(k1, lua_scripts, &which);
            fprintf(stdout, "[%s][slot://%d][key://%s][redis://%s] %s => %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), r3c::get_key_slot(&k1),
                    k1.c_str(), r3c::node2string(which).c_str(), k1.c_str(), lua_scripts.c_str());

            // k2
            redis.eval(k2, lua_scripts, &which);
            fprintf(stdout, "[%s][slot://%d][key://%s][redis://%s] %s => %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), r3c::get_key_slot(&k2),
                    k2.c_str(), r3c::node2string(which).c_str(), k2.c_str(), lua_scripts.c_str());
        }
        catch (r3c::CRedisException& ex)
        {
            fprintf(stderr, "[%s] `EVAL` failed: %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), ex.str().c_str());
        }

        r3c::millisleep(1000);
    }
}

// 可用来观察整个集群挂掉后，请求是否倾斜
void f2(r3c::CRedisClient& redis)
{
    const std::string lua_scripts = "local v=redis.call('get',KEYS[1]);return v;";
    r3c::Node which;

    for (int i=0; i<600; ++i)
    {
        // k1
        const std::string k1 = "K1";
        try
        {
            redis.eval(k1, lua_scripts, &which);
            fprintf(stdout, "[%s][slot://%d][key://%s][redis://%s] %s => %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), r3c::get_key_slot(&k1),
                    k1.c_str(), r3c::node2string(which).c_str(), k1.c_str(), lua_scripts.c_str());
        }
        catch (r3c::CRedisException& ex)
        {
            fprintf(stderr, "[%s][key:%s] `EVAL` failed: %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), k1.c_str(), ex.str().c_str());
        }

        // k2
        const std::string k2 = "K2";
        try
        {
            redis.eval(k2, lua_scripts, &which);
            fprintf(stdout, "[%s][slot://%d][key://%s][redis://%s] %s => %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), r3c::get_key_slot(&k2),
                    k2.c_str(), r3c::node2string(which).c_str(), k2.c_str(), lua_scripts.c_str());
        }
        catch (r3c::CRedisException& ex)
        {
            fprintf(stderr, "[%s][key:%s] `EVAL` failed: %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), k2.c_str(), ex.str().c_str());
        }

        r3c::millisleep(1000);
    }
}

void f3(r3c::CRedisClient& redis)
{
    const std::string lua_scripts = "local v=redis.call('get','K2');return v;";
    r3c::Node which;

    // k1
    for (int i=0; i<600; ++i)
    {
        try
        {
            const std::string k1 = "K1";
            redis.eval(k1, lua_scripts, &which);
            fprintf(stdout, "[%s][slot://%d][key://%s][redis://%s] %s => %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), r3c::get_key_slot(&k1),
                    k1.c_str(), r3c::node2string(which).c_str(), k1.c_str(), lua_scripts.c_str());
        }
        catch (r3c::CRedisException& ex)
        {
            fprintf(stderr, "[%s] `EVAL` failed: %s\n",
                    r3c::get_formatted_current_datetime(true).c_str(), ex.str().c_str());
        }

        r3c::millisleep(1000);
    }
}

void fill_f(F f[])
{
    f[0] = f0;
    f[1] = f1;
    f[2] = f2;
    f[3] = f3;
}
