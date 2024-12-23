#include <stdio.h>
#include <time.h>
#include <stdlib.h>

unsigned long long fib_pure(int n){
    if(n<2) return n;
    return fib_pure(n-1) + fib_pure(n-2);
}

// 메모이제이션 버전
unsigned long long fib_memo(int n, unsigned long long *memo){
    if(n<2) return n;
    if(memo[n] != 0) return memo[n];
    memo[n] = fib_memo(n-1, memo) + fib_memo(n-2, memo);
    return memo[n];
}

int main(){
    int n=45;
    unsigned long long *memo = calloc(n+1, sizeof(unsigned long long));

    clock_t startPure = clock();
    unsigned long long resultPure = fib_pure(n);
    clock_t endPure = clock();

    clock_t startMemo = clock();
    unsigned long long resultMemo = fib_memo(n, memo);
    clock_t endMemo = clock();

    printf("[Pure Recursion] Fib(%d)=%llu, took %.3f seconds\n",
           n, resultPure, (double)(endPure - startPure)/CLOCKS_PER_SEC);
    printf("[Memoization]   Fib(%d)=%llu, took %.3f seconds\n",
           n, resultMemo, (double)(endMemo - startMemo)/CLOCKS_PER_SEC);

    free(memo);
    return 0;
}