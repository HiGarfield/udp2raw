/*
 *  this file comes from https://github.com/kokke/tiny-AES128-C
 */

#pragma once

#include <stdint.h>

#ifdef USE_OPENSSL
// Use OpenSSL implementation
#include "openssl_wrapper.h"

static inline void AES_ECB_encrypt_buffer(const uint8_t* input, const uint8_t* key, uint8_t *output) {
    openssl_aes_ecb_encrypt(input, key, output);
}

static inline void AES_ECB_decrypt_buffer(const uint8_t* input, const uint8_t* key, uint8_t *output) {
    openssl_aes_ecb_decrypt(input, key, output);
}

static inline void AES_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv) {
    openssl_aes_cbc_encrypt(input, output, length, key, iv);
}

static inline void AES_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv) {
    openssl_aes_cbc_decrypt(input, output, length, key, iv);
}

static inline void AES_CFB_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv) {
    openssl_aes_cfb_encrypt(input, output, length, key, iv);
}

static inline void AES_CFB_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv) {
    openssl_aes_cfb_decrypt(input, output, length, key, iv);
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
