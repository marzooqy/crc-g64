#include <array>
#include "crc.h"

//Number of words to calculate in each iteration.
#define N 8

#if N > WIDTH
#error N must be smaller than WIDTH.
#endif

#if N > 8
#error Maximum supported value of N is 8.
#endif

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
    const uint64_t poly = REFIN ? reflect(POLY, WIDTH) : (uint64_t)POLY << (64 - WIDTH);
    std::array<uint64_t, 256> table = {};

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
    for(uint8_t i = 0; i < 8; i++) {
        crc = (crc >> 8) ^ table[crc & 0xff];
    }
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

//Perform all N subtractions for the word located in i.
uint64_t subs(uint64_t *in, uint64_t *out, uint8_t i) {
    uint64_t result = in[i + N];
    for(uint8_t n = 0; n < N; n++) {
        result ^= sub(out[n], WIDTH - i - (N - n));
    }
    return result;
}

#define REPEAT_63(x) x x x x x x x x x x x x x x x x \
                     x x x x x x x x x x x x x x x x \
                     x x x x x x x x x x x x x x x x \
                     x x x x x x x x x x x x x x x

uint64_t crc_g64(uint64_t crc, unsigned char const *buf, uint64_t len) {
    crc = crc_initial(crc);

    //Align the buffer to a word boundary.
    uint64_t off = (uintptr_t)buf & 0x7;
    uint64_t rem = (8 - off) & 0x7;
    if(rem < len) {
        crc = crc_bytes(crc, buf, rem);
        buf += rem;
        len -= rem;
    }

    if(len >= (WIDTH + N) * 8) {
        uint64_t const *ptr = (uint64_t const*)buf;
        uint64_t blocks[WIDTH + N];
        uint64_t *out = blocks;
        uint64_t *in = blocks + N;
        uint8_t x;

        //Load the first WIDTH + N words.
        for(uint8_t i = 0; i < WIDTH; i++) {
            in[i] = *ptr++;
        }
        in[0] ^= crc;

        len -= WIDTH * 8;

        //Reduce the buffer using P(x)^64, N words at a time.
        while(len >= N * 8) {
            out[0] = in[0];
            #if N >= 2
            out[1] = in[1] ^ sub(out[0], WIDTH - 1);
            #endif
            #if N >= 3
            out[2] = in[2] ^ sub(out[0], WIDTH - 2) ^ sub(out[1], WIDTH - 1);
            #endif
            #if N >= 4
            out[3] = in[3] ^ sub(out[0], WIDTH - 3) ^ sub(out[1], WIDTH - 2) ^ sub(out[2], WIDTH - 1);
            #endif
            #if N >= 5
            out[4] = in[4] ^ sub(out[0], WIDTH - 4) ^ sub(out[1], WIDTH - 3) ^ sub(out[2], WIDTH - 2) ^ sub(out[3], WIDTH - 1);
            #endif
            #if N >= 6
            out[5] = in[5] ^ sub(out[0], WIDTH - 5) ^ sub(out[1], WIDTH - 4) ^ sub(out[2], WIDTH - 3) ^ sub(out[3], WIDTH - 2) ^ sub(out[4], WIDTH - 1);
            #endif
            #if N >= 7
            out[6] = in[6] ^ sub(out[0], WIDTH - 6) ^ sub(out[1], WIDTH - 5) ^ sub(out[2], WIDTH - 4) ^ sub(out[3], WIDTH - 3) ^ sub(out[4], WIDTH - 2) ^ sub(out[5], WIDTH - 1);
            #endif
            #if N >= 8
            out[7] = in[7] ^ sub(out[0], WIDTH - 7) ^ sub(out[1], WIDTH - 6) ^ sub(out[2], WIDTH - 5) ^ sub(out[3], WIDTH - 4) ^ sub(out[4], WIDTH - 3) ^ sub(out[5], WIDTH - 2) ^ sub(out[6], WIDTH - 1);
            #endif

            x = 0;

            REPEAT_63(
                if(x < WIDTH - N) {
                    in[x] = subs(in, out, x);
                    x++;
                }
            )

            #if N >= 8
            in[WIDTH - 8] = (*ptr++) ^ sub(out[N - 8], 0) ^ sub(out[N - 7], 1) ^ sub(out[N - 6], 2) ^ sub(out[N - 5], 3) ^ sub(out[N - 4], 4) ^ sub(out[N - 3], 5) ^ sub(out[N - 2], 6) ^ sub(out[N - 1], 7);
            #endif
            #if N >= 7
            in[WIDTH - 7] = (*ptr++) ^ sub(out[N - 7], 0) ^ sub(out[N - 6], 1) ^ sub(out[N - 5], 2) ^ sub(out[N - 4], 3) ^ sub(out[N - 3], 4) ^ sub(out[N - 2], 5) ^ sub(out[N - 1], 6);
            #endif
            #if N >= 6
            in[WIDTH - 6] = (*ptr++) ^ sub(out[N - 6], 0) ^ sub(out[N - 5], 1) ^ sub(out[N - 4], 2) ^ sub(out[N - 3], 3) ^ sub(out[N - 2], 4) ^ sub(out[N - 1], 5);
            #endif
            #if N >= 5
            in[WIDTH - 5] = (*ptr++) ^ sub(out[N - 5], 0) ^ sub(out[N - 4], 1) ^ sub(out[N - 3], 2) ^ sub(out[N - 2], 3) ^ sub(out[N - 1], 4);
            #endif
            #if N >= 4
            in[WIDTH - 4] = (*ptr++) ^ sub(out[N - 4], 0) ^ sub(out[N - 3], 1) ^ sub(out[N - 2], 2) ^ sub(out[N - 1], 3);
            #endif
            #if N >= 3
            in[WIDTH - 3] = (*ptr++) ^ sub(out[N - 3], 0) ^ sub(out[N - 2], 1) ^ sub(out[N - 1], 2);
            #endif
            #if N >= 2
            in[WIDTH - 2] = (*ptr++) ^ sub(out[N - 2], 0) ^ sub(out[N - 1], 1);
            #endif
            in[WIDTH - 1] = (*ptr++) ^ sub(out[N - 1], 0);

            len -= N * 8;
        }

        //Find the CRC of the reduced buffer.
        crc = crc_zeros(in[0]);
        for(uint8_t i = 1; i < WIDTH; i++) {
            crc = crc_zeros(crc ^ in[i]);
        }

        buf = (unsigned char const*)ptr;
    }

    //Calculate the remaining bytes.
    crc = crc_bytes(crc, buf, len);
    return crc_final(crc);
}