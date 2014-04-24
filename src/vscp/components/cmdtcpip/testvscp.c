////////////////////////////////////////////////////////////////////////////////
// Simple VSCP TCP/IP sample
//
// Code sceleton fetched from 
// http://cs.baylor.edu/~donahoo/practical/CSockets/textcode.html
// 
// akhe@eurosource.se - http://www.vscp.org
//

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define USERNAME "admin"                              // logon username
#define PASSWORD "secret"                             // logon password
#define GUID "0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15"  // GUID to use
#define RCVBUFSIZE 2048   // TCP/IP received buffer size

// Prototypes

void DieWithError(char *errorMessage);  /* Error handling function */


//////////////////////////////////////////////////////////////////////////////
// main
//
// address class type data [port]
//

int main( int argc, char *argv[] )
{
  int sock;                       // Socket descriptor 
  struct sockaddr_in ServerAddr;  // Server address 
  unsigned short ServerPort;      // Server port 
  char *szServerIP;               // Server IP address (dotted quad) 
  char *szClass;                  // VSCP class
  char *szType;			  // VSCP type
  char *szData;			  // VSCP data
  
  char buf[ RCVBUFSIZE ];
  int bytesRcvd, totalBytesRcvd;  // Bytes read in single recv() 
                                  //	and total bytes read 

    
  // Test for correct number of arguments
  if ( ( argc < 5 ) || ( argc > 6 ) ) {
    fprintf(stderr, "Usage: %s <Server IP> <class> <type> <data1,data2,data3,....>. [port]\n",
	    argv[0]);
    exit(1);
  }
  
  szServerIP = argv[ 1 ];           // server IP address (dotted quad) 
  szClass = argv[ 2 ];      // VSCP class 
  szType = argv[ 3 ];
  szData = argv[ 4 ];
  
  if ( 6 == argc ) {
    ServerPort = atoi( argv[ 5 ] ); // Use given port, if any 
  }
  else {
    ServerPort = 9598;  // VSCP server standard port
    printf("port=%d\n", ServerPort);
  }
  
  // Create a a socket
  if ( ( sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 ) {
    DieWithError("socket() failed");
  }
  
  // Construct the server address structure 
  memset( &ServerAddr, 0, sizeof( ServerAddr ) );
  ServerAddr.sin_family      = AF_INET;            
  ServerAddr.sin_addr.s_addr = inet_addr( szServerIP );   
  ServerAddr.sin_port        = htons( ServerPort ); 
  
  // Establish the connection to the echo server
  if ( connect( sock, 
		(struct sockaddr *) &ServerAddr, 
		sizeof( ServerAddr ) ) < 0 ) {
    DieWithError("connect() failed");
  }
  
  // Check to see that we are connected
  if ( ( bytesRcvd = recv( sock, buf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
    DieWithError("recv() failed or connection closed prematurely");
  }
  
  printf( buf );
  
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
  if ( ( bytesRcvd = recv( sock, buf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
    DieWithError("Failed to set password.No response.");
  }

  // Check to see if username was accepted
  if ( NULL == strstr( buf, "OK" ) ) {
    DieWithError("Password.wa not accepted");
  }
  
  
  
  // * * * Set interface GUID * * *
  
  
  // Set interface GUID
  sprintf( buf, "SGID %s\r\n", GUID );
  if ( send( sock, buf, strlen( buf ), 0) != strlen( buf ) ) {
    DieWithError("Failed to send SGID command.");
  }
  
  // Get response
  if ( ( bytesRcvd = recv( sock, buf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
    DieWithError("Failed to set interface GUID. No response.");
  }
  
  // Check to see if the set GUID command was accepted
  if ( NULL == strstr( buf, "OK" ) ) {
    printf("Interface GUID not accepted.Use defult GUID instead.");
  }
  
  
  
  // * * * Send Event * * *
  
  
  // Send event
  sprintf( buf, 
	  "SEND 0,%s,%s,0,0,-,%s\r\n",
	  szClass,
	  szType,
	  szData );
  if ( send( sock, buf, strlen( buf ), 0) != strlen( buf ) ) {
    DieWithError("Failed to send event");
  }
  
  // Get response
  if ( ( bytesRcvd = recv( sock, buf, RCVBUFSIZE - 1, 0 ) ) <= 0 ) {
    DieWithError("Failed to send event. No response.");
  }
  
  // Check to see if the send event was accepted
  if ( NULL == strstr( buf, "OK" ) ) {
    printf("Send event command failed..");
  }
  
  
  close(sock);
  exit(0);
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

