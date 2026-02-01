#ifndef TEST_GENERIC_H
#define TEST_GENERIC_H

#include "TestClient.hpp"

class CTestGeneric : public CTestClient
{
public:
    CTestGeneric();
    bool StartTest(const std::string &strHost);

private:
    bool Test_Del();
    bool Test_Dump();
    bool Test_Exists();
    bool Test_Expire();
    bool Test_Expireat();
    bool Test_Keys();
    bool Test_Persist();
    bool Test_Pexpire();
    bool Test_Pexpireat();
    bool Test_Pttl();
    bool Test_Randomkey();
    bool Test_Rename();
    bool Test_Renamenx();
    bool Test_Scan();
    bool Test_Ttl();
    bool Test_Type();
};

#endif
