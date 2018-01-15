// vscpdlldef.h
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

#ifndef ___VSCPDLLDEF_H___
#define ___VSCPDLLDEF_H___


#include "vscp.h"

// This structure is returned by a driver from a call to
// VSCPGetVSCPWebPageInfo and gives information about
// webpages this driver can 


// Entry we generate for each page served.
struct vscpextpage
{
    // If true only admin will be allowed to access the page
    bool bOnlyAdmin;
    
    // Acceptable URL for this page.
    const char *url;

    // Mime type to set for the page.
    const char *mime;
  
    // Description of page
    const char *description;	
    
};

struct vscpextwebpageinfo{
    int nPages;	// Number of pages
    vscpextpage **pages;
};

#ifdef WIN32

typedef long ( __stdcall * LPFNDLL_VSCPOPEN) ( const char *pUsername, const char *pPassword, const char *pHost, short port, const char *pPrefix, const char *pParameter );
typedef int ( __stdcall * LPFNDLL_VSCPCLOSE) ( long handle );
typedef int ( __stdcall * LPFNDLL_VSCPBLOCKINGSEND ) ( long handle, vscpEvent *pEvent, unsigned long timeout );
typedef int ( __stdcall * LPFNDLL_VSCPBLOCKINGRECEIVE ) ( long handle, vscpEvent *pEvent, unsigned long timeout );
typedef unsigned long ( __stdcall * LPFNDLL_VSCPGETLEVEL) ( long handle  );
typedef long ( __stdcall *LPFNDLL_VSCPGETWEBPAGETEMPLATE) ( long handle, const char *url, char *page );
typedef int ( __stdcall *LPFNDLL_VSCPGETWEBPAGEINFO) ( long handle, const struct vscpextwebpageinfo *info );
typedef int ( __stdcall *LPFNDLL_VSCPWEBPAGEUPDATE) ( long handle, const char *url );
typedef unsigned long ( __stdcall * LPFNDLL_VSCPGETVERSION) ( void );
typedef unsigned long ( __stdcall * LPFNDLL_VSCPGETDLLVERSION) ( void );
typedef const char * ( __stdcall * LPFNDLL_VSCPGETVENDORSTRING) ( void );
typedef const char * ( __stdcall * LPFNDLL_VSCPGETDRIVERINFO) ( void );
typedef const char * ( __stdcall * LPFNDLL_VSCPGETSTATUS) ( unsigned long* pStatusCode );

#else

typedef long ( *LPFNDLL_VSCPOPEN ) ( const char *pUsername, const char *pPassword, const char *pHost, short port, const char *pPrefix, const char *pParameter );
typedef int ( *LPFNDLL_VSCPCLOSE ) ( long handle );
typedef int ( *LPFNDLL_VSCPBLOCKINGSEND ) ( long handle, const vscpEvent *pEvent, unsigned long timeout );
typedef int ( *LPFNDLL_VSCPBLOCKINGRECEIVE ) ( long handle, vscpEvent *pEvent, unsigned long timeout );
typedef unsigned long ( *LPFNDLL_VSCPGETLEVEL ) ( long handle  );
typedef long ( *LPFNDLL_VSCPGETWEBPAGETEMPLATE) ( long handle, const char *url, char *page );
typedef int ( *LPFNDLL_VSCPGETWEBPAGEINFO) ( long handle, const struct vscpextwebpageinfo *info );
typedef int ( *LPFNDLL_VSCPWEBPAGEUPDATE) ( long handle, const char *url );
typedef unsigned long ( *LPFNDLL_VSCPGETVERSION ) (  void );
typedef unsigned long ( *LPFNDLL_VSCPGETDLLVERSION ) (  void );
typedef const char *( *LPFNDLL_VSCPGETVENDORSTRING ) (  void );
typedef const char *( *LPFNDLL_VSCPGETDRIVERINFO ) (  void );
typedef const char *( *LPFNDLL_VSCPGETSTATUS) ( unsigned long* pStatusCode );

#endif


#endif // ___VSCPDLLDEF_H__



