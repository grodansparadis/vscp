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
// Adopted for VSCP
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

#define SOCKETTCP_NO_SSL                (0)     // Normal connect
#define SOCKETTCP_SSL                   (1)     // Connect SSL

#define SOCKETTCP_CONN_STATE_UNDEFINED  (0)
#define SOCKETTCP_CONN_STATE_TOCLOSE    (6)
#define SOCKETTCP_CONN_STATE_CLOSING    (7)
#define SOCKETTCP_CONN_STATE_CLOSED     (8)

#define SSL_PROTOCOL_VERSION            (0)
#define SSL_DO_VERIFY_PEER              (0)   // 0 == no, 1 == mandatory, 2 == optional
#define SSL_DEFAULT_VERIFY_PATHS        (1)   // 1 == yes
#define SSL_VERIFY_DEPTH                (9)
#define SSL_CIPHER_LIST                 "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256"
#define SSL_SHORT_TRUST                 (0)

#define MAX_REQUEST_SIZE                (16384)
#define STCP_BUF_LEN (                   8192)
#define LINGER_TIMEOUT                  (-2)

#ifndef SOCKET_TIMEOUT_QUANTUM          // in ms
#define SOCKET_TIMEOUT_QUANTUM          (2000)     // Default read timout in ms
#endif                                             // Smaller will be used 

#ifndef INVALID_SOCKET
#define INVALID_SOCKET                  (-1)
#endif

// Common error codedes
#define STCP_ERROR_TIMEOUT              (-1)
#define STCP_ERROR_STOPPED              (-2)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Unified socket address. For IPv6 support, add IPv6 address structure in
// the
// union u.
union usa
{
    struct sockaddr sa;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
};

// Describes listening socket, or socket which was accept()-ed by the master
// thread and queued for future handling by the worker thread.
struct socket
{
    int sock;                   // Listening socket
    union usa lsa;              // Local socket address
    union usa rsa;              // Remote socket address
    unsigned char is_ssl;       // Is port SSL-ed
    unsigned char ssl_redir;    // Is port supposed to redirect everything to SSL
                                //   port
    unsigned char in_use;       // Is valid
};


struct stcp_secure_client_options {
    const char *host;           /* Host address ip.v4 or ip.v6 */
    int port;                   /* Host port */
    const char *client_cert;    /* Client certificat path */
    const char *server_cert;    /* Server certificat path */
    /* ------------------------------------------------------- */
    char *pem; 
    char *chain;
    char *ca_path;
    char *ca_file;
};

// Connection types
enum {
    CONNECTION_INVALID,
    CONNECTION_CLIENT,
    CONNECTION_SERVER    
};

struct stcp_connection
{
    uint8_t conn_state;         // SOCKTCP_CONN_STATE...
    uint8_t conntype;           // Connection type
    uint16_t read_timeout;      // Read timeout in seconds

    struct socket client;       // Connected client
    
    char *buf;                  // Buffer for received data
    int buf_size;               // Buffer size
    int data_len;               // Total size of data in a buffer

    int must_close;             // 1 if connection must be closed
    volatile int stop_flag;     // Should we stop event loop
    
    // Request info
    char remote_addr[48];       // Client's IP address as a string.
    int remote_port;            // Client's port

    SSL *ssl;                   // SSL descriptor
    SSL_CTX *ssl_ctx;           // SSL context for client connections

    // Secure configuration data
    struct stcp_secure_client_options *secure_opts;
};

// Configuration settings
/*struct stcp_context
{
    
    SSL_CTX *ssl_ctx;                   // SSL context

    struct socket *listening_sockets;
    struct pollfd *listening_socket_fds;
    unsigned int num_listening_sockets;

    // TODO pthread_mutex_t thread_mutex;       // Protects (max|num)_threads

    struct socket *client_socks;
    void **client_wait_events;

    unsigned int max_request_size;      // The max request size

    // TODO pthread_t masterthreadid;           // The master thread ID
    unsigned int
    cfg_worker_threads;                 // The number of configured worker threads.
    // TODO pthread_t *worker_threadids;        // The worker thread IDs

    struct ttimers *timers;
    
    //struct stcp_memory_stat ctx_memory;

};*/


/*!
 *  Connect (unsecurely) to remote
 */
struct stcp_connection *
stcp_connect_remote( const char *hostip,
                        int port,
                        int use_ssl,
                        char *error_buffer,
                        size_t error_buffer_size,
                        int timeout );

/*!
 *  Connect securely to remote
 */

struct stcp_connection *
stcp_connect_remote_secure( struct stcp_secure_client_options *client_options,
                                char *error_buffer,
                                size_t error_buffer_size,
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



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // __sockettcp_h