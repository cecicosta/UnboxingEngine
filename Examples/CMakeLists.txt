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

add_executable(octree_ray_picking_lab OctreeRayPickingLab.cpp)
target_link_libraries(octree_ray_picking_lab unboxing_engine)

add_executable(coctree_torus_minimal COctreeTorusMinimal.cpp ../source/COctree.cpp)
target_link_libraries(coctree_torus_minimal unboxing_engine)

add_executable(radiosity_port RadiosityPort.cpp RadiosityCornellBox.cpp)
target_link_libraries(radiosity_port unboxing_engine)

add_executable(circle_triangle_collision CircleTriangleCollision.cpp)
target_link_libraries(circle_triangle_collision unboxing_engine)

add_executable(atom_simulation AtomSimulation.cpp)
target_link_libraries(atom_simulation unboxing_engine)