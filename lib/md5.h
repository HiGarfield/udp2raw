#ifndef UDP2RAW_MD5_H_
#define UDP2RAW_MD5_H_
#include <stdint.h>
#include <stddef.h>

#ifdef USE_OPENSSL
#include "openssl_wrapper.h"
static inline void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest) {
    openssl_md5(initial_msg, initial_len, digest);
}
#else
void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest);
#endif

#endif
