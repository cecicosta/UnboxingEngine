cmake_minimum_required(VERSION 3.21)
project(UnboxingEngineExamples)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../include
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(2d_collision_vectors 2DCollisionVectors.cpp)
target_link_libraries(2d_collision_vectors unboxing_engine)


