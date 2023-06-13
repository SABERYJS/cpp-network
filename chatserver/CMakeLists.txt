project(chatserver)
cmake_minimum_required(VERSION 3.10.2)


find_package(spdlog REQUIRED)
find_package(fmt)
find_package(Protobuf REQUIRED)

foreach(target servermain chatclient)
    if (${target} STREQUAL  "servermain")
        add_executable(${target}  
            ${target}.cc 
            ${target}.cc 
            ${GSOURCE_DIR}/TcpSession.h 
            ${GSOURCE_DIR}/TcpSession.cc  
            ${GSOURCE_DIR}/BaseTcpServer.h 
            ${GSOURCE_DIR}/BaseTcpServer.cc
            ${GSOURCE_DIR}/CommonUtils.h
            ${GSOURCE_DIR}/CommonUtils.cc
            ${GSOURCE_DIR}/AsyncDispatcher.h
            ${GSOURCE_DIR}/AsyncDispatcher.cc
            ${GSOURCE_DIR}/DispatcherImpl.h
            ${GSOURCE_DIR}/DispatcherImpl.cc
            ${GSOURCE_DIR}/DynamicBytes.h
            ${GSOURCE_DIR}/DynamicBytes.cc
            ${GSOURCE_DIR}/ThreadsPool.h
            ${GSOURCE_DIR}/AsyncTimer.h
            ChatSession.h
            ChatSession.cc
            ChatServer.h
            ChatServer.cc
            ${GSOURCE_DIR}/ProtobufAssembler.h
            ${PROTO_SOURCES}
            ServerHooks.h
            ServerHooks.cc
        )

        add_dependencies(${target} genprotofile)

    elseif (${target} STREQUAL  "chatclient")
        add_executable(${target}  
            ${target}.cc 
            ${GSOURCE_DIR}/BaseTcpClient.h
            ${GSOURCE_DIR}/BaseTcpClient.cc
            ${GSOURCE_DIR}/ProtobufAssembler.h
            ${PROTO_SOURCES}
            ${GSOURCE_DIR}/CommonUtils.h
            ${GSOURCE_DIR}/CommonUtils.cc
            ${GSOURCE_DIR}/ProtobufAssembler.h
            ${GSOURCE_DIR}/DynamicBytes.h
            ${GSOURCE_DIR}/DynamicBytes.cc
        )
    endif()
    target_link_libraries(${target} PUBLIC protobuf::libprotobuf)
    target_include_directories(${target} PUBLIC  ${GSOURCE_DIR} ${PROTO_SOURCES_DIR})
    target_link_libraries(${target} PRIVATE fmt::fmt rt pthread)
    target_link_libraries(${target} PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
endforeach()