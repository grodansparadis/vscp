// variablecodes.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2016
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


// VSCP daemon variable codes are defined in this file

#if !defined(VARIABLECODES__INCLUDED_)
#define VARIABLECODES__INCLUDED_

#define VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED                0   // No value
#define VSCP_DAEMON_VARIABLE_CODE_STRING                    1   // String value
#define VSCP_DAEMON_VARIABLE_CODE_BOOLEAN                   2   // Boolean value
#define VSCP_DAEMON_VARIABLE_CODE_INTEGER                   3   // Integer value
#define VSCP_DAEMON_VARIABLE_CODE_LONG                      4   // Long value
#define VSCP_DAEMON_VARIABLE_CODE_DOUBLE                    5   // Floating point value
#define VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT          6   // VSCP data coding
#define VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT                7   // VSCP event (Level II)
#define VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID           8   // VSCP event GUID
#define VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA           9   // VSCP event data
#define VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS          10  // VSCP event class
#define VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE           11  // VSCP event type
#define VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP      12  // VSCP event timestamp
#define VSCP_DAEMON_VARIABLE_CODE_DATETIME                  13  // Date + Time in iso format 
#define VSCP_DAEMON_VARIABLE_CODE_BASE64                    14  // Base64 encoded data
#define VSCP_DAEMON_VARIABLE_CODE_DATE                      15  // Date in iso format 
#define VSCP_DAEMON_VARIABLE_CODE_TIME                      16  // Time in iso format 

#define VSCP_DAEMON_VARIABLE_CODE_MIME                      100 // Mime type (mime-type;base64 encoded content)
#define VSCP_DAEMON_VARIABLE_CODE_HTML                      101 // HTML Page
#define VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT                102 // Javascript code
#define VSCP_DAEMON_VARIABLE_CODE_JSON                      103 // JSON data
#define VSCP_DAEMON_VARIABLE_CODE_XML                       104 // XML data
#define VSCP_DAEMON_VARIABLE_CODE_SQL                       105 // SQL data

#define VSCP_DAEMON_VARIABLE_CODE_LUA                       200 // LUA script
#define VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT                201 // LUA executed script result string

#define VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1                  300 // Contains XML file for User interface

#define VSCP_DAEMON_VARIABLE_CODE_DM_ROW                    500 // A DM row, comma seperated
#define VSCP_DAEMON_VARIABLE_CODE_DRIVER                    501 // Driver item, comma seperated
#define VSCP_DAEMON_VARIABLE_CODE_USER                      502 // User item, comma seperated
#define VSCP_DAEMON_VARIABLE_CODE_GROUP                     503 // Group item, comma seperated

// A non persistent variable is stored with the name as the hash value.
// A persistent variable is stored with $ + its name as the hash value.
// A non persistent array is stored with @ + its name as hash value.
// A persistent array is stored with $ + @ + its name as hash value.
#define VSCP_DAEMON_VARIABLE_PREFIX_PERSISTENT              '$' // Prefix for persistent variablename
#define VSCP_DAEMON_VARIABLE_PREFIX_ARRAY                   '@' // Prefix for array


#endif
