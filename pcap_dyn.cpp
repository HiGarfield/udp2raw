/*
 * pcap_dyn.cpp -- provides the pcap_dyn run-time loader implementation.
 *
 * The loader is only needed on Windows; every other platform uses the system
 * <pcap.h> that pcap_dyn.h forwards to, so nothing is compiled here for them.
 * This translation unit is therefore built only for MP builds on Windows.
 *
 * winsock2.h must be included before windows.h is ever pulled in (otherwise
 * MinGW warns "Please include winsock2.h before windows.h" and wpcap.dll gets
 * an incompatible struct sockaddr / struct timeval layout).
 */
#if defined(UDP2RAW_MP) && defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>

#define PCAP_DYN_IMPLEMENTATION
#include "pcap_dyn.h"

#endif /* UDP2RAW_MP && _WIN32 */
