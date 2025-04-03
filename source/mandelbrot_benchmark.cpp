#include "mandelbrot_benchmark.h"

#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <time.h>

#include "mandelbrot_utils.h"
#include "mandelbrot_logic_basic.h"
#include "mandelbrot_logic_intrinsics.h"
#include "mandelbrot_logic_array.h"


int main()
{
    Benchmark tests[] = {
        (Benchmark){
            .mandelbrot_func = calculateMandelbrot,
            .name = "basic version",
            .file_path = "results/basic_version.txt",
            .graphic_title = "Версия без оптимизаций",
            .warmup_runs = 50,
            .measure_runs = 50
        }
        //(Benchmark){
        //    .mandelbrot_func = calculateMandelbrotIntrinsics,
        //    .name = "simd version",
        //    .file_path = "results/simd_version.txt",
        //    .graphic_title = "Версия с SIMD инструкциями",
        //    .warmup_runs = 50,
        //    .measure_runs = 500
        //},
        //(Benchmark){
        //    .mandelbrot_func = calculateMandelbrotArray,
        //    .name = "array version",
        //    .file_path = "results/array_version.txt",
        //    .graphic_title = "Версия работающая на массивах",
        //    .warmup_runs = 50,
        //    .measure_runs = 500
        //} 
    };

    const int number_of_tests = sizeof(tests) / sizeof(Benchmark);

    for (int i = 0; i < number_of_tests; i++)
    {
        uint64_t* results = (uint64_t*)calloc(tests[i].measure_runs, sizeof(uint64_t));

        printf("Running %s...\n", tests[i].name);
        clock_t begin = clock();

        runBenchmark(&tests[i], results);
        saveResults(&tests[i], results);

        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        printf("Tests took: %g\n", time_spent);

        free(results);
    }
}

void runBenchmark(Benchmark* config, uint64_t* results)
{
    MandelbrotData mandelbrot_data = {};
    setDefaultMandelbrot(&mandelbrot_data);

    uint32_t* pixels = NULL;
    posix_memalign((void**)&pixels, 64, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    if (!pixels)
    {
        fprintf(stderr, "Error while allocating memory for testing\n");
        return;
    }

    for (int i = 0; i < config->measure_runs; i++)
    {
        uint32_t _; 
        uint32_t start = _rdtscp(&_);
        config->mandelbrot_func(SCREEN_WIDTH * sizeof(uint32_t), pixels, &mandelbrot_data);
        uint32_t end = _rdtscp(&_);
        results[i] = end - start;
    }

    free(pixels);
}


void saveResults(Benchmark* config, uint64_t* results)
{
    FILE* file = fopen(config->file_path, "w");
    if (!file)
    {
        fprintf(stderr, "Error while opening file");
        return;
    }

    fprintf(file, "%s\n", config->graphic_title);

    for (int i = 0; i < config->measure_runs; i++)
    {
        fprintf(file, "%lu\n", results[i]);
    }
}

