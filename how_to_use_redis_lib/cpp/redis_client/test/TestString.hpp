#ifndef TEST_STRING_H
#define TEST_STRING_H

#include "TestClient.hpp"

class CTestString : public CTestClient
{
public:
    CTestString();
    bool StartTest(const std::string &strHost);

private:
    bool Test_Append();
    bool Test_Bitcount();
    bool Test_Bitop();
    bool Test_Bitpos();
    bool Test_Decr();
    bool Test_Decrby();
    bool Test_Getbit();
    bool Test_Getrange();
    bool Test_Getset();
    bool Test_Incr();
    bool Test_Incrby();
    bool Test_Incrbyfloat();
    bool Test_Mget();
    bool Test_Mset();
    bool Test_Psetex();
    bool Test_Setbit();
    bool Test_Setex();
    bool Test_Setnx();
    bool Test_Setrange();
    bool Test_Strlen();
};

#endif
