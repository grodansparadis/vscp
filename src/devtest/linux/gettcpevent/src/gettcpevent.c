/***************************************************************************
 *   Copyright (C) 2006,2007 by Ake Hedman   				   				   *
 *   akhe@eurosource.se   						   						   *
 *	 Part of VSCP project, http://www.vscp.org							   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define USERNAME 		"admin"         // logon username
#define PASSWORD 		"secret"        // logon password
#define RCVBUFSIZE		512   			// TCP/IP received buffer size
#define GUID 			"0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15"  // GUID to use for i/f

// States for data parsing
#define		STATE_HEAD			0
#define		STATE_CLASS			1
#define 	STATE_TYPE			2
#define 	STATE_OBID			3
#define 	STATE_TIMESTAMP		4
#define		STATE_GUID			5
#define		STATE_DATA			6
// Prototypes

void DieWithError(char *errorMessage);  
int getResponse( int sock, char *buf );

//////////////////////////////////////////////////////////////////////////////
// main
//
// address [port]
//

int main(  int argc, char *argv[]  )
{
	int i,j;
	char *p;
	int sock;                       // Socket descriptor
	struct sockaddr_in ServerAddr;  // Server address
	unsigned short ServerPort;      // Server port
	char *szServerIP;               // Server IP address (dotted quad)
  
	char buf[RCVBUFSIZE];			// Receivebuffer
	char wrkbuf[512];				// Buffer for general work
	int bytesRcvd, totalBytesRcvd;  // Bytes read in single recv()
                                  	//	and total bytes read
	int nEvents;					// # events waiting to be received
	
	unsigned char guid[16];			// Storage for GUID

	int state;
	unsigned int vscp_head;
	unsigned int vscp_class;
	unsigned int vscp_type;
	unsigned long vscp_obid;
	unsigned long vscp_timestamp;
	char strguid[512];
	unsigned char data[512-25];
	int cntData;
	
	printf("VSCP client TCP/IP Event receive example.\n");
	printf("-----------------------------------------\n");
	printf("Use ctrl-C to end.\n");

	// Test for correct number of arguments
	if ( ( argc < 2 ) || ( argc > 3 ) ) {
		fprintf( stderr,
					"Usage: %s <Server IP> [port]\n",
					argv[0]);
		exit(1);
	}
  
	szServerIP = argv[ 1 ];   // server IP address (dotted quad)
  
	if ( 3 == argc ) {
		ServerPort = atoi( argv[ 2 ] ); // Use given port, if any
	}
	else {
		ServerPort = 9598;  // VSCP server standard port
	}
  
  	// Create a a socket
	if ( ( sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 ) {
		DieWithError("socket() failed");
	}

	// Non blocking
	//if ( fcntl( sock, F_SETFL, O_NDELAY ) < 0 ) {
	//	DieWithError("fcntl F_SETFL, O_NDELAY");
	//}
  
  	// Construct the server address structure 
	memset( &ServerAddr, 0, sizeof( ServerAddr ) );
	ServerAddr.sin_family      = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr( szServerIP );
	ServerAddr.sin_port        = htons( ServerPort );
  
  	// Establish the connection to the server
	if ( connect( sock,
		 			(struct sockaddr *) &ServerAddr,
		 			sizeof( ServerAddr ) ) < 0 ) {
		DieWithError("connect() failed");
	}
  
  	// Check to see that we are connected
	memset( buf, 0, sizeof( buf ) );
	if ( ( bytesRcvd = recv( sock, buf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
		DieWithError("recv() failed or connection closed prematurely");
	}
  
	if ( NULL == strstr( buf, "OK" ) ) {
		DieWithError("Failed to connect to server");
	}
  
  
  	// * * * Set username * * *
  
  
  	// Send the username to the server 
	sprintf( buf, "user %s\r\n", USERNAME );
	if ( send( sock, buf, strlen( buf ), 0) != strlen( buf ) ) {
		DieWithError("Failed to send username.");
	}
  
  	// Get response
	memset( buf, 0, sizeof( buf ) );
	if ( ( bytesRcvd = recv( sock, buf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
		DieWithError("Failed to set username. No response.");
	}
  
  	// Check to see if usernamewas accepted
	if ( NULL == strstr( buf, "OK" ) ) {
		DieWithError("Username not accepted.");
	}
  
  
  
  	// * * * Set password * * *
  
  
  	// Send password to the server 
	sprintf( buf, "pass %s\r\n", PASSWORD);
	if ( send( sock, buf, strlen( buf ), 0) != strlen( buf ) ) {
		DieWithError("Failed to send password.");
	}
  
  	// Get response
	memset( buf, 0, sizeof( buf ) );
	if ( ( bytesRcvd = recv( sock, buf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
		DieWithError("Failed to set password.No response.");
	}

  	// Check to see if username was accepted
	if ( NULL == strstr( buf, "OK" ) ) {
		DieWithError("Password.wa not accepted");
	}
  
  
  
  	// * * * Set interface GUID * * *
	// This is not needed but added here to make the example complete.
  
  	// Set interface GUID
	sprintf( buf, "SGID %s\r\n", GUID );
	if ( send( sock, buf, strlen( buf ), 0) != strlen( buf ) ) {
		DieWithError("Failed to send SGID command.");
	}
  
  	// Get response
	memset( buf, 0, sizeof( buf ) );
	if ( ( bytesRcvd = recv( sock, buf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
		DieWithError("Failed to set interface GUID. No response.");
	}
  
  	// Check to see if the set GUID command was accepted
	if ( NULL == strstr( buf, "OK" ) ) {
		printf("Interface GUID not accepted. Use defult GUID instead.");
	}

	while ( 1 ) {

		// Check if data is available
		memset( buf, 0, RCVBUFSIZE );
		sprintf( buf, "CDTA\r\n" );
		if ( send( sock, buf, strlen( buf ), 0 ) != strlen( buf ) ) {
			DieWithError("Failed to send CDTA command.");
		}
  
  		// Get response
		memset( buf, 0, RCVBUFSIZE );
		if ( ( bytesRcvd = recv( sock, buf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
			DieWithError("Failed to check for data. No response.");
		}

  		// Check # events waiting
		nEvents = atoi( buf );


		if ( nEvents ) {

			printf("Event(s) available\n");

			// Fetch event(s)
			for ( i=0; i<nEvents; i++ ) {
				
				// fetch data
				memset( buf, 0, RCVBUFSIZE );
				sprintf( buf, "RETR\r\n");
				if ( send( sock, buf, strlen( buf ), 0 ) != strlen( buf ) ) {
					DieWithError("Failed to send RETR command.");
				}

				getResponse( sock, buf );
		
				// Check to see if the event was received OK
				if ( NULL == strstr( buf, "OK" ) ) {
					printf("Event was not received OK. Response = %s", buf );
				}
				
				// Data received is packed as:
				//
				// head,class,type,obid,timestamp,GUID,data0,data1,data2,...........
				//
				// we have to parse the data.
				//

				state = STATE_HEAD;
				cntData = 0;
				p = strtok( buf, "," );
				while( p != NULL ) {

					switch ( state++ ) {

						case STATE_HEAD:				// Get head
							vscp_head = atoi( p );
							break;

						case STATE_CLASS:				// Get class
							vscp_class = atoi( p );
							break;

						case STATE_TYPE:				// Get type
							vscp_type = atoi( p );
							break;

						case STATE_OBID:				// Get obid
							vscp_obid = atol( p );
							break;

						case STATE_TIMESTAMP:			// Get timestamp
							vscp_timestamp = atol( p );
							break;

						case STATE_GUID:				// Get GUID string
							strcpy( strguid, p );
							break;

						default:		// data
							data[ cntData ] = atoi( p );
							if ( cntData  > (512-25 ) ) p = NULL;; // end it all
							cntData++;
							break;				

					} // switch

					p = strtok( NULL, "," );

				} // while

				// Extract the GUID
				memset( guid, 0, sizeof(guid) );
				p = strtok( strguid, ":" );
				for ( j=0; j<16; j++ ) {
					guid[ j ] = atoi( p );
					p = strtok( NULL, ":" );
					if ( NULL == p ) break;
				}

				// Report receied event
				printf("Head=%d obid=%lu timestamp=%lu \n",
							vscp_head,
							vscp_obid,
							vscp_timestamp );
				
				printf("CLASS=%d TYPE=%d\n",
					   		vscp_class,
							vscp_type );
				
				printf("GUID = ");
				for ( j=0; j<16; j++) {
					printf( "%d ", guid[ j ] );
				}
				printf(" MSB -> LSB \n");

				printf("DATA = ");
				for ( j=0; j<cntData; j++) {
					printf("%d ",data[ j ]);
				}
				printf("\n");

			} // for
			
		}
		else {
			usleep( 100 );
		}

	}
	

	close(sock);
  	return EXIT_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// getResponse 
//
//

int getResponse( int sock, char *buf )
{
	char respbuf[ RCVBUFSIZE ];
	int totcnt = 0;
	int cnt;

	do {

		if ( ( cnt = recv( sock, respbuf, RCVBUFSIZE - 1, MSG_PEEK ) ) <= 0 ) {
			return 0;
		}
		else {

			if ( cnt  ) {
				
				if ( ( cnt = recv( sock, respbuf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
					return 0;
				}

				memcpy( buf+totcnt, respbuf, cnt );
				totcnt += cnt;

				if ( NULL != strstr( buf, "+OK" ) ) break;
				if ( NULL != strstr( buf, "-OK" ) ) break;
				
			}
			
		}

	} while ( cnt );

	return totcnt;
	
}


///////////////////////////////////////////////////////////////////////////////
// DieWithError 
//
//

void DieWithError(char *errorMessage)
{
	perror( errorMessage );
	exit( 1 );
}

