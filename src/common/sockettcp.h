///////////////////////////////////////////////////////////////////////////////
// sockettcp.h:
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2004-2013 Sergey Lyubka
// Copyright (c) 2013-2017 the Civetweb developers
//
// Adopted for VSCP, Small changes  additions
// Copyright (c) 2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __sockettcp_h
#define __sockettcp_h

#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sys/types.h>
#if defined(_WIN32)
#include <Ws2ipdef.h>
#else
#include <sys/socket.h>
#include <netinet/ip.h>
#include <poll.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/engine.h>
#include <openssl/conf.h>
#include <openssl/dh.h>
#include <openssl/bn.h>
#include <openssl/opensslv.h>

#if !defined(FALSE)
#define FALSE   (0)
#endif

#if !defined(TRUE)
#define TRUE   (1)
#endif

/*
    If multithreading part of  ssl is initialized elsewhere
    SSL_ALREADY_INITIALIZED should be defined when compiling
    sockettcp.c
*/

#define STCP_NO_SSL                     (0)     /* Unsecure connect */
#define STCP_SSL                        (1)     /* Secure connection SSL */

#define STCP_CONN_STATE_UNDEFINED       (0)
#define STCP_CONN_STATE_CONNECTED       (3)    
#define STCP_CONN_STATE_TOCLOSE         (6)
#define STCP_CONN_STATE_CLOSING         (7)
#define STCP_CONN_STATE_CLOSED          (8)

#define STCP_CONFIG_TCP_NODELAY         (0)     /* Set to 1 to enable. If set the socket option will disable 
                                                   Nagle's algorithm on the connection which means that packets
                                                   will be sent as soon as possible instead of waiting for a 
                                                   full buffer or timeout to occur.
                                                   (Not the same as socket option typedef TCP_NODELAY) 
                                                */

#define STCP_SSL_PROTOCOL_VERSION        (0)
#define STCP_SSL_DO_VERIFY_PEER          (0)    /* 0 == no, 1 == mandatory, 2 == optional */
#define STCP_SSL_DEFAULT_VERIFY_PATHS    (1)    /* 1 == yes */
#define STCP_SSL_VERIFY_DEPTH            (9)
#define STCP_SSL_CIPHER_LIST             "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256"
//#define STCP_SSL_CIPHER_LIST            "kEECDH:kEDH:kRSA:AESGCM:AES256:AES128:3DES:SHA256:SHA84:SHA1:!aNULL:!eNULL:!EXP:!LOW:!MEDIUM!ADH:!AECDH"
#define STCP_SSL_SHORT_TRUST             (0)

/*
"ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:"
    "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:"
    "DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:"
    "ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:"
    "ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:"
    "ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:"
    "DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:"
    "DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:"
    "!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK"
*/

#define STCP_MAX_REQUEST_SIZE           (16384)
#define STCP_BUF_LEN                    (8192)
#define STCP_LINGER_TIMEOUT             (-2)

#ifndef STCP_WRITE_TIMEOUT
#define STCP_WRITE_TIMEOUT              (1000)      /* Timeout in ms to try to send in */
#endif

#ifndef STCP_TIMEOUT_QUANTUM                        /* in ms */
#define STCP_TIMEOUT_QUANTUM            (200)       /* Default read timout in ms */
#endif                                              /* Smaller will be used  */

#ifndef INVALID_SOCKET
#define INVALID_SOCKET                  (-1)
#endif

/* Common error codedes     */
#define STCP_OK                         (1)
#define STCP_ERROR                      (0)
#define STCP_ERROR_TIMEOUT              (-1)
#define STCP_ERROR_STOPPED              (-2)


/*!
    stcp_client_cert

    Client certificate information (part of web_request_info)
*/


// New nomenclature.
struct stcp_srv_client_cert {
    const char *subject;
    const char *issuer;
    const char *serial;
    const char *finger;
};

/*  
    Unified socket address. For IPv6 support, add IPv6 address structure in
    the union u.
*/
union usa
{
    struct sockaddr sa;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
};

/* 
    Describes a string (chunk of memory).
*/
struct msg
{
    const char *ptr;
    size_t len;
};

/* 
    Describes listening socket, or socket which was accept()-ed by the master
    thread and queued for future handling by the worker thread.
*/
struct socket
{
    int sock;                   /* Listening socket */
    union usa lsa;              /* Local socket address */
    union usa rsa;              /* Remote socket address */
    unsigned char is_ssl;       /* Is port SSL-ed */
    unsigned long id;           /* Can be used as an identifier */
};

/*!
    Options for secure connections
*/
struct stcp_secure_options 
{
    /*    
        * * * Client specific * * *   
    */
    const char *client_cert_path;    /* Client certificat path */
    const char *server_cert_path;    /* Default locations for trusted CA certificates (file in pem format) */

    /* ------------------------------------------------------- */

    /*
        * * '  Server specific * * * 
    */
    struct stcp_srv_client_cert *srv_client_cert;   /* Certificat for remote client */
    
    const char *pem;            /* Client/server path to combined key and cert */
    const char *chain;          /*  */
    const char *ca_path;        /* CA cert path for peers */
    const char *ca_file;        /* CA file fo rpeers */
    int  protocol_version;      /* 0 == default */
    int short_trust;            /* 0 == no */
    int verify_peer;            /* 0 == no, 1 == yes, 2 == optional */
    int default_verify_path;    /* 0 == no, 1 == yes */
    int verify_depth;           /* Set to zero for default */
    char *chipher_list;         /* NULL for default */

    /* 
        Ths flag should be set to non zero if the multi thread 
        initialization for openssl 1.0 locks has been done already.
    */
    int bNOInitMT;
};



/*!
    Describes every connection
*/
struct stcp_connection
{
    uint8_t conn_state;         /* STCP_CONN_STATE_... */

    struct socket client;       /* Connected client */

    time_t birth;               /* Time when connection was established */
    int must_close;             /* 1 if connection must be closed */
    volatile int stop_flag;     /* Should we stop event loop */
    
    // Request info
    char remote_addr[48];       /* Client's IP address as a string. */
    int remote_port;            /* Client's port */

    SSL *ssl;                   /* SSL descriptor */
    SSL_CTX *ssl_ctx;           /* SSL context for client connections */
};

/*! 
    Configuration settings
*/
struct server_context
{
    SSL_CTX *ssl_ctx;                   /* SSL context */

    struct socket *listening_sockets;
    struct pollfd *listening_socket_fds;
    unsigned int num_listening_sockets;
    
    int config_tcp_nodelay;             /* 
                                            Deafult = 0
                                            Set to 1 to enable. If set the socket option will disable 
                                            Nagle's algorithm on the connection which means that packets
                                            will be sent as soon as possible instead of waiting for a 
                                            full buffer or timeout to occur.
                                            (Not the same as socket option typedef TCP_NODELAY)
                                        */                                          
};


/*!
 * Init ssl multithreading locks
 */

int
stcp_init_mt_ssl(void);


/*!
    Init. SSL  (client or server)
    @param security_opts Settings for ssl
    @param ssl_ctx ssl context returned here on success    
    @return Non zero on success, zero on failure.
*/
int
stcp_init_ssl( SSL_CTX *ssl_ctx, 
                struct stcp_secure_options *secure_opts );

/*!
    Clean up SSL 
*/
void
stcp_uninit_ssl( void );

/*!
 *  Connect (unsecurely) to remote
 */
struct stcp_connection *
stcp_connect_remote( const char *hostip,
                        int port,
                        int timeout );

/*!
 *  Connect securely to remote
 */

struct stcp_connection *
stcp_connect_remote_secure( const char *host,
                                int port,
                                struct stcp_secure_options *secure_opts,
                                int timeout );

/*!
 * Close connection
 */ 
void
stcp_close_connection( struct stcp_connection *conn );


char
stcp_getc( struct stcp_connection *conn );


/*!
 * Read data from remote.
 * 
 * @param conn Connection to read from.
 * @buf Buffer to read data into.
 * @len Size of buffer.
 * @mstimeout Timeout in milliseconds for read (>=0 use timeout -1 - use 
 *              default timeout.).
 * @return    >= 0 Number of read characters. 
 *            -1= timeout, 
 *            -2 = stopped.
 */
int
stcp_read( struct stcp_connection *conn, void *buf, size_t len, int mstimeout );


/*!
 * Write data to remote.
 * 
 * @param conn Connection to read from.
 * @buf Buffer with data to write.
 * @len Number of chars to write.
 * @return 0 on error, otherwise number of characters written.
 */

int
stcp_write( struct stcp_connection *conn, const void *buf, size_t len );


/*!
 * Poll for action on an array with file descriptors
 * @param pfd Array with socket descriptors
 * @param n number of descriptors
 * @param mstimeout Number of milliseconds to wait or action
 * @stop_server Pointer to variable that if set to non zero will return
 *              immediately.
 * @return >0 success (descriptor causing action), -1 timeout, -2 stopped
 */
int
stcp_poll( struct pollfd *pfd,
                unsigned int n,
                int mstimeout,
                volatile int *stop_server );




// ----------------------------------------------------------------------------




/*!
 * Set up listening context
*/

int
stcp_listening( struct server_context *srv_ctx, 
                    const char *str_listening_port );

/*!
 * Accept new connection (called after poll)
 * 
 * @parm srv_ctx Pointer to server context (can be NULL)
 * @parm listening socket to accept
 * @param socket [out] Accepted connection
 * @return 1 on success, 0 on failure.
 * 
 */

int
stcp_accept( struct server_context *srv_ctx,
                const struct socket *listener, 
                struct socket *psocket );               

/*!
 * Close all listening sockets
 * 
 * @parm srv_ctx Pointer to server context
 */
void

stcp_close_all_listening_sockets( struct server_context *srv_ctx );

/*!
    INit data for a connected client (after accept)
    @param conn Pointer to client connection object
*/
void stcp_init_client_connection( struct stcp_connection *conn,
                                    struct stcp_secure_options *secure_opts );

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // __sockettcp_h