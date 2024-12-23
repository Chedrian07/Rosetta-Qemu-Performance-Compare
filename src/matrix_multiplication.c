#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 300  // 행렬 크기 증가
#define REP 3  // 반복 횟수

int main(){
    srand((unsigned)time(NULL));
    int **A = malloc(N*sizeof(int*));
    int **B = malloc(N*sizeof(int*));
    int **C = malloc(N*sizeof(int*));

    for(int i=0; i<N; i++){
        A[i] = malloc(N*sizeof(int));
        B[i] = malloc(N*sizeof(int));
        C[i] = malloc(N*sizeof(int));
        for(int j=0; j<N; j++){
            A[i][j] = rand() % 100;
            B[i][j] = rand() % 100;
        }
    }

    clock_t start = clock();
    // 여러 번 반복하여 행렬 곱셈 수행
    for(int r=0; r<REP; r++){
        for(int i=0; i<N; i++){
            for(int j=0; j<N; j++){
                int sum = 0;
                for(int k=0; k<N; k++){
                    sum += A[i][k] * B[k][j];
                }
                C[i][j] = sum;
            }
        }
    }
    clock_t end = clock();

    printf("Matrix Multiplication (size=%d) repeated %d times took %.3f seconds\n", 
           N, REP, (double)(end - start) / CLOCKS_PER_SEC);

    for(int i=0; i<N; i++){
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);
    return 0;
}