#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// DJB 해시
unsigned long djbHash(const char* str){
    unsigned long hash = 5381;
    int c;
    while((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

// CRC32 테이블 생성 (단순 예시)
unsigned long crc32_table[256];
void init_crc32_table(){
    for(unsigned long i=0; i<256; i++){
        unsigned long c = i;
        for(int j=0; j<8; j++){
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        }
        crc32_table[i] = c;
    }
}

unsigned long crc32(const char *buf, size_t len){
    unsigned long crc = 0xFFFFFFFF;
    for(size_t i=0; i<len; i++){
        unsigned char index = (unsigned char)((crc ^ buf[i]) & 0xFF);
        crc = (crc >> 8) ^ crc32_table[index];
    }
    return crc ^ 0xFFFFFFFF;
}

int main(){
    init_crc32_table();
    char testStr[100];
    clock_t start = clock();
    for(int i=0; i<20000000; i++){
        sprintf(testStr, "HashTest%d", i);
        // DJB 해시
        volatile unsigned long h1 = djbHash(testStr);
        // CRC32 해시
        volatile unsigned long h2 = crc32(testStr, strlen(testStr));
    }
    clock_t end = clock();
    printf("Hashing (DJB & CRC32) took %.3f seconds\n",
           (double)(end - start)/CLOCKS_PER_SEC);
    return 0;
}