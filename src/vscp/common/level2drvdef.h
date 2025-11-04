// level2drvdef.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2025 Ake Hedman, the VSCP project <info@vscp.org>
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

#ifndef ___VSCPDLLDEF_H___
#define ___VSCPDLLDEF_H___


#include "vscp.h"

#ifdef WIN32

typedef long ( __stdcall * LPFNDLL_VSCPOPEN ) ( const char *pPathConfig, const unsigned char*pguid );
typedef int ( __stdcall * LPFNDLL_VSCPCLOSE ) ( long handle );
typedef int ( __stdcall * LPFNDLL_VSCPWRITE ) ( long handle, const vscpEvent *pEvent, unsigned long timeout );
typedef int ( __stdcall * LPFNDLL_VSCPREAD ) ( long handle, vscpEvent *pEvent, unsigned long timeout );
typedef unsigned long ( __stdcall * LPFNDLL_VSCPGETVERSION ) (  void );
typedef const char *( __stdcall * LPFNDLL_VSCPGETVENDORSTRING ) ( void );

#else

typedef long ( *LPFNDLL_VSCPOPEN ) ( const char *pPathConfig, const unsigned char*pguid );
typedef int ( *LPFNDLL_VSCPCLOSE ) ( long handle );
typedef int ( *LPFNDLL_VSCPWRITE ) ( long handle, const vscpEvent *pEvent, unsigned long timeout );
typedef int ( *LPFNDLL_VSCPREAD ) ( long handle, vscpEvent *pEvent, unsigned long timeout );
typedef unsigned long ( *LPFNDLL_VSCPGETVERSION ) (  void );
typedef const char *( *LPFNDLL_VSCPGETVENDORSTRING ) ( void );

#endif


#endif // ___VSCPDLLDEF_H__
