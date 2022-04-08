cmake_minimum_required(VERSION 3.21)
project(CatBoxEngine)

set(CMAKE_CXX_STANDARD 14)

include_directories(
        ./include
        ./source)

add_subdirectory(./third_party/zlib-1.2.12)
set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/zlib-1.2.12)
set(ZLIB_LIBRARY ${PROJECT_SOURCE_DIR}/third_party/zlib-1.2.12/cmake-build-debug/libzlibstatic.a)

if(WIN32)
    set(FREETYPE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/third_party/freetype-windows-binaries/include)
    set(FREETYPE_LIBRARY ${PROJECT_SOURCE_DIR}/third_party/freetype-windows-binaries/release-dll/win64/freetype.lib)

    set(BUILD_SHARED_LIBS "ON")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    configure_file(./third_party/freetype-windows-binaries/release-dll/win64/freetype.dll ${CMAKE_BINARY_DIR} COPYONLY)
endif()

add_subdirectory(./third_party/SDL)
add_subdirectory(./third_party/SDL_image)
add_subdirectory(./third_party/SDL_ttf)
add_subdirectory(./third_party/glew-2.1.0/build/cmake)

include_directories(
        ./third_party/SDL/include
        ./third_party/zlib-1.2.12
        ./third_party/SDL_image
        ./third_party/SDL_ttf
        ./third_party/glew-2.1.0/include)

add_executable(catboxengine main.cpp)
target_link_libraries(catboxengine glew_s SDL2-static SDL2_image SDL2_ttf)

add_subdirectory(include)
add_subdirectory(source)