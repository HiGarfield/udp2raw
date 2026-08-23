// Regression tests for the bug-fix round:
//   Bug #1: conn_manager_t::clear_it was never initialized (connection.cpp),
//           and persisted across rehash/erase -> UB / crash in
//           clear_inactive0(). Fixed by initializing it in the constructor
//           and re-pointing it at mp.begin() on every insertion and erase.
//   Bug #3: my_encrypt() only validated the *input* length (<= max_data_len)
//           but the auth tag (md5+16/hmac+20/...) plus up to 15 bytes of CBC
//           padding can inflate the *output* beyond max_data_len, producing
//           packets the peer's my_decrypt() silently rejects. Fixed with the
//           get_max_plain_len() bound and an output-side check in my_encrypt().
//
// Build (from the repo root, mirrors the makefile flags; ASan+UBSan enabled):
//   g++ -std=c++11 -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers -I. -isystem libev -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer test/test_bugfix.cpp client.cpp common.cpp connection.cpp encrypt.cpp fd_manager.cpp log.cpp misc.cpp my_ev.cpp network.cpp server.cpp lib/md5.cpp lib/pbkdf2-sha1.cpp lib/pbkdf2-sha256.cpp lib/aes_faster_c/aes.cpp lib/aes_faster_c/wrapper.cpp -lpthread -lrt -o test/test_bugfix
//
// Run: test/test_bugfix   (exit 0 == all tests passed)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "connection.h"
#include "encrypt.h"
#include "misc.h"

// defined in misc.cpp
extern program_mode_t program_mode;
extern u32_t raw_ip_version;
// defined in encrypt.cpp (not exported via encrypt.h)
extern int is_hmac_used;

static void test_bug1_conn_clear() {
    printf("[Bug#1] conn_manager clear_it regression test\n");
    program_mode = server_mode;
    raw_ip_version = AF_INET;  // needed by my_ip_t::get_str1() in the clear log path

    // (a) fresh manager over an empty map must not crash (clear_it == mp.begin() == mp.end())
    conn_manager.clear_inactive0();

    // (b) insert many conns to force multiple rehashes; all are stale
    //     (last_state_time=0, state=server_idle) so every one must be collected.
    const int N = 20000;
    for (int i = 0; i < N; i++) {
        address_t addr;
        addr.from_ip_port(0x0a000001u + (i % 200), 1000 + i);
        conn_info_t &ci = conn_manager.find_insert(addr);
        ci.state.server_current_state = server_idle;
        ci.last_state_time = 0;  // stale -> should be collected
    }
    if ((int)conn_manager.mp.size() != N) {
        printf("FAIL Bug#1: mp.size()=%d after %d inserts\n", (int)conn_manager.mp.size(), N);
        exit(1);
    }

    int guard = 0;
    while (!conn_manager.mp.empty() && guard++ < N * 2) {
        conn_manager.clear_inactive0();
    }
    if (!conn_manager.mp.empty()) {
        printf("FAIL Bug#1: %d stale conns not collected\n", (int)conn_manager.mp.size());
        exit(1);
    }

    // (c) fresh conns must be kept across several clear passes
    for (int i = 0; i < 100; i++) {
        address_t addr;
        addr.from_ip_port(0x0b000001u + (i % 100), 2000 + i);
        conn_info_t &ci = conn_manager.find_insert(addr);
        ci.state.server_current_state = server_idle;
        ci.last_state_time = get_current_time();  // fresh -> must be kept
    }
    for (int i = 0; i < 5; i++) {
        conn_manager.clear_inactive0();
    }
    if ((int)conn_manager.mp.size() != 100) {
        printf("FAIL Bug#1: fresh conns wrongly collected, mp.size()=%d\n", (int)conn_manager.mp.size());
        exit(1);
    }
    printf("[Bug#1] OK: no crash/UB; stale conns collected, fresh conns kept\n");

    // cleanup so LeakSanitizer stays quiet: free the remaining (fresh) conns
    while (!conn_manager.mp.empty()) {
        conn_manager.erase(conn_manager.mp.begin());
    }
    assert(conn_manager.mp.empty());
}

struct encrypt_combo_t {
    const char *name;
    auth_mode_t auth;
    cipher_mode_t cipher;
    int expect_max_plain;  // exact bound for this combination
};

static void test_bug3_encrypt_maxlen() {
    printf("[Bug#3] my_encrypt output <= max_data_len regression test\n");
    my_init_keys("test_bugfix_key_123456", 1);

    // auth overhead: none=0, md5=16, crc32=4, simple=8, hmac_sha1=20
    // non-CBC (none/xor/cfb):            max_plain = max_data_len - auth_overhead
    // CBC (mac-then-encrypt):            max_plain = (max_data_len/16 - 1)*16 + 15 - auth_overhead
    //                                     (padding adds 1..16 bytes, 16 if already aligned)
    // CBC (AE, is_hmac_used):            max_plain = (max_data_len - auth_overhead)/16*16 - 1
    const encrypt_combo_t combos[] = {
        {"none+none", auth_none, cipher_none, 1800},
        {"none+cbc", auth_none, cipher_aes128cbc, 1791},
        {"md5+none", auth_md5, cipher_none, 1784},
        {"md5+xor", auth_md5, cipher_xor, 1784},
        {"md5+cfb", auth_md5, cipher_aes128cfb, 1784},
        {"md5+cbc", auth_md5, cipher_aes128cbc, 1775},
        {"crc32+cbc", auth_crc32, cipher_aes128cbc, 1787},
        {"simple+cbc", auth_simple, cipher_aes128cbc, 1783},
        {"hmac+cfb", auth_hmac_sha1, cipher_aes128cfb, 1780},
        {"hmac+cbc", auth_hmac_sha1, cipher_aes128cbc, 1775},
    };

    char in[buf_len];
    char out[buf_len];
    memset(in, 0x5a, sizeof(in));

    for (size_t i = 0; i < sizeof(combos) / sizeof(combos[0]); i++) {
        const encrypt_combo_t &cb = combos[i];
        auth_mode = cb.auth;
        cipher_mode = cb.cipher;
        is_hmac_used = (cb.auth == auth_hmac_sha1) ? 1 : 0;

        int maxp = get_max_plain_len();
        if (maxp != cb.expect_max_plain) {
            printf("FAIL Bug#3[%s]: get_max_plain_len()=%d, expected %d\n", cb.name, maxp, cb.expect_max_plain);
            exit(1);
        }

        // the exact bound must encrypt successfully and stay within max_data_len
        int len = maxp;
        if (my_encrypt(in, out, len) != 0 || len > max_data_len) {
            printf("FAIL Bug#3[%s]: my_encrypt(%d) ret=%d len=%d\n", cb.name, maxp, len, len);
            exit(1);
        }

        // one byte more must be rejected (output would exceed max_data_len)
        len = maxp + 1;
        if (my_encrypt(in, out, len) == 0) {
            printf("FAIL Bug#3[%s]: my_encrypt(%d) unexpectedly succeeded (len=%d)\n", cb.name, maxp + 1, len);
            exit(1);
        }
        printf("  %-10s max_plain_len=%4d  encrypt(max)->len=%4d OK, encrypt(max+1)->reject OK\n",
               cb.name, maxp, len);
    }
    printf("[Bug#3] OK: all auth x cipher combinations keep encrypted len <= max_data_len\n");
}

int main() {
    test_bug1_conn_clear();
    test_bug3_encrypt_maxlen();
    printf("ALL TESTS PASSED\n");
    return 0;
}
