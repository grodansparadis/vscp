// variablecodes.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
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

#if !defined(__REMOTE_VARIABLE_CODES__INCLUDED_)
#define __REMOTE_VARIABLE_CODES__INCLUDED_

#define VAR_MAXBUF_SIZE             0x10000 // Max size for variable strings etc

// Persistence flags
#define VSCP_VAR_PERSISTENT         true
#define VSCP_VAR_NON_PERISTENT      false

// Permissions for attr field
#define PERMISSION_OWNER_READ       0x400
#define PERMISSION_OWNER_WRITE      0x200
#define PERMISSION_OWNER_EXECUTE    0x100
#define PERMISSION_GROUP_READ       0x040 
#define PERMISSION_GROUP_WRITE      0x020 
#define PERMISSION_GROUP_EXECUTE    0x010 
#define PERMISSION_OTHER_READ       0x004
#define PERMISSION_OTHER_WRITE      0x002
#define PERMISSION_OTHER_EXECUTE    0x001

#define PERMISSION_OWNER_ALL        0x700
#define PERMISSION_OWNER_NONE       0x000
#define PERMISSION_GROUP_ALL        0x070 
#define PERMISSION_GROUP_NONE       0x000 
#define PERMISSION_OTHER_ALL        0x007
#define PERMISSION_OTHER_NONE       0x000

#define PERMISSION_ALL_READ         0x444
#define PERMISSION_ALL_WRITE        0x222

#define PERMISSION_ALL_RIGHTS       0x777
#define PERMISSION_NO_RIGHTS        0x000
#define PERMSSION_VARIABLE_DEFAULT  0x744

// ****************************************************************************
//                              Templates
// ****************************************************************************

/*

name,type,index,value

{
	"name": "variable-name",
	"type": 1,
    "index" : 2,
	"value": "This is a test variable"
    "attr" : attribute/permission flags as of above
}

optional index for array element

*/

#define VARIABLE_JSON_TEMPLATE                                                 \
    "{\n"                                                                      \
    "\"name\": \"%s\",\n"                                                      \
    "\"type\": %hu,\n"                                                         \
    "\"attr\": %d,\n"                                                          \
    "\"value\": %s,\n"                                                         \
    "}"



#define VARIABLE_XML_TEMPLATE                                                  \
    "<variable \n"                                                             \
    "name=\"%s\" \n"                                                           \
    "type=\"%hu\" \n"                                                          \
    "attr=\"%d\" \n"                                                           \
    "value=\"%s\" />"

// ****************************************************************************
//                          Remote variable types
// ****************************************************************************

#define VSCP_REMOTE_VARIABLE_CODE_UNASSIGNED      (0)   // No value
#define VSCP_REMOTE_VARIABLE_CODE_STRING          (1)   // String value
#define VSCP_REMOTE_VARIABLE_CODE_BOOLEAN         (2)   // Boolean value
#define VSCP_REMOTE_VARIABLE_CODE_INTEGER         (3)   // Integer value
#define VSCP_REMOTE_VARIABLE_CODE_NUMBER          (3)   // Integer value synonym
#define VSCP_REMOTE_VARIABLE_CODE_LONG            (4)   // Long value
#define VSCP_REMOTE_VARIABLE_CODE_DOUBLE          (5)   // Floating point value

#define VSCP_REMOTE_VARIABLE_CODE_MEASUREMENT     (6)   // VSCP data coding
#define VSCP_REMOTE_VARIABLE_CODE_EVENT           (7)   // VSCP event (Level II)
#define VSCP_REMOTE_VARIABLE_CODE_GUID            (8)   // VSCP GUID
#define VSCP_REMOTE_VARIABLE_CODE_EVENT_DATA      (9)   // VSCP event data
#define VSCP_REMOTE_VARIABLE_CODE_EVENT_CLASS     (10)  // VSCP event class
#define VSCP_REMOTE_VARIABLE_CODE_EVENT_TYPE      (11)  // VSCP event type
#define VSCP_REMOTE_VARIABLE_CODE_EVENT_TIMESTAMP (12)  // VSCP event timestamp
#define VSCP_REMOTE_VARIABLE_CODE_DATETIME        (13)  // Date + Time in iso format (YYYY-MM-DDTHH:MM:SS)
#define VSCP_REMOTE_VARIABLE_CODE_DATE            (14)  // Date in iso format (YYYY-MM-DD)
#define VSCP_REMOTE_VARIABLE_CODE_TIME            (15)  // Time in iso format (HH:MM:SS)
#define VSCP_REMOTE_VARIABLE_CODE_BLOB            (16)  // Base64 binary encoded data

#define VSCP_REMOTE_VARIABLE_CODE_UINT8           (17)
#define VSCP_REMOTE_VARIABLE_CODE_UINT16          (18)
#define VSCP_REMOTE_VARIABLE_CODE_UINT32          (19)
#define VSCP_REMOTE_VARIABLE_CODE_UINT64          (20)

#define VSCP_REMOTE_VARIABLE_CODE_INT8            (21)
#define VSCP_REMOTE_VARIABLE_CODE_INT16           (22)
#define VSCP_REMOTE_VARIABLE_CODE_INT32           (23)
#define VSCP_REMOTE_VARIABLE_CODE_INT64           (24)

#define VSCP_REMOTE_VARIABLE_CODE_MIME            (100)     // Mime type (mime-type;base64 encoded content)
#define VSCP_REMOTE_VARIABLE_CODE_HTML            (101)     // HTML Page
#define VSCP_REMOTE_VARIABLE_CODE_JAVASCRIPT      (102)     // Javascript code
#define VSCP_REMOTE_VARIABLE_CODE_JSON            (103)     // JSON data
#define VSCP_REMOTE_VARIABLE_CODE_XML             (104)     // XML data
#define VSCP_REMOTE_VARIABLE_CODE_SQL             (105)     // SQL data
#define VSCP_REMOTE_VARIABLE_CODE_PYTHON          (106)     // Python code
#define VSCP_REMOTE_VARIABLE_CODE_LUA             (107)     // LUA code


// ****************************************************************************
//                          Remote variable errors
// ****************************************************************************

// Moved to vscp.h


#endif
