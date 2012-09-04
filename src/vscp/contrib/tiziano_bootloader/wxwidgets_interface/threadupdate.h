/////////////////////////////////////////////////////////////////////////////
// Name:        threadupdate.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 30 Jun 2007 14:08:14 CEST
// RCS-ID:      
// Copyright:   (C) 2007 Ake Hedman, D Of Scandinavia, <ake@dofscandinavia.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://can.sourceforge.net) 
//
// Copyright (C) 2000-2008 Ake Hedman, D Of Scandinavia, <ake@dofscandinavia.com>
// 
// This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
// 
//  As a special exception, if other files instantiate templates or use macros
//  or inline functions from this file, or you compile this file and link it
//  with other works to produce a work based on this file, this file does not
//  by itself cause the resulting work to be covered by the GNU General Public
//  License. However the source code for this file must still be made available
//  in accordance with section (3) of the GNU General Public License.
// 
//  This exception does not invalidate any other reasons why a work based on
//  this file might be covered by the GNU General Public License.
// 
//  Alternative licenses for VSCP & Friends may be arranged by contacting 
//  D of Scandinavia at info@dofscandinavia.com, http://www.dofscandinavia.com
/////////////////////////////////////////////////////////////////////////////

#ifndef _THREADUPDATE_H_
#define _THREADUPDATE_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "threadupdate.h"
#endif

#include <wx/list.h>
#include <wx/ffile.h>
#include <wx/dynlib.h>

#include "ctrlobj.h"

#include "../../common/canaldlldef.h"
#include "../../common/canalsuperwrapper.h"
#include "thread_symbols.h"

class ThreadUpdate;

/*!
	This class implement a thread that handles
	transmit events
*/

class UPDATEWorkerThread : public wxThread
{

public:
	
	/// Constructor
	UPDATEWorkerThread();

	/// Destructor
	virtual ~UPDATEWorkerThread();

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
		Pointer to control object.
	*/
	ctrlObj *m_pCtrlObject;

	ThreadUpdate* m_pCtrl;

};

/////////////////////////////////////////////////////////////////////


class ThreadUpdate: public wxEvtHandler
{    
  	DECLARE_CLASS( ThreadUpdate )

public:
  	/// Constructors
  	ThreadUpdate();
  	
  	/// Destructor
  	~ThreadUpdate();

  	/// Initialises member variables
  	void Init();

	/// Exit function
 	void Close();

  	/// Start the worker threads
  	int startWorkerThreads( );

  	/// Stop the worker threads
  	void stopWorkerThreads( void );
  
  	/// TRUE as long as worker thread should run
  	bool m_bRun;
  
  	/// Common control object
  	ctrlObj* m_pCtrlObject;

  	/// Mutex that control the stopThreads method
  	wxMutex m_mutexStopThread;
  
  	// * * *   Threads  * * *
  
  	/// Worker thread for control and transmit
  	UPDATEWorkerThread * m_pWorkerThread;
  
	enum {
		ID_THREADUPDATE = wxID_ANY
	};

};

#endif
