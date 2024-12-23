#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 30000

void bubbleSort(int arr[], int n) {
    for(int i=0; i<n-1; i++){
        for(int j=0; j<n-1-i; j++){
            if(arr[j] > arr[j+1]){
                int temp = arr[j];
                arr[j]   = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

void quickSort(int arr[], int left, int right){
    if(left >= right) return;
    int pivot = arr[(left + right)/2];
    int i = left, j = right;
    while(i <= j){
        while(arr[i] < pivot) i++;
        while(arr[j] > pivot) j--;
        if(i <= j){
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            i++; j--;
        }
    }
    quickSort(arr, left, j);
    quickSort(arr, i, right);
}

int main(){
    srand((unsigned)time(NULL));
    
    // 큰 배열 생성
    int *arr1 = malloc(sizeof(int)*SIZE);
    int *arr2 = malloc(sizeof(int)*SIZE);
    for(int i=0; i<SIZE; i++) {
        int val = rand();
        arr1[i] = val;
        arr2[i] = val;
    }

    clock_t start = clock();
    bubbleSort(arr1, SIZE);
    clock_t mid = clock();
    quickSort(arr2, 0, SIZE-1);
    clock_t end = clock();

    printf("Bubble Sort took %.3f seconds\n", 
           (double)(mid - start) / CLOCKS_PER_SEC);
    printf("Quick Sort took %.3f seconds\n", 
           (double)(end - mid) / CLOCKS_PER_SEC);

    free(arr1);
    free(arr2);
    return 0;
}