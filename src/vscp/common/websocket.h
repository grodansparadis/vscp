// websocket.h: 
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

#if !defined(WEBSOCKET_H__INCLUDED_)
#define WEBSOCKET_H__INCLUDED_

#include <vscp.h>
//#include <controlobject.h>

//******************************************************************************
//                                WEBSOCKETS
//******************************************************************************

// websocket types
#define WEBSOCKET_SUBYPE_STANDARD   "vscp-std"      // Original form
#define WEBSOCKET_SUBTYPE_JSON      "vscp-json"     // JSON format

#define MAX_VSCPWS_MESSAGE_QUEUE    (512)

// This is the time it takes for an expired websocket session to be 
// removed by the system.
#define WEBSOCKET_EXPIRE_TIME       (2*60)

// Authentication states
enum {
    WEBSOCK_CONN_STATE_NULL = 0,
    WEBSOCK_CONN_STATE_CONNECTED,
    WEBSOCK_CONN_STATE_DATA
};

enum {
    WEBSOCK_ERROR_NO_ERROR,                     // Everything is OK.
    WEBSOCK_ERROR_SYNTAX_ERROR,                 // Syntax error.
    WEBSOCK_ERROR_UNKNOWN_COMMAND,              // Unknown command.
    WEBSOCK_ERROR_TX_BUFFER_FULL,               // Transmit buffer full.
    WEBSOCK_ERROR_MEMORY_ALLOCATION,            // Problem allocating memory.
    WEBSOCK_ERROR_VARIABLE_DEFINED,             // Variable is already defined.
    WEBSOCK_ERROR_VARIABLE_UNKNOWN,             // Cant find variable
    WEBSOCK_ERROR_VARIABLE_NO_STOCK,            // Cant add stock variable
    WEBSOCK_ERROR_NOT_AUTHORISED,               // Not authorised
    WEBSOCK_ERROR_NOT_ALLOWED_TO_SEND_EVENT,    // Not authorized to send events
    WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,       // Not allowed to do that
    WEBSOCK_ERROR_MUST_HAVE_TABLE_NAME,         // Must have a table name
    WEBSOCK_ERROR_END_DATE_IS_WRONG,            // End date must be later than start date
    WEBSOCK_ERROR_INVALID_DATE,                 // Invalid date
    WEBSOCK_ERROR_TABLE_NOT_FOUND,              // Table not found
    WEBSOCK_ERROR_TABLE_NO_DATA,                // No data in table
    WEBSOCK_ERROR_TABLE_ERROR_READING,          // Error reading table
    WEBSOCK_ERROR_TABLE_CREATE_FORMAT,          // Table create formats was wrong
    WEBSOCK_ERROR_TABLE_DELETE_FAILED,          // Table delete failed
    WEBSOCK_ERROR_TABLE_LIST_FAILED,            // Table list failed        
    WEBSOCK_ERROR_TABLE_FAILED_TO_GET,          // Failed to get table
    WEBSOCK_ERROR_TABLE_FAILED_GET_DATA,        // Failed to get table data
    WEBSOCK_ERROR_TABLE_FAILED_CLEAR,           // Failed to clear table
    WEBSOCK_ERROR_TABLE_LOG_MISSING_VALUE,      // Missing value 
    WEBSOCK_ERROR_TABLE_LOG_FAILED,             // Failed to log data
    WEBSOCK_ERROR_TABLE_NEED_SQL,               // Missing SQL expression   
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_RECORDS,  // Failed to get # records 
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_FIRSTDATE,// Failed to get first date  
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_LASTDATE, // Failed to get last date
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_SUM,      // Failed to get sum
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MIN,      // Failed to get min
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MAX,      // Failed to get max
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_AVERAGE,  // Failed to get average
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MEDIAN,   // Failed to get median
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_STDDEV,   // Failed to get stddev
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_VARIANCE, // Failed to get variance
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MODE,     // Failed to get mode
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_UPPERQ,   // Failed to get upperq
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_LOWERQ,   // Failed to get lowerq
    WEBSOCK_ERROR_TABLE_FAILED_COMMAND_CLEAR     // Failed to clear entries
};

#define	WEBSOCK_STR_ERROR_NO_ERROR                      "Everything is OK"
#define WEBSOCK_STR_ERROR_SYNTAX_ERROR                  "Syntax error"
#define WEBSOCK_STR_ERROR_UNKNOWN_COMMAND               "Unknown command"
#define WEBSOCK_STR_ERROR_TX_BUFFER_FULL                "Transmit buffer full"
#define WEBSOCK_STR_ERROR_MEMORY_ALLOCATION             "Having problems to allocate memory"
#define WEBSOCK_STR_ERROR_VARIABLE_DEFINED              "Variable is already defined"
#define WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN              "Unable to find variable"
#define WEBSOCK_STR_ERROR_VARIABLE_UPDATE               "Unable to update variable"
#define WEBSOCK_STR_ERROR_VARIABLE_NO_STOCK             "Stock variables can't be added/created"
#define WEBSOCK_STR_ERROR_NOT_AUTHORISED                "Not authorised"
#define WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_SEND_EVENT     "Not allowed to send event"
#define WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT        "Not allowed to do that (check privileges)"
#define WEBSOCK_STR_ERROR_MUST_HAVE_TABLE_NAME          "A table name must be given as parameter"
#define WEBSOCK_STR_ERROR_END_DATE_IS_WRONG             "End date must be later than the start date"
#define WEBSOCK_STR_ERROR_INVALID_DATE                  "Invalid date"
#define WEBSOCK_STR_ERROR_TABLE_NOT_FOUND               "Table not found"
#define WEBSOCK_STR_ERROR_TABLE_NO_DATA                 "No data in table"
#define WEBSOCK_STR_ERROR_TABLE_ERROR_READING           "Error reading table"
#define WEBSOCK_STR_ERROR_TABLE_CREATE_FORMAT           "Table create format was wrong"
#define WEBSOCK_STR_ERROR_TABLE_DELETE_FAILED           "Table delete failed"
#define WEBSOCK_STR_ERROR_TABLE_LIST_FAILED             "Table list failed"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_TO_GET           "Failed to get table (is it available?)"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_GET_DATA         "Failed to get table data"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_CLEAR            "Failed to clear table"
#define WEBSOCK_STR_ERROR_TABLE_LOG_MISSING_VALUE       "A value is needed"
#define WEBSOCK_STR_ERROR_TABLE_LOG_FAILED              "Failed to log data"
#define WEBSOCK_STR_ERROR_TABLE_NEED_SQL                "Missing SQL expression"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_RECORDS   "Failed to get number of records"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_FIRSTDATE "Failed to get first date"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_LASTDATE  "Failed to get last date"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_SUM       "Failed to get sum"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MIN       "Failed to get min"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MAX       "Failed to get max"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_AVERAGE   "Failed to get average"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MEDIAN    "Failed to get median"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_STDDEV    "Failed to get stddev"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_VARIANCE  "Failed to get variance"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MODE      "Failed to get mode"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_UPPERQ    "Failed to get upperq"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_LOWERQ    "Failed to get lowerq"
#define WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_CLEAR     "Failed to clear table enteries"

WX_DECLARE_LIST(vscpEventFilter, TRIGGERLIST);

#define WEBSOCKET_MAINCODE_POSITIVE         "+"
#define WEBSOCKET_MAINCODE_NEGATIVE         "-"

#define WEBSOCKET_MAINCODE_COMMAND          "C"
#define WEBSOCKET_MAINCODE_EVENT            "E"
#define WEBSOCKET_MAINCODE_VARIABLE         "V"

#define WEBSOCKET_SUBCODE_VARIABLE_CHANGED  "C"
#define WEBSOCKET_SUBCODE_VARIABLE_CREATED  "N"
#define WEBSOCKET_SUBCODE_VARIABLE_DELETED  "D"

class websock_session {
    
public:    
       
    websock_session(void);
    ~websock_session(void);
    
    // Connection object
    struct web_connection *m_conn;    
    
    // Connection state (see enums above)
    int m_conn_state;  
        
    // Unique ID for this session. 
    char m_websocket_key[33];     // Sec-WebSocket-Key 

    // 16 byte iv (SID) for this session
    char m_sid[33];

    // Protocol version
    int m_version;      // Sec-WebSocket-Version

    // Time when this session was last active.
    time_t lastActiveTime;
    
    // Concatenated message receive
    wxString m_strConcatenated;
        
    // Client structure for websocket
    CClientItem *m_pClientItem;

    // Event triggers
    //      Used for acknowledge automatisation
    //      VSCP_TYPE_INFORMATION_ACTION_TRIGGER
    
    bool bEventTrigger;             // True to activate event trigger functionality.
    uint32_t triggerTimeout;        // Time out before trig (or error) must occur.
    TRIGGERLIST listTriggerOK;      // List with positive triggers.
    TRIGGERLIST listTriggerERR;     // List with negative triggers.
    
    // Variable triggers
    bool bVariableTrigger;          // True to activate event trigger functionality.
};

WX_DECLARE_LIST(websock_session, WEBSOCKETSESSIONLIST);

// Public functions 

void  websock_post_incomingEvents( void );

#endif