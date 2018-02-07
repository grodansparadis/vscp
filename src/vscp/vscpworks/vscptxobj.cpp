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

#include "vscptxobj.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
//

VscpTXObj::VscpTXObj()
{
  m_bActive = false;          // Inactive as default
  m_bUseDefaultGUID = true;   // Use standard GUID
  m_count = 1;                // One event sent when triggered
  m_period = 0;               // No automatic transmit
  m_trigger = 0;              // No trigger

  m_pworkerThread = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
//

VscpTXObj::~VscpTXObj()
{
  if ( NULL != m_pworkerThread ) {
    m_pworkerThread->m_bQuit = true;
    m_pworkerThread->Wait();
    delete m_pworkerThread;
  }
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// receiveWorkerThreadAmin
//

periodicSenderThread::periodicSenderThread()
                        : wxThread( wxTHREAD_JOINABLE )
{
  m_bQuit = false;
}

///////////////////////////////////////////////////////////////////////////////
// receiveWorkerThread
//

periodicSenderThread::~periodicSenderThread()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void *periodicSenderThread::Entry()
{
	while ( !TestDestroy() && m_bQuit )
	{
    wxSleep( 1 );
	} // while

	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void periodicSenderThread::OnExit()
{

}
