// duktape_vscp.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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
//

#if !defined(VSCP_LUA_VSCP__INCLUDED_)
#define VSCP_LUA_VSCP__INCLUDED_



/*!
 *  print data on console if available
 * 
 * Lua Parameter 0..n: String to log.
 * Lua Return: Nothing.
 * 
 */
int lua_vscp_print( struct lua_State *L );

/*!
 *  Log data
 * 
 * Lua Parameter 0: String to log.
 * Lua Parameter 1: level (int)
 * Lua Parameter 2: type (int)
 * Lua Return: True on success.
 * 
 */
int lua_vscp_log( struct lua_State *L );

/*!
 *  Sleep for some milliseconds
 * 
 * Lua Parameter 0: Sleep time in milliseconds
 * Lua Return: True on success.
 * 
 */
int lua_vscp_sleep( struct lua_State *L );


/*!
 *  Sleep for some milliseconds
 * 
 * Lua Parameter 0: String to encode in BASE64 format
 * Lua Return: Encoded string
 * 
 */
int lua_vscp_base64_encode( struct lua_State *L );


/*!
 * Decode a BASE64 string
 * 
 * Lua Parameter 0: BASE64 string to decode.
 * Return: Decoded string.
 * 
 */
int lua_vscp_base64_decode( struct lua_State *L );


/*!
 *  Get variable in specific format from it's name.
 *  
 *  vscp.readvariable( "name"[, format ] )
 * 
 *  Lua Parameter 1: Variable name
 *  Lua Parameter 2: Optional format
 *  Lua: Return: Variable in selected format or error. 
 * 
 *      format = 0 - string   ***default****
 *      format = 1 - XML
 *      format = 2 - JSON
 *      format = 3 - Just value (can be base64 encoded)
 *      format = 4 - Just note (always base64 encoded)
 */
int lua_vscp_readVariable( struct lua_State *L );

/*!
 *  Write a VSCP variable and create it if it does not exist.
 * 
 * vscp.readvariable( "name"[, format ] )
 * 
 *  Lua Parameter 1: variable
 *  Lua Parameter 2: Optional format
 *  Lua: Return: 1 on success.
 * 
 */
int lua_vscp_writeVariable( struct lua_State *L );

/*!
 * Write remote variable value
 * 
 * Lua Parameter 1: variable name
 * Lua Parameter 2: variable value
 * Lua Parameter 3: OPTIONAL Flag for BASE64 coding of value.
 * Lua: Return: 1 on success.
 * 
 */
int lua_vscp_writeVariableValue( struct lua_State *L );

/*!
 *  Delete VSCP variable
 * 
 *  Lua Parameter 0: ClientItem
 *  Lua Parameter 1: Name of variable
 *  Lua: Return: 1 on success.
 * 
 */
int lua_vscp_deleteVariable( struct lua_State *L );


int lua_vscp_isVariableBase64Encoded( struct lua_State *L ) ;

int lua_vscp_isVariablePersistent( struct lua_State *L );

int lua_vscp_isVariableNumerical( struct lua_State *L );

int lua_vscp_isStockVariable( struct lua_State *L );

/*!
 *  Send VSCP event on the local client queue
 * 
 *  Lua Parameter 0: ClientItem
 *  Lua Parameter 1: Event as JSON object
 *  Lua: Return: True if variable got updated/added.
 */
int lua_vscp_sendEvent( struct lua_State *L );

/*!
 * Fetch on VSCP event from the local client queue
 * 
 *  Lua Parameter 0: ClientItem
 *  Lua: Return: Event as JSON object or NULL if no event available
 */
int lua_vscp_getEvent( struct lua_State *L );

/*!
 * Return number of events in the local client queue
 * 
 *  Lua Parameter 0: ClientItem
 *  Lua: Return: Number of events in the client queue
 */
int lua_vscp_getCountEvent( struct lua_State *L ); 

/*!
 * Set VSCP filter for the local client queue
 * 
 *  Lua Parameter 0: ClientItem
 *  Lua Parameter 1: Filter + Mask in JSON format.
 *  Lua: Return: True for success, False for failure
 */
int lua_vscp_setFilter( struct lua_State *L );


/*!
 * Check if a measurement is a measurement event.
 * 
 *  Lua Parameter 0: Event to check in JSON format.
 *  Lua: Return: True if event is a measurement, False if not.
 */
int lua_is_Measurement( struct lua_State *L );

/*!
 * Send measurement event
 * 
 *  Lua Parameter 0: Event to check in JSON format.
 *  Lua: Return: True if event is a measurement, False if not.
 */
int lua_send_Measurement( struct lua_State *L );


/*!
 *  Get measurement value
 *  Lua Parameter 0: Event to check in JSON format.
 *  Lua: Return: Value as double.
 */
int lua_get_MeasurementValue( struct lua_State *L ); 

/*!
 * Get unit from measurement event
 */
int lua_get_MeasurementUnit( struct lua_State *L );


/*!
 * Get sensor index from measurement event
 */
int lua_get_MeasurementSensorIndex( struct lua_State *L );


/*!
 * Get zone from measurement event
 */
int lua_get_MeasurementZone( struct lua_State *L );


/*!
 * Get sub zone from measurement event
 */

int lua_get_MeasurementSubZone( struct lua_State *L );




#endif
