#ifndef MANDELBROT_LOGIC_INTRINSICS_H
#define MANDELBROT_LOGIC_INTRINSICS_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdint.h>
#include <immintrin.h>

#include "mandelbrot_struct.h"

void calculateMandelbrotIntrinsicsSeparated(int pitch,
                                            uint32_t* pixels,
                                            MandelbrotData* data);
void calculateIterationsFieldIntrinsics(MandelbrotData* data);

#endif // MANDELBROT_LOGIC_INTRINSICS_H
