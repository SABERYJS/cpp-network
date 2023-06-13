project(demo1)
cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_CXX_STANDARD 14)

find_package(spdlog REQUIRED)
find_package(fmt)

link_directories("/usr/local/lib64/" "/usr/local/lib/")


file(GLOB PROTO_SOURCES protocompiled/*.h protocompiled/*.cc)
file(GLOB PROTO_SOURCES_DIR protocompiled)

OPTION(WITH_BUILD_TEST "build test program" OFF) # Enabled by default

foreach(target  server client ssl_server)
    if (${target} STREQUAL  "server")
        add_executable(${target}  
            ${target}.cc 
            TcpSession.h 
            TcpSession.cc  
            BaseTcpServer.h 
            BaseTcpServer.cc
            CommonUtils.h
            CommonUtils.cc
            AsyncDispatcher.h
            AsyncDispatcher.cc
            DispatcherImpl.h
            DispatcherImpl.cc
            DynamicBytes.h
            DynamicBytes.cc
            EchoSession.h
            EchoSession.cc
            EchoServer.h
            EchoServer.cc
            ThreadsPool.h
            AsyncTimer.h
        )
    elseif (${target} STREQUAL  "client")
        add_executable(${target}  
            ${target}.cc 
            BaseTcpClient.h
            BaseTcpClient.cc
            )
    elseif (${target} STREQUAL  "ssl_server")
        add_executable(${target}  
            ${target}.cc
            TcpSession.h 
            TcpSession.cc  
            BaseTcpServer.h 
            BaseTcpServer.cc
            CommonUtils.h
            CommonUtils.cc
            AsyncDispatcher.h
            AsyncDispatcher.cc
            DispatcherImpl.h
            DispatcherImpl.cc
            DynamicBytes.h
            DynamicBytes.cc
            ThreadsPool.h
            AsyncTimer.h
            SSLTcpServer.h
            SSLTcpServer.cc
            SSLTcpSession.h
            SSLTcpSession.cc
            SSLEchoServer.h
            SSLEchoSession.h
            )
        target_link_libraries(${target} PUBLIC ssl crypto)
    endif()
    target_link_libraries(${target} PRIVATE fmt::fmt rt pthread)
    target_link_libraries(${target} PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
endforeach()



add_custom_command(
                   OUTPUT ${CMAKE_SOURCE_DIR}/protocompiled/protocols.pb.h ${CMAKE_SOURCE_DIR}/protocompiled/protocols.pb.cc
                   COMMAND protoc -I ${CMAKE_SOURCE_DIR}/protosources/ --cpp_out=${CMAKE_SOURCE_DIR}/protocompiled ${CMAKE_SOURCE_DIR}/protosources/*.proto
                   DEPENDS ${CMAKE_SOURCE_DIR}/protosources/protocols.proto
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                   COMMENT "compile proto files")

add_custom_target(genprotofile 
                    DEPENDS ${CMAKE_SOURCE_DIR}/protocompiled/protocols.pb.h ${CMAKE_SOURCE_DIR}/protocompiled/protocols.pb.cc)

set(GSOURCE_DIR ${CMAKE_SOURCE_DIR})

if (WITH_BUILD_TEST)
    message(STATUS  "WITH_BUILD_TEST macro enabled")
    add_subdirectory(test)
endif()

add_subdirectory(http)
add_subdirectory(websocket)
add_subdirectory(chatserver)
add_subdirectory(rpc)
