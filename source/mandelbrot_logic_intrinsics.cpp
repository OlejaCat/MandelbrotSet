#include "mandelbrot_logic_intrinsics.h"

#include <assert.h>
#include <math.h>

#include "screen_constants.h"
#include "mandelbrot_utils.h"


// static ----------------------------------------------------------------------

static __m256i getColorFromIteration(__m256i iterations, 
                                     const SDL_PixelFormatDetails* format);
static __m256i calculateIterationsFromPositionIntrinsics(__m256d x_pixels, 
                                                         __m256d y_pixels, 
                                                         MandelbrotData* data);

// public ----------------------------------------------------------------------


void calculateMandelbrotIntrinsics(int pitch, 
                                   uint32_t* pixels,
                                   const SDL_PixelFormatDetails* format,
                                   MandelbrotData* data)
{
    assert(pixels != NULL);
    assert(format != NULL);
    assert(data   != NULL);

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x+=4) 
        {
            __m256d x_pixels = _mm256_setr_pd((double)(x),
                                              (double)(x + 1),
                                              (double)(x + 2),
                                              (double)(x + 3));
            __m256d y_pixels = _mm256_set1_pd((double)y);
            __m256i iterations = calculateIterationsFromPositionIntrinsics(x_pixels, y_pixels, data);
            int64_t iterations_array[4] = {};
            _mm256_store_si256((__m256i*)iterations_array, iterations);
            for (int x0 = 0; x0 < 4; x0++)
            {
                uint32_t color = getColorFromIteration((int)(iterations_array[x0]), format);
                pixels[y * (pitch / 4) + x + x0] = color;
            }
        }
    }
}


static __m256i calculateColorFromPositionIntrinsics(__m256d x_pixels, 
                                                    __m256d y_pixels, 
                                                    MandelbrotData* data)
{
    assert(data != NULL);

    __m256d mandelbrot_width = _mm256_set1_pd(data->width);
    __m256d screen_width = _mm256_set1_pd((double)SCREEN_WIDTH);

    __m256d norm_x = _mm256_mul_pd(x_pixels, mandelbrot_width);
    norm_x = _mm256_div_pd(norm_x, screen_width);

    __m256d mandelbrot_height = _mm256_set1_pd(data->height);
    __m256d screen_height = _mm256_set1_pd((double)SCREEN_HEIGHT);

    __m256d norm_y = _mm256_sub_pd(screen_height, y_pixels);
    norm_y = _mm256_mul_pd(norm_y, mandelbrot_height);
    norm_y = _mm256_div_pd(norm_y, screen_height);

    __m256d twos = _mm256_set1_pd(2.0);
    mandelbrot_width  = _mm256_div_pd(mandelbrot_width, twos);
    mandelbrot_height = _mm256_div_pd(mandelbrot_height, twos);

    __m256d center_x = _mm256_set1_pd(data->center_x);
    __m256d center_y = _mm256_set1_pd(data->center_y);

    __m256d x0 = _mm256_add_pd(norm_x, center_x);
    x0 = _mm256_sub_pd(x0, mandelbrot_width);

    __m256d y0 = _mm256_add_pd(norm_y, center_y);
    y0 = _mm256_sub_pd(y0, mandelbrot_height);

    __m256d X2 = _mm256_setzero_pd();
    __m256d Y2 = _mm256_setzero_pd();

    __m256d X = _mm256_setzero_pd();
    __m256d Y = _mm256_setzero_pd();

    __m256d W = _mm256_setzero_pd();

    __m256i iteration = _mm256_setzero_si256();
    __m256d coordinates_sum = _mm256_setzero_pd();
    __m256d cmp_result = _mm256_setzero_pd();
    __m256d root_radius = _mm256_setzero_pd();
    __m256d radius_max = _mm256_set1_pd(4.0f);
    int64_t mask = 0;
    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        root_radius = _mm256_add_pd(X2, Y2); 
        cmp_result = _mm256_cmp_pd(root_radius, radius_max, _CMP_LE_OS);
        mask = _mm256_movemask_pd(cmp_result);

        if (!mask)
        {
            break;
        }
        
        X = _mm256_sub_pd(X2, Y2);
        X = _mm256_add_pd(X, x0);

        Y = _mm256_sub_pd(W, X2);
        Y = _mm256_sub_pd(Y, Y2);
        Y = _mm256_add_pd(Y, y0);

        X2 = _mm256_mul_pd(X, X);
        Y2 = _mm256_mul_pd(Y, Y);

        coordinates_sum = _mm256_add_pd(X, Y);
        W = _mm256_mul_pd(coordinates_sum, coordinates_sum);

        iteration = _mm256_sub_epi64(iteration, 
                                     _mm256_castpd_si256(cmp_result));
    }
    
    return iteration;
}         


//static __m256i getColorFromIteration(__m256i iterations, const SDL_PixelFormatDetails* format) 
//{
//    assert(format != NULL);
//
//    if (iterations >= MAX_ITERATIONS)
//    {
//        return SDL_MapRGBA(format, NULL, 0, 0, 0, 255);
//    }
//    
//    double t = (double)iterations / MAX_ITERATIONS;
//
//    t = pow(t, 0.5);
//
//    uint8_t r, g, b;
//    if (t < 0.25) {
//        r = 50;
//        g = 4 * t * 255;
//        b = 255;
//    } else if (t < 0.5) {
//        r = 50;
//        g = 255;
//        b = 255 - 4 * (t - 0.25) * 255;
//    } else if (t < 0.75) {
//        r = 4 * (t - 0.5) * 255;
//        g = 255;
//        b = 50;
//    } else {
//        r = 255;
//        g = 255 - 4 * (t - 0.75) * 255;
//        b = 50;
//    }
//
//    uint32_t color = SDL_MapRGBA(format, NULL, r, g, b, 255);
//    return color;
//}



