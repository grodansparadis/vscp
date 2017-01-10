// vscpcoapserver.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 
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


#if !defined(VSCPCOAPSEVER_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define VSCPCOAPSEVER_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_


#include "wx/thread.h"
#include "wx/socket.h"

#include "userlist.h"
#include "controlobject.h"


typedef const char * ( * COMMAND_METHOD) (  void );


/*!
    This class implement the VSCP MQTT broker
*/

class VSCPCoAPServerThread : public wxThread
{

public:
    
    /// Constructor
    VSCPCoAPServerThread();

    /// Destructor
    ~VSCPCoAPServerThread();

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

};


#endif




