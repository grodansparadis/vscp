// canaldlldef.h
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


#ifndef ___CANALDLLDEF_H___
#define ___CANALDLLDEF_H___

#ifdef WIN32

#endif  // WIN32

#include "canal.h"

#ifdef WIN32
typedef long ( __stdcall * LPFNDLL_CANALOPEN) ( const char *pDevice, unsigned long flags );
typedef int ( __stdcall * LPFNDLL_CANALCLOSE) ( long handle );
typedef unsigned long ( __stdcall * LPFNDLL_CANALGETLEVEL) ( long handle  );
typedef int ( __stdcall * LPFNDLL_CANALSEND) (  long handle, const PCANALMSG pCanalMsg );
typedef int ( __stdcall * LPFNDLL_CANALRECEIVE) ( long handle,  PCANALMSG pCanalMsg );
typedef int ( __stdcall * LPFNDLL_CANALDATAAVAILABLE) ( long handle );
typedef int ( __stdcall * LPFNDLL_CANALGETSTATUS) ( long handle,  PCANALSTATUS pCanalStatus );
typedef int ( __stdcall * LPFNDLL_CANALGETSTATISTICS) ( long handle,  PCANALSTATISTICS pCanalStatistics  );
typedef int ( __stdcall * LPFNDLL_CANALSETFILTER) (  long handle, unsigned long filter );
typedef int ( __stdcall * LPFNDLL_CANALSETMASK) (  long handle, unsigned long mask );
typedef int ( __stdcall * LPFNDLL_CANALSETBAUDRATE) (  long handle, unsigned long baudrate );
typedef unsigned long ( __stdcall * LPFNDLL_CANALGETVERSION) (  void );
typedef unsigned long ( __stdcall * LPFNDLL_CANALGETDLLVERSION) (  void );
typedef const char * ( __stdcall * LPFNDLL_CANALGETVENDORSTRING) (  void );
// Generation 2
typedef int ( __stdcall * LPFNDLL_CANALBLOCKINGSEND) (  long handle, const PCANALMSG pCanalMsg, unsigned long timeout );
typedef int ( __stdcall * LPFNDLL_CANALBLOCKINGRECEIVE) ( long handle,  PCANALMSG pCanalMsg, unsigned long timeout );
typedef const char * ( __stdcall * LPFNDLL_CANALGETDRIVERINFO) ( void );

#else // UNIX

typedef long ( *LPFNDLL_CANALOPEN ) ( const char *pDevice, unsigned long flags );
typedef int ( *LPFNDLL_CANALCLOSE ) ( long handle );
typedef unsigned long ( *LPFNDLL_CANALGETLEVEL ) ( long handle  );
typedef int ( *LPFNDLL_CANALSEND ) (  long handle, const PCANALMSG pCanalMsg );
typedef int ( *LPFNDLL_CANALRECEIVE ) ( long handle, PCANALMSG pCanalMsg );
typedef int ( *LPFNDLL_CANALDATAAVAILABLE ) ( long handle );
typedef int ( *LPFNDLL_CANALGETSTATUS ) ( long handle,  PCANALSTATUS pCanalStatus );
typedef int ( *LPFNDLL_CANALGETSTATISTICS ) ( long handle,  PCANALSTATISTICS pCanalStatistics  );
typedef int ( *LPFNDLL_CANALSETFILTER ) (  long handle, unsigned long filter );
typedef int ( *LPFNDLL_CANALSETMASK ) (  long handle, unsigned long mask );
typedef int ( *LPFNDLL_CANALSETBAUDRATE ) (  long handle, unsigned long baudrate );
typedef unsigned long ( *LPFNDLL_CANALGETVERSION ) (  void );
typedef unsigned long ( *LPFNDLL_CANALGETDLLVERSION ) (  void );
typedef const char *( *LPFNDLL_CANALGETVENDORSTRING ) (  void );
// Generation 2
typedef int ( *LPFNDLL_CANALBLOCKINGSEND ) (  long handle, const PCANALMSG pCanalMsg, unsigned long timeout );
typedef int ( *LPFNDLL_CANALBLOCKINGRECEIVE ) ( long handle, PCANALMSG pCanalMsg, unsigned long timeout );
typedef const char * ( *LPFNDLL_CANALGETDRIVERINFO) ( void );

#endif // WIN32


#endif // ___CANALDLLDEF_H___


