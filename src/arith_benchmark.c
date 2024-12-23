#include <stdio.h>
#include <math.h>
#include <time.h>

int main() {
    clock_t start = clock();
    volatile double result = 0.0;
    for(int i = 0; i < 200000000; i++){
        result += i;
        result *= 1.000001;
        result /= 1.0000001;
        result = sin(result) + cos(result);
        result = pow(result, 1.0001);
    }
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Arith Benchmark took %.3f seconds, result=%f\n", elapsed, result);
    return 0;
}