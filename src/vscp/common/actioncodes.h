// actioncodes.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 
// Grodans Paradis AB, <akhe@grodansparadis.com>
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
//
// For current list see VSCP specification.

// Do noting. The dm-row can be enabled but the action does nothing
#define VSCP_DAEMON_ACTION_CODE_NOOP                        0x00000000  // No operation



// Execute an external program or script. The action parameter consist
// of program name + path plus optional parameters.
// Format of parameter is: path to external program + parameter e.g. "echo Hello World!"   
#define VSCP_DAEMON_ACTION_CODE_EXECUTE                     0x00000010  // Execute external program



// A predefined internal procedure is executed. The action parameter is
// sent as a parameter to the internal procedure. 
// Format: procedure name;Parameter(s)sent as UTF8
#define VSCP_DAEMON_ACTION_INTERNAL_PROCEDURE_EXECUTE       0x00000012  // Execute internal script




// A method of a dl/dll is executed.
// The action parameter forms a semicolon separated list with values where the first parameter is the
// name of the dl/dll, the second is the method to call and the last part is sent as a parameter to 
// the method. 
// Fromat: path to dll;method of dll;Parameter(s)sent as UTF8
#define VSCP_DAEMON_ACTION_CODE_EXECUTE_METHOD              0x00000030  // Execute method from dll/dl




// Send an event defined by the action parameter.
// Fromat: priority;class;type;GUID;data|variable
#define VSCP_DAEMON_ACTION_CODE_SEND_EVENT                  0x00000040  // Send VSCP event




// Send a conditional event defined by the action parameter. 
// First event is sent if the testcase is evaluated as true and
// the second event is sent if evaluated as false
// Fromat: logical test;priority;class;type;GUID;data;priority;class;type;GUID;data
#define VSCP_DAEMON_ACTION_CODE_SEND_EVENT_CONDITIONAL      0x00000041  // Send VSCP event, conditional




// Send a set of event(s) from a file
// Format: Path to file.
#define VSCP_DAEMON_ACTION_CODE_SEND_EVENTS_FROM_FILE       0x00000042 // Send VSCP events from file




// Send event to remote VSCP server
// Format: Non
#define VSCP_DAEMON_ACTION_CODE_SEND_TO_REMOTE              0x00000043 // Send VSCP events to remote VSCP server





// Store value in variable
// The value can be another variable. 
// Format: variable;value
#define VSCP_DAEMON_ACTION_CODE_STORE_VARIABLE              0x00000050  // Store in variable




// Store value in array
// Format; array;index;value
#define VSCP_DAEMON_ACTION_CODE_STORE_ARRAY                 0x00000051  // Store in array




// Add value to variable
// Format: variable;value to add
#define VSCP_DAEMON_ACTION_CODE_ADD_VARIABLE                0x00000052  // Add to variable




// Subtract value from variable
// Format: variable;value to subtract
#define VSCP_DAEMON_ACTION_CODE_SUBTRACT_VARIABLE           0x00000053  // Subtract from variable




// Multiply value with variable
// Format: variable;value to multiply the variable with
#define VSCP_DAEMON_ACTION_CODE_MULTIPLY_VARIABLE           0x00000054	// Multiply with variable




// Divide value with variable
// Format: variable;value to divide the variable with
#define VSCP_DAEMON_ACTION_CODE_DIVIDE_VARIABLE             0x00000055  // Divide with variable


// Start a timer
// Format: timerid;settime
// timerid is any numerical id > 0
// Settime is set as HH:MM:SS:MS
#define VSCP_DAEMON_ACTION_CODE_START_TIMER					0x00000060  // Start timer

// Pause a timer
// Format: timerid
// timerid is any numerical id > 0
#define VSCP_DAEMON_ACTION_CODE_PAUSE_TIMER					0x00000061  // Pause timer

// Stop a timer
// Format: timerid
// timerid is any numerical id > 0
#define VSCP_DAEMON_ACTION_CODE_STOP_TIMER					0x00000062  // Stop timer

// Resume a timer
// Format: timerid
// timerid is any numerical id > 0
#define VSCP_DAEMON_ACTION_CODE_RESUME_TIMER				0x00000063  // Resume timer

// Write to file
// Format: timerid
// timerid is any numerical id > 0
#define VSCP_DAEMON_ACTION_CODE_WRITE_FILE					0x00000070  // Write file

// Get URL
// Format: url
// 
#define VSCP_DAEMON_ACTION_CODE_GET_URL						0x00000075  // Get URL


#endif
