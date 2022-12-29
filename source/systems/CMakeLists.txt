cmake_minimum_required(VERSION 3.21)
project(OpenGL_SDL_RenderSystem)

set(CMAKE_CXX_STANDARD 17)


set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../../third_party/zlib-1.2.12)

if (WIN32)
    set(ZLIB_LIBRARY ${PROJECT_SOURCE_DIR}/../../third_party/zlib-1.2.12/cmake-build-debug/libzlibstatic.a)

    #Set up SDL_ttf dependency
    set(FREETYPE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/../../third_party/freetype-windows-binaries/include)
    set(FREETYPE_LIBRARY ${PROJECT_SOURCE_DIR}/../../third_party/freetype-windows-binaries/release\ dll/win64/freetype.lib)

    #Enable building dlls
    set(BUILD_SHARED_LIBS TRUE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    configure_file(../../third_party/freetype-windows-binaries/release\ dll/win64/freetype.dll ${CMAKE_BINARY_DIR} COPYONLY)

else()
    set(PNG_ARM_NEON off)
endif ()

include_directories(
        ../../third_party/SDL/include
        ../../third_party/zlib-1.2.12
        ../../third_party/SDL_image
        ../../third_party/SDL_ttf
        ../../third_party/glew-2.1.0/include)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(gl_sdl_render_system 2DCollisionVectors.cpp ../source/systems/OpenGLRenderSystem.cpp)
target_link_libraries(gl_sdl_render_system SDL2main glew SDL2 SDL2_image SDL2_ttf)

#Set up SDL_image dependency
target_sources(gl_sdl_render_system PUBLIC ../../third_party/zlib-1.2.12)
target_sources(gl_sdl_render_system PUBLIC ../../third_party/glew-2.1.0/build/cmake)
target_sources(gl_sdl_render_system PUBLIC ../../third_party/SDL)
target_sources(gl_sdl_render_system PUBLIC ../../third_party/SDL_image)
target_sources(gl_sdl_render_system PUBLIC ../../third_party/SDL_ttf)
target_sources(gl_sdl_render_system PUBLIC ../../tests)


if (WIN32)
    #Copy OpenGL wrapper dll after building
    add_custom_command(
            TARGET gl_sdl_render_system
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/glew32d.dll ${CMAKE_BINARY_DIR}
    )
endif ()
