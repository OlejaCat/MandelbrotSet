add_executable(mandel WIN32 main.cpp mandelbrot_logic.cpp)
target_link_libraries(mandel PRIVATE SDL3::SDL3)

