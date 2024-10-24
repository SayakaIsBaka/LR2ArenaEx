cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

PROJECT(lr2arenaex-server)

# fetch latest argparse
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

add_executable(lr2arenaex-server server.cpp ../src/server/server.cpp ../include/Garnet/Garnet.cpp)
target_include_directories(lr2arenaex-server PRIVATE ../src/ ../include/)
target_link_libraries(lr2arenaex-server argparse)
if(WIN32)
    target_link_libraries(lr2arenaex-server ws2_32)
endif()