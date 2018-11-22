/* File: base64.h
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


#ifndef VSCP_BASE64
#define VSCP_BASE64

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * vscp_base64_encode
 * 
 * Encode data into BASE64
 * 
 * @param src Pointer to data to encode
 * @param int Length of data to encode.
 * @param dst Pointer to buffer that will receive result.
 * 
 */
void
vscp_base64_encode( const unsigned char *src, int src_len, char *dst );

/*
 * vscp_base64_decode
 * 
 * Decode BASE64 data.
 * 
 * @param src Pointer to BASE64 string to decode.
 * @param Length of BASE64 string to decode.
 * @param dst Buffer for result.
 * @param dst_len Size of result buffer.
 * @return 0-n on error where n is the error point. -1 if OK 
 * 
 */
int
vscp_base64_decode( const unsigned char *src, int src_len, char *dst, size_t *dst_len );


#ifdef __cplusplus
}
#endif


#endif