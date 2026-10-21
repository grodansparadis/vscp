/*
    Copyright (c) 2011, Dongsheng Song <songdongsheng@live.cn>

    Licensed to the Apache Software Foundation (ASF) under one or more
    contributor license agreements.  See the NOTICE file distributed with
    this work for additional information regarding copyright ownership.
    The ASF licenses this file to You under the Apache License, Version 2.0
    (the "License"); you may not use this file except in compliance with
    the License.  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
	Simple Windows replacement for POSIX semaphores
	Modified by Daniel Tillett from libpthread <http://github.com/songdongsheng/libpthread>
	Copyright (c) 2015-2026, Daniel Tillett <daniel.tillett @ gmail.com>
*/

#ifndef SIMPLE_WINDOWS_POSIX_SEMAPHORE_H
#define SIMPLE_WINDOWS_POSIX_SEMAPHORE_H	1

/**
    @file semaphore.h
    @brief POSIX Semaphore Definitions and Routines
*/

#if !defined(_WIN32)

/* Not Windows: defer to the platform's own POSIX semaphores so portable code
   can include this header unconditionally. */
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC system_header
#include_next <semaphore.h>
#else
#error "This semaphore.h is a Windows replacement; use the system <semaphore.h> instead."
#endif

#else /* _WIN32 */

/**
    @defgroup sem POSIX Semaphore Definitions and Routines
    @{
*/

#include <errno.h>     /* EINVAL, ETIMEDOUT, ..., etc. */
#include <fcntl.h>     /* O_CREAT, O_EXCL */
#include <limits.h>    /* INT_MAX */
#include <sys/types.h> /* mode_t (MinGW) */
#include <time.h>      /* struct timespec */

#ifndef PTHREAD_PROCESS_SHARED
#define PTHREAD_PROCESS_PRIVATE	0
#define PTHREAD_PROCESS_SHARED	1
#endif

/* Support POSIX.1b semaphores.  */
#ifndef _POSIX_SEMAPHORES
#define _POSIX_SEMAPHORES		200809L
#endif

#ifndef SEM_VALUE_MAX
#define SEM_VALUE_MAX			INT_MAX
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT				138 /* Value used by the MSVC and MinGW-w64 CRTs. */
#endif

#ifndef EOVERFLOW
#define EOVERFLOW				132 /* Value used by the MSVC and MinGW-w64 CRTs. */
#endif

#ifndef _MODE_T_
#define _MODE_T_				1
typedef unsigned short _mode_t;

#ifndef NO_OLDNAMES
typedef _mode_t mode_t;
#endif
#endif  /* _MODE_T_ */

/* Visual Studio 2015 and later, and MinGW-w64, define struct timespec in <time.h>. */
#if defined(_MSC_VER) && _MSC_VER < 1900 && !defined(_TIMESPEC_DEFINED)
#define _TIMESPEC_DEFINED		1
struct timespec {
	time_t  tv_sec;       /* Seconds */
	long    tv_nsec;      /* Nanoseconds */
	};
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
    Semaphore object. Treat it as opaque: it wraps a Win32 semaphore HANDLE.
*/
typedef struct simple_win_sem {
	void *handle;
	} sem_t;

#ifndef SEM_FAILED
#define SEM_FAILED				((sem_t *) 0)
#endif

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
int sem_post(sem_t *sem);
int sem_getvalue(sem_t *sem, int *value);
int sem_destroy(sem_t *sem);
sem_t *sem_open(const char *name, int oflag, ... /* mode_t mode, unsigned int value */);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);

#ifdef __cplusplus
	}
#endif

/** @} */

#endif /* _WIN32 */

#endif /* SIMPLE_WINDOWS_POSIX_SEMAPHORE_H */
