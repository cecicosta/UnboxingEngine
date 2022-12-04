#Google tests
enable_testing()

target_sources(gtest PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/googletest
)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

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

add_executable(unboxing_engine_minimal_unit_tests
        main.cpp
        MatrixTests.cpp
        ../include/Matrix.h
        ../include/Quaternion.h
        ../source/Quaternion.cpp
        ../include/UVector.h
        ../source/UVector.cpp
        ../include/MathUtils.h
        ../include/BoundingBox.h
        ../source/BoundingBox.cpp
        ../include/internal_components/TransformComponent.h AlgorithmsTests.cpp BoundingBoxTest.cpp)
target_link_libraries(unboxing_engine_minimal_unit_tests
        gtest
        gmock
)

add_test(NAME unboxing_engine_tests COMMAND unboxing_engine_tests --exe unboxing_engine_tests.exe)
#gtest_discover_tests(TARGET unboxing_engine_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )

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
