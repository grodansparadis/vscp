// vscpdlldef.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2017
// Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
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



