// actioncodes.h
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


// VSCP daemon action codes are defined in this file

#if !defined(ACTIONCODES__INCLUDED_)
#define ACTIONCODES__INCLUDED_

//
// Variable substitution for parameters
// ====================================
// %variable - variable (Can't use any of the reserved names below).
// %head - VSCP event head
// %priority - VSCP event priority
// %hardcoded - 1 for hardcoded 0 else.
// %class - VSCP event class
// %type - VSCP event type
// %data - all databytes
// %data[n] - data byte n
// %guid - VSCP GUID
// %obid - VSCP event obid.
// %timestamp - VSCP event timestamp
// %isodate - Current date in ISO format 1999:12:31
// %isotime - Current time in ISO format 23:59:59
// %mstime - current time in milliseconds
// %unixtime - Time since epoch as a 32-bit unsigned long
//
// For current list see VSCP specification.

// noop - Do nothing. The dm-row can be enabled but the action does nothing
#define VSCP_DAEMON_ACTION_CODE_NOOP                        0x00000000  // No operation



// execute - Execute an external program or script. The action parameter consist
// of program name + path plus optional parameters.
// Format of parameter is: path to external program + parameter e.g. "echo Hello World!"   
#define VSCP_DAEMON_ACTION_CODE_EXECUTE                     0x00000010  // Execute external program


// conditional execute - Execute an external program or script if specified variable is true. 
// The action parameter consist of condition variable + program name + path plus optional parameters.
// Format of parameter is: condition variable + path to external program + parameter e.g. "echo Hello World!"   
#define VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL         0x00000011  // Execute external program conditional


// procex - A predefined internal procedure is executed. The action parameter is
// sent as a parameter to the internal procedure. 
// Format: procedure name;Parameter(s)sent as UTF8
#define VSCP_DAEMON_ACTION_INTERNAL_PROCEDURE_EXECUTE       0x00000012  // Execute internal script




// method - A method of a dl/dll is executed.
// The action parameter forms a semicolon separated list with values where the first parameter is the
// name of the dl/dll, the second is the method to call and the last part is sent as a parameter to 
// the method. 
// Fromat: path to dll;method of dll;Parameter(s)sent as UTF8
#define VSCP_DAEMON_ACTION_CODE_EXECUTE_METHOD              0x00000030  // Execute method from dll/dl




// sendevent - Send an event defined by the action parameter.
// Fromat: priority;class;type;GUID;data|variable
#define VSCP_DAEMON_ACTION_CODE_SEND_EVENT                  0x00000040  // Send VSCP event




// bsendevent - Send a conditional event defined by the action parameter. 
// First event is sent if the testcase is evaluated as true and
// the second event is sent if evaluated as false
// Fromat: logical test;priority;class;type;GUID;data;priority;class;type;GUID;data
#define VSCP_DAEMON_ACTION_CODE_SEND_EVENT_CONDITIONAL      0x00000041  // Send VSCP event, conditional




// filesendevent - Send a set of event(s) from a file
// Format: Path to file.
#define VSCP_DAEMON_ACTION_CODE_SEND_EVENTS_FROM_FILE       0x00000042 // Send VSCP events from file


// Send event to remote VSCP server
// Format: Non
#define VSCP_DAEMON_ACTION_CODE_SEND_TO_REMOTE              0x00000043 // Send VSCP events to remote VSCP server


// storevar - Store value in variable
// The value can be another variable. 
// Format: variable;value
#define VSCP_DAEMON_ACTION_CODE_STORE_VARIABLE              0x00000050  // Store in variable


// storearray - Store value in array
// Format; array;index;value
#define VSCP_DAEMON_ACTION_CODE_STORE_ARRAY                 0x00000051  // Store in array


// addvar - Add value to variable
// Format: variable;value to add
#define VSCP_DAEMON_ACTION_CODE_ADD_VARIABLE                0x00000052  // Add to variable


// subvar - Subtract value from variable
// Format: variable;value to subtract
#define VSCP_DAEMON_ACTION_CODE_SUBTRACT_VARIABLE           0x00000053  // Subtract from variable


// multvar - Multiply value with variable
// Format: variable;value to multiply the variable with
#define VSCP_DAEMON_ACTION_CODE_MULTIPLY_VARIABLE           0x00000054	// Multiply with variable


// divvar - Divide value with variable
// Format: variable;value to divide the variable with
#define VSCP_DAEMON_ACTION_CODE_DIVIDE_VARIABLE             0x00000055  // Divide with variable


// checkvarsettrue - Check variable set other to true
// Format: value;unit;operation;variable;flag-variable
#define VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_TRUE         0x00000056  // Check variable true


// checkvarsetfalse - Check variable set other to false
// Format: value;unit;operation;variable;flag-variable
#define VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_FALSE        0x00000057  // Check variable flase

// checkvarsetcond - Check variable set other to logic outcome
// Format: value;unit;operation;variable;flag-variable
#define VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE              0x00000058  // Check variable 
                                                                        // set result variable

// checkmeasurement - Check measurement and set variable to logic outcome
// Format: value;unit;operation;flag-variable
#define VSCP_DAEMON_ACTION_CODE_CHECK_MEASUREMENT           0x00000059  // Check measurement


// starttimer - Start a timer
// Format: timerid;settime
// timerid is any numerical id > 0
// Settime is set as HH:MM:SS:MS
#define VSCP_DAEMON_ACTION_CODE_START_TIMER                 0x00000060  // Start timer

// pausetimer - Pause a timer
// Format: timerid
// timerid is any numerical id > 0
#define VSCP_DAEMON_ACTION_CODE_PAUSE_TIMER                 0x00000061  // Pause timer

// stoptimer - Stop a timer
// Format: timerid
// timerid is any numerical id > 0
#define VSCP_DAEMON_ACTION_CODE_STOP_TIMER                  0x00000062  // Stop timer

// resumetimer - Resume a timer
// Format: timerid
// timerid is any numerical id > 0
#define VSCP_DAEMON_ACTION_CODE_RESUME_TIMER                0x00000063  // Resume timer

// writefile - Write to file
// Format: timerid
// timerid is any numerical id > 0
#define VSCP_DAEMON_ACTION_CODE_WRITE_FILE                  0x00000070  // Write file

// storeifmin - Store if minimum
// Format: variable;unit,sensor-index,zone,subzone
#define VSCP_DAEMON_ACTION_CODE_STORE_MIN                   0x00000071  // Store if minimum

// storeifmax - Store if maximum
// Format: variable;unit,sensor-index,zone,subzone
#define VSCP_DAEMON_ACTION_CODE_STORE_MAX                   0x00000072  // Store if maximum

// http - Get/Post URL
// Format: url
// 
#define VSCP_DAEMON_ACTION_CODE_GET_PUT_POST_URL            0x00000075  // Get/Post URL

// writetable - Write to table
// Format: tablename,date,float
// Example: temperature_furnice,%isobothms,%measurement.float 
#define VSCP_DAEMON_ACTION_CODE_WRITE_TABLE	            0x00000080  // Write Table

// cleartable - Clear table
// Format: tablename[,optional SQL expression]
// Example: table1 
#define VSCP_DAEMON_ACTION_CODE_CLEAR_TABLE	            0x00000081  // Clear Table

// runlua - Run LUA script
// Format: script(params);rv
#define VSCP_DAEMON_ACTION_CODE_RUN_LUASCRIPT               0x00000100  // Run LUA Script

// runjavascript - Run JavaScript
// Format: script(params);rv
#define VSCP_DAEMON_ACTION_CODE_RUN_JAVASCRIPT              0x00000200  // Run JavaScript


#endif
