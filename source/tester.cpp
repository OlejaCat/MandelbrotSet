#include <x86intrin.h>
#include <stdio.h>
#include <stdint.h>


int main()
{
    uint64_t i = 0;
    unsigned int ui;
    i = __rdtscp(&ui);
    printf("%llu ticks\n", i);
    int64_t res = 0;
    for (int j = 0; j < 1000000; j++)
    {
        res += j * 10; 
    }
    i = __rdtscp(&ui);
    printf("%llu ticks\n", i);
    printf("TSC_AUX was %x\n", ui);
}
