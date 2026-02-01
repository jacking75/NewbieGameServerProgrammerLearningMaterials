// Writed by yijian (eyjian@qq.com)
#include "r3c.h"
#include "utils.h"
#include <inttypes.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static void my_log_write(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: r3c_cmd command parameter ...\n");
        exit(1);
    }

	r3c::set_info_log_write(my_log_write);
	r3c::set_debug_log_write(my_log_write);

    try
    {
        const char* cmd = argv[1];
        const char* key = argv[2];
        const char* nodes = getenv("H");
        if (NULL == nodes)
        {
            fprintf(stderr, "Environment[H] not set, example: export H=127.0.0.1:6379,127.0.0.1:6380,127.0.0.1:6381\n");
            exit(1);
        }
        if ('\0' == *nodes)
        {
            fprintf(stderr, "Environment[H] without value, example: export H=127.0.0.1:6379,127.0.0.1:6380,127.0.0.1:6381\n");
            exit(1);
        }
        const char* colon = strchr(nodes, ':');
        if (NULL == colon)
        {
            fprintf(stderr, "Environment[H] with error value, example: export H=127.0.0.1:6379,127.0.0.1:6380,127.0.0.1:6381\n");
            exit(1);
        }

        int i = 0, j=0;
        int ret = 0;
        int offset = 0;
        int count = 0;
        int end = 0;
        int start = 0;
        int64_t ret64 = 0;
        int64_t cursor = 0;
        int64_t min;
        int64_t max;
        int64_t increment;
        int64_t new_value_int64;
        int64_t result_int64;
        uint32_t seconds;
        std::string str;
        std::string value;
        std::vector<std::string> fields;
        std::vector<std::string> values;
        std::vector<std::pair<std::string, int64_t> > values2;
        std::map<std::string, std::string> map;
        std::map<std::string, std::string>::iterator iter;
        std::vector<std::string> vec;
        std::pair<std::string, uint16_t> which_node;
        std::string password = (getenv("PASSWORD")!=NULL)? getenv("PASSWORD"): "";
        r3c::CRedisClient redis_client(nodes, password);

        if (0 == strcasecmp(cmd, "sha1"))
        {
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd sha1 string\n");
                exit(1);
            }

            str =  key;
            fprintf(stdout, "%s\n", r3c::strsha1(str).c_str());
        }
        else if (0 == strcasecmp(cmd, "slot"))
        {
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd slot key\n");
                exit(1);
            }

            str =  key;
            unsigned int slot = r3c::get_key_slot(&str);
            fprintf(stdout, "[%s] => %u\n", key, slot);
        }
        else if (0 == strcasecmp(cmd, "list"))
        {
            std::vector<struct r3c::NodeInfo> nodes_info;
            int num_nodes = redis_client.list_nodes(&nodes_info);

            fprintf(stdout, "number of nodes (from %s:%d): %d\n", which_node.first.c_str(), which_node.second, num_nodes);
            for (std::vector<struct r3c::NodeInfo>::size_type i=0; i<nodes_info.size(); ++i)
                std::cout << "[" << i << "] " << nodes_info[i] << std::endl;
        }
        else if (0 == strcasecmp(cmd, "flushall"))
        {
            fprintf(stdout, "[" PRINT_COLOR_YELLOW"NOTICE" PRINT_COLOR_NONE"] To clear only a node, set `HOSTS` to a single node\n\n");
            redis_client.flushall();
        }
        ////////////////////////////////////////////////////////////////////////////
        // KEY VALUE
        else if (0 == strcasecmp(cmd, "type"))
        {
            // TYPE
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd type key\n");
                exit(1);
            }

            std::string key_type;
            if (!redis_client.key_type(key, &key_type, &which_node))
                fprintf(stderr, "key[%s] not exists\n", key);
            else
                fprintf(stdout, "key[%s]: %s\n", key, key_type.c_str());
        }
        else if (0 == strcasecmp(cmd, "del"))
        {
            // DEL
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd del key\n");
                exit(1);
            }

            if (!redis_client.del(key, &which_node))
                fprintf(stderr, "[%s] not exist\n", key);
            else
                fprintf(stdout, "[%s] deleted\n", key);
        }
        else if (0 == strcasecmp(cmd, "exists"))
        {
            // EXISTS command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd exists key\n");
                exit(1);
            }

            if (redis_client.exists(key, &which_node))
                fprintf(stdout, "[%s] exist\n", key);
            else
                fprintf(stderr, "[%s] not exist\n", key);
        }
        else if (0 == strcasecmp(cmd, "expire"))
        {
            // EXPIRE command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd expire key seconds\n");
                exit(1);
            }

            seconds = atoi(argv[3]);
            if (redis_client.expire(key, seconds, &which_node))
                fprintf(stdout, "[%s] exist\n", key);
            else
                fprintf(stderr, "[%s] not exist\n", key);
        }
        else if ((0 == strcasecmp(cmd, "eval")) || (0 == strcasecmp(cmd, "evalsha")))
        {
            // EVAL/EVALSHA command
            if (argc < 4)
            {
                if (0 == strcasecmp(cmd, "eval"))
                {
                    fprintf(stderr, "Usage1: r3c_cmd EVAL key lua_scripts\n");
                    fprintf(stderr, "Usage1: r3c_cmd EVAL key lua_scripts parameter1 parameter2 ...\n");
                }
                else
                {
                    fprintf(stderr, "Usage1: r3c_cmd EVALSHA key sha1\n");
                    fprintf(stderr, "Usage1: r3c_cmd EVALSHA key sha1 parameter1 parameter2 ...\n");
                }

                exit(1);
            }

            if (4 == argc)
            {
                const char* lua_scripts_or_sha1 = argv[3];
                if (0 == strcasecmp(cmd, "eval"))
                {
                    std::vector<std::string> parameters;
                    for (int i=4; i<argc; ++i)
                        parameters.push_back(argv[i]);

                    // r3c_cmd eval r3c_k1 "local v=redis.call('get','r3c_k1');return v"
                    // r3c_cmd eval r3c_k1 "local v=redis.call('set','r3c_k1','123');return v"
                    const r3c::RedisReplyHelper reply = redis_client.eval(key, lua_scripts_or_sha1, parameters, &which_node);
                    if (reply)
                        std::cout << reply.get();
                }
            }
            else
            {
                const char* lua_scripts_or_sha1 = argv[3];
                std::vector<std::string> parameters(argc-4);

                for (int i=4; i<argc; ++i)
                    parameters[i-4] = argv[i];

                if (0 == strcasecmp(cmd, "eval"))
                {
                    const r3c::RedisReplyHelper reply = redis_client.eval(key, lua_scripts_or_sha1, parameters, &which_node);
                    if (reply)
                        std::cout << reply;
                }
                else
                {
                    const r3c::RedisReplyHelper reply = redis_client.evalsha(key, lua_scripts_or_sha1, parameters, &which_node);
                    if (reply)
                        std::cout << reply;
                }
            }
        }
        else if (0 == strcasecmp(cmd, "ttl"))
        {
            // TTL command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd ttl key\n");
                exit(1);
            }

            int ttl = redis_client.ttl(key, &which_node);
            if (ttl >= 0)
                fprintf(stdout, "[%s] %d\n", key, ttl);
            else if (-1 == ttl)
                fprintf(stderr, "[%s] no associated expire\n", key);
            else if (-2 == ttl)
                fprintf(stderr, "[%s] not exist\n", key);
            else
                fprintf(stderr, "unknown error\n");
        }
        else if (0 == strcasecmp(cmd, "set"))
        {
            // SET command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd set key value\n");
                exit(1);
            }

            redis_client.set(key, argv[3], &which_node);
        }
        else if (0 == strcasecmp(cmd, "setnx"))
        {
            // SETNX command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd setnx key value\n");
                exit(1);
            }

            if (redis_client.setnx(key, argv[3], &which_node))
                fprintf(stdout, "[%s] ok\n", key);
            else
                fprintf(stderr, "[%s] exists\n", key);
        }
        else if (0 == strcasecmp(cmd, "setex"))
        {
            // SETEX command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd setex key seconds value\n");
                exit(1);
            }

            seconds = atoi(argv[3]);
            redis_client.setex(key, argv[4], seconds, &which_node);
            fprintf(stdout, "[%s] ok\n", key);
        }
        else if (0 == strcasecmp(cmd, "setnxex"))
        {
            // SETNXEX command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd setnxex key seconds value\n");
                exit(1);
            }

            seconds = atoi(argv[3]);
            if (redis_client.setnxex(key, argv[4], seconds, &which_node))
                fprintf(stdout, "[%s] ok\n", key);
            else
                fprintf(stdout, "[%s] exists\n", key);
        }
        else if (0 == strcasecmp(cmd, "get"))
        {
            // GET command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd get key\n");
                exit(1);
            }

            if (!redis_client.get(key, &value, &which_node))
            {
                fprintf(stderr, "[%s] not exist\n", key);
            }
            else
            {
                fprintf(stdout, "[%s] => %s\n", key, value.c_str());
            }
        }
        else if (0 == strcasecmp(cmd, "incrby"))
        {
            // INCRBY command
            if ((argc != 4) && (argc != 5))
            {
                fprintf(stderr, "Usage1: r3c_cmd incrby key increment\n");
                fprintf(stderr, "Usage2: r3c_cmd incrby key increment timeout_seconds\n");
                exit(1);
            }

            increment = atoll(argv[3]);
            if (4 == argc)
            {
                new_value_int64 = redis_client.incrby(key, increment, &which_node);
            }
            else
            {
                seconds = static_cast<uint32_t>(atoll(argv[4]));
                new_value_int64 = redis_client.incrby(key, increment, increment, seconds, &which_node);
            }
            fprintf(stdout, "%" PRId64"\n", new_value_int64);
        }
        else if (0 == strcasecmp(cmd, "scan"))
        {
            // SCAN command
            if ((argc < 3) || (argc > 5))
            {
                fprintf(stderr, "Usage1: r3c_cmd scan cursor\n");
                fprintf(stderr, "Usage2: r3c_cmd scan cursor count\n");
                fprintf(stderr, "Usage3: r3c_cmd scan cursor pattern\n");
                fprintf(stderr, "Usage4: r3c_cmd scan cursor pattern count\n");
                exit(1);
            }

            cursor = static_cast<int64_t>(atoll(argv[2]));
            if (3 == argc)
            {
                cursor = redis_client.scan(cursor, &values, &which_node);
            }
            else if (4 == argc)
            {
                count = atoi(argv[3]);
                if (count > 0)
                    cursor = redis_client.scan(cursor, count, &values, &which_node);
                else
                    cursor = redis_client.scan(cursor, argv[3], &values, &which_node);
            }
            else if (5 == argc)
            {
                cursor = redis_client.scan(cursor, argv[2], atoi(argv[3]), &values, &which_node);
            }

            fprintf(stdout, "cursor: %" PRId64", count: %d\n", cursor, static_cast<int>(values.size()));
            for (i=0; i<static_cast<int>(values.size()); ++i)
                fprintf(stdout, "%s\n", values[i].c_str());
        }
        ////////////////////////////////////////////////////////////////////////////
        // LIST
        else if (0 == strcasecmp(cmd, "llen"))
        {
            // LLEN command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd llen key\n");
                exit(1);
            }

            result_int64 = redis_client.llen(key, &which_node);
            fprintf(stdout, "[%s] => %" PRId64"\n", key, result_int64);
        }
        else if (0 == strcasecmp(cmd, "lpop"))
        {
            // LPOP command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd lpop key\n");
                exit(1);
            }

            if (redis_client.lpop(key, &value, &which_node))
                fprintf(stdout, "[%s] => %s\n", key, value.c_str());
            else
                fprintf(stdout, "empty\n");
        }
        else if (0 == strcasecmp(cmd, "lpush"))
        {
            // LPUSH command
            if (argc < 4)
            {
                fprintf(stderr, "Usage: r3c_cmd lpush key value1 value2 ...\n");
                exit(1);
            }

            values.clear();
            for (i=3; i<argc; ++i)
                values.push_back(argv[i]);
            count = redis_client.lpush(key, values, &which_node);
            fprintf(stdout, "[%s] => %d\n", key, count);
        }
        else if (0 == strcasecmp(cmd, "lrange"))
        {
            // LRANGE command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd lrange key start end\n");
                exit(1);
            }

            end = atoi(argv[4]);
            start = atoi(argv[3]);
            count = redis_client.lrange(key, start, end, &values, &which_node);
            fprintf(stdout, "count: %d\n", count);
            for (i=0; i<count; ++i)
                fprintf(stdout, "%s\n", values[i].c_str());
        }
        else if (0 == strcasecmp(cmd, "ltrim"))
        {
            // LTRIM command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd ltrim key start end\n");
                exit(1);
            }

            end = atoi(argv[4]);
            start = atoi(argv[3]);
            redis_client.ltrim(key, start, end, &which_node);
        }
        else if (0 == strcasecmp(cmd, "rpop"))
        {
            // RPOP command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd rpop key\n");
                exit(1);
            }

            if (redis_client.rpop(key, &value, &which_node))
                fprintf(stdout, "%s\n", value.c_str());
            else
                fprintf(stderr, "empty\n");
        }
        else if (0 == strcasecmp(cmd, "rpush"))
        {
            // RPUSH command
            if (argc < 4)
            {
                fprintf(stderr, "Usage: r3c_cmd rpush key value1 value2 ...\n");
                exit(1);
            }

            if (4 == argc)
            {
                count = redis_client.rpush(key, argv[3], &which_node);
            }
            else
            {
                values.clear();
                for (i=3; i<argc; ++i)
                    values.push_back(argv[i]);
                count = redis_client.rpush(key, values, &which_node);
            }
            fprintf(stdout, "[%s] => %d\n", key, count);
        }
        else if (0 == strcasecmp(cmd, "rpushx"))
        {
            // RPUSHX command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd rpushx key value\n");
                exit(1);
            }

            count = redis_client.rpushx(key, argv[3], &which_node);
            fprintf(stdout, "[%s] => %d\n", key, count);
        }
        ////////////////////////////////////////////////////////////////////////////
        // HASH
        else if (0 == strcasecmp(cmd, "hdel"))
        {
            // HDEL command
            if (argc < 4)
            {
                fprintf(stderr, "Usage: r3c_cmd hdel key field1 field2 ...\n");
                exit(1);
            }

            if (4 == argc)
            {
                if (redis_client.hdel(key, argv[3], &which_node))
                    fprintf(stdout, "[%s] deleted\n", key);
                else
                    fprintf(stderr, "[%s] not exists\n", key);
            }
            else
            {
                std::vector<std::string> fields;
                for (i=3; i<argc; ++i)
                    fields.push_back(argv[i]);
                count = redis_client.hmdel(key, fields, &which_node);
                if (count > 0)
                    fprintf(stdout, "[%s] deleted: %d\n", key, count);
                else
                    fprintf(stderr, "[%s] not exists\n", key);
            }
        }
        else if (0 == strcasecmp(cmd, "hexists"))
        {
            // HEXISTS command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd hexists key field\n");
                exit(1);
            }

            if (redis_client.hexists(key, argv[3], &which_node))
                fprintf(stdout, "[%s:%s] exist\n", key, argv[3]);
            else
                fprintf(stderr, "[%s:%s] not exist\n", key, argv[3]);
        }
        else if (0 == strcasecmp(cmd, "hlen"))
        {
            // HLEN command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd hlen key\n");
                exit(1);
            }

            new_value_int64 = redis_client.hlen(key, &which_node);
            fprintf(stdout, "[%s] fields count: %" PRId64"\n", key, new_value_int64);
        }
        else if (0 == strcasecmp(cmd, "hset"))
        {
            // HSET command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd hset key field value\n");
                exit(1);
            }

            redis_client.hset(key, argv[3], argv[4], &which_node);
        }
        else if (0 == strcasecmp(cmd, "hsetex"))
        {
            // HSET command
            if (argc != 6)
            {
                fprintf(stderr, "Usage: r3c_cmd hsetex key field value timeout_seconds\n");
                exit(1);
            }

            seconds = static_cast<uint32_t>(atoll(argv[5]));
            redis_client.hsetex(key, argv[3], argv[4], seconds, &which_node);
        }
        else if (0 == strcasecmp(cmd, "hsetnx"))
        {
            // HSETNX command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd hsetnx key field value\n");
                exit(1);
            }

            if (redis_client.hsetnx(key, argv[3], argv[4], &which_node))
                fprintf(stdout, "[%s:%s] ok\n", key, argv[3]);
            else
                fprintf(stderr, "[%s:%s] exists\n", key, argv[3]);
        }
        else if (0 == strcasecmp(cmd, "hsetnxex"))
        {
            // HSETNX command
            if (argc != 6)
            {
                fprintf(stderr, "Usage: r3c_cmd hsetnxex key field value timeout_seconds\n");
                exit(1);
            }

            seconds = static_cast<uint32_t>(atoll(argv[5]));
            if (redis_client.hsetnxex(key, argv[3], argv[4], seconds, &which_node))
                fprintf(stdout, "[%s:%s] ok\n", key, argv[3]);
            else
                fprintf(stderr, "[%s:%s] exists\n", key, argv[3]);
        }
        else if (0 == strcasecmp(cmd, "hget"))
        {
            // HGET command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd hget key field\n");
                exit(1);
            }

            if (!redis_client.hget(key, argv[3], &value, &which_node))
            {
                fprintf(stdout, "[%s:%s] not exist\n", key, argv[3]);
            }
            else
            {
                fprintf(stdout, "[%s:%s] => %s\n", key, argv[3], value.c_str());
            }
        }
        else if (0 == strcasecmp(cmd, "hincrby"))
        {
            // HINCRBY command
            if ((argc < 5) || (argc%2 != 1))
            {
                fprintf(stderr, "Usage: r3c_cmd hincrby key field1 increment1 field2 increment2 ...\n");
                exit(1);
            }

            if (5 == argc)
            {
                increment = atoll(argv[4]);
                new_value_int64 = redis_client.hincrby(key, argv[3], increment, &which_node);
                fprintf(stdout, "%" PRId64"\n", new_value_int64);
            }
            else
            {
                std::vector<std::pair<std::string, int64_t> > increments((argc-3)/2);
                for (i=3,j=0; i<argc; i+=2,++j)
                {
                    increments[j].first = argv[i];
                    increments[j].second = static_cast<int64_t>(atoll(argv[i+1]));
                }

                std::vector<int64_t> values;
                redis_client.hmincrby(key, increments, &values, &which_node);
                for (std::vector<int64_t>::size_type k=0; k<values.size(); ++k)
                    fprintf(stdout, "%" PRId64"\n", values[k]);
            }
        }
        else if (0 == strcasecmp(cmd, "hmset"))
        {
            // HMSET command
            if ((argc < 5) || (argc % 2 != 1))
            {
                fprintf(stderr, "Usage: r3c_cmd hmset key field1 value1 field2 value2 ...\n");
                exit(1);
            }

            for (i=3; i<argc; i+=2)
                map[argv[i]] = argv[i+1];
            redis_client.hmset(key, map, &which_node);
        }
        else if (0 == strcasecmp(cmd, "hmget"))
        {
            // HMGET command
            if (argc < 4)
            {
                fprintf(stderr, "Usage: r3c_cmd hmget key field1 field2 ...\n");
                exit(1);
            }

            for (i=3; i<argc; ++i)
                vec.push_back(argv[i]);
            const int x = redis_client.hmget(key, vec, &map, false, &which_node);
            fprintf(stdout, "x: %d\n", x);
            if (map.empty())
                fprintf(stdout, "not exists\n");
            else
                for (iter=map.begin(); iter!=map.end(); ++iter)
                    fprintf(stdout, "%s => %s\n", iter->first.c_str(), iter->second.c_str());
        }
        else if (0 == strcasecmp(cmd, "hgetall"))
        {
            // HGETALL command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd hgetall key\n");
                exit(1);
            }

            count = redis_client.hgetall(key, &map, &which_node);
            if (0 == count)
            {
                fprintf(stderr, "[%s] not exists\n", key);
            }
            else
            {
                for (iter=map.begin(); iter!=map.end(); ++iter)
                    fprintf(stdout, "%s => %s\n", iter->first.c_str(), iter->second.c_str());
            }
        }
        else if (0 == strcasecmp(cmd, "hkeys"))
        {
            // HKEYS command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd hkeys key\n");
                exit(1);
            }

            count = redis_client.hkeys(key, &fields, &which_node);
            if (0 == count)
            {
                fprintf(stderr, "[%s] not exists\n", key);
            }
            else
            {
                for (std::vector<std::string>::size_type k=0; k<fields.size(); ++k)
                    fprintf(stdout, "%s\n", fields[k].c_str());
            }
        }
        else if (0 == strcasecmp(cmd, "hvals"))
        {
            // HVALS command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd hvals key\n");
                exit(1);
            }

            count = redis_client.hvals(key, &values, &which_node);
            if (0 == count)
            {
                fprintf(stderr, "[%s] not exists\n", key);
            }
            else
            {
                for (std::vector<std::string>::size_type k=0; k<values.size(); ++k)
                    fprintf(stdout, "%s\n", values[k].c_str());
            }
        }
        else if (0 == strcasecmp(cmd, "hscan"))
        {
            // HSCAN command
            if ((argc < 4) || (argc > 6))
            {
                fprintf(stderr, "Usage1: r3c_cmd hscan key cursor\n");
                fprintf(stderr, "Usage2: r3c_cmd hscan key cursor count\n");
                fprintf(stderr, "Usage3: r3c_cmd hscan key cursor pattern\n");
                fprintf(stderr, "Usage4: r3c_cmd hscan key cursor pattern count\n");
                exit(1);
            }

            if (4 == argc)
                cursor = redis_client.hscan(key, atoll(argv[3]), &map, &which_node);
            else if (6 == argc)
                cursor = redis_client.hscan(key, atoll(argv[3]), argv[4], atoi(argv[5]), &map, &which_node);
            else if (5 == argc)
            {
                count = atoi(argv[4]);
                if (count > 0)
                    cursor = redis_client.hscan(key, atoll(argv[3]), count, &map, &which_node);
                else
                    cursor = redis_client.hscan(key, atoll(argv[3]), argv[4], &map, &which_node);
            }

            fprintf(stdout, "cursor: %" PRId64", count: %d\n", cursor, static_cast<int>(map.size()));
            for (iter=map.begin(); iter!=map.end(); ++iter)
                fprintf(stdout, "%s => %s\n", iter->first.c_str(), iter->second.c_str());
        }
        ////////////////////////////////////////////////////////////////////////////
        // SET
        else if (0 == strcasecmp(cmd, "sadd"))
        {
            // SADD command
            if (argc < 4)
            {
                fprintf(stderr, "Usage: r3c_cmd sadd key value1 value2 ...\n");
                exit(1);
            }

            if (4 == argc)
            {
                count = redis_client.sadd(key, argv[3], &which_node);
            }
            else
            {
                for (i=3; i<argc; ++i)
                    values.push_back(argv[i]);
                count = redis_client.sadd(key, values, &which_node);
            }
            fprintf(stdout, "%d\n", count);
        }
        else if (0 == strcasecmp(cmd, "scard"))
        {
            // SCARD command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd scard key\n");
                exit(1);
            }

            count = redis_client.scard(key, &which_node);
            fprintf(stdout, "%d\n", count);
        }
        else if (0 == strcasecmp(cmd, "sismember"))
        {
            // SISMEMBER command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd sismember key member\n");
                exit(1);
            }

            if (redis_client.sismember(key, argv[3], &which_node))
                fprintf(stdout, "YES\n");
            else
                fprintf(stdout, "NO\n");
        }
        else if (0 == strcasecmp(cmd, "smembers"))
        {
            // SMEMBERS command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd smembers key\n");
                exit(1);
            }

            count = redis_client.smembers(key, &values, &which_node);
            for (i=0; i<count; ++i)
                fprintf(stdout, "%s\n", values[i].c_str());
        }
        else if (0 == strcasecmp(cmd, "spop"))
        {
            // SPOP command
            if ((argc != 3) && (argc != 4))
            {
                fprintf(stderr, "Usage1: r3c_cmd spop key\n");
                fprintf(stderr, "Usage2: r3c_cmd spop key count\n");
                exit(1);
            }

            if (3 == argc)
            {
                if (redis_client.spop(key, &value, &which_node))
                    fprintf(stdout, "%s\n", value.c_str());
                else
                    fprintf(stdout, "empty\n");
            }
            else
            {
                count = redis_client.spop(key, atoi(argv[3]), &values, &which_node);
                fprintf(stdout, "%d\n", count);
                for (i=0; i<count; ++i)
                {
                    fprintf(stdout, "%s\n", values[i].c_str());
                }
            }
        }
        else if (0 == strcasecmp(cmd, "srandmember"))
        {
            // SRANDMEMBER command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd srandmember key count\n");
                exit(1);
            }

            count = redis_client.srandmember(key, atoi(argv[3]), &values, &which_node);
            for (i=0; i<count; ++i)
                fprintf(stdout, "%s\n", values[i].c_str());
        }
        else if (0 == strcasecmp(cmd, "srem"))
        {
            // SREM command
            if (argc < 4)
            {
                fprintf(stderr, "Usage: r3c_cmd srem key member1 member2 ...\n");
                exit(1);
            }

            if (4 == argc)
            {
                count = redis_client.srem(key, argv[3], &which_node);
            }
            else
            {
                for (i=3; i<argc; ++i)
                    values.push_back(argv[i]);
                count = redis_client.srem(key, values, &which_node);
            }
            fprintf(stdout, "%d\n", count);
        }
        else if (0 == strcasecmp(cmd, "sscan"))
        {
            // SSCAN command
            if ((argc < 4) || (argc > 6))
            {
                fprintf(stderr, "Usage1: r3c_cmd sscan key cursor\n");
                fprintf(stderr, "Usage2: r3c_cmd sscan key cursor count\n");
                fprintf(stderr, "Usage3: r3c_cmd sscan key cursor pattern\n");
                fprintf(stderr, "Usage4: r3c_cmd sscan key cursor pattern count\n");
                exit(1);
            }

            cursor = static_cast<int64_t>(atoll(argv[3]));
            if (4 == argc)
            {
                cursor = redis_client.sscan(key, cursor, &values, &which_node);
            }
            else if (5 == argc)
            {
                count = atoi(argv[4]);
                if (count > 0)
                    cursor = redis_client.sscan(key, cursor, count, &values, &which_node);
                else
                    cursor = redis_client.sscan(key, cursor, argv[4], &values, &which_node);
            }
            else if (6 == argc)
            {
                cursor = redis_client.sscan(key, cursor, argv[4], atoi(argv[5]), &values, &which_node);
            }

            fprintf(stdout, "cursor: %" PRId64", count: %d\n", cursor, static_cast<int>(values.size()));
            for (i=0; i<static_cast<int>(values.size()); ++i)
                fprintf(stdout, "%s\n", values[i].c_str());
        }
        ////////////////////////////////////////////////////////////////////////////
        // SORTED SET
        else if (0 == strcasecmp(cmd, "zrem"))
        {
            // ZREM command
            if (argc < 4)
            {
                fprintf(stderr, "Usage: r3c_cmd zrem key field1 score2 field2 ...\n");
                exit(1);
            }

            for (i=3; i<argc; ++i)
                fields.push_back(argv[i]);
            count = redis_client.zrem(key, fields, &which_node);
            fprintf(stdout, "%d\n", count);
        }
        else if (0 == strcasecmp(cmd, "zadd"))
        {
            // ZADD command
            if ((argc < 5) || (argc%2!=1))
            {
                fprintf(stderr, "Usage: r3c_cmd zadd key score1 field1 score2 field2 ...\n");
                exit(1);
            }

            if (5 == argc)
            {
                count = redis_client.zadd(key, argv[4], atol(argv[3]), r3c::Z_NS, &which_node);
                fprintf(stdout, "%d\n", count);
            }
            else
            {
                std::map<std::string, int64_t> map;
                for (i=3; i<argc; i+=2)
                    map[argv[i+1]] = atol(argv[i]);
                count = redis_client.zadd(key, map, r3c::Z_NS, &which_node);
                fprintf(stdout, "%d\n", count);
            }
        }
        else if (0 == strcasecmp(cmd, "zcard"))
        {
            // ZCARD command
            if (argc != 3)
            {
                fprintf(stderr, "Usage: r3c_cmd zcard key\n");
                exit(1);
            }

            ret64 = redis_client.zcard(key, &which_node);
            fprintf(stdout, "count: %" PRId64"\n", ret64);
        }
        else if (0 == strcasecmp(cmd, "zcount"))
        {
            // ZCOUNT command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd zcount key min max\n");
                exit(1);
            }

            min = atoll(argv[3]);
            max = atoll(argv[4]);
            count = redis_client.zcount(key, min, max, &which_node);
            fprintf(stdout, "%d\n", count);
        }
        else if (0 == strcasecmp(cmd, "zincrby"))
        {
            // ZINCRBY command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd zincrby key increment field\n");
                exit(1);
            }

            int64_t m = redis_client.zincrby(key, argv[4], atoll(argv[3]), &which_node);
            fprintf(stdout, "%" PRId64"\n", m);
        }
        else if (0 == strcasecmp(cmd, "zrange"))
        {
            // ZRANGE command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd zrange key start end\n");
                exit(1);
            }

            i = 0;
            start = atoi(argv[3]);
            end = atoi(argv[4]);
            std::vector<std::pair<std::string, int64_t> > vec;
            ret = redis_client.zrange(key, start, end, true, &vec, &which_node);
            fprintf(stdout, "number: %d\n", ret);
            for (std::vector<std::pair<std::string, int64_t> >::iterator iter=vec.begin(); iter!=vec.end(); ++iter,++i)
                fprintf(stdout, "[%d]%s => %" PRId64"\n", i, iter->first.c_str(), iter->second);
        }
        else if (0 == strcasecmp(cmd, "zrevrange"))
        {
            // ZREVRANGE command
            if (argc != 5)
            {
                fprintf(stderr, "Usage: r3c_cmd zrevrange key start end\n");
                exit(1);
            }

            i = 0;
            start = atoi(argv[3]);
            end = atoi(argv[4]);
            std::vector<std::pair<std::string, int64_t> > vec;
            ret = redis_client.zrevrange(key, start, end, true, &vec, &which_node);
            fprintf(stdout, "number: %d\n", ret);
            for (std::vector<std::pair<std::string, int64_t> >::iterator iter=vec.begin(); iter!=vec.end(); ++iter,++i)
                fprintf(stdout, "[%d]%s => %" PRId64"\n", i, iter->first.c_str(), iter->second);
        }
        else if (0 == strcasecmp(cmd, "zrangebyscore"))
        {
            // ZRANGEBYSCORE command
            if ((argc != 5) && (argc != 7))
            {
                fprintf(stderr, "Usage1: r3c_cmd zrangebyscore key min max\n");
                fprintf(stderr, "Usage2: r3c_cmd zrangebyscore key min max offset count\n");
                exit(1);
            }

            i = 0;
            min = atol(argv[3]);
            max = atol(argv[4]);
            std::vector<std::pair<std::string, int64_t> > vec;

            if (5 == argc)
            {
                ret = redis_client.zrangebyscore(key, min, max, true, &vec, &which_node);
            }
            else
            {
                offset = atol(argv[5]);
                count = atol(argv[6]);
                ret = redis_client.zrangebyscore(key, min, max, offset, count, true, &vec, &which_node);
            }
            fprintf(stdout, "number: %d\n", ret);
            for (std::vector<std::pair<std::string, int64_t> >::iterator iter=vec.begin(); iter!=vec.end(); ++iter,++i)
                fprintf(stdout, "[%d]%s => %" PRId64"\n", i, iter->first.c_str(), iter->second);
        }
        else if (0 == strcasecmp(cmd, "zrevrangebyscore"))
        {
            // ZREVRANGEBYSCORE command
            if ((argc != 5) && (argc != 7))
            {
                fprintf(stderr, "Usage1: r3c_cmd zrevrangebyscore key max min\n");
                fprintf(stderr, "Usage2: r3c_cmd zrangebyscore key max min offset count\n");
                exit(1);
            }

            i = 0;
            max = atol(argv[3]);
            min = atol(argv[4]);
            std::vector<std::pair<std::string, int64_t> > vec;

            if (5 == argc)
            {
                ret = redis_client.zrevrangebyscore(key, max, min, true, &vec, &which_node);
            }
            else
            {
                offset = atol(argv[5]);
                count = atol(argv[6]);
                ret = redis_client.zrevrangebyscore(key, max, min, offset, count, true, &vec, &which_node);
            }
            fprintf(stdout, "number: %d\n", ret);
            for (std::vector<std::pair<std::string, int64_t> >::iterator iter=vec.begin(); iter!=vec.end(); ++iter,++i)
                fprintf(stdout, "[%d]%s => %" PRId64"\n", i, iter->first.c_str(), iter->second);
        }
        else if (0 == strcasecmp(cmd, "zrank"))
        {
            // ZRANK command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd zrank key field\n");
                exit(1);
            }

            int rank = redis_client.zrank(key, argv[3], &which_node);
            fprintf(stdout, "[%s] => %d\n", argv[3], rank);
        }
        else if (0 == strcasecmp(cmd, "zrevrank"))
        {
            // ZREVRANK command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd zrevrank key field\n");
                exit(1);
            }

            int rank = redis_client.zrevrank(key, argv[3], &which_node);
            fprintf(stdout, "[%s] => %d\n", argv[3], rank);
        }
        else if (0 == strcasecmp(cmd, "zscore"))
        {
            // ZSCORE command
            if (argc != 4)
            {
                fprintf(stderr, "Usage: r3c_cmd zscore key field\n");
                exit(1);
            }

            double score = redis_client.zscore(key, argv[3], &which_node);
            fprintf(stdout, "[%s] => %.2f\n", argv[3], score);
        }
        else if (0 == strcasecmp(cmd, "zscan"))
        {
            // ZSCAN command
            if ((argc < 4) || (argc > 6))
            {
                fprintf(stderr, "Usage1: r3c_cmd zscan key cursor\n");
                fprintf(stderr, "Usage2: r3c_cmd zscan key cursor count\n");
                fprintf(stderr, "Usage3: r3c_cmd zscan key cursor pattern\n");
                fprintf(stderr, "Usage4: r3c_cmd zscan key cursor pattern count\n");
                exit(1);
            }

            cursor = static_cast<int64_t>(atoll(argv[3]));
            if (4 == argc)
            {
                cursor = redis_client.zscan(key, cursor, &values2, &which_node);
            }
            else if (5 == argc)
            {
                count = atoi(argv[4]);
                if (count > 0)
                    cursor = redis_client.zscan(key, cursor, count, &values2, &which_node);
                else
                    cursor = redis_client.zscan(key, cursor, argv[4], &values2, &which_node);
            }
            else if (6 == argc)
            {
                cursor = redis_client.zscan(key, cursor, argv[4], atoi(argv[5]), &values2, &which_node);
            }

            fprintf(stdout, "cursor: %" PRId64", count: %d\n", cursor, static_cast<int>(values2.size()));
            for (i=0; i<static_cast<int>(values2.size()); ++i)
                fprintf(stdout, "%s => %" PRId64"\n", values2[i].first.c_str(), values2[i].second);
        }
        else
        {
            fprintf(stderr, "command[%s] not supported\n", cmd);
            exit(1);
        }
    }
    catch (r3c::CRedisException& ex)
    {
        fprintf(stderr, PRINT_COLOR_RED"%s" PRINT_COLOR_NONE"\n", ex.str().c_str());
        exit(1);
    }

    return 0;
}
