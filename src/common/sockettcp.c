///////////////////////////////////////////////////////////////////////////////
// sockettcp.h:
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2004-2013 Sergey Lyubka
// Copyright (c) 2013-2017 the Civetweb developers ()
//
// Adopted for VSCP, Small changes  additions
// Copyright (c) 2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#if defined(_WIN32)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS /* Disable deprecation warning in VS2005 */
#endif
#ifndef _WIN32_WINNT /* defined for tdm-gcc so we can use getnameinfo */
#define _WIN32_WINNT 0x0501
#endif
#else
#if defined(__GNUC__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE /* for setgroups() */
#endif
#if defined(__linux__) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 600 /* For flockfile() on Linux */
#endif
#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE /* For fseeko(), ftello() */
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64 /* Use 64-bit file offsets by default */
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS /* <inttypes.h> wants this for C++ */
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS /* C++ wants that for INT64_MAX */
#endif
#ifdef __sun
#define __EXTENSIONS__  /* to expose flockfile and friends in stdio.h */
#define __inline inline /* not recognized on older compiler versions */
#endif
#endif

#if defined(__GNUC__) || defined(__MINGW32__)
/* Disable unused macros warnings - not all defines are required
* for all systems and all compilers. */
#pragma GCC diagnostic ignored "-Wunused-macros"
/* A padding warning is just plain useless */
#pragma GCC diagnostic ignored "-Wpadded"
#endif

#if defined(__clang__) /* GCC does not (yet) support this pragma */
/* We must set some flags for the headers we include. These flags
* are reserved ids according to C99, so we need to disable a
* warning for that. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-id-macro"
#endif

#if defined(_WIN32)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS /* Disable deprecation warning in VS2005 */
#endif
#if !defined(_WIN32_WINNT) /* defined for tdm-gcc so we can use getnameinfo */
#define _WIN32_WINNT 0x0501
#endif
#else
#if defined(__GNUC__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE /* for setgroups() */
#endif
#if defined(__linux__) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 600 /* For flockfile() on Linux */
#endif
#if !defined(_LARGEFILE_SOURCE)
#define _LARGEFILE_SOURCE /* For fseeko(), ftello() */
#endif
#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64 /* Use 64-bit file offsets by default */
#endif
#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS /* <inttypes.h> wants this for C++ */
#endif
#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS /* C++ wants that for INT64_MAX */
#endif
#if !defined(_DARWIN_UNLIMITED_SELECT)
#define _DARWIN_UNLIMITED_SELECT
#endif
#if defined(__sun)
#define __EXTENSIONS__  /* to expose flockfile and friends in stdio.h */
#define __inline inline /* not recognized on older compiler versions */
#endif
#endif

#if defined(__clang__)
/* Enable reserved-id-macro warning again. */
#pragma GCC diagnostic pop
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#include "vscpmd5.h"

/* Flags for SSL usage */
#define NO_SSL          0
#define USE_SSL         1

#if defined(_WIN32)

#include <winsock2.h> /* DTL add for SO_EXCLUSIVE */
#include <windows.h>
#include <ws2tcpip.h>

typedef const char *SOCK_OPT_TYPE;

#if !defined(PATH_MAX)
#define W_PATH_MAX (MAX_PATH)
/* at most three UTF-8 chars per wchar_t */
#define PATH_MAX (W_PATH_MAX * 3)
#else
#define W_PATH_MAX ((PATH_MAX + 2) / 3)
#endif

#ifndef _IN_PORT_T
#ifndef in_port_t
#define in_port_t u_short
#endif
#endif

#include <process.h>
#include <direct.h>
#include <io.h>

#define MAKEUQUAD(lo, hi)                                                      \
	((uint64_t)(((uint32_t)(lo)) | ((uint64_t)((uint32_t)(hi))) << 32))
#define RATE_DIFF (10000000) /* 100 nsecs */
#define EPOCH_DIFF (MAKEUQUAD(0xd53e8000, 0x019db1de))
#define SYS2UNIX_TIME(lo, hi)                                                  \
	((time_t)((MAKEUQUAD((lo), (hi)) - EPOCH_DIFF) / RATE_DIFF))

/* Visual Studio 6 does not know __func__ or __FUNCTION__
* The rest of MS compilers use __FUNCTION__, not C99 __func__
* Also use _strtoui64 on modern M$ compilers */
#if defined(_MSC_VER)
#if (_MSC_VER < 1300)
#define STRX(x) #x
#define STR(x) STRX(x)
#define __func__ __FILE__ ":" STR(__LINE__)
#define strtoull(x, y, z) ((unsigned __int64)_atoi64(x))
#define strtoll(x, y, z) (_atoi64(x))
#else
#define __func__ __FUNCTION__
#define strtoull(x, y, z) (_strtoui64(x, y, z))
#define strtoll(x, y, z) (_strtoi64(x, y, z))
#endif
#endif /* _MSC_VER */

#define ERRNO ((int)(GetLastError()))
#define NO_SOCKLEN_T

#if defined(_WIN64) || defined(__MINGW64__)
#define SSL_LIB "ssleay64.dll"
#define CRYPTO_LIB "libeay64.dll"
#else
#define SSL_LIB "ssleay32.dll"
#define CRYPTO_LIB "libeay32.dll"
#endif

#define O_NONBLOCK (0)
#ifndef W_OK
#define W_OK (2) /* http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx */
#endif
#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif /* !EWOULDBLOCK */
#define _POSIX_
#define INT64_FMT "I64d"
#define UINT64_FMT "I64u"

#define WINCDECL __cdecl
#define vsnprintf_impl _vsnprintf
#define access _access
#define mg_sleep(x) (Sleep(x))

#define pipe(x) _pipe(x, MG_BUF_LEN, _O_BINARY)
#ifndef popen
#define popen(x, y) (_popen(x, y))
#endif
#ifndef pclose
#define pclose(x) (_pclose(x))
#endif
#define close(x) (_close(x))
#define dlsym(x, y) (GetProcAddress((HINSTANCE)(x), (y)))
#define RTLD_LAZY (0)
#define fseeko(x, y, z) ((_lseeki64(_fileno(x), (y), (z)) == -1) ? -1 : 0)
#define fdopen(x, y) (_fdopen((x), (y)))
#define write(x, y, z) (_write((x), (y), (unsigned)z))
#define read(x, y, z) (_read((x), (y), (unsigned)z))
#define flockfile(x) (EnterCriticalSection(&global_log_file_lock))
#define funlockfile(x) (LeaveCriticalSection(&global_log_file_lock))
#define sleep(x) (Sleep((x)*1000))
#define rmdir(x) (_rmdir(x))
#define timegm(x) (_mkgmtime(x))
#define NEED_TIMEGM

#if !defined(fileno)
#define fileno(x) (_fileno(x))
#endif /* !fileno MINGW #defines fileno */

typedef HANDLE pthread_mutex_t;
typedef DWORD pthread_key_t;
typedef HANDLE pthread_t;

typedef struct {
    CRITICAL_SECTION threadIdSec;
    struct mg_workerTLS *waiting_thread; /* The chain of threads */
} pthread_cond_t;

#ifndef __clockid_t_defined
typedef DWORD clockid_t;
#endif
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC (1)
#endif
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME (2)
#endif
#ifndef CLOCK_THREAD
#define CLOCK_THREAD (3)
#endif
#ifndef CLOCK_PROCESS
#define CLOCK_PROCESS (4)
#endif


#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#define _TIMESPEC_DEFINED
#endif
#ifndef _TIMESPEC_DEFINED
struct timespec {
    time_t tv_sec; /* seconds */
    long tv_nsec;  /* nanoseconds */
};
#endif

#if !defined(WIN_PTHREADS_TIME_H)
#define MUST_IMPLEMENT_CLOCK_GETTIME
#endif

#ifdef MUST_IMPLEMENT_CLOCK_GETTIME
#define clock_gettime mg_clock_gettime
static int
clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    FILETIME ft;
    ULARGE_INTEGER li, li2;
    BOOL ok = FALSE;
    double d;
    static double perfcnt_per_sec = 0.0;

    if (tp) {
        memset(tp, 0, sizeof(*tp));

        if (clk_id == CLOCK_REALTIME) {

            /* BEGIN: CLOCK_REALTIME = wall clock (date and time) */
            GetSystemTimeAsFileTime(&ft);
            li.LowPart = ft.dwLowDateTime;
            li.HighPart = ft.dwHighDateTime;
            li.QuadPart -= 116444736000000000; /* 1.1.1970 in filedate */
            tp->tv_sec = (time_t)(li.QuadPart / 10000000);
            tp->tv_nsec = (long)(li.QuadPart % 10000000) * 100;
            ok = TRUE;
            /* END: CLOCK_REALTIME */

        }
        else if (clk_id == CLOCK_MONOTONIC) {

            /* BEGIN: CLOCK_MONOTONIC = stopwatch (time differences) */
            if (perfcnt_per_sec == 0.0) {
                QueryPerformanceFrequency((LARGE_INTEGER *)&li);
                perfcnt_per_sec = 1.0 / li.QuadPart;
            }
            if (perfcnt_per_sec != 0.0) {
                QueryPerformanceCounter((LARGE_INTEGER *)&li);
                d = li.QuadPart * perfcnt_per_sec;
                tp->tv_sec = (time_t)d;
                d -= tp->tv_sec;
                tp->tv_nsec = (long)(d * 1.0E9);
                ok = TRUE;
            }
            /* END: CLOCK_MONOTONIC */

        }
        else if (clk_id == CLOCK_THREAD) {

            /* BEGIN: CLOCK_THREAD = CPU usage of thread */
            FILETIME t_create, t_exit, t_kernel, t_user;
            if (GetThreadTimes(GetCurrentThread(),
                &t_create,
                &t_exit,
                &t_kernel,
                &t_user)) {
                li.LowPart = t_user.dwLowDateTime;
                li.HighPart = t_user.dwHighDateTime;
                li2.LowPart = t_kernel.dwLowDateTime;
                li2.HighPart = t_kernel.dwHighDateTime;
                li.QuadPart += li2.QuadPart;
                tp->tv_sec = (time_t)(li.QuadPart / 10000000);
                tp->tv_nsec = (long)(li.QuadPart % 10000000) * 100;
                ok = TRUE;
            }
            /* END: CLOCK_THREAD */

        }
        else if (clk_id == CLOCK_PROCESS) {

            /* BEGIN: CLOCK_PROCESS = CPU usage of process */
            FILETIME t_create, t_exit, t_kernel, t_user;
            if (GetProcessTimes(GetCurrentProcess(),
                &t_create,
                &t_exit,
                &t_kernel,
                &t_user)) {
                li.LowPart = t_user.dwLowDateTime;
                li.HighPart = t_user.dwHighDateTime;
                li2.LowPart = t_kernel.dwLowDateTime;
                li2.HighPart = t_kernel.dwHighDateTime;
                li.QuadPart += li2.QuadPart;
                tp->tv_sec = (time_t)(li.QuadPart / 10000000);
                tp->tv_nsec = (long)(li.QuadPart % 10000000) * 100;
                ok = TRUE;
            }
            /* END: CLOCK_PROCESS */

        }
        else {

            /* BEGIN: unknown clock */
            /* ok = FALSE; already set by init */
            /* END: unknown clock */
        }
    }

    return ok ? 0 : -1;
}
#endif

#define pid_t HANDLE /* MINGW typedefs pid_t to int. Using #define here. */

static int pthread_mutex_lock(pthread_mutex_t *);
static int pthread_mutex_unlock(pthread_mutex_t *);
static void path_to_unicode(const struct mg_connection *conn,
    const char *path,
    wchar_t *wbuf,
    size_t wbuf_len);

/* All file operations need to be rewritten to solve #246. */

struct mg_file;

static const char *
mg_fgets(char *buf, size_t size, struct mg_file *filep, char **p);


/* POSIX dirent interface */
struct dirent {
    char d_name[PATH_MAX];
};

typedef struct DIR {
    HANDLE handle;
    WIN32_FIND_DATAW info;
    struct dirent result;
} DIR;

#if defined(_WIN32) && !defined(POLLIN)
#if !defined(HAVE_POLL)
struct pollfd {
    SOCKET fd;
    short events;
    short revents;
};
#define POLLIN (0x0300)
#endif
#endif

/* Mark required libraries */
#if defined(_MSC_VER)
#pragma comment(lib, "Ws2_32.lib")
#endif

#else

#include <sys/errno.h>
#include <sys/time.h>
#include <sys/wait.h>
//#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <stdint.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/tcp.h>
typedef const void *SOCK_OPT_TYPE;

#include <pwd.h>
#include <unistd.h>
#include <grp.h>
#include <dirent.h>
#define vsnprintf_impl vsnprintf

#include <dlfcn.h>

#include <poll.h> // AKHE

#include <pthread.h>
#include <unistd.h>

#if !defined(SSL_LIB)
#define SSL_LIB "libssl.so"
#endif
#if !defined(CRYPTO_LIB)
#define CRYPTO_LIB "libcrypto.so"
#endif

#define INVALID_SOCKET (-1)
#define INT64_FMT PRId64
#define UINT64_FMT PRIu64
#define WINCDECL

#endif /* unix block */

/* Listen backlog   */
#if !defined(SOMAXCONN)
#define SOMAXCONN (100)
#endif

/* Size of the accepted socket queue */
#if !defined(MGSQLEN)
#define MGSQLEN (20)
#endif

#include <openssl/ssl.h>
#include <openssl/err.h> 
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/engine.h>
#include <openssl/conf.h>
#include <openssl/dh.h>
#include <openssl/bn.h>
#include <openssl/opensslv.h>

#include <vscpmd5.h>
#include "sockettcp.h"



#if defined(_WIN32)  

#define pid_t HANDLE /* MINGW typedefs pid_t to int. Using #define here. */

static int pthread_mutex_lock(pthread_mutex_t *);
static int pthread_mutex_unlock(pthread_mutex_t *);


static void path_to_unicode( const struct mg_connection *conn,
                                const char *path,
                                wchar_t *wbuf,
                                size_t wbuf_len);


#if defined(_WIN32) && !defined(POLLIN)
#ifndef HAVE_POLL
struct pollfd {
	SOCKET fd;
	short events;
	short revents;
};
#define POLLIN (0x0300)
#endif
#endif

/* Mark required libraries */
#if defined(_MSC_VER)
#pragma comment(lib, "Ws2_32.lib")
#endif




#else  /* defined(_WIN32)  WINDOWS / UNIX include block */

#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <stdint.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/tcp.h>
typedef const void *SOCK_OPT_TYPE;


#include <pwd.h>
#include <unistd.h>
#include <grp.h>
#include <dirent.h>
#define vsnprintf_impl vsnprintf

#if !defined(NO_SSL_DL) && !defined(NO_SSL)
#include <dlfcn.h>
#endif
#include <pthread.h>
#if defined(__MACH__)
#define SSL_LIB "libssl.dylib"
#define CRYPTO_LIB "libcrypto.dylib"
#else
#if !defined(SSL_LIB)
#define SSL_LIB "libssl.so"
#endif
#if !defined(CRYPTO_LIB)
#define CRYPTO_LIB "libcrypto.so"
#endif
#endif
#ifndef O_BINARY
#define O_BINARY (0)
#endif /* O_BINARY */
#define closesocket(a) (close(a))
#define mg_mkdir(conn, path, mode) (mkdir(path, mode))
#define mg_remove(conn, x) (remove(x))
#define mg_sleep(x) (usleep((x)*1000))
#define mg_opendir(conn, x) (opendir(x))
#define mg_closedir(x) (closedir(x))
#define mg_readdir(x) (readdir(x))
#define ERRNO (errno)
#define INVALID_SOCKET (-1)
#define INT64_FMT PRId64
#define UINT64_FMT PRIu64
typedef int SOCKET;
#define WINCDECL

#if defined(__hpux)
/* HPUX 11 does not have monotonic, fall back to realtime */
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC CLOCK_REALTIME
#endif

/* HPUX defines socklen_t incorrectly as size_t which is 64bit on
 * Itanium.  Without defining _XOPEN_SOURCE or _XOPEN_SOURCE_EXTENDED
 * the prototypes use int* rather than socklen_t* which matches the
 * actual library expectation.  When called with the wrong size arg
 * accept() returns a zero client inet addr and check_acl() always
 * fails.  Since socklen_t is widely used below, just force replace
 * their typedef with int. - DTL
 */
#define socklen_t int
#endif /* hpux */

#endif /* defined(_WIN32) && !defined(__SYMBIAN32__) -                         \
          WINDOWS / UNIX include block */

#if defined(_MSC_VER)
/* 'type cast' : conversion from 'int' to 'HANDLE' of greater size */
#pragma warning(disable : 4306)
/* conditional expression is constant: introduced by FD_SET(..) */
#pragma warning(disable : 4127)
/* non-constant aggregate initializer: issued due to missing C99 support */
#pragma warning(disable : 4204)
/* padding added after data member */
#pragma warning(disable : 4820)
/* not defined as a preprocessor macro, replacing with '0' for '#if/#elif' */
#pragma warning(disable : 4668)
/* no function prototype given: converting '()' to '(void)' */
#pragma warning(disable : 4255)
/* function has been selected for automatic inline expansion */
#pragma warning(disable : 4711)
#endif

/* DTL -- including winsock2.h works better if lean and mean */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef __clang__
/* When using -Weverything, clang does not accept it's own headers
 * in a release build configuration. Disable what is too much in
 * -Weverything. */
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

#define SHUTDOWN_RD (0)
#define SHUTDOWN_WR (1)
#define SHUTDOWN_BOTH (2)

// stcp_init_library counter
static int stcp_init_called = 0;

static int stcp_ssl_initialized = 0;

static pthread_key_t sTlsKey; // Thread local storage index

struct stcp_workerTLS {
    int is_master;
    unsigned long thread_idx;
#if defined(_WIN32)
    HANDLE pthread_cond_helper_mutex;
    struct stcp_workerTLS *next_waiting_thread;
#endif
};

#define stcp_sleep(x) (usleep((x)*1000))


////////////////////////////////////////////////////////////////////////////////
// stcp_get_current_time_ns
//

static uint64_t
stcp_get_current_time_ns( void )
{
    struct timespec tsnow;
    clock_gettime(CLOCK_REALTIME, &tsnow);
    return ( ( (uint64_t)tsnow.tv_sec ) * 1000000000) + (uint64_t)tsnow.tv_nsec;
}


// ****************** Windows specific ******************

#if defined(_WIN32)

#if defined(_WIN32) && !defined(POLLIN)
#ifndef HAVE_POLL
struct pollfd {
    SOCKET fd;
    short events;
    short revents;
};
#define POLLIN (0x0300)
#endif
#endif


void usleep(__int64 usec)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}


#ifndef HAVE_POLL
static int
stcp_poll(struct pollfd *pfd, unsigned int n, int milliseconds)
{
    struct timeval tv;
    fd_set set;
    unsigned int i;
    int result;
    SOCKET maxfd = 0;

    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    FD_ZERO(&set);

    for (i = 0; i < n; i++) {
        FD_SET((SOCKET)pfd[i].fd, &set);
        pfd[i].revents = 0;

        if (pfd[i].fd > maxfd) {
            maxfd = pfd[i].fd;
        }
    }

    if ((result = select((int)maxfd + 1, &set, NULL, NULL, &tv)) > 0) {
        for (i = 0; i < n; i++) {
            if (FD_ISSET(pfd[i].fd, &set)) {
                pfd[i].revents = POLLIN;
            }
        }
    }

    /* We should subtract the time used in select from remaining
    * "milliseconds", in particular if called from mg_poll with a
    * timeout quantum.
    * Unfortunately, the remaining time is not stored in "tv" in all
    * implementations, so the result in "tv" must be considered undefined.
    * See http://man7.org/linux/man-pages/man2/select.2.html */

    return result;
}
#endif /* HAVE_POLL */


////////////////////////////////////////////////////////////////////////////////
// set_blocking_mode
//

static int
set_blocking_mode(int sock)
{
    unsigned long non_blocking = 0;
    return ioctlsocket(sock, (long) FIONBIO, &non_blocking);
}

static int
set_non_blocking_mode(int sock)
{
    unsigned long non_blocking = 1;
    return ioctlsocket(sock, (long)FIONBIO, &non_blocking);
}

///////////////////////////////////////////////////////////////////////////////
//                                 PTHREAD
///////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// pthread_mutex_init
//

static int
pthread_mutex_init(pthread_mutex_t *mutex, void *unused)
{
    (void) unused;
    *mutex = CreateMutex(NULL, FALSE, NULL);
    return (*mutex == NULL) ? -1 : 0;
}

////////////////////////////////////////////////////////////////////////////////
// pthread_mutex_destroy
//

static int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    return (CloseHandle(*mutex) == 0) ? -1 : 0;
}

////////////////////////////////////////////////////////////////////////////////
// pthread_mutex_lock
//

static int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
    return ( WaitForSingleObject(*mutex, (DWORD) INFINITE) == WAIT_OBJECT_0) ? 0
            : -1;
}

////////////////////////////////////////////////////////////////////////////////
// pthread_mutex_trylock
//

static int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    switch ( WaitForSingleObject(*mutex, 0) ) {

        case WAIT_OBJECT_0:
            return 0;

        case WAIT_TIMEOUT:
            return -2; // EBUSY

    }

    return -1;
}


////////////////////////////////////////////////////////////////////////////////
// pthread_mutex_unlock
//

static int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    return (ReleaseMutex(*mutex) == 0) ? -1 : 0;
}

////////////////////////////////////////////////////////////////////////////////
// pthread_cond_init
//

static int
pthread_cond_init(pthread_cond_t *cv, const void *unused)
{
    (void) unused;
    InitializeCriticalSection(&cv->threadIdSec);
    cv->waiting_thread = NULL;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// pthread_cond_timedwait
//

static int
pthread_cond_timedwait(pthread_cond_t *cv,
                       pthread_mutex_t *mutex,
                       const struct timespec *abstime)
{
    struct stcp_workerTLS **ptls,
            *tls = (struct stcp_workerTLS *) pthread_getspecific(sTlsKey);
    int ok;
    int64_t nsnow, nswaitabs, nswaitrel;
    DWORD mswaitrel;

    EnterCriticalSection(&cv->threadIdSec);

    // Add this thread to cv's waiting list
    ptls = &cv->waiting_thread;

    for (; *ptls != NULL; ptls = &(*ptls)->next_waiting_thread) {
        ;
    }

    tls->next_waiting_thread = NULL;
    *ptls = tls;
    LeaveCriticalSection(&cv->threadIdSec);

    if (abstime) {
        nsnow = stcp_get_current_time_ns();
        nswaitabs =
                (((int64_t) abstime->tv_sec) * 1000000000) + abstime->tv_nsec;
        nswaitrel = nswaitabs - nsnow;

        if (nswaitrel < 0) {
            nswaitrel = 0;
        }

        mswaitrel = (DWORD) (nswaitrel / 1000000);
    }
    else {
        mswaitrel = (DWORD) INFINITE;
    }

    pthread_mutex_unlock(mutex);
    ok = (WAIT_OBJECT_0
            == WaitForSingleObject(tls->pthread_cond_helper_mutex, mswaitrel));

    if (!ok) {
        ok = 1;
        EnterCriticalSection(&cv->threadIdSec);
        ptls = &cv->waiting_thread;

        for (; *ptls != NULL; ptls = &(*ptls)->next_waiting_thread) {

            if (*ptls == tls) {
                *ptls = tls->next_waiting_thread;
                ok = 0;
                break;
            }

        }

        LeaveCriticalSection(&cv->threadIdSec);

        if (ok) {
            WaitForSingleObject(tls->pthread_cond_helper_mutex,
                                (DWORD) INFINITE);
        }

    }

    // This thread has been removed from cv's waiting list
    pthread_mutex_lock(mutex);

    return ok ? 0 : -1;
}

////////////////////////////////////////////////////////////////////////////////
// pthread_cond_wait
//

static int
pthread_cond_wait( pthread_cond_t *cv, pthread_mutex_t *mutex )
{
    return pthread_cond_timedwait( cv, mutex, NULL );
}

////////////////////////////////////////////////////////////////////////////////
// pthread_cond_signal
//

static int
pthread_cond_signal( pthread_cond_t *cv )
{
    HANDLE wkup = NULL;
    BOOL ok = FALSE;

    // TODO EnterCriticalSection( &cv->threadIdSec );

    if (cv->waiting_thread) {
        // TODO wkup = cv->waiting_thread->pthread_cond_helper_mutex;
        // TODO cv->waiting_thread = cv->waiting_thread->next_waiting_thread;

        ok = SetEvent(wkup);
        assert(ok);
    }

    LeaveCriticalSection( &cv->threadIdSec );

    return ok ? 0 : 1;
}

////////////////////////////////////////////////////////////////////////////////
// pthread_cond_broadcast
//

static int
pthread_cond_broadcast( pthread_cond_t *cv )
{
    EnterCriticalSection( &cv->threadIdSec );

    while ( cv->waiting_thread ) {
        pthread_cond_signal(cv);
    }

    LeaveCriticalSection( &cv->threadIdSec );

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// pthread_cond_destroy
//

static int
pthread_cond_destroy( pthread_cond_t *cv )
{
    EnterCriticalSection( &cv->threadIdSec );
    assert( NULL == cv->waiting_thread );
    LeaveCriticalSection( &cv->threadIdSec );
    DeleteCriticalSection( &cv->threadIdSec );

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// event_create
//

static void *
event_create(void)
{
    return (void *) CreateEvent(NULL, FALSE, FALSE, NULL);
}

////////////////////////////////////////////////////////////////////////////////
// event_wait
//

static int
event_wait(void *eventhdl)
{
    int res = WaitForSingleObject((HANDLE) eventhdl, (DWORD) INFINITE);
    return (res == WAIT_OBJECT_0);
}

////////////////////////////////////////////////////////////////////////////////
// event_signal
//

static int
event_signal(void *eventhdl)
{
    return (int) SetEvent((HANDLE) eventhdl);
}

////////////////////////////////////////////////////////////////////////////////
// event_destroy
//

static void
event_destroy(void *eventhdl)
{
    CloseHandle((HANDLE) eventhdl);
}

////////////////////////////////////////////////////////////////////////////////
// set_close_on_exec
//

static void
set_close_on_exec(SOCKET sock)
{
    (void)sock; // Unused.
}


////////////////////////////////////////////////////////////////////////////////
// report_error
//

// TODO Make general

static void
stcp_report_error(const char *fmt, ...)
{
    va_list args;

    //flockfile(stdout);   // TODO
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    putchar('\n');
    //funlockfile(stdout); // TODO
    fflush(stdout);
}


#else   // windows vs. unix



// ****************** Unix specific ******************


////////////////////////////////////////////////////////////////////////////////
// report_error
//

// TODO Make general

static void 
stcp_report_error( const char *fmt, ... ) 
{
    va_list args;

    flockfile( stdout );
    va_start( args, fmt );
    vprintf( fmt, args );
    va_end( args );
    putchar('\n');
    funlockfile( stdout );
    fflush( stdout );
}


////////////////////////////////////////////////////////////////////////////////
// set_close_on_exec
//

static void
set_close_on_exec( SOCKET fd )
{
    if ( fcntl( fd, F_SETFD, FD_CLOEXEC ) != 0 ) {
        ;
    }

}


////////////////////////////////////////////////////////////////////////////////
// set_blocking_mode
//

static int
set_non_blocking_mode(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);

    if ( flags < 0 ) {
	    return -1;
    }

    if ( fcntl( sock, F_SETFL, ( flags | O_NONBLOCK ) ) < 0 ) {
	    return -1;
    }

    return 0;
}

static int
set_blocking_mode( int sock )
{
    int flags = fcntl( sock, F_GETFL, 0 );

    if ( flags < 0 ) {
	    return -1;
    }

    if ( fcntl( sock, F_SETFL, flags & (~(int)(O_NONBLOCK ) ) ) < 0 ) {
	    return -1;
    }

    return 0;
}

#endif    // windows vs. unix

/* va_copy should always be a macro, C99 and C++11 - DTL */
#ifndef va_copy
#define va_copy(x, y) ((x) = (y))
#endif

#ifdef _WIN32
/* Create substitutes for POSIX functions in Win32. */

#if defined(__MINGW32__)
/* Show no warning in case system functions are not used. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif


static CRITICAL_SECTION global_log_file_lock;

static DWORD
pthread_self(void)
{
    return GetCurrentThreadId();
}


static int
pthread_key_create(
    pthread_key_t *key,
    void(*_ignored)(void *) /* destructor not supported for Windows */
)
{
    (void)_ignored;

    if ((key != 0)) {
        *key = TlsAlloc();
        return (*key != TLS_OUT_OF_INDEXES) ? 0 : -1;
    }
    return -2;
}


static int
pthread_key_delete(pthread_key_t key)
{
    return TlsFree(key) ? 0 : 1;
}


static int
pthread_setspecific(pthread_key_t key, void *value)
{
    return TlsSetValue(key, value) ? 0 : 1;
}


/*static void *
pthread_getspecific(pthread_key_t key)
{
    return TlsGetValue(key);
}*/

#if defined(__MINGW32__)
/* Enable unused function warning again */
#pragma GCC diagnostic pop
#endif

static struct pthread_mutex_undefined_struct *pthread_mutex_attr = NULL;
#else
static pthread_mutexattr_t pthread_mutex_attr;
#endif /* _WIN32 */


/* mg_init_library counter */
static int mg_init_library_called = 0;

#if !defined(NO_SSL)
static int mg_ssl_initialized = 0;
#endif

static pthread_key_t sTlsKey; /* Thread local storage index */
static int thread_idx_max = 0;

#if defined(MG_LEGACY_INTERFACE)
#define MG_ALLOW_USING_GET_REQUEST_INFO_FOR_RESPONSE
#endif

struct mg_workerTLS {
    int is_master;
    unsigned long thread_idx;
#if defined(_WIN32) 
    HANDLE pthread_cond_helper_mutex;
    struct mg_workerTLS *next_waiting_thread;
#endif
#if defined(MG_ALLOW_USING_GET_REQUEST_INFO_FOR_RESPONSE)
    char txtbuf[4];
#endif
};


#if defined(__GNUC__) || defined(__MINGW32__)
/* Show no warning in case system functions are not used. */
#if GCC_VERSION >= 40500
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif /* GCC_VERSION >= 40500 */
#endif /* defined(__GNUC__) || defined(__MINGW32__) */
#if defined(__clang__)
/* Show no warning in case system functions are not used. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif


static const char *
stcp_ssl_error( void );     // Forward declaration

static pthread_mutex_t global_lock_mutex;


#if defined(_WIN32)
// Forward declaration for Windows
static int pthread_mutex_lock(pthread_mutex_t *mutex);

static int pthread_mutex_unlock(pthread_mutex_t *mutex);
#endif


////////////////////////////////////////////////////////////////////////////////
// stcp_global_lock
//

static void
stcp_global_lock(void)
{
    (void)pthread_mutex_lock( &global_lock_mutex );
}

////////////////////////////////////////////////////////////////////////////////
// stcp_global_unlock
//

static void
stcp_global_unlock(void)
{
    (void)pthread_mutex_unlock( &global_lock_mutex );
}


////////////////////////////////////////////////////////////////////////////////
// atomic_inc
//

static int
atomic_inc(volatile int *addr)
{
    int ret;
#if defined(_WIN32)  && !defined(NO_ATOMICS)
    // Depending on the SDK, this function uses either
    // (volatile unsigned int *) or (volatile LONG *),
    // so whatever you use, the other SDK is likely to raise a warning. */
    ret = InterlockedIncrement((volatile long *) addr);
#elif defined(__GNUC__)                                                        \
    && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)))           \
    && !defined(NO_ATOMICS)
    ret = __sync_add_and_fetch(addr, 1);
#else
    stcp_global_lock();
    ret = (++(*addr));
    stcp_global_unlock();
#endif
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// atomic_dec
//

static int
atomic_dec(volatile int *addr)
{
    int ret;
#if defined( _WIN32 )  && !defined( NO_ATOMICS )
    // Depending on the SDK, this function uses either
    // (volatile unsigned int *) or (volatile LONG *),
    // so whatever you use, the other SDK is likely to raise a warning.
    ret = InterlockedDecrement((volatile long *) addr);
#elif defined(__GNUC__)                                                        \
    && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 0)))           \
    && !defined(NO_ATOMICS)
    ret = __sync_sub_and_fetch(addr, 1);
#else
    stcp_global_lock();
    ret = (--(*addr));
    stcp_global_unlock();
#endif
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// stcp_current_thread_id
//
// Get a unique thread ID as unsigned long, independent from the data type
// of thread IDs defined by the operating system API.
// If two calls to stcp_current_thread_id  return the same value, they calls
// are done from the same thread. If they return different values, they are
// done from different threads. (Provided this function is used in the same
// process context and threads are not repeatedly created and deleted, but
// vscpweb does not do that).
// This function must match the signature required for SSL id callbacks:
// CRYPTO_set_id_callback
//

static unsigned long
stcp_current_thread_id( void )
{
#ifdef _WIN32
    return GetCurrentThreadId();
#else

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
    // For every compiler, either "sizeof(pthread_t) > sizeof(unsigned long)"
    // or not, so one of the two conditions will be unreachable by construction.
    // Unfortunately the C standard does not define a way to check this at
    // compile time, since the #if preprocessor conditions can not use the sizeof
    // operator as an argument.
#endif

    if ( sizeof( pthread_t ) > sizeof( unsigned long ) ) {

        // This is the problematic case for CRYPTO_set_id_callback:
        // The OS pthread_t can not be cast to unsigned long.
        struct stcp_workerTLS *tls =
                (struct stcp_workerTLS *)pthread_getspecific( sTlsKey );

        if ( NULL == tls ) {

            // SSL called from an unknown thread: Create some thread index.
            tls = (struct stcp_workerTLS *)malloc(sizeof (struct stcp_workerTLS));
            tls->is_master = -2; /* -2 means "3rd party thread" */
            tls->thread_idx = (unsigned)atomic_inc(&thread_idx_max);
            pthread_setspecific( sTlsKey, tls );

        }

        return tls->thread_idx;
    }
    else {

        // pthread_t may be any data type, so a simple cast to unsigned long
        // can rise a warning/error, depending on the platform.
        // Here memcpy is used as an anything-to-anything cast.
        unsigned long ret = 0;
        pthread_t t = pthread_self();
        memcpy(&ret, &t, sizeof (pthread_t));
        return ret;

    }

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif

}

/* Darwin prior to 7.0 and Win32 do not have socklen_t */
#ifdef NO_SOCKLEN_T
typedef int socklen_t;
#endif /* NO_SOCKLEN_T */
#define _DARWIN_UNLIMITED_SELECT

#define IP_ADDR_STR_LEN (50) /* IPv6 hex string is 46 chars */

#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL (0)
#endif


////////////////////////////////////////////////////////////////////////////////
// strlcpy
//

static void strlcpy( register char *dst, register const char *src, size_t n )
{
    for (; *src != '\0' && n > 1; n--) {
        *dst++ = *src++;
    }
    
    *dst = '\0';
}


////////////////////////////////////////////////////////////////////////////////
// stcp_strndup
//

static char *stcp_strndup( const char *ptr, size_t len )
{
    char *p;

    if ( ( p = (char *)malloc( len + 1 ) ) != NULL ) {
        strlcpy( p, ptr, len + 1 );
    }

    return p;
}

////////////////////////////////////////////////////////////////////////////////
// stcp_strdup
//

static char *stcp_strdup( const char *str )
{
    return strndup( str, strlen( str ) );
}

////////////////////////////////////////////////////////////////////////////////
// hexdump2string
//

static int
hexdump2string( void *mem, int memlen, char *buf, int buflen )
{
    int i;
    const char hexdigit[] = "0123456789abcdef";

    if ( ( memlen <= 0 ) || ( buflen <= 0 ) ) {
        return 0;
    }

    if ( buflen < (3 * memlen) ) {
        return 0;
    }

    for ( i = 0; i < memlen; i++)  {
        if ( i > 0 ) {
            buf[3 * i - 1] = ' ';
        }
        buf[3 * i] = hexdigit[(((uint8_t *) mem)[i] >> 4) & 0xF];
        buf[3 * i + 1] = hexdigit[((uint8_t *) mem)[i] & 0xF];
    }
    
    buf[3 * memlen - 1] = 0;

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// sockaddr_to_string
//

static void
sockaddr_to_string( char *buf, size_t len, const union usa *usa )
{
    buf[0] = '\0';

    if ( !usa ) {
        return;
    }

    if ( AF_INET == usa->sa.sa_family ) {

        getnameinfo( &usa->sa,
                        sizeof( usa->sin ),
                        buf,
                        (unsigned)len,
                        NULL,
                        0,
                        NI_NUMERICHOST );

    }
    else if ( AF_INET6 == usa->sa.sa_family ) {

        getnameinfo( &usa->sa,
                        sizeof( usa->sin6 ),
                        buf,
                        (unsigned)len,
                        NULL,
                        0,
                        NI_NUMERICHOST );

    }

}


///////////////////////////////////////////////////////////////////////////////
//                                 OPENSSL
///////////////////////////////////////////////////////////////////////////////

#ifdef OPENSSL_API_1_1
#else
static pthread_mutex_t *ssl_mutexes;
#endif /* OPENSSL_API_1_1 */

////////////////////////////////////////////////////////////////////////////////
// ssl_use_pem_file
//

static int
ssl_use_pem_file( SSL_CTX *ssl_ctx, const char *pem, const char *chain )
{
    if ( 0 == SSL_CTX_use_certificate_file( ssl_ctx, pem, 1 ) ) {
        stcp_report_error( "Cannot open certificate file %s: %s",
                            pem,
                            stcp_ssl_error() );
        return 0;
    }

    // could use SSL_CTX_set_default_passwd_cb_userdata
    if ( 0 == SSL_CTX_use_PrivateKey_file( ssl_ctx, pem, 1 ) ) {
        stcp_report_error( "Cannot open private key file %s: %s",
                            pem,
                            stcp_ssl_error() );
        return 0;
    }

    if ( 0 == SSL_CTX_check_private_key( ssl_ctx ) ) {
        stcp_report_error( "Certificate and private key do not match: %s",
                            pem );
        return 0;
    }

    // In contrast to OpenSSL, wolfSSL does not support certificate
    // chain files that contain private keys and certificates in
    // SSL_CTX_use_certificate_chain_file.
    // The vscpweb-Server used pem-Files that contained both information.
    // In order to make wolfSSL work, it is split in two files.
    // One file that contains key and certificate used by the server and
    // an optional chain file for the ssl stack.
    //
    if (chain) {
        if ( 0 == SSL_CTX_use_certificate_chain_file( ssl_ctx, chain ) ) {
            stcp_report_error( "Cannot use certificate chain file %s: %s",
                                pem,
                                stcp_ssl_error() );
            return 0;
        }
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// refresh_trust
//

static int
refresh_trust( struct stcp_connection *conn, 
                const char *pem, 
                const char *chain,
                const char *ca_path,
                const char *ca_file )
{
    static int reload_lock = 0;
    static long int data_check = 0;
    volatile int *p_reload_lock = (volatile int *) &reload_lock;

    struct stat cert_buf;
    long int t;
    //const char *pem;
    //const char *chain;
    int should_verify_peer;

    if ( NULL == pem ) {
        // If peem is NULL and conn->ctx->callbacks.init_ssl is not,
        // refresh_trust still can not work.
        return 0;
    }
    
    if ( NULL == chain ) {
        // pem is not NULL here
        chain = pem;
    }
    if (*chain == 0) {
        chain = NULL;
    }

    t = data_check;
    if (stat(pem, &cert_buf) != -1) {
        t = (long int) cert_buf.st_mtime;
    }

    if (data_check != t) {
        
        data_check = t;

        should_verify_peer = 0;
        if ( 1 == STCP_SSL_DO_VERIFY_PEER ) {
            should_verify_peer = 1;
        }
        else if ( 0 == STCP_SSL_DO_VERIFY_PEER ) {
            should_verify_peer = 1;
        }

        if ( should_verify_peer ) {
            
            if ( SSL_CTX_load_verify_locations( conn->ssl_ctx,
                                                    ca_file,
                                                    ca_path ) != 1) {
                stcp_report_error(
                            "SSL_CTX_load_verify_locations error: %s "
                            "ssl_verify_peer requires setting "
                            "either ssl_ca_path or ssl_ca_file. Is any of them "
                            "present in "
                            "the .conf file?",
                            stcp_ssl_error() );
                return 0;
            }
        }

        if ( 1 == atomic_inc( p_reload_lock ) ) {
            if ( 0 == ssl_use_pem_file( conn->ssl_ctx, pem, chain ) ) {
                return 0;
            }
            *p_reload_lock = 0;
        }
    }
    // lock while cert is reloading
    while (*p_reload_lock) {
        sleep(1);
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// make_ssl
//
// Make socket SSL socket
//

static int
make_ssl( struct stcp_connection *conn,
            struct stcp_secure_options *secure_opts,
            SSL_CTX *s,
            int (*func)(SSL *),
            volatile int *stop_server )
{
    int ret, err;
    unsigned i;

    if ( ( NULL == conn ) || ( NULL == secure_opts ) ) {
        return 0;
    }

    if ( STCP_SSL_SHORT_TRUST ) {
        int trust_ret = refresh_trust( conn, 
                                        secure_opts->pem,
                                        secure_opts->chain,
                                        secure_opts->ca_path,
                                        secure_opts->ca_file ); 
        if ( !trust_ret ) {
            return trust_ret;
        }
    }

    conn->ssl = SSL_new( s );
    if ( NULL == conn->ssl ) {
        return 0;
    }

    SSL_set_app_data( conn->ssl, (char *)conn );

    ret = SSL_set_fd( conn->ssl, conn->client.sock );
    if ( ret != 1 ) {
        err = SSL_get_error( conn->ssl, ret );
        (void)err; // TODO: set some error message
        SSL_free( conn->ssl );
        conn->ssl = NULL;
        // Avoid CRYPTO_cleanup_all_ex_data(); See discussion:
        // https://wiki.openssl.org/index.php/Talk:Library_Initialization
#ifndef OPENSSL_API_1_1
        ERR_remove_state( 0 );    // deprecated in 1.0.0, solved by going to 1.1.0
#endif
        return 0;
    }

    // SSL functions may fail and require to be called again:
    // see https://www.openssl.org/docs/manmaster/ssl/SSL_get_error.html
    // Here "func" could be SSL_connect or SSL_accept.
    for ( i = 16; i <= 1024; i *= 2 ) {

        ret = func( conn->ssl );
        if  (ret != 1 ) {

            err = SSL_get_error( conn->ssl, ret );
            if ( ( err == SSL_ERROR_WANT_CONNECT ) ||
                 ( err == SSL_ERROR_WANT_ACCEPT ) ||
                 ( err == SSL_ERROR_WANT_READ ) ||
                 ( err == SSL_ERROR_WANT_WRITE ) ) {
                // Need to retry the function call "later".
                // See https://linux.die.net/man/3/ssl_get_error
                // This is typical for non-blocking sockets.
                if ( *stop_server ) {
                    // Don't wait if the server is going to be stopped.
                    break;
                }

                stcp_sleep( i );

            }
            else if ( err == SSL_ERROR_SYSCALL ) {
                // This is an IO error. Look at errno.
                err = errno;
                // TODO: set some error message
                (void)err;
                break;
            }
            else {
                // This is an SSL specific error
                // TODO: set some error message
                break;
            }

        }
        else {
            // success
            break;
        }
    }

    if ( ret != 1 ) {
        SSL_free( conn->ssl );
        conn->ssl = NULL;
        // Avoid CRYPTO_cleanup_all_ex_data(); See discussion:
        // https://wiki.openssl.org/index.php/Talk:Library_Initialization
#ifndef OPENSSL_API_1_1
        ERR_remove_state( 0 );    // deprecated in 1.0.0, solved by going to 1.1.0
#endif
        return 0;
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// ssl_error
//
// Return OpenSSL error message (from CRYPTO lib)
//

static const char *
stcp_ssl_error( void )
{
    unsigned long err;
    err = ERR_get_error();
    return ( ( err == 0 ) ? "" : ERR_error_string( err, NULL ) );
}


////////////////////////////////////////////////////////////////////////////////
// ssl_get_client_cert_info
//

static void
ssl_get_client_cert_info( struct stcp_connection *conn,
                            struct stcp_srv_client_cert *client_cert )
{
    X509 *cert = SSL_get_peer_certificate( conn->ssl );
    if ( cert ) {
        char str_subject[1024];
        char str_issuer[1024];
        char str_finger[1024];
        unsigned char buf[256];
        char *str_serial = NULL;
        unsigned int ulen;
        int ilen;
        unsigned char *tmp_buf;
        unsigned char *tmp_p;

        /* Handle to algorithm used for fingerprint */
        const EVP_MD *digest = EVP_get_digestbyname("sha1");

        /* Get Subject and issuer */
        X509_NAME *subj = X509_get_subject_name( cert );
        X509_NAME *iss = X509_get_issuer_name( cert );

        /* Get serial number */
        ASN1_INTEGER *serial = X509_get_serialNumber( cert );

        /* Translate serial number to a hex string */
        BIGNUM *serial_bn = ASN1_INTEGER_to_BN( serial, NULL );
        str_serial = BN_bn2hex( serial_bn );
        BN_free( serial_bn );

        /* Translate subject and issuer to a string */
        (void)X509_NAME_oneline( subj, str_subject, (int)sizeof( str_subject ) );
        (void)X509_NAME_oneline( iss, str_issuer, (int)sizeof( str_issuer ) );

        /* Calculate SHA1 fingerprint and store as a hex string */
        ulen = 0;

        /* 
            ASN1_digest is deprecated. Do the calculation manually,
            using EVP_Digest. 
        */
        ilen = i2d_X509( cert, NULL );
        tmp_buf = (ilen > 0) ? (unsigned char *)malloc( (unsigned)ilen + 1 )
                             : NULL;
        if ( tmp_buf ) {
            
            tmp_p = tmp_buf;
            (void)i2d_X509( cert, &tmp_p );
            if ( !EVP_Digest( tmp_buf, (unsigned)ilen, buf, &ulen, digest, NULL ) ) {
                ulen = 0;
            }

            free( tmp_buf );
        }

        if ( !hexdump2string( buf, (int)ulen, str_finger, (int)sizeof( str_finger ) ) ) {
            *str_finger = 0;
        }

        client_cert = (struct stcp_srv_client_cert *)
		                                malloc( sizeof( struct stcp_srv_client_cert ) );

        if ( client_cert ) {
            client_cert->subject = strdup( str_subject );
            client_cert->issuer = strdup( str_issuer );
            client_cert->serial = strdup( str_serial );
            client_cert->finger = strdup( str_finger );
        }
        else {
            stcp_report_error( "Out of memory: Cannot allocate memory for client "
                               "certificate" );
        }

        /* 
            Strings returned from bn_bn2hex must be freed using OPENSSL_free,
            see https://linux.die.net/man/3/bn_bn2hex
        */
        OPENSSL_free( str_serial );

        /* Free certificate memory */
        X509_free( cert );
    }

}


////////////////////////////////////////////////////////////////////////////////
// ssl_locking_callback
//

#ifdef OPENSSL_API_1_1

    // Not needed of for 1.1

#else

static void
ssl_locking_callback( int mode, int mutex_num, const char *file, int line )
{
    (void)line;
    (void)file;

    if ( mode & 1 ) {
        // 1 is CRYPTO_LOCK
        (void)pthread_mutex_lock( &ssl_mutexes[ mutex_num ] );
    }
    else {
        (void)pthread_mutex_unlock( &ssl_mutexes[ mutex_num ] );
    }
}
#endif

/*
    If multithreading part of  ssl is initialized elsewhere
    SSL_ALREADY_INITIALIZED should be defined whdn compiling
    sockettcp.c
*/

#if defined(SSL_ALREADY_INITIALIZED)
static int cryptolib_users = 1; // Reference counter for crypto library.
#else
static int cryptolib_users = 0; // Reference counter for crypto library.
#endif


////////////////////////////////////////////////////////////////////////////////
// stcp_init_mt_ssl
//

int
stcp_init_mt_ssl( void )
{

#ifdef OPENSSL_API_1_1

    if ( atomic_inc( &cryptolib_users ) > 1 ) {
        return 1;
    }

#else // not OPENSSL_API_1_1

    int i;
    size_t size;

    if ( atomic_inc( &cryptolib_users ) > 1 ) {
        return 1;
    }

    // Initialize locking callbacks, needed for thread safety.
    // http://www.openssl.org/support/faq.html#PROG1
    //
    i = CRYPTO_num_locks();
    if ( i < 0 ) {
        i = 0;
    }

    size = sizeof( pthread_mutex_t ) * ((size_t)(i));

    if ( 0 == size ) {
        ssl_mutexes = NULL;
    }
    else if ( NULL == ( ssl_mutexes = (pthread_mutex_t *)malloc( size ) ) ) {
        stcp_report_error( "Cannot allocate mutexes: %s", stcp_ssl_error() );
        return 0;
    }

    for ( i = 0; i < CRYPTO_num_locks(); i++ ) {
        pthread_mutex_init( &ssl_mutexes[i], (void *)&pthread_mutex_attr );
    }

    CRYPTO_set_locking_callback( &ssl_locking_callback );
    CRYPTO_set_id_callback( &stcp_current_thread_id );

#endif // OPENSSL_API_1_1

    return 1;
}





#ifdef OPENSSL_API_1_1

////////////////////////////////////////////////////////////////////////////////
// ssl_get_protocol
//

static unsigned long
ssl_get_protocol(int version_id)
{
    long unsigned ret = SSL_OP_ALL;
    if (version_id > 0)
        ret |= SSL_OP_NO_SSLv2;
    if (version_id > 1)
        ret |= SSL_OP_NO_SSLv3;
    if (version_id > 2)
        ret |= SSL_OP_NO_TLSv1;
    if (version_id > 3)
        ret |= SSL_OP_NO_TLSv1_1;
    return ret;
}

#else

////////////////////////////////////////////////////////////////////////////////
// ssl_get_protocol
//

static long
ssl_get_protocol( int version_id )
{
    long ret = SSL_OP_ALL;
    if ( version_id > 0 )
        ret |= SSL_OP_NO_SSLv2;
    if ( version_id > 1 )
        ret |= SSL_OP_NO_SSLv3;
    if ( version_id > 2 )
        ret |= SSL_OP_NO_TLSv1;
    if ( version_id > 3 )
        ret |= SSL_OP_NO_TLSv1_1;
    return ret;
}
#endif // OPENSSL_API_1_1

////////////////////////////////////////////////////////////////////////////////
// ssl_info_callback
//
// SSL callback documentation:
// https://www.openssl.org/docs/man1.1.0/ssl/SSL_set_info_callback.html
// https://linux.die.net/man/3/ssl_set_info_callback
//

static void
ssl_info_callback(SSL *ssl, int what, int ret)
{
    (void) ret;

    if ( what & SSL_CB_HANDSHAKE_START ) {
        SSL_get_app_data(ssl);
    }
    if ( what & SSL_CB_HANDSHAKE_DONE)  {
        // TODO: check for openSSL 1.1
        // #define SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS 0x0001
        // ssl->s3->flags |= SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS;
    }
}

////////////////////////////////////////////////////////////////////////////////
// stcp_init_ssl
//
// Dynamically load SSL library.
//

int
stcp_init_ssl( SSL_CTX *ssl_ctx, struct stcp_secure_options *secure_opts )
{
    int callback_ret;
    int should_verify_peer;
    int peer_certificate_optional;
    int use_default_verify_paths;
    int verify_depth;
    time_t now_rt = time(NULL);
    struct timespec now_mt;
    md5_byte_t ssl_context_id[16];
    md5_state_t md5state;
    int protocol_ver;

    /* Must have secure options */
    if ( NULL == secure_opts ) {
        return 0;
    }

    /* 
        If PEM file is not specified and the init_ssl callback
        is not specified, skip SSL initialization.
    */

    if ( NULL == secure_opts->pem ) {
        return 1;
    }

    if ( NULL == secure_opts->chain ) {
        secure_opts->chain = secure_opts->pem;
    }
    
    if ( ( secure_opts->chain != NULL ) && ( *secure_opts->chain == 0 ) ) {
        secure_opts->chain = NULL;
    }

    /* Init. ssl multithread locks for ssl 1.0 */
    if ( !(secure_opts->bNOInitMT) ) {
        if ( !stcp_init_mt_ssl() ) {
            stcp_report_error( "Failed to init ssl\n" );
            return 0;
        }
    }


#ifdef OPENSSL_API_1_1
    // Initialize SSL library
    OPENSSL_init_ssl(0, NULL);
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS
                     | OPENSSL_INIT_LOAD_CRYPTO_STRINGS,
                     NULL);

    if ( NULL == ( ssl_ctx = SSL_CTX_new( TLS_server_method() ) ) ) {
        stcp_report_error( "SSL_CTX_new (server) error: %s", stcp_ssl_error() );
        return 0;
    }
#else
    // Initialize SSL library
    SSL_library_init();
    SSL_load_error_strings();

    if ( NULL == ( ssl_ctx = SSL_CTX_new( SSLv23_server_method() ) ) ) {
        stcp_report_error( "SSL_CTX_new (server) error: %s", stcp_ssl_error() );
        return 0;
    }
#endif // OPENSSL_API_1_1

    SSL_CTX_clear_options( ssl_ctx,
                            SSL_OP_NO_SSLv2 | 
                            SSL_OP_NO_SSLv3 | 
                            SSL_OP_NO_TLSv1 | 
                            SSL_OP_NO_TLSv1_1 );

    SSL_CTX_set_options( ssl_ctx, ssl_get_protocol( STCP_SSL_PROTOCOL_VERSION ) );
    SSL_CTX_set_options( ssl_ctx, SSL_OP_SINGLE_DH_USE);
    SSL_CTX_set_options( ssl_ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
    SSL_CTX_set_options( ssl_ctx, SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
    SSL_CTX_set_options( ssl_ctx, SSL_OP_NO_COMPRESSION);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
#endif
    // Depending on the OpenSSL version, the callback may be
    // 'void (*)(SSL *, int, int)' or 'void (*)(const SSL *, int, int)'
    // yielding in an "incompatible-pointer-type" warning for the other
    // version. It seems to be "unclear" what is correct:
    // https://bugs.launchpad.net/ubuntu/+source/openssl/+bug/1147526
    // https://www.openssl.org/docs/man1.0.2/ssl/ssl.html
    // https://www.openssl.org/docs/man1.1.0/ssl/ssl.html
    // https://github.com/openssl/openssl/blob/1d97c8435171a7af575f73c526d79e1ef0ee5960/ssl/ssl.h#L1173
    // Disable this warning here.
    // Alternative would be a version dependent ssl_info_callback and
    // a const-cast to call 'char *SSL_get_app_data(SSL *ssl)' there.
    //
    SSL_CTX_set_info_callback( ssl_ctx, (void *)ssl_info_callback );

#ifdef __clang__
#pragma clang diagnostic pop
#endif


    // Use some UID as session context ID.   TODO
    vscpmd5_init( &md5state );
    vscpmd5_append( &md5state, (const md5_byte_t *)&now_rt, sizeof( now_rt ) );
    clock_gettime( CLOCK_MONOTONIC, &now_mt );
    vscpmd5_append( &md5state, (const md5_byte_t *)&now_mt, sizeof( now_mt ) );
    vscpmd5_append( &md5state,
                        (const md5_byte_t *)"Stay foolish be hungry",
                        strlen("Stay foolish be hungry") );
    vscpmd5_append( &md5state, (const md5_byte_t *)secure_opts, sizeof (*secure_opts) );
    vscpmd5_finish( &md5state, ssl_context_id );

    SSL_CTX_set_session_id_context( ssl_ctx,
                                    (const unsigned char *)&ssl_context_id,
                                    sizeof( ssl_context_id ) );

    if ( secure_opts->pem != NULL ) {

        if ( !ssl_use_pem_file( ssl_ctx, 
                                    secure_opts->pem, 
                                    secure_opts->chain ) ) {
            return 0;
        }
    }

    // Should we support client certificates?
    // Default is "no".
    should_verify_peer = 0;
    peer_certificate_optional = 0;
    if ( 1 == STCP_SSL_DO_VERIFY_PEER ) {
        // Mandatory
        should_verify_peer = 1;
        peer_certificate_optional = 0;
    }
    else if ( 2 == STCP_SSL_DO_VERIFY_PEER ) {
        // Optional
        should_verify_peer = 1;
        peer_certificate_optional = 1;
    }
   

    use_default_verify_paths = STCP_SSL_DEFAULT_VERIFY_PATHS;

    if ( should_verify_peer ) {

        if ( SSL_CTX_load_verify_locations( ssl_ctx, 
                                                secure_opts->ca_file, 
                                                secure_opts->ca_path )
            != 1) {
            stcp_report_error(
                        "SSL_CTX_load_verify_locations error: %s "
                        "ssl_verify_peer requires setting "
                        "either ssl_ca_path or ssl_ca_file. Is any of them "
                        "present in "
                        "the .conf file?",
                        stcp_ssl_error() );
            return 0;
        }

        if ( peer_certificate_optional ) {
            SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_PEER, NULL);
        }
        else {
            SSL_CTX_set_verify( ssl_ctx,
                                    SSL_VERIFY_PEER |
                                    SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                                    NULL );
        }

        if ( use_default_verify_paths &&
                ( SSL_CTX_set_default_verify_paths( ssl_ctx ) != 1 ) ) {
            stcp_report_error( "SSL_CTX_set_default_verify_paths error: %s",
                                stcp_ssl_error() );
            return 0;
        }

        SSL_CTX_set_verify_depth( ssl_ctx, STCP_SSL_VERIFY_DEPTH );

    }

    if ( SSL_CTX_set_cipher_list( ssl_ctx, STCP_SSL_CIPHER_LIST ) != 1 ) {
        stcp_report_error( "SSL_CTX_set_cipher_list error: %s", stcp_ssl_error() );
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// stcp_uninit_ssl
//

void
stcp_uninit_ssl( void )
{
#ifdef OPENSSL_API_1_1

    if ( 0 == atomic_dec( &cryptolib_users ) ) {

        // Shutdown according to
        // https://wiki.openssl.org/index.php/Library_Initialization#Cleanup
        // http://stackoverflow.com/questions/29845527/how-to-properly-uninitialize-openssl
        //
        CONF_modules_unload(1);
#else
    int i;

    if ( 0 == atomic_dec( &cryptolib_users ) ) {

        // Shutdown according to
        // https://wiki.openssl.org/index.php/Library_Initialization#Cleanup
        // http://stackoverflow.com/questions/29845527/how-to-properly-uninitialize-openssl
        //
        CRYPTO_set_locking_callback(NULL);
        CRYPTO_set_id_callback(NULL);
        ENGINE_cleanup();
        CONF_modules_unload(1);
        ERR_free_strings();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
        ERR_remove_state(0);        // deprecated in 1.0.0, solved by going to 1.1.0

        for (i = 0; i < CRYPTO_num_locks(); i++) {
            pthread_mutex_destroy( &ssl_mutexes[i] );
        }
        free(ssl_mutexes);
        ssl_mutexes = NULL;
#endif // OPENSSL_API_1_1
    }
}


////////////////////////////////////////////////////////////////////////////////
// is_valid_port
//

static int
is_valid_port( unsigned long port )
{
    return (port <= 0xffff);
}


////////////////////////////////////////////////////////////////////////////////
// stcp_inet_pton
//
// af - Address family (AF_INET, AF_INET6)
// src - host (srv.domain.com)
// dst - ip-address
// dstlen - size of buufer for ip-address
//

static int
stcp_inet_pton( int af, const char *src, void *dst, size_t dstlen )
{
    struct addrinfo hints, *res, *ressave;
    int func_ret = 0;
    int gai_ret;

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = af;

    gai_ret = getaddrinfo(src, NULL, &hints, &res);
    if (gai_ret != 0) {

        // gai_strerror could be used to convert gai_ret to a string
        // POSIX return values: see
        // http://pubs.opengroup.org/onlinepubs/9699919799/functions/freeaddrinfo.html
        //
        // Windows return values: see
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520%28v=vs.85%29.aspx
        //
        return 0;
    }

    ressave = res;

    while (res) {

        if (dstlen >= (size_t) res->ai_addrlen) {
            memcpy(dst, res->ai_addr, res->ai_addrlen);
            func_ret = 1;
        }

        res = res->ai_next;
    }

    freeaddrinfo(ressave);

    return func_ret;
}


////////////////////////////////////////////////////////////////////////////////
// set_tcp_nodelay
//

static int
set_tcp_nodelay( int sock, int nodelay_on )
{
    if ( setsockopt( sock,
                        IPPROTO_TCP,
                        TCP_NODELAY,
                        (SOCK_OPT_TYPE) &nodelay_on,
                        sizeof( nodelay_on ) ) != 0 ) {
        // Error
        return 1;
    }

    /// OK
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// stcp_connect_socket
//

static int
stcp_connect_socket( const char *hostip,
                        int port,
                        int use_ssl,
                        int *sock,          // output: socket, must not be NULL
                        union usa *sa       // output: socket address, must not be NULL
                    )
{
    int ip_ver = 0;
    *sock = INVALID_SOCKET;
    memset(sa, 0, sizeof (*sa));

    if ( NULL == hostip ) {
        stcp_report_error( "NULL host" );
        return 0;
    }

    if ( (port <= 0) || !is_valid_port((unsigned) port)) {
        stcp_report_error("invalid port");
        return 0;
    }

    (void)use_ssl;

    if ( stcp_inet_pton( AF_INET, hostip, &sa->sin, sizeof( sa->sin ) ) ) { // .sin_addr
        sa->sin.sin_family = AF_INET;
        sa->sin.sin_port = htons((uint16_t) port);
        ip_ver = 4;
    }
    else if ( stcp_inet_pton( AF_INET6, hostip, &sa->sin6, sizeof( sa->sin6 ) ) ) { // .sin6_addr
        sa->sin6.sin6_family = AF_INET6;
        sa->sin6.sin6_port = htons((uint16_t) port);
        ip_ver = 6;
    }
    else if ( hostip[0] == '[' ) {
        // While getaddrinfo on Windows will work with [::1],
        // getaddrinfo on Linux only works with ::1 (without []).
        size_t l = strlen(hostip + 1);
        char *h = (l > 1) ? strdup(hostip + 1) : NULL;
        if ( h ) {
            h[l - 1] = 0;
            if ( stcp_inet_pton(AF_INET6, h, &sa->sin6, sizeof( sa->sin6 ) ) ) { // .sin6_addr
                sa->sin6.sin6_family = AF_INET6;
                sa->sin6.sin6_port = htons((uint16_t) port);
                ip_ver = 6;
            }
            free(h);
        }

    }

    if ( 0 == ip_ver ) {
        stcp_report_error("host not found");
        return 0; 
    }
    
    if ( 4 == ip_ver ) {
        *sock = socket(PF_INET, SOCK_STREAM, 0);
    }
    else if ( 6 == ip_ver ) {
        *sock = socket(PF_INET6, SOCK_STREAM, 0);
    }

    if (*sock == INVALID_SOCKET) {
        stcp_report_error("socket(): %s");
                            // strerror(ERRNO) );
        return 0;
    }

    if ( ( 4 == ip_ver ) &&
            ( 0 == connect( *sock,
                                (struct sockaddr *)&sa->sin,
                                sizeof(sa->sin) ) ) ) {
        // connected with IPv4
        if ( 0 == set_non_blocking_mode( *sock ) ) {
            return 1;   // Ok
	    }

        // failed
	    // TODO: specific error message
        
    }

    if ( ( 6 == ip_ver ) &&
        ( 0 == connect( *sock,
                            (struct sockaddr *)&sa->sin6,
                            sizeof(sa->sin6) ) ) ) {
        // connected with IPv6
        if ( 0 == set_non_blocking_mode(*sock) ) {
            return 1;   // Ok
        }

	    // failed
	    // TODO: specific error message

    }

    // Not connected
    stcp_report_error("connect(%s:%d): %s",
                        hostip,
                        port,
                        strerror( ERRNO ) );
#ifdef _WIN32
    closesocket(*sock);
#else
    close(*sock);
#endif
    *sock = INVALID_SOCKET;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// stcp_connect_remote_impl
//

static struct stcp_connection *
stcp_connect_remote_impl( const char *host,
                            int port,
                            struct stcp_secure_options *secure_options,
                            int bUseSSL,
                            int timeout )
{
    struct stcp_connection *conn = NULL;
    int sock;
    union usa sa;
    struct sockaddr *psa;
    socklen_t len;

    // Need secure options if SSL
    if ( bUseSSL && ( NULL == secure_options ) ) {
        return 0;
    }

    conn = (struct stcp_connection *)calloc( 1, sizeof( struct stcp_connection ) );
    if ( NULL == conn ) {
        // Error
        return NULL;
    }

    if ( bUseSSL ) {
        // Init SSL subsystem
        if ( 0 == stcp_init_ssl( conn->ssl_ctx, secure_options ) ) {
            free( conn );
            return NULL;
        }
    }

    if ( !stcp_connect_socket( host,
                                port,
                                bUseSSL,
                                &sock,
                                &sa ) ) {
        // ebuf is set by connect_socket,
        // free all memory and return NULL;
        free( conn );
        return NULL;
    }

#ifdef OPENSSL_API_1_1
    if ( bUseSSL &&
            ( NULL == ( conn->ssl_ctx = SSL_CTX_new( TLS_client_method() ) ) ) ) {
        stcp_report_error("SSL_CTX_new error");
        close(sock);
        free(conn);
        return NULL;
    }
#else
    if ( bUseSSL &&
            ( NULL == ( conn->ssl_ctx = SSL_CTX_new( SSLv23_client_method() ) ) ) ) {
        unsigned long ssl_err = ERR_get_error();
        const char* const str = ERR_reason_error_string( ssl_err );
        stcp_report_error("SSL_CTX_new error. %s", str );
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        free(conn);
        return NULL;
    }
#endif // OPENSSL_API_1_1

    len = ( sa.sa.sa_family == AF_INET) ? sizeof( conn->client.rsa.sin )
                                            : sizeof( conn->client.rsa.sin6 );
    psa = ( sa.sa.sa_family == AF_INET )
            ? (struct sockaddr *)&( conn->client.rsa.sin )
                : (struct sockaddr *)&( conn->client.rsa.sin6 );

    conn->client.sock = sock;
    conn->client.lsa = sa;

    if ( getsockname(sock, psa, &len) != 0 ) {
        stcp_report_error( "getsockname() failed: %s", strerror( ERRNO ) );
    }

    conn->client.is_ssl = bUseSSL ? 1 : 0;

    if ( bUseSSL ) {

        // TODO: Check ssl_verify_peer and ssl_ca_path here.
        // SSL_CTX_set_verify call is needed to switch off server
        // certificate checking, which is off by default in OpenSSL and
        // on in yaSSL.
        // TODO: SSL_CTX_set_verify(conn->client_ssl_ctx,
        // SSL_VERIFY_PEER, verify_ssl_server);

        if ( secure_options->client_cert_path ) {
            if ( !ssl_use_pem_file( conn->ssl_ctx,
                                    secure_options->client_cert_path,
                                    NULL ) ) {
                stcp_report_error( "Can not use SSL client certificate" );
                SSL_CTX_free( conn->ssl_ctx );
#ifdef _WIN32
                closesocket(sock);
#else
                close(sock);
#endif
                free( conn );
                return NULL;
            }
        }

        // Set default locations for trusted CA certificates (file in pem format)
        if ( secure_options->server_cert_path ) {
            SSL_CTX_load_verify_locations( conn->ssl_ctx,
                                            secure_options->server_cert_path,
                                            NULL );
            SSL_CTX_set_verify( conn->ssl_ctx, SSL_VERIFY_PEER, NULL );
        }
        else {
            SSL_CTX_set_verify( conn->ssl_ctx, SSL_VERIFY_NONE, NULL );
        }

        if ( !make_ssl( conn,
                        secure_options,
                        conn->ssl_ctx,
                        SSL_connect,
                        &(conn->stop_flag ) ) ) {
            stcp_report_error("SSL connection error");
            SSL_CTX_free( conn->ssl_ctx );
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            free( conn );
            return NULL;
        }
        
    }

    if ( 0 != set_non_blocking_mode( sock ) ) {
        // TODO: handle error
        ;
    }

    conn->conn_state = STCP_CONN_STATE_CONNECTED;
    return conn;
}

////////////////////////////////////////////////////////////////////////////////
// stcp_connect_remote_secure
//

struct stcp_connection *
stcp_connect_remote_secure( const char *host,
                                int port,
                                struct stcp_secure_options *client_options,
                                int timeout )
{
    struct stcp_connection *conn;

    conn =  stcp_connect_remote_impl( host,
                                        port,
                                        client_options,
                                        USE_SSL,
                                        timeout );
                                    
    return conn;                                        
}

////////////////////////////////////////////////////////////////////////////////
// stcp_connect_remote
//

struct stcp_connection *
stcp_connect_remote( const char *host,
                        int port,
                        int timeout )
{ 
    return stcp_connect_remote_impl( host,
                                        port,
                                        NULL,
                                        NO_SSL,
                                        timeout );
}


////////////////////////////////////////////////////////////////////////////////
// stcp_close_socket_gracefully
//

static void
stcp_close_socket_gracefully( struct stcp_connection *conn )
{
#if defined(_WIN32)
    char buf[STCP_BUF_LEN];
    int n;
#endif
    struct linger linger;
    int error_code = 0;
    int linger_timeout = -2;
    socklen_t opt_len = sizeof (error_code);

    if ( NULL == conn ) {
        return;
    }

    // http://msdn.microsoft.com/en-us/library/ms739165(v=vs.85).aspx:
    // "Note that enabling a nonzero timeout on a nonblocking socket
    // is not recommended.", so set it to blocking now
    set_blocking_mode( conn->client.sock );

    /// Send FIN to the client
    shutdown(conn->client.sock, SHUTDOWN_WR);


#if defined(_WIN32)
    // Read and discard pending incoming data. If we do not do that and
    // close
    // the socket, the data in the send buffer may be discarded. This
    // behaviour is seen on Windows, when client keeps sending data
    // when server decides to close the connection; then when client
    // does recv() it gets no data back.
    do {
        n = stcp_pull_inner( NULL, conn, buf, sizeof( buf ), /* Timeout in s: */ 1.0 );
    }
    while (n > 0);
#endif

    // Set linger option according to configuration
    if (STCP_LINGER_TIMEOUT >= 0) {
        // Set linger option to avoid socket hanging out after close. This
        // prevent ephemeral port exhaust problem under high QPS.
        linger.l_onoff = 1;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#endif
#if defined(__GNUC__) || defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
        // Data type of linger structure elements may differ,
        // so we don't know what cast we need here.
        // Disable type conversion warnings.
        linger.l_linger = (linger_timeout + 999) / 1000;

#if defined(__GNUC__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    }
    else {
        linger.l_onoff = 0;
        linger.l_linger = 0;
    }

    if (STCP_LINGER_TIMEOUT < -1) {
        // Default: don't configure any linger
    }
    else if ( getsockopt( conn->client.sock,
                            SOL_SOCKET,
                            SO_ERROR,
                            (char *) &error_code,
                            &opt_len) != 0 ) {
        // Cannot determine if socket is already closed. This should
        // not occur and never did in a test. Log an error message
        // and continue.
        stcp_report_error("getsockopt(SOL_SOCKET SO_ERROR) failed: %s",
                            strerror(ERRNO) );
    }
    else if ( error_code == ECONNRESET ) {
        // Socket already closed by client/peer, close socket without linger
    }
    else {

        // Set linger timeout
        if ( setsockopt( conn->client.sock,
                            SOL_SOCKET,
                            SO_LINGER,
                            (char *)&linger,
                            sizeof( linger ) ) != 0 ) {
            stcp_report_error("setsockopt(SOL_SOCKET SO_LINGER(%i,%i)) failed: %s",
                                linger.l_onoff,
                                linger.l_linger,
                                strerror( ERRNO ) );
        }

    }

    // Now we know that our FIN is ACK-ed, safe to close
#ifdef _WIN32
    closesocket( conn->client.sock );
#else
    close( conn->client.sock );
#endif
    conn->client.sock = INVALID_SOCKET;
}

////////////////////////////////////////////////////////////////////////////////
// close_connection
//

static void
close_connection( struct stcp_connection *conn )
{
    if ( STCP_CONN_STATE_CONNECTED != conn->conn_state ) {
        conn->conn_state = STCP_CONN_STATE_CLOSED; // closed
        return;
    }

    conn->conn_state = STCP_CONN_STATE_TOCLOSE; // to close

    // Set close flag, so keep-alive loops will stop
    conn->must_close = 1;

    conn->conn_state = STCP_CONN_STATE_CLOSING; // closing

    if ( conn->ssl != NULL ) {
        // Run SSL_shutdown twice to ensure complexly close SSL connection
        SSL_shutdown( conn->ssl );
        SSL_free( conn->ssl );
        // Avoid CRYPTO_cleanup_all_ex_data(); See discussion:
        // https://wiki.openssl.org/index.php/Talk:Library_Initialization
#ifndef OPENSSL_API_1_1
        ERR_remove_state(0);        // deprecated in 1.0.0, solved by going to 1.1.0
#endif
        conn->ssl = NULL;
    }

    if ( conn->client.sock != INVALID_SOCKET ) {
        stcp_close_socket_gracefully( conn );
        conn->client.sock = INVALID_SOCKET;
    }

    conn->conn_state = STCP_CONN_STATE_CLOSED; // closed
}

////////////////////////////////////////////////////////////////////////////////
// stcp_close_connection
//

void
stcp_close_connection( struct stcp_connection *conn )
{
    if ( NULL == conn ) {
        return;
    }

    close_connection( conn );

    if ( conn->ssl_ctx != NULL ) {
        SSL_CTX_free( (SSL_CTX *)conn->ssl_ctx );
    }

    // If client free connection data
    free( conn );
    
}

/*!
 ****************************************************************************
 * stcp_poll
 *
 * Use milliseconds -1 to get STCP_TIMEOUT_QUANTUM timeout.
 *
 * @param pfd - Pointer to array with file descriptors.
 * @param n - Number of file descriptors in the pfd array.
 * @param milliseconds - Time to wait for data on file decriptor.
 *   If set too zero STCP_TIMEOUT_QUANTUM is used. If timout is
 * set lower than quantum one timeout is performed. If greater several.
 * @param stop_server - Pointer to int that can be set externally 
 *                      to stop the blocking operation.
 *
 * @return >0 success, -1 timeout, -2 stopped
 */

#ifndef _WIN32

int
stcp_poll( struct pollfd *pfd,
                unsigned int n,
                int mstimeout,
                volatile int *stop_server )
{
    // Call poll, but only for a maximum time of a few seconds.
    // This will allow to stop the server after some seconds, instead
    // of having to wait for a long socket timeout.
    int ms_now = STCP_TIMEOUT_QUANTUM; // Sleep quantum in ms

    do {
        
        int result;

        if ( *stop_server ) {
            // Shut down signal
            return -2;
        }

        // Set mstimeout to lowest value
        if ( ( mstimeout >= 0 ) &&
             ( mstimeout < ms_now ) ) {
            ms_now = mstimeout;
        }

        result = poll( pfd, n, ms_now );
        if ( result != 0 ) {
            // On success, a positive number is returned; this is the number of
            // structures which have nonzero revents fields. Negative return value
            // is error. Forward both to the caller.  (0 == timeout)
            return result;
        }
        
        // Poll timed out (==0)

        // Poll returned timeout (0).
        if ( mstimeout > 0 ) {
            mstimeout -= ms_now;
        }

    }
    while ( mstimeout > 0 );

    // timeout: return 0
    return 0;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// stcp_push_inner
//
//
// Write data to the IO channel - opened file descriptor, socket or SSL
// descriptor.
// Return value:
//  >=0 .. number of bytes successfully written
//   -1 .. timeout
//   -2 .. error
//

static int
stcp_push_inner( struct stcp_connection *conn,
                    FILE *fp,
                    const char *buf,
                    int len,
                    double timeout )
{
    uint64_t start = 0, now = 0, timeout_ns = 0;
    int n, err;
    unsigned ms_wait = STCP_TIMEOUT_QUANTUM; // Sleep quantum in ms

#ifdef _WIN32
    typedef int len_t;
#else
    typedef size_t len_t;
#endif

    if ( timeout > 0 ) {
        now = stcp_get_current_time_ns();
        start = now;
        timeout_ns = (uint64_t) (timeout * 1.0E9);
    }

    if (conn == NULL) {
        return -2;
    }

    // Try to read until it succeeds, fails, times out, or the server
    // shuts down.
    for (;;) {

        if ( conn->ssl != NULL ) {
            
            n = SSL_write(conn->ssl, buf, len);
            if (n <= 0) {
                err = SSL_get_error(conn->ssl, n);
                if ( ( SSL_ERROR_SYSCALL == err ) && ( -1 == n  ) ) {
                    err = errno;
                }
                else if (( err == SSL_ERROR_WANT_READ ) ||
                         ( err == SSL_ERROR_WANT_WRITE ) ) {
                    n = 0;
                }
                else {
                    //DEBUG_TRACE("SSL_write() failed, error %d", err);
                    return -2;
                }
            }
            else {
                err = 0;
            }
        }
        else

            if (fp != NULL) {
            n = (int) fwrite(buf, 1, (size_t) len, fp);
            if (ferror(fp)) {
                n = -1;
                err = errno;
            }
            else {
                err = 0;
            }
        }
        else {
            n = (int)send( conn->client.sock, buf, (len_t)len, MSG_NOSIGNAL );
            err = (n < 0) ? errno : 0;
#ifdef _WIN32
            if (err == WSAEWOULDBLOCK) {
                err = 0;
                n = 0;
            }
            else if (err == EPIPE ) {   // Broken pipe
                conn->conn_state = STCP_CONN_STATE_UNDEFINED;
                return -2;    
            }
#else
            if (err == EWOULDBLOCK) {
                err = 0;
                n = 0;
            }
            else if (err == EPIPE ) {  // Broken pipe
                conn->conn_state = STCP_CONN_STATE_UNDEFINED;
                return -2;    
            }
#endif
            if (n < 0) {
                // shutdown of the socket at client side
                return -2;
            }
        }

        if (conn->stop_flag) {
            return -2;
        }

        if ( (n > 0) || ((n == 0) && (len == 0) ) ) {
            // some data has been read, or no data was requested
            return n;
        }
        if ( n < 0 ) {
            // socket error - check errno
            //DEBUG_TRACE("send() failed, error %d", err);

            // TODO (mid): error handling depending on the error code.
            // These codes are different between Windows and Linux.
            // Currently there is no problem with failing send calls,
            // if there is a reproducible situation, it should be
            // investigated in detail.
            return -2;
        }

        // Only in case n=0 (timeout), repeat calling the write function

        // If send failed, wait before retry
        if ( fp != NULL ) {
            // For files, just wait a fixed time,
            // maybe an average disk seek time.
            stcp_sleep(ms_wait > 10 ? 10 : ms_wait);
        }
        else {
            // For sockets, wait for the socket using select
            fd_set wfds;
            struct timeval tv;
            int sret;

#if defined(__GNUC__) || defined(__MINGW32__)
            // GCC seems to have a flaw with it's own socket macros:
            // http://www.linuxquestions.org/questions/programming-9/impossible-to-use-gcc-with-wconversion-and-standard-socket-macros-841935/
            //
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

            FD_ZERO(&wfds);
            FD_SET( conn->client.sock, &wfds);
            tv.tv_sec = (time_t) (ms_wait / 1000);
            tv.tv_usec = (long) ((ms_wait % 1000) * 1000);

            sret = select( (int)conn->client.sock + 1, NULL, &wfds, NULL, &tv );

#if defined(__GNUC__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif

            if ( sret > 0 ) {
                // We got ready to write. Don't check the timeout
                // but directly go back to write again.
                continue;
            }
        }

        if  (timeout > 0 ) {
            now = stcp_get_current_time_ns();
            if ( (now - start) > timeout_ns ) {
                // Timeout
                break;
            }
        }
    }

    (void)err;  /* 
                    Avoid unused warning if NO_SSL is set and DEBUG_TRACE 
	                is not used 
                */

    return -1;
}

////////////////////////////////////////////////////////////////////////////////
// stcp_push_all
//

static int64_t
stcp_push_all( struct stcp_connection *conn,
                FILE *fp,
                const char *buf,
                int64_t len )
{
    int64_t n, nwritten = 0;

    while ( ( len > 0 ) && ( conn->stop_flag == 0 ) ) {

        n = stcp_push_inner( conn, 
                                fp, 
                                buf + nwritten, 
                                (int)len, 
                                STCP_WRITE_TIMEOUT );

        if (n < 0) {
            if (nwritten == 0) {
                nwritten = n; /* Propagate the error */
            }
            break;
        }
        else if (n == 0) {
            break; /* No more data to write */
        }
        else {
            nwritten += n;
            len -= n;
        }
    }

    return nwritten;
}

////////////////////////////////////////////////////////////////////////////////
// stcp_pull_inner
//
// Read from IO channel - opened file descriptor, socket, or SSL descriptor.
// Return value:
//  >=0 .. number of bytes successfully read
//   -1 .. timeout
//   -2 .. stopped (socket closed by remote)
//

static int
stcp_pull_inner( FILE *fp,
                struct stcp_connection *conn,
                char *buf,
                int len,
                int mstimeout )
{
    int nread, err = 0;

#ifdef _WIN32
    typedef int len_t;
#else
    typedef size_t len_t;
#endif
    int ssl_pending;


    // We need an additional wait loop around this, because in some cases
    // with TLSwe may get data from the socket but not from SSL_read.
    // In this case we need to repeat at least once.

    if ( fp != NULL ) {
        // Use read() instead of fread(), because if we're reading from the
        // CGI pipe, fread() may block until IO buffer is filled up. We
        // cannot afford to block and must pass all read bytes immediately
        // to the client.
        nread = (int)read( fileno(fp), buf, (size_t)len );
        err = (nread < 0) ? errno : 0;

        if ( ( 0 == nread ) && ( len > 0 ) ) {
            // Should get data, but got EOL
            return -2;
	    }

    }
    else if ( ( conn->ssl != NULL) &&
             ( ( ssl_pending = SSL_pending(conn->ssl) ) > 0 ) ) {
        // We already know there is no more data buffered in conn->buf
        // but there is more available in the SSL layer. So don't poll
        // conn->client.sock yet.
        if (ssl_pending > len) {
            ssl_pending = len;
        }

        nread = SSL_read( conn->ssl, buf, ssl_pending );

        if ( nread <= 0 ) {
            err = SSL_get_error(conn->ssl, nread);
            if ((err == SSL_ERROR_SYSCALL) && (nread == -1)) {
                err = errno;
            }
            else if ( ( err == SSL_ERROR_WANT_READ ) ||
                      ( err == SSL_ERROR_WANT_WRITE ) ) {
                nread = 0;
            }
            else {
                //DEBUG_TRACE("SSL_read() failed, error %d", err);
                return -1;
            }
        }
        else {
            err = 0;
        }

    }

    // SSL read
    else if ( conn->ssl != NULL ) {

        struct pollfd pfd[1];
        int pollres;

        pfd[0].fd = conn->client.sock;
        pfd[0].events = POLLIN;
        pollres = stcp_poll( pfd, 1, mstimeout, &(conn->stop_flag) );

        if ( conn->stop_flag ) {
            return -2;
        }

        if ( pollres > 0 ) {

            nread = SSL_read( conn->ssl, buf, len );
            
            if ( nread <= 0 ) {
                err = SSL_get_error( conn->ssl, nread );
                if ( ( err == SSL_ERROR_SYSCALL ) && ( nread == -1 ) ) {
                    err = errno;
                }
                else if ( ( err == SSL_ERROR_WANT_READ ) ||
                          ( err == SSL_ERROR_WANT_WRITE ) ) {
                    nread = 0;
                }
                else {
                    //DEBUG_TRACE("SSL_read() failed, error %d", err); TODO
                    return -2;
                }
            }
            else {
                err = 0;
            }

        }
        else if ( pollres < 0 ) {
            // Error
            return -2;
        }
        else {
            // pollres = 0 means timeout
            nread = 0;
        }

    }

    // Non SSL read
    else {
        struct pollfd pfd[1];
        int pollres; 

        pfd[0].fd = conn->client.sock;
        pfd[0].events = POLLIN;
        pollres = stcp_poll( pfd, 1, mstimeout, &( conn->stop_flag ) );

        if ( conn->stop_flag ) {
            return -2; 
        }

        if ( pollres > 0 ) {
            
            nread = (int)recv( conn->client.sock, buf, (len_t)len, 0 );
            err = (nread < 0) ? errno : 0;
            
            if ( nread <= 0 ) {
                // shutdown of the socket at client side
                return -2;
            }
        }
        else if ( pollres < 0 ) {
            // error callint poll
            return -2;
        }
        else {
            // pollres = 0 means timeout
            nread = 0;
        }
    }

    if ( conn->stop_flag ) {
        return -2;
    }

    if ( ( nread > 0 ) || ( ( nread == 0 ) && ( len == 0 ) ) ) {
        // some data has been read, or no data was requested
        return nread;
    }

    if ( nread < 0 ) {
        // socket error - check errno
#ifdef _WIN32
        if (err == WSAEWOULDBLOCK) {
            // TODO (low): check if this is still required
            // standard case if called from close_socket_gracefully
            return -2;
        }
        else if (err == WSAETIMEDOUT) {
            // TODO (low): check if this is still required
            // timeout is handled by the while loop
            return 0;
        }
        else if (err == WSAECONNABORTED) {
            // See https://www.chilkatsoft.com/p/p_299.asp
            return -2;
        }
        else {
            //DEBUG_TRACE("recv() failed, error %d", err);
            return -2;
        }
#else
        // TODO: POSIX returns either EAGAIN or EWOULDBLOCK in both cases,
        // if the timeout is reached and if the socket was set to non-
        // blocking in close_socket_gracefully, so we can not distinguish
        // here. We have to wait for the timeout in both cases for now.

        if ( ( err == EAGAIN ) || ( err == EWOULDBLOCK ) || ( err == EINTR ) ) {
            // TODO (low): check if this is still required
            // EAGAIN/EWOULDBLOCK:
            // standard case if called from close_socket_gracefully
            // => should return -1
            // or timeout occurred
            // => the code must stay in the while loop

            // EINTR can be generated on a socket with a timeout set even
            // when SA_RESTART is effective for all relevant signals
            // (see signal(7)).
            // => stay in the while loop
        }
        else {
            //DEBUG_TRACE("recv() failed, error %d", err);
            return -2;
        }
#endif
    }

    // Timeout occurred, but no data available.
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
// stcp_pull_all
//
// Read len bytes within timout set by conn.timeout
// Will read until timeout or error/abort.
// 
// >= 0 Read data
// < 0 - Error
//
// set mstimeout to -1 for no mstimeout

static int
stcp_pull_all( FILE *fp, struct stcp_connection *conn, char *buf, int len, int mstimeout )
{
    int n, nread = 0;
    uint64_t start_time = 0, now = 0, timeout_ns = 0;

    // Set timeout
    if ( mstimeout >= 0 ) {
        start_time = stcp_get_current_time_ns();
        timeout_ns = (uint64_t)((double)mstimeout * 1.0E6);
    }

    while ( ( len > 0 ) && ( 0 == conn->stop_flag ) ) {

        n = stcp_pull_inner( fp, conn, buf + nread, len, mstimeout );
        
        if ( STCP_ERROR_STOPPED == n ) {
            if ( 0 == nread ) {
                nread = -1; // Propagate the error
            }
            break;
        }
        else if ( STCP_ERROR_TIMEOUT == n ) {
            // timeout
            if ( mstimeout >= 0 ) {
                now = stcp_get_current_time_ns();
                if ( (now - start_time) <= timeout_ns ) {
                    continue;
                }
            }
            break;
        }
        else if ( n == 0 ) {
            break; // No more data to read
        }
        else {
            nread += n;
            len -= n;            
        }

    } // while

    return nread;
}


////////////////////////////////////////////////////////////////////////////////
// stcp_read_inner
//
// >= 0 Read data
// < 0 - Error
//

static int
stcp_read_inner( struct stcp_connection *conn, void *buf, size_t len, int mstimeout )
{
    int64_t n, nread;
    int64_t len64 =
            (int64_t) ((len > INT_MAX) ? INT_MAX : len); // since the return value is
                                                         // int, we may not read more
	                                                     // bytes
    if ( conn == NULL ) {
        return -1;
    }

    nread = 0;
    
    // Read new data from the remote socket.
    if ( ( n = stcp_pull_all( NULL, conn, (char *)buf, (int)len64, mstimeout ) ) >= 0 ) {
        nread += n;
    }
    else {
        nread = ( (nread > 0) ? nread : n); 
     }

    // Disconnected=
    if ( -2 == nread) {
        if (errno == EPIPE ) {  // Broken pipe
            conn->conn_state = STCP_CONN_STATE_UNDEFINED;
        }
    }

    return (int)nread;

}

////////////////////////////////////////////////////////////////////////////////
// stcp_getc
//

char
stcp_getc( struct stcp_connection *conn )
{
    char c;
    if ( NULL == conn ) {
        return 0;
    }

    if ( stcp_read_inner( conn, &c, 1, 0 ) <= 0 ) {
        return (char) 0;
    }

    return c;

}

////////////////////////////////////////////////////////////////////////////////
// stcp_read
//

int
stcp_read( struct stcp_connection *conn, void *buf, size_t len, int mstimeout )
{
    if ( len > INT_MAX ) {
        len = INT_MAX;
    }

    if ( ( conn == NULL ) || ( NULL == buf ) ) {
        return -1; 
    }
    
    memset( buf, 0, len );

    return stcp_read_inner( conn, buf, len, mstimeout );
}

////////////////////////////////////////////////////////////////////////////////
// stcp_write
//

int
stcp_write( struct stcp_connection *conn, const void *buf, size_t len )
{
    int64_t total;

    if ( ( NULL == conn ) || ( NULL == buf ) ) {
        return 0;
    }
    
    if ( 0 == len ) return 0;
    
    total = stcp_push_all( conn,
                            NULL,
                            (const char *)buf,
                            (int64_t)len);

    return (int)total;
}


// -----------------------------------------------------------------------------
//                                  S E R V E R
// -----------------------------------------------------------------------------



void
stcp_close_all_listening_sockets( struct server_context *srv_ctx )
{
    unsigned int i;
    if ( !srv_ctx ) {
        return;
    }

    for ( i = 0; i < srv_ctx->num_listening_sockets; i++ ) {
        closesocket( srv_ctx->listening_sockets[i].sock );
        srv_ctx->listening_sockets[i].sock = INVALID_SOCKET;
    }
    
    free( srv_ctx->listening_sockets );
    srv_ctx->listening_sockets = NULL;
    free( srv_ctx->listening_socket_fds );
    srv_ctx->listening_socket_fds = NULL;
}


////////////////////////////////////////////////////////////////////////////////
// next_option
//
// A helper function for traversing a comma separated list of values.
// It returns a list pointer shifted to the next value, or NULL if the end
// of the list found.
// Value is stored in msg vector. If value has form "x=y", then eq_val
// vector is initialized to point to the "y" part, and val vector length
// is adjusted to point only to "x".
//

static const char *
next_option( const char *list, struct msg *val, struct msg *eq_val )
{
    int end;

reparse:

    if (val == NULL || list == NULL || *list == '\0') {
        // End of the list
        return NULL;
    }

    // Skip over leading LWS
    while (*list == ' ' || *list == '\t') {
	list++;
    }

    val->ptr = list;
    if ( ( list = strchr(val->ptr, ',') ) != NULL ) {
	// Comma found. Store length and shift the list ptr
	val->len = ((size_t)(list - val->ptr));
	list++;
    }
    else {
        // This value is the last one
	list = val->ptr + strlen(val->ptr);
	val->len = ((size_t)(list - val->ptr));

        // Adjust length for trailing LWS
	end = (int)val->len - 1;
	while ( ( end >= 0 ) &&
                ( ( val->ptr[end] == ' ' ) ||
                  ( val->ptr[end] == '\t') ) )
            end--;
	val->len = (size_t)(end + 1);

        if ( 0 == val->len ) {
            // Ignore any empty entries.
            goto reparse;
	}

        if ( eq_val != NULL ) {
            // Value has form "x=y", adjust pointers and lengths
            // so that val points to "x", and eq_val points to "y".
            eq_val->len = 0;
            eq_val->ptr = (const char *)memchr(val->ptr, '=', val->len);
            if ( eq_val->ptr != NULL ) {
                eq_val->ptr++; // Skip over '=' character
                eq_val->len = ((size_t)(val->ptr - eq_val->ptr)) + val->len;
		val->len = ((size_t)(eq_val->ptr - val->ptr)) - 1;
            }

        }

    }

    return list;
}

////////////////////////////////////////////////////////////////////////////////
// parse_port_string
//
// Valid listening port specification is: [ip_address:]port[s]
// Examples for IPv4: 80, 443s, 127.0.0.1:3128, 192.0.2.3:8080s
// Examples for IPv6: [::]:80, [::1]:80,
//   [2001:0db8:7654:3210:FEDC:BA98:7654:3210]:443s
//   see https://tools.ietf.org/html/rfc3513#section-2.2
// In order to bind to both, IPv4 and IPv6, you can either add
// both ports using 8080,[::]:8080, or the short form +8080.
// Both forms differ in detail: 8080,[::]:8080 create two sockets,
// one only accepting IPv4 the other only IPv6. +8080 creates
// one socket accepting IPv4 and IPv6. Depending on the IPv6
// environment, they might work differently, or might not work
// at all - it must be tested what options work best in the
// relevant network environment.
//
// msg          - [in] A message chunk
// so           - [out] IP Address
// ip_version   - [out] Version of IP-address (4/6/0==failure)
// 
// Returns: 0 = failure. 1 == success

static int
parse_port_string( const struct msg *msg, 
                    struct socket *so, 
                    int *ip_version )
{
    unsigned int a, b, c, d, port;
    int ch, len;
    const char *cb;
    char buf[100] = {0};

    // MacOS needs that. If we do not zero it, subsequent bind() will fail.
    // Also, all-zeroes in the socket address means binding to all addresses
    // for both IPv4 and IPv6 (INADDR_ANY and IN6ADDR_ANY_INIT).
    memset( so, 0, sizeof (*so) );
    so->lsa.sin.sin_family = AF_INET;
    *ip_version = 0;

    // Initialize port and len as invalid.
    port = 0;
    len = 0;

    // Test for different ways to format this string
    if ( 5 == sscanf( msg->ptr, 
                        "%u.%u.%u.%u:%u%n", 
                        &a, &b, &c, &d, &port, &len ) ) {

        // Bind to a specific IPv4 address, e.g. 192.168.1.5:8080
        so->lsa.sin.sin_addr.s_addr = htonl((a << 24) | (b << 16) | (c << 8) | d );
        so->lsa.sin.sin_port = htons((uint16_t) port);
        *ip_version = 4;

    }
    else if ( ( 2 == sscanf( msg->ptr, "[%49[^]]]:%u%n", buf, &port, &len ) ) &&
                stcp_inet_pton( AF_INET6, buf, 
                                    &so->lsa.sin6, 
                                    sizeof(so->lsa.sin6) ) ) {

        // IPv6 address, examples: see above
        // so->lsa.sin6.sin6_family = AF_INET6; already set by web_inet_pton
        so->lsa.sin6.sin6_port = htons((uint16_t) port);
        *ip_version = 6;

    }
    else if ( ( msg->ptr[0] == '+') &&
                ( 1 == sscanf(msg->ptr + 1, "%u%n", &port, &len ) ) ) {

        // Port is specified with a +, bind to IPv6 and IPv4, INADDR_ANY
        // Add 1 to len for the + character we skipped before
        len++;

        // Set socket family to IPv6, do not use IPV6_V6ONLY
        so->lsa.sin6.sin6_family = AF_INET6;
        so->lsa.sin6.sin6_port = htons((uint16_t) port);
        *ip_version = 4 + 6;

    }
    else if ( sscanf(msg->ptr, "%u%n", &port, &len ) == 1 ) {
        // If only port is specified, bind to IPv4, INADDR_ANY
        so->lsa.sin.sin_port = htons( (uint16_t) port );
        *ip_version = 4;
    }
    else if ( (cb = strchr(msg->ptr, ':')) != NULL ) {
        // Could be a hostname
        // Will only work for RFC 952 compliant hostnames,
        // starting with a letter, containing only letters,
        // digits and hyphen ('-'). Newer specs may allow
        // more, but this is not guaranteed here, since it
        // may interfere with rules for port option lists.

        *(char *)cb = 0;   // Use a const cast here and modify the string.
		                    // We are going to restore the string later.

        if ( stcp_inet_pton( AF_INET,
                                msg->ptr,
                                &so->lsa.sin,
                                sizeof (so->lsa.sin) ) ) {
            if ( 1 == sscanf(cb + 1, "%u%n", &port, &len) ) {
                *ip_version = 4;
                so->lsa.sin.sin_family = AF_INET;
                so->lsa.sin.sin_port = htons((uint16_t) port);
                len += (int) (cb - msg->ptr) + 1;
            }
            else {
                port = 0;
                len = 0;
            }

        }
        else if ( stcp_inet_pton( AF_INET6,
                                    msg->ptr,
                                    &so->lsa.sin6,
                                    sizeof( so->lsa.sin6 ) ) ) {
            if ( 1 == sscanf( cb + 1, "%u%n", &port, &len ) ) {
                *ip_version = 6;
                so->lsa.sin6.sin6_family = AF_INET6;
                so->lsa.sin.sin_port = htons( (uint16_t)port );
                len += (int)(cb - msg->ptr) + 1;
            }
            else {
                port = 0;
                len = 0;
            }

        }

        *(char *)cb = ':'; // restore the string

    }
    else {
        // Parsing failure.  
    }

    // sscanf and the option splitting code ensure the following condition
    if ( (len < 0) && ((unsigned) len > (unsigned) msg->len)) {
        *ip_version = 0;
        return 0;
    }

    ch = msg->ptr[len]; /* Next character after the port number */
    so->is_ssl = (ch == 's');
    //so->ssl_redir = (ch == 'r');

    // Make sure the port is valid and vector ends with 's', 'r' or ','
    if ( is_valid_port( port ) &&
            ( (ch == '\0') || (ch == 's') || (ch == 'r') || (ch == ',') ) ) {
        return 1;
    }

    // Reset ip_version to 0 of there is an error
    *ip_version = 0;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// init_listening_port
//

int
stcp_listening( struct server_context *srv_ctx, 
                    const char *str_listening_port )
{
    const char *list;
    int on = 1;
    int off = 0;
    struct msg msg;
    struct socket so, *ptr;

    struct pollfd *pfd;
    union usa usa;
    socklen_t len;
    int ip_version;
    
    int portsTotal = 0;
    int portsOk = 0;

    /* Check pointers */
    if ( ( NULL == srv_ctx ) || 
         ( NULL == str_listening_port ) ) {
        return 0;
    }

    /* Init. defaults */

    memset(&so, 0, sizeof(so));
    memset( &usa, 0, sizeof( usa ) );
    len = sizeof( usa );
    list = str_listening_port;

    msg.ptr = str_listening_port;
    msg.len = strlen( str_listening_port );
    
    while ( ( list = next_option( list, &msg, NULL) ) != NULL ) {

        portsTotal++;

        if ( !parse_port_string( &msg, &so, &ip_version ) ) {
            stcp_report_error( "%.*s: invalid port spec. Expecting list of: %s",
                                (int) msg.len,
                                msg.ptr,
                                "[IP_ADDRESS:]PORT[s]");
            return 0;
        }

        if  ( so.is_ssl && ( NULL == srv_ctx->ssl_ctx ) ) {
            stcp_report_error( "Cannot add SSL socket. Is -ssl_certificate "
                               "option set?" );
            return 0;
        }

        if ( INVALID_SOCKET == 
                 ( so.sock = socket( so.lsa.sa.sa_family, SOCK_STREAM, 6 ) ) ) {
            stcp_report_error( "cannot create socket" );
            return 0;
        }

#ifdef _WIN32
        
        // Windows SO_REUSEADDR lets many procs binds to a
        // socket, SO_EXCLUSIVEADDRUSE makes the bind fail
        // if someone already has the socket -- DTL */
        // NOTE: If SO_EXCLUSIVEADDRUSE is used,
        // Windows might need a few seconds before
        // the same port can be used again in the
        // same process, so a short Sleep may be
        // required between web_stop and web_start.
        //
        if ( setsockopt(so.sock,
                        SOL_SOCKET,
                        SO_EXCLUSIVEADDRUSE,
                        (SOCK_OPT_TYPE) & on,
                        sizeof(on) ) != 0) {

            // Set reuse option, but don't abort on errors.
            stcp_report_error( "cannot set socket option SO_EXCLUSIVEADDRUSE" );
        }
#else
        if ( setsockopt( so.sock,
                            SOL_SOCKET,
                            SO_REUSEADDR,
                            ( (SOCK_OPT_TYPE)&on ),
                            sizeof( on ) ) != 0 ) {

            // Set reuse option, but don't abort on errors.
            stcp_report_error( "cannot set socket option SO_REUSEADDR (entry %i)" );
        }
#endif

        if ( ip_version > 4 ) {  /* Could be 6 for IPv6 only or 10 (4+6) for IPv4+IPv6 */
            
            if ( ip_version > 6 ) {
                
                if ( ( AF_INET6 == so.lsa.sa.sa_family )  && 
                        setsockopt( so.sock,
                                        IPPROTO_IPV6,
				        IPV6_V6ONLY,
				        (void *)&off,
				        sizeof( off ) ) != 0 ) {

                        /* Set IPv6 only option, but don't abort on errors. */
			        stcp_report_error( "cannot set socket option IPV6_V6ONLY=off (entry %i)",
					                    portsTotal );
                }
            }
            else {

                if ( ( AF_INET6 == so.lsa.sa.sa_family ) &&
                    ( setsockopt( so.sock,
                                    IPPROTO_IPV6,
                                    IPV6_V6ONLY,
                                    (void *)&off,
                                    sizeof( off ) ) != 0 ) ) {

                    // Set IPv6 only option, but don't abort on errors.
                    stcp_report_error( "cannot set socket option IPV6_V6ONLY" );
                }
                
            }

        }
        
        set_non_blocking_mode( so.sock );

        if ( so.lsa.sa.sa_family == AF_INET ) {

            len = sizeof( so.lsa.sin );
            if ( bind( so.sock, 
                        &(so.lsa.sa), 
                        len ) != 0) {

                stcp_report_error( "cannot bind to %.*s: %d (%s)",
                                    (int)msg.len,
                                    msg.ptr,
                                    (int)ERRNO,
                                    strerror( errno ) );
                closesocket( so.sock );
                so.sock = INVALID_SOCKET;
                return 0;
            }
        }
        else if ( so.lsa.sa.sa_family == AF_INET6 ) {

            len = sizeof( so.lsa.sin6 );
            if ( bind( so.sock, 
                        &(so.lsa.sa), 
                        len ) != 0 ) {
                stcp_report_error( "cannot bind to IPv6 %.*s: %d (%s)",
                                    (int)msg.len,
                                    msg.ptr,
                                    (int)ERRNO,
                                    strerror( errno ) );
                closesocket( so.sock );
                so.sock = INVALID_SOCKET;
                return 0;
            }
        }
        else {
            stcp_report_error( "cannot bind: address family not supported (entry %i)" );
            closesocket( so.sock );
            so.sock = INVALID_SOCKET;
            return 0;
        }

        if ( listen( so.sock, SOMAXCONN ) != 0 ) {

            stcp_report_error( "cannot listen to %.*s: %d (%s)",
                                (int)msg.len,
                                msg.ptr,
                                (int)ERRNO,
                                strerror( errno ) );
            closesocket( so.sock );
            so.sock = INVALID_SOCKET;
            return 0;
        }

        if ( ( getsockname( so.sock, &(usa.sa), &len ) != 0 ) ||
             ( usa.sa.sa_family != so.lsa.sa.sa_family ) ) {

            int err = (int)ERRNO;
            stcp_report_error( "call to getsockname failed %.*s: %d (%s)",
                                (int)msg.len,
                                msg.ptr,
                                err,
                                strerror( errno ) );
            closesocket( so.sock );
            so.sock = INVALID_SOCKET;
            return 0;
        }

        /* Update lsa port in case of random free ports */
        if ( AF_INET6 == so.lsa.sa.sa_family ) {
            so.lsa.sin6.sin6_port = usa.sin6.sin6_port;
        }
        else {
            so.lsa.sin.sin_port = usa.sin.sin_port;
        }
        
        if ( NULL == ( ptr = 
                (struct socket *)realloc( srv_ctx->listening_sockets,
                                            ( srv_ctx->num_listening_sockets + 1 ) *
                                            sizeof( srv_ctx->listening_sockets[0] ) ) ) ) {

            stcp_report_error( "Out of memory" );
            closesocket( so.sock );
            so.sock = INVALID_SOCKET;
            continue;
        }

        if ( NULL == ( pfd = 
                (struct pollfd *)realloc( srv_ctx->listening_socket_fds,
                                            ( srv_ctx->num_listening_sockets + 1 ) *
		                                    sizeof( srv_ctx->listening_socket_fds[0] ) ) ) ) {

            stcp_report_error( "Out of memory" );
            closesocket( so.sock );
            so.sock = INVALID_SOCKET;
            free(ptr);
            continue;
        }
    
        set_close_on_exec( so.sock );
        srv_ctx->listening_sockets = ptr;
        srv_ctx->listening_sockets[ srv_ctx->num_listening_sockets ] = so;
        srv_ctx->listening_socket_fds = pfd;
        srv_ctx->num_listening_sockets++;
        portsOk++;
        
    } // while
    
    if ( portsOk != portsTotal ) {
        stcp_close_all_listening_sockets( srv_ctx );
        portsOk = 0;
    }
    
    return portsOk;
}


////////////////////////////////////////////////////////////////////////////////
// accept_new_connection
//

int
stcp_accept( struct server_context *srv_ctx, 
                const struct socket *listener, 
                struct socket *psocket )
{
    //char src_addr[ IP_ADDR_STR_LEN ];
    socklen_t len = sizeof( psocket->rsa );
    int on = 1;

    /* Check pointers and listening socklet */
    if ( ( NULL == listener ) || ( NULL == psocket ) || !listener->sock ) {
        return 0;
    }

    if ( INVALID_SOCKET == 
       ( psocket->sock = accept( listener->sock, &(psocket->rsa.sa), &len ) ) ) {
        stcp_report_error( "accept() failed: %s",
                            strerror( ERRNO ) ) ;
        return 0;                                    
    }
    /*else if ( !check_acl( ctx, ntohl( *(uint32_t *)&psocket->rsa.sin.sin_addr ) ) ) {
        sockaddr_to_string(src_addr, sizeof (src_addr), &psocket->rsa);
        //web_cry(fc(ctx), "%s: %s is not allowed to connect", __func__, src_addr );
        closesocket( psocket->sock );
    }*/
    else {

        // Put so socket structure into the queue
#if defined(_WIN32)
        (void)SetHandleInformation((HANDLE)(intptr_t)psocket->sock, HANDLE_FLAG_INHERIT, 0);
#else
        if ( fcntl( psocket->sock, F_SETFD, FD_CLOEXEC ) != 0 ) {
            // Failed TODO
        }
#endif
        
        // If listner is ssl this is to
        psocket->is_ssl = listener->is_ssl;

        if ( getsockname( psocket->sock, &psocket->lsa.sa, &len ) != 0 ) {
            stcp_report_error( "getsockname() failed: %s",
                                strerror( ERRNO ) ) ;
        }

        // Set TCP keep-alive. This is needed because if HTTP-level
        // keep-alive is enabled, and client resets the connection, server 
        // won't get TCP FIN or RST and will keep the connection open 
        // forever. With TCP keep-alive, next keep-alive handshake will 
        // figure out that the client is down and will close the server end.
        // Thanks to Igor Klopov who suggested the patch.
        if ( setsockopt( psocket->sock,
                            SOL_SOCKET,
                            SO_KEEPALIVE,
                            (SOCK_OPT_TYPE)&on,
                            sizeof( on ) ) != 0 ) {
            stcp_report_error( "setsockopt(SOL_SOCKET SO_KEEPALIVE) failed: %s",
                                strerror( ERRNO ) );
        }

        // Disable TCP Nagle's algorithm. Normally TCP packets are coalesced
        // to effectively fill up the underlying IP packet payload and
        // reduce the overhead of sending lots of small buffers. However
        // this hurts the server's throughput (ie. operations per second)
        // when HTTP 1.1 persistent connections are used and the responses
        // are relatively small (eg. less than 1400 bytes).
        //
        if ( ( NULL != srv_ctx ) &&  srv_ctx->config_tcp_nodelay ) {
            if ( set_tcp_nodelay( psocket->sock, 1 ) != 0 ) {
                stcp_report_error( "setsockopt(IPPROTO_TCP TCP_NODELAY) failed: %s",
                                    strerror( ERRNO ) );
            }
        }

        // We are using non-blocking sockets. Thus, the
        // set_sock_timeout(so.sock, timeout);
        // call is no longer required.

        // The "non blocking" property should already be
        // inherited from the parent socket. Set it for
	    // non-compliant socket implementations. */
	    set_non_blocking_mode( psocket->sock );

    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// stcp_init_client_connection
//

void stcp_init_client_connection( struct stcp_connection *conn,
                                    struct stcp_secure_options *secure_opts )
{
    // Check conn pointer
    if ( NULL == conn ) {
        return;
    }

    // If secure then secure options must be set
    if ( conn->client.is_ssl && ( NULL == secure_opts ) ) {
        return;
    }

    conn->conn_state = STCP_CONN_STATE_CONNECTED;

    conn->birth = time( NULL );

    // Fill in IP, port info early so even if SSL setup below fails,
    // error handler would have the corresponding info.
    // Thanks to Johannes Winkelmann for the patch.
    if ( AF_INET6 == conn->client.rsa.sa.sa_family ) {
        conn->remote_port = ntohs( conn->client.rsa.sin6.sin6_port );
    }
    else {
        conn->remote_port = ntohs( conn->client.rsa.sin.sin_port );
    }

    sockaddr_to_string( conn->remote_addr,
                            sizeof( conn->remote_addr ),
                            &conn->client.rsa );

    if ( conn->client.is_ssl ) {

        // Secure connection
        if ( make_ssl( conn,
                        secure_opts,
                        conn->ssl_ctx,
                        SSL_accept,
                        &(conn->stop_flag ) ) ) {

            // Get SSL client certificate information (if set)
            ssl_get_client_cert_info( conn, secure_opts->srv_client_cert );

        }

    }

}