// Android stand-in for Apple's libcompression (<compression.h>), which nvn_capture.c uses for LZFSE chunks. Android has no LZFSE: the encoder reports
// no gain (0), so every chunk is stored raw (CAP_RAW, which the capture format has for chunks that do not shrink); the decoder reports failure (0), so a
// capture made on the Mac with LZFSE chunks is refused by cap_next (-1) instead of read wrongly. Captures made on Android read anywhere
#pragma once
#include <stddef.h>
#include <stdint.h>
typedef enum { COMPRESSION_LZ4 = 0x100, COMPRESSION_ZLIB = 0x205, COMPRESSION_LZMA = 0x306, COMPRESSION_LZ4_RAW = 0x101, COMPRESSION_LZFSE = 0x801 } compression_algorithm;
static inline size_t compression_encode_buffer(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size, void *scratch, compression_algorithm a) {
    (void)dst; (void)dst_size; (void)src; (void)src_size; (void)scratch; (void)a; return 0; }
static inline size_t compression_decode_buffer(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size, void *scratch, compression_algorithm a) {
    (void)dst; (void)dst_size; (void)src; (void)src_size; (void)scratch; (void)a; return 0; }
