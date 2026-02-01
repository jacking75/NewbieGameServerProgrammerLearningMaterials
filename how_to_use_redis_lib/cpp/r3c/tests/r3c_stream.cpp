// Writed by yijian (eyjian@qq.com)
// For test stream
#include "r3c.h"
#include "utils.h"
#include <iostream>

typedef void (*TESTCASE)(r3c::CRedisClient&);
static void init_testcase(TESTCASE testcase[]);
static void usage(char* argv[]);

// argv[1] redis nodes
// argv[2] specify the test case
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        usage(argv);
        exit(1);
    }
    else
    {
        try
        {
            const int n = atoi(argv[2]);
            const int num_testcases = 10;
            TESTCASE testcase[num_testcases];
            r3c::CRedisClient redis(argv[1], r3c::RP_READ_REPLICA);
            init_testcase(testcase);

            if (n<0 || n>=num_testcases-1)
            {
                usage(argv);
                exit(1);
            }
            else
            {
                (*testcase[n])(redis);
                return 0;
            }
        }
        catch (r3c::CRedisException& ex)
        {
            fprintf(stderr, "%s\n", ex.str().c_str());
            exit(1);
        }
    }
}

void usage(char* argv[])
{
    fprintf(stderr, "Usage: %s <redis nodes> testcase\n", argv[0]);
    fprintf(stderr, "Example: %s 192.168.1.61:6379,192.168.1.62:6379 1\n", argv[0]);
}


// test xreadgroup
static void testcase0(r3c::CRedisClient& redis)
{
    const std::string group = "group";
    const std::string consumer = "consumer";
    std::vector<std::string> keys(2);
    std::vector<std::string> ids(2);
    std::vector<r3c::FVPair> fvpairs(3);
    int count = 10;

    keys[0] = "k0";
    keys[1] = "k1";

    // XADD
    fvpairs[0].field = "field00";
    fvpairs[0].value = "value00";
    fvpairs[1].field = "field01";
    fvpairs[1].value = "value01";
    fvpairs[2].field = "field02";
    fvpairs[2].value = "value02";
    ids[0] = redis.xadd(keys[0], "*", fvpairs);

    fvpairs[0].field = "field10";
    fvpairs[0].value = "value10";
    fvpairs[1].field = "field11";
    fvpairs[1].value = "value11";
    fvpairs[2].field = "field12";
    fvpairs[2].value = "value12";
    ids[1] = redis.xadd(keys[1], "*", fvpairs);

    // XGROUP CREATE
    try
    {
        redis.xgroup_create(keys[0], group, "$");
    }
    catch (r3c::CRedisException& ex)
    {
        if (!r3c::is_busygroup_error(ex.errtype()))
            throw;
    }
    try
    {
        redis.xgroup_create(keys[1], group, "$");
    }
    catch (r3c::CRedisException& ex)
    {
        if (!r3c::is_busygroup_error(ex.errtype()))
            throw;
    }

    // XREADGROUP
    ids[0] = ">";
    ids[1] = ">";
    std::vector<r3c::Stream> values;
    redis.xreadgroup(group, consumer, keys, ids, count, &values);
    std::cout << values << std::endl;
}

// test xreadgroup
static void testcase1(r3c::CRedisClient& redis)
{
    const std::string group = "group";
    const std::string consumer = "consumer";
    std::vector<std::string> keys(1);
    std::vector<r3c::FVPair> fvpairs(3);
    int count = 10;
    int64_t block_milliseconds = 0;
    bool noack = true;

    keys[0] = "k000";

    // XGROUP CREATE
    try
    {
        redis.xgroup_create(keys[0], group, "$", true);
    }
    catch (r3c::CRedisException& ex)
    {
        if (!r3c::is_busygroup_error(ex.errtype()))
            throw;
    }

    // XADD
    fvpairs[0].field = "field00";
    fvpairs[0].value = "value00";
    fvpairs[1].field = "field01";
    fvpairs[1].value = "value01";
    fvpairs[2].field = "field02";
    fvpairs[2].value = "value02";
    redis.xadd(keys[0], "*", fvpairs);

    // XREADGROUP
    std::vector<r3c::StreamEntry> values;
    redis.xreadgroup(group, consumer, keys[0], count, block_milliseconds, noack, &values);
    std::cout << values << std::endl;
}

// test xread
static void testcase2(r3c::CRedisClient& redis)
{
    std::vector<std::string> keys(2);
    std::vector<std::string> ids(2);
    std::vector<r3c::FVPair> fvpairs(3);
    int count = 10;

    keys[0] = "k2";
    keys[1] = "k3";

    // XADD
    fvpairs[0].field = "field00";
    fvpairs[0].value = "value00";
    fvpairs[1].field = "field01";
    fvpairs[1].value = "value01";
    fvpairs[2].field = "field02";
    fvpairs[2].value = "value02";
    ids[0] = redis.xadd(keys[0], "*", fvpairs);

    fvpairs[0].field = "field10";
    fvpairs[0].value = "value10";
    fvpairs[1].field = "field11";
    fvpairs[1].value = "value11";
    fvpairs[2].field = "field12";
    fvpairs[2].value = "value12";
    ids[1] = redis.xadd(keys[1], "*", fvpairs);

    // XREAD
    ids[0] = "0-0";
    ids[1] = "0-0";
    std::vector<r3c::Stream> values;
    redis.xread(keys, ids, count, &values);
    std::cout << values << std::endl;
}

// test xrange
static void testcase3(r3c::CRedisClient& redis)
{
    std::string key = "k4";
    std::string start, end;
    std::vector<r3c::FVPair> fvpairs(3);
    std::vector<std::string> ids(2);
    std::vector<r3c::StreamEntry> values;

    // XADD
    fvpairs[0].field = "field00";
    fvpairs[0].value = "value00";
    fvpairs[1].field = "field01";
    fvpairs[1].value = "value01";
    fvpairs[2].field = "field02";
    fvpairs[2].value = "value02";
    redis.xadd(key, "*", fvpairs);

    // XADD
    fvpairs[0].field = "field10";
    fvpairs[0].value = "value10";
    fvpairs[1].field = "field11";
    fvpairs[1].value = "value11";
    fvpairs[2].field = "field12";
    fvpairs[2].value = "value12";
    redis.xadd(key, "*", fvpairs);

    start = "-";
    end = "+";
    redis.xrange(key, start, end, &values);
    std::cout << values << std::endl;
}

// test xpending
static void testcase4(r3c::CRedisClient& redis)
{
    std::string key = "k1";
    std::string group = "g1";
    struct r3c::GroupPending groups;

    // Group
    redis.xpending(key, group, &groups);
}

// test xpending
static void testcase5(r3c::CRedisClient& redis)
{
    std::string key = "k1";
    std::string group = "g1";
    struct r3c::GroupPending groups;

    // Group
    redis.xpending(key, group, &groups);
    fprintf(stdout, "COUNT:%d\n", groups.count);
    fprintf(stdout, "stratid: %s\n", groups.start.c_str());
    fprintf(stdout, "endid: %s\n", groups.end.c_str());
    for (std::vector<struct r3c::ConsumerPending>::size_type i=0; i<groups.consumers.size(); ++i)
    {
        fprintf(stdout, "[CONSUMER%zu] %s: %d\n", i, groups.consumers[i].name.c_str(), groups.consumers[i].count);
    }

    // Consumer
    if (!groups.consumers.empty())
    {
        std::vector<struct r3c::DetailedPending> pendings;
        const int n = redis.xpending(key, group, groups.start, groups.end, groups.count, groups.consumers[0].name, &pendings);

        fprintf(stdout, "\n");
        for (int i=0; i<n; ++i)
        {
            const struct r3c::DetailedPending& pending = pendings[i];
            fprintf(stdout, "ID: %s\n", pending.id.c_str());
            fprintf(stdout, "Consumer: %s\n", pending.consumer.c_str());
            fprintf(stdout, "Elapsed: %" PRId64" ms\n", pending.elapsed);
            fprintf(stdout, "Delivered: %" PRId64"\n", pending.delivered);
        }
        fprintf(stdout, "\n");
    }
}

static void testcase6(r3c::CRedisClient& redis)
{
    std::string key = "k1";
    std::string group = "g1";
    std::string consumer = "c1";
    std::string id = "*";
    std::vector<std::string> ids;
    std::vector<r3c::StreamEntry> entries;
    std::vector<r3c::FVPair> fvpairs(3);
    int64_t count = 10;
    int64_t block_milliseconds = -1;
    int64_t miniidle = 1;
    bool noack = true;

    redis.del(key);
    fvpairs[0].field = "f0";
    fvpairs[0].value = "v0";
    fvpairs[1].field = "f1";
    fvpairs[1].value = "v1";
    fvpairs[2].field = "f2";
    fvpairs[2].value = "v2";
    redis.xadd(key, id, fvpairs);

    redis.xgroup_create(key, group, "$", true);
    redis.xreadgroup(group, consumer, key, count, block_milliseconds, noack, &entries);
    std::cout << entries << std::endl;

    fprintf(stdout, "\n");
    extract_ids(entries, &ids);
    entries.clear();
    redis.xclaim(key, group, "c2", miniidle, ids, &entries);
}

// test:
// xinfo_consumers
// xinfo_groups
// xinfo_stream
static void testcase7(r3c::CRedisClient& redis)
{
    std::string key = "k1";
    std::string group = "g1";
    std::string consumer = "c1";
    std::string id = "*";
    std::vector<std::string> ids;
    std::vector<r3c::StreamEntry> entries;
    std::vector<r3c::FVPair> fvpairs(5);
    int64_t count = 10;
    int64_t block_milliseconds = -1;
    bool noack = true;

    redis.del(key);
    fvpairs[0].field = "f0";
    fvpairs[0].value = "v0";
    fvpairs[1].field = "f1";
    fvpairs[1].value = "v1";
    fvpairs[2].field = "f2";
    fvpairs[2].value = "v2";
    fvpairs[3].field = "f3";
    fvpairs[3].value = "v3";
    fvpairs[4].field = "f4";
    fvpairs[4].value = "v4";
    redis.xadd(key, id, fvpairs);

    redis.xgroup_create(key, group, "$", true);
    redis.xreadgroup(group, "c1", key, count, block_milliseconds, noack, &entries);
    std::cout << entries << std::endl;
    redis.xreadgroup(group, "c2", key, count, block_milliseconds, noack, &entries);
    std::cout << entries << std::endl;
    redis.xreadgroup(group, "c3", key, count, block_milliseconds, noack, &entries);
    std::cout << entries << std::endl;

    {
        std::vector<struct r3c::ConsumerInfo> consumers_info;
        std::vector<struct r3c::GroupInfo> groups_info;
        struct r3c::StreamInfo stream_info;

        fprintf(stdout, "info consumers:\n");
        redis.xinfo_consumers(key, group, &consumers_info);
        fprintf(stdout, "info groups:\n");
        redis.xinfo_groups(key, &groups_info);
        fprintf(stdout, "info stream:\n");
        redis.xinfo_stream(key, &stream_info);
        std::cout << stream_info << std::endl;
    }
}

void init_testcase(TESTCASE testcase[])
{
    int i = 0;
    testcase[i++] = testcase0;
    testcase[i++] = testcase1;
    testcase[i++] = testcase2;
    testcase[i++] = testcase3;
    testcase[i++] = testcase4;
    testcase[i++] = testcase5;
    testcase[i++] = testcase6;
    testcase[i++] = testcase7;
}
