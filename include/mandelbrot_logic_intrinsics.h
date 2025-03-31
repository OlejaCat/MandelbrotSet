#ifndef MANDELBROT_LOGIC_INTRINSICS_H
#define MANDELBROT_LOGIC_INTRINSICS_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdint.h>
#include <immintrin.h>

#include "mandelbrot_struct.h"

void calculateMandelbrotIntrinsics(int pitch, 
                                   uint32_t* pixels,
                                   const SDL_PixelFormatDetails* format,
                                   MandelbrotData* data);

#endif
