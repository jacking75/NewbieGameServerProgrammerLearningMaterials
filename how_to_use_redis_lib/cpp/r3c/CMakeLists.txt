# Writed by yijian (eyjian@qq.com)

# 2.8.11
cmake_minimum_required(VERSION 2.8.11)
project(r3c)

# 定义颜色值，message()时可用到
if (NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(ColourBold  "${Esc}[1m")
    set(Red         "${Esc}[31m")
    set(Green       "${Esc}[32m")
    set(Yellow      "${Esc}[33m")
    set(Blue        "${Esc}[34m")
endif ()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE "Debug")

add_definitions(-DSLEEP_USE_POLL)
#add_definitions(-DR3C_TEST)
add_definitions(-D__STDC_FORMAT_MACROS=1)
add_definitions(-D__STDC_CONSTANT_MACROS=1)
#add_definitions(-O2)
add_definitions(-g -ggdb -fPIC -Wall -W -Wwrite-strings -Wno-missing-field-initializers -fstrict-aliasing)

include(CheckCXXCompilerFlag)
# 如果编译环境支持C++20，则打开C++20开关
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
if (COMPILER_SUPPORTS_CXX20)
    message("${Yellow}enable C++20${ColourReset}")
    add_definitions("-std=c++20")
else ()
    message("${Red}C++20 not supported${ColourReset}")

    # 如果编译环境支持C++17，则打开C++17开关
    CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
    if (COMPILER_SUPPORTS_CXX17)
        message("${Yellow}C++17 enabled${ColourReset}")
        add_definitions("-std=c++17")
    else ()
        message("${Red}C++17 not supported${ColourReset}")

        # 如果编译环境支持C++14，则打开C++14开关
        CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
        if (COMPILER_SUPPORTS_CXX14)
            message("${Yellow}C++14 enabled${ColourReset}")
            add_definitions("-std=c++14")
        else ()
            message("${Red}C++14 not supported${ColourReset}")
            
            # 如果编译环境支持C++11，则打开C++11开关
            CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
            if (COMPILER_SUPPORTS_CXX11)
                message("${Yellow}C++11 enabled${ColourReset}")
                #add_definitions("-std=c++11")
                add_compile_options("-std=c++11")
            else ()
                message("${Red}C++11$ not supported{ColourReset}")
            endif()
        endif()
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
link_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (HIREDIS)
    set(HAVE_HIREDIS 1 CACHE INTERNAL HAVE_HIREDIS)
    include_directories(${HIREDIS}/include)
    link_directories(${HIREDIS}/lib)
else ()
    # hiredis.h
    find_path(
        HIREDIS_INCLUDE
        NAMES
        hiredis.h
        PATHS 
        /usr/local/hiredis/include/hiredis
        /usr/include
        /usr/local/include/hiredis/
        /usr/local/thirdparty/hiredis/include/hiredis
        ${CMAKE_INSTALL_PREFIX}/include/hiredis
        NO_DEFAULT_PATH
    )
    # libhiredis.a
    find_path(
        HIREDIS_LIB
        NAMES
        libhiredis.a
        PATHS 
        /usr/local/lib/
        /usr/local/hiredis/lib
        /usr/lib
        /usr/local/thirdparty/hiredis/lib
        ${CMAKE_INSTALL_PREFIX}/lib
        NO_DEFAULT_PATH
    )
        
    if ("${HIREDIS_INCLUDE}" STREQUAL "HIREDIS_INCLUDE-NOTFOUND" OR "${HIREDIS_LIB}" STREQUAL "HIREDIS_INCLUDE-NOTFOUND")
        message("${Red}`hiredis` not installed${ColourReset}")
        set(HAVE_HIREDIS 0 CACHE INTERNAL HAVE_HIREDIS)
        include_directories(${CMAKE_INSTALL_PREFIX}/include)
        link_directories(${CMAKE_INSTALL_PREFIX}/lib)
    else ()
        message("${Yellow}Found `hiredis` at ${HIREDIS_LIB} [libhiredis.a] and ${HIREDIS_INCLUDE}/include [hiredis.h] ${ColourReset}")
        set(HAVE_HIREDIS 1 CACHE INTERNAL HAVE_HIREDIS)
        include_directories(${HIREDIS_INCLUDE}/..)
        link_directories(${HIREDIS_LIB})
    endif ()
endif ()

# deps
if (NOT HAVE_HIREDIS)
    add_subdirectory(deps)
endif ()

# libr3c.a
add_library(
    r3c
    STATIC
    r3c.cpp 
    utils.cpp
    sha1.cpp
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
link_directories(${CMAKE_CURRENT_SOURCE_DIR})

# tests
add_subdirectory(tests)

# 安装（-DCMAKE_INSTALL_PREFIX）
install(
        TARGETS
        r3c
        DESTINATION lib
)
install(
        FILES r3c.h
        DESTINATION include/r3c
)
