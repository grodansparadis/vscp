// FILE: knownnodes.cpp 
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2015 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#ifdef WIN32
#include <sys/types.h>   // for type definitions 
#include <winsock2.h>
#include <ws2tcpip.h>    // for win socket structs 
#endif

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/datetime.h>
#include <wx/listimpl.cpp>
#include <wx/hashmap.h>

#ifdef WIN32
#else

#define _POSIX

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#endif

#include "daemonvscp.h"
#include "canal_win32_ipc.h"
#include "canal_macro.h"
#include "vscp.h"
#include "vscphelper.h"
#include "clientlist.h"
#include <dllist.h>
#include <md5.h>
#include <crc8.h>
#include "controlobject.h"
#include "guid.h"


WX_DEFINE_LIST( VSCP_KNOWN_NODES_LIST );

///////////////////////////////////////////////////////////////////////////////
// CNodeInformation CTOR
//

CNodeInformation::CNodeInformation()
{
    m_pClientItem = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// CNodeInformation DTOR
//

CNodeInformation::~CNodeInformation()
{
    ;
}






///////////////////////////////////////////////////////////////////////////////
// CKnownNodes CTOR
//

CKnownNodes::CKnownNodes()
{
    m_pClientItem = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// CKnownNodes DTOR
//

CKnownNodes::~CKnownNodes()
{
    ;
}