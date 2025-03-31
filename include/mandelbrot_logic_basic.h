#ifndef MANDELBROT_LOGIC_H
#define MANDELBROT_LOGIC_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "mandelbrot_struct.h"

void calculateMandelbrot(int pitch, 
                         uint32_t* pixels,
                         const SDL_PixelFormatDetails* format,
                         MandelbrotData* data);

#endif
