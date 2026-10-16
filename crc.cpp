#include <array>
#include <stdio.h>
#include "crc.h"

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
                crc = (crc >> 1) ^ (crc & 1 ? poly : 0);
            }
        } else {
            crc <<= 56;
            for(uint8_t j = 0; j < 8; j++) {
                crc = (crc << 1) ^ (crc >> 63 ? poly : 0);
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

//Returns either the value to be subtracted or zero depending on the bit of POLY.
#define sub(value, bit) (((POLY >> (bit)) & 1) ? (value) : 0)

uint64_t crc_g64(uint64_t crc, unsigned char const *buf, uint64_t len) {
    const uint64_t N = 2;
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

    if(len >= (WIDTH + N) * 8) {
        uint64_t const *ptr = (uint64_t const*)buf;
        uint64_t blocks[WIDTH + N];
        uint64_t *out = blocks;
        uint64_t *in = blocks + N;

        for(uint8_t i = 0; i < WIDTH; i++) {
            in[i] = *ptr++;
        }
        in[0] ^= crc;

        len -= WIDTH * 8;

        while(len >= N * 8) {
            out[0] = in[0];
            out[1] = in[1] ^ sub(out[0], WIDTH - 1);

            for(uint8_t i = 0; i < WIDTH - N; i++) {
                in[i] = in[i + N] ^ sub(out[0], WIDTH - i - 2) ^ sub(out[1], WIDTH - i - 1);
            }

            in[WIDTH - 2] = (*ptr++) ^ sub(out[0], 0) ^ sub(out[1], 1);
            in[WIDTH - 1] = (*ptr++) ^ sub(out[1], 0);

            len -= N * 8;
        }

        crc = crc_zeros(in[0]);
        for(uint8_t i = 1; i < WIDTH; i++) {
            crc = crc_zeros(crc ^ in[i]);
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