#ifndef MANDELBROT_UTILS_H
#define MANDELBROT_UTILS_H

#include "screen_constants.h"
#include "mandelbrot_struct.h"

const int MAX_ITERATIONS = 500;

const double SCREEN_RATIO = (double)SCREEN_WIDTH / SCREEN_HEIGHT;
const double DEFAULT_WIDTH = 3.0;

const double DEFAULT_ZOOM = 1.0;
const double DEFAULT_CENTER_X = -0.75;
const double DEFAULT_CENTER_Y = 0.0;

void setDefaultMandelbrot(MandelbrotData* data);
void updateDimension(MandelbrotData* data);

#endif // MANDELBROT_UTILS_H
