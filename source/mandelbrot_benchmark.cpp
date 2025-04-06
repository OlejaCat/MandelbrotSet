#include "mandelbrot_benchmark.h"

#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "mandelbrot_utils.h"
#include "mandelbrot_logic_basic.h"
#include "mandelbrot_logic_intrinsics.h"
#include "mandelbrot_logic_array.h"


int main()
{
    int which = PRIO_PROCESS;
    id_t pid = getpid();
    int priority = -20;
    setpriority(which, pid, priority);

    Benchmark tests[] = {
        (Benchmark){
            .mandelbrot_func = calculateIterationField,
            .name = "only iterations basic version O3",
            .file_path = "only_iterations_basic_version_O3.txt",
            .graphic_title = "Версия без оптимизаий -O3",
            .warmup_runs = 10000,
            .measure_runs = 2000
        },
        (Benchmark){
            .mandelbrot_func = calculateIterationsFieldIntrinsics,
            .name = "only iterations simd version -O3",
            .file_path = "results/only_iterations_simd_version_O3.txt",
            .graphic_title = "Версия с SIMD инструкциями -O3",
            .warmup_runs = 200,
            .measure_runs = 10000
        },
        (Benchmark){
            .mandelbrot_func = calculateIterationFieldArray,
            .name = "only iterations array version -O3",
            .file_path = "results/only_iterations_array_version_O3.txt",
            .graphic_title = "Версия работающая на массивах -O3",
            .warmup_runs = 200,
            .measure_runs = 10000
        } 
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
    pixels = (uint32_t*)aligned_alloc(32, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    if (!pixels)
    {
        fprintf(stderr, "Error while allocating memory for testing\n");
        return;
    }

    volatile int temp = 0;
    for (int i = 0; i < config->warmup_runs; i++)
    {
        config->mandelbrot_func(&mandelbrot_data);
        temp++;
    }

    for (int i = 0; i < config->measure_runs; i++)
    {
        uint32_t _; 
        uint32_t start = _rdtscp(&_);
        config->mandelbrot_func(&mandelbrot_data);
        uint32_t end = _rdtscp(&_);
        results[i] = end - start;
        temp++;
        if (temp % 100 == 0)
        {
            printf("%.2f%% done\n", (double)temp / (config->measure_runs + config->warmup_runs) * 100.0);
        }
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

