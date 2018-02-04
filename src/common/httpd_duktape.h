// This file is part of the CivetWeb web server.
// See https://github.com/civetweb/civetweb/
// (C) 2015-2017 by the CivetWeb authors, MIT license.


#ifndef INCLUDE_HTTDP_DUKTAPE___H
#define INCLUDE_HTTDP_DUKTAPE___H

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Many of the functions here will be moved back to static so beware.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <duktape.h>

static const char *web_conn_id = "\xFF"
        "web_conn";
static const char *web_ctx_id = "\xFF"
        "web_ctx";

/**
 * @brief
 * @param udata
 * @param size
 */
void *web_duk_mem_alloc( void *udata, duk_size_t size );

/**
 * @brief
 * @param udata
 * @param ptr
 * @param newsize
 */
void *web_duk_mem_realloc( void *udata, void *ptr, duk_size_t newsize );

/**
 * @brief
 * @param udata
 * @param ptr
 */
void web_duk_mem_free( void *udata, void *ptr );

/**
 * @brief
 * @param duk_ctx
 * @param code
 * @param msg
 */
void web_duk_fatal_handler( duk_context *duk_ctx,
                                duk_errcode_t code,
                                const char *msg );

/**
 * @brief
 * @param duk_ctx
 * @return
 */
duk_ret_t duk_itf_write( duk_context *duk_ctx );

/**
 * @brief
 * @param duk_ctx
 * @return
 */
duk_ret_t duk_itf_read( duk_context *duk_ctx );

/**
 * @brief
 * @param duk_ctx
 * @return
 */
duk_ret_t duk_itf_getoption( duk_context *duk_ctx );

/**
 * @brief
 * @param conn
 * @param script_name
 */
void web_exec_duktape_script( struct web_connection *conn,
                                const char *script_name );

#endif