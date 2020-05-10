add_library(gamepad STATIC)

set_property(TARGET gamepad PROPERTY CXX_STANDARD 17)
set_property(TARGET gamepad PROPERTY CXX_STANDARD_REQUIRED ON)


target_include_directories(gamepad
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)


target_sources(gamepad PRIVATE
    hook.cpp
    log.cpp
    device-linux.hpp
    binding-default.hpp
    binding-default.cpp
    )

if (UNIX)
    find_package(Threads)
    target_link_libraries (gamepad ${CMAKE_THREAD_LIBS_INIT})
endif()

if (UNIX AND NOT APPLE)
    target_sources(gamepad PRIVATE
        hook-linux.cpp
        device-linux.cpp
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
        )
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
