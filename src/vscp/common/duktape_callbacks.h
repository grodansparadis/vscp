// duktape_callbacks.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2017
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#define JAVASCRIPT_OK       1
#define JAVASCRIPT_ERROR    0

// JavaScripts executes as the "javascript" user.

// For loadable modules node style
duk_ret_t js_resolve_module( duk_context *ctx );
duk_ret_t js_load_module( duk_context *ctx );

/*!
 *  print data on console if available
 * 
 * JavaScript Parameter 0..n: String to log.
 * JavaScript Return: Nothing.
 * 
 */
duk_ret_t js_vscp_print( duk_context *ctx );

/*!
 *  Log data
 * 
 * JavaScript Parameter 0: Stringto log.
 * JavaScript Parameter 1: level (int)
 * JavaScript Parameter 2: type (int)
 * JavaScript Return: Nothing.
 * 
 */
duk_ret_t js_vscp_log( duk_context *ctx );

/*!
 *  Sleep for some milliseconds
 * 
 * JavaScript Parameter 0: Sleep time in milliseconds
 * JavaScript Return: True on success.
 * 
 */
duk_ret_t js_vscp_sleep( duk_context *ctx );


/*!
 *  Get variable as a JSON object from it's name. If the variable is
 *  not found a null object is returned.
 *  
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript Parameter 1: Name of variable to read
 *  JavaScript: Return: Variable on JSON format. NULL if no variable with
 *                      that name exists.
 * 
 *  @param v7 Pointer to v7 object.
 *  @param res Pointer to JSON object or NULL if variable was not found.
 *  @return v7 error code.
 */
duk_ret_t js_vscp_readVariable( duk_context *ctx );

/*!
 *  Write a VSCP variable and create it if it does not exist.
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript Parameter 1: Variable as JSON object
 *  JavaScript: Return: True if variable got updated/added. 
 * 
 *  @param v7 Pointer to v7 object.
 *  @param res Pointer to JSON object or NULL if variable was not found.
 *  @return v7 error code.
 */
duk_ret_t js_vscp_writeVariable( duk_context *ctx );

/*!
 *  Delete VSCP variable
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript Parameter 1: Name of variable
 *  JavaScript: Return: True if variable got deleted.
 * 
 *  @param v7 Pointer to v7 object.
 *  @param res Pointer to JSON object or NULL if variable was not found.
 *  @return v7 error code.
 */
duk_ret_t js_vscp_deleteVariable( duk_context *ctx );


/*!
 *  Send VSCP event on the local client queue
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript Parameter 1: Event as JSON object
 *  JavaScript: Return: True if variable got updated/added.
 */
duk_ret_t js_vscp_sendEvent( duk_context *ctx );

/*!
 * Fetch on VSCP event from the local client queue
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript: Return: Event as JSON object or NULL if no event available
 */
duk_ret_t js_vscp_getEvent( duk_context *ctx );

/*!
 * Return number of events in the local client queue
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript: Return: Number of events in the client queue
 */
duk_ret_t js_vscp_getCountEvent( duk_context *ctx ); 

/*!
 * Set VSCP filter for the local client queue
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript Parameter 1: Filter + Mask in JSON format.
 *  JavaScript: Return: True for success, False for failure
 */
duk_ret_t js_vscp_setFilter( duk_context *ctx );


/*!
 * Check if a measurement is a measurement event.
 * 
 *  JavaScript Parameter 0: Event to check in JSON format.
 *  JavaScript: Return: True if event is a measurement, False if not.
 */
duk_ret_t js_is_Measurement( duk_context *ctx );

/*!
 * Send measurement event
 * 
 *  JavaScript Parameter 0: Event to check in JSON format.
 *  JavaScript: Return: True if event is a measurement, False if not.
 */
duk_ret_t js_send_Measurement( duk_context *ctx );


/*!
 *  Get measurement value
 *  JavaScript Parameter 0: Event to check in JSON format.
 *  JavaScript: Return: Value as double.
 */
duk_ret_t js_get_MeasurementValue( duk_context *ctx ); 

/*!
 * Get unit from measurement event
 */
duk_ret_t js_get_MeasurementUnit( duk_context *ctx );


/*!
 * Get sensor index from measurement event
 */
duk_ret_t js_get_MeasurementSensorIndex( duk_context *ctx );


/*!
 * Get zone from measurement event
 */
duk_ret_t js_get_MeasurementZone( duk_context *ctx );


/*!
 * Get sub zone from measurement event
 */

duk_ret_t js_get_MeasurementSubZone( duk_context *ctx );
