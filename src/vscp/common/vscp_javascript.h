// vscp_javascript.h
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

#if !defined(VSCP_JAVASCRIPT__INCLUDED_)
#define VSCP_JAVASCRIPT__INCLUDED_


////////////////////////////////////////////////////////////////////////////////
// actionThread_JavaScript
//

class actionThread_JavaScript : public wxThread {
    
public:

    /// Constructor
    actionThread_JavaScript( wxString& strScript,
                                wxThreadKind kind = wxTHREAD_DETACHED );

    /// Destructor
    virtual ~actionThread_JavaScript();

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
        Termination control
     */
    bool m_bQuit;
    
    /*!
     * Script
     */
    wxString m_wxstrScript;
    
    /// JavaScript executing id
    uint64_t m_id;
    
    /// Time when script was started
    wxDateTime m_start;
    
    /// Time when script was stopped
    wxDateTime m_stop;
    
    /// Client item for script
    CClientItem *m_pClientItem;
    
    /// Feed event
    vscpEventEx m_feedEvent;
    
};



#endif