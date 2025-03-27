#ifndef MANDELBROT_LOGIC_H
#define MANDELBROT_LOGIC_H

#include <stdint.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


typedef struct MandelbrotData
{
    double zoom;
    double center_x;
    double center_y;
    double width;
    double height;    
} MandelbrotData;

int startMandelbrot(SDL_Renderer* renderer, SDL_Texture* texture);

#endif
