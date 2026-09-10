#ifndef VSCP_WINDOWS_PCH_H
#define VSCP_WINDOWS_PCH_H

#if defined(_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

#endif  // VSCP_WINDOWS_PCH_H
