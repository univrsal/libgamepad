add_executable(libgamepad_tests
    test.cpp
)

include_directories("../include")

if (ENABLE_SHARED)
    target_link_libraries(libgamepad_tests static)
    add_dependencies(libgamepad_tests static)
elseif (ENABLE_STATIC)
    target_link_libraries(libgamepad_tests shared)
    add_dependencies(libgamepad_tests shared)
else()
    target_link_libraries(libgamepad_tests gamepad)
    add_dependencies(libgamepad_tests gamepad)
endif()

install(TARGETS libgamepad_tests DESTINATION bin)