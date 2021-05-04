// websocket.h:
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project
// <akhe@vscp.org>
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

#if !defined(WEBSOCKET_H__INCLUDED_)
#define WEBSOCKET_H__INCLUDED_

#include <vscp.h>
#include <clientlist.h>

//******************************************************************************
//                                WEBSOCKETS
//******************************************************************************

// websocket types
#define WEBSOCKET_SUBYPE_STANDARD "vscp-std"  // Original form
#define WEBSOCKET_SUBTYPE_JSON    "vscp-json" // JSON format

#define MAX_VSCPWS_MESSAGE_QUEUE (512)

// This is the time it takes for an expired websocket session to be
// removed by the system.
#define WEBSOCKET_EXPIRE_TIME (2 * 60)

// Authentication states
enum
{
    WEBSOCK_CONN_STATE_NULL = 0,
    WEBSOCK_CONN_STATE_CONNECTED,
    WEBSOCK_CONN_STATE_DATA
};

enum
{
    WEBSOCK_ERROR_NO_ERROR          = 0,            // Everything is OK.
    WEBSOCK_ERROR_SYNTAX_ERROR      = 1,            // Syntax error.
    WEBSOCK_ERROR_UNKNOWN_COMMAND   = 2,            // Unknown command.
    WEBSOCK_ERROR_TX_BUFFER_FULL    = 3,            // Transmit buffer full.
    WEBSOCK_ERROR_MEMORY_ALLOCATION = 4,            // Problem allocating memory.
    WEBSOCK_ERROR_NOT_AUTHORISED    = 5,            // Not authorised-
    WEBSOCK_ERROR_NOT_ALLOWED_TO_SEND_EVENT = 6,    // Not authorized to send events.
    WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT = 7,       // Not allowed to do that.
    WEBSOCK_ERROR_PARSE_FORMAT            = 8,      // Parse error, invalid format.
    WEBSOCK_ERROR_UNKNOWN_TYPE = 9,                 // Unkown object type
    WEBSOCK_ERROR_GENERAL = 10,                     // General errors and exceptions
};

#define WEBSOCK_STR_ERROR_NO_ERROR        "Everything is OK."
#define WEBSOCK_STR_ERROR_SYNTAX_ERROR    "Syntax error."
#define WEBSOCK_STR_ERROR_UNKNOWN_COMMAND "Unknown command."
#define WEBSOCK_STR_ERROR_TX_BUFFER_FULL  "Transmit buffer full."
#define WEBSOCK_STR_ERROR_MEMORY_ALLOCATION                                    \
    "Having problems to allocate memory."
#define WEBSOCK_STR_ERROR_NOT_AUTHORISED            "Not authorised."
#define WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_SEND_EVENT "Not allowed to send event."
#define WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT                               \
    "Not allowed to do that (check privileges)"
#define WEBSOCK_STR_ERROR_PARSE_FORMAT "Parse error, invalid format."
#define WEBSOCK_STR_ERROR_UNKNOWN_TYPE "Unknown type, only know 'COMMAND' and 'EVENT'."
#define WEBSOCK_STR_ERROR_GENERAL "Exception or other general error."

#define WEBSOCKET_MAINCODE_POSITIVE "+"
#define WEBSOCKET_MAINCODE_NEGATIVE "-"

#define WEBSOCKET_MAINCODE_COMMAND  "C"
#define WEBSOCKET_MAINCODE_EVENT    "E"
#define WEBSOCKET_MAINCODE_VARIABLE "V"

#define WEBSOCKET_SUBCODE_VARIABLE_CHANGED "C"
#define WEBSOCKET_SUBCODE_VARIABLE_CREATED "N"
#define WEBSOCKET_SUBCODE_VARIABLE_DELETED "D"

#define WS_TYPE_1 1
#define WS_TYPE_2 2

class websock_session
{

  public:
    websock_session(void);
    ~websock_session(void);

    // ws type 1/2
    uint8_t m_wstypes;

    // Connection object
    struct mg_connection* m_conn;

    // Connection state (see enums above)
    int m_conn_state;

    // Unique ID for this session.
    char m_websocket_key[33]; // Sec-WebSocket-Key

    // 16 byte iv (SID) for this session
    char m_sid[33];

    // Protocol version
    int m_version; // Sec-WebSocket-Version

    // Time when this session was last active.
    time_t lastActiveTime;

    // Concatenated message receive
    std::string m_strConcatenated;

    // Client structure for websocket 
    CClientItem* m_pClientItem;
};

#define WS2_COMMAND                                                            \
    "{"                                                                        \
    " \"type\" : \"CMD\", "                                                    \
    " \"command\" : \"%s\", "                                                  \
    " \"args\" : %s"                                                           \
    "}"

#define WS2_EVENT                                                              \
    "{"                                                                        \
    " \"type\" : \"EVENT\", "                                                  \
    " \"event\" : "                                                            \
    " %s "                                                                     \
    "}"

#define WS2_POSITIVE_RESPONSE                                                  \
    "{"                                                                        \
    " \"type\" : \"+\", "                                                      \
    " \"command\" : \"%s\", "                                                  \
    " \"args\" : %s"                                                           \
    "}"

#define WS2_NEGATIVE_RESPONSE                                                  \
    "{"                                                                        \
    " \"type\" : \"-\", "                                                      \
    " \"command\" : \"%s\", "                                                  \
    " \"errcode\" : %d, "                                                   \
    " \"errstr\" : \"%s\" "                                                 \
    "}"

#define WS2_VARIABLE                                                           \
    "{"                                                                        \
    " \"type\" : \"VARIABLE\", "                                               \
    " \"variable\" : "                                                         \
    " %s "                                                                     \
    "}"

const int MSG_TYPE_COMMAND           = 0; // Built in command
const int MSG_TYPE_XCOMMAND          = 1; // Add on command
const int MSG_TYPE_EVENT             = 2; // Event
const int MSG_TYPE_RESPONSE_POSITIVE = 3; // Positive reply
const int MSG_TYPE_RESPONSE_NEGATIVE = 4; // Negative reply
const int MSG_TYPE_VARIABLE          = 5; // Changed variable

class w2msg
{
  public:
    w2msg(void);
    ~w2msg(void);

    /*
        Event (E)/Command (C)/Response (+)/Variable (V)
    */
    int m_type;

    /*
        Command/Response/Variable arguments
    */
    std::map<std::string, std::string> m_arguments;

    /*
        Holder for Event
    */
    vscpEventEx m_ex;
};

// Public functions

void
websock_post_incomingEvents(void);

#endif