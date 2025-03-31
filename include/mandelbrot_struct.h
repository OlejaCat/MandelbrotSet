#ifndef MANDELBROT_STRUCT_H
#define MANDELBROT_STRUCT_H

typedef struct MandelbrotData
{
    int max_iterations;
    double zoom;
    double center_x;
    double center_y;
    double width;
    double height;    
} MandelbrotData;

#endif
