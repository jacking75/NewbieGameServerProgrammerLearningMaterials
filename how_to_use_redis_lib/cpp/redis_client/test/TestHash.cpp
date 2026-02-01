#include "TestHash.hpp"

CTestHash::CTestHash()
{
}

bool CTestHash::StartTest(const std::string &strHost)
{
    bool bSuccess = false;
    std::cout << "start to test hash command" << std::endl;
    if (!m_redis.Initialize(strHost, 6379, 2, 10))
        std::cout << "init redis client failed" << std::endl;
    else
        bSuccess = Test_Hexists() && Test_Hdel() && Test_Hgetall() && Test_Hincrby() &&
                   Test_Hincrbyfloat() && Test_Hkeys() && Test_Hlen() && Test_Hmget() &&
                   Test_Hmset() && Test_Hscan() && Test_Hsetnx() && Test_Hvals();
    std::cout << std::endl;
    return bSuccess;
}

bool CTestHash::Test_Hdel()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(4, 3))
        return PrintResult("hdel", false);

    bool bSuccess = false;
    long nDelVal, nExtVal;
    if (m_redis.Hdel("tk_str_1", "field_1", &nDelVal) == RC_REPLY_ERR &&
        m_redis.Hdel("tk_hash_4", "field_10", &nDelVal) == RC_SUCCESS && nDelVal == 0 &&
        m_redis.Hdel("tk_hash_3", "field_10", &nDelVal) == RC_SUCCESS && nDelVal == 0 &&
        m_redis.Hdel("tk_hash_3", "field_2", &nDelVal) == RC_SUCCESS && nDelVal == 1 &&
        m_redis.Hexists("tk_hash_3", "field_2", &nExtVal) == RC_SUCCESS && nExtVal == 0)
        bSuccess = true;
    return PrintResult("hdel", bSuccess);
}

bool CTestHash::Test_Hexists()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(4, 3))
        return PrintResult("hexists", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Hexists("tk_str_1", "field_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Hexists("tk_hash_4", "field_1", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Hexists("tk_hash_3", "field_10", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Hexists("tk_hash_3", "field_2", &nVal) == RC_SUCCESS && nVal == 1)
        bSuccess = true;
    return PrintResult("hexists", bSuccess);
}

bool CTestHash::Test_Hgetall()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(4, 3))
        return PrintResult("hgetall", false);

    bool bSuccess = false;
    std::map<std::string, std::string> mapKv;
    if (m_redis.Hgetall("tk_str_1", &mapKv) == RC_REPLY_ERR &&
        m_redis.Hgetall("tk_hash_4", &mapKv) == RC_SUCCESS && mapKv.empty() &&
        m_redis.Hgetall("tk_hash_3", &mapKv) == RC_SUCCESS && mapKv.size() == 3)
    {
        std::map<std::string, std::string>::iterator it;
        if ((it = mapKv.find("field_1")) != mapKv.end() && it->second == "value_1" &&
            (it = mapKv.find("field_2")) != mapKv.end() && it->second == "value_2" &&
            (it = mapKv.find("field_3")) != mapKv.end() && it->second == "value_3")
            bSuccess = true;
    }
    return PrintResult("hgetall", bSuccess);
}

bool CTestHash::Test_Hincrby()
{
    if (!InitStringEnv(2, 1) || !InitHashEnv(1, 1))
        return PrintResult("hincrby", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Hincrby("tk_str_1", "field_1", 4, &nVal) == RC_REPLY_ERR &&
        m_redis.Hincrby("tk_hash_1", "field_1", 4, &nVal) == RC_REPLY_ERR &&
        m_redis.Hincrby("tk_hash_1", "field_2", 4, &nVal) == RC_SUCCESS && nVal == 4 &&
        m_redis.Hincrby("tk_hash_1", "field_2", 5, &nVal) == RC_SUCCESS && nVal == 9 &&
        m_redis.Hset("tk_hash_1", "field_2", "3.4") == RC_SUCCESS &&
        m_redis.Hincrby("tk_hash_1", "field_2", 4, &nVal) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("hincrby", bSuccess);
}

bool CTestHash::Test_Hincrbyfloat()
{
    if (!InitStringEnv(2, 1) || !InitHashEnv(1, 1))
        return PrintResult("hincrbyfloat", false);

    bool bSuccess = false;
    long nVal;
    double dVal;
    if (m_redis.Hincrbyfloat("tk_str_1", "field_1", 4.2, &dVal) == RC_REPLY_ERR &&
        m_redis.Hincrbyfloat("tk_hash_1", "field_1", 4.2, &dVal) == RC_REPLY_ERR &&
        m_redis.Hincrbyfloat("tk_hash_1", "field_2", 4.2, &dVal) == RC_SUCCESS && std::abs(dVal - 4.2) < FLOAT_ZERO &&
        m_redis.Hincrbyfloat("tk_hash_1", "field_2", 5.1, &dVal) == RC_SUCCESS && std::abs(dVal - 9.3) < FLOAT_ZERO &&
        m_redis.Hincrby("tk_hash_1", "field_3", 3, &nVal) == RC_SUCCESS && nVal == 3 &&
        m_redis.Hincrbyfloat("tk_hash_1", "field_3", 2.7, &dVal) == RC_SUCCESS && std::abs(dVal - 5.7) < 0.000001)
        bSuccess = true;
    return PrintResult("hincrbyfloat", bSuccess);
}

bool CTestHash::Test_Hkeys()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(10, 9))
        return PrintResult("hkeys", false);

    bool bSuccess = false;
    std::vector<std::string> vecKey;
    if (m_redis.Hkeys("tk_str_1", &vecKey) == RC_REPLY_ERR &&
        m_redis.Hkeys("tk_hash_10", &vecKey) == RC_SUCCESS && vecKey.empty() &&
        m_redis.Hkeys("tk_hash_6", &vecKey) == RC_SUCCESS && vecKey.size() == 6 &&
        IsInContainer(vecKey, "field_1") && IsInContainer(vecKey, "field_2") && IsInContainer(vecKey, "field_3") &&
        IsInContainer(vecKey, "field_4") && IsInContainer(vecKey, "field_5") && IsInContainer(vecKey, "field_6"))
        bSuccess = true;
    return PrintResult("hkeys", bSuccess);
}

bool CTestHash::Test_Hlen()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(10, 9))
        return PrintResult("hlen", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Hlen("tk_str_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Hlen("tk_hash_10", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Hlen("tk_hash_6", &nVal) == RC_SUCCESS && nVal == 6)
        bSuccess = true;
    return PrintResult("hlen", bSuccess);
}

bool CTestHash::Test_Hmget()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(10, 9))
        return PrintResult("hmget", false);

    bool bSuccess = false;
    std::vector<std::string> vecField;
    std::vector<std::string> vecVal;
    std::map<std::string, std::string> mapFv;
    vecField.push_back("field_1");
    vecField.push_back("field_3");
    vecField.push_back("field_10");
    vecField.push_back("field_4");
    vecField.push_back("field_9");
    if (m_redis.Hmget("tk_str_1", vecField, &vecVal) == RC_REPLY_ERR &&
        m_redis.Hmget("tk_hash_10", vecField, &vecVal) == RC_SUCCESS && vecVal.size() == 5 &&
        vecVal[0].empty() && vecVal[1].empty() && vecVal[2].empty() && vecVal[3].empty() && vecVal[4].empty() &&
        m_redis.Hmget("tk_hash_8", vecField, &vecVal) == RC_SUCCESS && vecVal.size() == 5 &&
        vecVal[0] == "value_1" && vecVal[1] == "value_3" && vecVal[2].empty() && vecVal[3] == "value_4" && vecVal[4].empty() &&
        m_redis.Hmget("tk_hash_8", vecField, &mapFv) == RC_SUCCESS && mapFv.size() == 3 &&
        IsPairInMap(std::string("field_1"), std::string("value_1"), mapFv) &&
        IsPairInMap(std::string("field_3"), std::string("value_3"), mapFv) &&
        IsPairInMap(std::string("field_4"), std::string("value_4"), mapFv))
        bSuccess = true;
    return PrintResult("hmget", bSuccess);
}

bool CTestHash::Test_Hmset()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(10, 9))
        return PrintResult("hmset", false);

    bool bSuccess = false;
    std::vector<std::string> vecField;
    std::vector<std::string> vecVal;
    std::vector<std::string> vecVal1;
    std::map<std::string, std::string> mapFv1, mapFv2;
    vecField.push_back("field_1");
    vecField.push_back("field_3");
    vecField.push_back("field_10");
    vecField.push_back("field_4");
    vecField.push_back("field_9");
    vecVal.push_back("new_1");
    vecVal.push_back("new_3");
    vecVal.push_back("new_10");
    vecVal.push_back("new_4");
    vecVal.push_back("new_9");
    mapFv1.insert(std::make_pair("field_1", "new_101"));
    mapFv1.insert(std::make_pair("field_3", "new_103"));
    mapFv1.insert(std::make_pair("field_10", "new_110"));
    mapFv1.insert(std::make_pair("field_4", "new_104"));
    mapFv1.insert(std::make_pair("field_9", "new_109"));

    if (m_redis.Hmset("tk_str_1", vecField, vecVal) == RC_REPLY_ERR &&
        m_redis.Hmset("tk_hash_8", vecField, vecVal) == RC_SUCCESS &&
        m_redis.Hmget("tk_hash_8", vecField, &vecVal1) == RC_SUCCESS && vecVal1.size() == 5 &&
        vecVal1[0] == "new_1" && vecVal[1] == "new_3" && vecVal[2] == "new_10" &&
        vecVal[3] == "new_4" && vecVal[4] == "new_9" &&
        m_redis.Hmset("tk_hash_10", mapFv1) == RC_SUCCESS &&
        m_redis.Hmget("tk_hash_10", vecField, &mapFv2) == RC_SUCCESS && mapFv2.size() == 5 &&
        IsPairInMap(std::string("field_1"), std::string("new_101"), mapFv2) &&
        IsPairInMap(std::string("field_3"), std::string("new_103"), mapFv2) &&
        IsPairInMap(std::string("field_10"), std::string("new_110"), mapFv2) &&
        IsPairInMap(std::string("field_4"), std::string("new_104"), mapFv2) &&
        IsPairInMap(std::string("field_9"), std::string("new_109"), mapFv2))
        bSuccess = true;
    return PrintResult("hmset", bSuccess);
}

bool CTestHash::Test_Hscan()
{
    return true;
}

bool CTestHash::Test_Hsetnx()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(2, 1))
        return PrintResult("hsetnx", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Hsetnx("tk_str_1", "field_1", "new_1") == RC_REPLY_ERR &&
        m_redis.Hsetnx("tk_hash_1", "field_1", "new_1") == RC_REPLY_ERR &&
        m_redis.Hsetnx("tk_hash_1", "field_2", "new_2") == RC_SUCCESS &&
        m_redis.Hget("tk_hash_1", "field_1", &strVal) == RC_SUCCESS && strVal == "value_1" &&
        m_redis.Hget("tk_hash_1", "field_2", &strVal) == RC_SUCCESS && strVal == "new_2")
        bSuccess = true;
    return PrintResult("hsetnx", bSuccess);
}

bool CTestHash::Test_Hvals()
{
    if (!InitStringEnv(1, 1) || !InitHashEnv(10, 9))
        return PrintResult("hvals", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Hvals("tk_str_1", &vecVal) == RC_REPLY_ERR &&
        m_redis.Hvals("tk_hash_10", &vecVal) == RC_SUCCESS && vecVal.empty() &&
        m_redis.Hvals("tk_hash_6", &vecVal) == RC_SUCCESS && vecVal.size() == 6 &&
        IsInContainer(vecVal, "value_1") && IsInContainer(vecVal, "value_2") && IsInContainer(vecVal, "value_3") &&
        IsInContainer(vecVal, "value_4") && IsInContainer(vecVal, "value_5") && IsInContainer(vecVal, "value_6"))
        bSuccess = true;
    return PrintResult("hvals", bSuccess);
}
