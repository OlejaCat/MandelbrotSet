#include "mandelbrot_logic_basic.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <assert.h>
#include <math.h>

#include "screen_constants.h"
#include "mandelbrot_utils.h"


// static ---------------------------------------------------------------------


static int calculateColors(int pitch, uint32_t* pixels, MandelbrotData* data);
static int calculateIterationFromPosition(int x_pixel, 
                                          int y_pixel, 
                                          MandelbrotData* data);


// public ---------------------------------------------------------------------


void calculateMandelbrot(int pitch, 
                         uint32_t* pixels,
                         MandelbrotData* data)
{
    assert(pixels != NULL);
    assert(data   != NULL);

    int pitch_u32 = pitch / sizeof(uint32_t);
    uint32_t* palette = data->colors;

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++) 
        {
            int iterations = calculateIterationFromPosition(x, y, data);
            pixels[y * pitch_u32 + x] = palette[iterations % MAX_ITERATIONS];
        }
    }
}


void calculateMandelbrotSeparate(int pitch,
                                 uint32_t* pixels,
                                 MandelbrotData* data)
{
    assert(data   != NULL);
    assert(pixels != NULL);

    calculateIterationField(data);
    calculateColors(pitch, pixels, data);
}


void calculateIterationField(MandelbrotData* data)
{
    assert(data != NULL);

    int* field = data->iterations_per_pixel;
    
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++) 
        {
            int iterations = calculateIterationFromPosition(x, y, data);
            field[y * SCREEN_WIDTH + x] = iterations % MAX_ITERATIONS;
        }
    }
}


// static ---------------------------------------------------------------------


static int calculateColors(int pitch, uint32_t* pixels, MandelbrotData* data)
{
    assert(data != NULL);

    int* field = data->iterations_per_pixel;
    uint32_t* palette = data->colors;
    int pitch_u32 = pitch / sizeof(uint32_t);

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            int iterations = field[y * SCREEN_WIDTH + x];
            pixels[y * pitch_u32 + x] = palette[iterations % MAX_ITERATIONS];
        }
    }

    return 0;
}


static int calculateIterationFromPosition(int x_pixel, 
                                          int y_pixel, 
                                          MandelbrotData* data)
{
    assert(data != NULL);

    double norm_x = (x_pixel / (double)SCREEN_WIDTH) * data->width;
    double norm_y = ((SCREEN_HEIGHT - y_pixel) / (double)SCREEN_HEIGHT) * data->height;

    const double x0 = norm_x - (data->width / 2) + data->center_x;
    const double y0 = norm_y - (data->height / 2) + data->center_y;
    
    double x2 = 0.0;
    double y2 = 0.0;
    double w = 0.0;

    int iteration = 0;
    while (x2 + y2 <= 4.0 && iteration < MAX_ITERATIONS)
    {
        double x = x2 - y2 + x0;
        double y = w - x2 - y2 + y0;

        w = (x + y) * (x + y);

        x2 = x * x;
        y2 = y * y;

        iteration++;
    }
    
    return iteration;
}         


