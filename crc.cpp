#include <array>
#include <stdio.h>
#include "crc.h"

//Convert a boolean into an AND mask.
#define AND_MASK(c) (-(uint64_t)(c))

//Reflect an integer x of width w.
constexpr uint64_t reflect(uint64_t x, uint8_t w) {
    x = ((x >> 32) & 0xffffffff) | ((x << 32) & 0xffffffff00000000);
    x = ((x >> 16) & 0xffff0000ffff) | ((x << 16) & 0xffff0000ffff0000);
    x = ((x >> 8) & 0xff00ff00ff00ff) | ((x << 8) & 0xff00ff00ff00ff00);
    x = ((x >> 4) & 0xf0f0f0f0f0f0f0f) | ((x << 4) & 0xf0f0f0f0f0f0f0f0);
    x = ((x >> 2) & 0x3333333333333333) | ((x << 2) & 0xcccccccccccccccc);
    x = ((x >> 1) & 0x5555555555555555) | ((x << 1) & 0xaaaaaaaaaaaaaaaa);
    return x >> (64 - w);
}

//Swap the bytes of an integer.
constexpr uint64_t swap(uint64_t x) {
    return ((x & 0xff00000000000000) >> 56) | ((x & 0xff) << 56)
         | ((x & 0xff000000000000) >> 40) | ((x & 0xff00) << 40)
         | ((x & 0xff0000000000) >> 24) | ((x & 0xff0000) << 24)
         | ((x & 0xff00000000) >> 8) | ((x & 0xff000000) << 8);
}

//Build the crc table.
constexpr std::array<uint64_t, 256> crc_build_table() {
    std::array<uint64_t, 256> table;

    const uint64_t poly = REFIN ? reflect(POLY, WIDTH) : (uint64_t)POLY << (64 - WIDTH);

    for(uint16_t i = 0; i < 256; i++) {
        uint64_t crc = i;

        if(REFIN) {
            for(uint8_t j = 0; j < 8; j++) {
                crc = (crc >> 1) ^ (poly & AND_MASK(crc & 1));
            }
        } else {
            crc <<= 56;
            for(uint8_t j = 0; j < 8; j++) {
                crc = (crc << 1) ^ (poly & AND_MASK(crc >> 63));
            }
            crc = swap(crc);
        }

        table[i] = crc;
    }

    return table;
}

//Define constants.
const uint64_t crc_init = (REFOUT ? reflect(INIT, WIDTH) : INIT) ^ XOROUT;
const std::array<uint64_t, 256> table = crc_build_table();

//Align the CRC.
static uint64_t crc_initial(uint64_t crc) {
    crc ^= XOROUT;
    if(REFIN ^ REFOUT) {
        crc = reflect(crc, WIDTH);
    }
    if(!REFIN) {
        crc = swap(crc << (64 - WIDTH));
    }
    return crc;
}

//Return the CRC to its original alignment.
static uint64_t crc_final(uint64_t crc) {
    if(!REFIN) {
        crc = swap(crc) >> (64 - WIDTH);
    }
    if(REFIN ^ REFOUT) {
        crc = reflect(crc, WIDTH);
    }
    return crc ^ XOROUT;
}

//Apply 8 zeros to crc.
static uint64_t crc_zeros(uint64_t crc) {
    //printf("Preword %llx\n", crc);
    for(uint8_t i = 0; i < 8; i++) {
        crc = (crc >> 8) ^ table[crc & 0xff];
    }
    //printf("Processed %llx\n", crc);
    return crc;
}

//Compute the CRC of the input buffer byte-by-byte.
static uint64_t crc_bytes(uint64_t crc, unsigned char const *buf, uint64_t len) {
    while(len--) {
        crc = (crc >> 8) ^ table[(crc ^ *buf++) & 0xff];
    }
    return crc;
}

//Calculate the CRC using the crc table.
uint64_t crc_table(uint64_t crc, unsigned char const *buf, uint64_t len) {
    crc = crc_initial(crc);
    crc = crc_bytes(crc, buf, len);
    return crc_final(crc);
}

//Calculate the CRC using the G64 algorithm.

/* Math:
   G(x) = P(x)^64
   R(x) = M(x) mod G(x)
   CRC(x) = R(x)*x^w mod P(x)

   Where:
   P(x) is the polynomial.
   M(x) is the message.
   w is the width of the CRC.
*/

uint64_t crc_g64(uint64_t crc, unsigned char const *buf, uint64_t len) {
    //printf("Start %llx\n", crc);
    crc = crc_initial(crc);
    //printf("Initial %llx\n", crc);
    //Align the buffer to a word boundary.
    uint64_t off = (uintptr_t)buf & 0x7;
    uint64_t rem = (8 - off) & 0x7;
    if(rem < len) {
        crc = crc_bytes(crc, buf, rem);
        buf += rem;
        len -= rem;
    }

    //printf("Mem %llx\n", crc);

    if(len >= 64 * 8 + 8) {
        const uint64_t poly = POLY;
        uint64_t const *ptr = (uint64_t const*)buf;
        uint64_t blocks[WIDTH];
        uint64_t out;

        for(uint8_t i = 0; i < WIDTH; i++) {
            blocks[i] = *ptr++;
        }
        blocks[0] ^= crc;

        len -= WIDTH * 8;

        while(len >= 8) {
            out = blocks[0];

            for(uint8_t i = 0; i < WIDTH - 1; i++) {
                blocks[i] = blocks[i + 1] ^ (((poly >> (WIDTH - i - 1)) & 1) ? out : 0);
            }

            for(uint8_t i = WIDTH - 1; i < WIDTH; i++) {
                blocks[i] = (*ptr++) ^ (((poly >> (WIDTH - i - 1)) & 1) ? out : 0);
            }

            len -= 8;
        }

        crc = crc_zeros(blocks[0]);
        for(uint8_t i = 1; i < WIDTH; i++) {
            crc = crc_zeros(crc ^ blocks[i]);
            //printf("Words %llx\n", crc);
        }

        //printf("End Words %llx\n", crc);

        buf = (unsigned char const*)ptr;
    }

    //Calculate the remaining bytes.
    crc = crc_bytes(crc, buf, len);
    //printf("Bytes %llx\n", crc);
    crc = crc_final(crc);
    //printf("Final %llx\n", crc);
    return crc;
}