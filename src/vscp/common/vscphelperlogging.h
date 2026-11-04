///////////////////////////////////////////////////////////////////////////////
// vscphelperlogging.h
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

#pragma once

#include <stdarg.h>
#include <stddef.h>

#if defined(_WIN32) && defined(VSCP_HELPER_LOGGING_EXPORTS)
#define VSCP_HELPER_LOGGING_API __declspec(dllexport)
#elif defined(__GNUC__) && !defined(_WIN32) && !defined(VSCP_HELPER_LOGGING_PRIVATE)
#define VSCP_HELPER_LOGGING_API __attribute__((visibility("default")))
#else
#define VSCP_HELPER_LOGGING_API
#endif

#ifdef __cplusplus
#define VSCP_HELPER_LOGGING_NOEXCEPT noexcept
extern "C" {
#else
#define VSCP_HELPER_LOGGING_NOEXCEPT
#endif

enum vscphelper_log_level {
    VSCP_HELPER_LOG_LEVEL_TRACE = 0,
    VSCP_HELPER_LOG_LEVEL_DEBUG = 1,
    VSCP_HELPER_LOG_LEVEL_INFO = 2,
    VSCP_HELPER_LOG_LEVEL_WARN = 3,
    VSCP_HELPER_LOG_LEVEL_ERROR = 4,
    VSCP_HELPER_LOG_LEVEL_CRITICAL = 5,
    VSCP_HELPER_LOG_LEVEL_OFF = 6
};

/* Configuration is library-wide for this linked copy of the VSCP core.
 * The console sink writes to stderr. Before configuration, errors go to
 * stderr; a failed configuration retains the previous logger.
 * Functions return VSCP error codes. The last error is thread-local and
 * remains valid until the next configure/disable/flush call or logging
 * failure on that thread.
 */
VSCP_HELPER_LOGGING_API int
vscphelper_configureLogging(int consoleLevel,
                            int fileLevel,
                            const char* filePath,
                            size_t maxFileSize,
                            size_t maxFiles) VSCP_HELPER_LOGGING_NOEXCEPT;
VSCP_HELPER_LOGGING_API int vscphelper_disableLogging(void) VSCP_HELPER_LOGGING_NOEXCEPT;
VSCP_HELPER_LOGGING_API int vscphelper_flushLogging(void) VSCP_HELPER_LOGGING_NOEXCEPT;
VSCP_HELPER_LOGGING_API const char* vscphelper_getLastLoggingError(void) VSCP_HELPER_LOGGING_NOEXCEPT;

/* Internal emission interface used by VSCP sources. */
VSCP_HELPER_LOGGING_API void
vscphelper_logTransportError(const char* format, va_list arguments) VSCP_HELPER_LOGGING_NOEXCEPT;
VSCP_HELPER_LOGGING_API int vscphelper_isLoggingEnabled(int level) VSCP_HELPER_LOGGING_NOEXCEPT;
VSCP_HELPER_LOGGING_API void
vscphelper_logMessage(int level, const char* message, size_t length) VSCP_HELPER_LOGGING_NOEXCEPT;
VSCP_HELPER_LOGGING_API void
vscphelper_reportLoggingFailure(const char* message) VSCP_HELPER_LOGGING_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#undef VSCP_HELPER_LOGGING_NOEXCEPT

#if defined(__cplusplus) && defined(VSCP_ENABLE_HELPER_LOGGING)
#include <exception>
#include <string>
#include <utility>

#include <spdlog/common.h>

template <typename... Args>
static void vscphlp_log(spdlog::level::level_enum level,
                        spdlog::format_string_t<Args...> format,
                        Args&&... args) noexcept
{
    try {
        if (vscphelper_isLoggingEnabled(static_cast<int>(level))) {
            std::string message = spdlog::fmt_lib::format(format, std::forward<Args>(args)...);
            vscphelper_logMessage(static_cast<int>(level), message.data(), message.size());
        }
    }
    catch (const std::exception& error) {
        vscphelper_reportLoggingFailure(error.what());
    }
    catch (...) {
        vscphelper_reportLoggingFailure("Unknown exception while logging");
    }
}

#define VSCPHLP_LOG_TRACE(...) vscphlp_log(spdlog::level::trace, __VA_ARGS__)
#define VSCPHLP_LOG_DEBUG(...) vscphlp_log(spdlog::level::debug, __VA_ARGS__)
#define VSCPHLP_LOG_INFO(...) vscphlp_log(spdlog::level::info, __VA_ARGS__)
#define VSCPHLP_LOG_WARN(...) vscphlp_log(spdlog::level::warn, __VA_ARGS__)
#define VSCPHLP_LOG_ERROR(...) vscphlp_log(spdlog::level::err, __VA_ARGS__)
#define VSCPHLP_LOG_CRITICAL(...) vscphlp_log(spdlog::level::critical, __VA_ARGS__)
#endif
