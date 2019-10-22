/*
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

#ifndef VSCPWEB_HEADER_INCLUDED
#define VSCPWEB_HEADER_INCLUDED

#include <time.h>   // for time_t

#include <vscp.h>
#include <version.h>

// Synced with Civetwev version
#define CIVETWEB_VERSION "1.10"
#define CIVETWEB_VERSION_MAJOR (1)
#define CIVETWEB_VERSION_MINOR (10)
#define CIVETWEB_VERSION_PATCH (0)
#define CIVETWEB_COPYRIGHT "Copyright (c) 2013-2018 the Civetweb developers"

#ifndef VSCPWEB_API
#if defined(_WIN32)
#if defined(VSCPWEB_DLL_EXPORTS)
#define VSCPWEB_API __declspec(dllexport)
#elif defined(VSCPWEB_DLL_IMPORTS)
#define VSCPWEB_API __declspec(dllimport)
#else
#define VSCPWEB_API
#endif
#elif __GNUC__ >= 4
#define VSCPWEB_API __attribute__((visibility("default")))
#else
#define VSCPWEB_API
#endif
#endif

#include <stdio.h>
#include <stddef.h>

#ifndef INT64_MAX
#define INT64_MAX (9223372036854775807)
#endif

#ifndef MAX_WORKER_THREADS
#define MAX_WORKER_THREADS (1024 * 64)
#endif

#ifndef SOCKET_TIMEOUT_QUANTUM  // in ms
#define SOCKET_TIMEOUT_QUANTUM (2000)
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Helpers
char *web_strndup(const char *ptr, size_t len);
char *web_strdup( const char *str );
void web_free_ex( void *memory, const char *file, unsigned line );


#define WEB_OK      1
#define WEB_ERROR   0


////////////////////////////////////////////////////////////////////////////////
// web_getOptionCount
//
// Get number of options
//

int web_getOptionCount( void );

////////////////////////////////////////////////////////////////////////////////
// skip_quoted
//
// Skip the characters until one of the delimiters characters found.
// 0-terminate resulting word. Skip the delimiter and following whitespaces.
// Advance pointer to buffer to the next word. Return found 0-terminated
// word.
// Delimiters can be quoted with quotechar.
//

char *
web_skip_quoted( char **buf,
                    const char *delimiters,
                    const char *whitespace,
                    char quotechar );

///////////////////////////////////////////////////////////////////////////////
// web_init
//
// Initialize this library. This should be called once before any other
// function from this library. This function is not guaranteed to be
// thread safe.
//
// Parameters:
//   features: bit mask for features to be initialized.
//   https://github.com/civetweb/civetweb/blob/master/docs/api/mg_init_library.md
//
// Return value:
//   initialized features
//   0: error
//

VSCPWEB_API unsigned web_init( unsigned features );


///////////////////////////////////////////////////////////////////////////////
// web_exit
//
// Un-initialize this library.
//
// Return value:
//   0: error
//

VSCPWEB_API unsigned web_exit( void );

struct web_context;    // Handle for the HTTP service itself
struct web_connection; // Handle for the individual connection

void web_set_connection_code( struct web_connection *conn, int code );
void web_set_must_close( struct web_connection *conn );

// Maximum number of headers
#define WEB_MAX_HEADERS (255)

struct web_header {
    const char *name;  // HTTP header name
    const char *value; // HTTP header value
};

///////////////////////////////////////////////////////////////////////////////
// web_request_info
//
// This structure contains information about the HTTP request.
//

struct web_request_info {
    const char *request_method; // "GET", "POST", etc

    const char *request_uri;    // URL-decoded URI (absolute or relative,
                                // as in the request)

    const char *local_uri;      // URL-decoded URI (relative). Can be NULL
                                // if the request_uri does not address a
                                // resource at the server host.

    const char *http_version;   // E.g. "1.0", "1.1"

    const char *query_string;   // URL part after '?', not including '?', or
                                // NULL

    const char *remote_user;    // Authenticated user, or NULL if no auth
                                // used

    char remote_addr[48];       // Client's IP address as a string.
                                // TODO IPv6 flag

    long long content_length;   // Length (in bytes) of the request body,
                                // can be -1 if no length was given.

    int remote_port;            // Client's port

    int is_ssl;                 // 1 if SSL-ed, 0 if not

    void *user_data;            // User data pointer passed to web_start()

    void *conn_data;            // Connection-specific user data

    int num_headers;            // Number of HTTP headers

    struct web_header
        http_headers[ WEB_MAX_HEADERS ];        // Allocate maximum headers

    struct web_client_cert *client_cert;        // Client certificate information

    const char *acceptedWebSocketSubprotocol;   // websocket subprotocol,
                                                // accepted during handshake
};

///////////////////////////////////////////////////////////////////////////////
// web_response_info
//
// This structure contains information about the HTTP request.
// This structure may be extended in future versions.
//

struct web_response_info {

    int status_code;                // E.g. 200

    const char *status_text;        // E.g. "OK"

    const char *http_version;       // E.g. "1.0", "1.1"

    long long content_length;       // Length (in bytes) of the request body,
                                    // can be -1 if no length was given.

    int num_headers;                // Number of HTTP headers

    struct web_header
        http_headers[ WEB_MAX_HEADERS ]; // Allocate maximum headers

};

///////////////////////////////////////////////////////////////////////////////
// client_cert
//
// Client certificate information (part of web_request_info)
//


// New nomenclature.
struct web_client_cert {
    const char *subject;
    const char *issuer;
    const char *serial;
    const char *finger;
};


///////////////////////////////////////////////////////////////////////////////
// web_callbacks
//
// This structure needs to be passed to web_start(), to let vscpweb know
//   which callbacks to invoke. For a detailed description, see
//   https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md
//

struct web_callbacks {

        //   Called when vscpweb has received new HTTP request.
        //
        //   If the callback returns one, it must process the request
        //   by sending valid HTTP headers and a body. vscpweb will not do
        //   any further processing. Otherwise it must return zero.
        //   Note that since V1.7 the "begin_request" function is called
        //   before an authorization check. If an authorization check is
        //   required, use a request_handler instead.
        //
        //   Return value:
        //     0: vscpweb will process the request itself. In this case,
        //        the callback must not send any data to the client.
        //     1-999: callback already processed the request. vscpweb will
        //            not send any data after the callback returned. The
        //            return code is stored as a HTTP status code for the
        //            access log.

    int (*begin_request)(struct web_connection *);

    // Called when vscpweb has finished processing request.
    void (*end_request)(const struct web_connection *,
                                int reply_status_code);

    // Called when vscpweb is about to log a message. If callback returns
    //   non-zero, vscpweb does not log anything.
    int (*log_message)(const struct web_connection *,
                                const char *message);

    // Called when vscpweb is about to log access. If callback returns
    //   non-zero, vscpweb does not log anything.
    int (*log_access)(const struct web_connection *,
                                const char *message);

    // Called when vscpweb initializes SSL library.
    //
    //   Parameters:
    //     user_data: parameter user_data passed when starting the server.
    //
    //   Return value:
    //     0: vscpweb will set up the SSL certificate.
    //     1: vscpweb assumes the callback already set up the certificate.
    //    -1: initializing ssl fails.

    int (*init_ssl)(void *ssl_context, void *user_data);

    // Called when vscpweb is closing a connection.  The per-context mutex is
    //   locked when this is invoked.
    //
    //   Websockets:
    //   Before web_set_websocket_handler has been added, it was primarily useful
    //   for noting when a websocket is closing, and used to remove it from any
    //   application-maintained list of clients.
    //   Using this callback for websocket connections is deprecated: Use
    //   web_set_websocket_handler instead.
    //
    //   Connection specific data:
    //   If memory has been allocated for the connection specific user data
    //   (web_request_info->conn_data, web_get_user_connection_data),
    //   this is the last chance to free it.

    void (*connection_close)(const struct web_connection *);


    // Called when vscpweb tries to open a file. Used to intercept file open
    //   calls, and serve file data from memory instead.
    //
    //   Parameters:
    //      path:     Full path to the file to open.
    //      data_len: Placeholder for the file size, if file is served from
    //                memory.
    //   Return value:
    //     NULL: do not serve file from memory, proceed with normal file open.
    //     non-NULL: pointer to the file contents in memory. data_len must be
    //       initialized with the size of the memory block.
    //

    const char *(*open_file)(const struct web_connection *,
                                    const char *path,
                                    size_t *data_len);

    // Called when webserver is about to serve Lua server page, if
    //   Lua support is enabled.
    //
    //   Parameters:
    //     lua_context: "lua_State *" pointer.

    void (*init_lua)(const struct web_connection *,
                                void *lua_context);

    // Called when vscpweb is about to send HTTP error to the client.
    //   Implementing this callback allows to create custom error pages.
    //
    //   Parameters:
    //     status: HTTP error status code.
    //
    //   Return value:
    //     1: run vscpweb error handler.
    //     0: callback already handled the error.

    int (*http_error)(struct web_connection *, int status);

    // Called after vscpweb context has been created, before requests
    //   are processed.
    //
    //   Parameters:
    //     ctx: context handle

    void (*init_context)(const struct web_context *ctx);

    // Called when a new worker thread is initialized.
    //
    //   Parameters:
    //     ctx: context handle
    //     thread_type:
    //       0 indicates the master thread
    //       1 indicates a worker thread handling client connections
    //       2 indicates an internal helper thread (timer thread)
    //

    void (*init_thread)(const struct web_context *ctx, int thread_type);

    // Called when vscpweb context is deleted.
    //   Parameters:
    //     ctx: context handle

    void (*exit_context)(const struct web_context *ctx);

    // Called when initializing a new connection object.
    // Can be used to initialize the connection specific user data
    // (web_request_info->conn_data, web_get_user_connection_data).
    // When the callback is called, it is not yet known if a
    // valid HTTP(S) request will be made.
    //
    // Parameters:
    //   conn: not yet fully initialized connection object
    //   conn_data: output parameter, set to initialize the
    //              connection specific user data
    //
    // Return value:
    //   must be 0
    //   Otherwise, the result is undefined

    int (*init_connection)(const struct web_connection *conn, void **conn_data);
};

///////////////////////////////////////////////////////////////////////////////
// web_start
//
// Start web server.
//
// Parameters:
//     callbacks: web_callbacks structure with user-defined callbacks.
//     options: NULL terminated list of option_name, option_value pairs that
//              specify vscpweb configuration parameters.
//
// Side-effects: on UNIX, ignores SIGCHLD and SIGPIPE signals. If custom
//      processing is required for these, signal handlers must be set up
//      after calling web_start().
//
//
// Example:
//     const char *options[] = {
//       "document_root", "/var/www",
//       "listening_ports", "80,443s",
//       NULL
//     };
//     struct web_context *ctx = web_start(&my_func, NULL, options);
//
//   Refer to https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md
//   for the list of valid option and their possible values.
//
// Return:
//     web server context, or NULL on error.
//

VSCPWEB_API struct web_context *
web_start( const struct web_callbacks *callbacks,
            void *user_data,
            const char **configuration_options );

///////////////////////////////////////////////////////////////////////////////
// web_stop
//
// Stop the web server.
//
//   Must be called last, when an application wants to stop the web server and
//   release all associated resources. This function blocks until all vscpweb
//   threads are stopped. Context pointer becomes invalid.
//

VSCPWEB_API void web_stop( struct web_context * );

///////////////////////////////////////////////////////////////////////////////
// web_request_handler
//
//   Called when a new request comes in.  This callback is URI based
//   and configured with web_set_request_handler().
//
//   Parameters:
//      conn: current connection information.
//      cbdata: the callback data configured with web_set_request_handler().
//   Returns:
//      0: the handler could not handle the request, so fall through.
//      1 - 999: the handler processed the request. The return code is
//               stored as a HTTP status code for the access log.

typedef int (*web_request_handler)( struct web_connection *conn, void *cbdata );

///////////////////////////////////////////////////////////////////////////////
// web_set_request_handler
//
//   Sets or removes a URI mapping for a request handler.
//   This function uses web_lock_context internally.
//
//   URI's are ordered and prefixed URI's are supported. For example,
//   consider two URIs: /a/b and /a
//           /a   matches /a
//           /a/b matches /a/b
//           /a/c matches /a
//
//   Parameters:
//      ctx: server context
//      uri: the URI (exact or pattern) for the handler
//      handler: the callback handler to use when the URI is requested.
//               If NULL, an already registered handler for this URI will
//               be removed.
//               The URI used to remove a handler must match exactly the
//               one used to register it (not only a pattern match).
//      cbdata: the callback data to give to the handler when it is called.
//

VSCPWEB_API void web_set_request_handler( struct web_context *ctx,
                                                const char *uri,
                                                web_request_handler handler,
                                                void *cbdata );

///////////////////////////////////////////////////////////////////////////////
// web_websocket_connect_handler
//
// Callback types for websocket handlers in C/C++.
//
//   web_websocket_connect_handler
//       Is called when the client intends to establish a websocket connection,
//       before websocket handshake.
//       Return value:
//         0: civetweb proceeds with websocket handshake.
//         1: connection is closed immediately.
//
//   web_websocket_ready_handler
//       Is called when websocket handshake is successfully completed, and
//       connection is ready for data exchange.
//
//   web_websocket_data_handler
//       Is called when a data frame has been received from the client.
//       Parameters:
//         bits: first byte of the websocket frame, see websocket RFC at
//               http://tools.ietf.org/html/rfc6455, section 5.2
//         data, data_len: payload, with mask (if any) already applied.
//       Return value:
//         1: keep this websocket connection open.
//         0: close this websocket connection.
//
//   web_connection_close_handler
//       Is called, when the connection is closed.
//

typedef int (*web_websocket_connect_handler)( const struct web_connection *,
                                                    void *);
typedef void (*web_websocket_ready_handler)( struct web_connection *, void *);
typedef int (*web_websocket_data_handler)( struct web_connection *,
                                                    int,
                                                    char *,
                                                    size_t,
                                                    void * );
typedef void (*web_websocket_close_handler)( const struct web_connection *,
                                                    void *);

///////////////////////////////////////////////////////////////////////////////
// web_websocket_subprotocols
//
// struct web_websocket_subprotocols
//
// List of accepted subprotocols
//

struct web_websocket_subprotocols {
    int nb_subprotocols;
    char **subprotocols;
};

///////////////////////////////////////////////////////////////////////////////
// web_set_websocket_handler
//
//   Set or remove handler functions for websocket connections.
//   This function works similar to web_set_request_handler - see there.
//

VSCPWEB_API void
web_set_websocket_handler( struct web_context *ctx,
                                const char *uri,
                                web_websocket_connect_handler connect_handler,
                                web_websocket_ready_handler ready_handler,
                                web_websocket_data_handler data_handler,
                                web_websocket_close_handler close_handler,
                                void *cbdata );

///////////////////////////////////////////////////////////////////////////////
// web_set_websocket_handler
//
//   Set or remove handler functions for websocket connections.
//   This function works similar to web_set_request_handler - see there.
//

VSCPWEB_API void web_set_websocket_handler_with_subprotocols(
                        struct web_context *ctx,
                        const char *uri,
                        struct web_websocket_subprotocols *subprotocols,
                        web_websocket_connect_handler connect_handler,
                        web_websocket_ready_handler ready_handler,
                        web_websocket_data_handler data_handler,
                        web_websocket_close_handler close_handler,
                        void *cbdata );


///////////////////////////////////////////////////////////////////////////////
// web_authorization_handler
//
//   Callback function definition for web_set_auth_handler
//
//   Parameters:
//      conn: current connection information.
//      cbdata: the callback data configured with web_set_request_handler().
//   Returns:
//      0: access denied
//      1: access granted
//

typedef int (*web_authorization_handler)( struct web_connection *conn,
                                                void *cbdata );

///////////////////////////////////////////////////////////////////////////////
// web_set_auth_handler
//
//   Sets or removes a URI mapping for an authorization handler.
//   This function works similar to web_set_request_handler - see there.
//

VSCPWEB_API void web_set_auth_handler( struct web_context *ctx,
                                            const char *uri,
                                            web_authorization_handler handler,
                                            void *cbdata );

///////////////////////////////////////////////////////////////////////////////
// web_get_option
//
// Get the value of particular configuration parameter.
//   The value returned is read-only. vscpweb does not allow changing
//   configuration at run time.
//   If given parameter name is not valid, NULL is returned. For valid
//   names, return value is guaranteed to be non-NULL. If parameter is not
//   set, zero-length string is returned.

VSCPWEB_API const char *web_get_option( const struct web_context *ctx,
                                            const char *name);



// Get context from connection.

VSCPWEB_API struct web_context *
web_get_context( const struct web_connection *conn );

///////////////////////////////////////////////////////////////////////////////
// web_get_user_data
//
// Get user data passed to web_start from context.
//

VSCPWEB_API void *web_get_user_data( const struct web_context *ctx );

///////////////////////////////////////////////////////////////////////////////
// web_set_user_connection_data
//
// Set user data for the current connection.
//

VSCPWEB_API void web_set_user_connection_data( struct web_connection *conn,
                                                void *data);

///////////////////////////////////////////////////////////////////////////////
// web_get_user_connection_data
//
// Get user data set for the current connection.
//

VSCPWEB_API void *
web_get_user_connection_data( const struct web_connection *conn );

///////////////////////////////////////////////////////////////////////////////
// web_get_request_link
//
// Get a formatted link corresponding to the current request
//
//   Parameters:
//      conn: current connection information.
//      buf: string buffer (out)
//      buflen: length of the string buffer
//   Returns:
//      <0: error
//      >=0: ok
//

VSCPWEB_API int
web_get_request_link( const struct web_connection *conn,
                        char *buf,
                        size_t buflen );


struct web_option {
    const char *name;
    int type;
    const char *default_value;
};

enum {
    WEB_CONFIG_TYPE_UNKNOWN = 0x0,
    WEB_CONFIG_TYPE_NUMBER = 0x1,
    WEB_CONFIG_TYPE_STRING = 0x2,
    WEB_CONFIG_TYPE_FILE = 0x3,
    WEB_CONFIG_TYPE_DIRECTORY = 0x4,
    WEB_CONFIG_TYPE_BOOLEAN = 0x5,
    WEB_CONFIG_TYPE_EXT_PATTERN = 0x6,
    WEB_CONFIG_TYPE_STRING_LIST = 0x7,
    WEB_CONFIG_TYPE_STRING_MULTILINE = 0x8
};

///////////////////////////////////////////////////////////////////////////////
// web_option
//
// Return array of struct web_option, representing all valid configuration
//   options of civetweb.c.
//   The array is terminated by a NULL name option.
//

VSCPWEB_API const struct web_option *web_get_valid_options( void );


struct web_server_ports {
    int protocol;       // 1 = IPv4, 2 = IPv6, 3 = both
    int port;           // port number
    int is_ssl;         // https port: 0 = no, 1 = yes
    int is_redirect;    // redirect all requests: 0 = no, 1 = yes
    int _reserved1;
    int _reserved2;
    int _reserved3;
    int _reserved4;
};

///////////////////////////////////////////////////////////////////////////////
// web_get_server_ports
//
// Get the list of ports that civetweb is listening on.
//   The parameter size is the size of the ports array in elements.
//   The caller is responsibility to allocate the required memory.
//   This function returns the number of struct web_server_ports elements
//   filled in, or <0 in case of an error.
//

VSCPWEB_API int web_get_server_ports( const struct web_context *ctx,
                                        int size,
                                        struct web_server_ports *ports );

///////////////////////////////////////////////////////////////////////////////
// web_modify_passwords_file
//
// Add, edit or delete the entry in the passwords file.
//
// This function allows an application to manipulate .htpasswd files on the
// fly by adding, deleting and changing user records. This is one of the
// several ways of implementing authentication on the server side. For another,
// cookie-based way please refer to the examples/chat in the source tree.
//
// Parameter:
//   passwords_file_name: Path and name of a file storing multiple passwords
//   realm: HTTP authentication realm (authentication domain) name
//   user: User name
//   password:
//      If password is not NULL, entry modified or added.
//     If password is NULL, entry is deleted.
//
//  Return:
//    1 on success, 0 on error.
//

VSCPWEB_API int web_modify_passwords_file( const char *passwords_file_name,
                                            const char *realm,
                                            const char *user,
                                            const char *password );

///////////////////////////////////////////////////////////////////////////////
// web_get_request_info
//
// Return information associated with the request.
// Use this function to implement a server and get data about a request
// from a HTTP/HTTPS client.
// Note: Before CivetWeb 1.10, this function could be used to read
// a response from a server, when implementing a client, although the
// values were never returned in appropriate web_request_info elements.
// It is strongly advised to use web_get_response_info for clients.
//

VSCPWEB_API const struct web_request_info *
web_get_request_info( const struct web_connection * );

///////////////////////////////////////////////////////////////////////////////
// web_get_response_info
//
// Return information associated with a HTTP/HTTPS response.
// Use this function in a client, to check the response from
// the server.
//

VSCPWEB_API const struct web_response_info *
web_get_response_info( const struct web_connection * );

///////////////////////////////////////////////////////////////////////////////
// web_write
//
// Send data to the client.
//   Return:
//    0   when the connection has been closed
//    -1  on error
//    >0  number of bytes written on success
//

VSCPWEB_API int web_write( struct web_connection *,
                                const void *buf,
                                size_t len );

///////////////////////////////////////////////////////////////////////////////
// web_websocket_write
//
// Send data to a websocket client wrapped in a websocket frame.  Uses
//   web_lock_connection to ensure that the transmission is not interrupted,
//   i.e., when the application is proactively communicating and responding to
//   a request simultaneously.
//
//   Send data to a websocket client wrapped in a websocket frame.
//   This function is available when civetweb is compiled with -DUSE_WEBSOCKET
//
//   Return:
//    0   when the connection has been closed
//    -1  on error
//    >0  number of bytes written on success
//

VSCPWEB_API int web_websocket_write( struct web_connection *conn,
                                        int opcode,
                                        const char *data,
                                        size_t data_len );

///////////////////////////////////////////////////////////////////////////////
// web_websocket_client_write
//
// Send data to a websocket server wrapped in a masked websocket frame.  Uses
//   web_lock_connection to ensure that the transmission is not interrupted,
//   i.e., when the application is proactively communicating and responding to
//   a request simultaneously.
//
//   Send data to a websocket server wrapped in a masked websocket frame.
//   This function is available when civetweb is compiled with -DUSE_WEBSOCKET
//
//   Return:
//    0   when the connection has been closed
//    -1  on error
//    >0  number of bytes written on success
//

VSCPWEB_API int web_websocket_client_write( struct web_connection *conn,
                                                int opcode,
                                                const char *data,
                                                size_t data_len );

///////////////////////////////////////////////////////////////////////////////
// web_lock_connection
// web_unlock_connection
//
// Blocks until unique access is obtained to this connection. Intended for use
//   with websockets only.
//   Invoke this before web_write or web_printf when communicating with a
//   websocket if your code has server-initiated communication as well as
//   communication in direct response to a message.

VSCPWEB_API void web_lock_connection( struct web_connection *conn );
VSCPWEB_API void web_unlock_connection( struct web_connection *conn );

///////////////////////////////////////////////////////////////////////////////
// web_lock_context
//
// Lock server context.  This lock may be used to protect resources
//   that are shared between different connection/worker threads.

VSCPWEB_API void web_lock_context(struct web_context *ctx);
VSCPWEB_API void web_unlock_context(struct web_context *ctx);


// Opcodes, from http://tools.ietf.org/html/rfc6455

enum {
    WEB_WEBSOCKET_OPCODE_CONTINUATION = 0x0,
    WEB_WEBSOCKET_OPCODE_TEXT = 0x1,
    WEB_WEBSOCKET_OPCODE_BINARY = 0x2,
    WEB_WEBSOCKET_OPCODE_CONNECTION_CLOSE = 0x8,
    WEB_WEBSOCKET_OPCODE_PING = 0x9,
    WEB_WEBSOCKET_OPCODE_PONG = 0xa
};

// Macros for enabling compiler-specific checks for printf-like arguments.
#undef PRINTF_FORMAT_STRING
#if defined(_MSC_VER) && _MSC_VER >= 1400
#include <sal.h>
#if defined(_MSC_VER) && _MSC_VER > 1400
#define PRINTF_FORMAT_STRING(s) _Printf_format_string_ s
#else
#define PRINTF_FORMAT_STRING(s) __format_string s
#endif
#else
#define PRINTF_FORMAT_STRING(s) s
#endif

#ifdef __GNUC__
#define PRINTF_ARGS(x, y) __attribute__((format(printf, x, y)))
#else
#define PRINTF_ARGS(x, y)
#endif

///////////////////////////////////////////////////////////////////////////////
// web_printf
//
// Send data to the client using printf() semantics.
//   Works exactly like web_write(), but allows to do message formatting.
//

VSCPWEB_API int web_printf( struct web_connection *,
                                    PRINTF_FORMAT_STRING(const char *fmt),
                                    ...) PRINTF_ARGS(2, 3);

///////////////////////////////////////////////////////////////////////////////
// web_send_chunk
//
// Send a part of the message body, if chunked transfer encoding is set.
// Only use this function after sending a complete HTTP request or response
// header with "Transfer-Encoding: chunked" set.
//

VSCPWEB_API int web_send_chunk( struct web_connection *conn,
                                        const char *chunk,
                                        unsigned int chunk_len );

///////////////////////////////////////////////////////////////////////////////
// web_send_file
//
// Send contents of the entire file together with HTTP headers.
//

VSCPWEB_API void web_send_file( struct web_connection *conn,
                                        const char *path );

///////////////////////////////////////////////////////////////////////////////
// web_send_http_error
//
// Send HTTP error reply.
//

VSCPWEB_API void web_send_http_error( struct web_connection *conn,
                                            int status_code,
                                            PRINTF_FORMAT_STRING(const char *fmt),
                                            ...) PRINTF_ARGS(3, 4);

///////////////////////////////////////////////////////////////////////////////
// web_send_digest_access_authentication_request
//
// Send HTTP digest access authentication request.
// Browsers will send a user name and password in their next request, showing
// an authentication dialog if the password is not stored.
//
// Parameters:
//   conn: Current connection handle.
//   realm: Authentication realm. If NULL is supplied, the sever domain
//          set in the authentication_domain configuration is used.
// Return:
//   < 0   Error
//

VSCPWEB_API int
web_send_digest_access_authentication_request( struct web_connection *conn,
                                                    const char *realm );

///////////////////////////////////////////////////////////////////////////////
// web_check_digest_access_authentication
//
// Check if the current request has a valid authentication token set.
// A file is used to provide a list of valid user names, realms and
// password hashes. The file can be created and modified using the
// mg_modify_passwords_file API function.
// Parameters:
//   conn: Current connection handle.
//   realm: Authentication realm. If NULL is supplied, the sever domain
//          set in the authentication_domain configuration is used.
//   filename: Path and name of a file storing multiple password hashes.
// Return:
//   > 0   Valid authentication
//   0     Invalid authentication
//   < 0   Error (all values < 0 should be considered as invalid
//         authentication, future error codes will have negative
//         numbers)
//   -1    Parameter error
//   -2    File not found
//

VSCPWEB_API int
web_check_digest_access_authentication( struct web_connection *conn,
                                                const char *realm,
                                                const char *filename);

///////////////////////////////////////////////////////////////////////////////
// web_send_mime_file
//
// Send contents of the entire file together with HTTP headers.
// Parameters:
//   conn: Current connection handle.
//   path: Full path to the file to send.
//   mime_type: Content-Type for file.  NULL will cause the type to be
//              looked up by the file extension.
//

VSCPWEB_API void web_send_mime_file( struct web_connection *conn,
                                            const char *path,
                                            const char *mime_type );

///////////////////////////////////////////////////////////////////////////////
// web_send_mime_file2
//
// Send contents of the entire file together with HTTP headers.
//   Parameters:
//     conn: Current connection information.
//     path: Full path to the file to send.
//     mime_type: Content-Type for file.  NULL will cause the type to be
//                looked up by the file extension.
//     additional_headers: Additional custom header fields appended to the header.
//                         Each header should start with an X-, to ensure it is
//                         not included twice.
//                         NULL does not append anything.
//

VSCPWEB_API void web_send_mime_file2( struct web_connection *conn,
                                            const char *path,
                                            const char *mime_type,
                                            const char *additional_headers );


///////////////////////////////////////////////////////////////////////////////
// web_store_body
//
// Store body data into a file.
//
// Read entire request body and store it in a file "path".
//   Return:
//     < 0   Error
//     >= 0  Number of bytes stored in file "path".
//

VSCPWEB_API long long web_store_body( struct web_connection *conn,
                                            const char *path );


///////////////////////////////////////////////////////////////////////////////
// web_read
//
// Read data from the remote end, return number of bytes read.
//   Return:
//     0     connection has been closed by peer. No more data could be read.
//     < 0   read error. No more data could be read from the connection.
//     > 0   number of bytes read into the buffer.

VSCPWEB_API int web_read( struct web_connection *,
                                void *buf,
                                size_t len );

///////////////////////////////////////////////////////////////////////////////
// web_get_header
//
// Get the value of particular HTTP header.
//
//   This is a helper function. It traverses request_info->http_headers array,
//   and if the header is present in the array, returns its value. If it is
//   not present, NULL is returned.

VSCPWEB_API const char *web_get_header( const struct web_connection *,
                                            const char *name );

///////////////////////////////////////////////////////////////////////////////
// web_get_var
//
// Get a value of particular form variable.
//
//   Parameters:
//     data: pointer to form-uri-encoded buffer. This could be either POST data,
//           or request_info.query_string.
//     data_len: length of the encoded data.
//     var_name: variable name to decode from the buffer
//     dst: destination buffer for the decoded variable
//     dst_len: length of the destination buffer
//
//   Return:
//     On success, length of the decoded variable.
//     On error:
//        -1 (variable not found).
//        -2 (destination buffer is NULL, zero length or too small to hold the
//            decoded variable).
//
//   Destination buffer is guaranteed to be '\0' - terminated if it is not
//   NULL or zero length.
//

VSCPWEB_API int web_get_var( const char *data,
                                    size_t data_len,
                                    const char *var_name,
                                    char *dst,
                                    size_t dst_len );

///////////////////////////////////////////////////////////////////////////////
// web_get_var2
//
// Get a value of particular form variable.
//
//   Parameters:
//     data: pointer to form-uri-encoded buffer. This could be either POST data,
//           or request_info.query_string.
//     data_len: length of the encoded data.
//     var_name: variable name to decode from the buffer
//     dst: destination buffer for the decoded variable
//     dst_len: length of the destination buffer
//     occurrence: which occurrence of the variable, 0 is the first, 1 the
//                 second...
//                this makes it possible to parse a query like
//                b=x&a=y&a=z which will have occurrence values b:0, a:0 and a:1
//
//   Return:
//     On success, length of the decoded variable.
//     On error:
//        -1 (variable not found).
//        -2 (destination buffer is NULL, zero length or too small to hold the
//            decoded variable).
//
//   Destination buffer is guaranteed to be '\0' - terminated if it is not
//   NULL or zero length.
//

VSCPWEB_API int web_get_var2( const char *data,
                                    size_t data_len,
                                    const char *var_name,
                                    char *dst,
                                    size_t dst_len,
                                    size_t occurrence );

///////////////////////////////////////////////////////////////////////////////
// web_get_cookie
//
// Fetch value of certain cookie variable into the destination buffer.
//
//   Destination buffer is guaranteed to be '\0' - terminated. In case of
//   failure, dst[0] == '\0'. Note that RFC allows many occurrences of the same
//   parameter. This function returns only first occurrence.
//
//   Return:
//     On success, value length.
//     On error:
//        -1 (either "Cookie:" header is not present at all or the requested
//            parameter is not found).
//        -2 (destination buffer is NULL, zero length or too small to hold the
//            value). *
//

VSCPWEB_API int web_get_cookie( const char *cookie,
                                        const char *var_name,
                                        char *buf,
                                        size_t buf_len );

///////////////////////////////////////////////////////////////////////////////
// web_download
//
// Download data from the remote web server.
//     host: host name to connect to, e.g. "foo.com", or "10.12.40.1".
//     port: port number, e.g. 80.
//     use_ssl: wether to use SSL connection.
//     error_buffer, error_buffer_size: error message placeholder.
//     request_fmt,...: HTTP request.
//   Return:
//     On success, valid pointer to the new connection, suitable for web_read().
//     On error, NULL. error_buffer contains error message.
//   Example:
//     char ebuf[100];
//     struct web_connection *conn;
//     conn = web_download("google.com", 80, 0, ebuf, sizeof(ebuf),
//                        "%s", "GET / HTTP/1.0\r\nHost: google.com\r\n\r\n");
//

VSCPWEB_API struct web_connection *
web_download( const char *host,
                    int port,
                    int use_ssl,
                    char *error_buffer,
                    size_t error_buffer_size,
                    PRINTF_FORMAT_STRING(const char *request_fmt),
                    ...) PRINTF_ARGS(6, 7);

///////////////////////////////////////////////////////////////////////////////
// web_close_connection
//
// Close the connection opened by web_download().
//

VSCPWEB_API void web_close_connection( struct web_connection *conn );


///////////////////////////////////////////////////////////////////////////////
// web_form_data_handler
//
// This structure contains callback functions for handling form fields.
//   It is used as an argument to web_handle_form_request.
//

struct web_form_data_handler {
    // This callback function is called, if a new field has been found.
    // The return value of this callback is used to define how the field
    // should be processed.
    //
    // Parameters:
    //   key: Name of the field ("name" property of the HTML input field).
    //   filename: Name of a file to upload, at the client computer.
    //             Only set for input fields of type "file", otherwise NULL.
    //   path: Output parameter: File name (incl. path) to store the file
    //         at the server computer. Only used if FORM_FIELD_STORAGE_STORE
    //         is returned by this callback. Existing files will be
    //         overwritten.
    //   pathlen: Length of the buffer for path.
    //   user_data: Value of the member user_data of web_form_data_handler
    //
    // Return value:
    //   The callback must return the intended storage for this field
    //   (See FORM_FIELD_STORAGE_*).
    //

    int (*field_found)( const char *key,
                                const char *filename,
                                char *path,
                                size_t pathlen,
                                void *user_data );

    // If the "field_found" callback returned FORM_FIELD_STORAGE_GET,
    // this callback will receive the field data.
    //
    // Parameters:
    //   key: Name of the field ("name" property of the HTML input field).
    //   value: Value of the input field.
    //   user_data: Value of the member user_data of web_form_data_handler
    //
    // Return value:
    //   TODO: Needs to be defined.
    //

    int (*field_get)( const char *key,
                            const char *value,
                            size_t valuelen,
                            void *user_data );

    // If the "field_found" callback returned FORM_FIELD_STORAGE_STORE,
    // the data will be stored into a file. If the file has been written
    // successfully, this callback will be called. This callback will
    // not be called for only partially uploaded files. The
    // web_handle_form_request function will either store the file completely
    // and call this callback, or it will remove any partial content and
    // not call this callback function.
    //
    // Parameters:
    //   path: Path of the file stored at the server.
    //   file_size: Size of the stored file in bytes.
    //   user_data: Value of the member user_data of web_form_data_handler
    //
    // Return value:
    //   TODO: Needs to be defined.
    //

    int (*field_store)( const char *path, long long file_size, void *user_data );

    // User supplied argument, passed to all callback functions.
    void *user_data;
};

///////////////////////////////////////////////////////////////////////////////
//
//
// Return values definition for the "field_found" callback in
// web_form_data_handler.
//

enum {

    // Skip this field (neither get nor store it). Continue with the
    // next field.
    WEB_FORM_FIELD_STORAGE_SKIP = 0x0,

    // Get the field value.
    WEB_FORM_FIELD_STORAGE_GET = 0x1,

    // Store the field value into a file.
    WEB_FORM_FIELD_STORAGE_STORE = 0x2,

    // Stop parsing this request. Skip the remaining fields.
    WEB_FORM_FIELD_STORAGE_ABORT = 0x10
};

///////////////////////////////////////////////////////////////////////////////
// web_handle_form_request
//
// Process form data.
// Returns the number of fields handled, or < 0 in case of an error.
// Note: It is possible that several fields are already handled successfully
// (e.g., stored into files), before the request handling is stopped with an
// error. In this case a number < 0 is returned as well.
// In any case, it is the duty of the caller to remove files once they are
// no longer required.
//

VSCPWEB_API int web_handle_form_request( struct web_connection *conn,
                                            struct web_form_data_handler *fdh );

///////////////////////////////////////////////////////////////////////////////
// web_thread_func_t
//
// Convenience function -- create detached thread.
//   Return: 0 on success, non-0 on error.
//

typedef void *(*web_thread_func_t)(void *);
VSCPWEB_API int web_start_thread( web_thread_func_t f, void *p );

///////////////////////////////////////////////////////////////////////////////
// web_get_builtin_mime_type
//
// Return builtin mime type for the given file name.
//   For unrecognized extensions, "text/plain" is returned.
//

VSCPWEB_API const char *web_get_builtin_mime_type( const char *file_name );

///////////////////////////////////////////////////////////////////////////////
// web_get_response_code_text
//
// Get text representation of HTTP status code.
//

VSCPWEB_API const char *
web_get_response_code_text( const struct web_connection *conn,
                                int response_code );

///////////////////////////////////////////////////////////////////////////////
// web_version
//
// Return CivetWeb version.
//

VSCPWEB_API const char *web_version( void );

///////////////////////////////////////////////////////////////////////////////
// web_url_decode
//
// URL-decode input buffer into destination buffer.
//   0-terminate the destination buffer.
//   form-url-encoded data differs from URI encoding in a way that it
//   uses '+' as character for space, see RFC 1866 section 8.2.1
//   http://ftp.ics.uci.edu/pub/ietf/html/rfc1866.txt
//   Return: length of the decoded data, or -1 if dst buffer is too small.
//

VSCPWEB_API int web_url_decode( const char *src,
                                        int src_len,
                                        char *dst,
                                        int dst_len,
                                        int is_form_url_encoded );

///////////////////////////////////////////////////////////////////////////////
// web_url_encode
//
// URL-encode input buffer into destination buffer.
//   returns the length of the resulting buffer or -1
//   is the buffer is too small.
//

VSCPWEB_API int web_url_encode( const char *src,
                                        char *dst,
                                        size_t dst_len );

///////////////////////////////////////////////////////////////////////////////
// web_cry
//
// Print error message to the opened error log stream.
//   This utilizes the provided logging configuration.
//     conn: connection (not used for sending data, but to get parameters)
//     fmt: format string without the line return
//     ...: variable argument list
//   Example:
//     web_cry(conn,"i like %s", "logging");
//

VSCPWEB_API void web_cry( const struct web_connection *conn,
                                PRINTF_FORMAT_STRING(const char *fmt),
                                ...) PRINTF_ARGS(2, 3);

///////////////////////////////////////////////////////////////////////////////
// web_strcasecmp
//
// utility methods to compare two buffers, case insensitive.
// Return zero if equal.

//VSCPWEB_API int web_strcasecmp( const char *s1, const char *s2 );
//VSCPWEB_API int web_strncasecmp( const char *s1, const char *s2, size_t len );

///////////////////////////////////////////////////////////////////////////////
// web_connect_websocket_client
//
// Connect to a websocket as a client
//
//   Parameters:
//     host: host to connect to, i.e. "echo.websocket.org" or "192.168.1.1" or
//   "localhost"
//     port: server port
//     use_ssl: make a secure connection to server
//     error_buffer, error_buffer_size: buffer for an error message
//     path: server path you are trying to connect to, i.e. if connection to
//   localhost/app, path should be "/app"
//     origin: value of the Origin HTTP header
//     data_func: callback that should be used when data is received from the
//   server
//     user_data: user supplied argument
//
//   Return:
//     On success, valid web_connection object.
//     On error, NULL. Se error_buffer for details.
//

VSCPWEB_API struct web_connection *
web_connect_websocket_client( const char *host,
                                    int port,
                                    int use_ssl,
                                    char *error_buffer,
                                    size_t error_buffer_size,
                                    const char *path,
                                    const char *origin,
                                    web_websocket_data_handler data_func,
                                    web_websocket_close_handler close_func,
                                    void *user_data );

///////////////////////////////////////////////////////////////////////////////
// web_connect_client
//
// Connect to a TCP server as a client (can be used to connect to a HTTP server)
//
//   Parameters:
//     host: host to connect to, i.e. "www.wikipedia.org" or "192.168.1.1" or
//     "localhost"
//     port: server port
//     use_ssl: make a secure connection to server
//     error_buffer, error_buffer_size: buffer for an error message
//
//   Return:
//     On success, valid web_connection object.
//     On error, NULL. Se error_buffer for details.
//

VSCPWEB_API struct web_connection *
web_connect_client( const char *host,
                        int port,
                        int use_ssl,
                        char *error_buffer,
                        size_t error_buffer_size );

struct web_client_options {
    const char *host;
    int port;
    const char *client_cert;
    const char *server_cert;
    // TODO: add more data
};

///////////////////////////////////////////////////////////////////////////////
// web_connect_client_secure
//

VSCPWEB_API struct web_connection *
web_connect_client_secure( const struct web_client_options *client_options,
                                char *error_buffer,
                                size_t error_buffer_size );

enum { WEB_TIMEOUT_INFINITE = -1 };

///////////////////////////////////////////////////////////////////////////////
// web_get_response
//
// Wait for a response from the server
//
//   Parameters:
//     conn: connection
//     ebuf, ebuf_len: error message placeholder.
//     timeout: time to wait for a response in milliseconds (if < 0 then wait
//   forever)
//
//   Return:
//     On success, >= 0
//     On error/timeout, < 0
//

VSCPWEB_API int web_get_response( struct web_connection *conn,
                                        char *ebuf,
                                        size_t ebuf_len,
                                        int timeout );

///////////////////////////////////////////////////////////////////////////////
// web_check_feature
//
// Check which features where set when the civetweb library has been compiled.
//   The function explicitly addresses compile time defines used when building
//   the library - it does not mean, the feature has been initialized using a
//   web_init_library call.
//   web_check_feature can be called anytime, even before web_init_library has
//   been called.
//
//   Parameters:
//     feature: specifies which feature should be checked
//       The value is a bit mask. The individual bits are defined as:
//         1  serve files
//         2  support HTTPS
//         4  support CGI
//         8  support IPv6
//        16  support WebSocket
//        32  support Lua scripts and Lua server pages
//        64  support server side JavaScript
//       128  support caching
//       256  support server statistics
//       The result is undefined, if bits are set that do not represent a
//       defined feature (currently: feature >= 512).
//       The result is undefined, if no bit is set (feature == 0).
//
//   Return:
//     If feature is available, the corresponding bit is set
//     If feature is not available, the bit is 0
//

VSCPWEB_API unsigned web_check_feature( unsigned feature );

///////////////////////////////////////////////////////////////////////////////
// web_get_system_info
//
// Get information on the system. Useful for support requests.
//
//   Parameters:
//     buffer: Store system information as string here.
//     buflen: Length of buffer (including a byte required for a terminating 0).
//   Return:
//     Available size of system information, excluding a terminating 0.
//     The information is complete, if the return value is smaller than buflen.
//     The result is a JSON formatted string, the exact content may vary.
//   Note:
//     It is possible to determine the required buflen, by first calling this
//     function with buffer = NULL and buflen = NULL. The required buflen is
//     one byte more than the returned value.
//

VSCPWEB_API int web_get_system_info( char *buffer, int buflen );

///////////////////////////////////////////////////////////////////////////////
// web_get_context_info
//
// Get context information. Useful for server diagnosis.
//
//   Parameters:
//     ctx: Context handle
//     buffer: Store context information here.
//     buflen: Length of buffer (including a byte required for a terminating 0).
//   Return:
//     Available size of system information, excluding a terminating 0.
//     The information is complete, if the return value is smaller than buflen.
//     The result is a JSON formatted string, the exact content may vary.
//     Note:
//     It is possible to determine the required buflen, by first calling this
//     function with buffer = NULL and buflen = NULL. The required buflen is
//     one byte more than the returned value. However, since the available
//     context information changes, you should allocate a few bytes more.
//

VSCPWEB_API int
web_get_context_info( const struct web_context *ctx,
                            char *buffer,
                            int buflen );


///////////////////////////////////////////////////////////////////////////////
// web_get_connection_info
//
// Get connection information. Useful for server diagnosis.
//
//   Parameters:
//     ctx: Context handle
//     idx: Connection index
//    buffer: Store context information here.
//     buflen: Length of buffer (including a byte required for a terminating 0).
//   Return:
//     Available size of system information, excluding a terminating 0.
//     The information is complete, if the return value is smaller than buflen.
//     The result is a JSON formatted string, the exact content may vary.
//   Note:
//     It is possible to determine the required buflen, by first calling this
//     function with buffer = NULL and buflen = NULL. The required buflen is
//     one byte more than the returned value. However, since the available
//     context information changes, you should allocate a few bytes more.
//

VSCPWEB_API int web_get_connection_info( const struct web_context *ctx,
                                                int idx,
                                                char *buffer,
                                                int buflen );


///////////////////////////////////////////////////////////////////////////////
// Parsed Authorization header
//

struct web_authorization_header
{
    char *user,
    *uri,
    *cnonce,
    *response,
    *qop,
    *nc,
    *nonce;
};


///////////////////////////////////////////////////////////////////////////////
// web_parse_auth_header
//
// Return 1 on success. Always initializes the ah structure.
//

VSCPWEB_API int web_parse_auth_header( struct web_connection *conn,
                                        char *buf,
                                        size_t buf_size,
                                        struct web_authorization_header *ah );

///////////////////////////////////////////////////////////////////////////////
// www_check_password
//

VSCPWEB_API int
web_check_password( const char *method,
                        const char *ha1,
                        const char *uri,
                        const char *nonce,
                        const char *nc,
                        const char *cnonce,
                        const char *qop,
                        const char *response );

////////////////////////////////////////////////////////////////////////////////
// web_gmt_time_string
//
// Convert time_t to a string. According to RFC2616, Sec 14.18, this must be
// included in all responses other than 100, 101, 5xx.
//

void
web_gmt_time_string(char *buf, size_t buf_len, time_t *t);

VSCPWEB_API int
web_send_no_cache_header(struct web_connection *conn);

VSCPWEB_API int
web_send_static_cache_header(struct web_connection *conn);

VSCPWEB_API int
web_send_additional_header(struct web_connection *conn);

VSCPWEB_API const char *
web_suggest_connection_header(const struct web_connection *conn);


/*!
 * Initialize SSL multithreading
 * 
 * Exported because if other part of the system used SSL
 * This routine 
 */
VSCPWEB_API int
web_initialize_ssl( char *ebuf, size_t ebuf_len );


/*!
 * Get system information
 */
int web_get_system_info_impl( char *buffer, int buflen );

#define web_lua_load lua_load




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* VSCPWEB_HEADER_INCLUDED */
