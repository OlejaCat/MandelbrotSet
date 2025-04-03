cmake_minimum_required(VERSION 3.20)
project(mandel)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Wextra -pedantic -mavx2 -lm -march=native")

set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=undefined -fsanitize=address -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(${PROJECT_NAME} 
    source/main.cpp 
    source/mandelbrot_start.cpp 
    source/mandelbrot_logic_basic.cpp 
    source/mandelbrot_logic_intrinsics.cpp
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

add_executable(tester
    source/mandelbrot_benchmark.cpp
    source/mandelbrot_logic_basic.cpp 
    source/mandelbrot_logic_intrinsics.cpp
    source/mandelbrot_logic_array.cpp 
    source/mandelbrot_utils.cpp
)

target_link_libraries(tester
    PRIVATE 
        SDL3::SDL3
)

set_source_files_properties(
    source/mandelbrot_benchmark.cpp 
    PROPERTIES 
        COMPILE_OPTIONS "-O0"  
)

target_include_directories(tester
    PRIVATE
        include/
)
