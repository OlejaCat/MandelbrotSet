cmake_minimum_required(VERSION 3.5)
project(mandelbrot_set)

# Create an option to switch between a system sdl library and a vendored SDL library
option(MANDEL_VENDORED "Use vendored libraries" OFF)

if(MANDEL_VENDORED)
    # This assumes you have added SDL as a submodule in vendored/SDL
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL3 package,
    # 2. look for the SDL3-shared component, and
    # 3. fail if the shared component cannot be found.
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXECUTABLE_SUFFIX ".out")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -mavx2")

set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=undefined -fsanitize=address -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(source)

