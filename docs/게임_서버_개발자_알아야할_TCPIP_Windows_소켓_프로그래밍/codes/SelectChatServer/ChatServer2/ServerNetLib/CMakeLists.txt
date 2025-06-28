cmake_minimum_required(VERSION 3.9)
project(netLibSelect)

set(CMAKE_CXX_STANDARD 17)

add_library(netLibSelect STATIC
        ILog.h Define.h ITcpNetwork.h ServerNetErrorCode.h
        ServerNetErrorCode.h TcpNetwork.cpp TcpNetwork.h
        )

# 빌드 잘 되면 아래 것을 주석 처리해본다
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY  ../../bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ../../bin )

set_target_properties( netLibSelect
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ../../bin
        ARCHIVE_OUTPUT_DIRECTORY ../../bin
       )