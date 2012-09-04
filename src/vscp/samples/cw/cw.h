// cw.h
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
// $RCSfile: cw.h,v $                                       
// $Date: 2005/09/15 09:50:56 $                                  
// $Author: akhe $                                              
// $Revision: 1.8 $ 

#include "wx/toolbar.h"
#include "../../common/canal.h"

// Define a new application
class CanalWorksApp : public wxApp
{
 public:

	bool OnInit();

	int OnExit();

	/*!
		Load persistent data
	*/
	bool loadRegistryData( void );

	/*!
		Save persistent data
	*/
	void saveRegistryData( void );

	/*!
		Flag for list delete warings	
		Default is true.
	*/
	bool m_bDisplayDeleteWarning;
	
	/*!	
		Saved main window width	
	*/
	int m_sizeWidth;

	/*!	
		Saved main window height	
	*/
	int m_sizeHeight;

  /*!
    Username for daemon login
   */
  wxString m_strUsername;

  /*!
    Password for daemon login
   */
  wxString m_strPassword;

  /*!
    Hostname for daemon login
   */
  wxString m_strHostname;

  /*!
    Port for daemon login
  */
  short m_port;

};



///////////////////////////////////////////////////////////////////////////////
//			  M A I N  F R A M E
///////////////////////////////////////////////////////////////////////////////
// Define a new frame
class MyFrame : public wxMDIParentFrame
{
  
 public:
  wxTextCtrl *m_pdebugWindow;
  
  MyFrame( wxWindow *parent, 
						const wxWindowID id, 
						const wxString& title,
						const wxPoint& pos, 
						const wxSize& size, 
						const long style );
  
  void InitToolBar(wxToolBar* toolBar);
  
  void OnSize( wxSizeEvent& event );
  
	void OnAbout( wxCommandEvent& event );

	/*!
		Open a new CANAL client window
	*/
  void OnNewCANALWindow( wxCommandEvent& event );
	
	/*!
		Open a new VSCP client window
	*/
	void OnNewVSCPWindow( wxCommandEvent& event );

	/*!
		Open a new VSCP configuration window
	*/
	void OnNewVSCPConfigWindow( wxCommandEvent& event );
  
	void OnQuit( wxCommandEvent& event );
  void OnClose( wxCloseEvent& event );


	// * * * * * Helpers * * * * *

	/*!
		Get a numerical data value (hex or decimal) from a string
		@param szData Strng containing value in string form
		@return The converted number
	*/
	//uint32_t getDataValue( wxString strData );

	/*!
		Option Menu
	*/
	wxMenu *m_poption_menu;


  DECLARE_EVENT_TABLE()
};



    // menu items ids
    enum
      {
				MDI_QUIT = 100,
				MDI_NEW_CANAL_WINDOW,
				MDI_NEW_VSCP_WINDOW,
				MDI_NEW_VSCP_CONFIG,
				MDI_NEW_VSCP_BOOTLOAD,
				MDI_VIEW_SEND_TOOLS,
				MDI_ACTIVATE_INTERFACE,
				MDI_ACTIVATE_FILTER,
				MDI_CHANGE_TITLE,
				MDI_CHANGE_POSITION,
				MDI_CHANGE_SIZE,
				MDI_CHILD_QUIT,
				MDI_ABOUT
      };
