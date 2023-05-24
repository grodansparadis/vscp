# Civetweb API Reference

### `struct mg_callbacks;`

### Fields

| Field | Description |
| :--- | :--- |
|**`begin_request`**|**`int (*begin_request)( struct mg_connection *conn );`**|
| |The `begin_request()` callback function is called when CivetWeb has received a new HTTP request. If the callback function does not process the request, it should return 0. In that case CivetWeb will handle the request with the default callback routine. If the callback function returns a value between 1 and 999, CivetWeb does nothing and the callback function should do all the processing, including sending the proper HTTP headers etc. Starting at CivetWeb version 1.7, the function `begin_request()` is called before any authorization is done. If an authorization check is required, `request_handler()` should be used instead. The return value of the callback function is not only used to signal CivetWeb to not further process the request. The returned value is also stored as HTTP status code in the access log. |
|**`end_request`**|**`void (*end_request)(const struct mg_connection *conn, int reply_status_code);`**|
| |The callback function `end_request()` is called by CivetWeb when a request has been completely processed. It sends the reply status code which was sent to the client to the application.|
|**`log_message`**|**`int (*log_message)( const struct mg_connection *conn, const char *message );`**|
| |The callback function `log_message()` is called when CivetWeb is about to log a message. If the callback function returns 0, CivetWeb will use the default internal log routines to log the message. If a non-zero value is returned CivetWeb assumes that logging has already been done and no further action is performed.|
|**`log_access`**|**`int (*log_access)( const struct mg_connection *conn, const char *message );`**|
| |The callback function `log_access()` is called when CivetWeb is about to log a message. If the callback function returns 0, CivetWeb will use the default internal access log routines to log the access. If a non-zero value is returned, CivetWeb assumes that access logging has already been done and no further action is performed.|
|**`init_ssl`**|**`int (*init_ssl)( void *ssl_ctx, void *user_data );`**|
| |The callback function `init_ssl()` is called when CivetWeb initializes the SSL library. The `ssl_ctx` parameter is a pointer to the SSL context being configure. The parameter `user_data` contains a pointer to the data which was provided to `mg_start()` when the server was started. The callback function can return 0 to signal that CivetWeb should setup the SSL certificate. With a return value of 1 the callback function signals CivetWeb that the certificate has already been setup and no further processing is necessary. The value -1 should be returned when the SSL initialization fails.|
|**`init_ssl_domain`**|**`int (*init_ssl_domain)( const char *server_domain, void *ssl_ctx, void *user_data );`**|
| |The callback function `init_ssl_domain()` is called when CivetWeb initializes the SSL library. The parameter `server_domain` is a pointer to the `authentication_domain` config parameter of the domain being configured. The `ssl_ctx` parameter is a pointer to the SSL context being configure. The parameter `user_data` contains a pointer to the data which was provided to `mg_start()` when the server was started. The callback function can return 0 to signal that CivetWeb should setup the SSL certificate. With a return value of 1 the callback function signals CivetWeb that the certificate has already been setup and no further processing is necessary. The value -1 should be returned when the SSL initialization fails.|
|**`external_ssl_ctx`**|**`int (*external_ssl_ctx)( void **ssl_ctx, void *user_data );`**|
| |The callback function `external_ssl_ctx()` is called when civetweb is about to create (`*ssl_ctx` is `NULL`) or free (`*ssl_ctx` is not `NULL`) a SSL context. The parameter `user_data` contains a pointer to the data which was provided to `mg_start()` when the server was started. The callback function can return 0 to signal that CivetWeb should setup the SSL context. With a return value of 1 the callback function signals CivetWeb that the SSL context has already been setup and no further processing is necessary. Also with a return value of 1 other callback functions `init_ssl()` and `init_ssl_domain()` are not called. The value -1 should be returned when the SSL context initialization fails.|
|**`external_ssl_ctx_domain`**|**`int (*external_ssl_ctx_domain)( const char *server_domain, void **ssl_ctx, void *user_data );`**|
| |The callback function `external_ssl_ctx_domain()` is called when civetweb is about to create (`*ssl_ctx` is `NULL`) or free (`*ssl_ctx` is not `NULL`) a SSL context. The parameter `server_domain` is a pointer to the `authentication_domain` config parameter of the domain being created or freed. The parameter `user_data` contains a pointer to the data which was provided to `mg_start()` when the server was started. The callback function can return 0 to signal that CivetWeb should setup the SSL context. With a return value of 1 the callback function signals CivetWeb that the SSL context has already been setup and no further processing is necessary. Also with a return value of 1 other callback functions `init_ssl()` and `init_ssl_domain()` are not called. The value -1 should be returned when the SSL context initialization fails.|
|**`connection_close`**|**`void (*connection_close)( const struct mg_connection *conn );`**|
| |The callback function `connection_close()` is called before CivetWeb closes a connection. The per-context mutex is locked when the callback function is invoked. The function is primarily useful for noting when a websocket is closing and removing it from any application-maintained list of clients. *Using this callback for websocket connections is deprecated. Use* `mg_set_websocket_handler()` *instead.*|
|**`connection_closed`**|**`void (*connection_closed)( const struct mg_connection *conn );`**|
| |The callback function `connection_closed()` is called after CivetWeb has closed a connection (after SSL shutdown and after closing the socket). The per-context mutex is locked when the callback function is invoked.|
|**`init_lua`**|**`void (*init_lua)( const struct mg_connection *conn, void *lua_context, unsigned context_flags );`**|
| |The callback function `init_lua()` is called just before a Lua server page is to be served. Lua page serving must have been enabled at compile time for this callback function to be called. The parameter `lua_context` is a `lua_State *` pointer. The parameter `context_flags` indicate the type of Lua environment. |
|**`exit_lua`**|**`void (*init_lua)( const struct mg_connection *conn, void *lua_context, unsigned context_flags );`**|
| |The callback function `exit_lua()` is called when a Lua state is about to be closed. Lua page serving must have been enabled at compile time for this callback function to be called. The parameters are identical to `lua_init()`.|
|**`http_error`**|**`int (*http_error)( struct mg_connection *conn, int status, const char *msg );`**|
| |The callback function `http_error()` is called by CivetWeb just before an HTTP error is to be sent to the client. The function allows the application to send a custom error page. The status code of the error is provided as a parameter. If the application sends their own error page, it must return 0 to signal CivetWeb that no further processing is needed. If the returned value is not 0, CivetWeb will send an error page to the client.|
|**`init_context`**|**`void (*init_context)( const struct mg_context *ctx );`**|
| |The callback function `init_context()` is called after the CivetWeb server has been started and initialized, but before any requests are served. This allows the application to perform some initialization activities before the first requests are handled.|
|**`exit_context`**|**`void (*exit_context)( const struct mg_context *ctx );`**|
| |The callback function `exit_context()` is called by CivetWeb when the server is stopped. It allows the application to do some cleanup on the application side.|
|**`init_thread`**|**`void * (*init_thread)( const struct mg_context *ctx, int thread_type );`**|
| |The callback function `init_thread()` is called when a new thread is created by CivetWeb. It is always called from the newly created thread and can be used to initialize thread local storage data. The `thread_type` parameter indicates which type of thread has been created. following thread types are recognized:|
| |**0** - The master thread is created |
| |**1** - A worker thread which handles client connections has been created|
| |**2** - An internal helper thread (timer thread) has been created|
| |**3** - A websocket client thread has been created|
| |The returned `void *` is stored as user defined pointer in the thread local storage.|
|**`exit_thread`**|**`void (*exit_thread)( const struct mg_context *ctx, int thread_type, void * user_ptr);`**|
| |The callback function `exit_thread()` is called when a thread is about to exit. The parameters correspond to `init_thread`, with `user_ptr` being the return value.|
|**`init_connection`**|**`int (*init_connection)( struct mg_cconnection *conn, void ** conn_data);`**|
| |The callback function `init_connection()` is called when a new connection is created. It can be used to set user defined connection data (type `void *`) by setting `*conn_data`.|


### Description

Much of the functionality in the Civetweb library is provided through callback functions. The application registers their own processing functions with the Civetweb library and when an event happens, the appropriate callback function is called. In this way an application is able to have their processing code right at the heart of the webserver, without the need to change the code of the webserver itself. A number of callback functions are registered when the civetweb subsystem is started. Other may be added or changed at runtime with helper functions.

A pointer to a `mg_callbacks` structure is passed as parameter to the [`mg_start()`](mg_start.md) function to provide links to callback functions which the webserver will call at specific events. If a specific callback function is not supplied, CivetWeb will fallback to default internal callback routines. Callback functions give the application detailed control over how specific events should be handled.

### See Also

* [`mg_start();`](mg_start.md)
* [`mg_stop();`](mg_stop.md)
