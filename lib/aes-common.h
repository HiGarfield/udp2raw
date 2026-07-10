/*
 *  this file comes from https://github.com/kokke/tiny-AES128-C
 */

#pragma once

#include <stdint.h>
#include <limits.h>
#include <stdlib.h>

#ifdef USE_OPENSSL
// Use OpenSSL implementation
#include <limits.h>
#include <stdlib.h>
#include "openssl_wrapper.h"

static inline void AES_ECB_encrypt_buffer(const uint8_t* input, const uint8_t* key, uint8_t *output) {
    if (openssl_aes_ecb_encrypt(input, key, output) != 0) abort();
}

static inline void AES_ECB_decrypt_buffer(const uint8_t* input, const uint8_t* key, uint8_t *output) {
    if (openssl_aes_ecb_decrypt(input, key, output) != 0) abort();
}

static inline void AES_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv) {
    if (length > (uint32_t)INT_MAX) abort();
    if (openssl_aes_cbc_encrypt(input, output, (int)length, key, iv) != (int)length) abort();
}

static inline void AES_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv) {
    if (length > (uint32_t)INT_MAX) abort();
    if (openssl_aes_cbc_decrypt(input, output, (int)length, key, iv) != (int)length) abort();
}

static inline void AES_CFB_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv) {
    if (length > (uint32_t)INT_MAX) abort();
    if (openssl_aes_cfb_encrypt(input, output, (int)length, key, iv) != (int)length) abort();
}

static inline void AES_CFB_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv) {
    if (length > (uint32_t)INT_MAX) abort();
    if (openssl_aes_cfb_decrypt(input, output, (int)length, key, iv) != (int)length) abort();
}

#else
// Use built-in implementation

void AES_ECB_encrypt_buffer(const uint8_t* input, const uint8_t* key, uint8_t *output);
void AES_ECB_decrypt_buffer(const uint8_t* input, const uint8_t* key, uint8_t *output);

void AES_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

void AES_CFB_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES_CFB_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

#endif // USE_OPENSSL
