#ifndef TEST_ZSET_H
#define TEST_ZSET_H

#include "TestClient.hpp"

class CTestZset : public CTestClient
{
public:
    CTestZset();
    bool StartTest(const std::string &strHost);

    bool Test_Zcard();
    bool Test_Zcount();
    bool Test_Zincrby();
    bool Test_Zinterstore();
    bool Test_Zlexcount();
    bool Test_Zrangebylex();
    bool Test_Zrangebyscore();
    bool Test_Zrank();
    bool Test_Zrem();
    bool Test_Zremrangebylex();
    bool Test_Zremrangebyrank();
    bool Test_Zremrangebyscore();
    bool Test_Zrevrange();
    bool Test_Zrevrangebyscore();
    bool Test_Zrevrank();
    bool Test_Zscan();
    bool Test_Zscore();
    bool Test_Zunionstore();
    bool Test_Zrangewithscore();
};

#endif
