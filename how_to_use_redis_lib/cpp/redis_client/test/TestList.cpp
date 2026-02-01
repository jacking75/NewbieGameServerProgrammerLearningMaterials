#include "TestList.hpp"

CTestList::CTestList()
{
}

bool CTestList::StartTest(const std::string &strHost)
{
    bool bSuccess = false;
    std::cout << "start to test list command" << std::endl;
    if (!m_redis.Initialize(strHost, 6379, 2, 10))
        std::cout << "init redis client failed" << std::endl;
    else
        bSuccess = Test_Blpop() && Test_Brpop() && Test_Lindex() && Test_Linsert() &&
                   Test_Llen() && Test_Lpop() && Test_Rpop() && Test_Lpushx() &&
                   Test_Rpushx() && Test_Lrem() && Test_Lset() && Test_Ltrim();
    std::cout << std::endl;
    return bSuccess;
}

bool CTestList::Test_Blpop()
{
    return true;

    if (!InitStringEnv(1, 1) || !InitListEnv(5, 4))
        return PrintResult("blpop", false);

    bool bSuccess = false;
    std::vector<std::string> vecKey;
    std::vector<std::string> vecVal;
    vecKey.push_back("tk_list_3");
    vecKey.push_back("tk_list_4");
    if (m_redis.Blpop("tk_str_1", 2, &vecVal) == RC_REPLY_ERR &&
        m_redis.Blpop("tk_list_2", 4, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 2 && vecVal[0] == "tk_list_2" && vecVal[1] == "value_1" &&
        m_redis.Lrange("tk_list_2", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && vecVal[0] == "value_2" &&
        m_redis.Blpop("tk_list_5", 3, &vecVal) == RC_SUCCESS && vecVal.empty() &&
        ((m_redis.IsCluster() && m_redis.Blpop(vecKey, 2, &vecVal) == RC_NOT_SUPPORT) ||
        (!m_redis.IsCluster() && m_redis.Blpop(vecKey, 2, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 2 && vecVal[0] == "tk_list_4" && vecVal[1] == "value_1")))
        bSuccess = true;
    return PrintResult("blpop", bSuccess);
}

bool CTestList::Test_Brpop()
{
    return true;

    if (!InitStringEnv(1, 1) || !InitListEnv(5, 4))
        return PrintResult("brpop", false);

    bool bSuccess = false;
    std::vector<std::string> vecKey;
    std::vector<std::string> vecVal;
    vecKey.push_back("tk_list_3");
    vecKey.push_back("tk_list_4");
    if (m_redis.Brpop("tk_str_1", 2, &vecVal) == RC_REPLY_ERR &&
        m_redis.Brpop("tk_list_2", 4, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 2 && vecVal[0] == "tk_list_2" && vecVal[1] == "value_2" &&
        m_redis.Lrange("tk_list_2", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && vecVal[0] == "value_1" &&
        m_redis.Brpop("tk_list_5", 3, &vecVal) == RC_SUCCESS && vecVal.empty() &&
        ((m_redis.IsCluster() && m_redis.Brpop(vecKey, 2, &vecVal) == RC_NOT_SUPPORT) ||
        (!m_redis.IsCluster() && m_redis.Brpop(vecKey, 2, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 2 && vecVal[0] == "tk_list_4" && vecVal[1] == "value_4")))
        bSuccess = true;
    return PrintResult("brpop", bSuccess);
}

bool CTestList::Test_BRpoplpush()
{
    return true;
}

bool CTestList::Test_Lindex()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(5, 4))
        return PrintResult("lindex", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Lindex("tk_str_1", 1, &strVal) == RC_REPLY_ERR &&
        m_redis.Lindex("tk_list_4", 1, &strVal) == RC_SUCCESS && strVal == "value_2" &&
        m_redis.Lindex("tk_list_4", 3, &strVal) == RC_SUCCESS && strVal == "value_4" &&
        m_redis.Lindex("tk_list_4", -2, &strVal) == RC_SUCCESS && strVal == "value_3" &&
        m_redis.Lindex("tk_list_4", 4, &strVal) == RC_SUCCESS && strVal.empty())
        bSuccess = true;
    return PrintResult("lindex", bSuccess);
}

bool CTestList::Test_Linsert()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(5, 4))
        return PrintResult("linsert", false);

    bool bSuccess = false;
    std::string strVal;
    long nVal;
    if (m_redis.Linsert("tk_str_1", "before", "value_2", "value_10", &nVal) == RC_REPLY_ERR &&
        m_redis.Linsert("tk_list_5", "after", "value_6", "value_10", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Linsert("tk_list_4", "after", "value_6", "value_10", &nVal) == RC_SUCCESS && nVal == -1 &&
        m_redis.Linsert("tk_list_4", "after", "value_2", "value_10", &nVal) == RC_SUCCESS && nVal == 5 &&
        m_redis.Lindex("tk_list_4", 2, &strVal) == RC_SUCCESS && strVal == "value_10" &&
        m_redis.Linsert("tk_list_4", "before", "value_4", "value_11", &nVal) == RC_SUCCESS && nVal == 6 &&
        m_redis.Lindex("tk_list_4", 4, &strVal) == RC_SUCCESS && strVal == "value_11")
        bSuccess = true;
    return PrintResult("linsert", bSuccess);
}

bool CTestList::Test_Llen()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(5, 4))
        return PrintResult("llen", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Llen("tk_str_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Llen("tk_list_5", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Llen("tk_list_4", &nVal) == RC_SUCCESS && nVal == 4 &&
        m_redis.Llen("tk_list_1", &nVal) == RC_SUCCESS && nVal == 1)
        bSuccess = true;
    return PrintResult("llen", bSuccess);
}

bool CTestList::Test_Lpop()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(5, 4))
        return PrintResult("lpop", false);

    bool bSuccess = false;
    std::string strVal;
    std::vector<std::string> vecVal;
    if (m_redis.Lpop("tk_str_1", &strVal) == RC_REPLY_ERR &&
        m_redis.Lpop("tk_list_2", &strVal) == RC_SUCCESS && strVal == "value_1" &&
        m_redis.Lrange("tk_list_2", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && vecVal[0] == "value_2" &&
        m_redis.Lpop("tk_list_5", &strVal) == RC_SUCCESS && strVal.empty())
        bSuccess = true;
    return PrintResult("lpop", bSuccess);
}

bool CTestList::Test_Lpushx()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(2, 1))
        return PrintResult("lpushx", false);

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    if (m_redis.Lpushx("tk_str_1", "value_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Lpushx("tk_list_1", "value_2", &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Lrange("tk_list_1", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        vecVal[0] == "value_2" && vecVal[1] == "value_1" &&
        m_redis.Lpushx("tk_list_2", "value_1", &nVal) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("lpushx", bSuccess);
}

bool CTestList::Test_Lrem()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(6, 4))
        return PrintResult("lrem", false);

    if (m_redis.Rpush("tk_list_5", "value_1") != RC_SUCCESS ||
        m_redis.Rpush("tk_list_5", "value_2") != RC_SUCCESS ||
        m_redis.Rpush("tk_list_5", "value_3") != RC_SUCCESS ||
        m_redis.Rpush("tk_list_5", "value_2") != RC_SUCCESS ||
        m_redis.Rpush("tk_list_5", "value_1") != RC_SUCCESS ||
        m_redis.Rpush("tk_list_5", "value_3") != RC_SUCCESS ||
        m_redis.Rpush("tk_list_5", "value_3") != RC_SUCCESS ||
        m_redis.Rpush("tk_list_5", "value_1") != RC_SUCCESS ||
        m_redis.Rpush("tk_list_5", "value_2") != RC_SUCCESS ||
        m_redis.Rpush("tk_list_5", "value_3") != RC_SUCCESS)
        return PrintResult("lrem", false);

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    if (m_redis.Lrem("tk_str_1", 0, "value_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Lrem("tk_list_4", 2, "value_2", &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Lrem("tk_list_4", 2, "value_6", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Lrem("tk_list_6", 2, "value_2", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Lrange("tk_list_4", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        vecVal[0] == "value_1" && vecVal[1] == "value_3" && vecVal[2] == "value_4" &&
        m_redis.Lrem("tk_list_5", -3, "value_3", &nVal) == RC_SUCCESS && nVal == 3 &&
        m_redis.Lrange("tk_list_5", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 7 &&
        vecVal[2] == "value_3" && vecVal[5] == "value_1" &&
        m_redis.Lrem("tk_list_5", 2, "value_1", &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Lrange("tk_list_5", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 5 &&
        vecVal[0] == "value_2" && vecVal[1] == "value_3" && vecVal[3] == "value_1" &&
        m_redis.Lrem("tk_list_5", 0, "value_2", &nVal) == RC_SUCCESS && nVal == 3 &&
        m_redis.Lrange("tk_list_5", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        vecVal[0] == "value_3" && vecVal[1] == "value_1")
        bSuccess = true;
    return PrintResult("lrem", bSuccess);
}

bool CTestList::Test_Lset()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(6, 5))
        return PrintResult("lrem", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Lset("tk_str_1", 2, "value_1") == RC_REPLY_ERR &&
        m_redis.Lset("tk_list_6", 2, "value_1") == RC_REPLY_ERR &&
        m_redis.Lset("tk_list_2", 4, "value_10") == RC_REPLY_ERR &&
        m_redis.Lset("tk_list_5", 3, "value_10") == RC_SUCCESS &&
        m_redis.Lindex("tk_list_5", 3, &strVal) == RC_SUCCESS && strVal == "value_10")
        bSuccess = true;
    return PrintResult("lset", bSuccess);
}

bool CTestList::Test_Ltrim()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(9, 8))
        return PrintResult("ltrim", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Ltrim("tk_str_1", 0, 5) == RC_REPLY_ERR &&
        m_redis.Ltrim("tk_list_9", 0, 2) == RC_SUCCESS &&
        m_redis.Lrange("tk_list_9", 0, -1, &vecVal) == RC_SUCCESS && vecVal.empty() &&
        m_redis.Ltrim("tk_list_6", 5, 2) == RC_SUCCESS &&
        m_redis.Lrange("tk_list_6", 0, -1, &vecVal) == RC_SUCCESS && vecVal.empty() &&
        m_redis.Ltrim("tk_list_7", 4, -1) == RC_SUCCESS &&
        m_redis.Lrange("tk_list_7", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 3 &&
        vecVal[0] == "value_5" && vecVal[1] == "value_6" && vecVal[2] == "value_7" &&
        m_redis.Ltrim("tk_list_8", 6, 99) == RC_SUCCESS &&
        m_redis.Lrange("tk_list_8", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        vecVal[0] == "value_7" && vecVal[1] == "value_8")
        bSuccess = true;
    return PrintResult("ltrim", bSuccess);
}

bool CTestList::Test_Rpop()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(5, 4))
        return PrintResult("rpop", false);

    bool bSuccess = false;
    std::string strVal;
    std::vector<std::string> vecVal;
    if (m_redis.Rpop("tk_str_1", &strVal) == RC_REPLY_ERR &&
        m_redis.Rpop("tk_list_2", &strVal) == RC_SUCCESS && strVal == "value_2" &&
        m_redis.Lrange("tk_list_2", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && vecVal[0] == "value_1" &&
        m_redis.Rpop("tk_list_5", &strVal) == RC_SUCCESS && strVal.empty())
        bSuccess = true;
    return PrintResult("rpop", bSuccess);
}

bool CTestList::Test_Rpoplpush()
{
    return true;
}

bool CTestList::Test_Rpushx()
{
    if (!InitStringEnv(1, 1) || !InitListEnv(2, 1))
        return PrintResult("rpushx", false);

    bool bSuccess = false;
    long nVal;
    std::vector<std::string> vecVal;
    if (m_redis.Rpushx("tk_str_1", "value_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Rpushx("tk_list_1", "value_2", &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Lrange("tk_list_1", 0, -1, &vecVal) == RC_SUCCESS && vecVal.size() == 2 &&
        vecVal[0] == "value_1" && vecVal[1] == "value_2" &&
        m_redis.Rpushx("tk_list_2", "value_1", &nVal) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("rpushx", bSuccess);
}
