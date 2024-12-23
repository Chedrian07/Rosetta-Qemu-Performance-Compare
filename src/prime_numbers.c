#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LIMIT 2000000  // 기존보다 더 큰 범위

int main(){
    clock_t start = clock();

    // 에라토스테네스의 체
    char *prime = calloc(LIMIT+1, sizeof(char));
    for(int i=2; i<=LIMIT; i++){
        prime[i] = 1; // 1이면 소수
    }
    for(int i=2; i*i<=LIMIT; i++){
        if(prime[i]){
            for(int j=i*i; j<=LIMIT; j+=i){
                prime[j] = 0;
            }
        }
    }

    int count = 0;
    for(int i=2; i<=LIMIT; i++){
        if(prime[i]) count++;
    }

    clock_t end = clock();
    printf("Prime calculation up to %d took %.3f seconds, found %d primes\n",
           LIMIT, (double)(end - start)/CLOCKS_PER_SEC, count);

    free(prime);
    return 0;
}