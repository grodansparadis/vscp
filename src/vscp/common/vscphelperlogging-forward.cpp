///////////////////////////////////////////////////////////////////////////////
// vscphelperlogging-forward.cpp
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

#include "vscphelperlogging-internal.h"

extern "C" int
vscphelper_configureLogging(int consoleLevel, int fileLevel, const char* filePath,
                            size_t maxFileSize, size_t maxFiles) noexcept
{
    return vscphelper_shared_configureLogging(consoleLevel, fileLevel, filePath, maxFileSize, maxFiles);
}

extern "C" int vscphelper_disableLogging(void) noexcept
{
    return vscphelper_shared_disableLogging();
}

extern "C" int vscphelper_flushLogging(void) noexcept
{
    return vscphelper_shared_flushLogging();
}

extern "C" const char* vscphelper_getLastLoggingError(void) noexcept
{
    return vscphelper_shared_getLastLoggingError();
}

extern "C" void vscphelper_logTransportError(const char* format, va_list arguments) noexcept
{
    vscphelper_shared_logTransportError(format, arguments);
}

extern "C" int vscphelper_isLoggingEnabled(int level) noexcept
{
    return vscphelper_shared_isLoggingEnabled(level);
}

extern "C" void vscphelper_logMessage(int level, const char* message, size_t length) noexcept
{
    vscphelper_shared_logMessage(level, message, length);
}

extern "C" void vscphelper_reportLoggingFailure(const char* message) noexcept
{
    vscphelper_shared_reportLoggingFailure(message);
}

extern "C" void vscphelper_logFunctionEntry(const char* component, const char* function) noexcept
{
    vscphelper_shared_logFunctionEntry(component, function);
}

extern "C" void
vscphelper_logFunctionStatus(const char* component, const char* function, int status) noexcept
{
    vscphelper_shared_logFunctionStatus(component, function, status);
}

extern "C" void
vscphelper_logFunctionFailure(const char* component, const char* function, int errorCode) noexcept
{
    vscphelper_shared_logFunctionFailure(component, function, errorCode);
}
