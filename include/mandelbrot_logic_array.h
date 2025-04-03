#ifndef MANDELBROT_LOGIC_ARRAY_H
#define MANDELBROT_LOGIC_ARRAY_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdint.h>
#include "mandelbrot_struct.h"

void calculateMandelbrotArray(int pitch, 
                              uint32_t* pixels,
                              MandelbrotData* data);

#endif
