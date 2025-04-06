#ifndef MANDELBROT_BENCHMARK_H
#define MANDELBROT_BENCHMARK_H

#include <stdint.h>

#include "mandelbrot_struct.h"

//typedef void (*MandelbrotFunction)(int pitch, uint32_t* pixels, MandelbrotData* data);
typedef void (*MandelbrotFunction)(MandelbrotData* data);

typedef struct Benchmark
{
    MandelbrotFunction mandelbrot_func;
    const char* name;
    const char* file_path;
    const char* graphic_title;
    int warmup_runs;
    int measure_runs;
} Benchmark;

void runBenchmark(Benchmark* config, uint64_t* results);
void saveResults(Benchmark* config, uint64_t* results);

#endif // MANDELBROT_BENCHMARK_H
