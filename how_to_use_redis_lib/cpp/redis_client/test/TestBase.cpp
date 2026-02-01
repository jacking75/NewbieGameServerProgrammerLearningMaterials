#include "TestBase.hpp"

CTestBase::CTestBase()
{
}

bool CTestBase::StartTest(const std::string &strHost)
{
    bool bSuccess = false;
    std::cout << "start to test base command" << std::endl;
    if (!m_redis.Initialize(strHost, 6379, 2, 10))
        std::cout << "initialize redis client failed" << std::endl;
    else
        bSuccess = Test_Set() && Test_Lpush() && Test_Rpush() && Test_Sadd() &&
                   Test_Zadd() && Test_Hset() && Test_Get() && Test_Lrange() &&
                   Test_Smembers() && Test_Zrange() && Test_Hget();

    std::cout << std::endl;
    return bSuccess;
}

bool CTestBase::Test_Get()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(3, 2))
        return PrintResult("get", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Get("tk_str_2", &strVal) == RC_SUCCESS && strVal.empty() &&
        m_redis.Get("tk_str_1", &strVal) == RC_SUCCESS && strVal == "value_1" &&
        m_redis.Get("tk_list_1", &strVal) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("get", bSuccess);
}

bool CTestBase::Test_Hget()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(3, 2))
        return PrintResult("hget", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Hget("tk_str_1", "field_1", &strVal) == RC_REPLY_ERR &&
        m_redis.Hget("tk_hash_3", "field_1", &strVal) == RC_SUCCESS && strVal.empty() &&
        m_redis.Hget("tk_hash_2", "field_3", &strVal) == RC_SUCCESS && strVal.empty() &&
        m_redis.Hget("tk_hash_2", "field_1", &strVal) == RC_SUCCESS && strVal == "value_1")
        bSuccess = true;
    return PrintResult("hget", bSuccess);
}

bool CTestBase::Test_Hset()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(2, 1))
        return PrintResult("hset", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Hset("tk_str_1", "field_1", "new_1") == RC_REPLY_ERR &&
        m_redis.Hset("tk_hash_1", "field_1", "new_1") == RC_SUCCESS &&
        m_redis.Hset("tk_hash_1", "field_2", "new_2") == RC_SUCCESS &&
        m_redis.Hget("tk_hash_1", "field_2", &strVal) == RC_SUCCESS && strVal == "new_2" &&
        m_redis.Hset("tk_hash_2", "field_1", "new_1") == RC_SUCCESS &&
        m_redis.Hget("tk_hash_2", "field_1", &strVal) == RC_SUCCESS && strVal == "new_1")
        bSuccess = true;
    return PrintResult("hset", bSuccess);
}

bool CTestBase::Test_Lpush()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(2, 1))
        return PrintResult("lpush", false);

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    if (m_redis.Lpush("tk_str_1", "value_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Lpush("tk_list_1", "value_2", &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Lrange("tk_list_1", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        vecVal[0] == "value_2" && vecVal[1] == "value_1" &&
        m_redis.Lpush("tk_list_2", "value_1", &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Lrange("tk_list_2", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && vecVal[0] == "value_1")
        bSuccess = true;
    return PrintResult("lpush", bSuccess);
}

bool CTestBase::Test_Lrange()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(10, 9))
        return PrintResult("lrange", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Lrange("tk_str_1", 0, -1, &vecVal) == RC_REPLY_ERR &&
        m_redis.Lrange("tk_list_10", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 0 &&
        m_redis.Lrange("tk_list_3", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        IsInContainer(vecVal, "value_1") && IsInContainer(vecVal, "value_2") && IsInContainer(vecVal, "value_3") &&
        m_redis.Lrange("tk_list_9", 5, 7, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        IsInContainer(vecVal, "value_6") && IsInContainer(vecVal, "value_7") && IsInContainer(vecVal, "value_8") &&
        m_redis.Lrange("tk_list_9", 7, 100, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 2 && IsInContainer(vecVal, "value_8") && IsInContainer(vecVal, "value_9"))
        bSuccess = true;
    return PrintResult("lrange", bSuccess);
}

bool CTestBase::Test_Rpush()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(2, 1))
        return PrintResult("rpush", false);

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    if (m_redis.Rpush("tk_str_1", "value_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Rpush("tk_list_1", "value_2", &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Lrange("tk_list_1", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        IsInContainer(vecVal, "value_2") && IsInContainer(vecVal, "value_1") &&
        vecVal[0] == "value_1" && vecVal[1] == "value_2" &&
        m_redis.Rpush("tk_list_2", "value_1", &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Lrange("tk_list_2", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && vecVal[0] == "value_1")
        bSuccess = true;
    return PrintResult("rpush", bSuccess);
}

bool CTestBase::Test_Sadd()
{
    if (!InitStringEnv(1, 1) || !InitSetEnv(10, 9))
        return PrintResult("sadd", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Sadd("tk_str_1", "value_1") == RC_REPLY_ERR &&
        m_redis.Sadd("tk_set_8", "value_2") == RC_SUCCESS &&
        m_redis.Sadd("tk_set_8", "value_10") == RC_SUCCESS &&
        m_redis.Smembers("tk_set_8", &vecVal) == RC_SUCCESS &&
        vecVal.size() == 9 && IsInContainer(vecVal, "value_10") &&
        m_redis.Sadd("tk_set_10", "value_1") == RC_SUCCESS &&
        m_redis.Smembers("tk_set_10", &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && IsInContainer(vecVal, "value_1"))
        bSuccess = true;
    return PrintResult("sadd", bSuccess);
}

bool CTestBase::Test_Set()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("set", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Set("tk_str_1", "new_1") == RC_SUCCESS &&
        m_redis.Get("tk_str_1", &strVal) == RC_SUCCESS && strVal == "new_1" &&
        m_redis.Set("tk_str_2", "new_2") == RC_SUCCESS &&
        m_redis.Get("tk_str_2", &strVal) == RC_SUCCESS && strVal == "new_2" &&
        m_redis.Set("tk_list_1", "new_3") == RC_SUCCESS &&
        m_redis.Get("tk_list_1", &strVal) == RC_SUCCESS && strVal == "new_3")
        bSuccess = true;
    return PrintResult("set", bSuccess);
}

bool CTestBase::Test_Smembers()
{
    if (!InitStringEnv(1, 1) || !InitSetEnv(10, 9))
        return PrintResult("smembers", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Smembers("tk_str_1", &vecVal) == RC_REPLY_ERR &&
        m_redis.Smembers("tk_set_10", &vecVal) == RC_SUCCESS && vecVal.size() == 0 &&
        m_redis.Smembers("tk_set_1", &vecVal) == RC_SUCCESS && vecVal.size() == 1 &&
        m_redis.Smembers("tk_set_5", &vecVal) == RC_SUCCESS && vecVal.size() == 5 &&
        m_redis.Smembers("tk_set_9", &vecVal) == RC_SUCCESS && vecVal.size() == 9)
        bSuccess = true;
    return PrintResult("smembers", bSuccess);
}

bool CTestBase::Test_Zadd()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(10, 9))
        return PrintResult("zadd", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Zadd("tk_str_1", 1, "value_1") == RC_REPLY_ERR &&
        m_redis.Zadd("tk_zset_8", 1, "value_2") == RC_SUCCESS &&
        m_redis.Zadd("tk_zset_8", 1, "value_10") == RC_SUCCESS &&
        m_redis.Zrange("tk_zset_8", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 9 && IsInContainer(vecVal, "value_10") &&
        m_redis.Zadd("tk_zset_10", 1, "value_1") == RC_SUCCESS &&
        m_redis.Zrange("tk_zset_10", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && IsInContainer(vecVal, "value_1"))
        bSuccess = true;
    return PrintResult("zadd", bSuccess);
}

bool CTestBase::Test_Zrange()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(10, 9))
        return PrintResult("zrange", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Zrange("tk_str_1", 0, -1, &vecVal) == RC_REPLY_ERR &&
        m_redis.Zrange("tk_zset_10", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 0 &&
        m_redis.Zrange("tk_zset_3", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        vecVal[0] == "value_1" && vecVal[1] == "value_2" && vecVal[2] == "value_3" &&
        m_redis.Zrange("tk_zset_9", 5, 7, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        vecVal[0] == "value_6" && vecVal[1] == "value_7" && vecVal[2] == "value_8" &&
        m_redis.Zrange("tk_zset_9", 7, 100, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        vecVal[0] == "value_8" && vecVal[1] == "value_9")
        bSuccess = true;
    return PrintResult("zrange", bSuccess);
}

