/////////////////////////////////////////////////////////////////////////////
// Name:        frmscanfordevices.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     23/05/2009 17:40:41
// RCS-ID:      
// Copyright:   (C) 2009-2013 
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
/////////////////////////////////////////////////////////////////////////////

#ifndef _FRMSCANFORDEVICES_H_
#define _FRMSCANFORDEVICES_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmscanfordevices.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/toolbar.h"
#include "wx/tglbtn.h"
#include "wx/treectrl.h"
#include "wx/html/htmlwin.h"
////@end includes

#include "../common/canalsuperwrapper.h"
#include "../common/mdf.h"

// Scan popup menu
enum {
  Menu_Popup_GetNodeInfo = 3000,
  Menu_Popup_OpenConfig = 3001
};


/*!
 * Forward declarations
 */

////@begin forward declarations
class wxToggleButton;
class wxTreeCtrl;
class wxHtmlWindow;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_FRMSCANFORDEVICES_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMSCANFORDEVICES_TITLE _("Scan for Devices")
#define SYMBOL_FRMSCANFORDEVICES_IDNAME ID_SCANFORDEVICES
#define SYMBOL_FRMSCANFORDEVICES_SIZE wxSize(800, 470)
#define SYMBOL_FRMSCANFORDEVICES_POSITION wxDefaultPosition
////@end control identifiers

class scanElement : public wxTreeItemData 
{
public:
	uint8_t m_nodeid;
	uint8_t m_reg[256];
	wxString m_html;
};


/*!
 * frmScanforDevices class declaration
 */

class frmScanforDevices: public wxFrame
{    
  DECLARE_CLASS( frmScanforDevices )
  DECLARE_EVENT_TABLE()

public:
	/// Constructors
	frmScanforDevices();
	frmScanforDevices( wxWindow* parent, wxWindowID id = SYMBOL_FRMSCANFORDEVICES_IDNAME, const wxString& caption = SYMBOL_FRMSCANFORDEVICES_TITLE, const wxPoint& pos = SYMBOL_FRMSCANFORDEVICES_POSITION, const wxSize& size = SYMBOL_FRMSCANFORDEVICES_SIZE, long style = SYMBOL_FRMSCANFORDEVICES_STYLE );

	bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FRMSCANFORDEVICES_IDNAME, const wxString& caption = SYMBOL_FRMSCANFORDEVICES_TITLE, const wxPoint& pos = SYMBOL_FRMSCANFORDEVICES_POSITION, const wxSize& size = SYMBOL_FRMSCANFORDEVICES_SIZE, long style = SYMBOL_FRMSCANFORDEVICES_STYLE );

	/// Destructor
	~frmScanforDevices();

	/// Initialises member variables
	void Init();

	/// Creates the controls and sizers -
	void CreateControls();
  
	bool disableInterface( void );
  
	bool enableInterface( void );
  
	/// Interface type
	int m_interfaceType;

	/*!
		Data for the selected device
		id holds type of i/f (DLL, TCP/IP
    */
    //devItem m_itemDevice;
	//both_interface m_both;
	canal_interface m_canalif;
	vscp_interface m_vscpif;

	/*!
		CANAL driver level
	*/
	unsigned char m_driverLevel;
  
	/*!
		The wrapper for the CANAL 
		functionality.
	*/
	CCanalSuperWrapper m_csw;

	/// Module description file functionality
	CMDF m_mdf;

    /// Display info about node
	void getNodeInfo(wxCommandEvent& event);

    /// Open configuration window
	void openConfiguration(wxCommandEvent& event);

////@begin frmScanforDevices event handler declarations

  /// wxEVT_CLOSE_WINDOW event handler for ID_SCANFORDEVICES
  void OnCloseWindow( wxCloseEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_VSCPWORKS_EXIT
  void OnMenuitemExitClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_HELP
  void OnMenuitemHelpClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_HELP_FAQ
  void OnMenuitemHelpFaqClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_HELP_SC
  void OnMenuitemHelpScClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_HELP_THANKS
  void OnMenuitemHelpThanksClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_HELP_CREDITS
  void OnMenuitemHelpCreditsClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_HELP_VSCP_SITE
  void OnMenuitemHelpVscpSiteClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_HELP_ABOUT
  void OnMenuitemHelpAboutClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREE_DEVICE
  void OnTreeDeviceSelChanged( wxTreeEvent& event );

  /// wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK event handler for ID_TREE_DEVICE
  void OnTreeDeviceItemRightClick( wxTreeEvent& event );

  /// wxEVT_LEFT_DCLICK event handler for ID_TREE_DEVICE
  void OnLeftDClick( wxMouseEvent& event );

  /// wxEVT_COMMAND_HTML_LINK_CLICKED event handler for ID_HTMLWINDOW3
  void OnHtmlwindow3LinkClicked( wxHtmlLinkEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SCAN
  void OnButtonScanClick( wxCommandEvent& event );

////@end frmScanforDevices event handler declarations

////@begin frmScanforDevices member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end frmScanforDevices member function declarations

	/// Should we show tooltips?
	static bool ShowToolTips();

////@begin frmScanforDevices member variables
  wxStaticText* m_staticComboText;
  wxComboBox* m_comboNodeID;
  wxToggleButton* m_BtnActivateInterface;
  wxPanel* m_pPanel;
  wxTreeCtrl* m_DeviceTree;
  wxHtmlWindow* m_htmlWnd;
  /// Control identifiers
  enum {
    ID_SCANFORDEVICES = 27000,
    ID_MENU_VSCPWORKS_EXIT = 27001,
    ID_MENUITEM11 = 27005,
    ID_MENUITEM12 = 27006,
    ID_MENUITEM13 = 27007,
    ID_MENUITEM14 = 27008,
    ID_MENUITEM16 = 27010,
    ID_MENUITEM17 = 27011,
    ID_MENUITEM18 = 27012,
    ID_MENUITEM19 = 27013,
    ID_MENUITEM_HELP = 27018,
    ID_MENUITEM_HELP_FAQ = 27019,
    ID_MENUITEM_HELP_SC = 27020,
    ID_MENUITEM_HELP_THANKS = 27021,
    ID_MENUITEM_HELP_CREDITS = 27022,
    ID_MENUITEM_HELP_VSCP_SITE = 27023,
    ID_MENUITEM_HELP_ABOUT = 27024,
    ID_TOOLBAR2 = 27025,
    ID_TOOL1 = 27026,
    ID_TOOL2 = 27027,
    ID_TOOL3 = 27028,
    ID_TOOL4 = 27029,
    ID_TOOL5 = 27030,
    ID_TOOL12 = 27031,
    ID_COMBOBOX1 = 27032,
    ID_TOGGLEBUTTON = 27034,
    ID_PANEL_DEVICE_SCAN = 27004,
    ID_TREE_DEVICE = 27035,
    ID_HTMLWINDOW3 = 27002,
    ID_BUTTON_SCAN = 27003
  };
////@end frmScanforDevices member variables
};

#endif
  // _FRMSCANFORDEVICES_H_
