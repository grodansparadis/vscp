// udpReceiveThread.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://can.sourceforge.net) 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia,<akhe@eurosource.se>
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
// $RCSfile: canal_macro.h,v $                                       
// $Date: 2005/01/05 12:16:09 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 

#ifdef WIN32

#define LOCK_MUTEX( x )		( WaitForSingleObject( x, INFINITE ) )
#define UNLOCK_MUTEX( x )	( ReleaseMutex( x ) )
#define SLEEP( x )			( Sleep( x ) )
#define SYSLOG( a, b )		( wxLogError( b ) )
#define BZERO( a )			( memset( ( _u8* )&a, 0, sizeof( a ) ) )	

#else

#define LOCK_MUTEX( x )		( pthread_mutex_lock( &x ) )
#define UNLOCK_MUTEX( x )	( pthread_mutex_unlock( &x ) )
#define SLEEP( x )			( usleep( ( 1000 * x ) ) )
#define SYSLOG( a, b )		( syslog( a, b ) )
#define BZERO( a )			( bzero( ( _u8* )&a, sizeof( a ) ) )

#endif


#define MAX( a, b )	( ( (a) > (b) ) ? (a) : (b) )
#define MIN( a, b )	( ( (a) < (b) ) ? (a) : (b) )
#define ABS( a )	(( (int) (a) < 0 ) ? ((a) ^ 0xffffffff) + 1 : (a) )

