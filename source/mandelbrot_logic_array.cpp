#include "mandelbrot_logic_array.h"

#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "screen_constants.h"
#include "mandelbrot_utils.h"


// static ---------------------------------------------------------------------

#define ARRAY_SIZE 16

#define ARRAY_AND_SCALAR_OPERATION(OP, DESTINATION, SOURCE, SCALAR, SIZE) \
    for (int i_ = 0; i_ < SIZE; i_++) \
    { \
        DESTINATION[i_] = SOURCE[i_] OP SCALAR; \
    }

#define ARRAY_AND_ARRAY_OPERATION(OP, DESTINATION, SOURCE1, SOURCE2, SIZE) \
    for (int i_ = 0; i_ < SIZE; i_++) \
    { \
        DESTINATION[i_] = SOURCE1[i_] OP SOURCE2[i_]; \
    }

#define ARRAY_SET(DESTINATION, VALUE, SIZE) \
    for (int i_ = 0; i_ < SIZE; i_++) \
    { \
        DESTINATION[i_] = VALUE; \
    }

#define ARRAY_COMPARE_AND_SET_MASK(CMP, MASK, ARRAY, SCALAR, HAS_ACTIVE, SIZE) \
    HAS_ACTIVE = false; \
    for (int i_ = 0; i_ < SIZE; i_++) { \
        if (ARRAY[i_] CMP SCALAR) { \
            MASK[i_] = 1; \
            HAS_ACTIVE = true; \
        } else { \
            MASK[i_] = 0; \
        } \
    }


static uint32_t getColorFromIteration(int iterations, const SDL_PixelFormatDetails* format);
static void calculateIterationFromPosition(double x_pixels[ARRAY_SIZE], 
                                           double y_pixels[ARRAY_SIZE], 
                                           int* iterations,
                                           MandelbrotData* data);



// public ---------------------------------------------------------------------


void calculateMandelbrotArray(int pitch, 
                              uint32_t* pixels,
                              const SDL_PixelFormatDetails* format,
                              MandelbrotData* data)
{
    assert(pixels != NULL);
    assert(format != NULL);
    assert(data   != NULL);

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x+=ARRAY_SIZE) 
        {
            double x_pixels[ARRAY_SIZE] = {};
            double y_pixels[ARRAY_SIZE] = {};
            for (int i = 0; i < ARRAY_SIZE; i++)
            {
                x_pixels[i] = (double)(x + i);
                y_pixels[i] = (double)y;
            }
            int iterations[ARRAY_SIZE] = {0};
            calculateIterationFromPosition(x_pixels, y_pixels, iterations, data);
            for (int x0 = 0; x0 < ARRAY_SIZE; x0++)
            {
                pixels[y * (pitch / 4) + x + x0] = getColorFromIteration(iterations[x0], format);
            }
        }
    }
}


// static ---------------------------------------------------------------------



static inline void calculateIterationFromPosition(double x_pixels[ARRAY_SIZE], 
                                          double y_pixels[ARRAY_SIZE], 
                                          int* iteration,
                                          MandelbrotData* data)
{
    assert(data      != NULL);
    assert(x_pixels  != NULL);
    assert(y_pixels  != NULL);
    assert(iteration != NULL);

    double dx = data->width / (double)SCREEN_WIDTH;
    double dy = data->height / (double)SCREEN_HEIGHT;

    double norm_x[ARRAY_SIZE] = {};
    double norm_y[ARRAY_SIZE] = {};

    ARRAY_AND_SCALAR_OPERATION(*, norm_x, x_pixels, dx, ARRAY_SIZE);
    ARRAY_AND_SCALAR_OPERATION(-, norm_y, y_pixels, (double)SCREEN_HEIGHT, ARRAY_SIZE);
    ARRAY_AND_SCALAR_OPERATION(*, norm_y, y_pixels, dy, ARRAY_SIZE);

    double x0[ARRAY_SIZE] = {};
    double y0[ARRAY_SIZE] = {};

    ARRAY_AND_SCALAR_OPERATION(-, x0, norm_x, ((data->width / 2) - data->center_x), ARRAY_SIZE);
    ARRAY_AND_SCALAR_OPERATION(-, y0, norm_y, ((data->height / 2) - data->center_y), ARRAY_SIZE);

    double X2[ARRAY_SIZE] = {};
    double Y2[ARRAY_SIZE] = {};
    double X[ARRAY_SIZE] = {};
    double Y[ARRAY_SIZE] = {};
    double W[ARRAY_SIZE] = {};

    ARRAY_SET(X2, 0.0, ARRAY_SIZE)
    ARRAY_SET(Y2, 0.0, ARRAY_SIZE)
    ARRAY_SET(X, 0.0, ARRAY_SIZE)
    ARRAY_SET(Y, 0.0, ARRAY_SIZE)
    ARRAY_SET(W, 0.0, ARRAY_SIZE)

    double radius[ARRAY_SIZE] = {};
    int mask[ARRAY_SIZE] = {};
    bool has_active;

    ARRAY_SET(iteration, 0, ARRAY_SIZE)
    ARRAY_SET(mask, 1, ARRAY_SIZE)
    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        ARRAY_AND_ARRAY_OPERATION(+, radius, X2, Y2, ARRAY_SIZE)
        ARRAY_COMPARE_AND_SET_MASK(<=, mask, radius, 4.0, has_active, ARRAY_SIZE);

        if (!has_active)
        {
            break; 
        }

        ARRAY_AND_ARRAY_OPERATION(-, X, X2, Y2, ARRAY_SIZE)        
        ARRAY_AND_ARRAY_OPERATION(+, X, X, x0, ARRAY_SIZE)

        ARRAY_AND_ARRAY_OPERATION(-, Y, W, X2, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OPERATION(-, Y, Y, Y2, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OPERATION(+, Y, Y, y0, ARRAY_SIZE)

        ARRAY_AND_ARRAY_OPERATION(*, X2, X, X, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OPERATION(*, Y2, Y, Y, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OPERATION(+, W, X, Y, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OPERATION(*, W, W, W, ARRAY_SIZE)

        ARRAY_AND_ARRAY_OPERATION(+, iteration, iteration, mask, ARRAY_SIZE)
    }
}



static uint32_t getColorFromIteration(int iterations, const SDL_PixelFormatDetails* format) 
{
    assert(format != NULL);

    if (iterations >= MAX_ITERATIONS)
    {
        return SDL_MapRGBA(format, NULL, 0, 0, 0, 255);
    }
    
    double t = (double)iterations / MAX_ITERATIONS;

    t = pow(t, 0.5);

    uint8_t r, g, b;
    if (t < 0.25) {
        r = 50;
        g = 4 * t * 255;
        b = 255;
    } else if (t < 0.5) {
        r = 50;
        g = 255;
        b = 255 - 4 * (t - 0.25) * 255;
    } else if (t < 0.75) {
        r = 4 * (t - 0.5) * 255;
        g = 255;
        b = 50;
    } else {
        r = 255;
        g = 255 - 4 * (t - 0.75) * 255;
        b = 50;
    }

    uint32_t color = SDL_MapRGBA(format, NULL, r, g, b, 255);
    return color;
}

