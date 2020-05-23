cmake_minimum_required(VERSION 3.5)

project(libgamepad)
set(CMAKE_CXX_FLAGS                " ${CMAKE_CXX_FLAGS_INIT} -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG          "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")

add_subdirectory(src)
add_subdirectory(tests)

include_directories("./include")

target_include_directories("gamepad" PUBLIC
    "./include"
    "./src")
