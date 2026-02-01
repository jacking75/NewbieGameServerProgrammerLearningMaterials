#ifndef TEST_HASH_H
#define TEST_HASH_H

#include "TestClient.hpp"

class CTestHash : public CTestClient
{
public:
    CTestHash();
    bool StartTest(const std::string &strHost);

    bool Test_Hdel();
    bool Test_Hexists();
    bool Test_Hgetall();
    bool Test_Hincrby();
    bool Test_Hincrbyfloat();
    bool Test_Hkeys();
    bool Test_Hlen();
    bool Test_Hmget();
    bool Test_Hmset();
    bool Test_Hscan();
    bool Test_Hsetnx();
    bool Test_Hvals();
};

#endif
