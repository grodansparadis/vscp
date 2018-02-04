///////////////////////////////////////////////////////////////////////////////
// Canal_linux_ips.h: IPC common defines
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2017 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

// Shared memory objects
#define IPC_LISTEN_SHM_KEY      2901

// Semaphores
#define IPC_LISTEN_SEM_KEY      2902
#define LISTEN_SEM_RESOURCE     0   // semaphore 0 - Listentread resources
#define LISTEN_SEM_COMMAND      1   // seamphore 1 - Command

//#define IPC_CLIENTCMD_SEM_KEY	2903
#define CLIENT_CMD_SEM          0   // Semaphore 0 - Client command
#define CLIENT_DONE_SEM         1   // Semaphore 1 - Command done
