#Google tests
enable_testing()

include_directories(./tests ../include ../third_party/googletest/googletest/include)

add_executable(
        unboxing_engine_tests
        main.cpp
        ComponentSystemTests.cpp
)

target_link_libraries(
        unboxing_engine_tests
        ${CMAKE_BINARY_DIR}/source/libunboxing_engine_src_lib.a
        ${CMAKE_BINARY_DIR}/lib/libgtest.dll.a
)

#add_test(NAME unboxing_engine_tests COMMAND unboxing_engine_tests)

include(GoogleTest)
gtest_discover_tests(unboxing_engine_tests)