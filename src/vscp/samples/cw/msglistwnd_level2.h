// msglistwnd_level2.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (http://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: msglistwnd_level2.h,v $                                       
// $Date: 2005/05/13 17:01:46 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#include "wx/grid.h"
#include "wx/tglbtn.h"

#include "../../common/devitem.h"
#include "../../common/vscptcpif.h"
#include "../../common/canalsuperwrapper.h"

class Level2Child: public wxMDIChildFrame
{
 public:
   
  /// Constructor
  Level2Child( wxMDIParentFrame *parent, 
								const wxString& title, 
								const wxPoint& pos, 
								const wxSize& size, 
								const long style,
								devItem *pdev );

  /// Destructor
  ~Level2Child();

  void OnActivate( wxActivateEvent& event );

  /*!
    Activate deactivate send tools area
  */
  void OnViewSendTools( wxCommandEvent& event );


  void OnUpdateRefresh( wxUpdateUIEvent& event );
  void OnChangeTitle( wxCommandEvent& event );
  void OnChangePosition( wxCommandEvent& event );
  void OnChangeSize( wxCommandEvent& event );
  void OnQuit( wxCommandEvent& event );
  void OnSize( wxSizeEvent& event );
  void OnMove( wxMoveEvent& event );
  void OnClose( wxCloseEvent& event );

  /*!
    Key Event
  */
  void OnKey( wxKeyEvent& event );

  /*!
    Handle Clear List Button
  */
  void OnButtonClearList( wxCommandEvent& event );

  /*!
    Handle Send Message Button
  */
  void OnButtonSendMsg( wxCommandEvent& event );

  /*!
    Handle Send Message Burst Button
  */
  void OnButtonsendMsgBurst( wxCommandEvent& event );

  /*!
    Handle Activate Log Button
  */
  void OnButtonActivateInterface( wxCommandEvent& event );

  /*!
    Handle Activate Menu item
  */
  void OnButtonMenuActivateInterface( wxCommandEvent& event );

  /*!
    Handle Previous Message Button
  */
  void OnButtonPrevMsg( wxCommandEvent& event );

  /*!
    Handle Next Message Button
  */
  void OnButtonNextMsg( wxCommandEvent& event );

  /*!
    Handle Load Message Button
  */
  void OnButtonLoadMsg( wxCommandEvent& event );

  /*!
    Handle Save Message Button
  */
  void OnButtonSaveMsg( wxCommandEvent& event );

  /*!
    Handle Timer events
  */
  void OnTimerRefresh( wxTimerEvent& event );

  /*!
    Fill Grid with data from the i/f
  */
  void fillData( void );

  /*!
    Insert item into grid
  */
  void insertItem( vscpEvent *pEvent, bool bReceive = true );

  /*!
    Send Message
  */
  void OnSendMsg();

  /*!
    Fetch VSCP data from the VSCP command area
  */
  bool getVSCPData( vscpEvent *pEvent );

  /*!
    Write a CAN message into the CAN command area
  */
  bool writeDataToDialog( vscpEvent *pEvent );

  bool m_bConnected;

  /*!
    Sizer for the list window
  */
  wxBoxSizer *m_ptopSizer;

  /*!
    Timer event 
  */
  wxTimer m_timer;


  /*!
    VSCP Level II interface
  */
  CCanalSuperWrapper *m_pcsw;
	
  /*!
    Grid resource
  */
  wxGrid *m_pgrid;

  /*!
    Font for grid elements
  */
  wxFont *m_pgridFont;

  /*!
    CAN Command Window
  */
  wxWindow *m_pCmdWnd;

	/*!
    GUID data
  */
  wxString m_wxStrGUID[ 16 ];
	

  // * * * * * * * *  Command Window Controls * * * * * * * * 
	
 

  /*!
    Message Class field
  */
  wxTextCtrl *m_pMsgClass;

	/*!
    Message Type field
  */
  wxTextCtrl *m_pMsgType;

	/*!
    Message Data field
  */
  wxTextCtrl *m_pMsgData;

	/*!
    Spincontrol for Priority
  */
	wxSpinCtrl *m_pSpinPriority;

  /*!
    GUID fields	
  */
  wxTextCtrl *m_pEditGUID[ 16 ];

  /*!
    Previous sent message in saved list
  */
  wxButton *m_pPrevMsg;

  /*!
    Next sent message in saved list
  */
  wxButton *m_pNextMsg;

  /*!
    Load a Msg saved list
  */
  wxButton *m_pLoadMsg;

  /*!
    Save a Msg saved list
  */
  wxButton *m_pSaveMsg;

  /*!
    Clear the message list
  */
  wxButton *m_pClrList;

  /*!
    Send message
  */
  wxButton *m_pSend;


  /*!
    Activate Log
  */
  wxToggleButton *m_plogActivate;

  /*!
    Messsage Class
  */
  wxString m_wxStrMsgClass;

	  /*!
    Messsage Type
  */
  wxString m_wxStrMsgType;


  /*!
    Burst Count
  */
  wxString m_wxStrBurstCnt;

  DECLARE_EVENT_TABLE()

};


    // Object ID's
    enum {
			L2CtrlMsgGrid = 300,
			L2CtrlCmdWnd,

			// Command controls
			L2CtrlEditMsgClass,
			L2CtrlEditMsgType,

			L2CtrlEditMsgData,
			L2CtrlSpinPriority,
	
			L2CtrlButtonPrevMsg,
			L2CtrlButtonNextMsg,
			L2CtrlButtonLoadMsg,
			L2CtrlButtonSaveMsg,

			L2CtrlButtonClearList,
			L2CtrlButtonSendMsg,

			L2CtrlButtonActivateLog,

			L2RefreshTimer,

			
			L2CtrlData = 400,
			L2EditGUID = 600
	};

