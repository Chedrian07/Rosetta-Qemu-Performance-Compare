#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALLOC_COUNT 20000000

int main(){
    clock_t start = clock();
    srand((unsigned)time(NULL));

    for(int i=0; i<ALLOC_COUNT; i++){
        // 1~1024 byte 사이 랜덤 할당
        size_t size = rand()%1024 + 1;
        char *p = (char*)malloc(size);
        if(p){
            // 임의 접근
            p[0] = 'x';  
            // 일부만 free하여 의도적 누수
            if(rand()%5 != 0){
                free(p);
            }
        }
    }

    clock_t end = clock();
    printf("Memory allocation test (some leaks) took %.3f seconds\n",
           (double)(end - start)/CLOCKS_PER_SEC);
    return 0;
}