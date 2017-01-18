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
    Get variable as a JSON object from it's name. If the variable is
    not found a null object is returned.
  
    @param v7 Pointer to v7 object.
    @param res Pointer to JSON object or NULL if variable was not found.
    @return v7 error code.
 */
enum v7_err js_read_VSCP_Variable( struct v7 *v7, v7_val_t *res );

/*!
    Write a VSCP variable and create it if it does not exist.
  
    @param v7 Pointer to v7 object.
    @param res Pointer to JSON object or NULL if variable was not found.
    @return v7 error code.
 */
enum v7_err js_write_VSCP_Variable( struct v7 *v7, v7_val_t *res );