// vscp_base_client.h.
//
// Virtual base class for client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   (C) 2007-2020
// Ake Hedman, Grodans Paradis AB, <akhe@vscp.org>
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

#if !defined(VSCPCLIENTBASE_H__INCLUDED_)
#define VSCPCLIENTBASE_H__INCLUDED_

#include "vscp.h"    

#include <deque>
#include <list>
#include <string>

#ifdef WIN32
typedef void ( __stdcall * LPFNDLL_EV_CALLBACK) ( vscpEvent &ev );
typedef void ( __stdcall * LPFNDLL_EX_CALLBACK) ( vscpEventEx &ex );
#else
typedef void ( *LPFNDLL_EV_CALLBACK ) ( vscpEvent *pev );     // Event callback
typedef void ( *LPFNDLL_EX_CALLBACK ) ( vscpEventEx *pex );   // Event ex callback
#endif

class CVscpClient
{

public:
    CVscpClient();
    ~CVscpClient();

    /*!
        Connect to remote host
        @param bPoll If true polling is used.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int connect(void) = 0;

    /*!
        Disconnect from remote host
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int disconnect(void) = 0;

    /*!
        Check if connected.
        @return true if connected, false otherwise.
    */
    virtual bool isConnected(void) = 0;

    /*!
        Send VSCP event to remote host.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int send(vscpEvent &ev) = 0;

    /*!
        Send VSCP event to remote host.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int send(vscpEventEx &ex) = 0;

    /*!
        Receive VSCP event from remote host
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int receive(vscpEvent &ev) = 0;

    /*!
        Receive VSCP event ex from remote host
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int receive(vscpEventEx &ex) = 0;

    /*!
        Set interface filter
        @param filter VSCP Filter to set.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int setfilter(vscpEventFilter &filter) = 0;
    

    /*!
        Get number of events waiting to be received on remote
        interface
        @param pcount Pointer to an unsigned integer that get the count of events.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int getcount(uint16_t *pcount) = 0;

    /*!
        Clear the input queue
    */
    virtual int clear(void) = 0;

    /*!
        Get version from interface
        @param pmajor Pointer to uint8_t that get major version of interface.
        @param pminor Pointer to uint8_t that get minor version of interface.
        @param prelease Pointer to uint8_t that get release version of interface.
        @param pbuild Pointer to uint8_t that get build version of interface.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int getversion(uint8_t *pmajor,
                            uint8_t *pminor,
                            uint8_t *prelease,
                            uint8_t *pbuild) = 0;

    /*!
        Get interfaces
        @param iflist Get a list of available interfaces
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int getinterfaces(std::deque<std::string> &iflist) = 0;

    /*!
        Get capabilities (wcyd) from remote interface
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int getwcyd(uint64_t &wcyd) = 0;

    /*!
        Set (and enable) receive callback for events
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int setCallback(LPFNDLL_EV_CALLBACK evcallback);

    /*!
        Set (and enable) receive callback ex events
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int setCallback(LPFNDLL_EX_CALLBACK m_excallback);

    /*!
        Getter/setters for connection timeout
        Time is in milliseconds
    */
    virtual void setConnectionTimeout(uint32_t timeout) = 0;
    virtual uint32_t getConnectionTimeout(void) = 0;

    /*!
        Getter/setters for response timeout
        Time is in milliseconds
    */
    virtual void setResponseTimeout(uint32_t timeout) = 0;
    virtual uint32_t getResponseTimeout(void) = 0;

    /*!
        Check if ev callback is defined
        @return true if callback is defined
    */
    bool isEvCallback(void) {return (nullptr != m_evcallback); }

    /*!
        Check if ex callback is defined
        @return true if callback is defined
    */
    bool isExCallback(void) {return (nullptr != m_excallback); }

 public:

    /*!
        Callback for events
    */
    LPFNDLL_EV_CALLBACK m_evcallback;

    /*!
        Callback for ex events
    */
    LPFNDLL_EX_CALLBACK m_excallback;
};

#endif
