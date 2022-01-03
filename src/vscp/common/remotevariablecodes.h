// variablecodes.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, the VSCP project
// <info@vscp.org>
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

#define VAR_MAXBUF_SIZE 0x10000 // Max size for variable strings etc

// Persistence flags
#define VSCP_VAR_PERSISTENT    true
#define VSCP_VAR_NON_PERISTENT false

// Permissions for attr field
#define PERMISSION_OWNER_READ    0x400
#define PERMISSION_OWNER_WRITE   0x200
#define PERMISSION_OWNER_EXECUTE 0x100
#define PERMISSION_GROUP_READ    0x040
#define PERMISSION_GROUP_WRITE   0x020
#define PERMISSION_GROUP_EXECUTE 0x010
#define PERMISSION_OTHER_READ    0x004
#define PERMISSION_OTHER_WRITE   0x002
#define PERMISSION_OTHER_EXECUTE 0x001

#define PERMISSION_OWNER_ALL  0x700
#define PERMISSION_OWNER_NONE 0x000
#define PERMISSION_GROUP_ALL  0x070
#define PERMISSION_GROUP_NONE 0x000
#define PERMISSION_OTHER_ALL  0x007
#define PERMISSION_OTHER_NONE 0x000

#define PERMISSION_ALL_READ  0x444
#define PERMISSION_ALL_WRITE 0x222

#define PERMISSION_ALL_RIGHTS      0x777
#define PERMISSION_NO_RIGHTS       0x000
#define PERMSSION_VARIABLE_DEFAULT 0x744

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

#define VARIABLE_JSON_TEMPLATE                                                                                         \
  "{\n"                                                                                                                \
  "\"name\": \"%s\",\n"                                                                                                \
  "\"type\": %hu,\n"                                                                                                   \
  "\"attr\": %d,\n"                                                                                                    \
  "\"value\": %s,\n"                                                                                                   \
  "}"

#define VARIABLE_XML_TEMPLATE                                                                                          \
  "<variable \n"                                                                                                       \
  "name=\"%s\" \n"                                                                                                     \
  "type=\"%hu\" \n"                                                                                                    \
  "attr=\"%d\" \n"                                                                                                     \
  "value=\"%s\" />"

// ****************************************************************************
//                          Remote variable types
// ****************************************************************************

#define VSCP_REMOTE_VARIABLE_CODE_UNASSIGNED (0) // No type
#define VSCP_REMOTE_VARIABLE_CODE_STRING     (1) // String value
#define VSCP_REMOTE_VARIABLE_CODE_BOOLEAN    (2) // Boolean value

#define VSCP_REMOTE_VARIABLE_CODE_DATE (3) // Date in three bytes YY,MM,DD
#define VSCP_REMOTE_VARIABLE_CODE_TIME (4) // Time in three bytes HH,MM,SS

#define VSCP_REMOTE_VARIABLE_CODE_INT8   (5)
#define VSCP_REMOTE_VARIABLE_CODE_UINT8  (6)
#define VSCP_REMOTE_VARIABLE_CODE_INT16  (7)
#define VSCP_REMOTE_VARIABLE_CODE_UINT16 (8)

#define VSCP_REMOTE_VARIABLE_CODE_INT32   (9)
#define VSCP_REMOTE_VARIABLE_CODE_INTEGER (10)
#define VSCP_REMOTE_VARIABLE_CODE_NUMBER  (11)
#define VSCP_REMOTE_VARIABLE_CODE_UINT32  (12)

#define VSCP_REMOTE_VARIABLE_CODE_INT64  (13)
#define VSCP_REMOTE_VARIABLE_CODE_UINT64 (14)

// ****************************************************************************
//                          Remote variable errors
// ****************************************************************************

// Moved to vscp.h

#endif
