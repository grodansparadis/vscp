// 
// File:   main.c
//
// Copyright (C) 2000-2014 Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com 
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//
//

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <openssl/evp.h>

#include "../../common/version.h"
#include "../../common/vscp.h"

#define MYPORT "3490"	// the port users will be connecting to
#define BACKLOG 10		// how many pending connections queue will hold

void sigchld_handler( int s )
{
	while( waitpid( -1, NULL, WNOHANG ) > 0 );
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*!
	Perform MD5
 * @param pDigest pointer to holder for digest >= 128 bytes
	@param pstr string to encrypt.
	@return Encrypted data.
*/
char *md5( char *pDigest, unsigned char *pstr )
{
	EVP_MD_CTX mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len;

	OpenSSL_add_all_digests();

	md = EVP_get_digestbyname("md5");
	EVP_MD_CTX_init(&mdctx);
	EVP_DigestInit_ex(&mdctx, md, NULL);
	EVP_DigestUpdate(&mdctx, pstr, strlen((const char *) pstr));
	EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
	EVP_MD_CTX_cleanup(&mdctx);

	memset( pDigest, 0, sizeof(pDigest) );
	sprintf( pDigest,
		"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		md_value[0], md_value[1], md_value[2], md_value[3], md_value[4], md_value[5], md_value[6], md_value[7],
		md_value[8], md_value[9], md_value[10], md_value[11], md_value[12], md_value[13], md_value[14], md_value[15]);
	/*
		MD5_CTX *pctx;
		if ( 0 == MD5_Init( pctx ) ) return NULL;
		if ( 0 == MD5_Update( pctx, 
															pstr,
											strlen( (const char *)pstr ) ) ) return NULL;
		unsigned char buf[ 17 ];
		if ( 0 == MD5_Final( buf, pctx ) ) return NULL;	
		sprintf( m_szDigest, 
			"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\0",
			buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],
			buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15] ); 
	 */

	return pDigest;
}

/*
 * 
 */
int main(int argc, char** argv)
{
	int rv = FALSE;
	int bPasswordOK = FALSE;
	int yes=1;
	//char yes='1'; // Solaris people use this
	pid_t pid,sid;
	struct addrinfo hints, *servinfo, *p;
	int sockfd;
	int new_fd;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	struct sigaction sa;

	// Fork off the parent process 
	pid = fork();
	if ( pid < 0 ) {
		exit( EXIT_FAILURE );
	}
	
	// If we got a good PID, then
	//   we can exit the parent process. 
	if ( pid > 0 ) {
		exit( EXIT_SUCCESS );
	}
	
	// Change the file mode mask 
    umask(0);
	
	// Open any logs here 
        
	// Create a new SID for the child process 
	sid = setsid();
	if ( sid < 0 ) {
		// Log any failure 
		exit(EXIT_FAILURE);
    }
	
	// Change the current working directory 
    if ( (chdir("/")) <  0 ) {
		// Log any failure here 
        exit(EXIT_FAILURE);
    }
	
	// Close out the standard file descriptors 
    close( STDIN_FILENO );
    close( STDOUT_FILENO );
    close( STDERR_FILENO );
	
	
	
	// first, load up address structs with getaddrinfo():
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me
	//getaddrinfo( NULL, MYPORT, &hints, &res );
	
	if ( ( rv = getaddrinfo( NULL, MYPORT, &hints, &servinfo ) ) != 0 ) {
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror(rv) );
		return 1;
	}

	// make a socket:
	//sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
	
	
	
	 // loop through all the results and bind to the first we can
	for ( p = servinfo; p != NULL; p = p->ai_next ) {
		
		if ( ( sockfd = socket( p->ai_family, 
								p->ai_socktype,
								p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		
		// lose the pesky "Address already in use" error message
		if ( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int) ) == -1 ) {
			perror("setsockopt");
			exit(EXIT_FAILURE);
		} 
		
		if ( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
							sizeof(int)) == -1 ) {
			perror("setsockopt");
			exit(1);
		}
		
		if ( bind( sockfd, p->ai_addr, p->ai_addrlen) == -1 ) {
			close( sockfd );
			perror("server: bind");
			continue;
		}
		break;
	}
	
	if ( p == NULL ) {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}
	
	freeaddrinfo( servinfo ); // all done with this structure

	// bind it to the port we passed in to getaddrinfo():
	//bind( sockfd, res->ai_addr, res->ai_addrlen );
	
	//listen( sockfd, BACKLOG );
	
	if ( listen( sockfd, BACKLOG ) == -1 ) {
		perror("listen");
		exit(1);
	}
	
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	
	printf("server: waiting for connections...\n");
	 
	// now accept an incoming connection:
	//addr_size = sizeof their_addr;
	//new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
	        
    // Work loop
    while (1) {
		
		char buf[512];
		
		int sin_size = sizeof(their_addr) ;
		
		new_fd = accept( sockfd, (struct sockaddr *)&their_addr, &sin_size );
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		
		inet_ntop( their_addr.ss_family,
						get_in_addr((struct sockaddr *)&their_addr),
						buf, 
						sizeof(buf) );
		
		printf("server: got connection from %s\n", buf);
		if ( !fork() ) {		// this is the child process
			
			// Child doesn't need the listener
			close(sockfd);	
			
			sprintf( buf, "%s\n", "Welcome to bumblebee" );
			if ( -1 == send( new_fd, buf, strlen( buf ), 0 ) ) {
				perror("send");
				close(new_fd);
				exit(0);
			}
			
			sprintf( buf, "%s\n", VSCPD_DISPLAY_VERSION );
			send( new_fd, buf, strlen(buf), 0 );
			
			sprintf( buf, "%s", VSCPD_COPYRIGHT );
			send( new_fd, buf, strlen(buf), 0 );
			
			// Command loop
			while ( 1 ) {
				
				int len,i;
				char rcvBuf[128], saveBuf[128];
				
				if ( 0 == (len = recv( new_fd,
										rcvBuf,
										sizeof( rcvBuf ), 
										/*MSG_WAITALL*/ 0 ) ) ) {
					// Remote closed connection
					perror("Remote closed connection");
					close(new_fd);
					exit(0);
				}
				
				// We only deal with upper case strings
				for ( i=0; i<strlen(rcvBuf); i++){
					rcvBuf[i] &= 0xDF;
				}
				
				// Save last command
				strncpy( saveBuf, rcvBuf, sizeof(rcvBuf) );
				
				//send( new_fd, rcvBuf, strlen(rcvBuf), 0 );
repeat:				
				
				// End this session
				if ( NULL!= strstr( rcvBuf, "QUIT") ) {
					send( new_fd, "+OK\r\n", 5, 0 );
					perror("Command quit received");
					close(new_fd);
					exit(0);
				}
				// repeat last command
				else if ( NULL!= strstr( rcvBuf, "+") ) {
					strncpy( rcvBuf, saveBuf, sizeof(rcvBuf) );
					goto repeat;
				}
				// No operation
				else if ( NULL!= strstr( rcvBuf, "NOOP") ) {
					rv = TRUE;
				}
				// User named
				else if ( NULL!= strstr( rcvBuf, "USER") ) {
					
				}
				// Password
				else if ( NULL!= strstr( rcvBuf, "PASS") ) {
					
				}
				// Send event
				else if ( NULL!= strstr( rcvBuf, "SEND") ) {
					
				}
				// Fetch one or more event(s) from event inqueue
				else if ( NULL!= strstr( rcvBuf, "RETR") ) {
					
				}
				// Enter receive loop. Only leave when connection
				// terminates
				else if ( NULL!= strstr( rcvBuf, "RCVLOOP") ) {
					
				}
				// Check if there is data in the event inqueue
				else if ( NULL!= strstr( rcvBuf, "CDTA") ) {
					
				}
				// Clear the event inqueue
				else if ( NULL!= strstr( rcvBuf, "CLRA") ) {
					
				}
				// Get transmission statistics
				else if ( NULL!= strstr( rcvBuf, "STAT") ) {
					
				}
				// Get status information. 
				else if ( NULL!= strstr( rcvBuf, "INFO") ) {
					
				}
				// Get channel id
				else if ( NULL!= strstr( rcvBuf, "CHID") ) {
					
				}
				// Set GUID for channel. 
				else if ( NULL!= strstr( rcvBuf, "SGID") ) {
					
				}
				// Get GUID for channel. 
				else if ( NULL!= strstr( rcvBuf, "GGID") ) {
					
				}
				// Display version information
				else if ( NULL!= strstr( rcvBuf, "VERS") ) {
					
				}
				// Set incoming filter
				else if ( NULL!= strstr( rcvBuf, "SFLT") ) {
					
				}
				// Set incoming mask
				else if ( NULL!= strstr( rcvBuf, "SMSK") ) {
					
				}
				// Give command help
				else if ( NULL!= strstr( rcvBuf, "HELP") ) {
					rv = TRUE;
					
				}
				// This command is unknown
				else {
					send( new_fd, "+OK - Unknown command\r\n", 23, 0 );
				}
				
				if ( rv ) {
					send( new_fd, "+OK\r\n", 5, 0 );
					rv = FALSE;
				} 

			}
		}
		
		close(new_fd); // parent doesn't need this 
	}

	return( EXIT_SUCCESS );
}

