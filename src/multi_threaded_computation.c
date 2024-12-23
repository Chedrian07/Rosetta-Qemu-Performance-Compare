#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define THREAD_COUNT 8
#define SIZE 20000000

long long globalSum = 0;
int globalMax = 0;
int *arr;
pthread_mutex_t sumLock, maxLock;

// 스레드가 수행할 작업: 배열의 일부분 합계 및 최대값 계산
void* compute_partial(void* arg) {
    long thread_id = (long)arg;
    long start = thread_id * (SIZE / THREAD_COUNT);
    long end = start + (SIZE / THREAD_COUNT);
    long long localSum = 0;
    int localMax = 0;

    for(long i = start; i < end; i++) {
        localSum += arr[i];
        if(arr[i] > localMax) {
            localMax = arr[i];
        }
    }

    // 글로벌 합계 업데이트
    pthread_mutex_lock(&sumLock);
    globalSum += localSum;
    pthread_mutex_unlock(&sumLock);

    // 글로벌 최대값 업데이트
    pthread_mutex_lock(&maxLock);
    if(localMax > globalMax) {
        globalMax = localMax;
    }
    pthread_mutex_unlock(&maxLock);

    pthread_exit(NULL);
}

int main() {
    srand((unsigned)time(NULL));
    arr = malloc(sizeof(int) * SIZE);
    if(arr == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    // 배열 초기화
    for(int i = 0; i < SIZE; i++) {
        arr[i] = rand() % 100000;
    }

    pthread_t threads[THREAD_COUNT];
    pthread_mutex_init(&sumLock, NULL);
    pthread_mutex_init(&maxLock, NULL);

    clock_t start = clock();

    // 스레드 생성 및 작업 할당
    for(long t = 0; t < THREAD_COUNT; t++) {
        int rc = pthread_create(&threads[t], NULL, compute_partial, (void*)t);
        if(rc) {
            fprintf(stderr, "Error: Unable to create thread %ld\n", t);
            exit(EXIT_FAILURE);
        }
    }

    // 모든 스레드가 작업을 완료할 때까지 대기
    for(int t = 0; t < THREAD_COUNT; t++) {
        pthread_join(threads[t], NULL);
    }

    clock_t end = clock();

    printf("Multi-Threaded Computation: sum=%lld, max=%d, took %.3f seconds\n",
           globalSum, globalMax, (double)(end - start)/CLOCKS_PER_SEC);

    // 자원 해제
    pthread_mutex_destroy(&sumLock);
    pthread_mutex_destroy(&maxLock);
    free(arr);

    return 0;
}