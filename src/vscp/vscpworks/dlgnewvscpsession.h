/////////////////////////////////////////////////////////////////////////////
// Name:        dlgnewvscpsession.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 09:36:35 CEST
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

#ifndef _DLGNEWVSCPSESSION_H_
#define _DLGNEWVSCPSESSION_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgnewvscpsession.h"
#endif

/*!
 * Includes
 */


/*!
 * Forward declarations
 */


/*!
 * Control identifiers
 */

#define SYMBOL_DLGNEWVSCPSESSION_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGNEWVSCPSESSION_TITLE _("Select interface to use for session")
#define SYMBOL_DLGNEWVSCPSESSION_IDNAME ID_DIALOG_NEW_VSCP_SESSION
#define SYMBOL_DLGNEWVSCPSESSION_SIZE wxSize(400, 300)
#define SYMBOL_DLGNEWVSCPSESSION_POSITION wxDefaultPosition


/*!
 * dlgNewVSCPSession class declaration
 */

class dlgNewVSCPSession: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( dlgNewVSCPSession )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  dlgNewVSCPSession();
  dlgNewVSCPSession( wxWindow* parent, 
                        wxWindowID id = SYMBOL_DLGNEWVSCPSESSION_IDNAME, 
                        const wxString& caption = SYMBOL_DLGNEWVSCPSESSION_TITLE, 
                        const wxPoint& pos = SYMBOL_DLGNEWVSCPSESSION_POSITION, 
                        const wxSize& size = SYMBOL_DLGNEWVSCPSESSION_SIZE, 
                        long style = SYMBOL_DLGNEWVSCPSESSION_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGNEWVSCPSESSION_IDNAME, 
                    const wxString& caption = SYMBOL_DLGNEWVSCPSESSION_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGNEWVSCPSESSION_POSITION, 
                    const wxSize& size = SYMBOL_DLGNEWVSCPSESSION_SIZE, 
                    long style = SYMBOL_DLGNEWVSCPSESSION_STYLE );

  /// Destructor
  ~dlgNewVSCPSession();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();
  
  /*!
    Fill the listbox with data
  */
  void fillListBox( const wxString& strDecription );

  /*!
    Cleanup listbox (both visual and object data).
  */
  void cleanupListbox( void );

  /// wxEVT_INIT_DIALOG event handler for ID_DIALOG_NEW_VSCP_SESSION
  void OnInitDialog( wxInitDialogEvent& event );

  /// wxEVT_DESTROY event handler for ID_DIALOG_NEW_VSCP_SESSION
  void OnDestroy( wxWindowDestroyEvent& event );

  /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_INTERFACES
  void OnListboxInterfacesSelected( wxCommandEvent& event );

  /// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_INTERFACES
  void OnListboxInterfacesDoubleClicked( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
  void OnOKClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
  void OnCANCELClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD
  void OnButtonAddClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_EDIT
  void OnButtonEditClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_REMOVE
  void OnButtonRemoveClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
  void OnButtonCloneClick( wxCommandEvent& event );


  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );


  /// Should we show tooltips?
  static bool ShowToolTips();
	
	// Flag for unconnected mode line
	bool m_bShowUnconnectedMode;


  wxListBox* m_ctrlListInterfaces;
  /// Control identifiers
  enum {
    ID_DIALOG_NEW_VSCP_SESSION = 14000,
    ID_LISTBOX_INTERFACES = 14001,
    ID_BUTTON_ADD = 14002,
    ID_BUTTON_EDIT = 14003,
    ID_BUTTON_REMOVE = 14004,
    ID_BUTTON = 14005
  };

};

#endif
  // _DLGNEWVSCPSESSION_H_
