// udpReceiveThread.cpp
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


#ifdef WIN32

#define LOCK_MUTEX( x )         ( WaitForSingleObject( x, INFINITE ) )
#define UNLOCK_MUTEX( x )       ( ReleaseMutex( x ) )
#define SLEEP( x )              ( Sleep( x ) )
#define SYSLOG( a, b )          ( wxLogError( b ) )
#define BZERO( a )              ( memset( ( _u8* )&a, 0, sizeof( a ) ) )	

#else

#define LOCK_MUTEX( x )         ( pthread_mutex_lock( &x ) )
#define UNLOCK_MUTEX( x )       ( pthread_mutex_unlock( &x ) )
#define SLEEP( x )              ( usleep( ( 1000 * x ) ) )      // ms
#define SYSLOG( a, b )          ( syslog( a, b ) )
#define BZERO( a )              ( bzero( ( _u8* )&a, sizeof( a ) ) )

#endif

#ifndef MAX
#define MAX( a, b )     ( ( (a) > (b) ) ? (a) : (b) )
#endif

#ifndef MIN
#define MIN( a, b )     ( ( (a) < (b) ) ? (a) : (b) )
#endif

#ifndef ABS
#define ABS( a )        (( (int) (a) < 0 ) ? ((a) ^ 0xffffffff) + 1 : (a) )
#endif

