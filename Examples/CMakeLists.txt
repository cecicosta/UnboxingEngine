cmake_minimum_required(VERSION 3.21)
project(UnboxingEngineExamples)

set(CMAKE_CXX_STANDARD 17)

#Disable building dlls
#set(BUILD_SHARED_LIBS FALSE)
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK "ON")
set(INSTALL_GTEST "ON")

include_directories(
        ../include
        ../sdl_gl_render_system_lib
)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(2d_collision_vectors 2DCollisionVectors.cpp ../source/UnboxingEngine.cpp)
target_link_libraries(2d_collision_vectors unboxing_engine_src_lib sdl_gl_render_system_lib)

target_sources(2d_collision_vectors PUBLIC ../source)
target_sources(2d_collision_vectors PUBLIC ../tests)
