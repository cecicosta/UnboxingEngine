cmake_minimum_required(VERSION 3.21)
project(sdl_gl_render_system)

set(CMAKE_CXX_STANDARD 17)


if (WIN32)

    #Enable building dlls
    set(BUILD_SHARED_LIBS TRUE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

else()
    set(PNG_ARM_NEON off)
endif ()

set(SDL_SENSOR off)
set(SDL_JOYSTICK off)
set(SDL_HAPTIC off)
set(SDL_STATIC on)
set(SDL_SHARED off)


include_directories(
        ../include
        ../third_party/SDL
        ../third_party/SDL_image
        ../third_party/glew-2.1.0/include)
#Set up dependency
#include(../source/CMakeLists.txt)
#include(../third_party/zlib-1.2.12/CMakeLists.txt)
#include(../third_party/glew-2.1.0/build/cmake/CMakeLists.txt)
#include(../third_party/SDL/CMakeLists.txt)
#include(../third_party/SDL_image/CMakeLists.txt)
#include(../third_party/SDL_ttf/CMakeLists.txt)

add_subdirectory(../source/ ../source)
add_subdirectory("../third_party/glew-2.1.0/build/cmake/" "../third_party/glew-2.1.0/build/cmake")
add_subdirectory("../third_party/SDL/" "../third_party/SDL" EXCLUDE_FROM_ALL)
add_subdirectory("../third_party/SDL_image/" "../third_party/SDL_image")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

add_library(sdl_gl_render_system_lib STATIC "sdl_gl_render_system_lib.cpp" "sdl_gl_render_system_lib.h" "sdl_input_system_lib.h" "sdl_input_system_lib.cpp")
target_link_libraries(sdl_gl_render_system_lib PRIVATE SDL2-static glew_s unboxing_engine_src_lib)
