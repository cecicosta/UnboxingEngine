cmake_minimum_required(VERSION 3.21)
project(UnboxingEngineExamples)

set(CMAKE_CXX_STANDARD 17)


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


#Disable building dlls
#set(BUILD_SHARED_LIBS FALSE)
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK "ON")
set(INSTALL_GTEST "ON")

include_directories(
        ../include
        ../third_party/SDL/include
        ../third_party/zlib-1.2.12
        ../third_party/SDL_image
        ../third_party/SDL_ttf
        ../third_party/glew-2.1.0/include)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(2d_collision_vectors 2DCollisionVectors.cpp ../source/UnboxingEngine.cpp)
target_link_libraries(2d_collision_vectors unboxing_engine_src_lib SDL2main glew SDL2 SDL2_image SDL2_ttf)

#Set up SDL_image dependency
target_sources(2d_collision_vectors PUBLIC ../third_party/zlib-1.2.12)
target_sources(2d_collision_vectors PUBLIC ../source)
target_sources(2d_collision_vectors PUBLIC ../third_party/glew-2.1.0/build/cmake)
target_sources(2d_collision_vectors PUBLIC ../third_party/SDL)
target_sources(2d_collision_vectors PUBLIC ../third_party/SDL_image)
target_sources(2d_collision_vectors PUBLIC ../third_party/SDL_ttf)
target_sources(2d_collision_vectors PUBLIC ../third_party/googletest)
target_sources(2d_collision_vectors PUBLIC ../tests)


if (WIN32)
    #Copy OpenGL wrapper dll after building
    add_custom_command(
            TARGET 2d_collision_vectors
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/glew32d.dll ${CMAKE_BINARY_DIR}
    )
endif ()
