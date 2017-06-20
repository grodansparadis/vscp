/*
    Simple udp server
    http://www.binarytides.com/programming-udp-sockets-c-linux/

    receive_udp_frame [port] ["dump"]
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <crc.h>
#include <aes.h>
#include <vscp.h>

#define BUFLEN          1024   // Max length of buffer
#define DEFAULT_PORT    9999   // The port on which to listen for incoming data

void die(char *s)
{
    perror(s);
    exit(1);
}

int main( int argc, char* argv[] )
{
    int i;
    int port = DEFAULT_PORT;
    struct sockaddr_in si_me, si_other;
    int bDump = 0;
    int s, slen = sizeof(si_other) , recv_len;
    unsigned char buf[BUFLEN];

    // Default key for the VSCP server
    uint8_t key[] = { 0xA4,0xA8,0x6F,0x7D,0x7E,0x11,0x9B,0xA3,
                      0xF0,0xCD,0x06,0x88,0x1E,0x37,0x1B,0x98,
                      0x9B,0x33,0xB6,0xD6,0x06,0xA8,0x63,0xB6,
                      0x33,0xEF,0x52,0x9D,0x64,0x54,0x4F,0x8E };

    // Init. CRC data
    crcInit();

    if ( argc > 1 ) {
        if ( argc >= 2 ) {
            port = atoi( argv[1] );
        }

        if ( argc >= 3 ) {
            if ( NULL != strstr( argv[2], "dump" ) ) {
                bDump = 1;
            }
        }
    }

    // create a UDP socket
    if ( -1 == ( s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) ) {
        die("socket");
    }

    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons( port );
    si_me.sin_addr.s_addr = htonl( INADDR_ANY );

    // bind socket to port
    if ( -1 == bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) ) {
        die("bind");
    }

    printf("Waiting for VSCP UDP frames on port %d... (ctrl+c to abort)\n", port );
    fflush(stdout);

    // keep listening for data
    while ( 1 ) {

        // try to receive some data, this is a blocking call
        memset( buf, 0, sizeof( buf ) );
        if ( -1 == ( recv_len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &si_other, &slen) ) ) {
            die("recvfrom()");
        }

        // print details of the client/peer and the data received
        printf( "Received packet from %s:%d len = %d ",
                        inet_ntoa( si_other.sin_addr ),
                        ntohs( si_other.sin_port ),
                        recv_len );

        uint8_t output[1024];
        uint8_t appended_iv[16];
        size_t real_len = recv_len;

        // Preserve packet type which always is un encrypted
        output[0] = buf[0];

        switch ( GET_VSCP_MULTICAST_PACKET_ENCRYPTION( buf[0] ) ) {

            case VSCP_ENCRYPTION_AES256:
                printf("Encoding = AES256\n");
                memcpy( appended_iv, (buf + recv_len - 16 ), 16 );
                real_len -= 16; // Adjust frame length accordingly
                AES_CBC_decrypt_buffer( AES256,
                                        output+1,
                                        buf+1,
                                        real_len-1,
                                        key,
                                        (const uint8_t *)appended_iv );
                break;

            case VSCP_ENCRYPTION_AES192:
                printf("Encoding = AES192\n");
                memcpy( appended_iv, (buf + recv_len - 16 ), 16 );
                real_len -= 16; // Adjust frame length accordingly
                AES_CBC_decrypt_buffer( AES192,
                                        output+1,
                                        buf+1,
                                        real_len-1,
                                        key,
                                        (const uint8_t *)appended_iv );
                break;

            case VSCP_ENCRYPTION_AES128:
                printf("Encoding = AES128  real-len=%d\n", real_len );
                memcpy( appended_iv, (buf + recv_len - 16 ), 16 );
                real_len -= 16; // Adjust frame length accordingly
                AES_CBC_decrypt_buffer( AES128,
                                        output+1,
                                        buf+1,
                                        real_len-1,
                                        key,
                                        (const uint8_t *)appended_iv );
                break;

            case VSCP_ENCRYPTION_NONE:
                printf("Encoding = none\n");
                memcpy( output, buf, real_len );
                break;

            default:
                printf("Encoding = unknown\n");
                memcpy( output, buf, real_len );
                break;
        }


        if ( bDump && GET_VSCP_MULTICAST_PACKET_ENCRYPTION( buf[0] ) ) {
            printf("IV = ");
            for (i=0; i<16; i++ ) {
                printf("%02X ", appended_iv[i] );
            }
            printf("\n");

            printf("KEY = ");
            for (i=0; i<32; i++ ) {
                printf("%02X ", key[i] );
            }
            printf("\n");

            printf( "Decrypted frame (len=%d): ", real_len );
            for ( i=0; i<real_len; i++ ) {
                printf("%02X ", output[i] );
            }
            printf("\n");
        }

        if ( bDump ) {
            printf( "Frame: " );
            for ( i=0; i<recv_len; i++ ) {
                printf("%02X ", output[i] );
            }
            printf("\n");
            printf("Class=%d, Type=%d data size=%d\n",
                    ( ( output[VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB] >> 8) & 0x0f ) +
                        output[VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB],
                    ( ( output[VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB] >> 8) & 0x0f ) +
                        output[VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB],
                    ( ( output[VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB] >> 8) & 0x0f ) +
                        output[VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB] );
            printf("Year=%d, Month=%d Day=%d \n",
                    ( output[VSCP_MULTICAST_PACKET0_POS_YEAR_MSB] << 8 ) +
                        output[VSCP_MULTICAST_PACKET0_POS_YEAR_LSB],
                    output[VSCP_MULTICAST_PACKET0_POS_MONTH],
                    output[VSCP_MULTICAST_PACKET0_POS_DAY] );
            printf("Hour=%d, Minute=%d Second=%d \n",
                            output[VSCP_MULTICAST_PACKET0_POS_HOUR],
                            output[VSCP_MULTICAST_PACKET0_POS_MINUTE],
                            output[VSCP_MULTICAST_PACKET0_POS_SECOND] );
            printf("\n");
        }

        //now reply the client with the same data
        /*if ( -1 == sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen ) ) {
            die("sendto()");
        }*/
    }

    close(s);
    return 0;
}
