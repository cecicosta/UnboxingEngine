cmake_minimum_required(VERSION 3.21)
project(sdl_gl_render_system)

option(BUILD_WITH_EXTERNAL_LIBS OFF)

set(CMAKE_CXX_STANDARD 17)

if(${BUILD_WITH_EXTERNAL_LIBS})

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

    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    add_library(sdl_gl_render_system_lib STATIC "sdl_gl_render_system_lib.cpp" "sdl_gl_render_system_lib.h" "sdl_input_system_lib.h" "sdl_input_system_lib.cpp")
    target_link_libraries(sdl_gl_render_system_lib PUBLIC SDL2main glew_s SDL2 SDL2_image SDL2_ttf unboxing_engine_src_lib)

    target_sources(sdl_gl_render_system_lib PUBLIC ../source)
    #Set up SDL_image dependency
    add_subdirectory(../third_party/zlib-1.2.12 ${CMAKE_BINARY_DIR}/zlib-1.2.12/)
    add_subdirectory(../third_party/glew-2.1.0/build/cmake ${CMAKE_BINARY_DIR}/glew-2.1.0/)
    add_subdirectory(../third_party/SDL ${CMAKE_BINARY_DIR}/SDL/)
    add_subdirectory(../third_party/SDL_image ${CMAKE_BINARY_DIR}/SDL_image/)
    add_subdirectory(../third_party/SDL_ttf ${CMAKE_BINARY_DIR}/SDL_ttf/)

    if (WIN32)
        #Copy OpenGL wrapper dll after building
        #add_custom_command(
        #        TARGET sdl_gl_render_system_lib
        #        POST_BUILD
        #        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/glew32d.dll ${CMAKE_BINARY_DIR}
        #)
    endif ()
else()
    include_directories(../include)

    add_library(sdl_gl_render_system_lib STATIC "dummie_libs/sdl_gl_render_system_lib.h" "dummie_libs/sdl_gl_render_system_lib.cpp" "dummie_libs/sdl_input_system_lib.h" "dummie_libs/sdl_input_system_lib.cpp")
    
    target_link_libraries(sdl_gl_render_system_lib PUBLIC unboxing_engine_src_lib)

    target_sources(sdl_gl_render_system_lib PUBLIC ../source)
endif()