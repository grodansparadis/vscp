/* listner.c
 *
 * Use to listen for announce frames on the VSCP announce multicast channel
 *
 * Changed by Ake Hedman, for use with VSCP & Friends
 * Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
 *
 * Original header below
 *
 * listener.c -- joins a multicast group and echoes all data it receives from
 *		the group to its stdout...
 *
 * Antony Courtney,	25/11/94
 * Modified by: Frédéric Bastien (25/03/04)
 * to compile without warning and work correctly
 *
 * http://ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html
 */

 // Format listen ["dump|nodump"] [ip-addr-to filter for]
 // for example: listen dump 192.168.1.9

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include <vscp.h>

#define HELLO_PORT    9598
#define HELLO_GROUP   "224.0.23.158"
#define MSGBUFSIZE    1024

int main( int argc, char *argv[] )
{
    struct sockaddr_in addr;
    int sock, nbytes,addrlen;
    struct ip_mreq mreq;
    unsigned char buf[ MSGBUFSIZE ];
    char *pFilter = NULL;
    u_int yes = 1;
    int bDumpData = 0;

    if ( argc >= 2 ) {
        if ( NULL != strstr( argv[1], "dump" ) ) {
            bDumpData = 1;
        }
    }

    if ( argc >= 3 ) {
        pFilter = argv[2];
    }

    // create what looks like an ordinary UDP socket
    if ( ( sock = socket( AF_INET,SOCK_DGRAM, 0 ) ) < 0 ) {
	    perror("socket");
	    return -1;
    }

    // allow multiple sockets to use the same PORT number
    if  ( setsockopt( sock, SOL_SOCKET,SO_REUSEADDR, &yes, sizeof(yes) ) < 0 ) {
        perror("Reusing ADDR failed");
        return -1;
    }

    printf("Listen for announce events on the VSCP announce multicast channel.\n");
    printf("usage: announce-listner dump|nodump ip-address-to-filter-on\n");
    printf("CTRL +C to terminate.\n\n");

    // set up destination address
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl( INADDR_ANY ); // N.B.: differs from sender
    addr.sin_port = htons( HELLO_PORT );

    // bind to receive address
    if ( bind( sock, (struct sockaddr *)&addr, sizeof( addr ) ) < 0 ) {
	   perror("bind");
	   return -1;
   }

   // use setsockopt() to request that the kernel join a multicast group
   mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);
   mreq.imr_interface.s_addr=htonl(INADDR_ANY);
   if ( setsockopt( sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                          &mreq, sizeof( mreq ) ) < 0 ) {
	   perror("setsockopt");
       return -1;
   }



   // now just enter a read-print loop
   while ( 1 ) {
       addrlen = sizeof( addr );
	    if ( ( nbytes = recvfrom( sock,
                                    buf,
                                    MSGBUFSIZE,
                                    0,
			                        (struct sockaddr *)&addr,
                                    &addrlen ) ) < 0 ) {
            perror("recvfrom");
	        return -1;
	    }

        const char *peer;
        peer = inet_ntoa( addr.sin_addr  );

        if ( NULL != pFilter ) {
            if ( NULL == strstr( peer, pFilter ) ) continue;
        }

        int vscp_class =
            ( (int)buf[VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB] << 8 ) +
                        buf[VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB];

        int vscp_type =
            ((int)buf[VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB] << 8 ) +
                       buf[VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB];

        printf( "Announce frame received from ");
        printf( "%s, port=%d - ", peer, ntohs( addr.sin_port ) );
        printf( "class=%d,type=%d\n", vscp_class, vscp_type );

        if ( bDumpData ) {
            printf("Size of frame=%d.\nFrame=", nbytes );
            for ( int i=0; i<nbytes; i++ ) {
                printf("%02X ", buf[i] );
            }
            printf("\n\n");
        }

  }

  return 0;

}
