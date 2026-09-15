cmake_minimum_required(VERSION 3.10)
project(vector_space CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_executable(vector_space
    src/main.cpp
    src/game.cpp
    src/ship.cpp
    src/asteroid.cpp
    src/bullet.cpp
    src/particle.cpp
    src/vector_font.cpp
    src/pirate.cpp
)

target_include_directories(vector_space PRIVATE src ${SDL2_INCLUDE_DIRS})
target_link_libraries(vector_space PRIVATE ${SDL2_LIBRARIES})
target_compile_options(vector_space PRIVATE ${SDL2_CFLAGS_OTHER})
