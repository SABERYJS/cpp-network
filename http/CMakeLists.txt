project(mthttp)
cmake_minimum_required(VERSION 3.10.2)


find_package(spdlog REQUIRED)
find_package(fmt)

foreach(target  testweb)
    if (${target} STREQUAL  "testweb")
        add_executable(${target}  
            ${target}.cc 
        )
    endif()
    target_include_directories(${target} PUBLIC  ${GSOURCE_DIR})
    target_link_libraries(${target} PRIVATE fmt::fmt)
    target_link_libraries(${target} PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
endforeach()