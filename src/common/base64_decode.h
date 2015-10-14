/*
* This code originally came from here
*
* http://base64.sourceforge.net/b64.c
*
* with the following license:
*
* LICENCE:        Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.
*
*                Permission is hereby granted, free of charge, to any person
*                obtaining a copy of this software and associated
*                documentation files (the "Software"), to deal in the
*                Software without restriction, including without limitation
*                the rights to use, copy, modify, merge, publish, distribute,
*                sublicense, and/or sell copies of the Software, and to
*                permit persons to whom the Software is furnished to do so,
*                subject to the following conditions:
*
*                The above copyright notice and this permission notice shall
*                be included in all copies or substantial portions of the
*                Software.
*
*                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
*                KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*                WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
*                PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
*                OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
*                OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
*                OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*                SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* VERSION HISTORY:
*               Bob Trower 08/04/01 -- Create Version 0.00.00B
*
* I cleaned it up quite a bit to match the (linux kernel) style of the rest
* of libwebsockets; this version is under LGPL2 like the rest of libwebsockets
* since he explictly allows sublicensing, but I give the URL above so you can
* get the original with Bob's super-liberal terms directly if you prefer.
*/


#pragma once

#ifndef BASE64_DECODE_H
#define BASE64_DECODE_H

int lws_b64_encode_string( const char *in, int in_len, char *out, int out_size );
int lws_b64_decode_string( const char *in, char *out, int out_size );
int lws_b64_selftest( void );

#endif

