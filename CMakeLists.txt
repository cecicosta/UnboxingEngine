cmake_minimum_required(VERSION 3.21)
project(CatBoxEngine)

set(CMAKE_CXX_STANDARD 17)

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
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
    configure_file(./third_party/freetype-windows-binaries/release-dll/win64/freetype.dll ${CMAKE_BINARY_DIR} COPYONLY)
#    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

add_subdirectory(./third_party/glew-2.1.0/build/cmake)
add_subdirectory(./third_party/SDL)
add_subdirectory(./third_party/SDL_image)
add_subdirectory(./third_party/SDL_ttf)

include_directories(
        ./third_party/SDL/include
        ./third_party/zlib-1.2.12
        ./third_party/SDL_image
        ./third_party/SDL_ttf
        ./third_party/glew-2.1.0/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(catboxengine main.cpp include/IEngine.h include/EventDispatcher.h include/CoreEvents.h)
target_link_libraries(catboxengine opengl32 SDL2main glew SDL2 SDL2_image SDL2_ttf)

add_custom_command(
        TARGET catboxengine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/glew32d.dll ${CMAKE_BINARY_DIR}
)

add_subdirectory(include)
add_subdirectory(source)