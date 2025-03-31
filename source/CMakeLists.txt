add_executable(mandel WIN32 main.cpp mandelbrot_start.cpp mandelbrot_logic_basic.cpp mandelbrot_utils.cpp)
target_link_libraries(mandel PRIVATE SDL3::SDL3)

