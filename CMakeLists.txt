cmake_minimum_required(VERSION 3.21)
project(UnboxingEngine)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(./source)

#Disable building dlls
#set(BUILD_SHARED_LIBS FALSE)
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK "ON")
set(INSTALL_GTEST "OFF")
add_subdirectory(./third_party/googletest)
add_subdirectory(./tests)
add_subdirectory(./Examples)

include_directories(
        ./include)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(unboxing_engine main.cpp source/UnboxingEngine.cpp)
target_link_libraries(unboxing_engine unboxing_engine_src_lib OpenGL_SDL_RenderSystem)

target_sources(2d_collision_vectors PUBLIC ./source/systems)
