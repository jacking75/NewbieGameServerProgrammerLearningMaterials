// Writed by yijian (eyjian@qq.com)
// Redis命令扩展module
#include "redismodule.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// 带超时的INCRBY命令
// 格式：INCRBYEX KEY SECONDS INCREMENT
//
// 示例：
// ex.incrbyex k1 10 1
// ex.incrbyex k1 10 2
int incrbyex_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // When automatic memory management is enabled:
    // 1) don't need to close open keys
    // 2) don't need to free replies
    // 3) don't need to free RedisModuleString objects
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */
    if (argc != 4) return RedisModule_WrongArity(ctx);

    RedisModuleKey *key = (RedisModuleKey*)RedisModule_OpenKey(ctx, argv[1],
            REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_STRING &&
        type != REDISMODULE_KEYTYPE_EMPTY)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    RedisModuleString* seconds = argv[2];
    RedisModuleString* increment = argv[3];
    long long ll_seconds; // 过期时长
    long long ll_newval; // 新的值
    if (RedisModule_StringToLongLong(seconds, &ll_seconds) != REDISMODULE_OK) {
        return RedisModule_ReplyWithError(ctx, "ERR value is not an integer or out of range");
    }
    if (RedisModule_StringToLongLong(increment, &ll_newval) != REDISMODULE_OK) {
        return RedisModule_ReplyWithError(ctx, "ERR value is not an integer or out of range");
    }

    size_t len;
    char *s = RedisModule_StringDMA(key, &len, REDISMODULE_READ|REDISMODULE_WRITE);
    if (len == 0 || s == NULL || *s == '\0') {
        // set必须在Expire之前，否则会冲掉Expire的作用，
        // 这也是else分支未用RedisModule_StringSet的原因
        RedisModule_StringSet(key, increment);
        RedisModule_SetExpire(key, ll_seconds*1000); // 以秒为单位，需要乘以1000
    }
    else {
        char* endptr;
        long long ll_oldval = strtoll(s, &endptr, 10); // s不一定是有效的数字，所以需要做检查
        ll_newval = ll_newval + ll_oldval;
        if ((errno == ERANGE && (ll_oldval == LLONG_MAX || ll_oldval == LLONG_MIN))
                || (errno != 0 && ll_oldval == 0)) {
            return RedisModule_ReplyWithError(ctx, "ERR value is not an integer or out of range");
        }
        if (endptr == s || *endptr != '\0') {
            return RedisModule_ReplyWithError(ctx, "ERR value is not an integer or out of range");
        }

        size_t newval_len;
        RedisModuleString* newval = RedisModule_CreateStringFromLongLong(ctx, ll_newval);
        const char* newval_s = RedisModule_StringPtrLen(newval, &newval_len);
        if (newval_len > len)
            RedisModule_StringTruncate(key, newval_len);
        strncpy(s, newval_s, newval_len);
    }

    RedisModule_ReplicateVerbatim(ctx); // 写AOF和复制到slaves
    RedisModule_ReplyWithLongLong(ctx, ll_newval);
    return REDISMODULE_OK;
}

// 实现命令HMINCRBY，同时对HASH的多个field值进行增减操作
// 格式：HMINCRBY KEY FIELD1 VALUE1 FIELD2 VALUE2 FIELD3 VALUE3 ......
//
// 示例：
// hmincrby k1 f1 1 f2 2
// hmincrby k1 f5 1 f6 2 f7 3
int hmincrby_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // When automatic memory management is enabled:
    // 1) don't need to close open keys
    // 2) don't need to free replies
    // 3) don't need to free RedisModuleString objects
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */
    if (argc < 4 || argc % 2 != 0) return RedisModule_WrongArity(ctx);

    RedisModuleKey *key = (RedisModuleKey*)RedisModule_OpenKey(ctx, argv[1],
            REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_HASH &&
        type != REDISMODULE_KEYTYPE_EMPTY)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    const int count = argc/2 - 1; // 键值对个数
    std::vector<long long> newval_array(count); // 用来存储新值的数组
    for (int i=2; i<argc; i+=2) {
        RedisModuleString* field = argv[i];
        RedisModuleString* incrvalue = argv[i+1];
        long long ll_newvalue;
        if (RedisModule_StringToLongLong(incrvalue, &ll_newvalue) != REDISMODULE_OK) {
            return RedisModule_ReplyWithError(ctx, "ERR value is not an integer or out of range");
        }

        RedisModuleString *oldval;
        RedisModule_HashGet(key, REDISMODULE_HASH_NONE, field, &oldval, NULL);
        if (NULL == oldval) { // field不存在时
            RedisModule_HashSet(key,REDISMODULE_HASH_NONE, field, incrvalue, NULL);
        }
        else { // field已存在时
            long long ll_oldval;
            if (RedisModule_StringToLongLong(oldval, &ll_oldval) != REDISMODULE_OK) {
                return RedisModule_ReplyWithError(ctx, "ERR hash value is not an integer");
            }

            ll_newvalue = ll_newvalue + ll_oldval; // 累加得到新值
            RedisModuleString* newval = RedisModule_CreateStringFromLongLong(ctx, ll_newvalue);
            RedisModule_HashSet(key,REDISMODULE_HASH_NONE, field, newval, NULL);
        }

        newval_array[i] = ll_newvalue;
    }

    RedisModule_ReplicateVerbatim(ctx); // 写AOF和复制到slaves
    RedisModule_ReplyWithArray(ctx, count); // 返回数组类型的Reply
    for (std::vector<long long>::size_type i=0; i<newval_array.size(); ++i)
        RedisModule_ReplyWithLongLong(ctx, newval_array[i]);
    return REDISMODULE_OK;
}

extern "C"
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx, "command_extension", 1, REDISMODULE_APIVER_1)
        == REDISMODULE_ERR) return REDISMODULE_ERR;

    /* Log the list of parameters passing loading the module. */
    for (int j=0; j<argc; ++j) {
        const char *s = RedisModule_StringPtrLen(argv[j], NULL);
        printf("Module loaded with ARGV[%d] = %s\n", j, s);
    }

    if (RedisModule_CreateCommand(ctx, "ex.incrbyex",
            incrbyex_RedisCommand,"write", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "ex.hmincrby",
            hmincrby_RedisCommand,"write", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;
    return REDISMODULE_OK;
}
