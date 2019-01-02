// vscpd.h : main header file for the VSCPD application
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#if !defined(_VSCPD_H__INCLUDED_)
#define _VSCPD_H__INCLUDED_

#include <controlobject.h>

// Include the CANAL definitions
#include <canal.h>

/// Names of mutex's

#define CANALD_CLIENT_OBJ_MUTEX "____CANAL_CLIENT_OBJ_MUTEX____"
#define CANALD_DEVICE_OBJ_MUTEX "____CANAL_DEVICE_OBJ_MUTEX____"
#define CANALD_SEND_OBJ_MUTEX "____CANAL_SEND_OBJ_MUTEX____"
#define CANALD_RECEIVE_OBJ_MUTEX "____CANAL_RECEIVE_OBJ_MUTEX____"
#define CANALD_CLIENT_MUTEX "__CANAL_CLIENT_MUTEX__"

/////////////////////////////////////////////////////////////////////////////
// VSCPApp:
// See vscpd.cpp for the implementation of this class
//

class VSCPApp {
    
public:
    VSCPApp();

public:

    
    /*!
            Initialize the system
     */
    int init( std::string& strcfgfile, std::string& rootFolder );


    // Implementation

private:

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSCPD_H__3D1CDB8C_8027_46D5_9284_67750BEA4B7E__INCLUDED_)
