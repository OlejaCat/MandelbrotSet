#include "mandelbrot_logic_array.h"

#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "screen_constants.h"
#include "mandelbrot_utils.h"


// static ---------------------------------------------------------------------


#define ARRAY_SIZE 16

#define ARRAY_AND_SCALAR_OP(OP, DESTINATION, SOURCE, SCALAR, SIZE) \
    for (int i_ = 0; i_ < SIZE; i_++) \
    { \
        DESTINATION[i_] = SOURCE[i_] OP SCALAR; \
    }

#define ARRAY_AND_ARRAY_OP(OP, DESTINATION, SOURCE1, SOURCE2, SIZE) \
    for (int i_ = 0; i_ < SIZE; i_++) \
    { \
        DESTINATION[i_] = SOURCE1[i_] OP SOURCE2[i_]; \
    }

#define ARRAY_SET_ARRAY(DESTINATION, SOURCE, SIZE) \
    for (int i_ = 0; i_ < SIZE; i_++) \
    { \
        DESTINATION[i_] = SOURCE[i_]; \
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


static void calculateIterationsArray(double x0[ARRAY_SIZE], 
                                     double y0[ARRAY_SIZE],
                                     int iterations[ARRAY_SIZE]);


// public ---------------------------------------------------------------------


void calculateMandelbrotArray(int pitch, 
                              uint32_t* pixels,
                              MandelbrotData* data)
{
    assert(pixels != NULL);
    assert(data   != NULL);

    int pitch_u32 = pitch / sizeof(uint32_t);

    const double dx = data->width / SCREEN_WIDTH;
    const double dy = data->height / SCREEN_HEIGHT;
    const double offset_x = data->center_x - data->width / 2;
    const double offset_y = data->center_y - data->height / 2;

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        const double y0_value = (SCREEN_HEIGHT - y) * dy + offset_y;
        
        for (int x = 0; x < SCREEN_WIDTH; x += ARRAY_SIZE) 
        {
            double x0[ARRAY_SIZE] = {};
            double y0[ARRAY_SIZE] = {};
            int iterations[ARRAY_SIZE] = {0};
            
            for (int i = 0; i < ARRAY_SIZE; i++) 
            {
                x0[i] = (x + i) * dx + offset_x;
                y0[i] = y0_value;
            }
            
            calculateIterationsArray(x0, y0, iterations);
            
            for (int i = 0; i < ARRAY_SIZE; i++) 
            {
                pixels[y * pitch_u32 + x + i] = data->colors[iterations[i] % MAX_ITERATIONS];
            }
        }
    }
}

// static ---------------------------------------------------------------------


static void calculateIterationsArray(double x0[ARRAY_SIZE], 
                                     double y0[ARRAY_SIZE],
                                     int iterations[ARRAY_SIZE])
{
    assert(x0 != NULL);
    assert(y0 != NULL);
    assert(iterations != NULL);

    double x2[ARRAY_SIZE] = {0.0};
    double y2[ARRAY_SIZE] = {0.0};
    double w[ARRAY_SIZE]  = {0.0};

    int mask[ARRAY_SIZE] = {0};
    bool active = false;
    
    for (int i = 0; i < MAX_ITERATIONS; i++) 
    {
        double radius[ARRAY_SIZE] = {0.0};
        ARRAY_AND_ARRAY_OP(+, radius, x2, y2, ARRAY_SIZE);
        ARRAY_COMPARE_AND_SET_MASK(<=, mask, radius, 4.0, active, ARRAY_SIZE);
        if (!active)
        {
            break;
        }
        
        double x[ARRAY_SIZE] = {0.0};
        double y[ARRAY_SIZE] = {0.0};

        ARRAY_AND_ARRAY_OP(-, x, x2, y2, ARRAY_SIZE)        
        ARRAY_AND_ARRAY_OP(+, x, x, x0, ARRAY_SIZE)

        ARRAY_AND_ARRAY_OP(-, y, w, x2, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OP(-, y, y, y2, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OP(+, y, y, y0, ARRAY_SIZE)

        ARRAY_AND_ARRAY_OP(*, x2, x, x, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OP(*, y2, y, y, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OP(+, w, x, y, ARRAY_SIZE)
        ARRAY_AND_ARRAY_OP(*, w, w, w, ARRAY_SIZE)

        ARRAY_AND_ARRAY_OP(+, iterations, iterations, mask, ARRAY_SIZE)
    }
}
