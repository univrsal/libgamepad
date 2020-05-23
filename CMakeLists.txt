cmake_minimum_required(VERSION 3.5)

option(ENABLE_TESTS "Compile test binary (default: ON)" ON)

project(libgamepad)
if (UNIX)
    set(CMAKE_CXX_FLAGS                " ${CMAKE_CXX_FLAGS_INIT} -fPIC")
    set(CMAKE_CXX_FLAGS_DEBUG          "-g -DDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")
endif()


add_subdirectory(src)

if (ENABLE_TESTS)
    add_subdirectory(tests)
endif()

include_directories("./include")

target_include_directories("gamepad" PUBLIC
    "./include"
    "./src")
