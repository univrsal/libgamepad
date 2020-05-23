add_executable(libgamepad_tests
    test.cpp
)

include_directories("../include")

target_link_libraries(libgamepad_tests
    PRIVATE
        shared
)
