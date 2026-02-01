#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

#include "../src/RedisClient.hpp"
#include <iostream>

#ifdef _MSC_VER
#else
#include <unistd.h>
#endif

#include <time.h>
#include <cmath>
#include <algorithm>

#define FLOAT_ZERO 0.000001

template <typename T>
bool IsInContainer(const T &container, const typename T::value_type &value)
{
    return std::find(container.begin(), container.end(), value) != container.end();
}

template <typename K, typename V>
bool IsPairInMap(const K &key, const V &val, const std::map<K, V> &mapVal)
{
    auto iter = mapVal.find(key);
    if (iter == mapVal.end() || iter->second != val)
        return false;
    return true;
}

template <> bool IsInContainer(const std::set<std::string> &setVal, const std::string &strVal);

class CTestClient
{
public:
    CTestClient();

protected:
    static bool PrintResult(const std::string &strCmd, bool bSuccess);
    bool InitStringEnv(int nDel, int nSet);
    bool InitListEnv(int nDel, int nSet);
    bool InitSetEnv(int nDel, int nSet);
    bool InitZsetEnv(int nDel, int nSet);
    bool InitHashEnv(int nDel, int nSet);
    bool GetTime(struct timeval &tmVal);

protected:
    CRedisClient m_redis;
};

#endif
