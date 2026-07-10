#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_OPENSSL
#include "openssl_wrapper.h"
// When using OpenSSL, provide wrapper for sha1_hmac
static inline void sha1_hmac(const unsigned char *key, int keylen, const unsigned char *input, int ilen, unsigned char output[20]) {
    if (openssl_hmac_sha1(key, keylen, input, ilen, output) != 0) abort();
}
// sha1 is still needed by PBKDF2 implementation which uses it internally
// PBKDF2-HMAC-SHA1 relies on the built-in sha1() function
void sha1(const unsigned char *input, int ilen, unsigned char output[20]);
void PKCS5_PBKDF2_HMAC_SHA1(const unsigned char *password, size_t plen,
    const unsigned char *salt, size_t slen,
    const unsigned long iteration_count, const unsigned long key_length,
    unsigned char *output);
#else
void sha1(const unsigned char *input, int ilen, unsigned char output[20]);
void sha1_hmac(const unsigned char *key, int keylen, const unsigned char *input, int ilen, unsigned char output[20]);
void PKCS5_PBKDF2_HMAC_SHA1(const unsigned char *password, size_t plen,
    const unsigned char *salt, size_t slen,
    const unsigned long iteration_count, const unsigned long key_length,
    unsigned char *output);
#endif
