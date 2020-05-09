add_executable(libgamepad_tests
    test.cpp
)

target_link_libraries(libgamepad_tests
    PRIVATE
        gamepad
)
