/*
 * pcap_dyn.h -- self-contained libpcap binding for Windows
 *              (loads wpcap.dll at run time, no Npcap SDK required at build time)
 *
 * Design notes:
 *   1. This header IS a minimal <pcap.h>: every struct layout below is byte-for-byte
 *      identical to upstream libpcap, so no SDK header and no .lib is needed.
 *   2. Every pcap_xxx symbol is a *function pointer variable* with the same name as the
 *      original function, so call sites compile unchanged (indirect call through pointer).
 *   3. wpcap.dll is loaded with LoadLibrary() only at run time, so one binary works with
 *      Npcap 0.99 through the newest release (and even legacy WinPcap for the common API).
 *
 * License: struct/constant definitions derive from libpcap (BSD-3-Clause), safe to vendor.
 *
 * Usage:
 *   - Most translation units:      #include "pcap_dyn.h"
 *   - Exactly ONE .cpp (pcap_dyn.cpp) defines the macro before including:
 *                                  #define PCAP_DYN_IMPLEMENTATION
 *   - Non-Windows platforms: falls through to the system <pcap.h>, no changes needed.
 */
#ifndef PCAP_DYN_H
#define PCAP_DYN_H

#if !defined(_WIN32)
/* ------------------------------------------------------------------ */
/* Linux / macOS / BSD: just use the system libpcap                     */
/* ------------------------------------------------------------------ */
#include <pcap.h>

#else /* _WIN32 */
/* ------------------------------------------------------------------ */
/* Windows: self-contained definitions + run-time dynamic loading       */
/* ------------------------------------------------------------------ */

/* winsock2.h must come first. winsock.h (pulled in by windows.h) and winsock2.h are
 * mutually exclusive; mixing them gives wpcap.dll a struct sockaddr / struct timeval
 * layout it does not expect.
 *
 * If windows.h already arrived in this TU while winsock2.h has not, the order is wrong.
 * Fail loudly with an actionable message instead of MinGW's vague "#warning Please
 * include winsock2.h before windows.h". */
#if defined(_WINDOWS_) && !defined(_WINSOCK2API_)
#error \
    "pcap_dyn.h: windows.h was included before winsock2.h. Put #include <winsock2.h> " \
         "on the FIRST line of this .cpp, or move #include \"pcap_dyn.h\" above any " \
         "windows.h (putting it at the top of common.h fixes every TU at once)."
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===================== Basic types ===================== */

/* wpcap.dll exports plain cdecl undecorated names. Irrelevant on x64, but the
 * 32-bit MSYS2 MinGW-w32 target needs it declared explicitly. */
#if defined(_MSC_VER) || defined(__MINGW32__)
#define PCAP_CDECL __cdecl
#else
#define PCAP_CDECL
#endif

typedef unsigned int bpf_u_int32;

/* Capture handle: opaque. Never touch its members. */
typedef struct pcap pcap_t;

/* ===================== Public structs ===================== */
/* The four layouts below must match wpcap.dll exactly. pcap_compile() writes straight
 * into bpf_program, so a one-byte mismatch makes pcap_setfilter() silently drop packets
 * instead of raising any error. */

struct pcap_addr {
    struct pcap_addr *next;
    struct sockaddr *addr;
    struct sockaddr *netmask;
    struct sockaddr *broadaddr;
    struct sockaddr *dstaddr;
};
typedef struct pcap_addr pcap_addr_t;

struct pcap_if {
    struct pcap_if *next;
    char *name;
    char *description;
    struct pcap_addr *addresses;
    bpf_u_int32 flags;
};
typedef struct pcap_if pcap_if_t;

struct pcap_pkthdr {
    struct timeval ts;  /* timestamp */
    bpf_u_int32 caplen; /* captured length */
    bpf_u_int32 len;    /* original wire length */
};

struct bpf_insn {
    unsigned short code;
    unsigned char jt;
    unsigned char jf;
    bpf_u_int32 k;
};

struct bpf_program {
    unsigned int bf_len;
    struct bpf_insn *bf_insns;
};

typedef void (*pcap_handler)(unsigned char *user,
                             const struct pcap_pkthdr *h,
                             const unsigned char *bytes);

typedef enum {
    PCAP_D_INOUT = 0,
    PCAP_D_IN = 1,
    PCAP_D_OUT = 2
} pcap_direction_t;

/* ===================== Constants ===================== */

#define PCAP_ERRBUF_SIZE 256
#define PCAP_NETMASK_UNKNOWN 0xffffffffU
#define PCAP_IF_LOOPBACK 0x00000001U

#define PCAP_ERROR (-1)

/* Link-layer types */
#define DLT_NULL 0
#define DLT_EN10MB 1
#define DLT_RAW 12
#define DLT_IEEE802_11 105
#define DLT_LOOP 108
#define DLT_LINUX_SLL 113
#define DLT_LINUX_SLL2 276

/* ===================== Function pointer table ===================== */
/*
 * REQUIRED: missing entry means "wpcap.dll is too old / is not Npcap" -> hard error.
 *   Every one of these already existed in WinPcap 4.1.3 (libpcap 1.0.0), so any Npcap
 *   build whatsoever provides them.
 * OPTIONAL: resolved to NULL when absent; callers MUST null-check before use
 *   (these postdate WinPcap).
 */
#define PCAP_FN_REQUIRED(X)                                                                \
    X(int, pcap_findalldevs, (pcap_if_t **, char *))                                       \
    X(void, pcap_freealldevs, (pcap_if_t *))                                               \
    X(pcap_t *, pcap_open_live, (const char *, int, int, int, char *))                     \
    X(pcap_t *, pcap_create, (const char *, char *))                                       \
    X(int, pcap_set_snaplen, (pcap_t *, int))                                              \
    X(int, pcap_set_promisc, (pcap_t *, int))                                              \
    X(int, pcap_set_timeout, (pcap_t *, int))                                              \
    X(int, pcap_activate, (pcap_t *))                                                      \
    X(int, pcap_setdirection, (pcap_t *, pcap_direction_t))                                \
    X(int, pcap_datalink, (pcap_t *))                                                      \
    X(int, pcap_compile, (pcap_t *, struct bpf_program *, const char *, int, bpf_u_int32)) \
    X(int, pcap_setfilter, (pcap_t *, struct bpf_program *))                               \
    X(void, pcap_freecode, (struct bpf_program *))                                         \
    X(int, pcap_next_ex, (pcap_t *, struct pcap_pkthdr **, const unsigned char **))        \
    X(int, pcap_loop, (pcap_t *, int, pcap_handler, unsigned char *))                      \
    X(void, pcap_breakloop, (pcap_t *))                                                    \
    X(int, pcap_sendpacket, (pcap_t *, const unsigned char *, int))                        \
    X(void, pcap_close, (pcap_t *))                                                        \
    X(char *, pcap_geterr, (pcap_t *))                                                     \
    X(const char *, pcap_lib_version, (void))

#define PCAP_FN_OPTIONAL(X)                          \
    X(int, pcap_set_immediate_mode, (pcap_t *, int)) \
    X(int, pcap_set_buffer_size, (pcap_t *, int))

/* ===================== Declarations / definitions ===================== */

#ifdef PCAP_DYN_IMPLEMENTATION
#define PCAP_DYN_DECL(ret, name, args) ret(PCAP_CDECL *name) args = NULL;
#else
#define PCAP_DYN_DECL(ret, name, args) extern ret(PCAP_CDECL *name) args;
#endif

PCAP_FN_REQUIRED(PCAP_DYN_DECL)
PCAP_FN_OPTIONAL(PCAP_DYN_DECL)
#undef PCAP_DYN_DECL

/*
 * Load wpcap.dll and resolve every required function.
 * Returns 0 on success; on failure returns -1 and writes a human-readable reason into
 * errbuf (which must be at least PCAP_ERRBUF_SIZE bytes).
 *
 * IDEMPOTENT: the actual work runs only on the first call; every later call just replays
 * the stored status and message, so you may call it defensively from several entry points.
 * LAZY by design: nothing is loaded until you call this, so code paths that never touch
 * pcap (--help, --gen-rule, ...) still work on machines without Npcap installed.
 * Call it before any pcap_* call.
 */
int pcap_dyn_init(char *errbuf);

/* Handy for diagnostics, e.g. mylog(log_info, "Npcap runtime: %s\n", pcap_lib_version()); */

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef PCAP_DYN_IMPLEMENTATION

#include <windows.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int pcap_dyn_init(char *errbuf) {
    static char fallback[PCAP_ERRBUF_SIZE];
    static char saved[PCAP_ERRBUF_SIZE]; /* result of the first (and only) real attempt */
    static int status = -2;              /* -2 == not attempted yet */
    HMODULE h = NULL;

    if (errbuf == NULL)
        errbuf = fallback;

    /* Idempotent: replay the first attempt instead of loading the DLL twice. */
    if (status != -2) {
        memcpy(errbuf, saved, PCAP_ERRBUF_SIZE);
        return status;
    }
    errbuf[0] = '\0';

/* Single exit path: remember the outcome so later calls can replay it verbatim. */
#define PCAP_DYN_RETURN(v)                       \
    do {                                         \
        status = (v);                            \
        memcpy(saved, errbuf, PCAP_ERRBUF_SIZE); \
        return status;                           \
    } while (0)

    /* 1) Npcap installs wpcap.dll into System32\Npcap\ (a 32-bit process is redirected to
     *    SysWOW64\Npcap\). LOAD_WITH_ALTERED_SEARCH_PATH makes its dependency Packet.dll
     *    resolve from that same directory. */
    h = LoadLibraryExW(L"C:\\Windows\\System32\\Npcap\\wpcap.dll", NULL,
                       LOAD_WITH_ALTERED_SEARCH_PATH);

    /* 2) Still nothing: add that directory to the DLL search path and load by name. */
    if (h == NULL) {
        SetDllDirectoryW(L"C:\\Windows\\System32\\Npcap\\");
        h = LoadLibraryW(L"wpcap.dll");
    }

    /* 3) Last resort: default search order. Covers WinPcap, and Npcap installed with
     *    "Install Npcap in WinPcap API-compatible Mode". */
    if (h == NULL) {
        SetDllDirectoryW(NULL); /* restore default order for later LoadLibrary calls */
        h = LoadLibraryW(L"wpcap.dll");
    }

    if (h == NULL) {
        snprintf(errbuf, PCAP_ERRBUF_SIZE,
                 "wpcap.dll not found. Please install Npcap (https://npcap.com/#download); "
                 "if it is already installed, reinstall it with \"Install Npcap in "
                 "WinPcap API-compatible Mode\" checked.");
        PCAP_DYN_RETURN(-1);
    }

#define PCAP_DYN_LOAD(ret, name, args)                                             \
    do {                                                                           \
        void *fp = (void *)GetProcAddress(h, #name);                               \
        if (fp == NULL) {                                                          \
            snprintf(errbuf, PCAP_ERRBUF_SIZE,                                     \
                     "wpcap.dll is missing %s(); the installed Npcap is too old, " \
                     "please upgrade it.",                                         \
                     #name);                                                       \
            PCAP_DYN_RETURN(-1);                                                   \
        }                                                                          \
        name = (ret(PCAP_CDECL *) args)fp;                                         \
    } while (0);

    PCAP_FN_REQUIRED(PCAP_DYN_LOAD)
#undef PCAP_DYN_LOAD

/* Optional functions: NULL when unavailable, caller null-checks and skips. */
#define PCAP_DYN_LOAD_OPT(ret, name, args)                       \
    do {                                                         \
        void *fp = (void *)GetProcAddress(h, #name);             \
        name = (fp == NULL) ? NULL : (ret(PCAP_CDECL *) args)fp; \
    } while (0);

    PCAP_FN_OPTIONAL(PCAP_DYN_LOAD_OPT)
#undef PCAP_DYN_LOAD_OPT

    /* Success: leave saved[] empty (nothing useful to replay) and return 0. */
    saved[0] = '\0';
    PCAP_DYN_RETURN(0);
#undef PCAP_DYN_RETURN
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/*
 * Automatic initialization -- no call site required anywhere.
 *
 * A file-scope object in the same translation unit as PCAP_DYN_IMPLEMENTATION runs
 * pcap_dyn_init() during C++ static initialization, i.e. strictly before main() and
 * therefore strictly before any pcap_* call, on every code path. This removes the
 * ordering hazard entirely: it no longer matters whether the first pcap call comes from
 * device enumeration in client.cpp (no --dev) or from init_raw_socket() in network.cpp
 * (--dev given), and main.cpp needs no change at all.
 *
 * The return value is deliberately ignored here. Failure is recorded and replayed later,
 * so code paths that never touch pcap (--help, --gen-rule, --clear, ...) still run fine
 * on machines without Npcap. Call pcap_dyn_init(buf) later only if you want the message.
 */
#ifdef __cplusplus
namespace pcap_dyn_detail {
struct AutoInit {
    AutoInit() { pcap_dyn_init(NULL); }
};
/* Stored in .init_array when this TU is linked into the executable; a direct .o link
 * (as opposed to a .a member that nothing references) always keeps it. */
static AutoInit g_auto_init;
} /* namespace pcap_dyn_detail */
#endif

#endif /* PCAP_DYN_IMPLEMENTATION */

#endif /* _WIN32 */

#endif /* PCAP_DYN_H */
