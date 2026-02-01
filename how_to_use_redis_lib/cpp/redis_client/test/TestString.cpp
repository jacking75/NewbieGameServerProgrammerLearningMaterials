#include "TestString.hpp"
#include "Util.h"

CTestString::CTestString()
{
}

bool CTestString::StartTest(const std::string &strHost)
{
    bool bSuccess = false;
    std::cout << "start to test string command" << std::endl;
    if (!m_redis.Initialize(strHost, 6379, 2, 10))
        std::cout << "init redis client failed" << std::endl;
    else
        bSuccess = Test_Append() && Test_Bitcount() && Test_Bitop() && Test_Bitpos() &&
                   Test_Decr() && Test_Decrby() && Test_Getbit() && Test_Getrange() &&
                   Test_Getset() && Test_Incr() && Test_Incrby() && Test_Incrbyfloat() &&
                   Test_Mget() && Test_Mset() && Test_Psetex() && Test_Setbit() &&
                   Test_Setex() && Test_Setnx() && Test_Setrange() && Test_Strlen();
    std::cout << std::endl;
    return bSuccess;
}

bool CTestString::Test_Append()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("append", false);

    bool bSuccess = false;
    std::string strVal;
    long nLen;
    if (m_redis.Append("tk_str_2", "new_1", &nLen) == RC_SUCCESS && nLen == 5 &&
        m_redis.Get("tk_str_2", &strVal) == RC_SUCCESS && strVal == "new_1" &&
        m_redis.Append("tk_str_1", "new_2", &nLen) == RC_SUCCESS && nLen == 12 &&
        m_redis.Get("tk_str_1", &strVal) == RC_SUCCESS && strVal == "value_1new_2")
        bSuccess = true;
    return PrintResult("append", bSuccess);
}

bool CTestString::Test_Bitcount()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("bitcount", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Bitcount("tk_str_2", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Bitcount("tk_list_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Bitcount("tk_str_1", &nVal) == RC_SUCCESS && nVal == 30 &&
        m_redis.Bitcount("tk_str_1", 0, 0, &nVal) == RC_SUCCESS && nVal == 5 &&
        m_redis.Bitcount("tk_str_1", 1, 3, &nVal) == RC_SUCCESS && nVal == 12 &&
        m_redis.Bitcount("tk_str_1", 2, -1, &nVal) == RC_SUCCESS && nVal == 22)
        bSuccess = true;
    return PrintResult("bitcount", bSuccess);
}

bool CTestString::Test_Bitop()
{
    if (!InitStringEnv(10, 0) || !InitListEnv(1, 1))
        return PrintResult("bitop", false);

    bool bSuccess = false;
    long nVal;
    std::string strVal;
    std::vector<std::string> v1 = {"tk_str_1", "tk_str_2"};
    std::vector<std::string> v2 = {"tk_str_1", "tk_str_2"};
    if (m_redis.Set("tk_str_1", "\x86") == RC_SUCCESS &&
        m_redis.Set("tk_str_2", "\x2b") == RC_SUCCESS &&
        m_redis.Set("tk_str_3", "\xae\xc8") == RC_SUCCESS &&
        m_redis.Bitop("tk_str_4", "AND", v1, &nVal) == RC_SUCCESS &&
        m_redis.Get("tk_str_4", &strVal) == RC_SUCCESS && nVal == 1 && strVal == "\x02" &&
        m_redis.Bitop("tk_str_5", "OR", v1, &nVal) == RC_SUCCESS &&
        m_redis.Get("tk_str_5", &strVal) == RC_SUCCESS && nVal == 1 && strVal == "\xaf")
        bSuccess = true;
    return PrintResult("bitop", bSuccess);
}

bool CTestString::Test_Bitpos()
{
    if (!InitStringEnv(3, 0) || !InitListEnv(1, 1))
        return PrintResult("bitpos", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Set("tk_str_1", "\xff\xff\x0f") == RC_SUCCESS &&
        m_redis.Set("tk_str_2", "\xff\xff") == RC_SUCCESS &&
        m_redis.Bitpos("tk_list_1", 0, &nVal) == RC_REPLY_ERR &&
        m_redis.Bitpos("tk_str_1", 1, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Bitpos("tk_str_1", 0, &nVal) == RC_SUCCESS && nVal == 16 &&
        m_redis.Bitpos("tk_str_2", 0, &nVal) == RC_SUCCESS && nVal == 16 &&
        m_redis.Bitpos("tk_str_3", 0, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Bitpos("tk_str_3", 1, &nVal) == RC_SUCCESS && nVal == -1 &&
        m_redis.Bitpos("tk_str_1", 1, 0, 1, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Bitpos("tk_str_1", 0, 0, 1, &nVal) == RC_SUCCESS && nVal == -1 &&
        m_redis.Bitpos("tk_str_1", 0, 1, 2, &nVal) == RC_SUCCESS && nVal == 16)
        bSuccess = true;
    return PrintResult("bitpos", bSuccess);
}

bool CTestString::Test_Decr()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("decr", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Decr("tk_str_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Decr("tk_list_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Decr("tk_str_2", &nVal) == RC_SUCCESS && nVal == -1 &&
        m_redis.Decr("tk_str_2", &nVal) == RC_SUCCESS && nVal == -2 &&
        m_redis.Set("tk_str_2", "3.4") == RC_SUCCESS &&
        m_redis.Decr("tk_str_2", &nVal) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("decr", bSuccess);
}

bool CTestString::Test_Decrby()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("decrby", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Decrby("tk_str_1", 4, &nVal) == RC_REPLY_ERR &&
        m_redis.Decrby("tk_list_1", 4, &nVal) == RC_REPLY_ERR &&
        m_redis.Decrby("tk_str_2", 4, &nVal) == RC_SUCCESS && nVal == -4 &&
        m_redis.Decrby("tk_str_2", 5, &nVal) == RC_SUCCESS && nVal == -9 &&
        m_redis.Set("tk_str_2", "3.4") == RC_SUCCESS &&
        m_redis.Decrby("tk_str_2", 4, &nVal) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("decrby", bSuccess);
}

bool CTestString::Test_Getbit()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("getbit", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Getbit("tk_str_2", 2, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Getbit("tk_list_1", 2, &nVal) == RC_REPLY_ERR &&
        m_redis.Getbit("tk_str_1", 1, &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Getbit("tk_str_1", 2, &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Getbit("tk_str_1", 3, &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Getbit("tk_str_1", 5, &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Getbit("tk_str_1", 6, &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Getbit("tk_str_1", 0, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Getbit("tk_str_1", 4, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Getbit("tk_str_1", 7, &nVal) == RC_SUCCESS && nVal == 0)
        bSuccess = true;
    return PrintResult("getbit", bSuccess);
}

bool CTestString::Test_Getrange()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("getrange", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Getrange("tk_str_2", 2, 5, &strVal) == RC_SUCCESS && strVal.empty() &&
        m_redis.Getrange("tk_list_1", 2, 5, &strVal) == RC_REPLY_ERR &&
        m_redis.Getrange("tk_str_1", 2, 5, &strVal) == RC_SUCCESS && strVal == "lue_" &&
        m_redis.Getrange("tk_str_1", -4, -1, &strVal) == RC_SUCCESS && strVal == "ue_1")
        bSuccess = true;
    return PrintResult("getrange", bSuccess);
}

bool CTestString::Test_Getset()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("getset", false);

    bool bSuccess = false;
    std::string strVal1 = "value_2";
    std::string strVal2 = "value_3";
    if (m_redis.Getset("tk_str_1", &strVal1) == RC_SUCCESS && strVal1 == "value_1" &&
        m_redis.Get("tk_str_1", &strVal1) == RC_SUCCESS && strVal1 == "value_2" &&
        m_redis.Getset("tk_str_2", &strVal2) == RC_SUCCESS && strVal2.empty() &&
        m_redis.Get("tk_str_2", &strVal2) == RC_SUCCESS && strVal2 == "value_3" &&
        m_redis.Getset("tk_list_1", &strVal1) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("getset", bSuccess);
}

bool CTestString::Test_Incr()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("incr", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Incr("tk_str_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Incr("tk_list_1", &nVal) == RC_REPLY_ERR &&
        m_redis.Incr("tk_str_2", &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Incr("tk_str_2", &nVal) == RC_SUCCESS && nVal == 2 &&
        m_redis.Set("tk_str_2", "3.4") == RC_SUCCESS &&
        m_redis.Incr("tk_str_2", &nVal) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("incr", bSuccess);
}

bool CTestString::Test_Incrby()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("incrby", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Incrby("tk_str_1", 4, &nVal) == RC_REPLY_ERR &&
        m_redis.Incrby("tk_list_1", 4, &nVal) == RC_REPLY_ERR &&
        m_redis.Incrby("tk_str_2", 4, &nVal) == RC_SUCCESS && nVal == 4 &&
        m_redis.Incrby("tk_str_2", 5, &nVal) == RC_SUCCESS && nVal == 9 &&
        m_redis.Set("tk_str_2", "3.4") == RC_SUCCESS &&
        m_redis.Incrby("tk_str_2", 4, &nVal) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("incrby", bSuccess);
}

bool CTestString::Test_Incrbyfloat()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("incrbyfloat", false);

    bool bSuccess = false;
    long nVal;
    double dVal;
    if (m_redis.Incrbyfloat("tk_str_1", 4.2, &dVal) == RC_REPLY_ERR &&
        m_redis.Incrbyfloat("tk_list_1", 4.2, &dVal) == RC_REPLY_ERR &&
        m_redis.Incrby("tk_str_2", 4, &nVal) == RC_SUCCESS && nVal == 4 &&
        m_redis.Incrbyfloat("tk_str_2", 5.2, &dVal) == RC_SUCCESS && std::abs(dVal - 9.2) < FLOAT_ZERO &&
        m_redis.Set("tk_str_2", "3.4") == RC_SUCCESS &&
        m_redis.Incrbyfloat("tk_str_2", 4.2, &dVal) == RC_SUCCESS && std::abs(dVal - 7.6) < FLOAT_ZERO)
        bSuccess = true;
    return PrintResult("incrbyfloat", bSuccess);
}

bool CTestString::Test_Mget()
{
    if (!InitStringEnv(10, 9) || !InitListEnv(1, 1))
        return PrintResult("mget", false);

    bool bSuccess = false;
    std::vector<std::string> vecKey;
    std::vector<std::string> vecVal;
    vecKey.push_back("tk_str_1");
    vecKey.push_back("tk_str_9");
    vecKey.push_back("tk_str_10");
    vecKey.push_back("tk_str_3");
    vecKey.push_back("tk_str_8");
    vecKey.push_back("tk_str_6");
    vecKey.push_back("tk_list_1");
    vecKey.push_back("tk_str_4");
    if (m_redis.Mget(vecKey, &vecVal) == RC_SUCCESS && vecKey.size() == vecVal.size() &&
        vecVal[2] == "" && vecVal[6] == "" && vecVal[0] == "value_1" && vecVal[3] == "value_3" &&
        vecVal[5] == "value_6")
        bSuccess = true;
    return PrintResult("mget", bSuccess);
}

bool CTestString::Test_Mset()
{
    if (!InitStringEnv(10, 0) || !InitListEnv(1, 1))
        return PrintResult("mset", false);

    bool bSuccess = false;
    std::vector<std::string> vecKey;
    std::vector<std::string> vecVal;
    std::string strVal;
    vecKey.push_back("tk_str_1");
    vecKey.push_back("tk_str_2");
    vecKey.push_back("tk_str_3");
    vecKey.push_back("tk_str_4");
    vecKey.push_back("tk_str_5");
    vecKey.push_back("tk_str_6");
    vecKey.push_back("tk_list_1");
    vecVal.push_back("value_1");
    vecVal.push_back("value_2");
    vecVal.push_back("value_3");
    vecVal.push_back("value_4");
    vecVal.push_back("value_5");
    vecVal.push_back("value_6");
    vecVal.push_back("value_7");
    if (m_redis.Mset(vecKey, vecVal) == RC_SUCCESS &&
        m_redis.Get("tk_str_2", &strVal) == RC_SUCCESS && strVal == "value_2" &&
        m_redis.Get("tk_str_4", &strVal) == RC_SUCCESS && strVal == "value_4" &&
        m_redis.Get("tk_list_1", &strVal) == RC_SUCCESS && strVal == "value_7")
        bSuccess = true;
    return PrintResult("mset", bSuccess);
}

bool CTestString::Test_Psetex()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("setex", false);

    bool bSuccess = false;
    std::string strVal;
    while (1)
    {
        if (m_redis.Psetex("tk_str_1", 4700, "new_1") != RC_SUCCESS)
            break;
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Get("tk_str_1", &strVal) != RC_SUCCESS || strVal != "new_1")
            break;
		Util::SystemAPI::Sleep(2000);
        if (m_redis.Get("tk_str_1", &strVal) != RC_SUCCESS || !strVal.empty())
            break;

        if (m_redis.Psetex("tk_str_2", 4800, "new_2") != RC_SUCCESS)
            break;
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Get("tk_str_2", &strVal) != RC_SUCCESS || strVal != "new_2")
            break;
		Util::SystemAPI::Sleep(2000);
        if (m_redis.Get("tk_str_2", &strVal) != RC_SUCCESS || !strVal.empty())
            break;

        if (m_redis.Psetex("tk_list_1", 4900, "new_3") != RC_SUCCESS)
            break;
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Get("tk_list_1", &strVal) != RC_SUCCESS || strVal != "new_3")
            break;
		Util::SystemAPI::Sleep(2000);
        if (m_redis.Get("tk_list_1", &strVal) != RC_SUCCESS || !strVal.empty())
            break;

        bSuccess = true;
        break;
    }
    return PrintResult("psetex", bSuccess);
}

bool CTestString::Test_Setbit()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("setbit", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Setbit("tk_list_1", 2, false) == RC_REPLY_ERR &&
        m_redis.Setbit("tk_str_1", 5, false) == RC_SUCCESS &&
        m_redis.Setbit("tk_str_1", 7, true) == RC_SUCCESS &&
        m_redis.Setbit("tk_str_2", 2, true) == RC_SUCCESS &&
        m_redis.Getbit("tk_str_1", 5, &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Getbit("tk_str_1", 7, &nVal) == RC_SUCCESS && nVal == 1 &&
        m_redis.Getbit("tk_str_2", 2, &nVal) == RC_SUCCESS && nVal == 1)
        bSuccess = true;
    return PrintResult("setbit", bSuccess);
}

bool CTestString::Test_Setex()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("setex", false);

    bool bSuccess = false;
    std::string strVal;
    while (1)
    {
        if (m_redis.Setex("tk_str_1", 5, "new_1") != RC_SUCCESS)
            break;
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Get("tk_str_1", &strVal) != RC_SUCCESS || strVal != "new_1")
            break;
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Get("tk_str_1", &strVal) != RC_SUCCESS || !strVal.empty())
            break;

        if (m_redis.Setex("tk_str_2", 5, "new_2") != RC_SUCCESS)
            break;
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Get("tk_str_2", &strVal) != RC_SUCCESS || strVal != "new_2")
            break;
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Get("tk_str_2", &strVal) != RC_SUCCESS || !strVal.empty())
            break;

        if (m_redis.Setex("tk_list_1", 5, "new_3") != RC_SUCCESS)
            break;
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Get("tk_list_1", &strVal) != RC_SUCCESS || strVal != "new_3")
            break;
		Util::SystemAPI::Sleep(3000);
        if (m_redis.Get("tk_list_1", &strVal) != RC_SUCCESS || !strVal.empty())
            break;

        bSuccess = true;
        break;
    }
    return PrintResult("setex", bSuccess);
}

bool CTestString::Test_Setnx()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("setnx", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Setnx("tk_str_1", "new_1") == RC_OBJ_EXIST &&
        m_redis.Setnx("tk_list_1", "new_3") == RC_OBJ_EXIST &&
        m_redis.Setnx("tk_str_2", "new_2") == RC_SUCCESS &&
        m_redis.Get("tk_str_2", &strVal) == RC_SUCCESS && strVal == "new_2")
        bSuccess = true;
    return PrintResult("setnx", bSuccess);
}

bool CTestString::Test_Setrange()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("setrange", false);

    bool bSuccess = false;
    std::string strVal;
    if (m_redis.Setrange("tk_str_1", 3, "nw") == RC_SUCCESS &&
        m_redis.Get("tk_str_1", &strVal) == RC_SUCCESS && strVal == "valnw_1" &&
        m_redis.Setrange("tk_str_1", 6, "new_1") == RC_SUCCESS &&
        m_redis.Get("tk_str_1", &strVal) == RC_SUCCESS && strVal == "valnw_new_1" &&
        m_redis.Setrange("tk_str_2", 2, "new_2") == RC_SUCCESS &&
        m_redis.Get("tk_str_2", &strVal) == RC_SUCCESS && strVal.size() == 7 &&
        strVal.data()[0] == 0 && strVal.data()[1] == 0 && strncmp(strVal.data() + 2, "new_2", 5) == 0 &&
        m_redis.Setrange("tk_list_1", 3, "new_3") == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("setrange", bSuccess);
}

bool CTestString::Test_Strlen()
{
    if (!InitStringEnv(2, 1) || !InitListEnv(1, 1))
        return PrintResult("strlen", false);

    bool bSuccess = false;
    long nVal;
    if (m_redis.Strlen("tk_str_1", &nVal) == RC_SUCCESS && nVal == 7 &&
        m_redis.Strlen("tk_str_2", &nVal) == RC_SUCCESS && nVal == 0 &&
        m_redis.Strlen("tk_list_1", &nVal) == RC_REPLY_ERR)
        bSuccess = true;
    return PrintResult("strlen", bSuccess);
}

