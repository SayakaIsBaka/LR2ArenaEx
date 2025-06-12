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

set(IXWEBSOCKET_SOURCES
    ../include/ixwebsocket/IXBench.cpp
    ../include/ixwebsocket/IXCancellationRequest.cpp
    ../include/ixwebsocket/IXConnectionState.cpp
    ../include/ixwebsocket/IXDNSLookup.cpp
    ../include/ixwebsocket/IXExponentialBackoff.cpp
    ../include/ixwebsocket/IXGetFreePort.cpp
    ../include/ixwebsocket/IXGzipCodec.cpp
    ../include/ixwebsocket/IXHttp.cpp
    ../include/ixwebsocket/IXHttpClient.cpp
    ../include/ixwebsocket/IXHttpServer.cpp
    ../include/ixwebsocket/IXNetSystem.cpp
    ../include/ixwebsocket/IXSelectInterrupt.cpp
    ../include/ixwebsocket/IXSelectInterruptFactory.cpp
    ../include/ixwebsocket/IXSelectInterruptPipe.cpp
    ../include/ixwebsocket/IXSelectInterruptEvent.cpp
    ../include/ixwebsocket/IXSetThreadName.cpp
    ../include/ixwebsocket/IXSocket.cpp
    ../include/ixwebsocket/IXSocketConnect.cpp
    ../include/ixwebsocket/IXSocketFactory.cpp
    ../include/ixwebsocket/IXSocketServer.cpp
    ../include/ixwebsocket/IXSocketTLSOptions.cpp
    ../include/ixwebsocket/IXStrCaseCompare.cpp
    ../include/ixwebsocket/IXUdpSocket.cpp
    ../include/ixwebsocket/IXUrlParser.cpp
    ../include/ixwebsocket/IXUuid.cpp
    ../include/ixwebsocket/IXUserAgent.cpp
    ../include/ixwebsocket/IXWebSocket.cpp
    ../include/ixwebsocket/IXWebSocketCloseConstants.cpp
    ../include/ixwebsocket/IXWebSocketHandshake.cpp
    ../include/ixwebsocket/IXWebSocketHttpHeaders.cpp
    ../include/ixwebsocket/IXWebSocketPerMessageDeflate.cpp
    ../include/ixwebsocket/IXWebSocketPerMessageDeflateCodec.cpp
    ../include/ixwebsocket/IXWebSocketPerMessageDeflateOptions.cpp
    ../include/ixwebsocket/IXWebSocketProxyServer.cpp
    ../include/ixwebsocket/IXWebSocketServer.cpp
    ../include/ixwebsocket/IXWebSocketTransport.cpp
)

add_executable(lr2arenaex-server server.cpp ../src/server/server.cpp ${IXWEBSOCKET_SOURCES})
if (WIN32)
    target_compile_definitions(lr2arenaex-server PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_compile_definitions(lr2arenaex-server PUBLIC MSGPACK_NO_BOOST)
target_include_directories(lr2arenaex-server PRIVATE ../src/ ../include/)
target_link_libraries(lr2arenaex-server argparse)
if(WIN32)
    target_link_libraries(lr2arenaex-server ws2_32)
endif()