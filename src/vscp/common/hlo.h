// hlo.h
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

#if !defined(VSCP_HLO__INCLUDED_)
#define VSCP_HLO__INCLUDED_

#include <string>

#define HLO_XML_CMD_COMMAND_TEMPLATE                                           \
    "<vscp-cmd op=\"%s\" "                                                     \
    "name=\"%s\" "                                                             \
    "type=\"%s\" "                                                             \
    "value=\"%s\" "                                                            \
    "full=\"%s\" "                                                             \
    " />"

/*!
{
    "vscp-cmd" : {
        "op" : n,
        "name" : "name",
        "type" : n,
        "value" : "value string",
        "full" : true|false,
    }
}
*/

// All XML commands return this HLO content
//     RESULT="OK"  - for success. Description can be used at will.
//     RESULT="ERR" - for failure with cause of failure in description
#define HLO_XML_CMD_REPLY_TEMPLATE                                             \
    "<vscp-resp op=\"vscp-reply\" "                                            \
    "name=\"%s\" "                                                             \
    "result=\"%s\" "                                                           \
    "description=\"%s\" "                                                      \
    " />"

#define HLO_XML_READ_VAR_REPLY_TEMPLATE                                        \
    "<vscp-resp op=\"vscp-readvar\" "                                          \
    "name=\"%s\" "                                                             \
    "result=\"%s\" "                                                           \
    "type=%d "                                                                 \
    "value=\"%s\" />"

#define HLO_XML_READ_VAR_ERR_REPLY_TEMPLATE                                    \
    "<vscp-resp op=\"vscp-readvar\" "                                          \
    "name=\"%s\" "                                                             \
    "result=\"ERR\" "                                                          \
    "error-code=%d "                                                           \
    "description=\"%s\" />"

// ****************************************************************************
//                        HLO Remote variable operations
// ****************************************************************************

#define HLO_OP_NOOP         0   // No operation
#define HLO_OP_READ_VAR     1   // Read variable
#define HLO_OP_WRITE_VAR    2   // Write variable
#define HLO_OP_SAVE         3   // Save configuration
#define HLO_OP_LOAD         4   // Load configuration
#define HLO_OP_USER_DEFINED 150 // From 150 - 254 user defined codes
#define HLO_OP_UNKNOWN      255 // Unknow command

///////////////////////////////////////////////////////////////////////////////
// VSCP automation HLO object
//

class CHLO
{

  public:
    /// Constructor
    CHLO(void);

    // Destructor
    virtual ~CHLO(void);

    // ---------------------------------------

    // HLO operation
    uint8_t m_op;

    // HLO name (variable name/opt op. arg name)
    std::string m_name;

    // HLO type (Variable type/opt. op arg type
    uint32_t m_varType;

    // HLO value (Variable value/opt- op arg value)
    std::string m_value;

    // For VSCP remote variables full format
    bool m_bFull;

    // Print debug info
    bool m_bDebug;
};

#endif