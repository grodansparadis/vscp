// VSCPConfigWnd.h
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
// $RCSfile: vscpconfigwnd.h,v $                                       
// $Date: 2005/05/31 22:10:55 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 

#include "wx/grid.h"
#include "wx/tglbtn.h"
#include "../../common/canalsuperwrapper.h"

class VSCPConfigChild: public wxMDIChildFrame
{
 public:
   
  /// Constructor
  VSCPConfigChild( wxMDIParentFrame *parent, 
								const wxString& title, 
								const wxPoint& pos, 
								const wxSize& size, 
								const long style,
								 devItem *pdev );

  /// Destructor
  ~VSCPConfigChild();

  void OnActivate( wxActivateEvent& event );

  /*!
    Activate deactivate send tools area
  */
  void OnViewSendTools( wxCommandEvent& event );

	void OnButtonUpdate( wxCommandEvent& event );

	void OnRegisterHidden( wxGridEvent& event );
	void OnRegisterEdited( wxGridEvent& event );

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
    Fetch CAN data from the CAN command area
  */
  bool getCanData( canalMsg *pMsg );

  /*!
    Write a CAN message into the CAN command area
  */
  bool writeDataToDialog( canalMsg *pMsg );

	/*!
		Initialize standard register information
	*/
	void initStandardRegInfo( void );

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

	/*!
    Nickname data
  */
  wxString m_wxStrNickname;


	/*!
		Local Register content
	*/
	unsigned char m_registers[ 256 ];

	/*!
		Remote Register content
	*/
	unsigned char m_remote_registers[ 256 ];


  // * * * * * * * *  Command Window Controls * * * * * * * * 
	
  /*!
    Combo to select VSCP interface Level
  */
  wxComboBox *m_pLevelSelectCombo;


	/*!
    Update Registers
  */
  wxButton *m_pBtnUpdate;

	/*!
    Undo register changes.
  */
  wxButton *m_pBtnUndo;

	/*!
    Load MDF (Module Definition File) file
  */
	wxButton *m_pBtnLoadMDF;

	/*!
    Load registers from file
  */
	wxButton *m_pBtnLoadFile;

	/*!
    Save registers to file
  */
	wxButton *m_pBtnSaveFile;

	/*!
    GUID fields	
  */
  wxTextCtrl *m_pEditGUID[ 16 ];

	/*!
    Nickname	
  */
	wxTextCtrl *m_pEditNickname;

	/*!
    URL to MDF file
  */
	wxStaticText *m_pStaticMdfURL;



  DECLARE_EVENT_TABLE()
};


  // Object ID's
  enum {
			RegGrid = 500,
			RegCmdWnd,

			// Command controls
			RegRefreshTimer,
			RegButtonUpdate,
			RegButtonUndo,
			RegEditNickname,
			RegButtonLoadMDF,
			RegStaticMdfURL,
			RegButtonLoadFile,
			RegButtonSaveFile,

			RegEditGUID = 600
	};

