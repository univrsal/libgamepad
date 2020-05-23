add_library(gamepad OBJECT)

option(ENABLE_STATIC "Static library (default: OFF)" ON)
option(ENABLE_SHARED "Shared library (default: ON)" OFF)

target_include_directories(gamepad
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(gamepad PRIVATE
    hook.cpp
    log.cpp
    binding-default.hpp
    binding-default.cpp
    )

include(GNUInstallDirs)

if (ENABLE_SHARED OR ENABLE_STATIC)
    install(FILES ../include/libgamepad.hpp
        DESTINATION include)
    install(FILES ../include/gamepad/binding.hpp
        ../include/gamepad/binding-dinput.hpp
        ../include/gamepad/binding-xinput.hpp
        ../include/gamepad/binding-linux.hpp
        ../include/gamepad/config.h
        ../include/gamepad/device.hpp
        ../include/gamepad/hook.hpp
        ../include/gamepad/hook-dinput.hpp
        ../include/gamepad/hook-linux.hpp
        ../include/gamepad/hook-xinput.hpp
        ../include/gamepad/log.hpp
        DESTINATION include/gamepad)
endif()

if (UNIX)
    find_package(Threads)
    target_link_libraries (gamepad ${CMAKE_THREAD_LIBS_INIT})
    configure_file("../pc/gamepad.pc.in"
        "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

if (ENABLE_SHARED)
    add_library(shared SHARED $<TARGET_OBJECTS:gamepad>)
    set_target_properties(shared PROPERTIES OUTPUT_NAME "gamepad")
    install(TARGETS shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
    if (UNIX)
        target_link_libraries (shared ${CMAKE_THREAD_LIBS_INIT})
    endif()
endif()

if (ENABLE_STATIC)
    add_library(static STATIC $<TARGET_OBJECTS:gamepad>)
    set_target_properties(static PROPERTIES OUTPUT_NAME "gamepad")
    if (UNIX)
        target_link_libraries (static ${CMAKE_THREAD_LIBS_INIT})
    endif()
    install(TARGETS static DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if (UNIX AND NOT APPLE)
    target_sources(gamepad PRIVATE
        hook-linux.cpp
        device-linux.cpp
        device-linux.hpp
        binding-linux.cpp
        )
elseif (APPLE)
    target_sources(gamepad PRIVATE
        hook-darwin.cpp
        )
elseif(WIN32)
    target_sources(gamepad PRIVATE
        hook-xinput.cpp
        hook-dinput.cpp
        binding-dinput.cpp
        binding-xinput.cpp
        device-dinput.cpp
        device-dinput.hpp
        device-xinput.cpp
        device-xinput.hpp
        )
    add_compile_definitions(UNICODE _UNICODE)
    target_link_libraries(gamepad
        dinput8
        dxguid)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
