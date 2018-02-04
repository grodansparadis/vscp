// variablecodes.h
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
#define VSCP_DAEMON_VARIABLE_CODE_DATE                      14  // Date in iso format 
#define VSCP_DAEMON_VARIABLE_CODE_TIME                      15  // Time in iso format 
#define VSCP_DAEMON_VARIABLE_CODE_BLOB                      16  // Base64 binary encoded data

#define VSCP_DAEMON_VARIABLE_CODE_MIME                      100 // Mime type (mime-type;base64 encoded content)
#define VSCP_DAEMON_VARIABLE_CODE_HTML                      101 // HTML Page
#define VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT                102 // Javascript code
#define VSCP_DAEMON_VARIABLE_CODE_JSON                      103 // JSON data
#define VSCP_DAEMON_VARIABLE_CODE_XML                       104 // XML data
#define VSCP_DAEMON_VARIABLE_CODE_SQL                       105 // SQL data

#define VSCP_DAEMON_VARIABLE_CODE_LUA                       200 // LUA script
#define VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT                201 // LUA executed script result string

#define VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1                  300 // Contains XML file for User interface

#define VSCP_DAEMON_VARIABLE_CODE_DM_ROW                    500 // A DM row, comma separated
#define VSCP_DAEMON_VARIABLE_CODE_DRIVER                    501 // Driver item, comma separated
#define VSCP_DAEMON_VARIABLE_CODE_USER                      502 // User item, comma separated
#define VSCP_DAEMON_VARIABLE_CODE_FILTER                    503 // Filter item, comma separated


// When adding new type
// ====================
// add to variables
// Add to webserver admin i/f  variable: edit/list


#endif
