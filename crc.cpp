#include <array>
#include "crc.h"

//Number of words to calculate in each iteration.
#define N 8

#if N > WIDTH
#error N must be smaller than WIDTH.
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

//Manually unrolled loop.
//Different versions of REPEAT are used depending on the values of WIDTH and N to reduce compile-time.
#define REPEAT_8(x)  x x x x x x x x
#define REPEAT_16(x) REPEAT_8(x) REPEAT_8(x)
#define REPEAT_32(x) REPEAT_16(x) REPEAT_16(x)
#define REPEAT_64(x) REPEAT_32(x) REPEAT_32(x)

#if WIDTH <= 8
#define REPEAT_W(x) REPEAT_8(x)
#elif WIDTH <= 16
#define REPEAT_W(x) REPEAT_16(x)
#elif WIDTH <= 32
#define REPEAT_W(x) REPEAT_32(x)
#else
#define REPEAT_W(x) REPEAT_64(x)
#endif

#if N <= 8
#define REPEAT_N(x) REPEAT_8(x)
#elif N <= 16
#define REPEAT_N(x) REPEAT_16(x)
#elif N <= 32
#define REPEAT_N(x) REPEAT_32(x)
#else
#define REPEAT_N(x) REPEAT_64(x)
#endif

//Redundant terms should be eliminated during compile-time if the result of test can be predicted.
#define FOR_W(init, test, update, expr) init; REPEAT_W(if(test) {expr; update;})
#define FOR_N(init, test, update, expr) init; REPEAT_N(if(test) {expr; update;})

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
        uint64_t *in = blocks + N;
        int64_t i, n;

        //Load the first WIDTH words.
        for(i = 0; i < WIDTH; i++) {
            in[i] = *ptr++;
        }
        in[0] ^= crc;

        len -= WIDTH * 8;

        //Reduce the buffer using P(x)^64, N words at a time.
        while(len >= N * 8) {
            //Process the N outgoing blocks.
            FOR_N(i = 0, i < N, i++,
                blocks[i] = blocks[i + N];
                FOR_N(n = 0, n < i, n++,
                    blocks[i] ^= sub(blocks[n], WIDTH - i + n);
                )
            )

            //Process the first WIDTH - N blocks inside the register.
            FOR_W(i, i < WIDTH, i++,
                blocks[i] = blocks[i + N];
                FOR_N(n = 0, n < N, n++,
                    blocks[i] ^= sub(blocks[n], WIDTH - i + n);
                )
            )

            //Process the last N blocks inside the register.
            FOR_N(i, i < WIDTH + N, i++,
                blocks[i] = *ptr++;
                FOR_N(n = i - WIDTH, n < N, n++,
                    blocks[i] ^= sub(blocks[n], WIDTH - i + n);
                )
            )

            len -= N * 8;
        }

        //Find the CRC of the reduced buffer.
        crc = crc_zeros(in[0]);
        for(i = 1; i < WIDTH; i++) {
            crc = crc_zeros(crc ^ in[i]);
        }

        buf = (unsigned char const*)ptr;
    }

    //Calculate the remaining bytes.
    crc = crc_bytes(crc, buf, len);
    return crc_final(crc);
}