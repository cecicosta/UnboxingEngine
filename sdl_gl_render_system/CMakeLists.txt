cmake_minimum_required(VERSION 3.21)
project(sdl_gl_render_system)

set(CMAKE_CXX_STANDARD 17)

set(SDL_HAPTIC OFF)
set(SDL_JOYSTICK OFF)
set(SDL_HIDAPI OFF)
set(SDL_XINPUT OFF)
set(SDL_SENSOR OFF)
set(SDL_DYNAPI OFF)

set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../third_party/zlib-1.2.12)

if (WIN32)
    set(ZLIB_LIBRARY ${PROJECT_SOURCE_DIR}/../third_party/zlib-1.2.12/cmake-build-debug/libzlibstatic.a)

    #Set up SDL_ttf dependency
    set(FREETYPE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/../third_party/freetype-windows-binaries/include)
    set(FREETYPE_LIBRARY ${PROJECT_SOURCE_DIR}/../third_party/freetype-windows-binaries/release\ dll/win64/freetype.lib)

    #Enable building dlls
    set(BUILD_SHARED_LIBS TRUE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    configure_file(../third_party/freetype-windows-binaries/release\ dll/win64/freetype.dll ${CMAKE_BINARY_DIR} COPYONLY)

else()
    set(PNG_ARM_NEON off)
endif ()

include_directories(
        ../include
        ../third_party/SDL/include
        ../third_party/zlib-1.2.12
        ../third_party/SDL_image
        ../third_party/SDL_ttf
        ../third_party/glew-2.1.0/include)
#Set up dependency
#include(../source/CMakeLists.txt)
#include(../third_party/zlib-1.2.12/CMakeLists.txt)
#include(../third_party/glew-2.1.0/build/cmake/CMakeLists.txt)
#include(../third_party/SDL/CMakeLists.txt)
#include(../third_party/SDL_image/CMakeLists.txt)
#include(../third_party/SDL_ttf/CMakeLists.txt)

add_subdirectory("../third_party/zlib-1.2.12/" "../third_party/zlib-1.2.12")
add_subdirectory("../third_party/glew-2.1.0/build/cmake/" "../third_party/glew-2.1.0/build/cmake")
add_subdirectory("../third_party/SDL/" "../third_party/SDL")
add_subdirectory("../third_party/SDL_image/" "../third_party/SDL_image")
add_subdirectory("../third_party/SDL_ttf/" "../third_party/SDL_ttf")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

add_library(sdl_gl_render_system_lib STATIC "sdl_gl_render_system_lib.cpp" "sdl_gl_render_system_lib.h" "sdl_input_system_lib.h" "sdl_input_system_lib.cpp")
target_link_libraries(sdl_gl_render_system_lib PUBLIC zlib SDL2 SDL2Main SDL2_image glew_s unboxing_engine_src_lib)


if (WIN32)
    # Copy OpenGL wrapper dll after building
    #add_custom_command(
     #       TARGET sdl_gl_render_system_lib
     #       POST_BUILD
     #       COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/glew32d.dll ${CMAKE_BINARY_DIR}
    #)
endif ()
