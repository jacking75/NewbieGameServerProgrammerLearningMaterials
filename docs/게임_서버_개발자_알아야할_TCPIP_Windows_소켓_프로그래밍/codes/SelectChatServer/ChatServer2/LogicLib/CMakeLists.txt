cmake_minimum_required(VERSION 3.9)
project(chatServerLib)

set(CMAKE_CXX_STANDARD 17)

set( NET_LIB_INCLUDE_DIR ../ServerNetLib )
set( NET_LIBRARY_DIR ../../bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY  ../../bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ../../bin )

include_directories( ${NET_LIB_INCLUDE_DIR} )
link_directories( ${NET_LIBRARY_DIR} )

# https://stackoverflow.com/questions/39598323/how-to-properly-link-libraries-with-cmake
# target_link_libraries(chatServerLib libnetLibSelect.a)

add_library(chatServerLib STATIC
        Main.h
        Main.cpp
        ConsoleLogger.h 
        ConnectedUserManager.h
        User.h 
        UserManager.h 
        UserManager.cpp 
        PacketProcess.h 
        PacketProcess.cpp 
        PacketProcessLogin.cpp 
        PacketProcessLobby.cpp 
        PacketProcessRoom.cpp 
        Room.h 
        Room.cpp
        LobbyManager.h 
        LobbyManager.cpp 
        Lobby.h
        Lobby.cpp
        Game.h
        Game.cpp
        )



set_target_properties( chatServerLib
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ../../bin
        ARCHIVE_OUTPUT_DIRECTORY ../../bin
       )