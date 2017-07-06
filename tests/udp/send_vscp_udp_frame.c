/*
    Simple send VSCP udp frame client

    [host] [port] [ftype] [cnt] [bReply]

    host    - IP address for remote host (defaults to 127.0.0.1).
    port    - Port for remote host (default to 33333).
    ftype   - Frame type (defaults to type = 0 ).
    cnt     - Number of frames to send.
    bReply  - 0 or 1 for reply waiting.
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

#define DEFAULT_SERVER      "127.0.0.1" // Server to connect to
#define DEFAULT_FRAME_TYPE  0
#define DEFAULT_COUNT       1

#define BUFLEN  1024                // Max length of buffer

// Default key for the VSCP server
uint8_t key[] = { 0xA4,0xA8,0x6F,0x7D,0x7E,0x11,0x9B,0xA3,
                  0xF0,0xCD,0x06,0x88,0x1E,0x37,0x1B,0x98,
                  0x9B,0x33,0xB6,0xD6,0x06,0xA8,0x63,0xB6,
                  0x33,0xEF,0x52,0x9D,0x64,0x54,0x4F,0x8E };

void die(char *s)
{
    perror(s);
    exit(1);
}

// Temperature measurement
int makeFrameTypeUnEncrypted( unsigned char *frame )
{
    // Frame type, Type 0, unencrypted
    frame[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] = VSCP_ENCRYPTION_NONE;

    // Head
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] = 0;

    // Timestamp
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 1 ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 2 ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 3 ] = 0;

    // UTC time
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);

    // Date / time block 1956-11-02 04:23:52 GMT
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_MSB ] = ((1900 + tm.tm_year ) >> 8) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_LSB ] = (1900 + tm.tm_year) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_MONTH ] = tm.tm_mon;
    frame[ VSCP_MULTICAST_PACKET0_POS_DAY ] = tm.tm_mday;
    frame[ VSCP_MULTICAST_PACKET0_POS_HOUR ] = tm.tm_hour;q
    frame[ VSCP_MULTICAST_PACKET0_POS_MINUTE ] = tm.tm_min;
    frame[ VSCP_MULTICAST_PACKET0_POS_SECOND ] = tm.tm_sec;

    // Class = 1040 Measurement String
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB ] = 0x04;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB ] = 0x10;

    // Type = Temperature = 6
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB ] = 0x00;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB ] = 0x06;

    // GUID - dummy
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID ] = 0x00;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 1 ] = 0x01;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 2 ] = 0x02;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 3 ] = 0x03;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 4 ] = 0x04;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 5 ] = 0x05;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 6 ] = 0x06;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 7 ] = 0x07;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 8 ] = 0x08;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 9 ] = 0x09;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 10 ] = 0x0A;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 11 ] = 0x0B;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 12 ] = 0x0C;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 13 ] = 0x0D;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 14 ] = 0x0E;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 15 ] = 0x0F;

    // Size
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB ] = 13;

    // Data  Sensor index=2, Zone=1, sunzone2", Unit=1 (Celsius)
    // Temperature = 123.45678
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA ] = 0x02;       // Sensor idx = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 1 ] = 0x01;   // Zone = 1
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 2 ] = 0x02;   // Subzone = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 3 ] = 0x01;   // Unit = 1 Degrees Celsius
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 4 ] = 0x31;   // "1"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 5 ] = 0x32;   // "2"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 6 ] = 0x33;   // "3"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 7 ] = 0x2E;   // "."
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 8 ] = 0x34;   // "4"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 9 ] = 0x35;   // "5"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 10 ] = 0x36;  // "6"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 11 ] = 0x37;  // "7"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 12 ] = 0x38;  // "8"

    // Calculate CRC
    crc framecrc = crcFast( (unsigned char const *)frame + 1,
                    VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 );

    printf("CRC = %d\n", framecrc );

    // CRC
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 ] = ( framecrc >> 8 ) & 0xff;
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 1 ] = framecrc & 0xff;

    printf("Frame = ");
    for ( int i=0; i<50; i++ ) {
        printf("%02x ", frame[i] );
    }
    printf("\n");

    return (1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 2);
}



// Temperature measurement encrypted with AES128
int makeFrameTypeEncrypted( uint8_t type, unsigned char *frame )
{
    // Frame type, Type 0, unencrypted
    frame[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] = type;

    // Head
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] = 0;

    // Timestamp
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 1 ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 2 ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 3 ] = 0;

    // UTC time
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);

    // Date / time block 1956-11-02 04:23:52 GMT
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_MSB ] = (1900 + tm.tm_year) >> 8) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_LSB ] = (1900 + tm.tm_year) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_MONTH ] = tm.tm_mon;
    frame[ VSCP_MULTICAST_PACKET0_POS_DAY ] = tm.tm_mday;
    frame[ VSCP_MULTICAST_PACKET0_POS_HOUR ] = tm.tm_hour;q
    frame[ VSCP_MULTICAST_PACKET0_POS_MINUTE ] = tm.tm_min;
    frame[ VSCP_MULTICAST_PACKET0_POS_SECOND ] = tm.tm_sec;

    // Class = 1040 Measurement String
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB ] = 0x04;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB ] = 0x10;

    // Type = Temperature = 6
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB ] = 0x00;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB ] = 0x06;

    // GUID - dummy
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID ] = 0x00;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 1 ] = 0x01;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 2 ] = 0x02;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 3 ] = 0x03;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 4 ] = 0x04;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 5 ] = 0x05;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 6 ] = 0x06;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 7 ] = 0x07;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 8 ] = 0x08;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 9 ] = 0x09;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 10 ] = 0x0A;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 11 ] = 0x0B;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 12 ] = 0x0C;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 13 ] = 0x0D;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 14 ] = 0x0E;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 15 ] = 0x0F;

    // Size
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB ] = 13;

    // Data  Sensor index=2, Zone=1, sunzone2", Unit=1 (Celsius)
    // Temperature = 123.45678
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA ] = 0x02;       // Sensor idx = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 1 ] = 0x01;   // Zone = 1
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 2 ] = 0x02;   // Subzone = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 3 ] = 0x01;   // Unit = 1 Degrees Celsius
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 4 ] = 0x31;   // "1"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 5 ] = 0x32;   // "2"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 6 ] = 0x33;   // "3"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 7 ] = 0x2E;   // "."
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 8 ] = 0x34;   // "4"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 9 ] = 0x35;   // "5"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 10 ] = 0x36;  // "6"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 11 ] = 0x37;  // "7"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 12 ] = 0x38;  // "8"

    // Calculate CRC
    crc framecrc = crcFast( (unsigned char const *)frame + 1,
                    VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 );

    printf("CRC = %d\n", framecrc );

    // CRC
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 ] = ( framecrc >> 8 ) & 0xff;
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 1 ] = framecrc & 0xff;

    printf("Frame = ");
    for ( int i=0; i<50; i++ ) {
        printf("%02x ", frame[i] );
    }
    printf("\n");

    // Encrypt frame
    uint8_t outframe[1024];
    memset( outframe, 0, sizeof(outframe) );

    uint8_t iv[16];
    getRandomIV( iv, 16 );

    printf("IV = ");
    for ( int i=0; i<16; i++ ) {
        printf("%02x ", iv[i] );
    }
    printf("\n");

    size_t padlen = VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 2;
    padlen = padlen + ( 16 - (padlen % 16 ) );

    AES_CBC_encrypt_buffer( AES128,
                            outframe,
                            frame + 1,
                            padlen + 1,
                            key,
                            iv );

    // Replace with encrypted version
    memcpy( frame + 1, outframe, padlen );

    // Append iv
    memcpy( frame + 1 + padlen,
                iv, 16 );

    printf("Encrypted Frame = ");
    for ( int i=0; i<padlen+16; i++ ) {
        printf("%02x ", frame[i] );
    }
    printf("\n");

    return (padlen + 16 + 1);
}

int main( int argc, char* argv[] )
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    unsigned char udpframe[ BUFLEN ];
    unsigned char buf[ BUFLEN ];

    char *pServer = DEFAULT_SERVER;
    int port = VSCP_DEFAULT_UDP_PORT;
    int FrameType = DEFAULT_FRAME_TYPE;
    int cnt = DEFAULT_COUNT;
    int bReply = 0;

    // Init. CRC data
    crcInit();

    if ( argc > 1 ) {

        // Host ("127.0.0.1"")
        if ( argc >= 2 ) {
            pServer = argv[1];
        }

        // Port 9598
        if ( argc >= 3 ) {
            port = atoi( argv[2] );
        }

        // Frame type
        if ( argc >= 4 ) {
            FrameType = atoi( argv[3] );
            if ( FrameType > 1 ) {
                printf("Frametype is invalid. Can be 0-1.");
                die("frametype");
            }
        }

        // Count
        if ( argc >= 5 ) {
            cnt = atoi( argv[4] );
            if ( !cnt ) {
                printf("Count must be greater than zero.");
                die("cnt");
            }
        }

        // bReply
        if ( argc >= 6 ) {
            bReply = atoi(argv[5]);
        }
    }


    if ( -1 == ( s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) ) {
        die("socket");
    }

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(port);

    if ( 0 == inet_aton( pServer , &si_other.sin_addr) )  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    int len;

    switch ( FrameType ) {

        case 1:
            len = makeFrameTypeEncrypted( VSCP_ENCRYPTION_AES128, udpframe );
            break;

        case 2:
            len = makeFrameTypeEncrypted( VSCP_ENCRYPTION_AES192, udpframe );
            break;

        case 3:
            len = makeFrameTypeEncrypted( VSCP_ENCRYPTION_AES256, udpframe );
            break;

        default:
        case 0:
            len = makeFrameTypeUnEncrypted( udpframe );
            break;
    }

    for ( int i=0; i<cnt; i++ ) {

        // send the message
        if ( -1 == sendto( s,
                            udpframe,
                            len,
                            0,
                            (struct sockaddr *) &si_other,
                            slen ) ) {
            die("sendto()");
        }

        if ( bReply ) {

            printf("Sent frame %d - Waiting for response...\n", i );

            //receive a reply and print it

            //clear the buffer by filling null, it might have previously received data
            memset(buf,'\0', BUFLEN);

            //try to receive some data, this is a blocking call
            int rcvcnt;
            if ( -1 == ( rcvcnt = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen ) ) ) {
                die("recvfrom()");
            }

            printf("Got response len = %d\n", rcvcnt );
            for ( int n=0; n<rcvcnt; n++ ) {
                printf("%02X ", buf[n] );
            }
            printf("\n");

            uint16_t class = ( (uint16_t)buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB ] << 8 ) +
                       buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB ];

            uint16_t type = ( (uint16_t)buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB ] << 8 ) +
                        buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB ];

            if ( ( VSCP_CLASS1_ERROR == class ) &&
                 ( VSCP_TYPE_ERROR_SUCCESS == type ) )  {
                printf("Reply = Success.\n");
            }
            else if ( ( VSCP_CLASS1_ERROR == class ) &&
                 ( VSCP_TYPE_ERROR_ERROR == type ) )  {
                printf("Reply = Error.\n");
            }
            else  {
                printf("Unknown reply.\n");
            }
        }
        else {
            printf("Sent frame %d \n", i );
        }


    }

    close(s);
    return 0;
}
