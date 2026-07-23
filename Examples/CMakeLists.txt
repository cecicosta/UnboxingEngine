cmake_minimum_required(VERSION 3.5)
project(UnboxingEngineExamples)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../include
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(2d_collision_vectors 2DCollisionVectors.cpp)
target_link_libraries(2d_collision_vectors unboxing_engine)

add_executable(bresenham_voxelization_port BresenhamVoxelizationPort.cpp)
target_link_libraries(bresenham_voxelization_port unboxing_engine)

add_executable(bresenham_voxelization_octree_port BresenhamVoxelizationOctreePort.cpp)
target_link_libraries(bresenham_voxelization_octree_port unboxing_engine)

add_executable(path_intersection_triangle_example PathIntersectionTriangleExample.cpp)
target_link_libraries(path_intersection_triangle_example unboxing_engine)

add_executable(radiosity_port RadiosityPort.cpp RadiosityCornellBox.cpp)
target_link_libraries(radiosity_port unboxing_engine)

add_executable(radiosity_octree_port RadiosityOctreePort.cpp RadiosityCornellBox.cpp)
target_link_libraries(radiosity_octree_port unboxing_engine)
