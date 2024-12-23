#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CHUNK_SIZE 1024*1024  // 1MB
#define WRITE_COUNT 100       // 100 MB
#define READ_ITER   3         // 파일 읽기 반복 횟수

int main(){
    FILE *fp;
    clock_t start = clock();
    char *data = malloc(CHUNK_SIZE);

    // 임의 데이터 채우기
    for(int i=0; i<CHUNK_SIZE; i++){
        data[i] = (char)(rand() % 256);
    }

    // 쓰기
    fp = fopen("test_file.bin","wb");
    if(!fp) {
        perror("fopen write");
        free(data);
        return 1;
    }
    for(int i=0; i<WRITE_COUNT; i++){
        fwrite(data, 1, CHUNK_SIZE, fp);
    }
    fclose(fp);

    // 읽기 (여러 번)
    for(int r=0; r<READ_ITER; r++){
        fp = fopen("test_file.bin","rb");
        if(!fp) {
            perror("fopen read");
            free(data);
            return 1;
        }
        while(!feof(fp)){
            fread(data, 1, CHUNK_SIZE, fp);
        }
        fclose(fp);
    }

    clock_t end = clock();
    printf("File I/O (100MB write, read %d times) took %.3f seconds\n",
           READ_ITER, (double)(end - start)/CLOCKS_PER_SEC);

    free(data);
    return 0;
}