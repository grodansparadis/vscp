// CanalPipe.cpp: implementation of the CVSCPPipe class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part is part of CANAL (CAN Abstraction Layer)
//  (http://www.vscp.org)
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: vscppipe.cpp,v $                                       
// $Date: 2005/01/05 12:50:53 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
//////////////////////////////////////////////////////////////////////

#ifdef WIN32


#else

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
typedef unsigned long DWORD;

#endif


#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include  "wx/ownerdrw.h"
#endif

#include "vscpPipe.h"
#include "pipecmd.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVSCPPipe::CVSCPPipe()
{
#ifdef WIN32
	m_hPipe = NULL;
#else
	m_Pipe = 0;
#endif
}


CVSCPPipe::~CVSCPPipe()
{
#ifdef WIN32
	if ( NULL != m_hPipe ) {
		DisconnectNamedPipe( m_hPipe );
		CloseHandle( m_hPipe );
		m_hPipe = NULL;
	}
#else
	if ( 0 != m_Pipe ) {
		close( m_Pipe );
		m_Pipe = 0;
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////
// open
//

bool CVSCPPipe::doCmdOpen( const char *szInterface, unsigned long flags )
{
	char cmd[ 512 ];
	char response[ 32 ];
	DWORD nRead = 0;
	unsigned char *p;
	
#ifdef WIN32	
	
	// If open we don't need to open it again	
	if ( NULL != m_hPipe ) return false;
	
	if ( INVALID_HANDLE_VALUE == 
						( m_hPipe = CreateFile( VSCP_CLIENT_PIPE,
													GENERIC_READ | 
													GENERIC_WRITE,
													0,
													NULL,
													OPEN_EXISTING,
													FILE_ATTRIBUTE_NORMAL,
													NULL ) ) ) {

		// Failed to create the pipe. Probably nothing at the other end
		m_hPipe = NULL;
		return false;	
	}
	else {

		DWORD mode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
		SetNamedPipeHandleState( m_hPipe,
									&mode,
									NULL, NULL );

		memset( cmd, 0, sizeof( cmd ) );
		cmd[ 0/*VSCP_UDP_POS_COMMAND*/ ] = VSCP_COMMAND_OPEN;

		// Flags
		p = (unsigned char *)&flags;

#ifdef BIGENDIAN 
			cmd[ 1 ] = *(p+0); 
			cmd[ 2 ] = *(p+1);
			cmd[ 3 ] = *(p+2);
			cmd[ 4 ] = *(p+3);	
#else 
			cmd[ 1 ] = *(p+3);	// Store bigendian
			cmd[ 2 ] = *(p+2);
			cmd[ 3 ] = *(p+1);
			cmd[ 4 ] = *(p+0);
#endif

	
		// Perform the transaction
		if ( !TransactNamedPipe( m_hPipe, 
									cmd, 
									sizeof( cmd ), 
									response, 
									sizeof( response ), 
									&nRead, NULL ) ) {
			
			// Transaction failed
			m_err = GetLastError();
			doCmdClose();
			return false;
		}

		if ( VSCP_PIPE_RESPONSE_SUCCESS != response[ 0 ] ) {
			// Command failed
			m_err = GetLastError();
			doCmdClose();
			return false;
		}
	}

#else	// U N I X

	if ( 0 != m_Pipe ) return false;

	//
	// USR   -- Read/Write
	// GROUP -- Read/Write
	// OTHER -- Read/write
	//
	
	if ( -1 == mkfifo( VSCP_CLIENT_PIPE, S_IRUSR | S_IWUSR |
							S_IRGRP | S_IWGRP |
							S_IROTH | S_IWOTH )  &&
							( EEXIST != errno ) ) {
			
		syslog( LOG_CRIT, "Unable to create fifo. Terminating VSCP Pipe Open.");
		return false;
	}

	if ( -1 == ( m_Pipe = open( VSCP_CLIENT_PIPE, O_RDWR  ) ) ) {
		syslog( LOG_CRIT, "Unable to open fifo. Terminating VSCP Pipe Open.");
		return false;
	}
	
	memset( cmd, 0, sizeof( cmd ) );
	cmd[ 0/*VSCP_UDP_POS_COMMAND*/ ] = VSCP_PIPE_COMMAND_OPEN;

	// Flags
	p = (unsigned char *)&flags;

#ifdef BIGENDIAN 
	cmd[ 1 ] = *(p+0); 
	cmd[ 2 ] = *(p+1);
	cmd[ 3 ] = *(p+2);
	cmd[ 4 ] = *(p+3);	
#else 
	cmd[ 1 ] = *(p+3);	// Store bigendian
	cmd[ 2 ] = *(p+2);
	cmd[ 3 ] = *(p+1);
	cmd[ 4 ] = *(p+0);
#endif

	if ( -1 == ( write( m_Pipe, cmd, sizeof( cmd ) ) ) ) {
	
		if ( -1 == ( nRead = read( m_Pipe, response, sizeof( response ) ) ) ) {
			
			if ( VSCP_PIPE_RESPONSE_SUCCESS != response[ 0 ] ) {
			
				// Command failed
				m_err = errno;
				doCmdClose();
				return false;
				
			}
			
		}
		else{
			// Command failed
			m_err = errno;
			doCmdClose();
			return false;
		}
		
		
	}
	else {
		// Command failed
		m_err = errno;
		doCmdClose();
		return false;
	}

#endif  // UNIX

	return true;
}

 
///////////////////////////////////////////////////////////////////////////////
// close
//

bool CVSCPPipe::doCmdClose( void )
{
	char cmd[ 312 ];
	char response[ 32 ];
	DWORD nRead = 0;

#ifdef WIN32
	if ( NULL == m_hPipe ) return false;
#else
	if ( 0 == m_Pipe ) return false;
#endif

	cmd[ 0/*VSCP_UDP_POS_COMMAND*/ ] = VSCP_PIPE_COMMAND_CLOSE;

#ifdef WIN32

	if ( !TransactNamedPipe( m_hPipe, 
								cmd, 
								sizeof( cmd ), 
								response, 
								sizeof( response ), 
								&nRead, NULL ) ) {	
		// Transaction failed			
		m_err = GetLastError();
		return false;
	}

#else
	response[ 0 ] = VSCP_PIPE_RESPONSE_NONE;
	if ( -1 == ( write( m_Pipe, cmd, sizeof( cmd ) ) ) ) {
	
		if ( -1 == ( nRead = read( m_Pipe, response, sizeof( response ) ) ) ) {
				m_err = errno;
				return false;
		}
		
	}
#endif

	if ( VSCP_PIPE_RESPONSE_SUCCESS != response[ 0 ] ) {
			// Command failed
#ifdef WIN32			
			m_err = GetLastError();
#else
			m_err = errno;
#endif
			return false;
	}

#ifdef WIN32
	DisconnectNamedPipe( m_hPipe );
	CloseHandle( m_hPipe );
	m_hPipe = NULL;
#else
	close( m_Pipe );
	m_Pipe = 0;
#endif

	return true;
}




///////////////////////////////////////////////////////////////////////////////
// doCmdNOOP
//

bool CVSCPPipe::doCmdNOOP( void )
{	
	char cmd[32];
	char response[32];
	DWORD nRead = 0;

	
	cmd[ 0/*VSCP_UDP_POS_COMMAND*/ ] = VSCP_PIPE_COMMAND_NONE;

#ifdef WIN32
	if ( !TransactNamedPipe( m_hPipe, 
								cmd, 
								sizeof( cmd ), 
								response, 
								sizeof( response ), 
								&nRead, 
								NULL ) ) {			
		m_err = GetLastError();
	}
#else
	response[ 0 ] = VSCP_PIPE_RESPONSE_NONE;
	if ( -1 == ( write( m_Pipe, cmd, sizeof( cmd ) ) ) ) {
	
		if ( -1 == ( nRead = read( m_Pipe, response, sizeof( response ) ) ) ) {
				m_err = errno;
				return false;
		}
		
	}
#endif	

	if ( ( 1 != nRead ) && ( VSCP_PIPE_RESPONSE_SUCCESS == response[ 0 ] ) ) {
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

bool CVSCPPipe::doCmdSend( vscpMsg *pMsg )
{	
	char cmd[ 1 + 25 + 4 + 512 - 25 ];
	char response[32];
	DWORD nRead = 0;
	unsigned char *p;
	
	cmd[ 0/*VSCP_UDP_POS_COMMAND*/ ] = VSCP_PIPE_COMMAND_SEND;

	// Set realtive time for packet (ms)
	//pMsg->timestamp = GetTickCount();

	if ( pMsg->sizeData > 487 ) pMsg->sizeData = 487;

#ifdef BIGENDIAN 

	// Head
	p = (unsigned char *)&pMsg->vscp_type;
	cmd[ POS_HEAD + 0 ] = *(p+0);

	// VSCP class
	p = (unsigned char *)&pMsg->vscp_class;
	cmd[ POS_CLASS + 0 ] = *(p+0); 
	cmd[ POS_CLASS + 1 ] = *(p+1); 

	// VSCP type
	p = (unsigned char *)&pMsg->vscp_type;
	cmd[ POS_TYPE + 0 ] = *(p+0); 
	cmd[ POS_TYPE + 1 ] = *(p+1); 

	// Node address
	p = (unsigned char *)&pMsg->address;
	memcpy( &cmd[ POS_ADDRESS + 0 ], p, 16 );

	// VSCP Size
	p = (unsigned char *)&pMsg->size;
	cmd[ POS_SIZE + 0 ] = *(p+0); 
	cmd[ POS_SIZE + 1 ] = *(p+1);

	if ( pMsg->size > 0) {
		// Data Max 487 (512- 25) bytes
		// size = sz - command_byte - control_bytes
		memcpy( &cmd[ POS_DATA ], pMsg->pdata, pMsg->size );
	}

	// VSCP CRC
	p = (unsigned char *)&pMsg->crc;
	cmd[ POS_CRC + 0 ] = *(p+0); 
	cmd[ POS_CRC + 1 ] = *(p+1);
									

#else 

	// Head
	p = (unsigned char *)&pMsg->vscp_type;
	cmd[ VSCP_UDP_POS_HEAD + 0 ] = *(p+0);

	// VSCP class
	p = (unsigned char *)&pMsg->vscp_class;
	cmd[ VSCP_UDP_POS_CLASS + 0 ] = *(p+1); 
	cmd[ VSCP_UDP_POS_CLASS + 1 ] = *(p+0); 

	// VSCP type
	p = (unsigned char *)&pMsg->vscp_type;
	cmd[ VSCP_UDP_POS_TYPE + 0 ] = *(p+1); 
	cmd[ VSCP_UDP_POS_TYPE + 1 ] = *(p+0); 

	// Node address
	p = (unsigned char *)&pMsg->GUID;
	memcpy( &cmd[ VSCP_UDP_POS_GUID + 0 ], p, 16 );

	// Data Max 487 (512- 25) bytes
	// size = sz - command_byte - control_bytes
	memcpy( &cmd[ VSCP_UDP_POS_DATA + 0 ], pMsg->pdata, pMsg->sizeData );

	// VSCP Size
	p = (unsigned char *)&pMsg->sizeData;
	cmd[ VSCP_UDP_POS_SIZE + 0 ] = *(p+1); 
	cmd[ VSCP_UDP_POS_SIZE + 1 ] = *(p+0);

	// VSCP CRC
	p = (unsigned char *)&pMsg->crc;
	cmd[ VSCP_UDP_POS_CRC + 0 ] = *(p+1); 
	cmd[ VSCP_UDP_POS_CRC + 1 ] = *(p+0);
									

#endif	

#ifdef WIN32
	if ( !TransactNamedPipe( m_hPipe, 
								cmd, 
								( 1 + 25 + 4 + pMsg->sizeData ), // cmd + struct + timestamp + data
								response, 
								sizeof( response ), 
								&nRead, 
								NULL ) ) {			
		m_err = GetLastError();
	}
#else
	response[ 0 ] = VSCP_PIPE_RESPONSE_NONE;
	if ( -1 == ( write( m_Pipe, cmd, sizeof( cmd ) ) ) ) {
	
		if ( -1 == ( nRead = read( m_Pipe, response, sizeof( response ) ) ) ) {
				m_err = errno;
				return false;
		}
		
	}
#endif	

	if ( VSCP_PIPE_RESPONSE_SUCCESS != response[ 0 ] ) {
		return false;
	}

	return true;
}

 

///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

bool CVSCPPipe::doCmdReceive( vscpMsg *pMsg, unsigned long id )
{	
	char cmd[ 1 ];
	char response[ 2 + 25 + 4 + 512 - 25 ];
	DWORD nRead = 0;
	unsigned char *p;

#ifdef WIN32
	if ( NULL == m_hPipe ) return false;
#else
	if ( 0 == m_Pipe ) return false;
#endif

	cmd[ 0/*VSCP_UDP_POS_COMMAND*/ ] = VSCP_PIPE_COMMAND_RECEIVE;
	
#ifdef WIN32	
	if ( !TransactNamedPipe( m_hPipe, 
								cmd, 
								sizeof( cmd ), 
								response, 
								sizeof( response ), 
								&nRead, 
								NULL ) ) {
		m_err = GetLastError();
	}
	
#else
	response[ 0 ] = VSCP_PIPE_RESPONSE_NONE;
	if ( -1 == ( write( m_Pipe, cmd, sizeof( cmd ) ) ) ) {
	
		if ( -1 == ( nRead = read( m_Pipe, response, sizeof( response ) ) ) ) {
				m_err = errno;
				return false;
		}
		
	}
#endif	
	
	if ( VSCP_PIPE_RESPONSE_SUCCESS != response[ 0 ] ) {
		return false;
	}

#ifdef BIGENDIAN 

	// Head
	p = (unsigned char *)&pMsg->vscp_type;
	*(p+0) = response[ POS_HEAD + 0 ];
								
	// VSCP class
	p = (unsigned char *)&pMsg->vscp_class;
	*(p+0) = response[ POS_CLASS + 0 ];
	*(p+1) = response[ POS_CLASS + 1 ]; 

	// VSCP type
	p = (unsigned char *)&pMsg->vscp_type;
	*(p+0) = response[ POS_TYPE + 0 ];
	*(p+1) = response[ POS_TYPE + 1 ];
									
	// Node address
	p = (unsigned char *)&pMsg->address;
	memcpy( p, &response[ POS_ADDRESS + 0 ], 16 );

	// Data Max 487 (512- 25) bytes
	if ( pMsg->size > 487 ) pMsg->size = 487; 

	// Number of valid data bytes
	p = (unsigned char *)&pMsg->size;
	*(p+0) = response[ POS_SIZE + 0 ];
	*(p+1) = response[ POS_SIZE + 1 ];

	if ( NULL != ( pMsg->pdata = new unsigned char [ pMsg->size ] ) ) {
		memcpy( pMsg->pdata, &response[ POS_DATA + 0 ], pMsg->size ); 
	}
									
	// CRC
	p = (unsigned char *)&pMsg->crc;
	*(p+0) = response[ POS_CRC + 0 ];
	*(p+1) = response[ POS_CRC + 1 ];
	
	// timestamp
	p = (unsigned char *)&pMsg->timestamp;
	*(p+0) = response[ POS_TIMESTAMP + 0 ];	
	*(p+1) = response[ POS_TIMESTAMP + 1 ];
	*(p+2) = response[ POS_TIMESTAMP + 2 ];
	*(p+3) = response[ POS_TIMESTAMP + 3 ];

#else 

	// Head
	p = (unsigned char *)&pMsg->vscp_type;
	*(p+0) = response[ VSCP_UDP_POS_HEAD + 0 ];
								
	// VSCP class
	p = (unsigned char *)&pMsg->vscp_class;
	*(p+1) = response[ VSCP_UDP_POS_CLASS + 0 ];
	*(p+0) = response[ VSCP_UDP_POS_CLASS + 1 ]; 

	// VSCP type
	p = (unsigned char *)&pMsg->vscp_type;
	*(p+1) = response[ VSCP_UDP_POS_TYPE + 0 ];
	*(p+0) = response[ VSCP_UDP_POS_TYPE + 1 ];
									
	// Node address
	p = (unsigned char *)&pMsg->GUID;
	memcpy( p, &response[ VSCP_UDP_POS_GUID + 0 ], 16 );

	// Number of valid data bytes
	p = (unsigned char *)&pMsg->sizeData;
	*(p+1) = response[ VSCP_UDP_POS_SIZE + 0 ];
	*(p+0) = response[ VSCP_UDP_POS_SIZE + 1 ];

	// Data Max 487 (512- 25) bytes
	if ( NULL != ( pMsg->pdata = new unsigned char [ pMsg->sizeData ] ) ) {
		// size = sz - command_byte - control_bytes
		memcpy( pMsg->pdata, &response[ VSCP_UDP_POS_DATA + 0 ], pMsg->sizeData ); 
	}
									
	// CRC
	p = (unsigned char *)&pMsg->crc;
	*(p+1) = response[ VSCP_UDP_POS_CRC + 0 ];
	*(p+0) = response[ VSCP_UDP_POS_CRC + 1 ];
	
#endif

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int CVSCPPipe::doCmdDataAvailable( void )
{
	int nMsg = 0;
	char cmd[ 1 ];
	char response[ 5 ];
	DWORD nRead = 0;
	unsigned char *p;

	cmd[ 0/*VSCP_UDP_POS_COMMAND*/ ] = VSCP_PIPE_COMMAND_CHECKDATA;
#ifdef WIN32	
	if ( !TransactNamedPipe( m_hPipe, 
								cmd, 
								sizeof( cmd ), 
								response, 
								sizeof( response ), 
								&nRead, 
								NULL ) ) {
			
		m_err = GetLastError();
	}
#else
	response[ 0 ] = VSCP_PIPE_RESPONSE_NONE;
	if ( -1 == ( write( m_Pipe, cmd, sizeof( cmd ) ) ) ) {
	
		if ( -1 == ( nRead = read( m_Pipe, response, sizeof( response ) ) ) ) {
				m_err = errno;
				return false;
		}
		
	}
#endif	
	
	if ( VSCP_PIPE_RESPONSE_SUCCESS != response[ 0 ] ) {
		return false;
	}

#ifdef BIGENDIAN 
	// Number of packets
	p = (unsigned char *)&nMsg;
	*(p+0) = response[ 1 ]; 
	*(p+1) = response[ 2 ];
	*(p+2) = response[ 3 ];
	*(p+3) = response[ 4 ];
#else
	// Number of packets
	p = (unsigned char *)&nMsg;
	*(p+3) = response[ 1 ];	
	*(p+2) = response[ 2 ];
	*(p+1) = response[ 3 ];
	*(p+0) = response[ 4 ];
#endif

	return nMsg;
}


///////////////////////////////////////////////////////////////////////////////
// docmdFilter
//

bool CVSCPPipe::doCmdFilter(  unsigned long filter )
{
	char cmd[ 512 ];
	char response[ 32 ];
	DWORD nRead = 0;
	unsigned char *p;
		
	// If open we don't need to open it again
#ifdef WIN32
	if ( NULL == m_hPipe ) return false;
#else
	if ( 0 == m_Pipe ) return false;
#endif

	memset( cmd, 0, sizeof( cmd ) );
	cmd[ 0/*VSCP_UDP_POS_COMMAND*/ ] = VSCP_PIPE_COMMAND_FILTER;

	// Fulter
	p = (unsigned char *)&filter;

#ifdef BIGENDIAN 
	cmd[ 1 ] = *(p+0); 
	cmd[ 2 ] = *(p+1);
	cmd[ 3 ] = *(p+2);
	cmd[ 4 ] = *(p+3);	
#else 
	cmd[ 1 ] = *(p+3);	// Store bigendian
	cmd[ 2 ] = *(p+2);
	cmd[ 3 ] = *(p+1);
	cmd[ 4 ] = *(p+0);
#endif
	
	// Perform the transaction
#ifdef WIN32	
	if ( !TransactNamedPipe( m_hPipe, 
								cmd, 
								sizeof( cmd ), 
								response, 
								sizeof( response ), 
								&nRead, NULL ) ) {
			
		// Transaction failed			
		m_err = GetLastError();
		
		doCmdClose();
		return false;
	}
#else
	response[ 0 ] = VSCP_PIPE_RESPONSE_NONE;
	if ( -1 == ( write( m_Pipe, cmd, sizeof( cmd ) ) ) ) {
	
		if ( -1 == ( nRead = read( m_Pipe, response, sizeof( response ) ) ) ) {
				m_err = errno;
				return false;
		}
		
	}
#endif	

	if ( VSCP_PIPE_RESPONSE_SUCCESS != response[ 0 ] ) {
		// Command failed
#ifdef WIN32		
		m_err = GetLastError();
#else
		m_err = errno;
#endif
		doCmdClose();
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// docmdMask
//

bool CVSCPPipe::doCmdMask(  unsigned long mask )
{
	char cmd[ 512 ];
	char response[ 32 ];
	DWORD nRead = 0;
	unsigned char *p;
	
	
	// If open we don't need to open it again
#ifdef WIN32
	if ( NULL == m_hPipe ) return false;
#else
	if ( 0 == m_Pipe ) return false;
#endif

	memset( cmd, 0, sizeof( cmd ) );
	cmd[ 0/*VSCP_UDP_POS_COMMAND*/ ] = VSCP_PIPE_COMMAND_MASK;

	// Fulter
	p = (unsigned char *)&mask;

#ifdef BIGENDIAN 
	cmd[ 1 ] = *(p+0); 
	cmd[ 2 ] = *(p+1);
	cmd[ 3 ] = *(p+2);
	cmd[ 4 ] = *(p+3);	
#else 
	cmd[ 1 ] = *(p+3);	// Store bigendian
	cmd[ 2 ] = *(p+2);
	cmd[ 3 ] = *(p+1);
	cmd[ 4 ] = *(p+0);
#endif
	
	// Perform the transaction
#ifdef WIN32	
	if ( !TransactNamedPipe( m_hPipe, 
								cmd, 
								sizeof( cmd ), 
								response, 
								sizeof( response ), 
								&nRead, NULL ) ) {
			
		// Transaction failed			
		m_err = GetLastError();
 
		doCmdClose();
		return false;
	}
	
#else
	response[ 0 ] = VSCP_PIPE_RESPONSE_NONE;
	if ( -1 == ( write( m_Pipe, cmd, sizeof( cmd ) ) ) ) {
	
		if ( -1 == ( nRead = read( m_Pipe, response, sizeof( response ) ) ) ) {
				m_err = errno;
				return false;
		}
		
	}
#endif	

	if ( VSCP_PIPE_RESPONSE_SUCCESS != response[ 0 ] ) {
		// Command failed
#ifdef WIN32			
		m_err = GetLastError();
#else
		m_err = errno;
#endif
		doCmdClose();
		return false;
	}

	return true;
}