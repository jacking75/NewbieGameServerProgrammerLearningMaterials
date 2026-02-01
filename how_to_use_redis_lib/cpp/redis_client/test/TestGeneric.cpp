#include "TestGeneric.hpp"
#include "Util.h"

CTestGeneric::CTestGeneric()
{
}

bool CTestGeneric::StartTest(const std::string &strHost)
{
    bool bSuccess = false;
    std::cout << "start to test generic command" << std::endl;
    if (!m_redis.Initialize(strHost, 6379, 2, 10))
        std::cout << "init redis client failed" << std::endl;
    else
        bSuccess = Test_Del() && Test_Exists() && Test_Expire() && Test_Expireat() &&
                   Test_Pexpire() && Test_Pexpireat() && Test_Persist() && Test_Pttl() &&
                   Test_Ttl() && Test_Dump() && Test_Rename() && Test_Renamenx() &&
                   Test_Type() && Test_Keys() && Test_Scan() && Test_Randomkey();

    std::cout << std::endl;
    return bSuccess;
}

bool CTestGeneric::Test_Del()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("del", false);

    bool bSuccess = false;
    long nDelVal;
    long nExtVal;
    if (m_redis.Del("tk_str_1", &nDelVal) == RC_SUCCESS && nDelVal == 1 &&
        m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 0 &&
        m_redis.Del("tk_str_2", &nDelVal) == RC_SUCCESS && nDelVal == 0 &&
        m_redis.Del("tk_list_1", &nDelVal) == RC_SUCCESS && nDelVal == 1 &&
        m_redis.Exists("tk_list_1", &nExtVal) == RC_SUCCESS && nExtVal == 0)
        bSuccess = true;
    return PrintResult("del", bSuccess);
}

bool CTestGeneric::Test_Dump()
{
    if (!InitStringEnv(4, 2) || !InitListEnv(3, 2))
        return PrintResult("dump", false);

    bool bSuccess = false;
    long nVal;
    std::string strDump;
    std::string strVal;
    std::vector<std::string> vecVal;
    if (m_redis.Dump("tk_str_3", &strDump) == RC_SUCCESS && strDump.empty() &&
        m_redis.Dump("tk_str_1", &strDump) == RC_SUCCESS && !strDump.empty() &&
        m_redis.Restore("tk_str_2", 0, strDump) == RC_REPLY_ERR &&
        m_redis.Restore("tk_str_4", 0, strDump) == RC_SUCCESS &&
        m_redis.Get("tk_str_4", &strVal) == RC_SUCCESS && strVal == "value_1" &&
        m_redis.Dump("tk_list_2", &strDump) == RC_SUCCESS && !strDump.empty() &&
        m_redis.Restore("tk_list_3", 10000, strDump) == RC_SUCCESS &&
        m_redis.Lrange("tk_list_3", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 2 && vecVal[0] == "value_1" && vecVal[1] == "value_2" &&
        m_redis.Ttl("tk_list_3", &nVal) == RC_SUCCESS && nVal >=8)
        bSuccess = true;

    PrintResult("dump", bSuccess);
    return PrintResult("restore", bSuccess);
}

bool CTestGeneric::Test_Exists()
{
    if (!InitStringEnv(2, 1))
        return PrintResult("exists", false);

    bool bSuccess = false;
    long nVal;
    if ((m_redis.Exists("tk_str_1", &nVal) == RC_SUCCESS && nVal == 1) &&
        (m_redis.Exists("tk_str_2", &nVal) == RC_SUCCESS && nVal == 0))
        bSuccess = true;

    return PrintResult("exists", bSuccess);
}

bool CTestGeneric::Test_Expire()
{
    if (!InitStringEnv(2, 1))
        return PrintResult("expire", false);

    bool bSuccess = false;
    long nExtVal;
    long nExpVal;
    if (m_redis.Expire("tk_str_2", 5, &nExpVal) == RC_SUCCESS && nExpVal == 0 &&
        m_redis.Expire("tk_str_1", 5, &nExpVal) == RC_SUCCESS && nExpVal == 1)
    {
        Util::SystemAPI::Sleep(3000);
        if (m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 1)
        {
			Util::SystemAPI::Sleep(3000);
            if (m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 0)
                bSuccess = true;
        }
    }
    return PrintResult("expire", bSuccess);
}

bool CTestGeneric::Test_Expireat()
{
    struct timeval tmVal;
    if (!InitStringEnv(2, 1) || !GetTime(tmVal))
        return PrintResult("expireat", false);

    bool bSuccess = false;
    long nTime = tmVal.tv_sec + 5;
    long nExpVal;
    long nExtVal;
    if (m_redis.Expireat("tk_str_2", nTime, &nExpVal) == RC_SUCCESS && nExpVal == 0 &&
        m_redis.Expireat("tk_str_1", nTime, &nExpVal) == RC_SUCCESS && nExpVal == 1)
    {
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 1)
        {
			Util::SystemAPI::Sleep(3000);
            if (m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 0)
                bSuccess = true;
        }
    }
    return PrintResult("expireat", bSuccess);
}

bool CTestGeneric::Test_Keys()
{
    int nRet = RC_SUCCESS;
    int i = 0;
    for (; i < 1000 && nRet == RC_SUCCESS; i++)
    {
        std::stringstream ss;
        ss << "tk_str_shawn_" << i;
        nRet = m_redis.Set(ss.str(), "value");
    }
    if (nRet != RC_SUCCESS)
        return PrintResult("keys", false);

    bool bSuccess = false;
    std::vector<std::string> vecVal;
    if (m_redis.Keys("tk_str_shawn_2?2", &vecVal) == RC_SUCCESS && vecVal.size() == 10)
    {
        bSuccess = true;
        for (i = 0; i < 10 && bSuccess; ++i)
        {
            std::stringstream ss;
            ss << "tk_str_shawn_2" << i << "2";
            if (std::find(vecVal.begin(), vecVal.end(), ss.str()) == vecVal.end())
                bSuccess = false;
        }
    }
    return PrintResult("keys", bSuccess);
}

bool CTestGeneric::Test_Persist()
{
    if (!InitStringEnv(2, 1))
        return PrintResult("persist", false);

    bool bSuccess = false;
    long nPerVal;
    long nExtVal;
    if (m_redis.Persist("tk_str_2", &nPerVal) == RC_SUCCESS && nPerVal == 0 &&
        m_redis.Expire("tk_str_1", 5) == RC_SUCCESS)
    {
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Persist("tk_str_1", &nPerVal) == RC_SUCCESS && nPerVal == 1)
        {
			Util::SystemAPI::Sleep(3000);
            if (m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 1)
                bSuccess = true;
        }
    }
    return PrintResult("persist", bSuccess);
}

bool CTestGeneric::Test_Pexpire()
{
    if (!InitStringEnv(2, 1))
        return PrintResult("pexpire", false);

    bool bSuccess = false;
    long nExpVal;
    long nExtVal;
    if (m_redis.Pexpire("tk_str_2", 5000, &nExpVal) == RC_SUCCESS && nExpVal == 0 &&
        m_redis.Pexpire("tk_str_1", 5000, &nExpVal) == RC_SUCCESS && nExpVal == 1)
    {
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 1)
        {
			Util::SystemAPI::Sleep(3000);
            if (m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 0)
                bSuccess = true;
        }
    }
    return PrintResult("pexpire", bSuccess);
}

bool CTestGeneric::Test_Pexpireat()
{
    struct timeval tmVal;
    if (!InitStringEnv(2, 1) || !GetTime(tmVal))
        return PrintResult("pexpireat", false);

    bool bSuccess = false;
    long nTime = tmVal.tv_sec * 1000 + 4000;
    long nExtVal;
    long nExpVal;
    if (m_redis.Pexpireat("tk_str_2", nTime, &nExpVal) == RC_SUCCESS && nExpVal == 0 &&
        m_redis.Pexpireat("tk_str_1", nTime, &nExpVal) == RC_SUCCESS && nExpVal == 1)
    {
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 1)
        {
			Util::SystemAPI::Sleep(2000);
            if (m_redis.Exists("tk_str_1", &nExtVal) == RC_SUCCESS && nExtVal == 0)
                bSuccess = true;
        }
    }
    return PrintResult("pexpireat", bSuccess);
}

bool CTestGeneric::Test_Pttl()
{
    if (!InitStringEnv(2, 1))
        return PrintResult("pttl", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Pttl("tk_str_2", &nVal) == RC_SUCCESS && nVal == -2 &&
        m_redis.Pttl("tk_str_1", &nVal) == RC_SUCCESS && nVal == -1 &&
        m_redis.Pexpire("tk_str_1", 7800) == RC_SUCCESS)
    {
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Pttl("tk_str_1", &nVal) == RC_SUCCESS && nVal > 4000 && nVal < 4800)
        {
			Util::SystemAPI::Sleep(2000);
            if (m_redis.Pttl("tk_str_1", &nVal) == RC_SUCCESS && nVal > 2000 && nVal < 2800)
                bSuccess = true;
        }
    }
    return PrintResult("pttl", bSuccess);
}

bool CTestGeneric::Test_Randomkey()
{
    if (!InitStringEnv(4, 4))
        return PrintResult("randomkey", false);

    bool bSuccess = false;
    std::string strVal;
    long nVal;
    if (m_redis.Randomkey(&strVal) == RC_SUCCESS &&
        m_redis.Exists(strVal, &nVal) == RC_SUCCESS && nVal == 1)
        bSuccess = true;
    return PrintResult("randomkey", bSuccess);
}

bool CTestGeneric::Test_Rename()
{
    if (!InitStringEnv(4, 2) || !InitListEnv(2, 2))
        return PrintResult("Rename", false);

    bool bSuccess = false;
    long nVal;
    std::string strVal;
    std::vector<std::string> vecVal;
    if (m_redis.Rename("tk_str_3", "tk_str_4") == RC_REPLY_ERR &&
        m_redis.Rename("tk_str_1", "tk_str_3") == RC_SUCCESS &&
        m_redis.Exists("tk_str_1", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Get("tk_str_3", &strVal) == RC_SUCCESS && strVal == "value_1" &&
        m_redis.Rename("tk_str_2", "tk_str_3") == RC_SUCCESS &&
        m_redis.Get("tk_str_3", &strVal) == RC_SUCCESS && strVal == "value_2" &&
        m_redis.Rename("tk_list_2", "tk_str_3") == RC_SUCCESS &&
        m_redis.Exists("tk_list_2", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Lrange("tk_str_3", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 2 && vecVal[0] == "value_1" && vecVal[1] == "value_2" &&
        m_redis.Expire("tk_list_1", 2) == RC_SUCCESS &&
        m_redis.Rename("tk_list_1", "tk_list_2") == RC_SUCCESS &&
        m_redis.Exists("tk_list_1", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Lrange("tk_list_2", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && vecVal[0] == "value_1")
    {
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Exists("tk_list_2", &nVal) ==  RC_SUCCESS && nVal == 0)
            bSuccess = true;
    }
    return PrintResult("Rename", bSuccess);
}

bool CTestGeneric::Test_Renamenx()
{
    if (!InitStringEnv(4, 2) || !InitListEnv(2, 1))
        return PrintResult("Renamenx", false);

    bool bSuccess = true;
    long nVal;
    std::string strVal;
    std::vector<std::string> vecVal;
    if (m_redis.Renamenx("tk_str_3", "tk_str_4") == RC_REPLY_ERR &&
        m_redis.Renamenx("tk_str_1", "tk_str_3") == RC_SUCCESS &&
        m_redis.Exists("tk_str_1", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Get("tk_str_3", &strVal) == RC_SUCCESS && strVal == "value_1" &&
        m_redis.Renamenx("tk_str_2", "tk_str_3") == RC_OBJ_EXIST &&
        m_redis.Exists("tk_str_2", &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Renamenx("tk_list_1", "tk_str_3") == RC_OBJ_EXIST &&
        m_redis.Exists("tk_list_1", &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Expire("tk_list_1", 2) == RC_SUCCESS &&
        m_redis.Renamenx("tk_list_1", "tk_list_2") == RC_SUCCESS &&
        m_redis.Exists("tk_list_1", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Lrange("tk_list_2", 0, -1, &vecVal) == RC_SUCCESS &&
        vecVal.size() == 1 && vecVal[0] == "value_1")
    {
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Exists("tk_list_2", &nVal) ==  RC_SUCCESS && nVal == 0)
            bSuccess = true;
    }
    return PrintResult("Renamenx", bSuccess);
}

bool CTestGeneric::Test_Scan()
{
    return true;
}

bool CTestGeneric::Test_Ttl()
{
    if (!InitStringEnv(2, 1))
        return PrintResult("Ttl", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Ttl("tk_str_2", &nVal) == RC_SUCCESS && nVal == -2 &&
        m_redis.Ttl("tk_str_1", &nVal) == RC_SUCCESS && nVal == -1 &&
        m_redis.Expire("tk_str_1", 100) == RC_SUCCESS)
    {
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Ttl("tk_str_1", &nVal) == RC_SUCCESS && nVal > 95 && nVal < 98)
        {
			Util::SystemAPI::Sleep(5000);
            if (m_redis.Ttl("tk_str_1", &nVal) == RC_SUCCESS && nVal > 91 && nVal < 94)
                bSuccess = true;
        }
    }
    return PrintResult("Ttl", bSuccess);
}

bool CTestGeneric::Test_Type()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1) || !InitSetEnv(1, 1) ||
        !InitZsetEnv(1, 1) || !InitHashEnv(1, 1))
        return PrintResult("type", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Type("tk_str_2", &strVal) == RC_SUCCESS && strVal == "none" &&
        m_redis.Type("tk_str_1", &strVal) == RC_SUCCESS && strVal == "string" &&
        m_redis.Type("tk_list_1", &strVal) == RC_SUCCESS && strVal == "list" &&
        m_redis.Type("tk_set_1", &strVal) == RC_SUCCESS && strVal == "set" &&
        m_redis.Type("tk_zset_1", &strVal) == RC_SUCCESS && strVal == "zset" &&
        m_redis.Type("tk_hash_1", &strVal) == RC_SUCCESS && strVal == "hash")
        bSuccess = true;
    return PrintResult("type", bSuccess);
}
