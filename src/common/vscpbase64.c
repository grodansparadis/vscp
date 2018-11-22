/* File: vscpbase64.c
 * 
 * Copyright (c) 2004-2013 Sergey Lyubka
 * Copyright (c) 2013-2017 the Civetweb developers
 * Copyright (c) 2017-2018 Ake Hedman, Grodans Paradis AB
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "stdio.h"
#include "string.h"
#include "vscpbase64.h"


////////////////////////////////////////////////////////////////////////////////
// vscp_base64_encode
//

void
vscp_base64_encode( const unsigned char *src, int src_len, char *dst )
{
    static const char *b64 =
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int i, j, a, b, c;

	for (i = j = 0; i < src_len; i += 3) {
		a = src[i];
		b = ((i + 1) >= src_len) ? 0 : src[i + 1];
		c = ((i + 2) >= src_len) ? 0 : src[i + 2];

		dst[j++] = b64[a >> 2];
		dst[j++] = b64[((a & 3) << 4) | (b >> 4)];
		if (i + 1 < src_len) {
			dst[j++] = b64[(b & 15) << 2 | (c >> 6)];
		}
		if (i + 2 < src_len) {
			dst[j++] = b64[c & 63];
		}
	}
	while (j % 4 != 0) {
		dst[j++] = '=';
	}
	dst[j++] = '\0';
}

////////////////////////////////////////////////////////////////////////////////
// b64reverse
//

static unsigned char
b64reverse(char letter)
{
    if ((letter >= 'A') && (letter <= 'Z')) {
        return letter - 'A';
    }
    if ((letter >= 'a') && (letter <= 'z')) {
        return letter - 'a' + 26;
    }
    if ((letter >= '0') && (letter <= '9')) {
        return letter - '0' + 52;
    }
    if (letter == '+') {
        return 62;
    }
    if (letter == '/') {
        return 63;
    }
    if (letter == '=') {
        return 255; // normal end 
    }
    
    return 254;     // error 
}

////////////////////////////////////////////////////////////////////////////////
// vscp_base64_decode
//

int
vscp_base64_decode( const unsigned char *src, 
                        int src_len, 
                        char *dst, 
                        size_t *dst_len )
{
    int i;
    unsigned char a, b, c, d;

    *dst_len = 0;

    for ( i = 0; i < src_len; i += 4 ) {
        
        a = b64reverse( src[i] );
        if (a >= 254) {
            return i;
        }

        b = b64reverse(((i + 1) >= src_len) ? 0 : src[i + 1]);
        if (b >= 254) {
            return i + 1;
        }

        c = b64reverse(((i + 2) >= src_len) ? 0 : src[i + 2]);
        if (c == 254) {
            return i + 2;
        }

        d = b64reverse(((i + 3) >= src_len) ? 0 : src[i + 3]);
        if (d == 254) {
            return i + 3;
        }

        dst[(*dst_len)++] = (a << 2) + (b >> 4);
        if ( c != 255 ) {
            dst[(*dst_len)++] = (b << 4) + (c >> 2);
            if (d != 255) {
                dst[(*dst_len)++] = (c << 6) + d;
            }
        }
    }
    
    return -1;
}
