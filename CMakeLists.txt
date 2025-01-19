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


add_subdirectory(./sdl_gl_render_system)
add_subdirectory(./source)
add_subdirectory(./tests)
add_subdirectory(./Examples)
add_subdirectory(./third_party/googletest)

include_directories(
        ./include
        ./sdl_gl_render_system
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_library(unboxing_engine source/UnboxingEngine.cpp)
target_link_libraries(unboxing_engine PUBLIC unboxing_engine_src_lib INTERFACE sdl_gl_render_system_lib)
