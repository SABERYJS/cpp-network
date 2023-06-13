project(mtwebsockets)
cmake_minimum_required(VERSION 3.10.2)


find_package(spdlog REQUIRED)
find_package(fmt)

include_directories(${GSOURCE_DIR}/algorithm)

foreach(target  testwebsocket)
    if (${target} STREQUAL  "testwebsocket")
        add_executable(${target}  
            ${target}.cc 
            HttpHeader.h
            HttpHeaderTypes.h
            HttpRequest.cc
            HttpRequest.h
            ${GSOURCE_DIR}/CommonUtils.h
            ${GSOURCE_DIR}/CommonUtils.cc
            ${GSOURCE_DIR}/TcpSession.h 
            ${GSOURCE_DIR}/TcpSession.cc  
            ${GSOURCE_DIR}/BaseTcpServer.h 
            ${GSOURCE_DIR}/BaseTcpServer.cc
            ${GSOURCE_DIR}/AsyncDispatcher.h
            ${GSOURCE_DIR}/AsyncDispatcher.cc
            ${GSOURCE_DIR}/DispatcherImpl.h
            ${GSOURCE_DIR}/DispatcherImpl.cc
            ${GSOURCE_DIR}/DynamicBytes.h
            ${GSOURCE_DIR}/DynamicBytes.cc
            ${GSOURCE_DIR}/ThreadsPool.h
            ${GSOURCE_DIR}/AsyncTimer.h
            WebsocketSession.h
            WebsocketSession.cc
            WebsocketServer.h
            WebsocketServer.cc
            WebsocketException.h
            ${GSOURCE_DIR}/algorithm/base64.h
            ${GSOURCE_DIR}/algorithm/base64.cpp
        )
        target_link_libraries(${target} PUBLIC ssl crypto)
    endif()
    target_include_directories(${target} PUBLIC  ${GSOURCE_DIR})
    target_link_libraries(${target} PRIVATE fmt::fmt)
    target_link_libraries(${target} PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
endforeach()