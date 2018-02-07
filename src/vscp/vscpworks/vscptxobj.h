/////////////////////////////////////////////////////////////////////////////
// Name:        vscptxobj.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 10:05:16 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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
//  Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
// 

#ifndef _VSCPTXOBJ_H_
#define _VSCPTXOBJ_H_

#include "wx/wx.h"
#include <vscp.h>

class periodicSenderThread;


/*!
  Transmission object
*/

class VscpTXObj
{
  public:
  
    /*!
      Constructor
    */
    VscpTXObj();
    
    /*!
      Destructor
    */
    ~VscpTXObj();
  
  /*!
    Flag to activate transmission object
  */
  bool m_bActive;
  
  /*!
    Name for transmission object
  */
  wxString m_wxStrName;
  
  /*!
    Transmission object VSCP event
  */
  vscpEvent m_Event;
  
  /*!
    Flag if standard id should be used.
  */
  bool m_bUseDefaultGUID;
  
  /*!
    Number of events that should be send when 
    triggered.
  */
  unsigned long m_count;
  
  /*!
    Period in milliseconds between automatic 
    treansmission of events of the transmission
    object.
  */
  unsigned long m_period;
  
  /*!
    Trigger that cause a event transmission 
  */
  unsigned long m_trigger;
  
  /*!
    Worker thread for periodic send events
  */
  periodicSenderThread *m_pworkerThread;
  
};

/*!
	This class implement a thread that handles
	periodic transmit events
*/

class periodicSenderThread : public wxThread
{

public:
	
	/// Constructor
	periodicSenderThread();

	/// Destructor
	virtual ~periodicSenderThread();

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
    Shared control object
  */
  VscpTXObj m_CtrlObj;

};



#endif
