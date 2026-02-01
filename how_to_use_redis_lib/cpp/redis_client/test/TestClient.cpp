#include "TestClient.hpp"

CTestClient::CTestClient()
{
}

bool CTestClient::PrintResult(const std::string &strCmd, bool bSuccess)
{
    std::cout << "test " << strCmd << (bSuccess ? " success" : " failed") << std::endl;
    return bSuccess;
}

bool IsInContainer(const std::set<std::string> &setVal, const std::string &strVal)
{
    return setVal.find(strVal) != setVal.end();
}

bool CTestClient::InitStringEnv(int nDel, int nSet)
{
    int nRet = RC_SUCCESS;
    for (int i = 0; i < nDel && nRet >= 0; ++i)
    {
        std::stringstream ss;
        ss << "tk_str_" << i + 1;
        nRet = m_redis.Del(ss.str());
    }

    for (int i = 0; i < nSet && nRet >= 0; ++i)
    {
        std::stringstream ssKey, ssVal;
        ssKey << "tk_str_" << i + 1;
        ssVal << "value_" << i + 1;
        nRet = m_redis.Set(ssKey.str(), ssVal.str());
    }
    return nRet >= 0;
}

bool CTestClient::InitListEnv(int nDel, int nSet)
{
    int nRet = RC_SUCCESS;
    for (int i = 0; i < nDel && nRet >= 0; ++i)
    {
        std::stringstream ss;
        ss << "tk_list_" << i + 1;
        nRet = m_redis.Del(ss.str());
    }

    for (int i = 0; i < nSet && nRet >= 0; ++i)
    {
        std::stringstream ssKey;
        ssKey << "tk_list_" << i + 1;
        for (int j = 0; j < i + 1 && nRet >= 0; ++j)
        {
            std::stringstream ssVal;
            ssVal << "value_" << j + 1;
            nRet = m_redis.Rpush(ssKey.str(), ssVal.str());
        }
    }
    return nRet >= 0;
}

bool CTestClient::InitSetEnv(int nDel, int nSet)
{
    int nRet = RC_SUCCESS;
    for (int i = 0; i < nDel && nRet >= 0; ++i)
    {
        std::stringstream ss;
        ss << "tk_set_" << i + 1;
        nRet = m_redis.Del(ss.str());
    }

    for (int i = 0; i < nSet && nRet >= 0; ++i)
    {
        std::stringstream ssKey;
        ssKey << "tk_set_" << i + 1;
        for (int j = 0; j < i + 1 && nRet >= 0; ++j)
        {
            std::stringstream ssVal;
            ssVal << "value_" << j + 1;
            nRet = m_redis.Sadd(ssKey.str(), ssVal.str());
        }
    }
    return nRet >= 0;
}

bool CTestClient::InitZsetEnv(int nDel, int nSet)
{
    int nRet = RC_SUCCESS;
    for (int i = 0; i < nDel && nRet >= 0; ++i)
    {
        std::stringstream ss;
        ss << "tk_zset_" << i + 1;
        nRet = m_redis.Del(ss.str());
    }

    for (int i = 0; i < nSet && nRet >= 0; ++i)
    {
        std::stringstream ssKey;
        ssKey << "tk_zset_" << i + 1;
        for (int j = 0; j < i + 1 && nRet >= 0; ++j)
        {
            std::stringstream ssVal;
            ssVal << "value_" << j + 1;
            nRet = m_redis.Zadd(ssKey.str(), j + 1, ssVal.str());
        }
    }
    return nRet >= 0;
}

bool CTestClient::InitHashEnv(int nDel, int nSet)
{
    int nRet = RC_SUCCESS;
    for (int i = 0; i < nDel && nRet >= 0; ++i)
    {
        std::stringstream ss;
        ss << "tk_hash_" << i + 1;
        nRet = m_redis.Del(ss.str());
    }

    for (int i = 0; i < nSet && nRet >= 0; ++i)
    {
        std::stringstream ssKey;
        ssKey << "tk_hash_" << i + 1;
        for (int j = 0; j < i + 1 && nRet >= 0; ++j)
        {
            std::stringstream ssFld;
            std::stringstream ssVal;
            ssFld << "field_" << j + 1;
            ssVal << "value_" << j + 1;
            nRet = m_redis.Hset(ssKey.str(), ssFld.str(), ssVal.str());
        }
    }
    return nRet >= 0;
}

bool CTestClient::GetTime(struct timeval &tmVal)
{
    return m_redis.Time(&tmVal) == RC_SUCCESS;
}
