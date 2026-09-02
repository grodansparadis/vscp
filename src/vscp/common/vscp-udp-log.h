/*
 * vscp-udp-log.h — header-only UDP debug logging, gated by an env var.
 *
 * If VSCP_ENABLE_UDP_DEBUG is not set in the environment, no socket is
 * ever created and every VSCP_UDP_LOG() call only performs a single flag
 * check.
 *
 * If it is set, the first log call creates a UDP socket lazily and sends
 * log lines to the target address (default 127.0.0.1:9999, overridable via
 * VSCP_UDP_DEBUG_IP / VSCP_UDP_DEBUG_PORT).
 *
 * Example:
 *     VSCP_UDP_LOG("state=%d addr=0x%02X", state, addr);
 *
 * Override target at runtime:
 *     Linux/macOS:
 *         VSCP_ENABLE_UDP_DEBUG=1 \
 *         VSCP_UDP_DEBUG_IP=192.168.1.42 \
 *         VSCP_UDP_DEBUG_PORT=9999 ./your_app
 *     Windows:
 *         set VSCP_ENABLE_UDP_DEBUG=1
 *         set VSCP_UDP_DEBUG_IP=192.168.1.42
 *         set VSCP_UDP_DEBUG_PORT=9999
 *         your_app.exe
 *
 * NOTE: state is per translation unit (each .c file that includes this
 * header gets its own static socket/state). For a single shared socket
 * across a whole binary, move the non-inline parts into one .c file and
 * keep only the macro + extern declarations in the header. For a shared
 * driver-DLL scenario, pair this with an exported Drv_SetLogTarget()
 * style function instead of relying purely on the env var per DLL.
 *
 * Thread-safe: initialization and enable-state checks are guarded by a
 * per-translation-unit mutex so concurrent first-use calls do not race.
 */
#ifndef VSCP_UDP_LOG_H
#define VSCP_UDP_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef SOCKET vscp_udp_sock_t;
  #define VSCP_UDP_INVALID_SOCK INVALID_SOCKET
  static INIT_ONCE vscp_udp_log_mutex_once = INIT_ONCE_STATIC_INIT;
  static CRITICAL_SECTION vscp_udp_log_mutex;
  static BOOL CALLBACK vscp_udp_log_mutex_init_once(PINIT_ONCE once, PVOID param, PVOID *context)
  {
      (void)once;
      (void)param;
      (void)context;
      InitializeCriticalSection(&vscp_udp_log_mutex);
      return TRUE;
  }
  #define VSCP_UDP_LOG_LOCK() do { InitOnceExecuteOnce(&vscp_udp_log_mutex_once, vscp_udp_log_mutex_init_once, NULL, NULL); EnterCriticalSection(&vscp_udp_log_mutex); } while (0)
  #define VSCP_UDP_LOG_UNLOCK() LeaveCriticalSection(&vscp_udp_log_mutex)
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <pthread.h>
  typedef int vscp_udp_sock_t;
  #define VSCP_UDP_INVALID_SOCK (-1)
  static pthread_mutex_t vscp_udp_log_mutex = PTHREAD_MUTEX_INITIALIZER;
  #define VSCP_UDP_LOG_LOCK() pthread_mutex_lock(&vscp_udp_log_mutex)
  #define VSCP_UDP_LOG_UNLOCK() pthread_mutex_unlock(&vscp_udp_log_mutex)
#endif

#define VSCP_UDP_LOG_DEFAULT_IP   "192.168.1.7"
#define VSCP_UDP_LOG_DEFAULT_PORT 9999
#define VSCP_UDP_LOG_MAX_MSG      512

typedef enum {
    VSCP_UDP_LOG_UNINIT = 0,
    VSCP_UDP_LOG_DISABLED,
    VSCP_UDP_LOG_ENABLED
} vscp_udp_log_state_t;

static vscp_udp_log_state_t vscp_udp_log_state = VSCP_UDP_LOG_UNINIT;
static vscp_udp_sock_t      vscp_udp_log_sock  = VSCP_UDP_INVALID_SOCK;
static struct sockaddr_in   vscp_udp_log_addr;
static struct in_addr       vscp_udp_log_ip_addr;
static int                  vscp_udp_log_ip_set = 0;
static int                  vscp_udp_log_port   = 0;

static inline int vscp_udp_log_set_ip(const char *ip)
{
    if (vscp_udp_log_state != VSCP_UDP_LOG_UNINIT || ip == NULL ||
        inet_pton(AF_INET, ip, &vscp_udp_log_ip_addr) != 1) {
        return 0;
    }

    vscp_udp_log_ip_set = 1;
    return 1;
}

static inline int vscp_udp_log_set_port(int port)
{
    if (vscp_udp_log_state != VSCP_UDP_LOG_UNINIT || port < 1 || port > 65535) {
        return 0;
    }

    vscp_udp_log_port = port;
    return 1;
}

static inline int vscp_udp_log_set_address(const char *ip, int port)
{
    if (!vscp_udp_log_set_ip(ip)) {
        return 0;
    }
    if (!vscp_udp_log_set_port(port)) {
        vscp_udp_log_ip_set = 0;
        return 0;
    }
    return 1;
}

static inline void vscp_udp_log_init_locked(void)
{
    if (vscp_udp_log_state != VSCP_UDP_LOG_UNINIT) {
        return;
    }

    if (getenv("VSCP_ENABLE_UDP_DEBUG") == NULL) {
        vscp_udp_log_state = VSCP_UDP_LOG_DISABLED;
        return;
    }

    const char *ip     = getenv("VSCP_UDP_DEBUG_IP");
    const char *port_s = getenv("VSCP_UDP_DEBUG_PORT");
    if (!ip) ip = VSCP_UDP_LOG_DEFAULT_IP;
    int port = vscp_udp_log_port ? vscp_udp_log_port : (port_s ? atoi(port_s) : VSCP_UDP_LOG_DEFAULT_PORT);

#ifdef _WIN32
    {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            vscp_udp_log_state = VSCP_UDP_LOG_DISABLED;
            return;
        }
    }
#endif

    vscp_udp_log_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (vscp_udp_log_sock == VSCP_UDP_INVALID_SOCK) {
        vscp_udp_log_state = VSCP_UDP_LOG_DISABLED;
        return;
    }

    memset(&vscp_udp_log_addr, 0, sizeof(vscp_udp_log_addr));
    vscp_udp_log_addr.sin_family = AF_INET;
    vscp_udp_log_addr.sin_port   = htons((unsigned short)port);
    if (port < 1 || port > 65535) {
        goto vscp_udp_log_init_failed;
    }
    if (vscp_udp_log_ip_set) {
        vscp_udp_log_addr.sin_addr = vscp_udp_log_ip_addr;
    }
    else if (inet_pton(AF_INET, ip, &vscp_udp_log_addr.sin_addr) != 1) {
        goto vscp_udp_log_init_failed;
    }

    vscp_udp_log_state = VSCP_UDP_LOG_ENABLED;
    return;

vscp_udp_log_init_failed:
    #ifdef _WIN32
        closesocket(vscp_udp_log_sock);
        WSACleanup();
    #else
        close(vscp_udp_log_sock);
    #endif
        vscp_udp_log_sock = VSCP_UDP_INVALID_SOCK;
        vscp_udp_log_state = VSCP_UDP_LOG_DISABLED;
        return;
}

/* Decide once whether UDP debug logging is enabled. Touches the network
 * stack ONLY if VSCP_ENABLE_UDP_DEBUG is set. */
static inline void vscp_udp_log_init(void)
{
    VSCP_UDP_LOG_LOCK();
    vscp_udp_log_init_locked();
    VSCP_UDP_LOG_UNLOCK();
}

static inline int vscp_udp_log_is_enabled(void)
{
    int enabled = 0;
    VSCP_UDP_LOG_LOCK();
    if (vscp_udp_log_state == VSCP_UDP_LOG_UNINIT) {
        vscp_udp_log_init_locked();
    }
    if (vscp_udp_log_state == VSCP_UDP_LOG_ENABLED) {
        enabled = 1;
    }
    VSCP_UDP_LOG_UNLOCK();
    return enabled;
}

static inline void vscp_udp_log_send(const char *msg)
{
    size_t msg_len = 0;
    if (msg == NULL) {
        return;
    }
    while (msg_len < VSCP_UDP_LOG_MAX_MSG && msg[msg_len] != '\0') {
        ++msg_len;
    }
    sendto(vscp_udp_log_sock, msg, (int)msg_len, 0,
           (struct sockaddr *)&vscp_udp_log_addr, sizeof(vscp_udp_log_addr));
}

/* Main macro — printf-style. First call per translation unit pays the
 * getenv() cost; every call after that is one integer comparison when
 * disabled. */
#define VSCP_UDP_LOG(...)                                              \
    do {                                                               \
        if (vscp_udp_log_is_enabled()) {                                \
            char vscp_udp_log_buf_[VSCP_UDP_LOG_MAX_MSG];              \
            snprintf(vscp_udp_log_buf_, sizeof(vscp_udp_log_buf_),     \
                      __VA_ARGS__);                                    \
            vscp_udp_log_send(vscp_udp_log_buf_);                      \
        }                                                               \
    } while (0)

#endif /* VSCP_UDP_LOG_H */
