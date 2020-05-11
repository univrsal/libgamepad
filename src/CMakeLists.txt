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
