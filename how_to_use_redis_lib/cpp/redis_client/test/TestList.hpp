#ifndef TEST_LIST_H
#define TEST_LIST_H

#include "TestClient.hpp"

class CTestList : public CTestClient
{
public:
    CTestList();
    bool StartTest(const std::string &strHost);

    bool Test_Blpop();
    bool Test_Brpop();
    bool Test_BRpoplpush();
    bool Test_Lindex();
    bool Test_Linsert();
    bool Test_Llen();
    bool Test_Lpop();
    bool Test_Lpushx();
    bool Test_Lrem();
    bool Test_Lset();
    bool Test_Ltrim();
    bool Test_Rpop();
    bool Test_Rpoplpush();
    bool Test_Rpushx();
};

#endif
