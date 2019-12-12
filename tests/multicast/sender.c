/* sender.c
 *
 * Changed by Ake Hedman, for use with VSCP & Friends
 * Ake Hedman, Grodans Paradis AB, <akhe@vscp.org>
 *
 * Original header below
 * sender.c -- multicasts "hello, world!" to a multicast group once a second
 *
 * Antony Courtney,	25/11/94
 *
 * http://ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html
 */

 #include <unistd.h>
 #include <time.h>
 #include <string.h>
 #include <stdio.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>

 #define HELLO_PORT 44444
 #define HELLO_GROUP "224.0.23.158"

 int main( int argc, char *argv[] )
 {
   struct sockaddr_in addr;
   int sock, cnt;
   char *message = "Hello, World!";

   // create what looks like an ordinary UDP socket
   if ( ( sock = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 ) {
     perror("socket");
	   return -1;
    }

    // set up destination address
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( HELLO_GROUP );
    addr.sin_port=htons( HELLO_PORT );

    // now just sendto() our destination!
    while ( 1 ) {

      if ( sendto( sock, message, sizeof( message ), 0,
                  (struct sockaddr *) &addr,
		                        sizeof( addr ) ) < 0 ) {
	      perror("sendto");
	      return -1;
	  }

	  sleep(1);

  }

  return 0;
}
