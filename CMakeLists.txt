cmake_minimum_required(VERSION 3.21)
project(CatBoxEngine)

set(CMAKE_CXX_STANDARD 14)

include_directories(include
        third_party/SDL/include
        third_party/SDL_ttf
        third_party/SDL_image
        third_party/glew-2.1.0/include)

add_executable(catboxengine main.cpp)

add_subdirectory(include)
add_subdirectory(source)