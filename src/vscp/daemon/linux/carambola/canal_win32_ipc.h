// canal_win32_ipc.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2011 Ake Hedman, Grodans Paradis AB,<akhe@grodansparadis.com>
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
// $RCSfile: canal_win32_ipc.h,v $                                       
// $Date: 2005/09/06 21:19:10 $                                  
// $Author: akhe $                                              
// $Revision: 1.6 $ 

/// Names of mutex's
#define CANAL_RECEIVE_OBJ_MUTEX				_T("____CANAL_RECEIVE_OBJ_MUTEX____")
#define CANAL_UDP_OBJ_MUTEX					_T("____CANAL_UDP_OBJ_MUTEX____")
#define CANAL_CLIENT_MUTEX					_T("__CANAL_CLIENT_MUTEX__")

#define CANAL_LISTEN_SHARED_MEMORY_MUTEX	_T("CANAL_LISTEN_SHARED_MEMORY_MUTEX")

/// Shared memory  
#define CANAL_LISTEN_SHM_NAME				_T("CANAL_LISTEN_SHARED_MEMORY_SEMAPHORE")
#define CANAL_LISTEN_CLIENT_SHM_TEMPLATE	_T("CANAL_LISTEN_CLIENT_%lu")

/// Semaphores
#define CANAL_LISTEN_COMMAND_SEM			_T("CANAL_LISTEN_COMMAND_SEMAPHORE")
#define CANAL_LISTEN_DONE_SEM				_T("CANAL_LISTEN_DONE_SEMAPHORE")

#define CANAL_CLIENT_COMMAND_SEM_TEMPLATE	_T("CANAL_CLIENT_COMMAND_SEMAPHORE%lu")
#define CANAL_CLIENT_DONE_SEM_TEMPLATE		_T("CANAL_CLIENT_DONE_SEMAPHORE%lu")





