///////////////////////////////////////////////////////////////////////////////
// vscphelperlogging-internal.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Åke Hedman and contributors, The VSCP Project,
// <akhe@vscp.org>
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

/**
 * @file vscphelperlogging-internal.h
 * @brief Internal declarations for sharing the helper logger backend.
 *
 * This header is for VSCP library implementation files, not application use.
 * The exported shared functions let the public logging API forwarders in
 * vscp_util and vscp_core use the single backend instance in vscp_util.
 */

#pragma once

#include "vscphelperlogging.h"

/**
 * @brief Controls the Windows DLL visibility of the private backend symbols.
 *
 * The backend library defines VSCP_HELPER_LOGGING_BACKEND_EXPORTS when
 * building the functions; other Windows libraries import them.
 */
#if defined(_WIN32) && defined(VSCP_HELPER_LOGGING_BACKEND_EXPORTS)
#define VSCP_HELPER_LOGGING_BACKEND_API __declspec(dllexport)
#elif defined(_WIN32)
#define VSCP_HELPER_LOGGING_BACKEND_API __declspec(dllimport)
#elif defined(__GNUC__)
#define VSCP_HELPER_LOGGING_BACKEND_API __attribute__((visibility("default")))
#else
#define VSCP_HELPER_LOGGING_BACKEND_API
#endif

extern "C" {

/**
 * @brief Configure the shared logger.
 * @param consoleLevel Console sink threshold.
 * @param fileLevel Rotating-file sink threshold.
 * @param filePath File path when file logging is enabled.
 * @param maxFileSize Maximum active log file size in bytes.
 * @param maxFiles Maximum number of rotated files.
 * @return A VSCP error code, matching the public configuration API.
 */
VSCP_HELPER_LOGGING_BACKEND_API int
vscphelper_shared_configureLogging(int consoleLevel, int fileLevel, const char* filePath,
                                   size_t maxFileSize, size_t maxFiles) noexcept;
/** Disable logging in the shared backend. */
VSCP_HELPER_LOGGING_BACKEND_API int vscphelper_shared_disableLogging(void) noexcept;
/** Flush all configured backend sinks. */
VSCP_HELPER_LOGGING_BACKEND_API int vscphelper_shared_flushLogging(void) noexcept;
/**
 * @brief Return the calling thread's most recent backend logging error.
 * @return Error text, or an empty string when no error is recorded.
 */
VSCP_HELPER_LOGGING_BACKEND_API const char* vscphelper_shared_getLastLoggingError(void) noexcept;
/**
 * @brief Route a printf-style TCP transport diagnostic to the backend logger.
 * @param format printf-style message format.
 * @param arguments Arguments for @p format.
 */
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logTransportError(const char* format, va_list arguments) noexcept;
/**
 * @brief Check whether the shared logger accepts a spdlog-compatible level.
 * @param level Logging level.
 * @return Nonzero if the level is enabled; otherwise zero.
 */
VSCP_HELPER_LOGGING_BACKEND_API int vscphelper_shared_isLoggingEnabled(int level) noexcept;
/**
 * @brief Emit a message buffer at a spdlog-compatible level.
 * @param level Logging level.
 * @param message Message data; it need not be null-terminated.
 * @param length Number of bytes in @p message.
 */
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logMessage(int level, const char* message, size_t length) noexcept;
/** Report an internal logging failure to the backend's fallback diagnostic. */
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_reportLoggingFailure(const char* message) noexcept;
/**
 * @brief Emit a debug trace for entering a VSCP function.
 * @param component Component label.
 * @param function Function name.
 */
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logFunctionEntry(const char* component, const char* function) noexcept;
/**
 * @brief Emit a debug trace containing a function's numeric status.
 * @param component Component label.
 * @param function Function name.
 * @param status Status code.
 */
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logFunctionStatus(const char* component, const char* function, int status) noexcept;
/**
 * @brief Emit an error diagnostic for a failed VSCP function.
 * @param component Component label.
 * @param function Function name.
 * @param errorCode Error code, or zero when no code is available.
 */
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logFunctionFailure(const char* component, const char* function, int errorCode) noexcept;

}
