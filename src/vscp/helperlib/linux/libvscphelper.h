// libvscphelper.h : main header file for the canallogger.dll
// Linux version
//
// VSCP (Very Simple Control Protocol)
// http://www.vscp.org
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman,
// Grodans Paradis AB <info@grodansparadis.com>
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
//  This file is part of VSCP - Very Simple Control Protocol
//  http://www.vscp.org
//

#if !defined(VSCP_HELPER_LIB_H__INCLUDED_)
#define VSCP_HELPER_LIB_H__INCLUDED_

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define VSCP_DLL_SONAME "libvscphelper.1.3"

// This is the version info for this DLL - Change to your own value
#define VSCP_DLL_VERSION 3

// This is the vendor string - Change to your own value
#define VSCP_DLL_VENDOR                                                        \
    "Grodans Paradis AB, Sweden, https://www.grodansparadis.com"

/*!
        Add a driver object

        @parm plog Object to add
        @return handle or 0 for error
*/
long
addDriverObject(VscpRemoteTcpIf *pvscpif);

/*!
        Get a driver object from its handle

        @param handle for object
        @return pointer to object or NULL if invalid
                        handle.
*/
VscpRemoteTcpIf *
getDriverObject(long handle);

/*!
        Remove a driver object
        @param handle for object.
*/
void
removeDriverObject(long handle);

#endif // !defined(VSCP_HELPER_LIB_H__INCLUDED_)
