cmake_minimum_required(VERSION 3.5)

option(GAMEPAD_ENABLE_TESTS "Compile test binary (default: ON)" ON)
option(GAMEPAD_ENABLE_STATIC "Static library (default: OFF)" ON)
option(GAMEPAD_ENABLE_SHARED "Shared library (default: ON)" OFF)
option(GAMEPAD_ENABLE_JSON "Provide interface to load and save with json11 (default: ON)" ON)
option(GAMEPAD_ENABLE_INSTALL "Register files for install target (default: ON)" ON)

project(libgamepad VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_library(gamepad OBJECT
    ./src/hook.cpp
    ./src/log.cpp
    ./src/binding.cpp
    ./src/binding-default.cpp
    ./src/json11.cpp
    ./src/device.cpp
    )

set_property(TARGET gamepad PROPERTY POSITION_INDEPENDENT_CODE 1)

set(LGP_ENABLE_JSON OFF)
if (GAMEPAD_ENABLE_JSON)
    set(LGP_ENABLE_JSON ON)
endif()

target_include_directories(gamepad
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (GAMEPAD_ENABLE_INSTALL)
    include(GNUInstallDirs)
endif()

if (UNIX AND NOT APPLE)
    target_sources(gamepad PRIVATE
        ./src/linux/hook-linux.cpp
        ./src/linux/device-linux.cpp
        ./src/linux/device-linux.hpp
        ./src/linux/binding-linux.cpp
        )
elseif (APPLE)
    target_sources(gamepad PRIVATE
        ./src/darwin/hook-darwin.cpp
        )
elseif(WIN32)
    target_sources(gamepad PRIVATE
        ./src/windows/hook-xinput.cpp
        ./src/windows/hook-dinput.cpp
        ./src/windows/binding-dinput.cpp
        ./src/windows/binding-xinput.cpp
        ./src/windows/device-dinput.cpp
        ./src/windows/device-dinput.hpp
        ./src/windows/device-xinput.cpp
        ./src/windows/device-xinput.hpp
        )
    add_compile_definitions(UNICODE _UNICODE)
    target_link_libraries(gamepad
        dinput8
        dxguid)
endif()

if (GAMEPAD_ENABLE_SHARED)
    add_library(gamepad_shared SHARED $<TARGET_OBJECTS:gamepad>)
    set_target_properties(gamepad_shared PROPERTIES OUTPUT_NAME "gamepad")

    if (UNIX)
        target_link_libraries (gamepad_shared ${CMAKE_THREAD_LIBS_INIT})
    elseif (MSVC)
        target_link_libraries(gamepad_shared
            dinput8
            dxguid)
    endif()

    if (GAMEPAD_ENABLE_INSTALL)
        install(TARGETS gamepad_shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
endif()

if (GAMEPAD_ENABLE_STATIC)
    add_library(gamepad_static STATIC $<TARGET_OBJECTS:gamepad>)
    set_target_properties(gamepad_static PROPERTIES OUTPUT_NAME "gamepad")

    if (UNIX)
        target_link_libraries (gamepad_static ${CMAKE_THREAD_LIBS_INIT})
    elseif (MSVC)
        target_link_libraries(gamepad_static
            dinput8
            dxguid)
    endif()


    if (GAMEPAD_ENABLE_INSTALL)
        install(TARGETS gamepad_static DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
endif()

if (GAMEPAD_ENABLE_SHARED OR GAMEPAD_ENABLE_STATIC)
    if (GAMEPAD_ENABLE_INSTALL)
        install(FILES ./include/libgamepad.hpp
            DESTINATION include)
        install(FILES ./include/gamepad/binding.hpp
            ./include/gamepad/binding-dinput.hpp
            ./include/gamepad/binding-xinput.hpp
            ./include/gamepad/binding-linux.hpp
            ./include/gamepad/config.h
            ./include/gamepad/device.hpp
            ./include/gamepad/hook.hpp
            ./include/gamepad/hook-dinput.hpp
            ./include/gamepad/hook-linux.hpp
            ./include/gamepad/hook-xinput.hpp
            ./include/gamepad/log.hpp
            DESTINATION include/gamepad)
    endif()
endif()

if (UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries (gamepad ${CMAKE_THREAD_LIBS_INIT})
endif()

if (GAMEPAD_ENABLE_TESTS)
    add_executable(libgamepad_tests
        tests/test.cpp
    )

    if (UNIX)
        target_link_libraries(libgamepad_tests "${CMAKE_THREAD_LIBS_INIT}")
    endif()

    if (GAMEPAD_ENABLE_STATIC)
        target_link_libraries(libgamepad_tests gamepad_static)
        add_dependencies(libgamepad_tests gamepad_static)
    elseif (GAMEPAD_ENABLE_SHARED)
        target_link_libraries(libgamepad_tests gamepad_shared)
        add_dependencies(libgamepad_tests gamepad_shared)
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    else()
        target_link_libraries(libgamepad_tests gamepad)
        add_dependencies(libgamepad_tests gamepad)
    endif()

    if (GAMEPAD_ENABLE_INSTALL)
        install(TARGETS libgamepad_tests DESTINATION bin)
    endif()
endif()

if (UNIX)
    configure_file("./pc/gamepad.pc.in"
        "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" @ONLY)
    if (GAMEPAD_ENABLE_INSTALL)
        install(FILES "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" DESTINATION
            "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    endif()
endif()

include_directories("./include")

target_include_directories("gamepad" PUBLIC
    "./include"
    "./src")

set_target_properties("gamepad" PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/gamepad/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gamepad/config.h @ONLY)
