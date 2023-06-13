project(rpc)
cmake_minimum_required(VERSION 3.10.2)

find_package(spdlog REQUIRED)
find_package(fmt)
find_package(Protobuf REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)


OPTION(WITH_SYSTEMD_ENABLED "compile with systemd" OFF) # Enabled by default

# daemon or systemd
if (WITH_SYSTEMD_ENABLED)
    message(STATUS  "WITH_SYSTEMD_ENABLED macro enabled")
    add_compile_definitions(WITH_SYSTEMD_ENABLED)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/install_service.sh ${CMAKE_CURRENT_SOURCE_DIR}/rpcserver.service  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

foreach(target  parser rpcserver rpcclient)
    if (${target} STREQUAL "rpcserver")
        add_executable(${target}  
            ${target}.cc
            RpcService.h
            RpcService.cc
            RpcSession.h
            RpcSession.cc
            RpcServer.h
            RpcServer.cc
            RpcServiceClient.h
            RpcServiceClient.cc
            HandlerProvider.h
            RpcMethodHandler.h
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
            NormalService.h
            NormalService.cc
            ${PROTO_SOURCES}
            ${GSOURCE_DIR}/BaseTcpClient.h
            ${GSOURCE_DIR}/BaseTcpClient.cc
            NormalServiceImpl.h
            AppConfig.h
            AppConfig.cc
            ${GSOURCE_DIR}/CharArray.h
        )
    elseif (${target} STREQUAL  "parser")
        add_executable(${target}  
            ${target}.cc
            CppCodeGenerator.h
            CppCodeGenerator.cc
            TokensParser.h
            TokensParser.cc
        )
    elseif (${target} STREQUAL  "rpcclient")
        add_executable(${target}  
            ${target}.cc
            RpcServiceClient.h
            RpcServiceClient.cc
            ${GSOURCE_DIR}/CommonUtils.h
            ${GSOURCE_DIR}/CommonUtils.cc
            ${GSOURCE_DIR}/DynamicBytes.h
            ${GSOURCE_DIR}/DynamicBytes.cc
            ${PROTO_SOURCES}
            ${GSOURCE_DIR}/BaseTcpClient.h
            ${GSOURCE_DIR}/BaseTcpClient.cc
            NormalServiceClient.h
        )
    endif()
    target_link_libraries(${target} PUBLIC protobuf::libprotobuf)
    target_include_directories(${target} PUBLIC  ${GSOURCE_DIR} ${PROTO_SOURCES_DIR})
    target_link_libraries(${target} PRIVATE fmt::fmt rt pthread nlohmann_json::nlohmann_json)
    target_link_libraries(${target} PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
endforeach()