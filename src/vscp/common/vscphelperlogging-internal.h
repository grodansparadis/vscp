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

#pragma once

#include "vscphelperlogging.h"

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

VSCP_HELPER_LOGGING_BACKEND_API int
vscphelper_shared_configureLogging(int consoleLevel, int fileLevel, const char* filePath,
                                   size_t maxFileSize, size_t maxFiles) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API int vscphelper_shared_disableLogging(void) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API int vscphelper_shared_flushLogging(void) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API const char* vscphelper_shared_getLastLoggingError(void) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logTransportError(const char* format, va_list arguments) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API int vscphelper_shared_isLoggingEnabled(int level) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logMessage(int level, const char* message, size_t length) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_reportLoggingFailure(const char* message) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logFunctionEntry(const char* component, const char* function) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logFunctionStatus(const char* component, const char* function, int status) noexcept;
VSCP_HELPER_LOGGING_BACKEND_API void
vscphelper_shared_logFunctionFailure(const char* component, const char* function, int errorCode) noexcept;

}
