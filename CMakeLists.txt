cmake_minimum_required(VERSION 3.20)
project(mandel)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -mavx2 -march=native")

set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=undefined -fsanitize=address -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(${PROJECT_NAME} 
    source/main.cpp 
    source/mandelbrot_start.cpp 
    source/mandelbrot_logic_basic.cpp 
    source/mandelbrot_logic_array.cpp 
    source/mandelbrot_utils.cpp
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        SDL3::SDL3
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include/
)
