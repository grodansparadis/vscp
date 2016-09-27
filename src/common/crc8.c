/*  
 * crc8.c
 * 
 * Computes a 8-bit CRC 
 * 
 * http://www.rajivchakravorty.com/source-code/uncertainty/multimedia-sim/html/crc8_8c-source.html
 * http://sbs-forum.org/marcom/dc2/20_crc-8_firmware_implementations.pdf
 * http://websvn.hylands.org/filedetails.php?repname=Projects&path=%2Fcommon%2FCrc8.c&sc=1
 *
 * This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 */

#include <stdio.h>
#include "crc8.h"


#define GP  0x107   /* x^8 + x^2 + x + 1 */
#define DI  0x07

static unsigned char crc8_table[256];     /* 8-bit table */
static int made_table=0;

/*
 * Should be called before any other crc function.  
 */
 
void init_crc8()
{
    int i,j;
    unsigned char crc;

    if (!made_table) {
        for ( i=0; i<256; i++ ) {
            crc = i;
            for (j=0; j<8; j++) {
                crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
            }
            crc8_table[i] = crc & 0xFF;
            /* printf("table[%d] = %d (0x%X)\n", i, crc, crc); */
        }
        made_table=1;
    }
}

/*
 * For a byte array whose accumulated crc value is stored in *crc, computes
 * resultant crc obtained by appending m to the byte array
 */
void crc8( unsigned char *crc, unsigned char m )
{
    if ( !made_table ) {
        init_crc8();
    }

    *crc = crc8_table[(*crc) ^ m];
    *crc &= 0xFF;
}