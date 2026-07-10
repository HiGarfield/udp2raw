#note: experimental
#      currently only used for generating `compile_commands.json` for clangd.
#      to build this project, it's suggested to use `makefile` instead

cmake_minimum_required(VERSION 3.7)
project(udp2raw)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 11)

# OpenSSL support option
option(USE_OPENSSL "Use OpenSSL for cryptographic operations" ON)

set(SOURCE_FILES
        main.cpp 
        lib/md5.cpp
        lib/pbkdf2-sha1.cpp
        lib/pbkdf2-sha256.cpp
        encrypt.cpp
        log.cpp
        network.cpp
        common.cpp
        connection.cpp
        misc.cpp
        fd_manager.cpp
        client.cpp
        server.cpp
        my_ev.cpp
)

if(USE_OPENSSL)
    find_package(OpenSSL REQUIRED)
    add_definitions(-DUSE_OPENSSL)
    list(APPEND SOURCE_FILES lib/openssl_wrapper.cpp)
    message(STATUS "OpenSSL support: enabled")
else()
    list(APPEND SOURCE_FILES 
        lib/aes_faster_c/aes.cpp
        lib/aes_faster_c/wrapper.cpp
    )
    message(STATUS "OpenSSL support: disabled, using built-in crypto")
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers -O2 -g -fsanitize=address,undefined")

add_executable(udp2raw ${SOURCE_FILES})
target_link_libraries(udp2raw rt)
target_link_libraries(udp2raw pthread)

if(USE_OPENSSL)
    target_link_libraries(udp2raw OpenSSL::SSL OpenSSL::Crypto)
endif()

include_directories(SYSTEM "libev")
include_directories(".")
