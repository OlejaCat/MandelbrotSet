#ifndef TESTER_H
#define TESTER_H

#include <SDL

typedef struct BenchmarkResults
{
    double   mean;
    double   sigma;
    uint64_t min;
    uint64_t max;
} BenchmarkResults;

BenchmarkResults benchmark()

#endif
