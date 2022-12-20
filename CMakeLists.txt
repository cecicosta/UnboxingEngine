cmake_minimum_required(VERSION 3.21)
project(UnboxingEngine)

set(CMAKE_CXX_STANDARD 17)

#Set up SDL_image dependency
add_subdirectory(./third_party/zlib-1.2.12)
set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/zlib-1.2.12)

if (WIN32)
    set(ZLIB_LIBRARY ${PROJECT_SOURCE_DIR}/third_party/zlib-1.2.12/cmake-build-debug/libzlibstatic.a)

    #Set up SDL_ttf dependency
    set(FREETYPE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/third_party/freetype-windows-binaries/include)
    set(FREETYPE_LIBRARY ${PROJECT_SOURCE_DIR}/third_party/freetype-windows-binaries/release\ dll/win64/freetype.lib)

    #Enable building dlls
    set(BUILD_SHARED_LIBS TRUE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    configure_file(./third_party/freetype-windows-binaries/release\ dll/win64/freetype.dll ${CMAKE_BINARY_DIR}/Debug COPYONLY)

else()
    set(PNG_ARM_NEON off)
endif ()

add_subdirectory(./source)
add_subdirectory(./third_party/glew-2.1.0/build/cmake)
add_subdirectory(./third_party/SDL)
add_subdirectory(./third_party/SDL_image)
add_subdirectory(./third_party/SDL_ttf)

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
        ./include
        ./third_party/SDL/include
        ./third_party/zlib-1.2.12
        ./third_party/SDL_image
        ./third_party/SDL_ttf
        ./third_party/glew-2.1.0/include)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(unboxing_engine main.cpp source/UnboxingEngine.cpp "include/internal_components/RenderComponent.h" "include/systems/CollisionSystem.h" "include/internal_components/IRenderComponent.h")
target_link_libraries(unboxing_engine unboxing_engine_src_lib SDL2main glew SDL2 SDL2_image SDL2_ttf)

if (WIN32)
    #Copy OpenGL wrapper dll after building
    add_custom_command(
            TARGET unboxing_engine
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/Debug/glew32d.dll ${CMAKE_BINARY_DIR}/Debug
    )
endif ()
