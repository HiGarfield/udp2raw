/*
 * OpenSSL wrapper for udp2raw
 * Compatible with OpenSSL 1.0.x to latest version
 */

#ifndef UDP2RAW_OPENSSL_WRAPPER_H_
#define UDP2RAW_OPENSSL_WRAPPER_H_

#ifdef USE_OPENSSL

#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>

// Version compatibility macros
#if OPENSSL_VERSION_NUMBER < 0x10100000L
// OpenSSL 1.0.x compatibility
#define EVP_MD_CTX_new EVP_MD_CTX_create
#define EVP_MD_CTX_free EVP_MD_CTX_destroy

// HMAC_CTX allocation for OpenSSL 1.0.x
static inline HMAC_CTX* HMAC_CTX_new_compat(void) {
    HMAC_CTX *ctx = (HMAC_CTX*)malloc(sizeof(HMAC_CTX));
    if (ctx) {
        HMAC_CTX_init(ctx);
    }
    return ctx;
}

static inline void HMAC_CTX_free_compat(HMAC_CTX *ctx) {
    if (ctx) {
        HMAC_CTX_cleanup(ctx);
        free(ctx);
    }
}

#define HMAC_CTX_new HMAC_CTX_new_compat
#define HMAC_CTX_free HMAC_CTX_free_compat

#endif // OPENSSL_VERSION_NUMBER < 0x10100000L

// AES encryption/decryption using OpenSSL
int openssl_aes_cbc_encrypt(const unsigned char *input, unsigned char *output, 
                            int length, const unsigned char *key, const unsigned char *iv);
int openssl_aes_cbc_decrypt(const unsigned char *input, unsigned char *output, 
                            int length, const unsigned char *key, const unsigned char *iv);

int openssl_aes_cfb_encrypt(const unsigned char *input, unsigned char *output, 
                            int length, const unsigned char *key, const unsigned char *iv);
int openssl_aes_cfb_decrypt(const unsigned char *input, unsigned char *output, 
                            int length, const unsigned char *key, const unsigned char *iv);

int openssl_aes_ecb_encrypt(const unsigned char *input, const unsigned char *key, 
                            unsigned char *output);
int openssl_aes_ecb_decrypt(const unsigned char *input, const unsigned char *key, 
                            unsigned char *output);

// HMAC using OpenSSL
int openssl_hmac_sha1(const unsigned char *key, int key_len,
                     const unsigned char *data, int data_len,
                     unsigned char *output);

// MD5 using OpenSSL
int openssl_md5(const unsigned char *data, int data_len, unsigned char *output);

#endif // USE_OPENSSL

#endif // UDP2RAW_OPENSSL_WRAPPER_H_
