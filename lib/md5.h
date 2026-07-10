#ifndef UDP2RAW_MD5_H_
#define UDP2RAW_MD5_H_
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdlib.h>

#ifdef USE_OPENSSL
#include "openssl_wrapper.h"
static inline void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest) {
    if (initial_len > INT_MAX) abort();
    if (openssl_md5(initial_msg, (int)initial_len, digest) != 0) abort();
}
#else
void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest);
#endif

#endif
