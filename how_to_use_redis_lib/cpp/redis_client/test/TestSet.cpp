#include "TestSet.hpp"

CTestSet::CTestSet()
{
}

bool CTestSet::StartTest(const std::string &strHost)
{
    bool bSuccess = false;
    std::cout << "start to test set command" << std::endl;
    if (!m_redis.Initialize(strHost, 6379, 2, 10))
        std::cout << "init redis client failed" << std::endl;
    else
        bSuccess = Test_Scard() && Test_Sismember() && Test_Spop() && Test_Srem();
    std::cout << std::endl;
    return bSuccess;
}

bool CTestSet::Test_Scard()
{
    if (!InitStringEnv(1, 1) || !InitSetEnv(5, 4))
        return PrintResult("scard", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Scard("tk_str_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Scard("tk_set_5", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Scard("tk_set_3", &nVal) == RC_SUCCESS && nVal == 3)
        bSuccess = true;
    return PrintResult("scard", bSuccess);
}

bool CTestSet::Test_Sdiff()
{
    //if (!InitStringEnv(1, 1) || !InitSetEnv(6, 5) || !InitSetexEnv(6, 5))
    //    return PrintResult("sdiff", false);

    //bool bSuccess = false;
    //long nVal;
    //std::vector<std::string> vecVal;
    //std::vector<std::string> vecKey1, vecKey2;
    //vecKey1.push_back("tk_set_1");
    //vecKey1.push_back("tk_set_3");
    //vecKey1.push_back("tk_set_6");
    //vecKey2.push_back("tk_str_1");
    //vecKey2.push_back("tk_set_3");
    //if ((m_redis.IsCluster() && m_redis.Scard("tk_str_1", vecKey1, &vecVal) == RC_NOT_SUPPORT) ||
    //     RC_REPLY_ERR &&
    //    m_redis.Scard("tk_set_1", vecKey2, &vecVal) == RC_REPLY_ERR &&
    //    m_redis.Scard("tk_set_5", vecKey1, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
    //    IsInContainer(vecVal, "value_4") && IsInContainer(vecVal, "value_5") &&
    //    bSuccess = true;
    //return PrintResult("sdiff", bSuccess);
    return true;
}

bool CTestSet::Test_Sdiffstore()
{
    return true;
}

bool CTestSet::Test_Sinter()
{
    return true;
}

bool CTestSet::Test_Sinterstore()
{
    return true;
}

bool CTestSet::Test_Sismember()
{
    if (!InitStringEnv(1, 1) || !InitSetEnv(4, 3))
        return PrintResult("sismember", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Sismember("tk_str_1", "value_2", &nVal) == RC_REPLY_ERR &&
        m_redis.Sismember("tk_set_4", "value_2", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Sismember("tk_set_3", "value_10", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Sismember("tk_set_3", "value_2", &nVal) == RC_SUCCESS && nVal == 1)
        bSuccess = true;
    return PrintResult("sismember", bSuccess);
}

bool CTestSet::Test_Smove()
{
    return true;
}

bool CTestSet::Test_Spop()
{
    if (!InitStringEnv(1, 1) || !InitSetEnv(4, 3))
        return PrintResult("spop", false);

    bool bSuccess = false;
    long nVal;
    std::string strVal;
    if (m_redis.Spop("tk_str_1", &strVal) == RC_REPLY_ERR &&
        m_redis.Spop("tk_set_4", &strVal) == RC_SUCCESS && strVal.empty() &&
        m_redis.Spop("tk_set_3", &strVal) == RC_SUCCESS &&
        (strVal == "value_1" || strVal == "value_2" || strVal == "value_3") &&
        m_redis.Scard("tk_set_3", &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Spop("tk_set_3", &strVal) == RC_SUCCESS && !strVal.empty() &&
        m_redis.Spop("tk_set_3", &strVal) == RC_SUCCESS && !strVal.empty() &&
        m_redis.Spop("tk_set_3", &strVal) == RC_SUCCESS && strVal.empty())
        bSuccess = true;
    return PrintResult("spop", bSuccess);
}

bool CTestSet::Test_Srandmember()
{
    return true;
}

bool CTestSet::Test_Srem()
{
    if (!InitStringEnv(1, 1) || !InitSetEnv(8, 7))
        return PrintResult("srem", false);

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    vecVal.push_back("value_2");
    vecVal.push_back("value_4");
    vecVal.push_back("value_6");
    vecVal.push_back("value_8");
    if (m_redis.Srem("tk_str_1", "value_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Srem("tk_set_8", "value_1", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Srem("tk_set_7", "value_10", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Srem("tk_set_6", "value_5", &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Sismember("tk_set_6", "value_5", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Srem("tk_set_7", vecVal, &nVal) == RC_SUCCESS && nVal == 3)
        bSuccess = true;
    return PrintResult("srem", bSuccess);
}

bool CTestSet::Test_Sscan()
{
    return true;
}

bool CTestSet::Test_Sunion()
{
    return true;
}

bool CTestSet::Test_Sunionstore()
{
    return true;
}
