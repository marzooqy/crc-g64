## Benchmark

The performance depends on two factors:

- The width of the CRC. Smaller widths mean higher performance.

- The number of one bits that the polynomial has. Fewer bits mean higher speeds.

CRC64 is relatively slow as it requires a large number of operations to compute. In this case, an algorithm like [crc-braid](https://github.com/marzooqy/crc-braid) may be more optimal.

The constant N determines how many words should be reduced in each iteration of the loop. Larger values can improve the performance for polynomials with fewer bits.

Tested on a 12th generation Intel i7 processor.

| Model | Bits | Speed (GiB/s) |
|---|:-:|:-:|
| CRC3-GSM | 2 | 37.25 |
| CRC3-ROHC | 2 | 37.25 |
| CRC4-G-704 | 2 | 45.85 |
| CRC4-INTERLAKEN | 2 | 45.85 |
| CRC5-EPC-C1G2 | 2 | 45.85 |
| CRC5-G-704 | 3 | 23.84 |
| CRC5-USB | 2 | 49.67 |
| CRC6-CDMA2000-A | 4 | 29.80 |
| CRC6-CDMA2000-B | 3 | 39.74 |
| CRC6-DARC | 3 | 33.11 |
| CRC6-G-704 | 2 | 49.67 |
| CRC6-GSM | 5 | 27.09 |
| CRC7-MMC | 2 | 42.57 |
| CRC7-ROHC | 5 | 29.80 |
| CRC7-UMTS | 3 | 23.84 |
| CRC8-AUTOSAR | 5 | 28.38 |
| CRC8-BLUETOOTH | 5 | 23.84 |
| CRC8-CDMA2000 | 5 | 35.06 |
| CRC8-DARC | 4 | 31.37 |
| CRC8-DVB-S2 | 5 | 22.92 |
| CRC8-GSM-A | 4 | 29.80 |
| CRC8-GSM-B | 3 | 35.06 |
| CRC8-HITAG | 4 | 31.37 |
| CRC8-I-432-1 | 3 | 42.57 |
| CRC8-I-CODE | 4 | 28.38 |
| CRC8-LTE | 5 | 35.06 |
| CRC8-MAXIM-DOW | 3 | 39.74 |
| CRC8-MIFARE-MAD | 4 | 31.37 |
| CRC8-NRSC-5 | 3 | 37.25 |
| CRC8-OPENSAFETY | 5 | 25.92 |
| CRC8-ROHC | 3 | 42.57 |
| CRC8-SAE-J1850 | 4 | 31.37 |
| CRC8-SMBUS | 3 | 42.57 |
| CRC8-TECH-3250 | 4 | 31.37 |
| CRC8-WCDMA | 5 | 33.11 |
| CRC10-ATM | 5 | 27.09 |
| CRC10-CDMA2000 | 7 | 21.29 |
| CRC10-GSM | 6 | 24.84 |
| CRC11-FLEXRAY | 5 | 21.29 |
| CRC11-UMTS | 5 | 20.55 |
| CRC12-CDMA2000 | 7 | 19.23 |
| CRC12-DECT | 5 | 27.09 |
| CRC12-GSM | 6 | 19.87 |
| CRC12-UMTS | 5 | 27.09 |
| CRC13-BBC | 9 | 16.56 |
| CRC14-DARC | 3 | 22.08 |
| CRC14-GSM | 5 | 20.55 |
| CRC15-CAN | 7 | 17.03 |
| CRC15-MPT1327 | 6 | 18.63 |
| CRC16-ARC | 3 | 19.87 |
| CRC16-CDMA2000 | 8 | 14.19 |
| CRC16-CMS | 3 | 20.55 |
| CRC16-DDS-110 | 3 | 19.87 |
| CRC16-DECT-R | 5 | 18.06 |
| CRC16-DECT-X | 5 | 18.63 |
| CRC16-DNP | 9 | 13.55 |
| CRC16-EN-13757 | 9 | 13.25 |
| CRC16-GENIBUS | 3 | 22.92 |
| CRC16-GSM | 3 | 22.92 |
| CRC16-IBM-3740 | 3 | 22.08 |
| CRC16-IBM-SDLC | 3 | 23.84 |
| CRC16-ISO-IEC-14443-3-A | 3 | 23.84 |
| CRC16-KERMIT | 3 | 23.84 |
| CRC16-LJ1200 | 10 | 13.86 |
| CRC16-M17 | 8 | 15.28 |
| CRC16-MAXIM-DOW | 3 | 21.29 |
| CRC16-MCRF4XX | 3 | 23.84 |
| CRC16-MODBUS | 3 | 19.87 |
| CRC16-NRSC-5 | 4 | 21.29 |
| CRC16-OPENSAFETY-A | 8 | 15.28 |
| CRC16-OPENSAFETY-B | 10 | 14.54 |
| CRC16-PROFIBUS | 10 | 14.54 |
| CRC16-RIELLO | 3 | 22.92 |
| CRC16-SPI-FUJITSU | 3 | 22.92 |
| CRC16-T10-DIF | 10 | 13.86 |
| CRC16-TELEDISK | 7 | 14.19 |
| CRC16-TMS37157 | 3 | 22.92 |
| CRC16-UMTS | 3 | 20.55 |
| CRC16-USB | 3 | 19.87 |
| CRC16-XMODEM | 3 | 22.92 |
| CRC17-CAN-FD | 9 | 13.86 |
| CRC21-CAN-FD | 7 | 12.16 |
| CRC24-BLE | 7 | 12.96 |
| CRC24-FLEXRAY-A | 15 | 9.17 |
| CRC24-FLEXRAY-B | 15 | 9.46 |
| CRC24-INTERLAKEN | 11 | 9.77 |
| CRC24-LTE-A | 13 | 10.84 |
| CRC24-LTE-B | 5 | 12.16 |
| CRC24-OPENPGP | 13 | 10.64 |
| CRC24-OS-9 | 5 | 12.42 |
| CRC30-CDMA | 13 | 8.64 |
| CRC31-PHILIPS | 14 | 8.64 |
| CRC32-AIXM | 11 | 8.40 |
| CRC32-AUTOSAR | 19 | 7.01 |
| CRC32-BASE91-D | 14 | 8.05 |
| CRC32-BZIP2 | 14 | 8.64 |
| CRC32-CD-ROM-EDC | 7 | 10.28 |
| CRC32-CKSUM | 14 | 8.05 |
| CRC32-ISCSI | 17 | 6.93 |
| CRC32-ISO-HDLC | 14 | 8.64 |
| CRC32-JAMCRC | 14 | 8.64 |
| CRC32-MEF | 17 | 6.41 |
| CRC32-MPEG-2 | 14 | 8.51 |
| CRC32-XFER | 6 | 11.04 |
| CRC40-GSM | 5 | 7.54 |
| CRC64-ECMA-182 | 33 | 3.61 |
| CRC64-GO-ISO | 4 | 4.45 |
| CRC64-MS | 27 | 3.90 |
| CRC64-NVME | 32 | 3.82 |
| CRC64-REDIS | 32 | 4.05 |
| CRC64-WE | 33 | 3.63 |
| CRC64-XZ | 33 | 3.68 |
