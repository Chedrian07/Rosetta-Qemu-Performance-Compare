#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define THREAD_COUNT 8
#define SIZE 20000000

static long long globalSum = 0;
static int globalMax = 0;
int *arr;
pthread_mutex_t sumLock, maxLock;

void* partialComputation(void* arg) {
    long start = (long)arg;
    long end = start + SIZE/THREAD_COUNT;
    long long localSum = 0;
    int localMax = 0;

    for(long i = start; i < end; i++){
        localSum += arr[i];
        if(arr[i] > localMax) {
            localMax = arr[i];
        }
    }

    // 합산 업데이트
    pthread_mutex_lock(&sumLock);
    globalSum += localSum;
    pthread_mutex_unlock(&sumLock);

    // 최대값 업데이트
    pthread_mutex_lock(&maxLock);
    if(localMax > globalMax) {
        globalMax = localMax;
    }
    pthread_mutex_unlock(&maxLock);

    return NULL;
}

int main(){
    srand((unsigned)time(NULL));
    arr = malloc(sizeof(int)*SIZE);
    for(int i=0; i<SIZE; i++){
        arr[i] = rand() % 100000;
    }

    pthread_t threads[THREAD_COUNT];
    pthread_mutex_init(&sumLock, NULL);
    pthread_mutex_init(&maxLock, NULL);

    clock_t start = clock();
    for(long i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threads[i], NULL, partialComputation, (void*)(i*(SIZE/THREAD_COUNT)));
    }
    for(int i = 0; i < THREAD_COUNT; i++){
        pthread_join(threads[i], NULL);
    }
    clock_t end = clock();

    printf("Parallel: sum=%lld, max=%d, took %.3f seconds\n",
           globalSum, globalMax, (double)(end - start)/CLOCKS_PER_SEC);

    pthread_mutex_destroy(&sumLock);
    pthread_mutex_destroy(&maxLock);
    free(arr);
    return 0;
}