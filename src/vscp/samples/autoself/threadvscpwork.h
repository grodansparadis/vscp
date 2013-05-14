/////////////////////////////////////////////////////////////////////////////
// Name:        threadvscpwork.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 31 May 2007 17:00:07 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence: 
// 
/////////////////////////////////////////////////////////////////////////////

#ifndef _THREADVSCPWORK_H_
#define _THREADVSCPWORK_H_

#include "wx/wx.h"
#include "wx/thread.h"
#include "wx/socket.h"

#include "frmmain.h"
#include "frmNode.h"
#include "../../common/canaltcpif.h"

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
    Send a message to the log
    @param str message to log
  */
  void logmsg( wxString str );

	/*!
		Send the Status Update event to refresh the screen
		conent	
	*/
	void SendStatusUpdateEvent( void );

// --- Member variables ---

  /// Index into global node structure
  int m_FrameId;
  
  /// Textual representation of frame
  wxString m_strFrameName;
  

	/*!
		Pointer to the frame the worker thread do work for
	*/
	frmNode *m_pfrmNode;
  
  /*!
		Pointer to the main frame 
	*/
  frmMain * m_pfrmMain;

	/// Server TCP/IP interface
	CanalTcpIf m_srvif;	
	
  static const unsigned int SUBZONE_MODULE = 0;
  static const unsigned int SUBZONE_S1 = 1;
  static const unsigned int SUBZONE_S2 = 2;
  static const unsigned int SUBZONE_SB = 3;
  static const unsigned int SUBZONE_BLOCK = 4;
		
};


#endif

