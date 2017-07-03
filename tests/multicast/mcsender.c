/* sender.c
 *
 * Changed by Ake Hedman, for use with VSCP & Friends
 * Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
 *
 * Send VSCP event to VSCP multicast group.
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
 #include <time.h>
 #include <sys/time.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>

 #include <crc.h>
 #include <aes.h>
 #include <vscp.h>

 #define DEFAULT_PORT 44444
 #define DEFAULT_GROUP "224.0.23.158"

 // Default key for the VSCP server
 uint8_t key[] = { 0xA4,0xA8,0x6F,0x7D,0x7E,0x11,0x9B,0xA3,
                   0xF0,0xCD,0x06,0x88,0x1E,0x37,0x1B,0x98,
                   0x9B,0x33,0xB6,0xD6,0x06,0xA8,0x63,0xB6,
                   0x33,0xEF,0x52,0x9D,0x64,0x54,0x4F,0x8E };

////////////////////////////////////////////////////////////////////////////////
// makeFrameTypeEncrypted
//
// Temperature measurement

int makeFrameTypeUnEncrypted(unsigned char *frame)
{
    // Frame type, Type 0, unencrypted
    frame[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] = VSCP_ENCRYPTION_NONE;

    // Head
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] = 0;

    struct timeval tv = GetTimeStamp();

    // Timestamp
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP ] = (tv.tv_usec >> 24 ) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 1 ] = (tv.tv_usec >> 16 ) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 2 ] = (tv.tv_usec >> 8 ) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 3 ] = tv.tv_usec  & 0xff;

    // UTC time
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);

    // Date / time block 1956-11-02 04:23:52 GMT
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_MSB ] = ((1900 + tm.tm_year) >> 8) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_LSB ] = (1900 + tm.tm_year) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_MONTH ] = tm.tm_mon;
    frame[ VSCP_MULTICAST_PACKET0_POS_DAY ] = tm.tm_mday;
    frame[ VSCP_MULTICAST_PACKET0_POS_HOUR ] = tm.tm_hour;
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
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA ] = 0x02; // Sensor idx = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 1 ] = 0x01; // Zone = 1
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 2 ] = 0x02; // Subzone = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 3 ] = 0x01; // Unit = 1 Degrees Celsius
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 4 ] = 0x31; // "1"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 5 ] = 0x32; // "2"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 6 ] = 0x33; // "3"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 7 ] = 0x2E; // "."
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 8 ] = 0x34; // "4"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 9 ] = 0x35; // "5"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 10 ] = 0x36; // "6"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 11 ] = 0x37; // "7"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 12 ] = 0x38; // "8"

    // Calculate CRC
    crc framecrc = crcFast((unsigned char const *) frame + 1,
            VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13);

    printf("CRC = %d\n", framecrc);

    // CRC
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 ] = (framecrc >> 8) & 0xff;
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 1 ] = framecrc & 0xff;

    printf("Frame = ");
    for (int i = 0; i < 50; i++) {
        printf("%02x ", frame[i]);
    }
    printf("\n");

    return (1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 2);
}

////////////////////////////////////////////////////////////////////////////////
// GetTimeStamp
//

struct timeval GetTimeStamp( void ) 
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return tv;
}

////////////////////////////////////////////////////////////////////////////////
// makeFrameTypeEncrypted
//
// Temperature measurement encrypted with AES128
//

int makeFrameTypeEncrypted(uint8_t type, unsigned char *frame)
{
    // Frame type, Type 0, unencrypted
    frame[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] = type;

    // Head
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] = 0;
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] = 0;

    struct timeval tv = GetTimeStamp();

    // Timestamp
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP ] = (tv.tv_usec >> 24 ) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 1 ] = (tv.tv_usec >> 16 ) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 2 ] = (tv.tv_usec >> 8 ) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 3 ] = tv.tv_usec  & 0xff;

    // UTC time
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);

    // Date / time block 1956-11-02 04:23:52 GMT
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_MSB ] = ((1900 + tm.tm_year) >> 8) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_LSB ] = (1900 + tm.tm_year) & 0xff;
    frame[ VSCP_MULTICAST_PACKET0_POS_MONTH ] = tm.tm_mon;
    frame[ VSCP_MULTICAST_PACKET0_POS_DAY ] = tm.tm_mday;
    frame[ VSCP_MULTICAST_PACKET0_POS_HOUR ] = tm.tm_hour;
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
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA ] = 0x02; // Sensor idx = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 1 ] = 0x01; // Zone = 1
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 2 ] = 0x02; // Subzone = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 3 ] = 0x01; // Unit = 1 Degrees Celsius
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 4 ] = 0x31; // "1"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 5 ] = 0x32; // "2"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 6 ] = 0x33; // "3"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 7 ] = 0x2E; // "."
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 8 ] = 0x34; // "4"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 9 ] = 0x35; // "5"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 10 ] = 0x36; // "6"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 11 ] = 0x37; // "7"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 12 ] = 0x38; // "8"

    // Calculate CRC
    crc framecrc = crcFast((unsigned char const *) frame + 1,
            VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13);

    printf("CRC = %d\n", framecrc);

    // CRC
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 ] = (framecrc >> 8) & 0xff;
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 1 ] = framecrc & 0xff;

    printf("Frame = ");
    for (int i = 0; i < 50; i++) {
        printf("%02x ", frame[i]);
    }
    printf("\n");

    // Encrypt frame
    uint8_t outframe[1024];
    memset(outframe, 0, sizeof (outframe));

    uint8_t iv[16];
    getRandomIV(iv, 16);

    printf("IV = ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", iv[i]);
    }
    printf("\n");

    size_t padlen = VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 2;
    padlen = padlen + (16 - (padlen % 16));

    AES_CBC_encrypt_buffer(AES128,
            outframe,
            frame + 1,
            padlen + 1,
            key,
            iv);

    // Replace with encrypted version
    memcpy(frame + 1, outframe, padlen);

    // Append iv
    memcpy(frame + 1 + padlen,
            iv, 16);

    printf("Encrypted Frame = ");
    for (int i = 0; i < padlen + 16; i++) {
        printf("%02x ", frame[i]);
    }
    printf("\n");

    return (padlen + 16 + 1);
}

////////////////////////////////////////////////////////////////////////////////
// main
//

 int main( int argc, char *argv[] )
 {
   struct sockaddr_in addr;
   int sock, cnt;
   unsigned char mcframe[ 1024 ];
   int frameType = 0;

   // Init. CRC data
   crcInit();

   // create what looks like an ordinary UDP socket
   if ( ( sock = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 ) {
     perror("socket");
	   return -1;
    }

    // set up destination address
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr( DEFAULT_GROUP );
    addr.sin_port=htons( DEFAULT_PORT );

    while ( 1 ) {

        int len;

        switch ( frameType ) {

            case 1:
                len = makeFrameTypeEncrypted( VSCP_ENCRYPTION_AES128, mcframe );
                break;

            case 2:
                len = makeFrameTypeEncrypted( VSCP_ENCRYPTION_AES192, mcframe );
                break;

            case 3:
                len = makeFrameTypeEncrypted( VSCP_ENCRYPTION_AES256, mcframe );
                break;

            default:
            case 0:
                len = makeFrameTypeUnEncrypted( mcframe );
                break;
        }

      // now just sendto() our destination!
      if ( sendto( sock, mcframe, len, 0,
                  (struct sockaddr *) &addr,
		                        sizeof( addr ) ) < 0 ) {
	      perror("sendto");
	      return -1;
	  }

	  sleep(1);

  }

  return 0;
}
