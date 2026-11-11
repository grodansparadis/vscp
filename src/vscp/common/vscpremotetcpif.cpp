///////////////////////////////////////////////////////////////////////////////
// VscpRemoteTcpIf.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project
// <info@vscp.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef __GNUG__
//#pragma implementation
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include <deque>
#include <list>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <chrono>
#include <climits>
#include <condition_variable>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>
#include <mutex>
#include <system_error>
#include <thread>
#include <utility>

#include <crc.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include <math.h>
#include <stdlib.h>
#ifdef WIN32
#include <ws2tcpip.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "version.h"
#include "vscp.h"
#include "vscpdatetime.h"
#include "vscpremotetcpif.h"
#ifdef VSCP_ENABLE_HELPER_LOGGING
#include "vscphelperlogging.h"
#endif

namespace {

class SavedErrorState {
public:
  SavedErrorState() noexcept
    : errorNumber(errno)
#ifdef WIN32
    , windowsError(GetLastError())
    , socketError(WSAGetLastError())
#endif
  {
  }

  ~SavedErrorState() noexcept
  {
#ifdef WIN32
    WSASetLastError(socketError);
    SetLastError(windowsError);
#endif
    errno = errorNumber;
  }

private:
  int errorNumber;
#ifdef WIN32
  DWORD windowsError;
  int socketError;
#endif
};

void logTcpDebug(const char *function, const char *message) noexcept
{
#ifdef VSCP_ENABLE_HELPER_LOGGING
  SavedErrorState saved;
  VSCPHLP_LOG_DEBUG("VSCP TCP: {}: {}", function, message);
#else
  (void) function;
  (void) message;
#endif
}

void logTcpEntry(const char *function) noexcept
{
#ifdef VSCP_ENABLE_HELPER_LOGGING
  vscphelper_logFunctionEntry("TCP", function);
#else
  (void) function;
#endif
}

void transportError(const char *format, ...);

} // namespace

///////////////////////////////////////////////////////////////////////////////
// win_usleep
// 
// Function to sleep for a specified number of microseconds.
// @param usec The number of microseconds to sleep.
// @return None. Suspends the current thread for the specified duration.
  
#ifdef WIN32
static void
win_usleep(__int64 usec)
{
  logTcpEntry(__func__);
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative
                              // value indicates relative time

  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  if (!timer)
    transportError("cannot create waitable timer (error %lu)", GetLastError());
  if (!SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0))
    transportError("cannot set waitable timer (error %lu)", GetLastError());
  if (WaitForSingleObject(timer, INFINITE) == WAIT_FAILED)
    transportError("cannot wait for timer (error %lu)", GetLastError());
  if (!CloseHandle(timer))
    transportError("cannot close waitable timer (error %lu)", GetLastError());
}
#endif

namespace {

using Clock = std::chrono::steady_clock;
using Deadline = Clock::time_point;

// Limits for data buffered from the server, so a misbehaving server can not
// make the client use unbounded memory.
constexpr size_t maxPartialLine         = 65536;       // Unterminated text line
constexpr size_t maxCommandResponse     = 1024 * 1024; // Text reply to one command
constexpr size_t maxSingleChannelEvents = 10000;       // Queued receive loop events

#ifdef WIN32
using Socket = SOCKET;
const Socket invalidSocket = INVALID_SOCKET;
#else
using Socket = int;
const Socket invalidSocket = -1;
#endif

///////////////////////////////////////////////////////////////////////////////
// transportError
// 
// Function to report transport errors.
// @param format The format string for the error message.
// @param ... Additional arguments for the format string.
// @return None. Logs the transport error details.
  
void transportError(const char *format, ...)
{
  logTcpEntry(__func__);
  SavedErrorState saved;
  va_list arguments;
  va_start(arguments, format);
#ifdef VSCP_ENABLE_HELPER_LOGGING
  vscphelper_logTransportError(format, arguments);
#else
  std::fputs("VSCP TCP: ", stderr);
  std::vfprintf(stderr, format, arguments);
  std::fputc('\n', stderr);
#endif
  va_end(arguments);
}

int logTcpResult(const char *function, int result, bool routineReceive = false) noexcept
{
  if (result == VSCP_ERROR_SUCCESS) {
    return result;
  }

  if (routineReceive &&
      (result == VSCP_ERROR_TIMEOUT || result == -VSCP_ERROR_TIMEOUT ||
       result == VSCP_ERROR_RCV_EMPTY || result == -VSCP_ERROR_RCV_EMPTY)) {
#ifdef VSCP_ENABLE_HELPER_LOGGING
    vscphelper_logFunctionStatus("TCP", function, result);
#endif
  }
  else {
    transportError("%s failed (error %d)", function, result);
  }
  return result;
}

int logTcpReadResult(const char *function, int result) noexcept
{
  return result < 0 ? logTcpResult(function, result, true) : result;
}

bool isReceiveCommand(const std::string &command)
{
  logTcpEntry(__func__);
  size_t position = command.find_first_not_of(" \t\r\n\f\v");
  if (position == std::string::npos || command.size() - position < 4) {
    return false;
  }
  static constexpr char name[] = "RETR";
  for (size_t i = 0; i < sizeof(name) - 1; ++i) {
    if (std::toupper(static_cast<unsigned char>(command[position + i])) != name[i]) {
      return false;
    }
  }
  position += sizeof(name) - 1;
  return position == command.size() || std::isspace(static_cast<unsigned char>(command[position]));
}

bool isEmptyReceiveResponse(const std::string &response)
{
  logTcpEntry(__func__);
  static constexpr char emptyResponse[] = "-OK - No event(s) available";
  size_t position = response.rfind(emptyResponse);
  if (position == std::string::npos || (position && response[position - 1] != '\n')) {
    return false;
  }
  size_t end = position + sizeof(emptyResponse) - 1;
  return response.find_first_not_of(" \t\r\n", end) == std::string::npos;
}

bool isEmptyReceiveResponse(const std::deque<std::string> &lines)
{
  logTcpEntry(__func__);
  return !lines.empty() && isEmptyReceiveResponse(lines.back());
}

///////////////////////////////////////////////////////////////////////////////
// tlsError
// 
// Function to report TLS errors.
// @param operation The operation that caused the error.
// @return None. Logs the TLS error details.
  
void tlsError(const char *operation)
{
  logTcpEntry(__func__);
  unsigned long error = ERR_peek_last_error();
  char detail[256] = {};
  if (error)
    ERR_error_string_n(error, detail, sizeof(detail));
  transportError("%s: %s", operation, error ? detail : "TLS operation failed");
  ERR_clear_error();
}

///////////////////////////////////////////////////////////////////////////////
// SocketRuntime
// Structure to manage the socket runtime environment, including initialization
// and cleanup for platforms that require it (e.g., Windows).
// 
///////////////////////////////////////////////////////////////////////////////

struct SocketRuntime {
  int error = 0;

///////////////////////////////////////////////////////////////////////////////
// SocketRuntime
// 
// Function to initialize the socket runtime environment.
// This is necessary for platforms like Windows that require explicit
// initialization of the socket library.
//

#ifdef WIN32
  SocketRuntime()
  {
    logTcpEntry(__func__);
    WSADATA data = {};
    error = WSAStartup(MAKEWORD(2, 2), &data);
  }
  ~SocketRuntime()
  {
    logTcpEntry(__func__);
    if (!error && WSACleanup() != 0)
      transportError("cannot clean up sockets (error %d)", WSAGetLastError());
  }
#endif
};

///////////////////////////////////////////////////////////////////////////////
// socketError
// 
// Function to retrieve the last socket error code.
// @return The error code of the last socket operation.

int socketError()
{
  logTcpEntry(__func__);
#ifdef WIN32
  return WSAGetLastError();
#else
  return errno;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// socketInterrupted
//
// Function to check if a socket was interrupted.
// @param error The error code returned by the socket operation.
// @return True if the socket was interrupted, false otherwise.

bool socketInterrupted(int error)
{
  logTcpEntry(__func__);
#ifdef WIN32
  return error == WSAEINTR;
#else
  return error == EINTR;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// socketWouldBlock
// 
// Function to check if a socket would block on the next operation.
// @param error The error code returned by the socket operation.
// @return True if the socket would block, false otherwise.
//

bool socketWouldBlock(int error)
{
  logTcpEntry(__func__);
#ifdef WIN32
  return error == WSAEWOULDBLOCK;
#else
  return error == EAGAIN || error == EWOULDBLOCK;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// socketConnecting
// 
// Function to check if a socket is in the process of connecting.
// @param error The error code returned by the socket operation.
// @return True if the socket is connecting, false otherwise.
// 

bool socketConnecting(int error)
{
  logTcpEntry(__func__);
#ifdef WIN32
  return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS || error == WSAEALREADY ||
         error == WSAEINTR;
#else
  return error == EINPROGRESS || error == EALREADY || error == EINTR || socketWouldBlock(error);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// closeSocket
// 
// Function to close a socket.
// @param socket The socket to close.
// @note The function will safely handle invalid sockets.
// 

void closeSocket(Socket socket)
{
  logTcpEntry(__func__);
  if (socket == invalidSocket)
    return;
#ifdef WIN32
  int result = closesocket(socket);
#else
  int result = close(socket);
#endif
  if (result != 0)
    transportError("cannot close socket (error %d)", socketError());
}

///////////////////////////////////////////////////////////////////////////////
// setNonBlocking
// 
// Function to set a socket to non-blocking mode.
// @param socket The socket to configure.
// @return True if the operation was successful, false otherwise.
//

bool setNonBlocking(Socket socket)
{
  logTcpEntry(__func__);
#ifdef WIN32
  u_long enabled = 1;
  bool result = ioctlsocket(socket, FIONBIO, &enabled) == 0;
#else
  int flags = fcntl(socket, F_GETFL, 0);
  bool result = flags >= 0 && fcntl(socket, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
  if (!result)
    transportError("cannot set nonblocking socket mode (error %d)", socketError());
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// waitForSocket
// 
// Function to wait for a socket to become ready for reading or writing.
// @param socket The socket to monitor.
// @param write True to wait for writability, false to wait for readability.
// @param deadline The time point until which to wait.
// @return 1 if the socket is ready, 0 if the deadline is reached, -1 on error.
  
int waitForSocket(Socket socket, bool write, Deadline deadline)
{
  logTcpEntry(__func__);
  while (true) {
    auto now = Clock::now();
    if (now >= deadline)
      return 0;
    auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
    int timeout = static_cast<int>((std::min<int64_t>)(remaining + 1, INT_MAX));
#ifdef WIN32
    fd_set readSet, writeSet, errorSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);
    if (write)
      FD_SET(socket, &writeSet);
    else
      FD_SET(socket, &readSet);
    FD_SET(socket, &errorSet);
    timeval value = {timeout / 1000, (timeout % 1000) * 1000};
    int result = select(0, write ? nullptr : &readSet, write ? &writeSet : nullptr, &errorSet, &value);
#else
    pollfd descriptor = {socket, static_cast<short>(write ? POLLOUT : POLLIN), 0};
    int result = poll(&descriptor, 1, timeout);
#endif
    if (result > 0)
      return 1;
    if (!result)
      continue;
    int error = socketError();
    if (!socketInterrupted(error)) {
      transportError("socket wait failed (error %d)", error);
      return -1;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// waitForConnections
// 
// Function to wait for incoming connections on multiple sockets.
// @param sockets A vector of sockets to monitor for incoming connections.
// @param deadline The time point until which to wait for connections.
// @param ready A vector to store the indices of sockets that are ready for connections.
// @return 1 if one or more sockets are ready, 0 if the deadline is reached, -1 on error.

int waitForConnections(const std::vector<Socket> &sockets, Deadline deadline, std::vector<size_t> &ready)
{
  logTcpEntry(__func__);
#ifndef WIN32
  std::vector<pollfd> descriptors;
  for (Socket socket : sockets)
    descriptors.push_back({socket, POLLOUT, 0});
#endif
  while (true) {
    auto now = Clock::now();
    if (now >= deadline)
      return 0;
    auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
    int timeout = static_cast<int>((std::min<int64_t>)(remaining + 1, INT_MAX));
#ifdef WIN32
    fd_set writeSet, errorSet;
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);
    for (Socket socket : sockets) {
      FD_SET(socket, &writeSet);
      FD_SET(socket, &errorSet);
    }
    timeval value = {timeout / 1000, (timeout % 1000) * 1000};
    int result = select(0, nullptr, &writeSet, &errorSet, &value);
#else
    int result = poll(descriptors.data(), descriptors.size(), timeout);
#endif
    if (result > 0) {
      ready.clear();
      for (size_t i = 0; i < sockets.size(); ++i) {
#ifdef WIN32
        if (FD_ISSET(sockets[i], &writeSet) || FD_ISSET(sockets[i], &errorSet))
#else
        if (descriptors[i].revents)
#endif
          ready.push_back(i);
      }
      return 1;
    }
    if (!result)
      continue;
    int error = socketError();
    if (!socketInterrupted(error)) {
      transportError("socket wait failed (error %d)", error);
      return -1;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// ConnectionAttempts
// 
// Structure to manage multiple connection attempts.
// @note The structure will automatically close all sockets upon destruction.
// @note Individual sockets can be taken from the structure, removing them from management.

struct ConnectionAttempts {
  ~ConnectionAttempts()
  {
    logTcpEntry(__func__);
    closeAll();
  }

  ///////////////////////////////////////////////////////////////////////////////
  // closeAll
  // 
  // Closes all sockets managed by the structure and clears the list.
  // @note This will also be called automatically by the destructor.
  // 

  void closeAll()
  {
    logTcpEntry(__func__);
    for (Socket socket : sockets) {
      closeSocket(socket);
    }
    sockets.clear();
  }

  ///////////////////////////////////////////////////////////////////////////////
  // take
  // 
  // Takes ownership of the socket at the specified index, removing it from management.
  // @param index The index of the socket to take.
  // @return The socket that was taken.
  //
  Socket take(size_t index)
  {
    logTcpEntry(__func__);
    Socket socket = sockets[index];
    sockets.erase(sockets.begin() + index);
    return socket;
  }

  /// List of sockets managed by the structure.
  std::vector<Socket> sockets;
};

///////////////////////////////////////////////////////////////////////////////
// socketRead
// 
// Function to read data from a socket.
// @param socket The socket to read from.
// @param buffer The buffer to store the read data.
// @param length The number of bytes to read.
// @return The number of bytes read, or a negative value if an error occurs.
// @note The function will attempt to read the specified number of bytes, retrying if interrupted by signals.
// @note The function will return the number of bytes read, or a negative value if an error occurs.
// 

int socketRead(Socket socket, char *buffer, int length)
{
  logTcpEntry(__func__);
  int result;
  do {
    result = static_cast<int>(recv(socket, buffer, length, 0));
  } while (result < 0 && socketInterrupted(socketError()));
  if (result < 0) {
    int error = socketError();
    if (!socketWouldBlock(error))
      transportError("socket read failed (error %d)", error);
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// socketWrite
// 
// Function to write data to a socket.
// @param socket The socket to write to.
// @param buffer The buffer containing the data to send.
// @param length The number of bytes to send.
// @return The number of bytes written, or a negative value if an error occurs.
// @note The function will attempt to send the entire buffer, retrying if interrupted by signals.
// @note The function will respect the MSG_NOSIGNAL flag on platforms that support it to prevent SIGPIPE signals.

int socketWrite(Socket socket, const char *buffer, int length)
{
  logTcpEntry(__func__);
#if !defined(WIN32) && defined(MSG_NOSIGNAL)
  const int flags = MSG_NOSIGNAL;
#else
  const int flags = 0;
#endif
  int result;
  do {
    result = static_cast<int>(send(socket, buffer, length, flags));
  } while (result < 0 && socketInterrupted(socketError()));
  if (result < 0) {
    int error = socketError();
    if (!socketWouldBlock(error))
      transportError("socket write failed (error %d)", error);
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// AddressResult
// 
// Structure to hold the result of an asynchronous address resolution.
// @note The structure will be updated by the resolver task upon completion.
// @note The structure contains synchronization primitives to safely access the result from multiple threads.
// 

struct AddressResult {
  explicit AddressResult(std::shared_ptr<SocketRuntime> runtime) : runtime(std::move(runtime)) { logTcpEntry(__func__); }
  ~AddressResult()
  {
    logTcpEntry(__func__);
    if (addresses)
      freeaddrinfo(addresses);
  }

  std::shared_ptr<SocketRuntime> runtime;
  std::mutex mutex;
  std::condition_variable ready;
  bool done = false;
  int error = 0;
  addrinfo *addresses = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
// ResolverTask
// 
// Task for resolving a hostname asynchronously.
// @note The task will run in a separate thread and will update the associated AddressResult upon completion.ate the associated AddressResult upon completion.
//
  
struct ResolverTask {
  explicit ResolverTask(std::shared_ptr<AddressResult> state) : state(std::move(state)) { logTcpEntry(__func__); }
  ~ResolverTask()
  {
    logTcpEntry(__func__);
    if (worker.joinable())
      worker.join();
  }

  std::shared_ptr<AddressResult> state;
  std::thread worker;
};

///////////////////////////////////////////////////////////////////////////////
// ResolverTasks
// Class for managing multiple resolver tasks.
// @note The class provides thread-safe methods to keep and reap resolver tasks.
// 

class ResolverTasks {
public:

  /// Updates the list of resolver tasks by removing completed ones.
  /// Keeps a resolver task alive by adding it to the list of managed tasks.
  /// @note Both methods are thread-safe and can be called from multiple threads concurrently.
  void reap()
  {
    logTcpEntry(__func__);
    std::lock_guard<std::mutex> lock(mutex);
    auto task = tasks.begin();
    while (task != tasks.end()) {
      bool done;
      {
        std::lock_guard<std::mutex> resultLock((*task)->state->mutex);
        done = (*task)->state->done;
      }
      if (done)
        task = tasks.erase(task);
      else
        ++task;
    }
  }

  /// Adds a resolver task to the list of managed tasks.
  /// @param task The resolver task to keep alive.
  /// @note The method is thread-safe and can be called from multiple threads concurrently.
  void keep(std::unique_ptr<ResolverTask> task)
  {
    logTcpEntry(__func__);
    std::lock_guard<std::mutex> lock(mutex);
    tasks.push_back(std::move(task));
  }

private:
  std::mutex mutex;
  std::vector<std::unique_ptr<ResolverTask>> tasks;
};

///////////////////////////////////////////////////////////////////////////////
// ResolverTasks
// 
// Global resolver tasks manager accessor.  
//
// Provides access to the global resolver tasks manager.
// @return A reference to the global ResolverTasks instance.
//

ResolverTasks &resolverTasks()
{
  logTcpEntry(__func__);
  static ResolverTasks tasks;
  return tasks;
}

///////////////////////////////////////////////////////////////////////////////
// resolveAddress
// 
// Resolve a hostname to an address.
// @param host The hostname to resolve.
// @param port The port to resolve.
// @param runtime The socket runtime context.
// @param deadline The deadline for the resolution.
// @param result The result code of the resolution.
// @return A shared pointer to the address result on success, nullptr on failure.
// @note The function will not modify the underlying socket.
// @note The function will not block the calling thread indefinitely; it respects the provided deadline.  
// @note The function will perform the resolution asynchronously and may return before the resolution is complete.
// @note The function will keep timed-out lookups alive until they finish, including during library unload.
// @note The function will notify the caller when the resolution is complete, even if it timed out.
// @note The function will return nullptr if the resolution fails or times out.
// @note The function will return a valid address result only if the resolution succeeds.
// 
  
std::shared_ptr<AddressResult> resolveAddress(const std::string &host,
                                              const std::string &port,
                                              const std::shared_ptr<SocketRuntime> &runtime,
                                              Deadline deadline,
                                              int &result)
{
  logTcpEntry(__func__);
  if (Clock::now() >= deadline) {
    result = VSCP_ERROR_TIMEOUT;
    return nullptr;
  }
  resolverTasks().reap();
  auto state = std::make_shared<AddressResult>(runtime);
  std::unique_ptr<ResolverTask> task(new ResolverTask(state));
  try {
    task->worker = std::thread([state, host, port]() {
      addrinfo hints = {};
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      addrinfo *addresses = nullptr;
      int error = getaddrinfo(host.c_str(), port.c_str(), &hints, &addresses);
      {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->addresses = addresses;
        state->error = error;
        state->done = true;
      }
      state->ready.notify_one();
    });
  }
  catch (const std::system_error &error) {
    transportError("cannot start hostname resolver: %s", error.what());
    result = VSCP_ERROR_CONNECTION;
    return nullptr;
  }

  std::unique_lock<std::mutex> lock(state->mutex);
  if (!state->ready.wait_until(lock, deadline, [&state] { return state->done; })) {
    lock.unlock();
    // Keep timed-out lookups alive until they finish, including during library unload.
    resolverTasks().keep(std::move(task));
    result = VSCP_ERROR_TIMEOUT;
    return nullptr;
  }
  lock.unlock();
  task.reset();
  if (state->error || !state->addresses) {
    transportError("cannot resolve host %s (error %d)", host.c_str(), state->error);
    result = VSCP_ERROR_CONNECTION;
    return nullptr;
  }
  result = VSCP_ERROR_SUCCESS;
  return state;
}

///////////////////////////////////////////////////////////////////////////////
// socketBioCreate
// 
// Initialize a socket BIO.
// @param bio The BIO to initialize.
// @return 1 on success, 0 on failure.
// 
  
int socketBioCreate(BIO *bio)
{
  logTcpEntry(__func__);
  BIO_set_init(bio, 1);
  BIO_set_data(bio, nullptr);
  BIO_set_shutdown(bio, 0);
  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// socketBioDestroy
//
// Destroy a socket BIO.
// @param bio The BIO to destroy.
// @return 1 on success, 0 on failure.
//

int socketBioDestroy(BIO *bio)
{
  logTcpEntry(__func__);
  if (bio) {
    BIO_set_data(bio, nullptr);
    BIO_set_init(bio, 0);
  }
  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// socketBioControl
// 
// Control a socket BIO.
// @param bio The BIO to control.
// @param command The control command.
// @param arg1 The first argument for the command.
// @param arg2 The second argument for the command.
// @return 1 on success, 0 on failure.
// 

long socketBioControl(BIO *, int command, long, void *)
{
  logTcpEntry(__func__);
  return command == BIO_CTRL_FLUSH || command == BIO_C_SET_NBIO ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////
// socketBioRead
// 
// Read data from a socket BIO.
// @param bio The BIO to read from.
// @param buffer The buffer to store the read data.
// @param length The maximum number of bytes to read.
// @return The number of bytes read, or 0 on failure.
//
  
int socketBioRead(BIO *bio, char *buffer, int length)
{
  logTcpEntry(__func__);
  BIO_clear_retry_flags(bio);
  auto socket = static_cast<Socket *>(BIO_get_data(bio));
  if (!socket || !buffer || length <= 0)
    return 0;
  int result = socketRead(*socket, buffer, length);
  if (result < 0 && socketWouldBlock(socketError()))
    BIO_set_retry_read(bio);
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// socketBioWrite
//
// Write data to a socket BIO.
// @param bio The BIO to write to.
// @param buffer The buffer containing the data to write.
// @param length The number of bytes to write.
// @return The number of bytes written, or 0 on failure.
// @note The function will not modify the original BIO structure.
// @note The function will not modify the underlying socket.
//

int socketBioWrite(BIO *bio, const char *buffer, int length)
{
  logTcpEntry(__func__);
  BIO_clear_retry_flags(bio);
  auto socket = static_cast<Socket *>(BIO_get_data(bio));
  if (!socket || !buffer || length <= 0) {
    return 0;
  }
  int result = socketWrite(*socket, buffer, length);
  if (result < 0 && socketWouldBlock(socketError())) {
    BIO_set_retry_write(bio);
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// isDirectory
// 
// Check if a given path is a directory.
// @param path The path to check.
// @return true if the path is a directory, false otherwise.
// 

bool isDirectory(const std::string &path)
{
  logTcpEntry(__func__);
#ifdef WIN32
  DWORD attributes = GetFileAttributesA(path.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
#else
  struct stat attributes = {};
  return stat(path.c_str(), &attributes) == 0 && S_ISDIR(attributes.st_mode);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// readTlsFile
// Read the contents of a TLS file into a string.
// @param path The path to the TLS file.
// @param contents The string to store the file contents.
// @return true if the file was successfully read, false otherwise.
// @note The function will not modify the original file.
// @note The function will handle errors internally and report them via transportError.
// 
  
bool readTlsFile(const std::string &path, std::string &contents)
{
  logTcpEntry(__func__);
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    transportError("cannot read TLS file: %s", path.c_str());
    return false;
  }
  contents.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  if (file.bad() || contents.empty()) {
    transportError("empty or unreadable TLS file: %s", path.c_str());
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// CaDirectory
// Structure representing a TLS CA directory.
// Contains the path to the directory, the concatenated certificates, and a failure flag.
//

struct CaDirectory {
  std::string path;
  std::string certificates;
  bool failed;
};

///////////////////////////////////////////////////////////////////////////////
// readCaEntry
// 
// Read a single entry from a TLS CA directory.
//
// @param name The name of the directory entry.
// @param arg A pointer to a CaDirectory structure.
//
// This function reads the certificate from the specified directory entry and appends it to the certificates string in the CaDirectory structure.
// @note This function is intended to be used as a callback for iterating over directory entries.
// @note The directory entry should be a PEM-encoded certificate file.
// @note If reading the certificate fails, the failed flag in the CaDirectory structure will be set.
// @note This function will skip over non-directory entries that are not valid PEM files.
// @note The function will free any allocated BIO and X509 structures before returning.
// @note This function does not recurse into subdirectories.
// @note This function is platform-independent and works on both Windows and POSIX systems.
// @note The function will not modify the original directory entry files.
// @note The function will not throw exceptions and will handle errors internally.
// @note The function will not modify the original directory entry files.
// @return void.
//
  
void readCaEntry(const char *name, void *arg)
{
  logTcpEntry(__func__);
  auto &directory = *static_cast<CaDirectory *>(arg);
  if (directory.failed) {
    return;
  }
  std::string path = directory.path + "/" + name;
  if (isDirectory(path)) {
    return;
  }
  BIO *input = BIO_new_file(path.c_str(), "r");
  BIO *output = input ? BIO_new(BIO_s_mem()) : nullptr;
  if (input && !output) {
    directory.failed = true;
  }
  X509 *cert;
  while (input && output && (cert = PEM_read_bio_X509(input, nullptr, nullptr, nullptr))) {
    if (PEM_write_bio_X509(output, cert) != 1) {
      directory.failed = true;
    }
    else {
      char *data = nullptr;
      long length = BIO_get_mem_data(output, &data);
      directory.certificates.append(data, static_cast<size_t>(length));
      BIO_reset(output);
    }
    X509_free(cert);
    if (directory.failed)
      break;
  }
  BIO_free(input);
  BIO_free(output);
  if (!directory.failed) {
    ERR_clear_error(); // Directory entries may include unrelated files.
  }
}

///////////////////////////////////////////////////////////////////////////////
// readCaDirectory
// 
// Read all PEM certificates from a TLS CA directory.
// Returns true on success, false on failure.
// @param path The path to the TLS CA directory.
// @param certificates A string to store the concatenated PEM certificates.
// @param allowEmpty Whether to allow an empty directory without failing.
// @return True if the certificates were successfully read, false otherwise.
// 
  
bool readCaDirectory(const std::string &path, std::string &certificates, bool allowEmpty = false)
{
  logTcpEntry(__func__);
  if (!isDirectory(path)) {
    transportError("invalid TLS CA directory: %s", path.c_str());
    return false;
  }
  CaDirectory directory{path, "", false};
#ifdef WIN32
  std::string pattern = path;
  if (pattern.empty() || (pattern.back() != '/' && pattern.back() != '\\')) {
    pattern += '\\';
  }
  pattern += '*';
  WIN32_FIND_DATAA entry;
  HANDLE search = FindFirstFileA(pattern.c_str(), &entry);
  if (search != INVALID_HANDLE_VALUE) {
    do
      readCaEntry(entry.cFileName, &directory);
    while (FindNextFileA(search, &entry));
    DWORD error = GetLastError();
    FindClose(search);
    if (error != ERROR_NO_MORE_FILES) {
      transportError("cannot enumerate TLS CA directory: %s (error %lu)", path.c_str(), error);
      return false;
    }
  }
  else if (GetLastError() != ERROR_FILE_NOT_FOUND) {
    transportError("cannot open TLS CA directory: %s (error %lu)", path.c_str(), GetLastError());
    return false;
  }
#else
  DIR *entries = opendir(path.c_str());
  if (!entries) {
    transportError("cannot open TLS CA directory: %s (error %d)", path.c_str(), errno);
    return false;
  }
  while (true) {
    errno = 0;
    dirent *entry = readdir(entries);
    if (!entry) {
      int error = errno;
      closedir(entries);
      if (error) {
        transportError("cannot enumerate TLS CA directory: %s (error %d)", path.c_str(), error);
        return false;
      }
      break;
    }
    readCaEntry(entry->d_name, &directory);
  }
#endif
  if (directory.failed) {
    tlsError("cannot read certificates from TLS CA directory");
    return false;
  }
  certificates += directory.certificates;
  if (directory.certificates.empty() && !allowEmpty) {
    transportError("no PEM certificates in TLS CA directory: %s", path.c_str());
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// readTrustAnchors
//
// Read TLS trust anchors from the specified client configuration.
// @param client The client configuration containing TLS trust anchor information.
// @param ca A string to store the loaded TLS trust anchors.
// @return True if the trust anchors were successfully read, false otherwise.
// 

bool readTrustAnchors(VscpRemoteTcpIf &client, std::string &ca)
{
  logTcpEntry(__func__);
  if (!client.m_cafile.empty() && !readTlsFile(client.m_cafile, ca)) {
    return false;
  }
  if (!client.m_capath.empty() && !readCaDirectory(client.m_capath, ca)) {
    return false;
  }
  if (client.m_cafile.empty() && client.m_capath.empty()) {
    const char *file = getenv(X509_get_default_cert_file_env());
    const char *path = getenv(X509_get_default_cert_dir_env());
    std::string defaultFile = file ? file : X509_get_default_cert_file();
#ifdef WIN32
    std::ifstream defaultCertFile(defaultFile, std::ios::binary);
    bool hasDefaultCertFile = defaultCertFile && !isDirectory(defaultFile);
    defaultCertFile.close();
#else
    struct stat attributes = {};
    bool hasDefaultCertFile = stat(defaultFile.c_str(), &attributes) == 0 && !S_ISDIR(attributes.st_mode);
#endif
    if (hasDefaultCertFile && !readTlsFile(defaultFile, ca)) {
      return false;
    }
    std::deque<std::string> directories;
#ifdef WIN32
    const char *separator = ";";
#else
    const char *separator = ":";
#endif
    vscp_split(directories, path ? path : X509_get_default_cert_dir(), separator);
    for (const auto &directory : directories) {
      if (isDirectory(directory) && !readCaDirectory(directory, ca, true)) {
        return false;
      }
    }
  }
  if (ca.empty()) {
    transportError("no TLS trust anchors loaded");
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// rejectPassword
//
// Callback function to reject a password for TLS operations.
// @return Always returns 0 to indicate rejection.
//

int rejectPassword(char *, int, int, void *)
{
  logTcpEntry(__func__);
  // Supply no PEM password when loading TLS trust anchors, avoiding prompts
  // and preventing encrypted PEM content from being unlocked. This callback
  // does not handle the remote client's login password.
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// loadTrustAnchors
//
// Load the client's configured trust anchors into the TLS context.
// Returns false if no usable anchors can be loaded.
//

bool loadTrustAnchors(SSL_CTX *context, VscpRemoteTcpIf &client)
{
  logTcpEntry(__func__);
  std::string ca;
  if (!readTrustAnchors(client, ca))
    return false;
  if (ca.size() > INT_MAX) {
    transportError("TLS trust bundle is too large");
    return false;
  }
  BIO *input = BIO_new_mem_buf(ca.data(), static_cast<int>(ca.size()));
  STACK_OF(X509_INFO) *certificates = input ? PEM_X509_INFO_read_bio(input, nullptr, rejectPassword, nullptr) : nullptr;
  BIO_free(input);
  if (!certificates) {
    tlsError("cannot parse TLS trust anchors");
    return false;
  }
  X509_STORE *store = SSL_CTX_get_cert_store(context);
  bool valid = store != nullptr;
  int count = 0;
  for (int i = 0; valid && i < sk_X509_INFO_num(certificates); ++i) {
    X509_INFO *info = sk_X509_INFO_value(certificates, i);
    if (info->x509) {
      valid = X509_STORE_add_cert(store, info->x509) == 1;
      ++count;
    }
  }
  sk_X509_INFO_pop_free(certificates, X509_INFO_free);
  if (!valid || !count) {
    tlsError("cannot load TLS trust anchors");
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// tlsPassword
//
// Callback function to provide a password for TLS operations.
// @param buffer Buffer to store the password.
// @param size Size of the buffer.
// @param rwflag Unused read/write flag.
// @param arg Pointer to a std::string containing the password.
// @return The number of characters copied to the buffer.
//

int tlsPassword(char *buffer, int size, int, void *arg)
{
  logTcpEntry(__func__);
  const auto &password = *static_cast<std::string *>(arg);
  if (size <= 0 || password.size() > static_cast<size_t>(size)) {
    transportError("TLS private key password buffer is too small");
    return 0;
  }
  return static_cast<int>(password.copy(buffer, static_cast<size_t>(size)));
}

} // namespace

void
VscpRemoteTcpIf::logDebugEntry(const char *function) noexcept
{
  logTcpEntry(function);
}

///////////////////////////////////////////////////////////////////////////////
// Transport
//
// Transport structure for managing socket and TLS state.
// This structure encapsulates the necessary components for handling both
// plain socket communication and TLS-encrypted communication, including
// the SSL context, SSL object, and BIO method for custom I/O operations.
// It provides a convenient abstraction for managing the lifecycle and configuration of the transport layer.
// It abstracts the details of socket and TLS management, providing a unified interface for network communication.
// It simplifies the integration of TLS into the transport layer by handling the necessary setup and teardown operations.
// It ensures that both plain and encrypted communication can be handled seamlessly within the same transport abstraction.
// It provides a consistent interface for reading from and writing to the transport, regardless of whether TLS is enabled.
// It abstracts the complexity of managing TLS sessions, certificate verification, and secure communication channels.
// It manages the underlying socket resource, ensuring proper cleanup and preventing resource leaks.
// This structure is used internally by the VscpRemoteTcpIf class to manage network communication, providing a robust and secure transport layer for both plain and TLS-encrypted connections.
//

struct VscpRemoteTcpIf::Transport {
  std::shared_ptr<SocketRuntime> runtime = std::make_shared<SocketRuntime>();
  Socket socket = invalidSocket;
  SSL_CTX *context = nullptr;
  SSL *tls = nullptr;
  BIO_METHOD *bioMethod = nullptr;
  std::atomic<bool> connected{false};
  std::mutex tlsMutex;

  ///< Destructor to clean up TLS and socket resources.
  ~Transport()
  {
    logTcpEntry(__func__);
    SSL_free(tls);
    BIO_meth_free(bioMethod);
    SSL_CTX_free(context);
    closeSocket(socket);
  }

  bool configureTls(VscpRemoteTcpIf &client, const std::string &host);
  int open(VscpRemoteTcpIf &client, const std::string &host, const std::string &port);
  int read(void *buffer, size_t length, int timeout);
  int write(const void *buffer, size_t length, uint32_t timeout);
};

///////////////////////////////////////////////////////////////////////////////
// configureTls
//
// Configures the TLS context and SSL object for the transport.
// It sets up certificate verification, loads client certificates and keys if provided,
// and configures the hostname for TLS verification. Returns true on success, false on failure.
//

bool
VscpRemoteTcpIf::Transport::configureTls(VscpRemoteTcpIf &client, const std::string &host)
{
  logTcpEntry(__func__);
  ERR_clear_error();
  context = SSL_CTX_new(TLS_client_method());
  if (!context) {
    tlsError("cannot create TLS context");
    return false;
  }
  SSL_CTX_set_options(context, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
  SSL_CTX_set_verify(context, client.m_bVerifyPeer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, nullptr);
  if (client.m_bVerifyPeer && !loadTrustAnchors(context, client))
    return false;

  if (!client.m_certfile.empty() &&
      SSL_CTX_use_certificate_chain_file(context, client.m_certfile.c_str()) != 1) {
    ERR_clear_error();
    if (SSL_CTX_use_certificate_file(context, client.m_certfile.c_str(), SSL_FILETYPE_ASN1) != 1) {
      tlsError("cannot load TLS client certificate");
      return false;
    }
  }
  if (!client.m_keyfile.empty() || !client.m_certfile.empty()) {
    SSL_CTX_set_default_passwd_cb(context, tlsPassword);
    SSL_CTX_set_default_passwd_cb_userdata(context, &client.m_pwKeyfile);
    const std::string &keyfile = client.m_keyfile.empty() ? client.m_certfile : client.m_keyfile;
    if (SSL_CTX_use_PrivateKey_file(context, keyfile.c_str(), SSL_FILETYPE_PEM) != 1 ||
        (!client.m_certfile.empty() && SSL_CTX_check_private_key(context) != 1)) {
      tlsError("cannot load TLS client private key");
      return false;
    }
  }
  tls = SSL_new(context);
  if (!tls) {
    tlsError("cannot create TLS connection");
    return false;
  }
  // An IP address literal is checked against the certificate's IP
  // addresses, a name against its DNS names. SNI is only sent for names
  // (RFC 6066 does not allow IP addresses in SNI).
  unsigned char address[sizeof(in6_addr)];
  const bool isIpAddress = (1 == inet_pton(AF_INET, host.c_str(), address)) ||
                           (1 == inet_pton(AF_INET6, host.c_str(), address));
  if (client.m_bVerifyPeer) {
    const int hostOk = isIpAddress ? X509_VERIFY_PARAM_set1_ip_asc(SSL_get0_param(tls), host.c_str())
                                   : SSL_set1_host(tls, host.c_str());
    if (hostOk != 1) {
      tlsError("cannot configure TLS hostname");
      return false;
    }
  }
  if (!isIpAddress && SSL_set_tlsext_host_name(tls, host.c_str()) != 1) {
    tlsError("cannot configure TLS server name");
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// open
//
// Opens a connection to the specified host and port.
// It initializes the socket, performs the connection handshake, and sets up TLS if required.
// Returns VSCP_ERROR_CONNECTION on failure, or 0 on success.

int
VscpRemoteTcpIf::Transport::open(VscpRemoteTcpIf &client, const std::string &host, const std::string &port)
{
  logTcpEntry(__func__);
  if (runtime->error) {
    transportError("cannot initialize sockets (error %d)", runtime->error);
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  if (client.m_bTLS && !configureTls(client, host)) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  const auto deadline = Clock::now() + std::chrono::seconds(client.m_connectionTimeOut);
  int result = VSCP_ERROR_CONNECTION;
  auto addresses = resolveAddress(host, port, runtime, deadline, result);
  if (!addresses) {
    return logTcpResult(__func__, result);
  }
  int lastError = 0;
  addrinfo *address = addresses->addresses;
  ConnectionAttempts attempts;
  auto nextAttempt = Clock::now();
#ifdef WIN32
  const size_t maxPending = FD_SETSIZE;
#else
  const size_t maxPending = (std::numeric_limits<size_t>::max)();
#endif
  while (true) {
    if (Clock::now() >= deadline) {
      return logTcpResult(__func__, VSCP_ERROR_TIMEOUT);
    }
    if (address && attempts.sockets.size() < maxPending &&
        (attempts.sockets.empty() || Clock::now() >= nextAttempt)) {
      addrinfo *current = address;
      address = address->ai_next;
      Socket candidate = ::socket(current->ai_family, current->ai_socktype, current->ai_protocol);
      if (candidate == invalidSocket) {
        lastError = socketError();
        continue;
      }
      if (!setNonBlocking(candidate)) {
        lastError = socketError();
        closeSocket(candidate);
        continue;
      }
#ifdef SO_NOSIGPIPE
      int noSigpipe = 1;
      if (setsockopt(candidate, SOL_SOCKET, SO_NOSIGPIPE, &noSigpipe, sizeof(noSigpipe)) != 0) {
        lastError = socketError();
        closeSocket(candidate);
        continue;
      }
#endif
#ifdef WIN32
      int connectedNow = ::connect(candidate, current->ai_addr, static_cast<int>(current->ai_addrlen));
#else
      int connectedNow = ::connect(candidate, current->ai_addr, static_cast<socklen_t>(current->ai_addrlen));
#endif
      if (connectedNow == 0) {
        socket = candidate;
        break;
      }
      lastError = socketError();
      if (!socketConnecting(lastError)) {
        closeSocket(candidate);
        continue;
      }
      attempts.sockets.push_back(candidate);
      nextAttempt = Clock::now() + std::chrono::milliseconds(250);
    }
    if (attempts.sockets.empty() && !address) {
      transportError("cannot connect to %s:%s (error %d)", host.c_str(), port.c_str(), lastError);
      return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
    }
    if (attempts.sockets.empty()) {
      continue;
    }
    Deadline waitDeadline = deadline;
    if (address && attempts.sockets.size() < maxPending)
      waitDeadline = (std::min)(waitDeadline, nextAttempt);
    std::vector<size_t> ready;
    int waitResult = waitForConnections(attempts.sockets, waitDeadline, ready);
    if (waitResult < 0) {
      return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
    }
    bool failed = false;
    for (auto index = ready.rbegin(); index != ready.rend(); ++index) {
      Socket candidate = attempts.sockets[*index];
      int error = 0;
#ifdef WIN32
      int size = sizeof(error);
      int status = getsockopt(candidate, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error), &size);
#else
      socklen_t size = sizeof(error);
      int status = getsockopt(candidate, SOL_SOCKET, SO_ERROR, &error, &size);
#endif
      if (status != 0) {
        error = socketError();
      }
      if (!error) {
        socket = attempts.take(*index);
        break;
      }
      lastError = error;
      closeSocket(attempts.take(*index));
      failed = true;
    }
    if (socket != invalidSocket) {
      break;
    }
    if (failed) {
      nextAttempt = Clock::now();
    }
  }
  attempts.closeAll();

  if (tls) {
    // Keep TLS writes on the same SIGPIPE-safe path as plain TCP writes.
    static const int socketBioType = BIO_get_new_index() | BIO_TYPE_SOURCE_SINK;
    bioMethod = BIO_meth_new(socketBioType, "vscp socket");
    if (!bioMethod || !BIO_meth_set_create(bioMethod, socketBioCreate) ||
        !BIO_meth_set_destroy(bioMethod, socketBioDestroy) ||
        !BIO_meth_set_read(bioMethod, socketBioRead) ||
        !BIO_meth_set_write(bioMethod, socketBioWrite) ||
        !BIO_meth_set_ctrl(bioMethod, socketBioControl)) {
      tlsError("cannot create TLS socket transport");
      return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
    }
    BIO *bio = BIO_new(bioMethod);
    if (!bio) {
      tlsError("cannot create TLS socket");
      return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
    }
    BIO_set_data(bio, &socket);
    SSL_set_bio(tls, bio, bio);
    while (true) {
      ERR_clear_error();
      int status = SSL_connect(tls);
      if (status == 1) {
        break;
      }
      int error = SSL_get_error(tls, status);
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE) {
        tlsError("TLS handshake failed");
        return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
      }
      int ready = waitForSocket(socket, error == SSL_ERROR_WANT_WRITE, deadline);
      if (ready <= 0) {
        return logTcpResult(__func__, ready == 0 ? VSCP_ERROR_TIMEOUT : VSCP_ERROR_CONNECTION);
      }
    }
  }
  connected = true;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// read
//
// Read data from the transport with a specified timeout. Returns the number of bytes read or a negative error code.
// @param buffer Pointer to the buffer where the read data will be stored.
// @param length Maximum number of bytes to read.
// @param timeout Timeout in milliseconds for the read operation.
// @return Number of bytes read or a negative error code.
// @note If the connection is closed or an error occurs, a negative error code will be returned.
// @see write() for the corresponding write operation.
// @note This function will block until data is available, the timeout expires, or an error occurs.
// @note The timeout is specified in milliseconds.
// @note The buffer must be large enough to hold the requested number of bytes.
// @note The function will return immediately if the connection is closed.
// @note If a TLS connection is used, this function will handle the TLS handshake and decryption internally.
// @note The function will internally manage the socket readiness and TLS state as needed.
// @note The function will return the number of bytes read, or a negative error code if an error occurs.
// @note The function will return 0 if the timeout expires before any data is read.
// @note The function will return -VSCP_ERROR_STOPPED if the connection is closed or an error occurs.
// @note The function will return -VSCP_ERROR_TIMEOUT if the timeout expires before any data is read.
// @note The function will return -VSCP_ERROR_CONNECTION if a connection error occurs.
// @note The function will return -VSCP_ERROR_WOULD_BLOCK if the operation would block and the timeout has not yet expired.
// @note The function will return -VSCP_ERROR_INTERNAL if an internal error occurs.
// @note The function will return -VSCP_ERROR_INVALID_ARGUMENT if an invalid argument is provided.
// @note The function will return -VSCP_ERROR_NOT_IMPLEMENTED if the operation is not implemented.
// @note The function will return -VSCP_ERROR_IO if an I/O error occurs.
// @note The function will return -VSCP_ERROR_UNKNOWN if an unknown error occurs.
// @note The function will return -VSCP_ERROR_TLS if a TLS error occurs.
// @note The function will return -VSCP_ERROR_ABORTED if the operation is aborted.
// @note The function will return -VSCP_ERROR_INTERRUPTED if the operation is interrupted by a signal.
// @note The function will return -VSCP_ERROR_AGAIN if the operation should be retried.
// @note The function will return -VSCP_ERROR_WOULDBLOCK if the operation would block and the timeout has not yet expired.
// @note The function will return -VSCP_ERROR_INVALID_STATE if the connection is not in a valid state for reading.
// @note The function will return -VSCP_ERROR_NOT_CONNECTED if the connection is not established.
// @note The function will return -VSCP_ERROR_SHUTDOWN if the connection is being shut down.
// @note The function will return -VSCP_ERROR_WOULDBLOCK if the operation would block and the timeout has not yet expired.
// @note The function will return -VSCP_ERROR_UNKNOWN if an unknown error occurs.
//

int
VscpRemoteTcpIf::Transport::read(void *buffer, size_t length, int timeout)
{
  logTcpEntry(__func__);
  if (!connected) {
    return logTcpResult(__func__, -VSCP_ERROR_STOPPED, true);
  }
  if (timeout < 0) {
    timeout = TCPIP_DEFAULT_RESPONSE_TIMEOUT;
  }
  const auto deadline = Clock::now() + std::chrono::milliseconds(timeout);
  int size = static_cast<int>((std::min<size_t>)(length, INT_MAX));
  while (true) {
    bool waitWrite = false;
    if (tls) {
      std::lock_guard<std::mutex> lock(tlsMutex);
      ERR_clear_error();
      int count = SSL_read(tls, buffer, size);
      if (count > 0) {
        return count;
      }
      int error = SSL_get_error(tls, count);
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE) {
        if (error != SSL_ERROR_ZERO_RETURN) {
          tlsError("TLS read failed");
        }
        connected = false;
        return logTcpResult(__func__, -VSCP_ERROR_STOPPED, true);
      }
      waitWrite = error == SSL_ERROR_WANT_WRITE;
    }
    else {
      int count = socketRead(socket, static_cast<char *>(buffer), size);
      if (count > 0) {
        return count;
      }
      if (!count || !socketWouldBlock(socketError())) {
        connected = false;
        return logTcpResult(__func__, -VSCP_ERROR_STOPPED, true);
      }
    }
    int ready = waitForSocket(socket, waitWrite, deadline);
    if (ready == 0) {
      return 0;
    }
    if (ready < 0) {
      connected = false;
      return logTcpResult(__func__, -VSCP_ERROR_STOPPED, true);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// write
//
// @param buffer Pointer to the data to be written.
// @param length Number of bytes to write.
// @param timeout Timeout in milliseconds for the write operation.
// @note The function will return -VSCP_ERROR_TIMEOUT if the write operation times out.
// @note The function will return -VSCP_ERROR_STOPPED if the connection is stopped.
// @note The function will return -VSCP_ERROR_SUCCESS if the write operation succeeds.
// @note The function will return -VSCP_ERROR_UNKNOWN if an unknown error occurs.
// 

int
VscpRemoteTcpIf::Transport::write(const void *buffer, size_t length, uint32_t timeout)
{
  logTcpEntry(__func__);
  if (!connected) {
    return logTcpResult(__func__, -VSCP_ERROR_STOPPED);
  }
  const auto deadline = Clock::now() + std::chrono::milliseconds(timeout);
  const char *data = static_cast<const char *>(buffer);
  size_t written = 0;
  while (written < length) {
    int size = static_cast<int>((std::min<size_t>)(length - written, INT_MAX));
    bool waitWrite = true;
    if (tls) {
      std::lock_guard<std::mutex> lock(tlsMutex);
      ERR_clear_error();
      int count = SSL_write(tls, data + written, size);
      if (count > 0) {
        written += static_cast<size_t>(count);
        continue;
      }
      int error = SSL_get_error(tls, count);
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE) {
        tlsError("TLS write failed");
        connected = false;
        return logTcpResult(__func__, -VSCP_ERROR_STOPPED);
      }
      waitWrite = error == SSL_ERROR_WANT_WRITE;
    }
    else {
      int count = socketWrite(socket, data + written, size);
      if (count > 0) {
        written += static_cast<size_t>(count);
        continue;
      }
      if (!count || !socketWouldBlock(socketError())) {
        connected = false;
        return logTcpResult(__func__, -VSCP_ERROR_STOPPED);
      }
    }
    int ready = waitForSocket(socket, waitWrite, deadline);
    if (ready <= 0) {
      connected = false;
      return logTcpResult(__func__, ready == 0 ? -VSCP_ERROR_TIMEOUT : -VSCP_ERROR_STOPPED);
    }
  }
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//
// Returns whether a transport exists and is marked as connected.
// @return true if a transport exists and is connected, false otherwise.

bool
VscpRemoteTcpIf::isConnected(void)
{
  logTcpEntry(__func__);
  auto transport = std::atomic_load(&m_transport);
  return transport && transport->connected;
}

///////////////////////////////////////////////////////////////////////////////
// transportClose
//
// Releases the transport and its resources, if any.

void
VscpRemoteTcpIf::transportClose()
{
  logTcpEntry(__func__);
  stopSingleChannelReader();
  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    if (m_singleChannelReaderError == VSCP_ERROR_SUCCESS) {
      m_singleChannelReaderError = VSCP_ERROR_STOPPED;
    }
    if (m_singleChannelCommandPending) {
      m_singleChannelCommandPending = false;
      m_singleChannelCommandComplete = true;
      m_singleChannelCommandResult = m_singleChannelReaderError;
    }
  }
  m_singleChannelCondition.notify_all();
  std::atomic_store(&m_transport, std::shared_ptr<Transport>());
}

///////////////////////////////////////////////////////////////////////////////
// transportRead
//
// Reads data from the transport into the provided buffer.
// @param buffer Pointer to the buffer where the read data will be stored.
// @param length Maximum number of bytes to read.
// @param timeout Timeout for the read operation in milliseconds.
// @return Number of bytes read on success, or a negative error code on failure.
//

int
VscpRemoteTcpIf::transportRead(void *buffer, size_t length, int timeout)
{
  logTcpEntry(__func__);
  auto transport = std::atomic_load(&m_transport);
  return logTcpReadResult(__func__, transport ? transport->read(buffer, length, timeout) : -VSCP_ERROR_STOPPED);
}

///////////////////////////////////////////////////////////////////////////////
// transportWrite
//
// Writes data to the transport from the provided buffer.
// @param buffer Pointer to the buffer containing the data to write.
// @param length Number of bytes to write.
// @param timeout Timeout for the write operation in milliseconds.
// @return VSCP_ERROR_SUCCESS on success, or a negative error code on failure.
// @note If the write operation fails, the transport will be closed to maintain a consistent state.
//

int
VscpRemoteTcpIf::transportWrite(const void *buffer, size_t length, uint32_t timeout)
{
  logTcpEntry(__func__);
  auto transport = std::atomic_load(&m_transport);
  int result = transport ? transport->write(buffer, length, timeout) : -VSCP_ERROR_STOPPED;
  if (result != VSCP_ERROR_SUCCESS)
    transportClose();
  return logTcpResult(__func__, result);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VscpRemoteTcpIf::VscpRemoteTcpIf()
{
  logTcpEntry(__func__);
  m_lastResponseTime = 0;

  m_bTLS        = false;
  m_tlsMode     = tls_mode::auto_select;
  m_bVerifyPeer = true;

  m_bModeReceiveLoop     = false;
  m_bSingleChannelReceiveLoop = false;
  m_bBinaryMode          = false;
  m_singleChannelReaderStop = false;
  m_singleChannelCommandPending = false;
  m_singleChannelCommandComplete = false;
  m_singleChannelCommandResult = VSCP_ERROR_SUCCESS;
  m_singleChannelReaderError = VSCP_ERROR_SUCCESS;
  m_connectionTimeOut    = TCPIP_DEFAULT_CONNECT_TIMEOUT_SECONDS;
  m_responseTimeOut      = TCPIP_DEFAULT_RESPONSE_TIMEOUT;
  m_innerResponseTimeout = TCPIP_DEFAULT_INNER_RESPONSE_TIMEOUT;

  // Init. register read parameters
  m_registerOpErrorTimeout  = TCPIP_REGISTER_READ_ERROR_TIMEOUT;
  m_registerOpResendTimeout = TCPIP_REGISTER_READ_RESEND_TIMEOUT;
  m_registerOpMaxRetries    = TCPIP_REGISTER_READ_MAX_TRIES;

  // Set default version info
  m_version_year   = VSCPD_MAJOR_VERSION;
  m_version_month   = VSCPD_MINOR_VERSION;
  m_version_patch = VSCPD_PATCH_VERSION;
}

VscpRemoteTcpIf::~VscpRemoteTcpIf()
{
  logTcpEntry(__func__);
  doCmdClose();
}

bool
VscpRemoteTcpIf::isSingleChannelCommandAllowed(const std::string &command) const
{
  logTcpEntry(__func__);
  if (command.find_first_of("\r\n") != std::string::npos) {
    return false;
  }

  if (command == "NOOP" || command == "CLRA" || command == "QUITLOOP") {
    return true;
  }

  return command.size() > 5 && command.compare(0, 4, "SEND") == 0 &&
         std::isspace(static_cast<unsigned char>(command[4]));
}

int
VscpRemoteTcpIf::startSingleChannelReader(void)
{
  logTcpEntry(__func__);
  if (m_singleChannelReaderThread.joinable()) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::deque<std::string> bufferedLines;
  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_singleChannelReaderStop = false;
    m_singleChannelReaderError = VSCP_ERROR_SUCCESS;
    m_singleChannelCommandPending = false;
    m_singleChannelCommandComplete = false;
    m_singleChannelCommandResult = VSCP_ERROR_SUCCESS;
    m_singleChannelCommandResponse.clear();
    m_singleChannelInputBuffer = std::move(m_strResponse);
    m_strResponse.clear();
    bufferedLines = std::move(m_inputStrArray);
    m_inputStrArray.clear();
  }

  for (const auto &line : bufferedLines) {
    processSingleChannelLine(line);
  }

  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    if (m_singleChannelReaderError != VSCP_ERROR_SUCCESS) {
      return logTcpResult(__func__, m_singleChannelReaderError);
    }
  }

  try {
    m_singleChannelReaderThread = std::thread(&VscpRemoteTcpIf::singleChannelReaderLoop, this);
  }
  catch (const std::system_error &error) {
    transportError("%s: cannot start single-channel reader: %s", __func__, error.what());
    return VSCP_ERROR_ERROR;
  }

  return VSCP_ERROR_SUCCESS;
}

void
VscpRemoteTcpIf::stopSingleChannelReader(void)
{
  logTcpEntry(__func__);
  m_singleChannelReaderStop = true;
  m_singleChannelCondition.notify_all();
  if (m_singleChannelReaderThread.joinable() &&
      m_singleChannelReaderThread.get_id() != std::this_thread::get_id()) {
    m_singleChannelReaderThread.join();
  }
}

void
VscpRemoteTcpIf::processSingleChannelLine(const std::string &line)
{
  logTcpEntry(__func__);
  std::string responseLine = line;
  vscp_trim(responseLine);
  if (responseLine.empty() || responseLine == "+OK") {
    return;
  }

  const bool successResponse = responseLine.compare(0, 4, "+OK ") == 0;
  const bool errorResponse = responseLine.compare(0, 3, "-OK") == 0 ||
                             responseLine.compare(0, 4, "+ERR") == 0;
  if (successResponse || errorResponse) {
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      if (m_singleChannelCommandPending) {
        m_singleChannelCommandResponse += responseLine + "\r\n";
        m_singleChannelCommandPending = false;
        m_singleChannelCommandComplete = true;
        m_singleChannelCommandResult = successResponse ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
      }
    }
    m_singleChannelCondition.notify_all();
    return;
  }

  vscpEvent event{};
  if (vscp_convertStringToEvent(&event, responseLine)) {
    vscp_deleteEvent(&event);
    bool dropped = false;
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      if (m_singleChannelEventQueue.size() < maxSingleChannelEvents) {
        m_singleChannelEventQueue.push_back(std::move(responseLine));
      }
      else {
        dropped = true;
      }
    }
    if (dropped) {
      transportError("%s: receive queue full (%zu events). Event dropped: %s",
                     __func__, maxSingleChannelEvents, responseLine.c_str());
    }
    m_singleChannelCondition.notify_all();
    return;
  }
  vscp_deleteEvent(&event);

  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_singleChannelReaderError = VSCP_ERROR_READ;
    m_singleChannelReaderStop = true;
    if (m_singleChannelCommandPending) {
      m_singleChannelCommandPending = false;
      m_singleChannelCommandComplete = true;
      m_singleChannelCommandResult = m_singleChannelReaderError;
    }
  }
  transportError("%s: invalid line in single-channel receive loop", __func__);
  m_singleChannelCondition.notify_all();
}

void
VscpRemoteTcpIf::singleChannelReaderLoop(void)
{
  logTcpEntry(__func__);
  char buffer[8192];
  constexpr int readTimeout = 50;

  while (!m_singleChannelReaderStop) {
    int nRead = transportRead(buffer, sizeof(buffer), readTimeout);
    if (nRead == -VSCP_ERROR_TIMEOUT || nRead == 0) {
      continue;
    }
    if (nRead < 0) {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      if (m_singleChannelReaderError == VSCP_ERROR_SUCCESS) {
        m_singleChannelReaderError =
          nRead == -VSCP_ERROR_STOPPED ? VSCP_ERROR_STOPPED : VSCP_ERROR_ERROR;
      }
      if (m_singleChannelCommandPending) {
        m_singleChannelCommandPending = false;
        m_singleChannelCommandComplete = true;
        m_singleChannelCommandResult = m_singleChannelReaderError;
      }
      m_singleChannelCondition.notify_all();
      break;
    }

    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      m_lastResponseTime = vscp_getMsTimeStamp();
    }
    m_singleChannelInputBuffer.append(buffer, static_cast<size_t>(nRead));
    size_t newline;
    while ((newline = m_singleChannelInputBuffer.find('\n')) != std::string::npos) {
      std::string line = m_singleChannelInputBuffer.substr(0, newline);
      m_singleChannelInputBuffer.erase(0, newline + 1);
      processSingleChannelLine(line);
      if (m_singleChannelReaderStop) {
        break;
      }
    }

    if (m_singleChannelInputBuffer.size() > 65536) {
      {
        std::lock_guard<std::mutex> lock(m_singleChannelMutex);
        m_singleChannelReaderError = VSCP_ERROR_INVALID_FRAME;
        m_singleChannelReaderStop = true;
        if (m_singleChannelCommandPending) {
          m_singleChannelCommandPending = false;
          m_singleChannelCommandComplete = true;
          m_singleChannelCommandResult = m_singleChannelReaderError;
        }
      }
      transportError("%s: oversized line in single-channel receive loop", __func__);
      m_singleChannelCondition.notify_all();
    }
  }

  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    if (m_singleChannelCommandPending) {
      m_singleChannelCommandPending = false;
      m_singleChannelCommandComplete = true;
      m_singleChannelCommandResult =
        m_singleChannelReaderError == VSCP_ERROR_SUCCESS ? VSCP_ERROR_STOPPED : m_singleChannelReaderError;
    }
  }
  m_singleChannelCondition.notify_all();
}

int
VscpRemoteTcpIf::doSingleChannelCommand(const std::string &cmd, const std::string &command)
{
  logTcpEntry(__func__);
  if (!isSingleChannelCommandAllowed(command)) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  size_t terminator = cmd.find_first_of("\r\n");
  if (terminator == std::string::npos ||
      cmd.find_first_not_of("\r\n \t", terminator) != std::string::npos) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::lock_guard<std::mutex> commandLock(m_singleChannelCommandMutex);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    if (m_singleChannelReaderError != VSCP_ERROR_SUCCESS ||
        m_singleChannelReaderStop || !m_singleChannelReaderThread.joinable()) {
      return logTcpResult(__func__,
                          m_singleChannelReaderError == VSCP_ERROR_SUCCESS ?
                            VSCP_ERROR_STOPPED : m_singleChannelReaderError);
    }
    m_singleChannelCommandResponse.clear();
    m_singleChannelCommandComplete = false;
    m_singleChannelCommandPending = true;
  }

  if (VSCP_ERROR_SUCCESS != transportWrite(cmd.data(), cmd.length(), m_responseTimeOut)) {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_singleChannelCommandPending = false;
    m_singleChannelCommandComplete = false;
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  std::string response;
  int result = VSCP_ERROR_SUCCESS;
  {
    std::unique_lock<std::mutex> lock(m_singleChannelMutex);
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(m_responseTimeOut);
    if (!m_singleChannelCondition.wait_until(lock, deadline, [this]() {
          return m_singleChannelCommandComplete ||
                 m_singleChannelReaderError != VSCP_ERROR_SUCCESS ||
                 m_singleChannelReaderStop;
        })) {
      m_singleChannelCommandPending = false;
      m_singleChannelCommandComplete = false;
      m_singleChannelReaderError = VSCP_ERROR_TIMEOUT;
      m_singleChannelReaderStop = true;
      result = VSCP_ERROR_TIMEOUT;
    }
    else if (m_singleChannelCommandComplete) {
      result = m_singleChannelCommandResult;
      response = std::move(m_singleChannelCommandResponse);
      m_singleChannelCommandComplete = false;
    }
    else {
      result = m_singleChannelReaderError == VSCP_ERROR_SUCCESS ?
                 VSCP_ERROR_STOPPED : m_singleChannelReaderError;
    }
  }

  if (result == VSCP_ERROR_TIMEOUT) {
    m_singleChannelCondition.notify_all();
    stopSingleChannelReader();
    transportClose();
    m_bModeReceiveLoop = false;
    return logTcpResult(__func__, result);
  }

  if (!response.empty()) {
    m_strResponse = std::move(response);
    m_inputStrArray.clear();
    addInputStringArrayFromReply();
  }

  return logTcpResult(__func__, result);
}

///////////////////////////////////////////////////////////////////////////////
// checkReturnValue
//

bool
VscpRemoteTcpIf::checkReturnValue(bool bClear)
{
  logTcpEntry(__func__);
  return checkReturnValueForCommand(bClear, false);
}

bool
VscpRemoteTcpIf::checkReturnValueForCommand(bool bClear, bool routineReceive)
{
  logTcpEntry(__func__);
  if (m_bBinaryMode) {
    transportError("%s: text responses are unavailable in binary mode", __func__);
    return false;
  }
  bool rv = false;
  char buf[0x2000];

  if (bClear) {
    doClrInputQueue();
  }

  uint32_t start = vscp_getMsTimeStamp();
  while (((vscp_getMsTimeStamp() - start) < m_responseTimeOut)) {

    memset(buf, 0, sizeof(buf));
    uint32_t elapsed = vscp_getMsTimeStamp() - start;
    if (elapsed >= m_responseTimeOut)
      break;
    int remaining = static_cast<int>(std::min<uint32_t>(m_responseTimeOut - elapsed,
                                                       (std::numeric_limits<int>::max)()));
    int nRead = transportRead(buf, sizeof(buf) - 1,
                             m_innerResponseTimeout < 0 ? remaining : (std::min)(m_innerResponseTimeout, remaining));
    if (nRead < 0) {
      if (-VSCP_ERROR_TIMEOUT == nRead) {
        // VSCP_ERROR_TIMEOUT;
        rv = false;
        break;
      }
      else if (-VSCP_ERROR_STOPPED == nRead) {
        // VSCP_ERROR_STOPPED;
        rv = false;
        break;
      }
      break;
    }
    else if (nRead > 0) {
      m_lastResponseTime = vscp_getMsTimeStamp(); // Save last response time
      m_strResponse += std::string(buf, nRead);
      if (m_strResponse.length() > maxCommandResponse) {
        transportError("%s: oversized server response (%zu bytes)", __func__, m_strResponse.length());
        m_strResponse.clear();
        rv = false;
        break;
      }
    }

    // if err abort
    if (m_strResponse.npos != m_strResponse.find("+ERR")) {
      break;
    }

    // Check if we are done
    // A line with "+OK bla bla bla bla bla bla bla bla \r\n"
    // It can be hacked up.
    if (m_strResponse.npos != m_strResponse.find("+OK")) {

      // We check only for "\n" as it also could have been
      // broken up, even if rare
      if (NULL != strstr(buf, "\n")) {
        rv = true;
        break;
      }
    }

  } // while

  if (!rv) {
    if (routineReceive && isEmptyReceiveResponse(m_strResponse)) {
      logTcpDebug(__func__, "receive queue is empty");
    }
    else {
      transportError("%s: no successful server response", __func__);
    }
  }
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  loopRead
//

int
VscpRemoteTcpIf::rcvloopRead(int timeout)
{
  logTcpEntry(__func__);
  if (m_bBinaryMode) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }
  if (m_bModeReceiveLoop && m_bSingleChannelReceiveLoop) {
    std::unique_lock<std::mutex> lock(m_singleChannelMutex);
    auto ready = [this]() {
      return !m_singleChannelEventQueue.empty() ||
             m_singleChannelCommandComplete ||
             m_singleChannelReaderError != VSCP_ERROR_SUCCESS ||
             m_singleChannelReaderStop;
    };
    if (timeout < 0) {
      m_singleChannelCondition.wait(lock, ready);
    }
    else if (!m_singleChannelCondition.wait_for(lock, std::chrono::milliseconds(timeout), ready)) {
      return logTcpResult(__func__, VSCP_ERROR_TIMEOUT, true);
    }
    if (m_singleChannelReaderError != VSCP_ERROR_SUCCESS) {
      return logTcpResult(__func__, m_singleChannelReaderError, true);
    }
    if (m_singleChannelReaderStop) {
      return logTcpResult(__func__, VSCP_ERROR_STOPPED, true);
    }
    return VSCP_ERROR_SUCCESS;
  }

  // bool rv = false;
  char buf[8192];

  memset(buf, 0, sizeof(buf));
  int nRead = transportRead(buf, sizeof(buf), timeout);

  if (nRead < 0) {
    if (-VSCP_ERROR_TIMEOUT == nRead) {
      return logTcpResult(__func__, VSCP_ERROR_TIMEOUT, true);
    }
    else if (-VSCP_ERROR_STOPPED == nRead) {
      return logTcpResult(__func__, VSCP_ERROR_STOPPED, true);
    }
    return logTcpResult(__func__, VSCP_ERROR_ERROR, true);
  }
  else if (nRead > 0) {
    m_strResponse += std::string(buf, nRead);
    addInputStringArrayFromReply();
  }
  else {
    return logTcpResult(__func__, VSCP_ERROR_TIMEOUT, true);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  doCommand
//

int
VscpRemoteTcpIf::doCommand(const char *cmd)
{
  logTcpEntry(__func__);
  if (nullptr == cmd) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }
  std::string strCmd = cmd;
  int result = doCommand(strCmd);
  if (result == VSCP_ERROR_ERROR && isReceiveCommand(strCmd) && isEmptyReceiveResponse(m_inputStrArray)) {
    logTcpDebug(__func__, "receive queue is empty");
    return result;
  }
  return logTcpResult(__func__, result);
}

int
VscpRemoteTcpIf::doCommand(const std::string &cmd)
{
  logTcpEntry(__func__);
  if (m_bBinaryMode) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }
  std::string command = cmd;
  vscp_trim(command);
  vscp_makeUpper(command);
  if (command == "BINARY") {
    return logTcpResult(__func__, doCmdBinary());
  }
  if (m_bModeReceiveLoop && m_bSingleChannelReceiveLoop) {
    return doSingleChannelCommand(cmd, command);
  }
  bool ret = false;

  doClrInputQueue();

  // Make sure we are connected
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  if (VSCP_ERROR_SUCCESS != transportWrite(cmd.data(), cmd.length(), m_responseTimeOut)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  bool routineReceive = isReceiveCommand(command);
  ret = checkReturnValueForCommand(true, routineReceive);
  bool emptyReceive = !ret && routineReceive && isEmptyReceiveResponse(m_strResponse);
  addInputStringArrayFromReply();

  if (!ret) {
    if (emptyReceive) {
      logTcpDebug(__func__, "receive queue is empty");
      return VSCP_ERROR_ERROR;
    }
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  return VSCP_ERROR_SUCCESS;
}

int
VscpRemoteTcpIf::doCmdBinary(void)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }
  if (m_bBinaryMode) {
    return VSCP_ERROR_SUCCESS;
  }
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  crcInit();
  doClrInputQueue();
  int writeResult = transportWrite("BINARY\r\n", 8, m_responseTimeOut);
  if (writeResult != VSCP_ERROR_SUCCESS) {
    return logTcpResult(__func__, writeResult < 0 ? -writeResult : writeResult);
  }

  // Read exactly through the response line: binary data may immediately
  // follow it in the same TCP packet.
  std::string response;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(m_responseTimeOut);
  while (response.size() < 8192) {
    const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
      deadline - std::chrono::steady_clock::now()).count();
    if (remaining <= 0) {
      transportClose(); // The peer may already have switched modes.
      return logTcpResult(__func__, VSCP_ERROR_TIMEOUT);
    }
    char byte;
    int count = transportRead(&byte, 1, static_cast<int>((std::min<int64_t>)(remaining, INT_MAX)));
    if (count < 0) {
      transportClose();
      return logTcpResult(__func__, VSCP_ERROR_STOPPED);
    }
    if (!count) {
      continue;
    }
    m_lastResponseTime = vscp_getMsTimeStamp();
    response += byte;
    if (byte == '\n') {
      vscp_trim(response);
      if (response == "+OK" || response.compare(0, 4, "+OK ") == 0) {
        m_bBinaryMode = true;
        m_binaryInput.clear();
        return VSCP_ERROR_SUCCESS;
      }
      if (response.compare(0, 3, "-OK") == 0 || response.compare(0, 4, "+ERR") == 0) {
        return logTcpResult(__func__, VSCP_ERROR_ERROR);
      }
      transportClose();
      return logTcpResult(__func__, VSCP_ERROR_INVALID_FRAME);
    }
  }
  transportClose();
  return logTcpResult(__func__, VSCP_ERROR_INVALID_FRAME);
}

int
VscpRemoteTcpIf::sendBinaryFrame(const std::vector<uint8_t> &frame)
{
  logTcpEntry(__func__);
  std::lock_guard<std::mutex> lock(m_binaryWriteMutex);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }
  if (!m_bBinaryMode || frame.empty() || frame.size() > UINT16_MAX) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::vector<uint8_t> packet;
  packet.reserve(frame.size() + 2);
  packet.push_back(static_cast<uint8_t>(frame.size() >> 8));
  packet.push_back(static_cast<uint8_t>(frame.size()));
  packet.insert(packet.end(), frame.begin(), frame.end());
  int result = transportWrite(packet.data(), packet.size(), m_responseTimeOut);
  return logTcpResult(__func__, result < 0 ? -result : result);
}

int
VscpRemoteTcpIf::receiveBinaryFrame(std::vector<uint8_t> &frame, uint32_t timeout)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION, true);
  }
  if (!m_bBinaryMode) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
  while (true) {
    size_t needed = 2;
    if (m_binaryInput.size() >= 2) {
      const size_t length = (static_cast<size_t>(m_binaryInput[0]) << 8) | m_binaryInput[1];
      if (!length) {
        transportClose();
        m_binaryInput.clear();
        return logTcpResult(__func__, VSCP_ERROR_INVALID_FRAME, true);
      }
      needed += length;
      if (m_binaryInput.size() == needed) {
        frame.assign(m_binaryInput.begin() + 2, m_binaryInput.end());
        m_binaryInput.clear();
        return VSCP_ERROR_SUCCESS;
      }
    }
    const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
      deadline - std::chrono::steady_clock::now()).count();
    uint8_t buffer[8192];
    int count = transportRead(buffer, (std::min)(sizeof(buffer), needed - m_binaryInput.size()),
                              static_cast<int>((std::max<int64_t>)(0, (std::min<int64_t>)(remaining, INT_MAX))));
    if (count < 0) {
      transportClose();
      m_binaryInput.clear();
      return logTcpResult(__func__, VSCP_ERROR_STOPPED, true);
    }
    if (count > 0) {
      m_binaryInput.insert(m_binaryInput.end(), buffer, buffer + count);
      m_lastResponseTime = vscp_getMsTimeStamp();
    }
    else if (std::chrono::steady_clock::now() >= deadline) {
      return logTcpResult(__func__, VSCP_ERROR_TIMEOUT, true);
    }
  }
}

int
VscpRemoteTcpIf::processBinaryEventFrame(const std::vector<uint8_t> &frame,
                                         vscpEvent *event,
                                         bool acknowledge)
{
  logTcpEntry(__func__);
  if (nullptr == event) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }
  if (frame.size() < 38 || (frame[0] & 0x0f) || (frame[0] >> 4) > 1) {
    return logTcpResult(__func__, VSCP_ERROR_INVALID_FRAME, true);
  }
  const size_t sizeData = (static_cast<size_t>(frame[34]) << 8) | frame[35];
  if (sizeData > VSCP_MAX_DATA || frame.size() != sizeData + 38 ||
      !vscp_getEventFromFrame(event, frame.data(), frame.size())) {
    return logTcpResult(__func__, VSCP_ERROR_INVALID_FRAME, true);
  }
  if (!acknowledge) {
    return VSCP_ERROR_SUCCESS;
  }

  const uint8_t argument[] = {0, 0, frame[1], frame[2], frame[frame.size() - 2], frame.back()};
  std::vector<uint8_t> confirmation(13);
  int result = vscp_writeReplyToFrame(confirmation.data(), confirmation.size(), 0xffff,
                                      VSCP_ERROR_SUCCESS, argument, sizeof(argument));
  if (VSCP_ERROR_SUCCESS == result) {
    result = sendBinaryFrame(confirmation);
  }
  if (VSCP_ERROR_SUCCESS != result) {
    vscp_deleteEvent(event);
    event->sizeData = 0;
  }
  return logTcpResult(__func__, result, true);
}

int
VscpRemoteTcpIf::receiveBinaryEvent(vscpEvent *event, uint32_t timeout)
{
  logTcpEntry(__func__);
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
  while (true) {
    const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
      deadline - std::chrono::steady_clock::now()).count();
    std::vector<uint8_t> frame;
    int result = receiveBinaryFrame(frame, static_cast<uint32_t>((std::max<int64_t>)(0, remaining)));
    if (VSCP_ERROR_SUCCESS != result) {
      return logTcpResult(__func__, result, true);
    }

    // Raw frame APIs must be used for encrypted frames and command replies.
    if ((frame[0] & 0x0f) || frame.size() < 5) {
      return logTcpResult(__func__, VSCP_ERROR_INVALID_FRAME, true);
    }
    const uint8_t type = frame[0] >> 4;
    if (type == 15 && frame.size() == 13 && frame[1] == 0xff && frame[2] == 0xff) {
      if (crcFast(frame.data() + 1, static_cast<int>(frame.size() - 1))) {
        return logTcpResult(__func__, VSCP_ERROR_INVALID_FRAME, true);
      }
      const int error = (static_cast<int>(frame[3]) << 8) | frame[4];
      if (error) {
        return logTcpResult(__func__, error, true);
      }
      if (std::chrono::steady_clock::now() >= deadline) {
        return logTcpResult(__func__, VSCP_ERROR_TIMEOUT, true);
      }
      continue;
    }
    return logTcpResult(__func__, processBinaryEventFrame(frame, event), true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// getInputQueueCount
//

size_t
VscpRemoteTcpIf::getInputQueueCount(void)
{
  logTcpEntry(__func__);
  return m_inputStrArray.size();
}

///////////////////////////////////////////////////////////////////////////////
// doClrInputQueue
//

void
VscpRemoteTcpIf::doClrInputQueue(void)
{
  logTcpEntry(__func__);
  m_strResponse.clear();
  m_inputStrArray.clear();
}

///////////////////////////////////////////////////////////////////////////////
// addInputStringArrayFromReply
//

size_t
VscpRemoteTcpIf::addInputStringArrayFromReply(bool bClear)
{
  logTcpEntry(__func__);
  std::string tempStr;

  if (bClear) {
    m_inputStrArray.clear();
  }

  if (!m_strResponse.length()) {
    return 0;
  }

  // If readBuffer have a "\r\n" pair at the end it just contains full
  // reply rows. If not we need to leave ending part after "\r\n" pair
  // in the read buffer.

  // Get rest string that should not be handled now
  size_t fpos = m_strResponse.find_last_of('\n');
  if (std::string::npos == fpos) {
    tempStr = m_strResponse;                  // first CR not found at end of input
  }
  else {
    tempStr = m_strResponse.substr(fpos + 1); // "+OK - Success" or other command response
  }
 
  // Get the string that should be parsed for inclusion in the string array
  std::string strToArray = m_strResponse.substr(0, m_strResponse.length() - tempStr.length());

  // Set the readbuffer to the rest string
  m_strResponse = tempStr;
  if (m_strResponse.length() > maxPartialLine) {
    transportError("%s: oversized line from server discarded (%zu bytes)", __func__, m_strResponse.length());
    m_strResponse.clear();
  }

  // Parse the array string
  std::deque<std::string> tokens;
  vscp_split(tokens, strToArray, "\n");
  while (tokens.size()) {
    // Get line
    std::string str = tokens.front();
    tokens.pop_front();

    // Save line of not just whitespace
    vscp_trim(str);
    if (str.length()) {
      m_inputStrArray.push_back(str);
     }
  }

  return m_inputStrArray.size();
}

///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

int
VscpRemoteTcpIf::doCmdOpen(const std::string &strInterface, uint32_t flags)
{
  logTcpEntry(__func__);
  std::string str;
  std::string strUsername;
  std::string strPassword;
  std::string strHostname;

  TCPIP_UNUSED(flags);

  // Interface can be given as
  //      tcp://host:port;username;password
  //      stcp://host:port;username;password  (SSL/TLS connection)
  //      or
  //      host:port;username;password

  std::deque<std::string> tokens;
  vscp_split(tokens, strInterface, ";");

  // Hostname
  if (tokens.size()) {
    strHostname = tokens.front();
    tokens.pop_front();
    vscp_trim(strHostname);

    // Detect the transport prefix and pass it through to the second overload
    // so that prefix-based TLS auto-detection works correctly there.
    std::string strLower = vscp_lower(strHostname);
    std::string strAfterPrefix;
    if (vscp_startsWith(strLower, "stcp://", &strAfterPrefix)) {
      // Re-attach the canonical stcp:// prefix so the second overload sees it.
      strHostname = "stcp://" + strAfterPrefix;
    } else {
      // Strip tcp:// (plain) prefix; no prefix also means plain.
      std::string prefix = "tcp://";
      vscp_startsWith(strHostname, prefix, &strHostname);
    }
  }

  // Username
  if (tokens.size()) {
    strUsername = tokens.front();
    tokens.pop_front();
  }

  // Password
  if (tokens.size()) {
    strPassword = tokens.front();
    tokens.pop_front();
  }

  return logTcpResult(__func__, doCmdOpen(strHostname, strUsername, strPassword));
}

///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

int
VscpRemoteTcpIf::doCmdOpen(const std::string &strHostname,
                           const std::string &strUsername,
                           const std::string &strPassword)
{
  logTcpEntry(__func__);
  std::string buf;
  std::string str;

  // Username and password are sent as single protocol lines. A line break
  // (or NUL) would end the line early and inject further commands.
  const char forbidden[] = {'\r', '\n', '\0'};
  if (std::string::npos != strUsername.find_first_of(forbidden, 0, sizeof(forbidden)) ||
      std::string::npos != strPassword.find_first_of(forbidden, 0, sizeof(forbidden))) {
    transportError("%s: username or password contains a line break", __func__);
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::string strHost = vscp_lower(strHostname);
  vscp_trim(strHost);

  // Check for stcp:// prefix (secure connection)
  bool bPrefixSecure = false;
  if (vscp_startsWith(strHost, "stcp://", &strHost)) {
    bPrefixSecure = true;
  } else {
    vscp_startsWith(strHost, "tcp://", &strHost); // Remove "tcp://" if there
  }

  bool bSecure = false;
  if (tls_mode::force_tls == m_tlsMode) {
    bSecure = true;
  }
  else if (tls_mode::force_plain == m_tlsMode) {
    bSecure = false;
  }
  else {
    bSecure = bPrefixSecure;
  }
  m_bTLS = bSecure;

  // Bracketed IPv6 endpoints must not be split on every colon.
  size_t separator = strHost.rfind(':');
  if (separator == std::string::npos || separator == 0 || separator + 1 == strHost.size()) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }
  std::string portString = strHost.substr(separator + 1);
  if (portString.find_first_not_of("0123456789") != std::string::npos ||
      portString.size() > 5 || vscp_readStringValue(portString) == 0 ||
      vscp_readStringValue(portString) > 65535) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }
  std::string host = strHost.substr(0, separator);
  if (host.find_first_of("/?#@; \t\r\n") != std::string::npos)
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  if (host.front() == '[') {
    if (host.size() < 3 || host.back() != ']')
      return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }
  else if (host.find(':') != std::string::npos) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }
  doCmdClose();
  auto transport = std::make_shared<Transport>();
  std::atomic_store(&m_transport, transport);
  std::string socketHost = host.front() == '[' ? host.substr(1, host.size() - 2) : host;
  int openResult = transport->open(*this, socketHost, portString);
  if (openResult != VSCP_ERROR_SUCCESS) {
    transportClose();
    return logTcpResult(__func__, openResult);
  }

  logTcpDebug(__func__, "checking server greeting");
  if (!checkReturnValue(true)) {
    transportClose();
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // Username
  str = strUsername;
  vscp_trim(str);

  buf = std::string("USER ") + str + std::string("\r\n");

  if (VSCP_ERROR_SUCCESS != doCommand(buf)) {
    transportClose();
    return logTcpResult(__func__, VSCP_ERROR_USER);
  }

  // Password
  str = strPassword;
  vscp_trim(str);
  buf = std::string("PASS ") + str + std::string("\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(buf)) {
    transportClose();
    return logTcpResult(__func__, VSCP_ERROR_PASSWORD);
  }

  // Get version (for internal versioning)
  uint8_t v1, v2, v3; // Not used - stored by method
  doCmdVersion(&v1, &v2, &v3);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

int
VscpRemoteTcpIf::doCmdOpen(const std::string &strHostname,
                           short port,
                           const std::string &strUsername,
                           const std::string &strPassword)
{
  logTcpEntry(__func__);
  std::string strCombinedHost = strHostname + std::string(":") + vscp_str_format("%d", port);
  return logTcpResult(__func__, doCmdOpen(strCombinedHost, strUsername, strPassword));
}

///////////////////////////////////////////////////////////////////////////////
// close
//

int
VscpRemoteTcpIf::doCmdClose(void)
{
  logTcpEntry(__func__);
  if (isConnected() && !m_bBinaryMode) {

    // If receive loop active - end it
    if (m_bModeReceiveLoop) {
      if (m_bSingleChannelReceiveLoop) {
        doCmdQuitReceiveLoop();
        if (m_bModeReceiveLoop) {
          m_bModeReceiveLoop = false;
          stopSingleChannelReader();
        }
      }
      else {
        doCommand("QUITLOOP\r\n");
      }
    }

    // Try to behave
    doCommand("QUIT\r\n");
  }

  transportClose();
  m_bModeReceiveLoop = false;
  m_bBinaryMode = false;
  m_binaryInput.clear();
  m_inputStrArray.clear();
  m_strResponse.clear();
  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_singleChannelEventQueue.clear();
    m_singleChannelInputBuffer.clear();
    m_singleChannelCommandResponse.clear();
    m_singleChannelCommandPending = false;
    m_singleChannelCommandComplete = false;
    m_singleChannelReaderError = VSCP_ERROR_SUCCESS;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdNOOP
//

int
VscpRemoteTcpIf::doCmdNOOP(void)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // Allow this command in the receive loop only with single-channel mode.
  if (m_bModeReceiveLoop && !m_bSingleChannelReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  return logTcpResult(__func__, doCommand("NOOP\r\n"));
}

///////////////////////////////////////////////////////////////////////////////
// doCmdClear
//

int
VscpRemoteTcpIf::doCmdClear(void)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // Allow this command in the receive loop only with single-channel mode.
  if (m_bModeReceiveLoop && !m_bSingleChannelReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  int result = doCommand("CLRA\r\n");
  if (result == VSCP_ERROR_SUCCESS && m_bModeReceiveLoop && m_bSingleChannelReceiveLoop) {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_singleChannelEventQueue.clear();
  }
  return logTcpResult(__func__, result);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int
VscpRemoteTcpIf::doCmdSend(const vscpEvent *pEvent)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  if (nullptr == pEvent) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  // Allow this command in the receive loop only with single-channel mode.
  if (m_bModeReceiveLoop && !m_bSingleChannelReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::string strBuf, strWrk, strGUID;

  // Must be a valid data pointer if data
  if ((pEvent->sizeData > 0) && (NULL == pEvent->pdata)) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  // Validate datasize
  if (pEvent->sizeData > VSCP_MAX_DATA) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  if (m_bBinaryMode) {
    std::vector<uint8_t> frame(1 + VSCP_BINARY_PACKET_FRAME0_HEADER_LENGTH + pEvent->sizeData + 2);
    if (!vscp_writeEventToFrame(frame.data(), frame.size(), VSCP_ENCRYPTION_NONE, pEvent)) {
      return logTcpResult(__func__, VSCP_ERROR_INVALID_FRAME);
    }
    return logTcpResult(__func__, sendBinaryFrame(frame));
  }

  // send head,class,type,obid,,timestamp,GUID,data1,data2,data3....
  if (!vscp_convertEventToString(strBuf, pEvent)) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  // Note that the caller must delete the event (if it should be deleted)
  
  strBuf = "send " + strBuf;
  strBuf += "\r\n";

  return logTcpResult(__func__, doCommand(strBuf));
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSendEx
//

int
VscpRemoteTcpIf::doCmdSendEx(const vscpEventEx *pEventEx)
{
  logTcpEntry(__func__);
  std::string strBuf, strWrk, strGUID;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // Allow this command in the receive loop only with single-channel mode.
  if (m_bModeReceiveLoop && !m_bSingleChannelReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  if (NULL == pEventEx) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  // Validate datasize
  if (pEventEx->sizeData > VSCP_MAX_DATA) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  if (m_bBinaryMode) {
    vscpEvent event = {};
    if (!vscp_convertEventExToEvent(&event, pEventEx)) {
      return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
    }
    int result = doCmdSend(&event);
    delete[] event.pdata;
    return logTcpResult(__func__, result);
  }

  // send head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
  if (!vscp_convertEventExToString(strBuf, pEventEx)) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  strBuf = "send " + strBuf;
  strBuf += "\r\n";

  return logTcpResult(__func__, doCommand(strBuf));
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSendLevel1
//

int
VscpRemoteTcpIf::doCmdSendLevel1(const canalMsg *pCanalMsg)
{
  logTcpEntry(__func__);
  vscpEventEx event;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // Allow this command in the receive loop only with single-channel mode.
  if (m_bModeReceiveLoop && !m_bSingleChannelReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  if (NULL == pCanalMsg) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  event.vscp_class = (unsigned short) (0x1ff & (pCanalMsg->id >> 16));
  event.vscp_type  = (unsigned char) (0xff & (pCanalMsg->id >> 8));
  event.obid       = pCanalMsg->obid;
  event.timestamp  = pCanalMsg->timestamp;
  event.head       = 0x00;

  memset(event.GUID, 0, 16);

  event.GUID[0] = pCanalMsg->id & 0xff;

  event.sizeData = pCanalMsg->sizeData;
  memcpy(event.data, pCanalMsg->data, pCanalMsg->sizeData);

  return logTcpResult(__func__, doCmdSendEx(&event));
}

///////////////////////////////////////////////////////////////////////////////
// getEventFromLine
//

bool
VscpRemoteTcpIf::getEventFromLine(vscpEvent *pEvent, const std::string &strLine)
{
  logTcpEntry(__func__);
  // Check pointer
  if (NULL == pEvent) {
    transportError("%s: null event pointer", __func__);
    return false;
  }

  // Read event from string
  if (!vscp_convertStringToEvent(pEvent, strLine)) {
    transportError("%s: invalid event data", __func__);
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int
VscpRemoteTcpIf::doCmdReceive(vscpEvent *pEvent)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION, true);
  }

  if (NULL == pEvent) {
    return logTcpResult(__func__, VSCP_ERROR_MEMORY, true);
  }

  if (m_bBinaryMode) {
    return logTcpResult(__func__, receiveBinaryEvent(pEvent, m_responseTimeOut), true);
  }

  // If in receive loop terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  if (VSCP_ERROR_SUCCESS != doCommand("RETR 1\r\n")) {
    if (isEmptyReceiveResponse(m_inputStrArray)) {
      logTcpDebug(__func__, "receive queue is empty");
      return VSCP_ERROR_ERROR;
    }
    return logTcpResult(__func__, VSCP_ERROR_ERROR, true);
  }

  if (!m_inputStrArray.size()) {
    logTcpDebug(__func__, "receive queue is empty");
    return VSCP_ERROR_ERROR;
  }

  if (!getEventFromLine(pEvent, m_inputStrArray[0])) {
    return logTcpResult(__func__, VSCP_ERROR_READ, true);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceiveEx
//

int
VscpRemoteTcpIf::doCmdReceiveEx(vscpEventEx *pEventEx)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION, true);
  }

  if (NULL == pEventEx) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  if (m_bBinaryMode) {
    return logTcpResult(__func__, doCmdBlockingReceive(pEventEx, m_responseTimeOut), true);
  }

  // If in receive loop terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  if (VSCP_ERROR_SUCCESS != doCommand("RETR 1\r\n")) {
    if (isEmptyReceiveResponse(m_inputStrArray)) {
      logTcpDebug(__func__, "receive queue is empty");
      return VSCP_ERROR_ERROR;
    }
    return logTcpResult(__func__, VSCP_ERROR_ERROR, true);
  }

  if (!m_inputStrArray.size()) {
    return logTcpResult(__func__, VSCP_ERROR_RCV_EMPTY, true);
  }

  vscpEvent *pEvent = new vscpEvent;
  if (NULL == pEvent) {
    return logTcpResult(__func__, VSCP_ERROR_MEMORY, true);
  }

  if (!getEventFromLine(pEvent, m_inputStrArray[0])) {
    return logTcpResult(__func__, VSCP_ERROR_READ, true);
  }

  if (!vscp_convertEventToEventEx(pEventEx, pEvent)) {
    vscp_deleteEvent(pEvent);
    return logTcpResult(__func__, VSCP_ERROR_INVALID_FORMAT, true);
  }

  vscp_deleteEvent(pEvent);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceiveLevel1
//

int
VscpRemoteTcpIf::doCmdReceiveLevel1(canalMsg *pCanalMsg)
{
  logTcpEntry(__func__);
  vscpEventEx event;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION, true);
  }

  // Must have a valid pointer
  if (NULL == pCanalMsg) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  // If in receive loop terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  // Fetch event
  int receiveResult = doCmdReceiveEx(&event);
  if (VSCP_ERROR_SUCCESS != receiveResult) {
    if (receiveResult == VSCP_ERROR_RCV_EMPTY || receiveResult == VSCP_ERROR_TIMEOUT ||
        (receiveResult == VSCP_ERROR_ERROR && isEmptyReceiveResponse(m_inputStrArray))) {
      logTcpDebug(__func__, "receive queue is empty or timed out");
      return VSCP_ERROR_GENERIC;
    }
    return logTcpResult(__func__, VSCP_ERROR_GENERIC, true);
  }

  // No CAN or Level I event if data is > 8
  if (event.sizeData > 8) {
    return logTcpResult(__func__, VSCP_ERROR_GENERIC, true);
  }

  pCanalMsg->id = (unsigned long) ((event.head >> 5) << 20) | ((unsigned long) event.vscp_class << 16) |
                  ((unsigned long) event.vscp_type << 8) | event.GUID[15];

  pCanalMsg->obid     = event.obid;
  pCanalMsg->sizeData = (uint8_t) event.sizeData;
  if (pCanalMsg->sizeData) {
    memcpy(pCanalMsg->data, event.data, event.sizeData);
  }

  pCanalMsg->timestamp = event.timestamp;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdEnterReceiveLoop
//

int
VscpRemoteTcpIf::doCmdEnterReceiveLoop(void)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  if (m_bBinaryMode) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  // If in receive loop terminate
  if (m_bModeReceiveLoop) {
    return VSCP_ERROR_SUCCESS;
  }

  // Prevent false timeouts when starting up
  m_lastResponseTime = vscp_getMsTimeStamp();

  if (VSCP_ERROR_SUCCESS != doCommand("RCVLOOP\r\n")) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  m_bModeReceiveLoop = true;
  if (m_bSingleChannelReceiveLoop) {
    int result = startSingleChannelReader();
    if (VSCP_ERROR_SUCCESS != result) {
      m_bModeReceiveLoop = false;
      doCommand("QUITLOOP\r\n");
      return logTcpResult(__func__, result);
    }
  }

  return VSCP_ERROR_SUCCESS;
}

int
VscpRemoteTcpIf::setSingleChannelReceiveLoop(bool enable)
{
  logTcpEntry(__func__);
  if (m_bModeReceiveLoop || m_singleChannelReaderThread.joinable()) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  m_bSingleChannelReceiveLoop = enable;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdQuitReceiveLoop
//

int
VscpRemoteTcpIf::doCmdQuitReceiveLoop(void)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  if (m_bBinaryMode) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  // If **not** in receive loop terminate
  if (!m_bModeReceiveLoop) {
    return VSCP_ERROR_SUCCESS;
  }

  int result = doCommand("QUITLOOP\r\n");
  if (VSCP_ERROR_SUCCESS != result) {
    if (m_bSingleChannelReceiveLoop) {
      bool readerFailed;
      {
        std::lock_guard<std::mutex> lock(m_singleChannelMutex);
        readerFailed = m_singleChannelReaderError != VSCP_ERROR_SUCCESS;
      }
      if (readerFailed) {
        m_bModeReceiveLoop = false;
        stopSingleChannelReader();
        transportClose();
      }
    }
    return logTcpResult(__func__, VSCP_ERROR_TIMEOUT);
  }

  m_bModeReceiveLoop = false;
  if (m_bSingleChannelReceiveLoop) {
    stopSingleChannelReader();
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_singleChannelEventQueue.clear();
    m_singleChannelInputBuffer.clear();
  }
  m_inputStrArray.clear();

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingReceive
//
// Warning! Just works if in receive loop
//

int
VscpRemoteTcpIf::doCmdBlockingReceive(vscpEvent *pEvent, uint32_t mstimeout)
{
  logTcpEntry(__func__);
  char buf[0xffff];
  int nRead;

  // Check pointer
  if (NULL == pEvent) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  // Must be connected
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION, true);
  }

  if (m_bBinaryMode) {
    return logTcpResult(__func__, receiveBinaryEvent(pEvent, mstimeout), true);
  }

  // If **not** in receive loop terminate
  if (!m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  if (m_bSingleChannelReceiveLoop) {
    std::string eventLine;
    {
      std::unique_lock<std::mutex> lock(m_singleChannelMutex);
      auto ready = [this]() {
        return !m_singleChannelEventQueue.empty() ||
               m_singleChannelReaderError != VSCP_ERROR_SUCCESS ||
               m_singleChannelReaderStop ||
               !m_bModeReceiveLoop;
      };
      if (!m_singleChannelCondition.wait_for(lock, std::chrono::milliseconds(mstimeout), ready)) {
        return logTcpResult(__func__, VSCP_ERROR_TIMEOUT, true);
      }

      if (!m_singleChannelEventQueue.empty()) {
        eventLine = std::move(m_singleChannelEventQueue.front());
        m_singleChannelEventQueue.pop_front();
      }
      else if (m_singleChannelReaderError != VSCP_ERROR_SUCCESS) {
        return logTcpResult(__func__, m_singleChannelReaderError, true);
      }
      else {
        return logTcpResult(__func__, VSCP_ERROR_STOPPED, true);
      }
    }

    return logTcpResult(__func__,
                        getEventFromLine(pEvent, eventLine) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_READ,
                        true);
  }

  // If array already contains lines check if one of them is an event
  while (m_inputStrArray.size()) {

    std::string strItem = m_inputStrArray.front(); // Get first reply string
    m_inputStrArray.pop_front();                   // Remove it from the array
    if (!strItem.length()) {
      continue;
    }

    if ((strItem.npos == strItem.find("+OK")) && (strItem.npos == strItem.find("+ERR"))) {

      // This should be an event - Get it
      if (!getEventFromLine(pEvent, strItem)) {
        return logTcpResult(__func__, VSCP_ERROR_READ, true);
      }

      return VSCP_ERROR_SUCCESS;
    }
  }

  uint32_t startTime = vscp_getMsTimeStamp();

  while ((vscp_getMsTimeStamp() - startTime) < mstimeout) {

    uint32_t elapsed = vscp_getMsTimeStamp() - startTime;
    if (elapsed >= mstimeout)
      break;
    int remaining = static_cast<int>(std::min<uint32_t>(mstimeout - elapsed, (std::numeric_limits<int>::max)()));
    if ((nRead = transportRead(buf, sizeof(buf),
                              m_innerResponseTimeout < 0 ? remaining : (std::min)(m_innerResponseTimeout, remaining))) <= 0) {

      if (-VSCP_ERROR_STOPPED == nRead) {
        transportClose();
        return logTcpResult(__func__, VSCP_ERROR_STOPPED, true);
      }
#ifndef WIN32
      usleep(100);
#else
#endif
      continue;
    }

    m_lastResponseTime = vscp_getMsTimeStamp();

    // Save the response
    m_strResponse += std::string(buf, nRead);

    // Fill array
    addInputStringArrayFromReply();

    // Check if one of the items is an event
    while (m_inputStrArray.size()) {

      std::string strItem = m_inputStrArray.front(); // Get first reply string
      m_inputStrArray.pop_front();                   // Remove it from the array
      if (!strItem.length()) {
        continue;
      }

      // Not "+OK" or "+ERR"
      if ((strItem.npos == strItem.find("+OK")) && (strItem.npos == strItem.find("+ERR"))) {

        // This should be an event - Get it
        if (!getEventFromLine(pEvent, strItem)) {
          return logTcpResult(__func__, VSCP_ERROR_READ, true);
        }

        return VSCP_ERROR_SUCCESS;
      }

    } // Lines in array

  } // timeout

  return logTcpResult(__func__, VSCP_ERROR_TIMEOUT, true);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingReceive
//

int
VscpRemoteTcpIf::doCmdBlockingReceive(vscpEventEx *pEventEx, uint32_t timeout)
{
  logTcpEntry(__func__);
  int rv;
  vscpEvent e;

  // Check pointer
  if (NULL == pEventEx) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  // Must be connected
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION, true);
  }

  if (m_bBinaryMode) {
    vscpEvent event = {};
    int result = receiveBinaryEvent(&event, timeout);
    if (VSCP_ERROR_SUCCESS != result) {
      return logTcpResult(__func__, result, true);
    }
    bool converted = vscp_convertEventToEventEx(pEventEx, &event);
    delete[] event.pdata;
    return logTcpResult(__func__, converted ? VSCP_ERROR_SUCCESS : VSCP_ERROR_INVALID_FRAME, true);
  }

  // If not receive loop active terminate
  if (!m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER, true);
  }

  if (VSCP_ERROR_SUCCESS != (rv = doCmdBlockingReceive(&e, timeout))) {
    return logTcpResult(__func__, rv, true);
  }

  // Get event
  pEventEx->head       = e.head;
  pEventEx->vscp_class = e.vscp_class;
  pEventEx->vscp_type  = e.vscp_type;
  pEventEx->obid       = e.obid;
  pEventEx->timestamp  = e.timestamp;
  pEventEx->crc        = e.crc;
  pEventEx->sizeData   = e.sizeData;
  memcpy(pEventEx->GUID, e.GUID, 16);
  if ((NULL != e.pdata) && e.sizeData) {
    memcpy(pEventEx->data, e.pdata, e.sizeData);
    // Don't need the data anymore
    delete[] e.pdata;
    e.pdata = NULL;
  }

  return logTcpResult(__func__, rv, true);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int
VscpRemoteTcpIf::doCmdDataAvailable(void)
{
  logTcpEntry(__func__);
  std::string strLine;
  int nMsg = 0;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  if (VSCP_ERROR_SUCCESS != doCommand("CDTA\r\n")) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  if (!m_inputStrArray.size()) {
    return 0;
  }

  nMsg = atoi((const char *) m_inputStrArray.front().c_str());

  return nMsg;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdState (CANAL)
//

int
VscpRemoteTcpIf::doCmdStatus(canal_status_t *pStatus)
{
  logTcpEntry(__func__);
  std::string strBuf;
  std::string strWrk;
  std::string strLine;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::string strCmd("INFO\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  std::deque<std::string> tokens;
  vscp_split(tokens, m_inputStrArray.front(), ",");
  m_inputStrArray.pop_front();

  // lasterrorcode
  if (tokens.empty()) {
    return logTcpResult(__func__, VSCP_ERROR_GENERIC);
  }
  pStatus->lasterrorcode = vscp_readStringValue(tokens.front());
  tokens.pop_front();

  // lasterrorsubcode
  if (tokens.empty()) {
    return logTcpResult(__func__, VSCP_ERROR_GENERIC);
  }
  pStatus->lasterrorsubcode = vscp_readStringValue(tokens.front());
  tokens.pop_front();

  // lasterrorsubcode
  if (tokens.empty()) {
    return logTcpResult(__func__, VSCP_ERROR_GENERIC);
  }
  strWrk = tokens.front();
  tokens.pop_front();
  // The string comes from the server. Truncate it to the buffer size and
  // always zero terminate.
  memset(pStatus->lasterrorstr, 0, sizeof(pStatus->lasterrorstr));
  strWrk.copy(pStatus->lasterrorstr, sizeof(pStatus->lasterrorstr) - 1);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdStatus (VSCP)
//

int
VscpRemoteTcpIf::doCmdStatus(vscp_status_t *pStatus)
{
  logTcpEntry(__func__);
  if (NULL == pStatus) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  canal_status_t status;
  memset(&status, 0, sizeof(status)); // Defined content also when the call fails
  int rv = doCmdStatus(&status);

  pStatus->channel_status = status.channel_status;
  pStatus->lasterrorcode  = status.lasterrorcode;
  memset(pStatus->lasterrorstr, 0, sizeof(pStatus->lasterrorstr));
  strncpy(pStatus->lasterrorstr, status.lasterrorstr, sizeof(pStatus->lasterrorstr) - 1);
  pStatus->lasterrorsubcode = status.lasterrorsubcode;

  return logTcpResult(__func__, rv);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int
VscpRemoteTcpIf::doCmdStatistics(vscp_statistics_t *pStatistics)
{
  logTcpEntry(__func__);
  std::string strBuf;
  std::string strWrk;
  std::string strLine;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::string strCmd("STAT\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  std::deque<std::string> tokens;
  vscp_split(tokens, m_inputStrArray.front(), ",");

  // Undefined - Bus off for CAN, no meaning for VSCP L2
  pStatistics->x = 0;
  if (!tokens.empty()) {
    pStatistics->x = vscp_readStringValue(tokens.front());
  }

  // Undefined - Bus warnings for CAN, no meaning for VSCP L2
  pStatistics->y = 0;
  if (!tokens.empty()) {
    pStatistics->y = vscp_readStringValue(tokens.front());
  }

  // Undefined - Not defined on server side
  pStatistics->z = 0;

  // Overruns
  pStatistics->cntOverruns = 0;
  if (!tokens.empty()) {
    pStatistics->cntOverruns = vscp_readStringValue(tokens.front());
  }

  // Received data
  pStatistics->cntReceiveData = 0;
  if (!tokens.empty()) {
    pStatistics->cntReceiveData = vscp_readStringValue(tokens.front());
  }

  // Received Frames
  pStatistics->cntReceiveFrames = 0;
  if (!tokens.empty()) {
    pStatistics->cntReceiveFrames = vscp_readStringValue(tokens.front());
  }

  // Transmitted data
  pStatistics->cntTransmitData = 0;
  if (!tokens.empty()) {
    pStatistics->cntTransmitData = vscp_readStringValue(tokens.front());
  }

  // Transmitted frames
  pStatistics->cntTransmitFrames = 0;
  if (!tokens.empty()) {
    pStatistics->cntTransmitFrames = vscp_readStringValue(tokens.front());
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int
VscpRemoteTcpIf::doCmdStatistics(canal_statistics_t *pStatistics)
{
  logTcpEntry(__func__);
  int rv;
  canal_statistics_t canalstat;

  if (NULL == pStatistics) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  if (VSCP_ERROR_SUCCESS != (rv = doCmdStatistics(&canalstat))) {
    return logTcpResult(__func__, rv);
  }

  // It may be tempting to just do a copy here but don't they
  // will be different in the future for sure.
  pStatistics->cntBusOff         = 0;
  pStatistics->cntBusWarnings    = 0;
  pStatistics->cntOverruns       = 0;
  pStatistics->cntReceiveData    = canalstat.cntReceiveData;
  pStatistics->cntReceiveFrames  = canalstat.cntReceiveFrames;
  pStatistics->cntTransmitData   = canalstat.cntTransmitData;
  pStatistics->cntTransmitFrames = canalstat.cntTransmitFrames;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int
VscpRemoteTcpIf::doCmdFilter(const vscpEventFilter *pFilter)
{
  logTcpEntry(__func__);
  std::string strCmd;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  // filter-priority, filter-class, filter-type, filter-GUID
  strCmd = vscp_str_format("SFLT "
                           "%d,%d,%d,%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%"
                           "02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                           pFilter->filter_priority,
                           pFilter->filter_class,
                           pFilter->filter_type,
                           pFilter->filter_GUID[15],
                           pFilter->filter_GUID[14],
                           pFilter->filter_GUID[13],
                           pFilter->filter_GUID[12],
                           pFilter->filter_GUID[11],
                           pFilter->filter_GUID[10],
                           pFilter->filter_GUID[9],
                           pFilter->filter_GUID[8],
                           pFilter->filter_GUID[7],
                           pFilter->filter_GUID[6],
                           pFilter->filter_GUID[5],
                           pFilter->filter_GUID[4],
                           pFilter->filter_GUID[3],
                           pFilter->filter_GUID[2],
                           pFilter->filter_GUID[1],
                           pFilter->filter_GUID[0]);

  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  // mask-priority, mask-class, mask-type, mask-GUID
  strCmd = vscp_str_format("SMSK "
                           "%d,%d,%d,%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%"
                           "02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                           pFilter->mask_priority,
                           pFilter->mask_class,
                           pFilter->mask_type,
                           pFilter->mask_GUID[15],
                           pFilter->mask_GUID[14],
                           pFilter->mask_GUID[13],
                           pFilter->mask_GUID[12],
                           pFilter->mask_GUID[11],
                           pFilter->mask_GUID[10],
                           pFilter->mask_GUID[9],
                           pFilter->mask_GUID[8],
                           pFilter->mask_GUID[7],
                           pFilter->mask_GUID[6],
                           pFilter->mask_GUID[5],
                           pFilter->mask_GUID[4],
                           pFilter->mask_GUID[3],
                           pFilter->mask_GUID[2],
                           pFilter->mask_GUID[1],
                           pFilter->mask_GUID[0]);
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdFilter  (with mask)
//

int
VscpRemoteTcpIf::doCmdFilter(const std::string &filter, const std::string &mask)
{
  logTcpEntry(__func__);
  std::string strCmd;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  // Set filter
  strCmd = "SFLT ";
  strCmd += filter;
  strCmd += "\r\n";
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  // Set mask
  strCmd = "SMSK ";
  strCmd += mask;
  strCmd += "\r\n";
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

int
VscpRemoteTcpIf::doCmdVersion(uint8_t *pMajorVer, uint8_t *pMinorVer, uint8_t *pSubMinorVer)
{
  logTcpEntry(__func__);
  std::string strLine;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::string strCmd("VERS\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  std::deque<std::string> tokens;
  vscp_split(tokens, m_inputStrArray.front(), ",");

  // Major version
  *pMajorVer = 0;
  if (!tokens.empty()) {
    m_version_year = *pMajorVer = (uint8_t) vscp_readStringValue(tokens.front());
    tokens.pop_front();
  }
  else {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  // Minor version
  *pMinorVer = 0;
  if (!tokens.empty()) {
    m_version_month = *pMinorVer = (uint8_t) vscp_readStringValue(tokens.front());
    tokens.pop_front();
  }
  else {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  // Sub minor version
  *pSubMinorVer = 0;
  if (!tokens.empty()) {
    m_version_patch = *pSubMinorVer = (uint8_t) vscp_readStringValue(tokens.front());
    tokens.pop_front();
  }
  else {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

int
VscpRemoteTcpIf::doCmdVersion(uint8_t *pMajorVer, uint8_t *pMinorVer, uint8_t *pSubMinorVer, uint16_t *pBuildVer)
{
  logTcpEntry(__func__);
  int rv;

  if (VSCP_ERROR_SUCCESS != (rv = doCmdVersion(pMajorVer, pMinorVer, pSubMinorVer))) {
    return logTcpResult(__func__, rv);
  }

  *pBuildVer = 0;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdDLLVersion
//

unsigned long
VscpRemoteTcpIf::doCmdDLLVersion(void)
{
  logTcpEntry(__func__);
  return TCPIP_DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVendorString
//

const char *
VscpRemoteTcpIf::doCmdVendorString(void)
{
  logTcpEntry(__func__);
  return TCPIP_VENDOR_STRING;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetDriverInfo
//

const char *
VscpRemoteTcpIf::doCmdGetDriverInfo(void)
{
  logTcpEntry(__func__);
  return DRIVER_INFO_STRING;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetGUID
//

int
VscpRemoteTcpIf::doCmdGetGUID(unsigned char *pGUID)
{
  logTcpEntry(__func__);
  std::string strLine;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::string strCmd("GGID\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  if (getInputQueueCount() < 2) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }
  strLine = m_inputStrArray[m_inputStrArray.size() - 2];

  std::deque<std::string> tokens;
  vscp_split(tokens, strLine, ",");

  // GUID
  if (!tokens.empty()) {
    std::string str = tokens.front();
    tokens.pop_front();
    if (!vscp_getGuidFromStringToArray(pGUID, str)) {
      return logTcpResult(__func__, VSCP_ERROR_GENERIC);
    }
  }
  else {
    return logTcpResult(__func__, VSCP_ERROR_GENERIC);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetGUID
//

int
VscpRemoteTcpIf::doCmdGetGUID(cguid &ifguid)
{
  logTcpEntry(__func__);
  std::string strLine;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::string strCmd("GGID\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  if (getInputQueueCount() < 2) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }
  strLine = m_inputStrArray[m_inputStrArray.size() - 2];

  ifguid.getFromString(strLine);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSetGUID
//

int
VscpRemoteTcpIf::doCmdSetGUID(const unsigned char *pGUID)
{
  logTcpEntry(__func__);
  std::string strLine;
  std::string strCmd;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  if (NULL == pGUID) {
    return logTcpResult(__func__, VSCP_ERROR_GENERIC);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  strCmd = vscp_str_format("SGID "
                           "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%"
                           "02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                           pGUID[0],
                           pGUID[1],
                           pGUID[2],
                           pGUID[3],
                           pGUID[4],
                           pGUID[5],
                           pGUID[6],
                           pGUID[7],
                           pGUID[8],
                           pGUID[9],
                           pGUID[10],
                           pGUID[11],
                           pGUID[12],
                           pGUID[13],
                           pGUID[14],
                           pGUID[15]);

  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSetGUID
//

int
VscpRemoteTcpIf::doCmdSetGUID(cguid &ifguid)
{
  logTcpEntry(__func__);
  unsigned char GUID[16];
  memcpy(GUID, ifguid.getGUID(), 16);
  return logTcpResult(__func__, doCmdSetGUID(GUID));
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetChannelInfo
//

int
VscpRemoteTcpIf::doCmdGetChannelInfo(vscp_channel_info_t *pChannelInfo)
{
  logTcpEntry(__func__);
  int rv;
  std::string strLine;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // Must have a valid pointer
  if (NULL == pChannelInfo) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  // If receive loop active terminate
  if (m_bModeReceiveLoop) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::string strCmd("INFO\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  if (getInputQueueCount() < 2) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }
  strLine = m_inputStrArray[m_inputStrArray.size() - 2];

  std::deque<std::string> tokens;
  vscp_split(tokens, strLine, ",");

  // Channel
  if (!tokens.empty()) {
    pChannelInfo->channel = (uint16_t) vscp_readStringValue(tokens.front());
    tokens.pop_front();
  }
  else {
    return logTcpResult(__func__, VSCP_ERROR_GENERIC);
  }

  // Set the interface level/type
  pChannelInfo->channelType = CANAL_COMMAND_OPEN_VSCP_LEVEL2;

  // Get the channel GUID
  rv = doCmdGetGUID((unsigned char *) pChannelInfo->GUID);

  return logTcpResult(__func__, rv);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetChannelID
//

int
VscpRemoteTcpIf::doCmdGetChannelID(uint32_t *pChannelID)
{
  logTcpEntry(__func__);
  std::string strLine;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  // Check pointer
  if (NULL == pChannelID) {
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  std::string strCmd("CHID\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  if (getInputQueueCount() < 2) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }
  strLine = m_inputStrArray[m_inputStrArray.size() - 2];

  // Reply comes from the server - do not let a malformed line throw
  try {
    *pChannelID = (uint32_t) std::stoul(strLine);
  }
  catch (const std::exception &) {
    transportError("%s: invalid channel id reply", __func__);
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdInterfaceList
//

int
VscpRemoteTcpIf::doCmdInterfaceList(std::deque<std::string> &strarray)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  std::string strCmd("INTERFACE LIST\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  if (getInputQueueCount() < 2) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  // Handle the data (if any)
  for (unsigned int i = 0; i < getInputQueueCount(); i++) {
    if (std::string::npos == m_inputStrArray[i].find("+OK")) {
      strarray.push_back(m_inputStrArray[i]);
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdWcyd
//

int
VscpRemoteTcpIf::doCmdWcyd(uint64_t *pwcyd)
{
  logTcpEntry(__func__);
  if (NULL == pwcyd) {
    return logTcpResult(__func__, VSCP_ERROR_INVALID_POINTER);
  }

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  std::string strCmd("wcyd\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  if (getInputQueueCount() < 2) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  // Fetch capabilities line (aa:bb:cc:dd:ee:ff:gg:hh) eight hex chars
  std::string strLine = m_inputStrArray[m_inputStrArray.size() - 2];

  if (strLine.length() < 23) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  // Reply comes from the server - parse "aa:bb:...:hh" strictly and do
  // not let a malformed line throw
  uint64_t wcyd = 0;
  try {
    size_t pos;
    for (int i = 0; i < 8; i++) {
      unsigned long value = std::stoul(strLine, &pos, 16);
      if (value > 0xff || (7 != i && (pos >= strLine.length() || ':' != strLine[pos]))) {
        throw std::invalid_argument("wcyd");
      }
      wcyd = (wcyd << 8) | value;
      if (7 != i) {
        strLine = strLine.substr(pos + 1); // Take away first part of str "zz:"
      }
    }
  }
  catch (const std::exception &) {
    transportError("%s: invalid capabilities reply", __func__);
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }
  *pwcyd = wcyd;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdShutdown
//

int
VscpRemoteTcpIf::doCmdShutDown(void)
{
  logTcpEntry(__func__);
  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_CONNECTION);
  }

  std::string strCmd("SHUTDOWN\r\n");
  if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
    return logTcpResult(__func__, VSCP_ERROR_ERROR);
  }

  return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//                       R E G I S T E R   H E L P E R S
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// readLevel2Register
//

int
VscpRemoteTcpIf::readLevel2Register(uint32_t reg,
                                    uint16_t page,
                                    uint8_t *pval,
                                    cguid &ifGUID,
                                    cguid *pdestGUID,
                                    bool bLevel2)
{
  logTcpEntry(__func__);
  int rv = VSCP_ERROR_SUCCESS;
  std::string strBuf;
  vscpEventEx e;

  // Check pointers
  if (NULL == pval) {
    transportError("%s: null value pointer", __func__);
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }
  if (NULL == pdestGUID) {
    transportError("%s: null destination GUID", __func__);
    return logTcpResult(__func__, VSCP_ERROR_PARAMETER);
  }

  e.head      = VSCP_PRIORITY_NORMAL;
  e.timestamp = 0;
  e.obid      = 0;

  // Check if a specific interface is used
  if (!ifGUID.isNULL()) {

    // Event should be sent to a specific interface
    e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
    e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

    memset(e.GUID, 0, 16); // We use GUID for interface
    e.sizeData = 16 + 5;   // Interface GUID + nodeid + register to read

    ifGUID.writeGUID(e.data);

    e.data[16] = pdestGUID->getLSB(); // nodeid
    e.data[17] = page >> 8;           // Page MSB
    e.data[18] = page & 0xff;         // Page LSB
    e.data[19] = reg;                 // Register to read
    e.data[20] = 1;                   // Number of registers to read
  }
  else {

    // Event should be sent to all interfaces

    if (bLevel2) {

      // True real Level II event

      e.head       = VSCP_PRIORITY_NORMAL;
      e.vscp_class = VSCP_CLASS2_PROTOCOL;
      e.vscp_type  = VSCP2_TYPE_PROTOCOL_READ_REGISTER;

      memset(e.GUID, 0, 16); // We use GUID for interface

      e.sizeData = 22; // nodeid + register to read

      pdestGUID->writeGUID(e.data);    // Destination GUID
      e.data[16] = (reg >> 24) & 0xff; // Register to read
      e.data[17] = (reg >> 16) & 0xff;
      e.data[18] = (reg >> 8) & 0xff;
      e.data[19] = reg & 0xff;
      e.data[20] = 0x00; // Read one register
      e.data[21] = 0x01;
    }
    else {

      // Level I over CLASS2 to all interfaces
      e.head       = VSCP_PRIORITY_NORMAL;
      e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
      e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

      memset(e.GUID, 0, 16);        // We use GUID for interface
      e.sizeData = 16 + 5;          // nodeid + register to read
      pdestGUID->writeGUID(e.data); // Destination GUID

      e.data[16] = pdestGUID->getLSB(); // nodeid
      e.data[17] = page >> 8;           // Page MSB
      e.data[18] = page & 0xff;         // Page LSB
      e.data[19] = reg;                 // Register to read
      e.data[20] = 1;                   // Number of registers to read
    }
  }

  // Send the event. Keep a copy of the request for resends as e is
  // reused for received events.
  doCmdClear();
  e.timestamp = 0;
  const vscpEventEx request = e;
  doCmdSendEx(&e);

  uint32_t resendTime = m_registerOpResendTimeout;
  uint32_t startTime  = vscp_getMsTimeStamp();

  while (true) {

    if (0 < doCmdDataAvailable()) { // Message available

      if (VSCP_ERROR_SUCCESS == doCmdReceiveEx(&e)) { // Valid event

        // Level I Read reply?
        if ((VSCP_CLASS1_PROTOCOL == e.vscp_class) && (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type) &&
            (e.sizeData >= 5)) {

          if ((0 == e.data[0]) && // Frame index is first?
              ((page >> 8) == e.data[1]) && ((page & 0x0ff) == e.data[2]) &&
              (reg == e.data[3])) { // Requested register?

            if (pdestGUID->isSameGUID(e.GUID)) { // From correct node?
              if (NULL != pval) {
                *pval = e.data[4];
              }
              break;
            }

          } // Check for correct node
        }

        // Level II 512 Read reply?
        else if (!ifGUID.isNULL() && !bLevel2 && (VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class) &&
                 (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type) && (e.sizeData >= 21)) {

          if (pdestGUID->isSameGUID(e.GUID)) {

            if ((0 == e.data[16]) && // Frame index is first?
                ((page >> 8) == e.data[17]) && ((page & 0x0ff) == e.data[18]) &&
                (reg == e.data[19])) { // Requested register?

              // OK get the data
              if (NULL != pval) {
                *pval = e.data[20];
                break;
              }
            }
          }
        }
        // Level II Read reply?
        else if (ifGUID.isNULL() && bLevel2 && (VSCP_CLASS2_PROTOCOL == e.vscp_class) &&
                 (VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == e.vscp_type) && (e.sizeData >= 5)) {

          // from us
          if (pdestGUID->isSameGUID(e.GUID)) {

            uint32_t retreg = (e.data[0] << 24) + (e.data[1] << 16) + (e.data[2] << 8) + e.data[3];

            // Reg we requested?
            if (retreg == reg) {

              // OK get the data
              if (NULL != pval) {
                *pval = e.data[4];
                break;
              }
            }
          }
        }

      } // valid event
    }

    if ((vscp_getMsTimeStamp() - startTime) > m_registerOpErrorTimeout) {
      rv = CANAL_ERROR_TIMEOUT;
      break;
    }
    else if ((vscp_getMsTimeStamp() - startTime) > resendTime) {
      // Send the request again
      vscpEventEx resend = request;
      doCmdSendEx(&resend);
      resendTime += m_registerOpResendTimeout;
    }

#ifndef WIN32
    usleep(2000);
#else
    win_usleep(2000);
#endif

  } // while

  return logTcpResult(__func__, rv);
}

//////////////////////////////////////////////////////////////////////////////
// readLevel2Registers
//

int
VscpRemoteTcpIf::readLevel2Registers(uint32_t reg,
                                     uint16_t page,
                                     uint8_t count,
                                     uint8_t *pval,
                                     cguid &ifGUID,
                                     cguid *pdestGUID,
                                     bool bLevel2)
{
  logTcpEntry(__func__);
  int rv = CANAL_ERROR_SUCCESS;
  std::string strBuf;
  vscpEventEx e;
  uint8_t data[256]; // This makes range checking simpler

  // Max 128 bytes can be read in one go
  if (count > 128) {
    return logTcpResult(__func__, CANAL_ERROR_PARAMETER);
  }

  // Check pointers
  if (NULL == pval) {
    return logTcpResult(__func__, CANAL_ERROR_PARAMETER);
  }
  if (NULL == pdestGUID) {
    return logTcpResult(__func__, CANAL_ERROR_PARAMETER);
  }

  e.head      = VSCP_PRIORITY_NORMAL;
  e.timestamp = 0;
  e.obid      = 0;

  // Check if a specific interface is used
  if (!ifGUID.isNULL()) {

    // Event should be sent to a specific interface
    e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
    e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

    memset(e.GUID, 0, 16); // We use GUID for interface
    e.sizeData = 16 + 5;   // Interface GUID + nodeid + register to read

    ifGUID.writeGUID(e.data);

    e.data[16] = pdestGUID->getLSB(); // nodeid
    e.data[17] = page >> 8;           // Page MSB
    e.data[18] = page & 0xff;         // Page LSB
    e.data[19] = reg;                 // Register to read
    e.data[20] = count;               // Read count registers
  }
  else {

    // Event should be sent to all interfaces

    if (bLevel2) {

      // True real Level II event

      e.head       = VSCP_PRIORITY_NORMAL;
      e.vscp_class = VSCP_CLASS2_PROTOCOL;
      e.vscp_type  = VSCP2_TYPE_PROTOCOL_READ_REGISTER;

      memset(e.GUID, 0, 16); // We use GUID for interface

      e.sizeData = 22; // nodeid + register to read

      pdestGUID->writeGUID(e.data);    // Destination GUID
      e.data[16] = (reg >> 24) & 0xff; // Register to read
      e.data[17] = (reg >> 16) & 0xff;
      e.data[18] = (reg >> 8) & 0xff;
      e.data[19] = reg & 0xff;
      e.data[20] = 0; // Read count registers
      e.data[21] = count;
    }
    else {

      // Level I over CLASS2 to all interfaces
      e.head       = VSCP_PRIORITY_NORMAL;
      e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
      e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

      memset(e.GUID, 0, 16);        // We use GUID for interface
      e.sizeData = 16 + 5;          // nodeid + register to read
      pdestGUID->writeGUID(e.data); // Destination GUID

      e.data[16] = pdestGUID->getLSB(); // nodeid
      e.data[17] = page >> 8;           // Page MSB
      e.data[18] = page & 0xff;         // Page LSB
      e.data[19] = reg;                 // Register to read
      e.data[20] = count;               // Read count registers
    }
  }

  // Send the event. Keep a copy of the request for resends as e is
  // reused for received events.
  doCmdClear();
  e.timestamp = 0;
  const vscpEventEx request = e;
  doCmdSendEx(&e);

  // We should get 32 response frames back
  // a bit is set in response_flags for each frame
  unsigned long receive_flags = 0;
  unsigned char nPages        = count / 4;
  unsigned char lastpageCnt   = count % 4;
  if (lastpageCnt) {
    nPages++;
  }
  unsigned long allRcvValue = (unsigned long) pow(2.0, nPages) - 1;

  unsigned long resendTime = (unsigned long) m_registerOpResendTimeout * (1 + nPages);
  uint32_t startTime       = vscp_getMsTimeStamp();

  while (allRcvValue != (receive_flags & 0xffffffff)) { // mask for systems where long is > 32 bits

    int n;
    if (0 < (n = doCmdDataAvailable())) { // Message available

      while (CANAL_ERROR_SUCCESS == doCmdReceiveEx(&e)) { // Valid event

        // Quit if done?
        if (allRcvValue == (receive_flags & 0xffffffff)) break;

        // Level I Read reply?
        if ((VSCP_CLASS1_PROTOCOL == e.vscp_class) && (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type)) {

          if (pdestGUID->isSameGUID(e.GUID)) { // From correct node?

            // The frame index comes from the network. Only accept frames
            // we asked for, carrying all the bytes for that frame.
            const uint8_t frameIdx = e.data[0];
            const int frameBytes   = (lastpageCnt && (nPages - 1) == frameIdx) ? lastpageCnt : 4;
            if ((frameIdx < nPages) && (e.sizeData >= 4 + frameBytes) &&
                ((page >> 8) == e.data[1]) && ((page & 0x0ff) == e.data[2])) { // Requested page?

              // Mark frame as received
              receive_flags |= (1UL << frameIdx);

              for (int i = 0; i < frameBytes; i++) {
                data[frameIdx * 4 + i] = e.data[4 + i];
              }
            }

          } // Check for response from correct node
        }

        // Level II 512 Read reply?
        else if (!ifGUID.isNULL() && !bLevel2 && (VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class) &&
                 (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type)) {

          if (pdestGUID->isSameGUID(e.GUID)) {

            // The frame index comes from the network. Only accept frames
            // we asked for, carrying all the bytes for that frame.
            const uint8_t frameIdx = e.data[16];
            const int frameBytes   = (lastpageCnt && (nPages - 1) == frameIdx) ? lastpageCnt : 4;
            if ((frameIdx < nPages) && (e.sizeData >= 16 + 4 + frameBytes) &&
                ((page >> 8) == e.data[17]) && ((page & 0x0ff) == e.data[18])) { // Requested register?

              // Mark frame as received
              receive_flags |= (1UL << frameIdx);

              for (int i = 0; i < frameBytes; i++) {
                data[frameIdx * 4 + i] = e.data[16 + 4 + i];
              }
            }

          } // Check for response from correct node
        }

        // Level II Read reply?
        else if (ifGUID.isNULL() && bLevel2 && (VSCP_CLASS2_PROTOCOL == e.vscp_class) &&
                 (VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == e.vscp_type)) {

          // from us. Reply holds the start register followed by the data.
          if (pdestGUID->isSameGUID(e.GUID) && (e.sizeData >= 4 + count)) {

            uint32_t retreg = ((uint32_t) e.data[0] << 24) + ((uint32_t) e.data[1] << 16) +
                              ((uint32_t) e.data[2] << 8) + e.data[3];

            // Reg we requested?
            if (retreg == reg) {
              // All requested registers are in this reply
              memcpy(data, e.data + 4, count);
              receive_flags = allRcvValue;
              break;
            }
          }
        }

      } // valid event

    } // Data available

    if ((vscp_getMsTimeStamp() - startTime) > m_registerOpErrorTimeout) {
      rv = CANAL_ERROR_TIMEOUT;
      break;
    }
    else if ((vscp_getMsTimeStamp() - startTime) > resendTime) {
      // Send the request again
      vscpEventEx resend = request;
      doCmdSendEx(&resend);
      resendTime += (unsigned long) m_registerOpResendTimeout * (1 + nPages);
      logTcpDebug(__func__, "retrying register read");
    }

#ifndef WIN32
    usleep(2000);
#else
    win_usleep(2000);
#endif

  } // while

  // Copy data on success
  if (CANAL_ERROR_SUCCESS == rv) {
    memcpy(pval, data, count);
  }

  return logTcpResult(__func__, rv);
}

//////////////////////////////////////////////////////////////////////////////
// writeLevel2Register
//

int
VscpRemoteTcpIf::writeLevel2Register(uint32_t reg,
                                     uint16_t page,
                                     uint8_t *pval,
                                     cguid &ifGUID,
                                     cguid *pdestGUID,
                                     bool bLevel2)
{
  logTcpEntry(__func__);
  int rv          = CANAL_ERROR_SUCCESS;
  bool bInterface = false; // No specific interface set
  std::string strBuf;
  vscpEventEx e;

  // Check pointers
  if (NULL == pval) {
    return logTcpResult(__func__, CANAL_ERROR_PARAMETER);
  }
  if (NULL == pdestGUID) {
    return logTcpResult(__func__, CANAL_ERROR_PARAMETER);
  }

  if (!ifGUID.isNULL()) {

    e.head       = VSCP_PRIORITY_NORMAL;
    e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
    e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE;

    memset(e.GUID, 0, 16); // We use interface GUID

    e.sizeData = 16 + 5;

    pdestGUID->writeGUID(e.data);     // Destination GUID
    e.data[16] = pdestGUID->getLSB(); // nodeid
    e.data[17] = (page >> 8);
    e.data[18] = (page & 0xff);
    e.data[19] = reg;   // Register to write
    e.data[20] = *pval; // value to write
  }
  else {

    if (bLevel2) {

      e.head       = VSCP_PRIORITY_NORMAL;
      e.vscp_class = VSCP_CLASS2_PROTOCOL;
      e.vscp_type  = VSCP2_TYPE_PROTOCOL_WRITE_REGISTER;

      memset(e.GUID, 0, 16);        // We use interface GUID
      e.sizeData = 21;              // nodeid + register to read
      pdestGUID->writeGUID(e.data); // Destination GUID

      e.data[16] = (reg >> 24) & 0xff; // Register to write
      e.data[17] = (reg >> 16) & 0xff;
      e.data[18] = (reg >> 8) & 0xff;
      e.data[19] = reg & 0xff;
      e.data[20] = *pval; // Data to write
    }
    else {

      // Level I over CLASS2 to all interfaces

      e.head       = VSCP_PRIORITY_NORMAL;
      e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
      e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE;

      memset(e.GUID, 0, 16); // We use interface GUID
      e.sizeData = 16 + 5;
      pdestGUID->writeGUID(e.data); // Destination GUID

      e.data[16] = pdestGUID->getLSB(); // nodeid
      e.data[17] = page >> 8;           // Page MSB
      e.data[18] = page & 0xff;         // Page LSB
      e.data[19] = reg;                 // Register to write
      e.data[20] = *pval;               // value to write
    }
  }

  // Send the event. Keep a copy of the request for resends as e is
  // reused for received events.
  const vscpEventEx request = e;
  doCmdSendEx(&e);

  unsigned long resendTime = m_registerOpResendTimeout;
  uint32_t startTime       = vscp_getMsTimeStamp();

  while (true) {

    int n;
    if (0 < (n = doCmdDataAvailable())) { // Message available

      if (CANAL_ERROR_SUCCESS == doCmdReceiveEx(&e)) { // Valid event

        if ((VSCP_CLASS1_PROTOCOL == e.vscp_class) &&
            (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type) && (e.sizeData >= 5)) { // Read reply?

          if ((0 == e.data[0]) &&                                   // Frame index is first frame
              ((page >> 8) == e.data[1]) &&                         // Page is same as requested
              ((page & 0xff) == e.data[2]) && (reg == e.data[3])) { // Requested register?

            if (pdestGUID->isSameGUID(e.GUID)) { // From correct node?

              // We got a rw reply from the correct node. Is
              // the written data same as we expect?
              if (*pval != e.data[4]) {
                rv = CANAL_ERROR_REGISTER;
              }
              break;
            }
          }
        }

        // Level II 512 Read reply?
        else if (!ifGUID.isNULL() && !bLevel2 && (VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class) &&
                 (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type) && (e.sizeData >= 21)) {

          if (pdestGUID->getLSB() == e.data[15]) {

            if ((0 == e.data[16]) &&                                    // Frame index is first frame
                ((page >> 8) == e.data[17]) &&                          // Page is same as requested
                ((page & 0xff) == e.data[18]) && (reg == e.data[19])) { // Reg we requested?

              // We got a rw reply from the correct node. Is
              // the written data same as we expect?
              if (*pval != e.data[20]) { // Value byte follows the register
                rv = CANAL_ERROR_REGISTER;
              }
              break;
            }
          }
        }
        // Level II Read reply?
        else if (!bInterface && bLevel2 && (VSCP_CLASS2_PROTOCOL == e.vscp_class) &&
                 (VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == e.vscp_type) && (e.sizeData >= 5)) {

          // from us
          if (pdestGUID->isSameGUID(e.GUID)) {

            uint32_t retreg = (e.data[0] << 24) + (e.data[1] << 16) + (e.data[2] << 8) + e.data[3];

            // Reg we requested?
            if (retreg == reg) {

              // OK get the data
              if (NULL != pval) {
                // We go a rw reply from the correct node is
                // the written data same as we expect.
                if (*pval != e.data[4])
                  rv = CANAL_ERROR_REGISTER;
                break;
              }
            }
          }
        }
      }
    }
    else {
#ifndef WIN32
      usleep(2000);
#else
      win_usleep(2000);
#endif
    }

    if ((vscp_getMsTimeStamp() - startTime) > m_registerOpErrorTimeout) {
      rv = CANAL_ERROR_TIMEOUT;
      break;
    }
    else if ((vscp_getMsTimeStamp() - startTime) > resendTime) {
      // Send the request again
      vscpEventEx resend = request;
      doCmdSendEx(&resend);
      resendTime += m_registerOpResendTimeout;
    }

  } // while

  return logTcpResult(__func__, rv);
}

//////////////////////////////////////////////////////////////////////////////
// getMDFUrlFromLevel2Device
//

bool
VscpRemoteTcpIf::getMDFUrlFromLevel2Device(cguid &ifGUID, cguid &destGUID, std::string &strurl, bool bLevel2)
{
  logTcpEntry(__func__);
  char url[33];
  bool rv = true;

  memset(url, 0, sizeof(url));

  if (bLevel2) {

    // Level 2 device
    uint8_t *p = (uint8_t *) url;
    for (int i = 0; i < 32; i++) {

      if (CANAL_ERROR_SUCCESS != readLevel2Register(0xFFFFFFE0 + i,
                                                    0,   // Page = 0
                                                    p++, // read value
                                                    ifGUID,
                                                    &destGUID,
                                                    true)) { // Yes Level II
        rv = false;
        goto error;
      }
    }
  }
  else {

    // Level 1 device
    uint8_t *p = (uint8_t *) url;
    for (int i = 0; i < 32; i++) {

      if (CANAL_ERROR_SUCCESS != readLevel2Register(0xE0 + i,
                                                    0,   // Page = 0
                                                    p++, // read value
                                                    ifGUID,
                                                    &destGUID)) {
        rv = false;
        goto error;
      }
    }
  }

  strurl = std::string(url);
  if (strurl.npos == strurl.find("http://")) {
    std::string str;
    str = "http://";
    str += strurl;
    strurl = str;
  }

error:

  if (!rv) {
    transportError("%s: failed to retrieve MDF URL", __func__);
    strurl.clear();
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//                           Graphical helpers
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// fetchIterfaceGUID
//

int
VscpRemoteTcpIf::fetchIterfaceGUID(const std::string &ifName, cguid &guid)
{
  logTcpEntry(__func__);
  std::string str;
  std::deque<std::string> ifarray;
  uint8_t cnt = 0;

  if (!isConnected()) {
    return logTcpResult(__func__, VSCP_ERROR_NOT_OPEN);
  }

  // Get the interface list
  while (true) {

    // Count every attempt so a failing or empty reply can not loop forever
    cnt++;

    if (VSCP_ERROR_SUCCESS == doCmdInterfaceList(ifarray)) {

      if (ifarray.size()) {

        for (unsigned int i = 0; i < ifarray.size(); i++) {

          std::deque<std::string> tokens;
          vscp_split(tokens, ifarray[i], ",");

          std::string strOrdinal;
          if (!tokens.empty()) {
            strOrdinal = tokens.front();
            tokens.pop_front();
          }

          std::string strType;
          if (!tokens.empty()) {
            strType = tokens.front();
            tokens.pop_front();
          }

          std::string strIfGUID;
          if (!tokens.empty()) {
            strIfGUID = tokens.front();
            tokens.pop_front();
          }

          std::string strDescription;
          if (!tokens.empty()) {
            strDescription = tokens.front();
            tokens.pop_front();
          }

          size_t pos;
          std::string strName;
          if (std::string::npos != (pos = strDescription.find("|"))) {
            strName = strDescription.substr(0, pos);
            vscp_trim(strName);
          }

          if (0 == vscp_strcasecmp(strName.c_str(), ifName.c_str())) {

            // Save interface GUID;
            guid.getFromString(strIfGUID);
            return VSCP_ERROR_SUCCESS;
          }
        }
      }
    }

    // Give up after three tries
    if (cnt >= 3)
      break;
  }

  return logTcpResult(__func__, VSCP_ERROR_OPERATION_FAILED);
}
