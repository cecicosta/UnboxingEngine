#Google tests
enable_testing()

include_directories(
        ./tests
        ../include
        ../third_party/googletest/googletest/include
        ../third_party/googletest/googlemock/include)

add_executable(
        unboxing_engine_tests
        main.cpp
        ComponentSystemTests.cpp
        EventSystemTests.cpp)

target_link_libraries(
        unboxing_engine_tests
        unboxing_engine_src_lib
        gtest
        gmock
)

add_test(NAME unboxing_engine_tests COMMAND unboxing_engine_tests)
#gtest_discover_tests(unboxing_engine_tests)

if (WIN32)
    #Copy dll after building
    add_custom_command(
            TARGET unboxing_engine_tests
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/Debug/gtest.dll ${CMAKE_BINARY_DIR}/Debug
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/Debug/gmock.dll ${CMAKE_BINARY_DIR}/Debug
    )
endif ()

target_compile_definitions( unboxing_engine_tests PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY )
