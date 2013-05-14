/*
 * Winamp VSCP Plugin
 * Copyright (C) 1999-2003 Kosta Arvanitis
 * Copyright (C) 2005-2011 Ake Hedman, eurosource
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Kosta Arvanitis (karvanitis@hotmail.com)
 */

#ifndef __WINAMPVSCP_H
#define __WINAMPVSCP_H

#include "wa_ipc.h"		// from winamp plugin SDK

// internal buffer sizes
#define LIMIT_REQUEST_BUFFER 			2048

// application defines
#ifdef _DEBUG
const char* const APPNAME =		        "Winamp VSCP Plugin DEBUG";
#else
const char* const APPNAME =		        "Winamp VSCP Plugin";
#endif

const DWORD APPVERSION =                MAKEWORD(3, 0);

// useful
#define ARRAYSIZE(a) sizeof(a)/sizeof(a[0])

#define GPPHDR_VER 	                    0x10

const char* const VERSION =		        "v0.0";
const char* const EMAILADDR =	        "akhe@eurosource.se";
const char* const MAILTO =		        "mailto:akhe@eurosource.se";
const char* const WEBADDR =		        "http://www.vscp.org";
const char* const COPYRIGHT =	        "Copyright © 2005";
const char* const AUTHOR =		        "Ake Hedman";

#endif // __WINAMPVSCP_H
