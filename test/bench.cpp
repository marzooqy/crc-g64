#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "../crc.h"

const uint64_t len = 64000;
const uint64_t n = 10000;

int main() {
    unsigned char* buf = (unsigned char*) malloc(len);

    for(uint64_t i = 0; i < len; i++) {
        buf[i] = i & 0xff;
    }

    clock_t start = clock();

    for(uint64_t i = 0; i < n; i++) {
        crc_g64(crc_init, buf, len);
    }

    clock_t end = clock();

    float speed = (float)(len * n) / ((float)(end - start) / CLOCKS_PER_SEC) / powf(1024, 3);

    printf("%.2f", speed);

    free(buf);
}