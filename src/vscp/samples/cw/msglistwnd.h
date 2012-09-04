// MsgListWnd.cpp
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
// $RCSfile: msglistwnd.h,v $                                       
// $Date: 2005/04/26 07:13:08 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

#include "wx/grid.h"
#include "wx/tglbtn.h"
#include "../../common/canalsuperwrapper.h"

class CanalChild: public wxMDIChildFrame
{
 public:
   
  /// Constructor
  CanalChild( wxMDIParentFrame *parent, 
								const wxString& title, 
								const wxPoint& pos, 
								const wxSize& size, 
								const long style,
								 devItem *pdev );

  /// Destructor
  ~CanalChild();

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
  void insertItem( canalMsg *pMsg, bool bReceive = true );

  /*!
    Send Message
  */
  void OnSendMsg();

  /*!
    Fetch CAN data from the CAN command area
  */
  bool getCanData( canalMsg *pMsg );

  /*!
    Write a CAN message into the CAN command area
  */
  bool writeDataToDialog( canalMsg *pMsg );



  /*!
    Sizer for the list window
  */
  wxBoxSizer *m_ptopSizer;

  /*!
    Timer event 
  */
  wxTimer m_timer;

  /*!
    The wrapper for the CANAL 
    functionality.
  */
  CCanalSuperWrapper *m_pcsw;

  /*!
    Protect access to interface
  */
  bool m_bConnected;
	
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
	

  // * * * * * * * *  Command Window Controls * * * * * * * * 
	
  /*!
    Combo to select view mode
  */
  wxComboBox *m_pViewSelectCombo;

  /*!
    Radiobox for standard message id
  */
  wxRadioButton *m_poptionStandardMsgId;

  /*!
    Radiobox for extended message id
  */
  wxRadioButton *m_poptionExtendedMsgId;

  /*!
    Message ID field
  */
  wxTextCtrl *m_pMsgId;

  /*!
    Message Data fields	
  */
  wxTextCtrl *m_pMsgData[ 8 ]; 

  /*!
    Burst count
  */
  wxTextCtrl *m_pBurstCnt;

  /*!
    Remote Frame
  */
  wxCheckBox *m_pRemoteFrame;

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
    Send message burts
  */
  wxButton *m_pSendBurst;

  /*!
    Activate Log
  */
  wxToggleButton *m_plogActivate;

  /*!
    Messsage ID
  */
  wxString m_wxStrMsgId;

  /*!
    Message Data
  */
  wxString m_wxStrMsg[ 8 ];

  /*!
    Burts Count
  */
  wxString m_wxStrBurstCnt;

  DECLARE_EVENT_TABLE()
    };


    // Object ID's
    enum
      {
	CtrlMsgGrid = 300,
	CtrlCmdWnd,

	// Command controls
	CtrlViewSelectCombo,
	CtrlOptionStandardMsgId,
	CtrlOptionExtendedMsgId,
	CtrlEditMsgId,
	CtrlEditBurstCnt,
	CtrlCheckRemoteFrame,
	
	CtrlButtonPrevMsg,
	CtrlButtonNextMsg,
	CtrlButtonLoadMsg,
	CtrlButtonSaveMsg,
	CtrlButtonClearList,
	CtrlButtonSendMsg,
	CtrlButtonSendMsgBurst,
	CtrlButtonActivateLog,

	RefreshTimer,

	CtrlData = 400
      };

