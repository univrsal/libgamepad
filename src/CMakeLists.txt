add_library(gamepad OBJECT)

set_property(TARGET gamepad PROPERTY CXX_STANDARD 17)
set_property(TARGET gamepad PROPERTY CXX_STANDARD_REQUIRED ON)

option(ENABLE_SHARED "Shared library (default: ON)" ON)
option(ENABLE_STATIC "Static library (default: OFF)" OFF)

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

if (ENABLE_SHARED OR ENABLE_STATIC)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/libgamepad.hpp DESTINATION include)
endif()

if (UNIX)
    find_package(Threads)
    target_link_libraries (gamepad ${CMAKE_THREAD_LIBS_INIT})
    configure_file("../pc/gamepad.pc.in"
        "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc" DESTINATION
        "${CMAKE_INSTALL_LIB_DIR}/pkgconfig")
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
