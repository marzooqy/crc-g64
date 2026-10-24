#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../crc.h"

const char *test_data = "123456789";
const uint64_t LEN = 10000;

int main() {
    uint64_t value, value2;
    bool success = true;

    unsigned char* test_data2 = (unsigned char*) malloc(LEN);

    for(uint64_t i = 0; i < LEN; i++) {
        test_data2[i] = i & 0xff;
    }

    //test crc_table
    value = crc_table(crc_init, (unsigned char*) test_data, 9);
    printf("Table     %lx %lx ", value, (uint64_t)CHECK);
    printf("%s\n", value == CHECK ? "true" : "false");
    success &= value == CHECK;

    //small len
    value = crc_g64(crc_init, test_data2, 10);
    value2 = crc_table(crc_init, test_data2, 10);
    printf("Small len %lx %lx ", value, value2);
    printf("%s\n", value == value2 ? "true" : "false");
    success &= value == value2;

    //large len
    value = crc_g64(crc_init, test_data2, LEN);
    value2 = crc_table(crc_init, test_data2, LEN);
    printf("Large len %lx %lx ", value, value2);
    printf("%s\n", value == value2 ? "true" : "false");
    success &= value == value2;

    //chunked
    value = crc_g64(crc_init, test_data2, 1000);
    value = crc_g64(value, test_data2 + 1000, LEN - 1000);
    value2 = crc_table(crc_init, test_data2, LEN);
    printf("Chunked   %lx %lx ", value, value2);
    printf("%s\n", value == value2 ? "true" : "false");
    success &= value == value2;

    //unaligned
    for(uint8_t i = 1; i < 8; i++) {
        value = crc_g64(crc_init, test_data2 + i, LEN - i);
        value2 = crc_table(crc_init, test_data2 + i, LEN - i);
        if(value != value2) {
            printf("Unaligned  %lx %lx ", value, value2);
            printf("%s\n", value == value2 ? "true" : "false");
        }
        success &= value == value2;
    }

    free(test_data2);

    if(success) {
        return 0;
    } else {
        return 1;
    }
}