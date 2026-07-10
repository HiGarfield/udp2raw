/*
 * OpenSSL wrapper implementation for udp2raw
 * Compatible with OpenSSL 1.0.x to latest version
 */

#include "openssl_wrapper.h"

#ifdef USE_OPENSSL

#include <string.h>
#include <stdlib.h>

// AES-128-CBC encryption using OpenSSL
int openssl_aes_cbc_encrypt(const unsigned char *input, unsigned char *output, 
                            int length, const unsigned char *key, const unsigned char *iv) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len;
    int ciphertext_len;
    
    // Initialize encryption operation
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Disable padding - caller should handle padding
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    // Encrypt
    if (1 != EVP_EncryptUpdate(ctx, output, &len, input, length)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;
    
    // Finalize encryption
    if (1 != EVP_EncryptFinal_ex(ctx, output + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

// AES-128-CBC decryption using OpenSSL
int openssl_aes_cbc_decrypt(const unsigned char *input, unsigned char *output, 
                            int length, const unsigned char *key, const unsigned char *iv) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len;
    int plaintext_len;
    
    // Initialize decryption operation
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Disable padding - caller should handle padding
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    // Decrypt
    if (1 != EVP_DecryptUpdate(ctx, output, &len, input, length)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;
    
    // Finalize decryption
    if (1 != EVP_DecryptFinal_ex(ctx, output + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

// AES-128-CFB encryption using OpenSSL
int openssl_aes_cfb_encrypt(const unsigned char *input, unsigned char *output, 
                            int length, const unsigned char *key, const unsigned char *iv) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len;
    int ciphertext_len;
    
    // Initialize encryption operation
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cfb128(), NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Encrypt
    if (1 != EVP_EncryptUpdate(ctx, output, &len, input, length)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;
    
    // Finalize encryption
    if (1 != EVP_EncryptFinal_ex(ctx, output + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

// AES-128-CFB decryption using OpenSSL
int openssl_aes_cfb_decrypt(const unsigned char *input, unsigned char *output, 
                            int length, const unsigned char *key, const unsigned char *iv) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len;
    int plaintext_len;
    
    // Initialize decryption operation
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cfb128(), NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Decrypt
    if (1 != EVP_DecryptUpdate(ctx, output, &len, input, length)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;
    
    // Finalize decryption
    if (1 != EVP_DecryptFinal_ex(ctx, output + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

// AES-128-ECB encryption using OpenSSL (single block)
int openssl_aes_ecb_encrypt(const unsigned char *input, const unsigned char *key, 
                            unsigned char *output) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len;
    
    // Initialize encryption operation
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Disable padding
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    // Encrypt single block (16 bytes)
    if (1 != EVP_EncryptUpdate(ctx, output, &len, input, 16)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Finalize encryption
    if (1 != EVP_EncryptFinal_ex(ctx, output + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

// AES-128-ECB decryption using OpenSSL (single block)
int openssl_aes_ecb_decrypt(const unsigned char *input, const unsigned char *key, 
                            unsigned char *output) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len;
    
    // Initialize decryption operation
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Disable padding
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    // Decrypt single block (16 bytes)
    if (1 != EVP_DecryptUpdate(ctx, output, &len, input, 16)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    // Finalize decryption
    if (1 != EVP_DecryptFinal_ex(ctx, output + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

// HMAC-SHA1 using OpenSSL
int openssl_hmac_sha1(const unsigned char *key, int key_len,
                     const unsigned char *data, int data_len,
                     unsigned char *output) {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    // OpenSSL 3.0 and later - use EVP_MAC API
    EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
    if (!mac) return -1;
    
    EVP_MAC_CTX *ctx = EVP_MAC_CTX_new(mac);
    EVP_MAC_free(mac);
    if (!ctx) return -1;
    
    OSSL_PARAM params[2];
    const char *digest_name = "SHA1";
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>(digest_name), 0);
    params[1] = OSSL_PARAM_construct_end();
    
    if (1 != EVP_MAC_init(ctx, key, key_len, params)) {
        EVP_MAC_CTX_free(ctx);
        return -1;
    }
    
    if (1 != EVP_MAC_update(ctx, data, data_len)) {
        EVP_MAC_CTX_free(ctx);
        return -1;
    }
    
    size_t len = 20;
    if (1 != EVP_MAC_final(ctx, output, &len, 20)) {
        EVP_MAC_CTX_free(ctx);
        return -1;
    }
    
    EVP_MAC_CTX_free(ctx);
    return (len == 20) ? 0 : -1;
#else
    // OpenSSL 1.1.0 and later (but before 3.0) OR OpenSSL 1.0.x
    unsigned int len = 20;
    
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    // OpenSSL 1.1.0 and later
    HMAC_CTX *ctx = HMAC_CTX_new();
    if (!ctx) return -1;
    
    if (1 != HMAC_Init_ex(ctx, key, key_len, EVP_sha1(), NULL)) {
        HMAC_CTX_free(ctx);
        return -1;
    }
    
    if (1 != HMAC_Update(ctx, data, data_len)) {
        HMAC_CTX_free(ctx);
        return -1;
    }
    
    if (1 != HMAC_Final(ctx, output, &len)) {
        HMAC_CTX_free(ctx);
        return -1;
    }
    
    HMAC_CTX_free(ctx);
#else
    // OpenSSL 1.0.x
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    
    if (1 != HMAC_Init_ex(&ctx, key, key_len, EVP_sha1(), NULL)) {
        HMAC_CTX_cleanup(&ctx);
        return -1;
    }
    
    if (1 != HMAC_Update(&ctx, data, data_len)) {
        HMAC_CTX_cleanup(&ctx);
        return -1;
    }
    
    if (1 != HMAC_Final(&ctx, output, &len)) {
        HMAC_CTX_cleanup(&ctx);
        return -1;
    }
    
    HMAC_CTX_cleanup(&ctx);
#endif
    
    return (len == 20) ? 0 : -1;
#endif
}

// MD5 using OpenSSL
int openssl_md5(const unsigned char *data, int data_len, unsigned char *output) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;
    
    unsigned int len;
    
    if (1 != EVP_DigestInit_ex(ctx, EVP_md5(), NULL)) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    if (1 != EVP_DigestUpdate(ctx, data, data_len)) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    if (1 != EVP_DigestFinal_ex(ctx, output, &len)) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    EVP_MD_CTX_free(ctx);
    return (len == 16) ? 0 : -1;
}

#endif // USE_OPENSSL
