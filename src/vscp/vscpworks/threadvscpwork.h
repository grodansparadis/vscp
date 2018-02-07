/////////////////////////////////////////////////////////////////////////////
// Name:        threadvscpwork.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 31 May 2007 17:00:07 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence: 
// 
/////////////////////////////////////////////////////////////////////////////

#ifndef _THREADVSCPWORK_H_
#define _THREADVSCPWORK_H_

#include "wx/wx.h"
#include "wx/thread.h"
#include "wx/socket.h"

#include <vscpremotetcpif.h>
#include "frmvscpsession.h"

/*!
    This class implement the Client thread against
    the VSCP daemon TCP/IP interface
*/

class VscpWorkThread : public wxThread 
{

public:
    
    /// Constructor
    VscpWorkThread( wxThreadKind kind = wxTHREAD_DETACHED );

    /// Destructor
    ~VscpWorkThread();

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
        Send the Status Update event to refresh the screen
        conent	
    */
    void SendStatusUpdateEvent( void );

// --- Member variables ---

    /*!
        Pointer to the frame the worker thread do work for
    */
    frmVSCPSession *m_pfrmNode;

    /// Server TCP/IP interface
    VscpRemoteTcpIf m_vscptcpif;		
        
};


#endif

