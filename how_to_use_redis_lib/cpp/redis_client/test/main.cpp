#include <stdio.h>
#include "TestBase.hpp"
#include "TestGeneric.hpp"
#include "TestString.hpp"
#include "TestList.hpp"
#include "TestSet.hpp"
#include "TestHash.hpp"
#include "TestZset.hpp"
#include "TestConcur.hpp"

int main(int argc, char **argv)
{
    std::string strHost = "10.10.252.15";
    //std::string strHost = "127.0.0.1";

    while (1)
    {
        CTestBase testBase;
        if (!testBase.StartTest(strHost))
            break;

        //CTestGeneric testKeys;
        //if (!testKeys.StartTest(strHost))
        //    break;

        CTestString testStr;
        if (!testStr.StartTest(strHost))
            break;

        //CTestList testList;
        //if (!testList.StartTest(strHost))
        //    break;

        //CTestSet testSet;
        //if (!testSet.StartTest(strHost))
        //    break;

        //CTestHash testHash;
        //if (!testHash.StartTest(strHost))
        //    break;

        CTestZset testZset;
        if (!testZset.StartTest(strHost))
            break;

        //CTestConcur testConcur;
        //if (!testConcur.StartTest(strHost))
        //    break;

        break;
    }
    return 0;
}

