cmake_minimum_required(VERSION 3.21)
project(UnboxingEngine)

set(CMAKE_CXX_STANDARD 17)

#Disable building dlls
#set(BUILD_SHARED_LIBS FALSE)
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK "ON")
set(INSTALL_GTEST "OFF")

add_subdirectory(./source)

include_directories(
        ./include
        ./sdl_gl_render_system_lib
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(unboxing_engine main.cpp source/UnboxingEngine.cpp)
target_link_libraries(unboxing_engine unboxing_engine_src_lib sdl_gl_render_system_lib)

target_sources(unboxing_engine PUBLIC ./source)
target_sources(unboxing_engine PUBLIC ./tests)
target_sources(unboxing_engine PUBLIC ./Examples)
target_sources(unboxing_engine PUBLIC ./third_party/googletest)
target_sources(unboxing_engine PUBLIC ./sdl_gl_render_system_lib)