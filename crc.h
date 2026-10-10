#ifndef CRC_G64_H
#define CRC_G64_H

#include <stdbool.h>
#include <stdint.h>

#ifdef _MSC_VER
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#ifndef WIDTH
#error WIDTH is not defined.
#endif

#ifndef POLY
#error POLY is not defined.
#endif

#ifndef REFIN
#error REFIN is not defined.
#endif

#ifndef REFOUT
#error REFOUT is not defined.
#endif

#ifndef INIT
#error INIT is not defined.
#endif

extern "C" {

/* The initial CRC value. */
extern const uint64_t DLL_EXPORT crc_init;

/* Calculate the CRC using the table-based algorithm.
   Use crc_init as the initial CRC value.*/
uint64_t DLL_EXPORT crc_table(uint64_t crc, unsigned char const *buf, uint64_t len);

/* Calculate the CRC using the G64 algorithm.
   Use crc_init as the initial CRC value.*/
uint64_t DLL_EXPORT crc_g64(uint64_t crc, unsigned char const *buf, uint64_t len);

}

#endif
