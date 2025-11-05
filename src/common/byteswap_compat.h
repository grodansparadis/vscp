#ifndef BYTESWAP_COMPAT_H
#define BYTESWAP_COMPAT_H

#ifdef _WIN32
// Windows byteswap compatibility
#include <intrin.h>
#include <cstdint>

// Define the byteswap functions for Windows
static inline uint16_t bswap_16(uint16_t x) {
    return _byteswap_ushort(x);
}

static inline uint32_t bswap_32(uint32_t x) {
    return _byteswap_ulong(x);
}

static inline uint64_t bswap_64(uint64_t x) {
    return _byteswap_uint64(x);
}

#else
// Unix/Linux - use the standard header
#include <byteswap.h>
#endif

#endif // BYTESWAP_COMPAT_H