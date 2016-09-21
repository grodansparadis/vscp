// udpclientthread.h
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


#if !defined(UDPCLIENTTHREAD_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define UDPCLIENTTHREAD_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_


#include "wx/thread.h"
#include "wx/socket.h"

#include "userlist.h"
#include "controlobject.h"

#define VSCP_TCP_MAX_CLIENTS                1024
#define MSG_OK                              "+OK - Success.\r\n"
#define MSG_GOODBY                          "+OK - Connection closed by client.\r\n"
#define MSG_GOODBY2                         "+OK - Connection closed.\r\n"
#define MSG_USENAME_OK                      "+OK - User name accepted, password please\r\n"
#define MSG_PASSWORD_OK                     "+OK - Ready to work.\r\n"
#define MSG_QUEUE_CLEARED                   "+OK - All events cleared.\r\n"
#define MSG_RECEIVE_LOOP                    "+OK - Receive loop entered. QUITLOOP to terminate.\r\n"
#define MSG_QUIT_LOOP                       "+OK - Quit receive loop.\r\n"
#define MSG_CAN_MODE                        "+OK - CAN mode set.\r\n"

#define MSG_ERROR                           "-OK - Error\r\n"
#define MSG_UNKNOWN_COMMAND                 "-OK - Unknown command\r\n"
#define MSG_PARAMETER_ERROR                 "-OK - Invalid parameter or format\r\n"
#define MSG_BUFFER_FULL                     "-OK - Buffer Full\r\n"
#define MSG_NO_MSG                          "-OK - No event(s) available\r\n"

#define MSG_PASSWORD_ERROR                  "-OK - Invalid username or password.\r\n"
#define MSG_NOT_ACCREDITED                  "-OK - Need to log in to perform this command.\r\n"
#define MSG_INVALID_USER                    "-OK - Invalid user.\r\n"
#define MSG_NEED_USERNAME                   "-OK - Need a Username before a password can be entered.\r\n"

#define MSG_MAX_NUMBER_OF_CLIENTS           "-OK - Max number of clients connected.\r\n"
#define MSG_INTERNAL_ERROR                  "-OK - Server Internal error.\r\n"
#define MSG_INTERNAL_MEMORY_ERROR           "-OK - Internal Memory error.\r\n"
#define MSG_INVALID_REMOTE_ERROR            "-OK - Invalid or unknown peer.\r\n"

#define MSG_LOW_PRIVILEGE_ERROR             "-OK - User need higher privilege level to perform this operation.\r\n"
#define MSG_INTERFACE_NOT_FOUND             "-OK - Interface not found.\r\n"

#define MSG_VARIABLE_NOT_DEFINED            "-OK - Variable is not defined.\r\n"
#define MSG_MOT_ALLOWED_TO_SEND_EVENT       "-OK - Not allowed to sen this event (contact admin).\r\n"
#define MSG_INVALID_PATH                    "-OK - Invalid path.\r\n"


//class CControlObject;
//class CClientItem;

typedef const char * ( * COMMAND_METHOD) (  void );

/*!
    Class that defines one command
*/
typedef struct {
    wxString m_strCmd;                  // Command name
    uint8_t m_securityLevel;            // Security level for command (0-15)
    wxProcess *m_pfnCommand;            // Function to execute
} structUDPCommand;



/*!
    This class implement the listen thread for
    the vscpd connections on the UDP interface
*/

class VSCPUDPClientThread : public wxThread
{

public:
    
    /// Constructor
    VSCPUDPClientThread();

    /// Destructor
    ~VSCPUDPClientThread();

    /*!
        Thread code entry point
    */
    virtual void *Entry();

    /*!
        TCP/IP handler
    */
    static void ev_handler( struct mg_connection *nc, int ev, void *p );

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();


// --- Member variables ---

    /*!
        Termination control
    */
    bool m_bQuit;

    /*!
        Pointer to ower owner
    */
    CControlObject *m_pCtrlObject;

    /*!
        UDP Client
    */
    CClientItem *m_pClientItem;

    /// Last connand executed
    wxString m_lastCommand;

    // Current command
    wxString m_currentCommand;

    /// Current command i all upper case
    wxString m_currentCommandUC;

};




/*!
    This class implement a worker thread that
    logs UDP received data.
*/

class UDPWorkerThread : public wxThread
{

public:
    
    /// Constructor
    UDPWorkerThread();

    /// Destructor
    ~UDPWorkerThread();

    /*!
        Thread code entry point
    */
    virtual void *Entry();

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();


// --- Member variables ---

    /*!
        Termination control
    */
    bool m_bQuit;

    

};




#endif




