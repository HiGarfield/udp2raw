# OpenSSL Support in udp2raw

## Overview
udp2raw now supports using OpenSSL for cryptographic operations as an alternative to the built-in implementations. This provides better performance and leverages the well-tested OpenSSL library.

## Compatibility
The OpenSSL integration is compatible with:
- OpenSSL 1.0.x (1.0.0 and later)
- OpenSSL 1.1.x
- OpenSSL 3.0.x and later

The code automatically detects the OpenSSL version and uses the appropriate APIs to ensure compatibility.

## Building with OpenSSL

### Default Build (with OpenSSL)
By default, the build system will attempt to detect and use OpenSSL:
```bash
make
```

### Explicitly Enable OpenSSL
```bash
USE_OPENSSL=1 make
```

### Build without OpenSSL (use built-in crypto)
If you prefer to use the built-in cryptographic implementations:
```bash
USE_OPENSSL=0 make
```

## Features Supported by OpenSSL
When built with OpenSSL support, the following cryptographic operations use OpenSSL:
- AES-128-CBC encryption/decryption
- AES-128-CFB encryption/decryption
- AES-128-ECB encryption/decryption
- HMAC-SHA1
- MD5 hashing

## Implementation Details
- The OpenSSL wrapper code is in `lib/openssl_wrapper.h` and `lib/openssl_wrapper.cpp`
- Version-specific compatibility is handled through preprocessor directives
- OpenSSL 3.0+ uses the new EVP_MAC API for HMAC operations
- OpenSSL 1.1.x and 1.0.x use the legacy HMAC API with proper resource management

## Requirements
To build with OpenSSL support, you need:
- OpenSSL development headers (e.g., `libssl-dev` on Debian/Ubuntu)
- pkg-config (optional, but recommended for auto-detection)

### Installing OpenSSL development packages

#### Debian/Ubuntu
```bash
sudo apt-get install libssl-dev
```

#### CentOS/RHEL/Fedora
```bash
sudo yum install openssl-devel
# or on newer systems:
sudo dnf install openssl-devel
```

#### macOS
```bash
brew install openssl
```

## Performance Notes
Using OpenSSL generally provides better performance than the built-in implementations, especially on systems with hardware crypto acceleration support. OpenSSL can utilize:
- AES-NI instructions on x86/x64 CPUs
- ARM Crypto Extensions on ARM processors
- Other hardware acceleration features

## Verification
To verify that your build is using OpenSSL, check the linked libraries:
```bash
ldd udp2raw | grep -E "(ssl|crypto)"
```

You should see output similar to:
```
libssl.so.3 => /lib/x86_64-linux-gnu/libssl.so.3
libcrypto.so.3 => /lib/x86_64-linux-gnu/libcrypto.so.3
```

## Troubleshooting

### Build fails with "openssl/evp.h: No such file or directory"
Install the OpenSSL development package for your distribution (see Requirements section).

### Want to use built-in crypto instead of OpenSSL
Build with `USE_OPENSSL=0 make`

### Different OpenSSL location
If OpenSSL is installed in a non-standard location, you can specify it manually:
```bash
USE_OPENSSL=1 OPENSSL_CFLAGS="-I/path/to/openssl/include" OPENSSL_LIBS="-L/path/to/openssl/lib -lssl -lcrypto" make
```
