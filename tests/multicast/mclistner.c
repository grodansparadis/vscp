/* listner.c
 *
 * Use to listen for announce frames on the VSCP announce multicast channel
 *
 * Changed by Ake Hedman, for use with VSCP & Friends
 * Ake Hedman, the VSCP project, <info@vscp.org>
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

 // Format mclisten ["dump|nodump"] [ip-addr-to filter for]
 // for example: mclisten 192.168.1.9 port [interface]

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <vscp.h>

#define DEFAULT_PORT        44444
#define DEFAULT_GROUP       "224.0.23.158"
#define DEFAULT_INTERFACE   ""
#define MSGBUFSIZE          1024

int main( int argc, char *argv[] )
{
    struct sockaddr_in addr;
    int sock, nbytes,addrlen;
    struct ip_mreq mreq;
    unsigned char buf[ MSGBUFSIZE ];
    char *pFilter = NULL;
    u_int yes = 1;
    int bDumpData = 1;
    char group[32];
    char interface[32];
    int port = DEFAULT_PORT;

    if ( argc >= 2 ) {
        if ( NULL != strstr( argv[1], "dump" ) ) {
            bDumpData = 1;
        }
    }

    if ( argc >= 3 ) {
        port = argv[2];
    }

    if ( argc >= 4 ) {
        pFilter = argv[3];
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

    // set up destination address
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl( INADDR_ANY ); // N.B.: differs from sender
    addr.sin_port = htons( DEFAULT_PORT );

    // bind to receive address
    if ( bind( sock, (struct sockaddr *)&addr, sizeof( addr ) ) < 0 ) {
	   perror("bind");
	   return -1;
   }

   // use setsockopt() to request that the kernel join a multicast group
   mreq.imr_multiaddr.s_addr=inet_addr(DEFAULT_GROUP);
   mreq.imr_interface.s_addr=htonl(INADDR_ANY);
   if ( setsockopt( sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                          &mreq, sizeof( mreq ) ) < 0 ) {
	   perror("setsockopt");
       return -1;
   }

   printf("\nListen for multicast events on a VSCP multicast channel %s:%d.\n",
            DEFAULT_GROUP, DEFAULT_PORT );
   printf("usage: mclistner group port interface\n");
   printf("CTRL +C to terminate.\n\n");

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
        printf( "%s,\tport=%d - ", peer, ntohs( addr.sin_port ) );
        printf( "\tclass=%d,type=%d\n", vscp_class, vscp_type );

        if ( bDumpData ) {
            printf("Size of frame=%d.\nFrame=", nbytes );
            for ( int i=0; i<nbytes; i++ ) {
                printf("%02X ", buf[i] );
            }
            printf("\n");
            printf("Class=%d, Type=%d data size=%d\n",
                    ( ( buf[VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB] >> 8) & 0x0f ) +
                        buf[VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB],
                    ( ( buf[VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB] >> 8) & 0x0f ) +
                        buf[VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB],
                    ( ( buf[VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB] >> 8) & 0x0f ) +
                        buf[VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB] );
            printf("Year=%d, Month=%d Day=%d ",
                    ( buf[VSCP_MULTICAST_PACKET0_POS_YEAR_MSB] << 8 ) +
                    buf[VSCP_MULTICAST_PACKET0_POS_YEAR_LSB],
                    buf[VSCP_MULTICAST_PACKET0_POS_MONTH],
                    buf[VSCP_MULTICAST_PACKET0_POS_DAY] );
            printf("Hour=%d, Minute=%d Second=%d \n",
                            buf[VSCP_MULTICAST_PACKET0_POS_HOUR],
                            buf[VSCP_MULTICAST_PACKET0_POS_MINUTE],
                            buf[VSCP_MULTICAST_PACKET0_POS_SECOND] );
            printf("\n");

        }

  }

  return 0;

}
