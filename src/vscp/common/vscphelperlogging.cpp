///////////////////////////////////////////////////////////////////////////////
// vscphelperlogging.cpp
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

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <array>
#include <atomic>
#include <cerrno>
#include <cstdio>
#include <exception>
#include <memory>
#include <mutex>
#include <vector>

#include <vscp.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace {

static_assert(VSCP_HELPER_LOG_LEVEL_TRACE == static_cast<int>(spdlog::level::trace) &&
              VSCP_HELPER_LOG_LEVEL_DEBUG == static_cast<int>(spdlog::level::debug) &&
              VSCP_HELPER_LOG_LEVEL_INFO == static_cast<int>(spdlog::level::info) &&
              VSCP_HELPER_LOG_LEVEL_WARN == static_cast<int>(spdlog::level::warn) &&
              VSCP_HELPER_LOG_LEVEL_ERROR == static_cast<int>(spdlog::level::err) &&
              VSCP_HELPER_LOG_LEVEL_CRITICAL == static_cast<int>(spdlog::level::critical) &&
              VSCP_HELPER_LOG_LEVEL_OFF == static_cast<int>(spdlog::level::off),
              "VSCP and spdlog logging levels must match");

thread_local std::array<char, 512> loggingError{};
std::atomic<bool> debugEnabled{false};

struct SavedErrorState {
    int errorNumber = errno;
#ifdef _WIN32
    DWORD windowsError = GetLastError();
    int socketError = WSAGetLastError();
#endif

    ~SavedErrorState() noexcept
    {
#ifdef _WIN32
        WSASetLastError(socketError);
        SetLastError(windowsError);
#endif
        errno = errorNumber;
    }
};

void reportError(const char* message) noexcept
{
    std::snprintf(loggingError.data(), loggingError.size(), "%s",
                  message ? message : "Unknown logging failure");
    std::fprintf(stderr, "VSCP logging: %s\n", loggingError.data());
}

struct LoggingState {
    std::mutex mutex;
    std::shared_ptr<spdlog::logger> logger;
    bool defaultStderr = true;

    LoggingState()
    {
        auto sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
        logger = std::make_shared<spdlog::logger>("vscp-helper", sink);
        logger->set_level(spdlog::level::err);
        logger->set_pattern("%v");
        logger->set_error_handler([](const std::string& message) { reportError(message.c_str()); });
    }

    ~LoggingState()
    {
        debugEnabled.store(false, std::memory_order_release);
    }
};

LoggingState& loggingState()
{
    static LoggingState state;
    return state;
}

bool validLevel(int level)
{
    return level >= VSCP_HELPER_LOG_LEVEL_TRACE && level <= VSCP_HELPER_LOG_LEVEL_OFF;
}

std::shared_ptr<spdlog::logger> getLogger() noexcept
{
    try {
        auto& state = loggingState();
        std::lock_guard<std::mutex> lock(state.mutex);
        return state.logger;
    }
    catch (const std::exception& error) {
        reportError(error.what());
    }
    catch (...) {
        reportError("Unknown exception while accessing the logger");
    }
    return {};
}

} // namespace

extern "C" int
vscphelper_shared_configureLogging(int consoleLevel,
                                   int fileLevel,
                                   const char* filePath,
                                   size_t maxFileSize,
                                   size_t maxFiles) noexcept
{
    loggingError[0] = '\0';
    if (!validLevel(consoleLevel) || !validLevel(fileLevel) ||
        (fileLevel != VSCP_HELPER_LOG_LEVEL_OFF &&
         (!filePath || !*filePath || maxFileSize == 0 || maxFiles == 0 || maxFiles > 200000))) {
        reportError("Invalid logging level, file path, or rotation limits");
        return VSCP_ERROR_PARAMETER;
    }
    if (consoleLevel == VSCP_HELPER_LOG_LEVEL_OFF && fileLevel == VSCP_HELPER_LOG_LEVEL_OFF) {
        return vscphelper_shared_disableLogging();
    }

    try {
        auto& state = loggingState();
        std::lock_guard<std::mutex> lock(state.mutex);
        if (state.logger) {
            state.logger->flush();
            if (loggingError[0]) {
                return VSCP_ERROR_ERROR;
            }
        }
        std::vector<spdlog::sink_ptr> sinks;
        if (consoleLevel != VSCP_HELPER_LOG_LEVEL_OFF) {
            auto sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
            sink->set_level(static_cast<spdlog::level::level_enum>(consoleLevel));
            sinks.push_back(sink);
        }
        if (fileLevel != VSCP_HELPER_LOG_LEVEL_OFF) {
            auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                filePath, maxFileSize, maxFiles);
            sink->set_level(static_cast<spdlog::level::level_enum>(fileLevel));
            sinks.push_back(sink);
        }
        auto logger = std::make_shared<spdlog::logger>("vscp-helper", sinks.begin(), sinks.end());
        logger->set_level(static_cast<spdlog::level::level_enum>(
            consoleLevel < fileLevel ? consoleLevel : fileLevel));
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        logger->flush_on(spdlog::level::err);
        logger->set_error_handler([](const std::string& message) { reportError(message.c_str()); });
        state.logger = std::move(logger);
        state.defaultStderr = false;
        debugEnabled.store(state.logger->should_log(spdlog::level::debug), std::memory_order_release);
        return VSCP_ERROR_SUCCESS;
    }
    catch (const std::exception& error) {
        reportError(error.what());
        return VSCP_ERROR_ERROR;
    }
    catch (...) {
        reportError("Unknown exception while configuring logging");
        return VSCP_ERROR_ERROR;
    }
}

extern "C" int
vscphelper_shared_disableLogging(void) noexcept
{
    loggingError[0] = '\0';
    try {
        auto& state = loggingState();
        std::lock_guard<std::mutex> lock(state.mutex);
        debugEnabled.store(false, std::memory_order_release);
        state.defaultStderr = false;
        if (state.logger) {
            state.logger->flush();
        }
        state.logger.reset();
        return loggingError[0] ? VSCP_ERROR_ERROR : VSCP_ERROR_SUCCESS;
    }
    catch (const std::exception& error) {
        reportError(error.what());
        return VSCP_ERROR_ERROR;
    }
    catch (...) {
        reportError("Unknown exception while disabling logging");
        return VSCP_ERROR_ERROR;
    }
}

extern "C" int
vscphelper_shared_flushLogging(void) noexcept
{
    loggingError[0] = '\0';
    try {
        auto& state = loggingState();
        std::lock_guard<std::mutex> lock(state.mutex);
        if (state.logger) {
            state.logger->flush();
        }
        return loggingError[0] ? VSCP_ERROR_ERROR : VSCP_ERROR_SUCCESS;
    }
    catch (const std::exception& error) {
        reportError(error.what());
        return VSCP_ERROR_ERROR;
    }
    catch (...) {
        reportError("Unknown exception while flushing logging");
        return VSCP_ERROR_ERROR;
    }
}

extern "C" const char*
vscphelper_shared_getLastLoggingError(void) noexcept
{
    return loggingError.data();
}

extern "C" void vscphelper_shared_logTransportError(const char* format, va_list arguments) noexcept
{
    if (!format) {
        reportError("Invalid transport diagnostic format");
        return;
    }
    try {
        auto& state = loggingState();
        std::lock_guard<std::mutex> lock(state.mutex);
        if (state.defaultStderr) {
            std::fputs("VSCP TCP: ", stderr);
            std::vfprintf(stderr, format, arguments);
            std::fputc('\n', stderr);
            return;
        }
        if (!state.logger) {
            return;
        }
        if (!state.logger->should_log(spdlog::level::err)) {
            return;
        }
        std::array<char, 2048> message{};
        va_list copy;
        va_copy(copy, arguments);
        int length = std::vsnprintf(message.data(), message.size(), format, copy);
        va_end(copy);
        if (length < 0) {
            reportError("Cannot format transport diagnostic");
            return;
        }
        if (static_cast<size_t>(length) >= message.size()) {
            // Transport diagnostics can contain arbitrarily long paths/hostnames.
            std::vector<char> fullMessage(static_cast<size_t>(length) + 1);
            if (std::vsnprintf(fullMessage.data(), fullMessage.size(), format, arguments) < 0) {
                reportError("Cannot format transport diagnostic");
                return;
            }
            state.logger->error("VSCP TCP: {}", fullMessage.data());
        }
        else {
            state.logger->error("VSCP TCP: {}", message.data());
        }
    }
    catch (const std::exception& error) {
        reportError(error.what());
    }
    catch (...) {
        reportError("Unknown exception while logging a transport diagnostic");
    }
}

extern "C" int vscphelper_shared_isLoggingEnabled(int level) noexcept
{
    if (!validLevel(level) || level == VSCP_HELPER_LOG_LEVEL_OFF) {
        reportError("Invalid logging message level");
        return 0;
    }
    auto logger = getLogger();
    return logger && logger->should_log(static_cast<spdlog::level::level_enum>(level));
}

extern "C" void vscphelper_shared_logMessage(int level, const char* message, size_t length) noexcept
{
    if (!validLevel(level) || level == VSCP_HELPER_LOG_LEVEL_OFF || !message) {
        reportError("Invalid logging message or level");
        return;
    }
    try {
        auto logger = getLogger();
        if (logger) {
            logger->log(static_cast<spdlog::level::level_enum>(level),
                        spdlog::string_view_t(message, length));
        }
    }
    catch (const std::exception& error) {
        reportError(error.what());
    }
    catch (...) {
        reportError("Unknown exception while logging a message");
    }
}

extern "C" void vscphelper_shared_reportLoggingFailure(const char* message) noexcept
{
    reportError(message);
}

extern "C" void vscphelper_shared_logFunctionEntry(const char* component, const char* function) noexcept
{
    if (!debugEnabled.load(std::memory_order_acquire)) {
        return;
    }
    SavedErrorState saved;
    if (!component || !function) {
        reportError("Invalid function-entry diagnostic");
        return;
    }
    try {
        auto logger = getLogger();
        if (logger && logger->should_log(spdlog::level::debug)) {
            logger->debug("VSCP {}: {}: entered", component, function);
        }
    }
    catch (const std::exception& error) {
        reportError(error.what());
    }
    catch (...) {
        reportError("Unknown exception while logging a function entry");
    }
}

extern "C" void
vscphelper_shared_logFunctionStatus(const char* component, const char* function, int status) noexcept
{
    if (!debugEnabled.load(std::memory_order_acquire)) {
        return;
    }
    SavedErrorState saved;
    if (!component || !function) {
        reportError("Invalid function-status diagnostic");
        return;
    }
    try {
        auto logger = getLogger();
        if (logger && logger->should_log(spdlog::level::debug)) {
            logger->debug("VSCP {}: {} returned {}", component, function, status);
        }
    }
    catch (const std::exception& error) {
        reportError(error.what());
    }
    catch (...) {
        reportError("Unknown exception while logging a function status");
    }
}

extern "C" void
vscphelper_shared_logFunctionFailure(const char* component, const char* function, int errorCode) noexcept
{
    SavedErrorState saved;
    if (!component || !function) {
        reportError("Invalid function-failure diagnostic");
        return;
    }
    try {
        auto logger = getLogger();
        if (logger && logger->should_log(spdlog::level::err)) {
            if (errorCode) {
                logger->error("VSCP {}: {} failed (error {})", component, function, errorCode);
            }
            else {
                logger->error("VSCP {}: {} failed", component, function);
            }
        }
    }
    catch (const std::exception& error) {
        reportError(error.what());
    }
    catch (...) {
        reportError("Unknown exception while logging a function failure");
    }
}
