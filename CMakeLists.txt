cmake_minimum_required(VERSION 3.21)
project(UnboxingEngine)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_GMOCK "ON")
set(INSTALL_GTEST "OFF")


add_subdirectory(./sdl_gl_render_system)
add_subdirectory(./Examples)

include_directories(
        ./include
        ./sdl_gl_render_system
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_library(unboxing_engine STATIC source/UnboxingEngine.cpp)
target_link_libraries(unboxing_engine PUBLIC unboxing_engine_src_lib INTERFACE sdl_gl_render_system_lib)
