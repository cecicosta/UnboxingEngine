cmake_minimum_required(VERSION 3.21)
project(UnboxingEngine)

set(CMAKE_CXX_STANDARD 17)

#Disable building dlls
set(BUILD_SHARED_LIBS FALSE)
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK "ON")
set(INSTALL_GTEST "OFF")

set(BUILD_WITH_EXTERNAL_LIBS ON)
add_compile_definitions(-DBUILD_WITH_EXTERNAL_LIBS=1)

add_subdirectory(./source)
add_subdirectory(./sdl_gl_render_system)
#add_subdirectory(./tests)
add_subdirectory(./Examples ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

include_directories(
        ./include
        ./sdl_gl_render_system
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_library(unboxing_engine source/UnboxingEngine.cpp)
target_link_libraries(unboxing_engine PUBLIC unboxing_engine_src_lib PRIVATE sdl_gl_render_system_lib)
target_sources(unboxing_engine PUBLIC ./source)