// duktape_vscp_wrkthread.h
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