This repository contains an implementation of the unrolled generator CRC algorithm specified in [Chorba: A novel CRC32 implementation](https://arxiv.org/abs/2412.16398). It performs better than most known CRC algorithms.

## Compilation

The library requires defining the constants `WIDTH`, `POLY`, `INIT`, `REFIN`, `REFOUT`, and `XOROUT` according to the [CRC RevEng catalogue](https://reveng.sourceforge.io/crc-catalogue/all.htm).

For CRC32, this would look like:

```
g++ -c -DWIDTH=32 -DPOLY=0x04c11db7ULL -DINIT=0xffffffffULL -DREFIN=true -DREFOUT=true -DXOROUT=0xffffffffULL -O3 crc.cpp
```

For CRC7 and smaller, the constant N in crc.c has to be set less than or equal to the CRC width.

## Benchmark

Performance varies depending on the CRC algorithm used. For more information, check [BENCHMARK.md](BENCHMARK.md)

## References

1- [Fast CRCs (Extended Version)](https://arxiv.org/abs/1009.5949)

2- [chorba](https://github.com/samrussell/chorba)