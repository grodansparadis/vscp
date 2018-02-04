// devicethread.h
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

#if !defined(DEVICETHREAD_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define DEVICETHREAD_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#endif

#ifdef WIN32
#include <winsock2.h>
#endif

#include "wx/wx.h"
#include <wx/thread.h>
#include <wx/socket.h>
#include <wx/dynlib.h>

class CControlObject;
class CDeviceItem;
class deviceThread;

/*!
    This class implement a thread that write data
    to a blocking CANAL driver
*/

class deviceLevel1WriteThread : public wxThread
{

public:
    
    /// Constructor
    deviceLevel1WriteThread();

    /// Destructor
    virtual ~deviceLevel1WriteThread();

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
        Pointer to master thread.
    */
    deviceThread *m_pMainThreadObj;

  /*!
        Termination control
  */
  bool m_bQuit;


};


/*!
    This class implement a thread that read data
    from a blocking CANAL driver
*/

class deviceLevel1ReceiveThread : public wxThread
{

public:

    /// Constructor
    deviceLevel1ReceiveThread();

    /// Destructor
    virtual ~deviceLevel1ReceiveThread();

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
        Pointer to master thread.
    */
    deviceThread *m_pMainThreadObj;

  /*!
        Termination control
  */
  bool m_bQuit;

};


/*!
    This class implement a thread that write data
    to a blocking VSCP Level II driver
*/

class deviceLevel2WriteThread : public wxThread
{

public:

    /// Constructor
    deviceLevel2WriteThread();

    /// Destructor
    virtual ~deviceLevel2WriteThread();

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
        Pointer to master thread.
    */
    deviceThread *m_pMainThreadObj;

  /*!
    Termination control
  */
  bool m_bQuit;


};


/*!
    This class implement a thread that read data
    from a blocking VSCP Level II driver
*/

class deviceLevel2ReceiveThread : public wxThread
{

public:
    
    /// Constructor
    deviceLevel2ReceiveThread();

    /// Destructor
    virtual ~deviceLevel2ReceiveThread();

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
        Pointer to master thread.
    */
    deviceThread *m_pMainThreadObj;

  /*!
        Termination control
  */
  bool m_bQuit;

};


/*!
    This class implement a one of thread that look
    for specific events and react on them appropriately .
*/

class deviceThread : public wxThread
{

public:

    /// Constructor
    deviceThread();

    /// Destructor
    virtual ~deviceThread();

    /*!
        Thread code entry point
    */
    virtual void *Entry();


    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();


    /// dl/dll handler
    wxDynamicLibrary m_wxdll;

    /*!
        DeviceItem for device to control	
    */
    CDeviceItem *m_pDeviceItem;

    /*!
        Control object that invoked thread
    */
    CControlObject *m_pCtrlObject;

    /*!
        Holder for CANAL receive thread
    */
    deviceLevel1ReceiveThread *m_preceiveThread;

    /*!
        Holder for CANAL write thread
    */
    deviceLevel1WriteThread *m_pwriteThread;
    
    
    /*!
        Holder for VSCP Level II receive thread
    */
    deviceLevel2ReceiveThread *m_preceiveLevel2Thread;

    /*!
        Holder for VSCP Level II write thread
    */
    deviceLevel2WriteThread *m_pwriteLevel2Thread;

    /*!
        Check filter/mask to see if filter should be delivered

        The filter have the following matrix

        mask bit n	|	filter bit n	| msg id bit	|	result
        ===========================================================
            0               X                   X           Accept
            1               0                   0           Accept
            1               0                   1           Reject
            1               1                   0           Reject
            1               1                   1           Accept

            Formula is !( ( filter �d ) & mask )

        @param pclientItem Pointer to client item
        @param pcanalMsg Pointer to can message
        @return True if message is accepted false if rejected
        TODO
    */
        
};




#endif



