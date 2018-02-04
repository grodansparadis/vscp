
// This file is part of the CivetWeb web server.
// See https://github.com/civetweb/civetweb/
// (C) 2014-2017 by the CivetWeb authors, MIT license.

#ifndef INCLUDE_HTTDP_LUA___H
#define INCLUDE_HTTDP_LUA___H


#ifndef LSP_INCLUDE_MAX_DEPTH
#define LSP_INCLUDE_MAX_DEPTH (32)
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// UUID library and function pointer
union
{
    void *p;
    void (*f)(unsigned char uuid[16]);
} pf_uuid_generate;    

/**
 * @brief Assign buffer content to the variable 'name' as a string
 * @param L Lua context
 * @param name Name of the string
 * @param buffer Null terminated buffer pointing to string to assign to 'name'
 * @param buflen Len of buffer
 */
void web_reg_lstring( struct lua_State *L,
                            const char *name,
                            const void *buffer,
                            size_t buflen );

#define web_reg_string( L, name, val )                                               \
        web_reg_lstring( L, name, val, val ? strlen( val ) : 0 )

/**
 * @brief Assign buffer2 content to the variable given by buffer1 content
 * as a string
 * @param L
 * @param buffer1 Null terminated string which holds name of the string variable
 * @param buflen1 Length of buffer1
 * @param buffer2 Null terminated string which holds value of the string variable.
 * @param buflen2 Length of buffer2
 */
void
web_reg_llstring( struct lua_State *L,
                    const void *buffer1,
                    size_t buflen1,
                    const void *buffer2,
                    size_t buflen2 );


/**
 * @brief Assign integer to the variable 'name'
 * @param L
 * @param name
 * @param val
 */
void web_reg_int( struct lua_State *L, const char *name, int val );

/**
 * @brief Assign boolean to the variable 'name'
 * @param L
 * @param name
 * @param val
 */
void web_reg_boolean( struct lua_State *L, const char *name, int val );

/*!
 * @brief Assign uint32_t to the variable 'name'
 * @param L
 * @param name
 * @param val uint32_t value to assign to name
 */
void web_reg_uint32( struct lua_State *L, const char *name, uint32_t val );

/**
 * @brief Assign function with connect parameter to 'name'
 * @param L Lua context
 * @param name Name of function
 * @param func Pointer to function to assign
 * @param conn Connection pointer
 */
void web_reg_conn_function( struct lua_State *L,
                            const char *name,
                            lua_CFunction func,
                            struct web_connection *conn );

/**
 * @brief Assign function with clientitem parameter to 'name'
 * @param L Lua context
 * @param name Name of function
 * @param func Pointer to function to assign
 * @param pClient Pointer to clientitem structure.
 */
void web_reg_vscpclient_function( struct lua_State *L,
                                    const char *name,
                                    lua_CFunction func,
                                    void *pClient );

/**
 * @brief Assign function to 'name'
 * @param L Lua context
 * @param name Name of function
 * @param func Pointer to function to assign
 * @param pClient Pointer to client object
 */
void web_reg_function( struct lua_State *L, 
                        const char *name, 
                        lua_CFunction func );

/**
 * @brief
 * @param conn
 * @param err
 * @param L
 * @param lua_title
 * @param lua_operation
 */
void web_lua_cry( struct web_connection *conn,
                    int err,
                    lua_State *L,
                    const char *lua_title,
                    const char *lua_operation);

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_sock_close(lua_State *L);

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_sock_recv( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_sock_send( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_sock_gc( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_connect(lua_State *L);

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_error( lua_State *L );

/**
 * @brief
 * @param L
 */
void web_lsp_abort( lua_State *L );

/**
 * @brief
 * @param L
 * @param ud
 * @param sz
 * @return
 */
const char * web_lsp_var_reader( lua_State *L, void *ud, size_t *sz );

/**
 * @brief
 * @param conn
 * @param path
 * @param p
 * @param len
 * @param L
 * @return
 */
int web_run_lsp( struct web_connection *conn,
                    const char *path,
                    const char *p,
                    int64_t len,
                    lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_write( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_read(lua_State *L);

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_keep_alive( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_include( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_cry( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_redirect( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_send_file( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_get_time( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_get_var( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_get_mime_type( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_get_cookie( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_md5( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_url_encode( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_url_decode( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_base64_encode(lua_State *L);

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_base64_decode( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_get_response_code_text( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_random(lua_State *L);

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_get_info( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_get_option( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lsp_uuid( lua_State *L );

/**
 * @class lua_websock_data
 * @author Civetweb programmers
 * @date 25/10/17
 * @file httpd_lua.h
 * @brief
 */
struct lua_websock_data
{
    lua_State *state;
    char *script;
    unsigned references;
    struct web_connection *conn[MAX_WORKER_THREADS];
    pthread_mutex_t ws_mutex;
};

/**
 * @brief
 * @param L
 * @return
 */
int web_lwebsock_write( lua_State *L );

/**
 * @class laction_arg
 * @author Civetweb programmers
 * @date 25/10/17
 * @file httpd_lua.h
 * @brief
 */
struct laction_arg
{
    lua_State *state;
    const char *script;
    pthread_mutex_t *pmutex;
    char txt[1];
};

/**
 * @brief
 * @param arg
 * @return
 */
int web_lua_action( struct laction_arg *arg );

/**
 * @brief
 * @param arg
 * @return
 */
int web_lua_action_free( struct laction_arg *arg );

/**
 * @brief
 * @param L
 * @param is_periodic
 * @return
 */
int web_lwebsocket_set_timer( lua_State *L, int is_periodic );

/**
 * @brief
 * @param L
 * @return
 */
int web_lwebsocket_set_timeout( lua_State *L );

/**
 * @brief
 * @param L
 * @return
 */
int web_lwebsocket_set_interval( lua_State *L );

enum
{
    // web server page
    WEB_LUA_ENV_TYPE_LUA_SERVER_PAGE = 0,
    // Standard Lua script
    WEB_LUA_ENV_TYPE_PLAIN_LUA_PAGE = 1,
    // Websockets
    WEB_LUA_ENV_TYPE_LUA_WEBSOCKET = 2,
};

/**
 * @brief
 * @param conn
 * @param L
 */
void web_prepare_lua_request_info( struct web_connection *conn,
                                    lua_State *L );

/**
 * @brief
 * @param L
 */
void web_open_lua_libs( lua_State *L );

/**
 * @brief Prepare the Lua environment.
 * @param ctx - web context. Can be NULL
 * @param conn - Active connection. Can be NULL
 * @param ws_conn_list - websocket connetions. Can be NULL.
 * @param L - Lua context
 * @param script_name - Name for the script
 * @param lua_env_type - Environment type
 */
void web_prepare_lua_environment( struct web_context *ctx,
                                    struct web_connection *conn,
                                    struct lua_websock_data *ws_conn_list,
                                    lua_State *L,
                                    const char *script_name,
                                    int lua_env_type );

/**
 * @brief
 * @param L
 * @return
 */
int web_lua_error_handler( lua_State *L );

/**
 * @brief
 * @param ud
 * @param ptr
 * @param osize
 * @param nsize
 */
void *web_lua_allocator( void *ud, void *ptr, size_t osize, size_t nsize );

/**
 * @brief Execute a plain Lua script. Lua in-server module script: a CGI like 
 * script used to generate the entire reply.
 * @param conn - Active connection
 * @param path - Path to script
 * @param exports - Exported functions
 */
void web_exec_lua_script( struct web_connection *conn,
                            const char *path,
                            const void **exports );

/**
 * @brief Lua server page: an SSI like page containing mostly plain html code 
 * plus some tags with server generated contents.
 * @param conn
 * @param path
 * @param filep
 * @param ls
 * @return
 */
int web_handle_lsp_request( struct web_connection *conn,
                                const char *path,
                                struct web_file *filep,
                                struct lua_State *ls );

/**
 * @brief
 * @param script
 * @param conn
 */
void *web_lua_websocket_new( const char *script, struct web_connection *conn );

/**
 * @brief
 * @param conn
 * @param bits
 * @param data
 * @param data_len
 * @param ws_arg
 * @return
 */
int web_lua_websocket_data( struct web_connection *conn,
                                int bits,
                                char *data,
                                size_t data_len,
                            void *ws_arg );

/**
 * @brief
 * @param conn
 * @param ws_arg
 * @return
 */
int web_lua_websocket_ready( struct web_connection *conn, void *ws_arg );

/**
 * @brief
 * @param conn
 * @param ws_arg
 */
void web_lua_websocket_close( struct web_connection *conn, void *ws_arg );

/**
 * @brief Run a Lua script in a file
 * @param file_name File to execute
 * @param ctx
 * @param ebuf
 * @param ebuf_len
 * @return
 */
lua_State *web_exec_lua_context_file_script( const char *file_name,
                                                struct web_context *ctx,
                                                char *ebuf,
                                                size_t ebuf_len );

/**
 * @brief Run a Lua script in a string
 * @param pLuaStr String containg Lua scrit to run
 * @param ctx
 * @param ebuf
 * @param ebuf_len
 * @return
 */
lua_State *web_exec_lua_context_string_script( const char *pLuaStr,
                                                  struct web_context *ctx,
                                                  char *ebuf,
                                                  size_t ebuf_len );

/**
 * @brief
 */
void web_lua_init_optional_libraries( void );

/**
 * @brief
 */
void web_lua_exit_optional_libraries( void );


/**
 * @brief Run Lua script in a file.
 * @param file_name
 * @return
 */
int web_run_lua_file_script( const char *file_name );

/*!
 *
 * Run Lua script in string 
 *
 */
int
web_run_lua_string_script( const char *pStrLua );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif