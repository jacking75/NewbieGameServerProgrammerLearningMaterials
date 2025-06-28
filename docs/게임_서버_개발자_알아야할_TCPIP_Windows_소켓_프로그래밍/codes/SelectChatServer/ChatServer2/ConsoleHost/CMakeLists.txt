cmake_minimum_required(VERSION 3.9)
project(chatServer)

set(CMAKE_CXX_STANDARD 17)

set(Lib_LIBRARY_DIR ../bin)
set(Lib_LIBRARY_OUTPUT_DIR ../bin)

include_directories(
        ${Lib_LIBRARY_DIR}
)
link_directories(${Lib_LIBRARY_OUTPUT_DIR})
link_libraries(netLibSelect chatServerLib)

add_executable(chatServer main.cpp)
target_link_libraries( chatServer netLibSelect chatServerLib pthread)