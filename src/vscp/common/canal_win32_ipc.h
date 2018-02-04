// canal_win32_ipc.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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
//

/// Names of mutex's
#define CANAL_RECEIVE_OBJ_MUTEX             _T("____CANAL_RECEIVE_OBJ_MUTEX____")
#define CANAL_UDP_OBJ_MUTEX                 _T("____CANAL_UDP_OBJ_MUTEX____")
#define CANAL_CLIENT_MUTEX                  _T("__CANAL_CLIENT_MUTEX__")

#define CANAL_LISTEN_SHARED_MEMORY_MUTEX    _T("CANAL_LISTEN_SHARED_MEMORY_MUTEX")

/// Shared memory  
#define CANAL_LISTEN_SHM_NAME               _T("CANAL_LISTEN_SHARED_MEMORY_SEMAPHORE")
#define CANAL_LISTEN_CLIENT_SHM_TEMPLATE    _T("CANAL_LISTEN_CLIENT_%lu")

/// Semaphores
#define CANAL_LISTEN_COMMAND_SEM            _T("CANAL_LISTEN_COMMAND_SEMAPHORE")
#define CANAL_LISTEN_DONE_SEM               _T("CANAL_LISTEN_DONE_SEMAPHORE")

#define CANAL_CLIENT_COMMAND_SEM_TEMPLATE   _T("CANAL_CLIENT_COMMAND_SEMAPHORE%lu")
#define CANAL_CLIENT_DONE_SEM_TEMPLATE      _T("CANAL_CLIENT_DONE_SEMAPHORE%lu")





