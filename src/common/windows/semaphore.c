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


/**
    @file semaphore.c
    @brief Implementation Code of Semaphore Routines
*/

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT			0x0501 /* Windows XP: only XP-era APIs are used. */
#endif

#include "semaphore.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifndef SEMAPHORE_QUERY_STATE
#define SEMAPHORE_QUERY_STATE	0x0001
#endif

/* Access rights requested when opening an existing named semaphore. */
#define SEM_OPEN_ACCESS			(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE | SEMAPHORE_QUERY_STATE)

/* Namespace used for names that do not specify one. Global objects are
   visible to every process on the machine, matching POSIX named semaphores. */
#define SEM_DEFAULT_NAMESPACE	L"Global\\"

static int lc_set_errno(int result) {
	if (result != 0) {
		errno = result;
		return -1;
		}

	return 0;
	}

static HANDLE lc_handle(sem_t *sem) {
	return (sem == NULL) ? NULL : (HANDLE) sem->handle;
	}

/**
    Create an unnamed semaphore.
    @param sem The pointer of the semaphore object.
    @param pshared Must be 0 (PTHREAD_PROCESS_PRIVATE). A Win32 HANDLE is only
          valid inside the process that created it, so an unnamed semaphore
          cannot be shared between processes; use sem_open() instead.
    @param value The value argument specifies the initial value for
          the semaphore.
    @return If the function succeeds, the return value is 0.
           If the function fails, the return value is -1,
           with errno set to indicate the error (ENOSYS if pshared is non-zero).
*/
int sem_init(sem_t *sem, int pshared, unsigned int value) {
	HANDLE handle;

	if (sem == NULL || value > (unsigned int) SEM_VALUE_MAX) {
		return lc_set_errno(EINVAL);
		}

	if (pshared != 0) {
		return lc_set_errno(ENOSYS);
		}

	if ((handle = CreateSemaphoreW(NULL, (LONG) value, SEM_VALUE_MAX, NULL)) == NULL) {
		return lc_set_errno(ENOSPC);
		}

	sem->handle = handle;
	return 0;
	}

/**
    Acquire a semaphore.
    @param sem The pointer of the semaphore object.
    @return If the function succeeds, the return value is 0.
           If the function fails, the return value is -1,
           with errno set to indicate the error.
*/
int sem_wait(sem_t *sem) {
	HANDLE handle = lc_handle(sem);

	if (handle == NULL) {
		return lc_set_errno(EINVAL);
		}

	if (WaitForSingleObject(handle, INFINITE) != WAIT_OBJECT_0) {
		return lc_set_errno(EINVAL);
		}

	return 0;
	}

/**
    Try acquire a semaphore.
    @param sem The pointer of the semaphore object.
    @return If the function succeeds, the return value is 0.
           If the function fails, the return value is -1,
           with errno set to indicate the error.
*/
int sem_trywait(sem_t *sem) {
	DWORD rc;
	HANDLE handle = lc_handle(sem);

	if (handle == NULL) {
		return lc_set_errno(EINVAL);
		}

	if ((rc = WaitForSingleObject(handle, 0)) == WAIT_OBJECT_0) {
		return 0;
		}

	if (rc == WAIT_TIMEOUT) {
		return lc_set_errno(EAGAIN);
		}

	return lc_set_errno(EINVAL);
	}

/* Time conversion functions */

/*  Number of 100 ns intervals between the beginning of the Windows epoch
    (1 Jan 1601) and the Unix epoch (1 Jan 1970)
*/
#define DELTA_EPOCH_IN_100NS	INT64_C(116444736000000000)
#define HNS_PER_SEC				INT64_C(10000000)
#define HNS_PER_MS				INT64_C(10000)
#define NS_PER_HNS				100L
#define NS_PER_SEC				1000000000L

/* Return 100 ns intervals since the Unix epoch (1 Jan 1970) UTC */
static int64_t lc_now_in_100ns(void) {
	FILETIME ft;
	ULARGE_INTEGER t;

	GetSystemTimeAsFileTime(&ft);
	t.LowPart = ft.dwLowDateTime;
	t.HighPart = ft.dwHighDateTime;
	return (int64_t) t.QuadPart - DELTA_EPOCH_IN_100NS;
	}

/* Convert an absolute timespec to 100 ns intervals since the Unix epoch,
   rounding up so a wait never ends before the requested time and saturating
   instead of overflowing for far-future times. */
static int64_t lc_timespec_to_100ns(const struct timespec *ts) {
	/* Copy first: time_t is 32 bits on some CRTs, where comparing it with
	   the limit below directly is always false and warns. */
	int64_t sec = (int64_t) ts->tv_sec;

	if (sec < 0) {
		return 0;
		}

	if (sec >= INT64_MAX / HNS_PER_SEC - 1) {
		return INT64_MAX;
		}

	return sec * HNS_PER_SEC + (ts->tv_nsec + NS_PER_HNS - 1) / NS_PER_HNS;
	}

/**
    Try acquire a semaphore.
    @param sem The pointer of the semaphore object.
    @param abs_timeout The pointer of the structure that specifies an
          absolute timeout in seconds and nanoseconds since the Epoch,
          1970-01-01 00:00:00 +0000 (UTC).
    @return If the function succeeds, the return value is 0.
           If the function fails, the return value is -1,
           with errno set to indicate the error.
*/
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout) {
	DWORD rc;
	int64_t deadline, remaining;
	HANDLE handle = lc_handle(sem);

	if (handle == NULL) {
		return lc_set_errno(EINVAL);
		}

	/* As in POSIX, the timeout is not examined if the semaphore can be taken immediately. */
	if ((rc = WaitForSingleObject(handle, 0)) == WAIT_OBJECT_0) {
		return 0;
		}

	if (rc != WAIT_TIMEOUT) {
		return lc_set_errno(EINVAL);
		}

	if (abs_timeout == NULL || abs_timeout->tv_nsec < 0 || abs_timeout->tv_nsec >= NS_PER_SEC) {
		return lc_set_errno(EINVAL);
		}

	deadline = lc_timespec_to_100ns(abs_timeout);

	/* Windows waits are relative and in whole milliseconds, and the system
	   clock can be adjusted, so keep waiting until the clock has really
	   passed the deadline. This also handles waits longer than ~49 days. */
	while ((remaining = deadline - lc_now_in_100ns()) > 0) {
		int64_t ms = (remaining + HNS_PER_MS - 1) / HNS_PER_MS;
		DWORD wait_ms = (ms >= (int64_t) INFINITE) ? INFINITE - 1 : (DWORD) ms;

		if ((rc = WaitForSingleObject(handle, wait_ms)) == WAIT_OBJECT_0) {
			return 0;
			}

		if (rc != WAIT_TIMEOUT) {
			return lc_set_errno(EINVAL);
			}
		}

	return lc_set_errno(ETIMEDOUT);
	}

/**
    Release a semaphore.
    @param sem The pointer of the semaphore object.
    @return If the function succeeds, the return value is 0.
           If the function fails, the return value is -1,
           with errno set to indicate the error (EOVERFLOW if the value
           would exceed SEM_VALUE_MAX).
*/
int sem_post(sem_t *sem) {
	HANDLE handle = lc_handle(sem);

	if (handle == NULL) {
		return lc_set_errno(EINVAL);
		}

	if (ReleaseSemaphore(handle, 1, NULL) == 0) {
		return lc_set_errno(GetLastError() == ERROR_TOO_MANY_POSTS ? EOVERFLOW : EINVAL);
		}

	return 0;
	}

/* NtQuerySemaphore reads the count without changing it. It is exported by
   ntdll.dll on every Windows NT release but is not in the SDK headers, so
   it is looked up at run time. */
typedef struct {
	LONG CurrentCount;
	LONG MaximumCount;
	} lc_semaphore_basic_information;

typedef LONG (NTAPI *lc_nt_query_semaphore_fn)(HANDLE, int, PVOID, ULONG, PULONG);

/* Looked up on each call rather than cached: one-time initialisation
   (INIT_ONCE) needs Windows Vista, and sem_getvalue is not performance
   critical. */
static lc_nt_query_semaphore_fn lc_nt_query_semaphore(void) {
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");

	if (ntdll == NULL) {
		return NULL;
		}

	return (lc_nt_query_semaphore_fn) (void (*)(void)) GetProcAddress(ntdll, "NtQuerySemaphore");
	}

/**
    Get the value of a semaphore.
    @param sem The pointer of the semaphore object.
    @param value The pointer of the current value of the semaphore.
    @return If the function succeeds, the return value is 0.
           If the function fails, the return value is -1,
           with errno set to indicate the error.
    @remark The value is never negative; the number of waiters is not reported.
*/
int sem_getvalue(sem_t *sem, int *value) {
	LONG previous;
	lc_semaphore_basic_information info;
	lc_nt_query_semaphore_fn query;
	HANDLE handle = lc_handle(sem);

	if (handle == NULL || value == NULL) {
		return lc_set_errno(EINVAL);
		}

	if ((query = lc_nt_query_semaphore()) != NULL &&
		query(handle, 0 /* SemaphoreBasicInformation */, &info, (ULONG) sizeof(info), NULL) >= 0) {
		*value = (int) info.CurrentCount;
		return 0;
		}

	/* Fallback: briefly take one unit and give it back. Other threads may
	   observe the value drop by one while this happens. */
	switch (WaitForSingleObject(handle, 0)) {
		case WAIT_OBJECT_0:
			if (!ReleaseSemaphore(handle, 1, &previous)) {
				return lc_set_errno(EINVAL);
				}

			*value = (int) previous + 1;
			return 0;

		case WAIT_TIMEOUT:
			*value = 0;
			return 0;

		default:
			return lc_set_errno(EINVAL);
		}
	}

/**
    Destroy a semaphore.
    @param sem The pointer of the semaphore object.
    @return If the function succeeds, the return value is 0.
           If the function fails, the return value is -1,
           with errno set to indicate the error.
*/
int sem_destroy(sem_t *sem) {
	HANDLE handle = lc_handle(sem);

	if (handle == NULL) {
		return lc_set_errno(EINVAL);
		}

	if (CloseHandle(handle) == 0) {
		return lc_set_errno(EINVAL);
		}

	sem->handle = NULL;
	return 0;
	}

/* Convert a POSIX semaphore name (UTF-8, optionally starting with '/') to a
   Windows kernel object name. Names that already start with "Global\" or
   "Local\" are kept in that namespace; all others go in the Global one.
   Returns 0 or an errno value. */
static int lc_object_name(const char *name, wchar_t *out, int out_len) {
	static const wchar_t prefix[] = SEM_DEFAULT_NAMESPACE;
	const char *base;
	int prefix_len = 0;

	if (name == NULL) {
		return EINVAL;
		}

	if (*name == '/') {
		name++;
		}

	if (strncmp(name, "Global\\", 7) == 0) {
		base = name + 7;
		}

	else if (strncmp(name, "Local\\", 6) == 0) {
		base = name + 6;
		}

	else {
		base = name;
		prefix_len = (int) (sizeof(prefix) / sizeof(prefix[0])) - 1;
		memcpy(out, prefix, sizeof(prefix) - sizeof(prefix[0]));
		}

	if (*base == '\0' || strchr(base, '\\') != NULL) {
		return EINVAL;
		}

	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name, -1, out + prefix_len, out_len - prefix_len) == 0) {
		return (GetLastError() == ERROR_INSUFFICIENT_BUFFER) ? ENAMETOOLONG : EINVAL;
		}

	return 0;
	}

static int lc_open_error(DWORD error) {
	switch (error) {
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
			return ENOENT;

		case ERROR_ACCESS_DENIED:
			return EACCES;

		case ERROR_INVALID_HANDLE: /* The name belongs to a different kind of object. */
		case ERROR_INVALID_NAME:
		case ERROR_BAD_PATHNAME:
			return EINVAL;

		case ERROR_FILENAME_EXCED_RANGE:
			return ENAMETOOLONG;

		default:
			return ENOSPC;
		}
	}

/**
    Open a named semaphore.
    @param name The name of the semaphore object, as UTF-8. A leading '/'
          is ignored, so "/name" and "name" refer to the same semaphore.
          Names are created in the Global namespace unless they start with
          "Global\" or "Local\".
    @param oflag If O_CREAT is specified in oflag, then the semaphore is
          created if it does not already exist. If both O_CREAT and O_EXCL
          are specified in oflag, then an error is returned if a semaphore
          with the given name already exists.
    @param ... When O_CREAT is set: mode_t mode (ignored; the permissions
          that would be placed on the new semaphore) followed by
          unsigned int value (the initial value of the semaphore).
    @return On success, returns the address of the new semaphore; On error,
           returns SEM_FAILED (NULL), with errno set to indicate the error.
*/
sem_t *sem_open(const char *name, int oflag, ...) {
	int rc;
	unsigned int value;
	wchar_t object_name[MAX_PATH];
	HANDLE handle;
	sem_t *sem;

	if ((rc = lc_object_name(name, object_name, MAX_PATH)) != 0) {
		lc_set_errno(rc);
		return SEM_FAILED;
		}

	if (oflag & O_CREAT) {
		va_list ap;

		va_start(ap, oflag);
		(void) va_arg(ap, int); /* mode_t is promoted to int; permissions are not supported. */
		value = va_arg(ap, unsigned int);
		va_end(ap);

		if (value > (unsigned int) SEM_VALUE_MAX) {
			lc_set_errno(EINVAL);
			return SEM_FAILED;
			}

		handle = CreateSemaphoreW(NULL, (LONG) value, SEM_VALUE_MAX, object_name);

		if (handle != NULL && GetLastError() == ERROR_ALREADY_EXISTS && (oflag & O_EXCL)) {
			CloseHandle(handle);
			lc_set_errno(EEXIST);
			return SEM_FAILED;
			}
		}

	else {
		handle = OpenSemaphoreW(SEM_OPEN_ACCESS, FALSE, object_name);
		}

	if (handle == NULL) {
		lc_set_errno(lc_open_error(GetLastError()));
		return SEM_FAILED;
		}

	if ((sem = (sem_t *) malloc(sizeof(sem_t))) == NULL) {
		CloseHandle(handle);
		lc_set_errno(ENOMEM);
		return SEM_FAILED;
		}

	sem->handle = handle;
	return sem;
	}

/**
    Close a named semaphore.
    @param sem The pointer of the semaphore object returned by sem_open().
    @return If the function succeeds, the return value is 0.
           If the function fails, the return value is -1,
           with errno set to indicate the error.
*/
int sem_close(sem_t *sem) {
	if (sem_destroy(sem) != 0) {
		return -1;
		}

	free(sem);
	return 0;
	}

/**
    Remove a named semaphore.
    @param name The name of the semaphore object.
    @return If the function succeeds, the return value is 0.
           If the function fails, the return value is -1,
           with errno set to indicate the error (ENOENT if no semaphore
           with that name exists).
    @remark Windows destroys a semaphore when its last handle is closed and
           has no way to remove the name earlier, so this function only
           checks that the semaphore exists.
*/
int sem_unlink(const char *name) {
	int rc;
	wchar_t object_name[MAX_PATH];
	HANDLE handle;

	if ((rc = lc_object_name(name, object_name, MAX_PATH)) != 0) {
		return lc_set_errno(rc);
		}

	if ((handle = OpenSemaphoreW(SYNCHRONIZE, FALSE, object_name)) == NULL) {
		return lc_set_errno(GetLastError() == ERROR_ACCESS_DENIED ? EACCES : ENOENT);
		}

	CloseHandle(handle);
	return 0;
	}

#else

/* ISO C does not allow an empty translation unit. */
typedef int lc_semaphore_not_windows;

#endif /* _WIN32 */
