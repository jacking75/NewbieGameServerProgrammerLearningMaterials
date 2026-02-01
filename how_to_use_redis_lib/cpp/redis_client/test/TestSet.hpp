#ifndef TEST_SET_H
#define TEST_SET_H

#include "TestClient.hpp"

class CTestSet : public CTestClient
{
public:
    CTestSet();
    bool StartTest(const std::string &strHost);

    bool Test_Scard();
    bool Test_Sdiff();
    bool Test_Sdiffstore();
    bool Test_Sinter();
    bool Test_Sinterstore();
    bool Test_Sismember();
    bool Test_Smove();
    bool Test_Spop();
    bool Test_Srandmember();
    bool Test_Srem();
    bool Test_Sscan();
    bool Test_Sunion();
    bool Test_Sunionstore();
};

#endif
