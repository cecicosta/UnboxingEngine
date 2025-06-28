cmake_minimum_required(VERSION 3.21)
project(simulation)

set(CMAKE_CXX_STANDARD 17)

#Disable building dlls
#set(BUILD_SHARED_LIBS FALSE)
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#set(BUILD_GMOCK "ON")
#set(INSTALL_GTEST "ON")

include_directories(
        ../include
        ../sdl_gl_render_system
)

add_executable(simulation simulation.cpp)
target_link_libraries(simulation PUBLIC unboxing_engine)

