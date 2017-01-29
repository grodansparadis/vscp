// v7.h
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


// JavaScripts executes as the "javascript" user.

/*!
 *  Log data
 * 
 * JavaScript Parameter 0: String to log.
 * JavaScript Return: Nothing.
 * 
 */
enum v7_err js_vscp_log( struct v7 *v7, v7_val_t *res );


/*!
 *  Get variable as a JSON object from it's name. If the variable is
 *  not found a null object is returned.
 *  
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript Parameter 1: Name of variable to read
 *  JavaScript: Return: Variable on JSON format. NULL if no variable with
 *                      that name.
 * 
 *  @param v7 Pointer to v7 object.
 *  @param res Pointer to JSON object or NULL if variable was not found.
 *  @return v7 error code.
 */
enum v7_err js_vscp_readVariable( struct v7 *v7, v7_val_t *res );

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
enum v7_err js_vscp_writeVariable( struct v7 *v7, v7_val_t *res );

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
enum v7_err js_vscp_deleteVariable( struct v7 *v7, v7_val_t *res );


/*!
 *  Send VSCP event on the local client queue
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript Parameter 1: Event as JSON object
 *  JavaScript: Return: True if variable got updated/added.
 */
enum v7_err js_vscp_sendEvent( struct v7 *v7, v7_val_t *res );

/*!
 * Fetch on VSCP event from the local client queue
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript: Return: Event as JSON object or NULL if no event available
 */
enum v7_err js_vscp_getEvent( struct v7 *v7, v7_val_t *res );

/*!
 * Return number of events in the local client queue
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript: Return: Number of events in the client queue
 */
enum v7_err js_vscp_getCountEvent( struct v7 *v7, v7_val_t *res ); 

/*!
 * Set VSCP filter for the local client queue
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript Parameter 1: Filter + Mask in JSON format.
 *  JavaScript: Return: True for success, False for failure
 */
enum v7_err js_vscp_setFilter( struct v7 *v7, v7_val_t *res );


/*!
 * Check if a measurement is a measurement event.
 * 
 *  JavaScript Parameter 0: Event to check in JSON format.
 *  JavaScript: Return: True if event is a measurement, False if not.
 */
enum v7_err js_is_Measurement( struct v7 *v7, v7_val_t *res );

/*!
 * Send measurement event
 * 
 *  JavaScript Parameter 0: Event to check in JSON format.
 *  JavaScript: Return: True if event is a measurement, False if not.
 */
enum v7_err js_send_Measurement( struct v7 *v7, v7_val_t *res );

/*!
 * Get measurement value
 * 
 *  JavaScript Parameter 0: ClientItem
 *  JavaScript Parameter 2: Measurement in JSON format.
 *  JavaScript: Return: Measurement in JSON format,
 */
enum v7_err js_get_Measurement( struct v7 *v7, v7_val_t *res );

/*!
 *  Get measurement value
 *  JavaScript Parameter 0: Event to check in JSON format.
 *  JavaScript: Return: Value as double.
 */
enum v7_err js_get_MeasurementValue( struct v7 *v7, v7_val_t *res ); 

/*!
 * Get unit from measurement event
 */
enum v7_err js_get_MeasurementUnit( struct v7 *v7, v7_val_t *res );


/*!
 * Get sensor index from measurement event
 */
enum v7_err js_get_MeasurementSensorIndex( struct v7 *v7, v7_val_t *res ); 


/*!
 * Get zone from measurement event
 */
enum v7_err js_get_MeasurementZone( struct v7 *v7, v7_val_t *res );


/*!
 * Get sub zone from measurement event
 */

enum v7_err js_get_MeasurementSubZone( struct v7 *v7, v7_val_t *res );