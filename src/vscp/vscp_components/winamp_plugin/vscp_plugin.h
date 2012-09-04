/*
 * Winamp httpQ Plugin
 * Copyright (C) 1999-2003 Kosta Arvanitis
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
#ifndef __HTTPQ_H
#define __HTTPQ_H

// windows includes
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <crtdbg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#include "wa_ipc.h"
#include "array.h"


// winsock version
#define WINSOCKVERSION			MAKEWORD(2,0)

// internal buffer sizes
#define LIMIT_REQUEST_BUFFER 	2048

// default http header reply
const char* const HTTP_HEADER_OK	=	"HTTP/1.0 200 OK\r\n";
const char* const HTTP_HEADER_CONTENT = "Content-Type: text/html\r\n";
const char* const HTTP_HEADER_TERM =	"\r\n";

// term char
const char * const HTTP_TERMCODE =		"\r\n\r\n";

// reply codes
const char * const HTTPQ_REPLY_ERROR =  "0";
const char * const HTTPQ_REPLY_OK =	    "1";

// useful
#define ARRAYSIZE(a) sizeof(a)/sizeof(a[0])

// application defines
#ifdef _DEBUG
const char* const APPNAME =		        "Winamp httpQ Plugin DEBUG";
#else
const char* const APPNAME =		        "Winamp httpQ Plugin";
#endif

const DWORD APPVERSION =                MAKEWORD(3, 0);

const char* const VERSION =		        "v3.0";
const char* const EMAILADDR =	        "karvanitis@hotmail.com";
const char* const MAILTO =		        "mailto:karvanitis@hotmail.com";
const char* const WEBADDR =		        "http://httpq.sourceforge.net";
const char* const COPYRIGHT =	        "Copyright © 1999-2003";
const char* const AUTHOR =		        "Kosta Arvanitis";

#define GPPHDR_VER 	                    0x10


// config
#define MAX_STRING                      256

#define DEFAULT_IP_ADDRESS 			    0x00000000
#define DEFAULT_PORT 					4800
#define DEFAULT_ALLOW_ALL_IP_ENABLED 	1
#define DEFAULT_CONNECTION_TIMEOUT 		900
#define DEFAULT_AUTO_START_ENABLED 		0
#define DEFAULT_HTTP_HEADERS_ENABLED 	1
#define DEFAULT_NEW_LOG_TIME_PERIOD 	0

//const char* const DEFAULT_IP_ADDRESS_LABEL =    "[All Unassigned]";
const char* const DEFAULT_PASSWORD =			"pass";


#endif // __HTTPQ_H