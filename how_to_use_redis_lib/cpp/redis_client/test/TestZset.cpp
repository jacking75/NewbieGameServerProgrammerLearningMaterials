#include "TestZset.hpp"

CTestZset::CTestZset()
{
}

bool CTestZset::StartTest(const std::string &strHost)
{
    bool bSuccess = false;
    std::cout << "start to test zset command" << std::endl;
    if (!m_redis.Initialize(strHost, 6379, 2, 10))
        std::cout << "init redis client failed" << std::endl;
    else
        bSuccess = Test_Zcard() && Test_Zcount() && Test_Zincrby() && Test_Zinterstore() &&
                   Test_Zlexcount() && Test_Zrangebylex() && Test_Zrangebyscore() && Test_Zrank() &&
                   Test_Zrem() && Test_Zremrangebylex() && Test_Zremrangebyrank() && Test_Zremrangebyscore() &&
                   Test_Zrevrange() && Test_Zrevrangebyscore() && Test_Zrevrank() && Test_Zscan() &&
                   Test_Zscore() && Test_Zunionstore() && Test_Zrangewithscore();
    std::cout << std::endl;
    return bSuccess;
}

bool CTestZset::Test_Zcard()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(4, 3))
        return PrintResult("zcard", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Zcard("tk_str_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Zcard("tk_zset_3", &nVal) == RC_SUCCESS && nVal == 3 &&
        m_redis.Zcard("tk_zset_4", &nVal) == RC_SUCCESS && nVal == 0)
        bSuccess = true;
    return PrintResult("zcard", bSuccess);
}

bool CTestZset::Test_Zcount()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(10, 9))
        return PrintResult("zcount", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Zcount("tk_str_1", 1, 3, &nVal) == RC_REPLY_ERR &&
        m_redis.Zcount("tk_zset_3", 1.1, 1.9, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zcount("tk_zset_3", 1.2, 5.9, &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Zcount("tk_zset_9", 3.2, 6.7, &nVal) == RC_SUCCESS && nVal == 3 &&
        m_redis.Zcount("tk_zset_10", 3.2, 6.7, &nVal) == RC_SUCCESS && nVal == 0)
        bSuccess = true;
    return PrintResult("zcount", bSuccess);
}

bool CTestZset::Test_Zincrby()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(4, 3))
        return PrintResult("zincrby", false);

    bool bSuccess = false;
    double dVal;
    if (m_redis.Zincrby("tk_str_1", 1, "value_1", &dVal) == RC_REPLY_ERR &&
        m_redis.Zincrby("tk_zset_3", 2.3, "value_2", &dVal) == RC_SUCCESS && std::abs(dVal - 4.3) < FLOAT_ZERO &&
        m_redis.Zincrby("tk_zset_3", 6.5, "value_4", &dVal) == RC_SUCCESS && std::abs(dVal - 6.5) < FLOAT_ZERO &&
        m_redis.Zincrby("tk_zset_4", 3.7, "value_1", &dVal) == RC_SUCCESS && std::abs(dVal - 3.7) < FLOAT_ZERO)
        bSuccess = true;
    return PrintResult("zincrby", bSuccess);
}

bool CTestZset::Test_Zinterstore()
{
    return true;
}

bool CTestZset::Test_Zlexcount()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(2, 0))
        return PrintResult("zlexcount", false);

    for (int i = 0; i < 9; ++i)
    {
        std::stringstream sstream;
        sstream << "value_" << i + 1;
        if (m_redis.Zadd("tk_zset_1", 1, sstream.str()) != RC_SUCCESS)
        return PrintResult("zlexcount", false);
    }

    bool bSuccess = false;
    long nVal;
    if (m_redis.Zlexcount("tk_str_1", "(value_1", "(value_8", &nVal) == RC_REPLY_ERR &&
        m_redis.Zlexcount("tk_zset_1", "(value_1", "value_8", &nVal) == RC_REPLY_ERR &&
        m_redis.Zlexcount("tk_zset_2", "(value_1", "(value_8", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zlexcount("tk_zset_1", "(value_8", "(value_2", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zlexcount("tk_zset_1", "(value_2", "(value_8", &nVal) == RC_SUCCESS && nVal == 5 &&
        m_redis.Zlexcount("tk_zset_1", "[value_2", "[value_8", &nVal) == RC_SUCCESS && nVal == 7 &&
        m_redis.Zlexcount("tk_zset_1", "[value_2", "(value_99", &nVal) == RC_SUCCESS && nVal == 8)
        bSuccess = true;
    return PrintResult("zlexcount", bSuccess);
}

bool CTestZset::Test_Zrangebylex()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(2, 0))
        return PrintResult("zrangebylex", false);

    for (int i = 0; i < 9; ++i)
    {
        std::stringstream sstream;
        sstream << "value_" << i + 1;
        if (m_redis.Zadd("tk_zset_1", 1, sstream.str()) != RC_SUCCESS)
        return PrintResult("zrangebylex", false);
    }

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Zrangebylex("tk_str_1", "(value_1", "(value_8", &vecVal) == RC_REPLY_ERR &&
        m_redis.Zrangebylex("tk_zset_1", "value_1", "(value_8", &vecVal) == RC_REPLY_ERR &&
        m_redis.Zrangebylex("tk_zset_2", "(value_1", "(value_8", &vecVal) == RC_SUCCESS && vecVal.empty() &&
        m_redis.Zrangebylex("tk_zset_1", "(value_8", "(value_1", &vecVal) == RC_SUCCESS && vecVal.empty() &&
        m_redis.Zrangebylex("tk_zset_1", "(value_2", "(value_8", &vecVal) == RC_SUCCESS && vecVal.size() == 5 &&
        vecVal[0] == "value_3" && vecVal[1] == "value_4" && vecVal[2] == "value_5" &&
        vecVal[3] == "value_6" && vecVal[4] == "value_7" &&
        m_redis.Zrangebylex("tk_zset_1", "[value_3", "(value_7", &vecVal) == RC_SUCCESS && vecVal.size() == 4 &&
        vecVal[0] == "value_3" && vecVal[1] == "value_4" && vecVal[2] == "value_5" && vecVal[3] == "value_6" &&
        m_redis.Zrangebylex("tk_zset_1", "(value_7", "[value_99", &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        vecVal[0] == "value_8" && vecVal[1] == "value_9")
        bSuccess = true;
    return PrintResult("zrangebylex", bSuccess);
}

bool CTestZset::Test_Zrangebyscore()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(10, 9))
        return PrintResult("zrangebyscore", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    std::map<std::string, double> mapVal;
    if (m_redis.Zrangebyscore("tk_str_1", 1, 3, &vecVal) == RC_REPLY_ERR &&
        m_redis.Zrangebyscore("tk_zset_10", 1, 3, &vecVal) == RC_SUCCESS && vecVal.empty() &&
        m_redis.Zrangebyscore("tk_zset_9", 6.2, 6, &vecVal) == RC_SUCCESS && vecVal.size() == 0 &&
        m_redis.Zrangebyscore("tk_zset_9", 3, 6, &vecVal) == RC_SUCCESS && vecVal.size() == 4 &&
        vecVal[0] == "value_3" && vecVal[1] == "value_4" && vecVal[2] == "value_5" && vecVal[3] == "value_6" &&
        m_redis.Zrangebyscore("tk_zset_9", 3, 6, &mapVal) == RC_SUCCESS && mapVal.size() == 4 &&
        IsPairInMap(std::string("value_3"), 3.0, mapVal) && IsPairInMap(std::string("value_4"), 4.0, mapVal) &&
        IsPairInMap(std::string("value_5"), 5.0, mapVal) && IsPairInMap(std::string("value_6"), 6.0, mapVal) &&
        m_redis.Zrangebyscore("tk_zset_9", 6.01, 10.7, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        vecVal[0] == "value_7" && vecVal[1] == "value_8" && vecVal[2] == "value_9" &&
        m_redis.Zrangebyscore("tk_zset_9", 6.01, 10.7, &mapVal) == RC_SUCCESS && mapVal.size() == 3 &&
        IsPairInMap(std::string("value_7"), 7.0, mapVal) && IsPairInMap(std::string("value_8"), 8.0, mapVal) &&
        IsPairInMap(std::string("value_9"), 9.0, mapVal))
        bSuccess = true;
    return PrintResult("zrangebyscore", bSuccess);
}

bool CTestZset::Test_Zrank()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(10, 9))
        return PrintResult("zrank", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Zrank("tk_str_1", "value_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Zrank("tk_zset_9", "value_10", &nVal) == RC_OBJ_NOT_EXIST &&
        m_redis.Zrank("tk_zset_10", "value_1", &nVal) == RC_OBJ_NOT_EXIST &&
        m_redis.Zrank("tk_zset_8", "value_5", &nVal) == RC_SUCCESS && nVal == 4 &&
        m_redis.Zrank("tk_zset_8", "value_1", &nVal) == RC_SUCCESS && nVal == 0)
        bSuccess = true;
    return PrintResult("zrank", bSuccess);
}

bool CTestZset::Test_Zrem()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(4, 3))
        return PrintResult("zrem", false);

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    if (m_redis.Zrem("tk_str_1", "value_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Zrem("tk_zset_4", "value_1", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zrem("tk_zset_1", "value_10", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zrem("tk_zset_3", "value_2", &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Zrange("tk_zset_3", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        !IsInContainer(vecVal, "value_2"))
        bSuccess = true;
    return PrintResult("zrem", bSuccess);
}

bool CTestZset::Test_Zremrangebylex()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(2, 0))
        return PrintResult("zremrangebylex", false);

    for (int i = 0; i < 9; ++i)
    {
        std::stringstream sstream;
        sstream << "value_" << i + 1;
        if (m_redis.Zadd("tk_zset_1", 1, sstream.str()) != RC_SUCCESS)
        return PrintResult("zremrangebylex", false);
    }

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    if (m_redis.Zremrangebylex("tk_str_1", "(value_2", "(value_7", &nVal) == RC_REPLY_ERR &&
        m_redis.Zremrangebylex("tk_zset_1", "value_2", "(value_7", &nVal) == RC_REPLY_ERR &&
        m_redis.Zremrangebylex("tk_zset_2", "(value_2", "(value_7", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zremrangebylex("tk_zset_1", "(value_7", "(value_2", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zremrangebylex("tk_zset_1", "(value_2", "(value_7", &nVal) == RC_SUCCESS && nVal == 4 &&
        m_redis.Zrange("tk_zset_1", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 5 &&
        !IsInContainer(vecVal, "value_3") && !IsInContainer(vecVal, "value_4") &&
        !IsInContainer(vecVal, "value_5") && !IsInContainer(vecVal, "value_6") &&
        m_redis.Zremrangebylex("tk_zset_1", "[value_8", "(value_99", &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Zrange("tk_zset_1", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        !IsInContainer(vecVal, "value_8") && !IsInContainer(vecVal, "value_9"))
        bSuccess = true;
    return PrintResult("zremrangebylex", bSuccess);
}

bool CTestZset::Test_Zremrangebyrank()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(10, 9))
        return PrintResult("zremrangebyrank", false);

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    if (m_redis.Zremrangebyrank("tk_str_1", 3, 8, &nVal) == RC_REPLY_ERR &&
        m_redis.Zremrangebyrank("tk_str_10", 3, 8, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zremrangebyrank("tk_zset_9", 8, 3, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zremrangebyrank("tk_zset_9", 2, 5, &nVal) == RC_SUCCESS && nVal == 4 &&
        m_redis.Zrange("tk_zset_9", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 5 &&
        !IsInContainer(vecVal, "value_3") && !IsInContainer(vecVal, "value_4") &&
        !IsInContainer(vecVal, "value_5") && !IsInContainer(vecVal, "value_6") &&
        m_redis.Zremrangebyrank("tk_zset_7", 5, 10, &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Zrange("tk_zset_7", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 5 &&
        !IsInContainer(vecVal, "value_6") && !IsInContainer(vecVal, "value_7"))
        bSuccess = true;
    return PrintResult("zremrangebyrank", bSuccess);
}

bool CTestZset::Test_Zremrangebyscore()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(10, 9))
        return PrintResult("zremrangebyscore", false);

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    if (m_redis.Zremrangebyscore("tk_str_1", 3, 8, &nVal) == RC_REPLY_ERR &&
        m_redis.Zremrangebyscore("tk_zset_10", 3, 8, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zremrangebyscore("tk_zset_9", 8, 3, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Zremrangebyscore("tk_zset_9", 3, 8, &nVal) == RC_SUCCESS && nVal == 6 &&
        m_redis.Zrange("tk_zset_9", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        IsInContainer(vecVal, "value_1") && IsInContainer(vecVal, "value_2") &&
        IsInContainer(vecVal, "value_9") &&
        m_redis.Zremrangebyscore("tk_zset_7", 5.1, 10, &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Zrange("tk_zset_7", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 5 &&
        !IsInContainer(vecVal, "value_6") && !IsInContainer(vecVal, "value_7"))
        bSuccess = true;
    return PrintResult("zremrangebyscore", bSuccess);
}

bool CTestZset::Test_Zrevrange()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(10, 9))
        return PrintResult("zrevrange", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Zrevrange("tk_str_1", 0, -1, &vecVal) == RC_REPLY_ERR &&
        m_redis.Zrevrange("tk_zset_10", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 0 &&
        m_redis.Zrevrange("tk_zset_3", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        vecVal[0] == "value_3" && vecVal[1] == "value_2" && vecVal[2] == "value_1" &&
        m_redis.Zrevrange("tk_zset_9", 5, 7, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        vecVal[0] == "value_4" && vecVal[1] == "value_3" && vecVal[2] == "value_2" &&
        m_redis.Zrevrange("tk_zset_9", 7, 100, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        vecVal[0] == "value_2" && vecVal[1] == "value_1")
        bSuccess = true;
    return PrintResult("zrevrange", bSuccess);
}

bool CTestZset::Test_Zrevrangebyscore()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(10, 9))
        return PrintResult("zrevrangebyscore", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    std::map<std::string, double> mapVal;
    if (m_redis.Zrevrangebyscore("tk_str_1", 3, 1, &vecVal) == RC_REPLY_ERR &&
        m_redis.Zrevrangebyscore("tk_zset_10", 3, 1, &vecVal) == RC_SUCCESS && vecVal.empty() &&
        m_redis.Zrevrangebyscore("tk_zset_9", 6, 6.2, &vecVal) == RC_SUCCESS && vecVal.size() == 0 &&
        m_redis.Zrevrangebyscore("tk_zset_9", 6, 3, &vecVal) == RC_SUCCESS && vecVal.size() == 4 &&
        vecVal[0] == "value_6" && vecVal[1] == "value_5" && vecVal[2] == "value_4" && vecVal[3] == "value_3" &&
        m_redis.Zrevrangebyscore("tk_zset_9", 6, 3, &mapVal) == RC_SUCCESS && mapVal.size() == 4 &&
        IsPairInMap(std::string("value_3"), 3.0, mapVal) && IsPairInMap(std::string("value_4"), 4.0, mapVal) &&
        IsPairInMap(std::string("value_5"), 5.0, mapVal) && IsPairInMap(std::string("value_6"), 6.0, mapVal) &&
        m_redis.Zrevrangebyscore("tk_zset_9", 10.7, 6.01, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        vecVal[0] == "value_9" && vecVal[1] == "value_8" && vecVal[2] == "value_7" &&
        m_redis.Zrevrangebyscore("tk_zset_9", 10.7, 6.01, &mapVal) == RC_SUCCESS && mapVal.size() == 3 &&
        IsPairInMap(std::string("value_7"), 7.0, mapVal) && IsPairInMap(std::string("value_8"), 8.0, mapVal) &&
        IsPairInMap(std::string("value_9"), 9.0, mapVal))
        bSuccess = true;
    return PrintResult("zrevrangebyscore", bSuccess);
}

bool CTestZset::Test_Zrevrank()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(5, 4))
        return PrintResult("zrevrank", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Zrevrank("tk_str_1", "value_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Zrevrank("tk_zset_5", "value_1", &nVal) == RC_OBJ_NOT_EXIST &&
        m_redis.Zrevrank("tk_zset_4", "value_10", &nVal) == RC_OBJ_NOT_EXIST &&
        m_redis.Zrevrank("tk_zset_4", "value_3", &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Zrevrank("tk_zset_4", "value_1", &nVal) == RC_SUCCESS && nVal == 3)
        bSuccess = true;
    return PrintResult("zrevrank", bSuccess);
}

bool CTestZset::Test_Zscan()
{
    return true;
}

bool CTestZset::Test_Zscore()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(4, 3))
        return PrintResult("zscore", false);

    bool bSuccess = false;
    double dVal;
    if (m_redis.Zscore("tk_str_1", "value_1", &dVal) == RC_REPLY_ERR &&
        m_redis.Zscore("tk_zset_3", "value_2", &dVal) == RC_SUCCESS && std::abs(dVal - 2) < FLOAT_ZERO &&
        m_redis.Zscore("tk_zset_3", "value_4", &dVal) == RC_OBJ_NOT_EXIST &&
        m_redis.Zscore("tk_zset_4", "value_1", &dVal) == RC_OBJ_NOT_EXIST)
        bSuccess = true;
    return PrintResult("zscore", bSuccess);
}

bool CTestZset::Test_Zunionstore()
{
    return true;
}

bool CTestZset::Test_Zrangewithscore()
{
    if (!InitStringEnv(1, 1) || !InitZsetEnv(5, 4))
        return PrintResult("zrangewithscore", false);

    bool bSuccess = false;
    std::map<std::string, std::string> mapVal;
    if (//m_redis.Zrangewithscore("tk_str_1", 0, -1, &mapVal) == RC_REPLY_ERR &&
        //m_redis.Zrangewithscore("tk_zset_5", 0, -1, &mapVal) == RC_SUCCESS && mapVal.empty() &&
        m_redis.Zrangewithscore("tk_zset_4", 0, -1, &mapVal) == RC_SUCCESS && mapVal.size() == 4 &&
        IsPairInMap(std::string("value_1"), std::string("1"), mapVal) &&
        IsPairInMap(std::string("value_2"), std::string("2"), mapVal) &&
        IsPairInMap(std::string("value_3"), std::string("3"), mapVal) &&
        IsPairInMap(std::string("value_4"), std::string("4"), mapVal))
        bSuccess = true;
    return PrintResult("zrangewithscore", bSuccess);
}

