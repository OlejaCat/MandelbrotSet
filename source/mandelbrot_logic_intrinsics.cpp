#include "mandelbrot_logic_intrinsics.h"

#include <immintrin.h>
#include <assert.h>

#include "screen_constants.h"
#include "mandelbrot_utils.h"


// static ----------------------------------------------------------------------


static inline __m256i calculateIterationsFromPositionIntrinsics(__m256d x0, __m256d y0);
static inline __m128i calculateIterationsFromPositionIntrinsicsCastIter(__m256d x0, __m256d y0);
static __m256i calculateIterationsFromPositionIntrinsics(__m256d x0, __m256d y0);


// public ----------------------------------------------------------------------


void calculateMandelbrotIntrinsicsSeparated(int pitch,
                                            uint32_t* pixels,
                                            MandelbrotData* data)
{
    assert(data   != NULL);
    assert(pixels != NULL);
    assert((uintptr_t)pixels % 32 == 0 && "pixels must be 32-byte aligned");
    assert((uintptr_t)data->colors % 32 == 0 && "color palette must be 32-byte aligned");
    assert((uintptr_t)data->iterations_per_pixel % 32 == 0 && "iterations filed must be 32-byte aligned");

    int pitch_u32 = pitch / sizeof(uint32_t);
    int* field = data->iterations_per_pixel;

    calculateIterationsFieldIntrinsics(data);
    for (int y = 0; y < SCREEN_HEIGHT; y++)    
    {
        for (int x = 0; x < SCREEN_WIDTH; x += 8) 
        {
            __m256i iterations = _mm256_load_si256((__m256i*)(field + y * SCREEN_WIDTH + x));
            __m256i indices = _mm256_and_si256(iterations, _mm256_set1_epi32(MAX_ITERATIONS - 1));
            __m256i colors = _mm256_i32gather_epi32(
                (const int*)data->colors,
                indices,
                sizeof(uint32_t)
            );

            _mm256_store_si256(
                (__m256i*)(pixels + y * pitch_u32 + x),
                colors
            );
        }
    }
}


void calculateIterationsFieldIntrinsics(MandelbrotData* data)
{
    assert(data != NULL);
    assert((uintptr_t)data->colors % 32 == 0 && "color palette must be 32-byte aligned");
    assert((uintptr_t)data->iterations_per_pixel % 32 == 0 && "iterations field must be 32-byte aligned");

    int* field = data->iterations_per_pixel;

    const double dx = data->width / SCREEN_WIDTH;
    const double dy = data->height / SCREEN_HEIGHT;
    const __m256d offset_x = _mm256_set1_pd(data->center_x - data->width / 2);

    for (int y = 0; y < SCREEN_HEIGHT; y++) 
    {
        const double norm_y = (SCREEN_HEIGHT - y) * dy - data->height / 2 + data->center_y;
        const __m256d y0 = _mm256_set1_pd(norm_y);
        
        for (int x = 0; x < SCREEN_WIDTH; x += 8) 
        {
            __m256d x_pixels1 = _mm256_add_pd(
                _mm256_set1_pd(x),
                _mm256_set_pd(3.0, 2.0, 1.0, 0.0)
            );

            __m256d x_pixels2 = _mm256_add_pd(
                _mm256_set1_pd(x),
                _mm256_set_pd(7.0, 6.0, 5.0, 4.0)
            );

            __m256d x01 = _mm256_fmadd_pd(x_pixels1, _mm256_set1_pd(dx), offset_x);
            __m128i iterations1 = calculateIterationsFromPositionIntrinsicsCastIter(x01, y0);

            __m256d x02 = _mm256_fmadd_pd(x_pixels2, _mm256_set1_pd(dx), offset_x);
            __m128i iterations2 = calculateIterationsFromPositionIntrinsicsCastIter(x02, y0);

            _mm_store_si128(
                (__m128i*)(field + y * SCREEN_WIDTH + x),
                iterations1
            );

            _mm_store_si128(
                (__m128i*)(field + y * SCREEN_WIDTH + x + 4),
                iterations2
            );
        }
    }
}


// static ----------------------------------------------------------------------


static inline __m128i calculateIterationsFromPositionIntrinsicsCastIter(__m256d x0, __m256d y0) 
{
    __m256i iterations = calculateIterationsFromPositionIntrinsics(x0, y0);
    __m128i low = _mm256_castsi256_si128(iterations);
    __m128i high = _mm256_extracti128_si256(iterations, 1);

    return _mm_setr_epi32(
        _mm_cvtsi128_si32(low),
        _mm_extract_epi32(low, 2),
        _mm_cvtsi128_si32(high),
        _mm_extract_epi32(high, 2)
    ); 
}


static inline __m256i calculateIterationsFromPositionIntrinsics(__m256d x0, __m256d y0)
{
    __m256d x2 = _mm256_setzero_pd();
    __m256d y2 = _mm256_setzero_pd();
    __m256d w  = _mm256_setzero_pd();

    __m256i iterations = _mm256_setzero_si256();
    const __m256d max_radius = _mm256_set1_pd(4.0);
    
    for (int i = 0; i < MAX_ITERATIONS; i++) {
        __m256d mask = _mm256_cmp_pd(_mm256_add_pd(x2, y2), max_radius, _CMP_LE_OQ);

        if (!_mm256_movemask_pd(mask)) 
        {
            break;
        }
        
        __m256d x = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
        __m256d y = _mm256_add_pd(_mm256_sub_pd(_mm256_sub_pd(w, x2), y2), y0);

        w = _mm256_mul_pd(_mm256_add_pd(x, y), _mm256_add_pd(x, y));

        x2 = _mm256_mul_pd(x, x);
        y2 = _mm256_mul_pd(y, y);

        iterations = _mm256_sub_epi64(iterations, _mm256_castpd_si256(mask));
    }

    return iterations;
}

