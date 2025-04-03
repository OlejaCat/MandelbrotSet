#include "mandelbrot_logic_intrinsics.h"

#include <immintrin.h>
#include <assert.h>

#include "screen_constants.h"
#include "mandelbrot_utils.h"


// static ----------------------------------------------------------------------


static __m256i calculateColorsFromIterations(__m128i iter_low, 
                                                    __m128i iter_high, 
                                                    MandelbrotData* data);
static __m128i calculateIterationsFromPositionIntrinsics(__m256d x0, __m256d y0);


// public ----------------------------------------------------------------------


void calculateMandelbrotIntrinsics(int pitch, 
                                   uint32_t* pixels,
                                   MandelbrotData* data) 
{
    assert(data   != NULL);
    assert(pixels != NULL);
    assert((uintptr_t)pixels % 32 == 0 && "pixels must be 32-byte aligned");
    assert((uintptr_t)data->colors % 32 == 0 && "color palette must be 32-byte aligned");

    const int pitch_u32 = pitch / sizeof(uint32_t);
    const double dx = data->width / SCREEN_WIDTH;
    const double dy = data->height / SCREEN_HEIGHT;
    const __m256d offset_x = _mm256_set1_pd(data->center_x - data->width / 2);

    for (int y = 0; y < SCREEN_HEIGHT; y++) 
    {
        const double norm_y = (SCREEN_HEIGHT - y) * dy - data->height / 2 + data->center_y;
        const __m256d y0 = _mm256_set1_pd(norm_y);
        
        for (int x = 0; x < SCREEN_WIDTH; x += 8) 
        {
            __m256d x_pixels1 = _mm256_setr_pd(
                    (double)x, 
                    (double)(x + 1), 
                    (double)(x + 2), 
                    (double)(x + 3)
            );
            __m256d x01 = _mm256_fmadd_pd(x_pixels1, _mm256_set1_pd(dx), offset_x);
            __m128i iter_low = calculateIterationsFromPositionIntrinsics(x01, y0);

            __m256d x_pixels2 = _mm256_setr_pd(
                    (double)(x + 4), 
                    (double)(x + 5), 
                    (double)(x + 6), 
                    (double)(x + 7)
            );
            __m256d x02 = _mm256_fmadd_pd(x_pixels2, _mm256_set1_pd(dx), offset_x);
            __m128i iter_high = calculateIterationsFromPositionIntrinsics(x02, y0);

            __m256i colors = calculateColorsFromIterations(iter_low, iter_high, data);

            _mm256_storeu_si256((__m256i*)(pixels + y * pitch_u32 + x), colors);
        }
    }
}



// static ----------------------------------------------------------------------


static __m128i calculateIterationsFromPositionIntrinsics(__m256d x0, __m256d y0) 
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

    __m128i low = _mm256_castsi256_si128(iterations);
    __m128i high = _mm256_extracti128_si256(iterations, 1);

    return _mm_setr_epi32(
        _mm_cvtsi128_si32(low),
        _mm_extract_epi32(low, 2),
        _mm_cvtsi128_si32(high),
        _mm_extract_epi32(high, 2)
    ); 
}


static __m256i calculateColorsFromIterations(__m128i iter_low, 
                                                    __m128i iter_high, 
                                                    MandelbrotData* data)
{
    assert(data != NULL);

    __m256i iterations = _mm256_setr_epi32(
            _mm_extract_epi32(iter_low, 0),
            _mm_extract_epi32(iter_low, 1),
            _mm_extract_epi32(iter_low, 2),
            _mm_extract_epi32(iter_low, 3),
            _mm_extract_epi32(iter_high, 0),
            _mm_extract_epi32(iter_high, 1),
            _mm_extract_epi32(iter_high, 2),
            _mm_extract_epi32(iter_high, 3)
    );

    __m256i colors = _mm256_i32gather_epi32(
            (const int*)data->colors,
            _mm256_and_si256(iterations, _mm256_set1_epi32(0x1FF)),
            sizeof(uint32_t)
    );

    return colors;
}


