// canaldlldef.h
//
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.


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


