#ifndef MANDELBROT_STRUCT_H
#define MANDELBROT_STRUCT_H

#include <stdint.h>
#include <stdalign.h>

typedef struct MandelbrotData
{
    int   max_iterations;
    int*  iterations_per_pixel;

    alignas(32) uint32_t colors[512];

    double zoom;
    double center_x;
    double center_y;
    double width;
    double height;    
} MandelbrotData;

#endif
