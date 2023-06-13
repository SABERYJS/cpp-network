project(test)
cmake_minimum_required(VERSION 3.10.2)


find_package(spdlog REQUIRED)
find_package(fmt)
find_package(Protobuf REQUIRED)

foreach(target  testmulticlient simpletest prototest mysqltest pooltest mqtest)
    if (${target} STREQUAL  "testmulticlient")
        add_executable(${target}  
            ${target}.cc 
            ${GSOURCE_DIR}/BaseTcpClient.h
            ${GSOURCE_DIR}/BaseTcpClient.cc
        )
    elseif (${target} STREQUAL  "simpletest")
        add_executable(${target}  
            ${target}.cc
            )
    elseif (${target} STREQUAL  "prototest")
        add_executable(${target}  
                ${target}.cc
                ${PROTO_SOURCES}
                ${GSOURCE_DIR}/CommonUtils.h
                ${GSOURCE_DIR}/CommonUtils.cc
                ${GSOURCE_DIR}/DnsResolver.h
                ${GSOURCE_DIR}/DnsResolver.cc
            )
        target_link_libraries(${target} PUBLIC protobuf::libprotobuf jemalloc)
        add_dependencies(${target} genprotofile)
    elseif (${target} STREQUAL  "mysqltest")
        add_executable(${target}  
            ${target}.cc
            ${GSOURCE_DIR}/MysqlPool.h
            ${GSOURCE_DIR}/MysqlPool.cc
            )
        target_link_libraries(${target} PRIVATE mysqlcppconn) 
    elseif (${target} STREQUAL  "pooltest")
        add_executable(${target}  
            ${target}.cc
            ${GSOURCE_DIR}/MysqlPool.h
            ${GSOURCE_DIR}/MysqlPool.cc
            )
        target_link_libraries(${target} PRIVATE mysqlcppconn jemalloc)
    elseif (${target} STREQUAL  "mqtest")
        add_executable(${target}  
            ${target}.cc
            ${GSOURCE_DIR}/SafeMessageQueue.h
            )
        target_link_libraries(${target} PRIVATE  jemalloc) 
    endif()
    target_include_directories(${target} PUBLIC  ${GSOURCE_DIR} ${PROTO_SOURCES_DIR})
    target_link_libraries(${target} PRIVATE fmt::fmt rt pthread)
    target_link_libraries(${target} PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
endforeach()