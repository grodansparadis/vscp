// vscpmulticast_channel.h
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


#if !defined(VSCPMULTICAST_CHANNEL_H__INCLUDED_)
#define VSCPMULTICAST_CHANNEL_H__INCLUDED_


class multicastChannelVSCPThread : public wxThread
{

public:
    
    /// Constructor
    multicastChannelVSCPThread();

    /// Destructor
    ~multicastChannelVSCPThread();

    /*!
        Thread code entry point
    */
    virtual void *Entry();


    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();


    /*!
        Send multicast event
        @param sock Multicast socket to send on
        @param pEvent Event to send
        @param port Port to send multicast frame on.
        @return true on success, false on failure
    */
    bool sendMulticastEvent( int sock,
                                vscpEvent *pEvent,
                                int port );

    /*!
        Send multicast event Ex
        @param sock Multicast socket to send on
        @param pEventEx Event ex to send
        @param port Port to send multicast frame on.
        @return true on success, false on failure
    */
    bool sendMulticastEventEx( int sock,
                                vscpEventEx *pEventEx,
                                int port );

    /*!
        Termination control
    */
    bool m_bQuit;

};




#endif