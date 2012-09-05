///////////////////////////////////////////////////////////////////////////////
// Canal_linux_ips.h: IPC common defines
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2011 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $RCSfile: canal_unix_ipc.h,v $                                       
// $Date: 2005/03/14 22:58:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

// Shared memory objects
#define IPC_LISTEN_SHM_KEY		2901

// Semaphores
#define IPC_LISTEN_SEM_KEY		2902
#define LISTEN_SEM_RESOURCE		0	// semaphore 0 - Listentread resources
#define LISTEN_SEM_COMMAND		1	// seamphore 1 - Command

//#define IPC_CLIENTCMD_SEM_KEY	2903
#define CLIENT_CMD_SEM			0	// Semaphore 0 - Client command
#define CLIENT_DONE_SEM			1	// Semaphore 1 - Command done
