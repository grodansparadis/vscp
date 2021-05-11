// hlo.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project
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

#if !defined(VSCP_HLO__INCLUDED_)
#define VSCP_HLO__INCLUDED_

#include <json.hpp>  // Needs C++11  -std=c++11
#include <mustache.hpp>

#include <string>

// https://github.com/nlohmann/json
using json = nlohmann::json;

// https://github.com/kainjow/Mustache
using namespace kainjow::mustache;

/*!   

Command format
--------------------------------------------
{
    "op" : "operation-token",
    "arg" : "argument"
}

or

{
    "op" : "operation-token",
    "arg" : {
        "arg1" : "foo",
        "arg2" : "bar"
        ....
    }
}

Commands
--------
NOOP 
READVAR     full
WRITEVAR
DELVAR
SAVE
LOAD

<cmd command="command" arg="argument" /> 

Reply format
------------
Full format depends on command.

{
    "op" : "xxxx",	
    "rv" : "OK|ERROR" 
    "note" : "optional clear text message"
}

Remote Variable
---------------
{
    "op" : "xxxx",
    arg {
    	"name" : "variable-name",
    	"type" : n,
    	"value" : "binhex encoded value" or numeric 
    }
}

String format is always binhex encoded. A numeric value and 
also be sent this way and also a bool.

*/


// ****************************************************************************
//                        HLO Remote variable operations
//
// A remote variable is the same as an "abstraction" in an MDF
// ****************************************************************************

/*
{
    "op" : "noop"
}

json j;
j["op"] = "noop";

*/
#define HLO_OP_NOOP         "noop"      // No operation

/*
{
    "op" : "readvar",
    "arg" : {
        "name" : "variable name",
    }
}

{
    "op" : "readvar",
    "rv" : "ok",
    "arg" : {
        "name" : "variable name",
        "value" : value
    }
}
*/
#define HLO_OP_READ_VAR     "readvar"   // Read variable

/*
{
    "op" : "writevar",
    "arg" : {
        "name" : "variable name",
        "type" : remote-variable-type,  (defaults to "string")
        "value" : xxxx
    }
}

{
    "op" : "writevar",
    "result" : "ok|error"
}
*/
#define HLO_OP_WRITE_VAR    "writevar"  // Write variable

/*
{
    "op" : "delvar",
    "arg" : {
        "name" : "variable delete",
    }
}
*/
#define HLO_OP_DELETE_VAR   "delvar"    // Delete variable

/*
{
    "op" : "save",
    "arg" : "path"   (if needed)
}
*/
#define HLO_OP_SAVE         "save"      // Save configuration

/*
{
    "op" : "load",
    "arg" : "path"   (if needed)
}
*/
#define HLO_OP_LOAD         "load"      // Load configuration


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
    std::string m_op;

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
