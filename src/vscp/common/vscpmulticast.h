// File: vscpmulticast.h
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

#ifndef VSCP_MULTICAST_H
#define VSCP_MULTICAST_H

#pragma once

#include <knownnodes.h>

// Part of VSCP Works

//WX_DECLARE_LIST( MyListElement, listEvents );

class worksMulticastThread : public wxThread
{

public:

    /// Constructor
    worksMulticastThread();

    /// Destructor
    ~worksMulticastThread();

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

    // This is the known node information container
    // It is protected by the member m_mutexKnownNodes
    // CKnownNodes m_knownNodes;  TODO

};

#endif