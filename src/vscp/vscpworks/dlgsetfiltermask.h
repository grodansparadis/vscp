/////////////////////////////////////////////////////////////////////////////
// Name:        dialogsetfiltermask.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     21/03/2009 00:45:10
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

#ifndef _DIALOGSETFILTERMASK_H_
#define _DIALOGSETFILTERMASK_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgsetfiltermask.h"
#endif

/*!
 * Includes
 */

#include "vscphelper.h"

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_DIALOGSETFILTERMASK_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_DIALOGSETFILTERMASK_TITLE _("Dialog Set filter/mask")
#define SYMBOL_DIALOGSETFILTERMASK_IDNAME ID_DIALOGSETFILTERMASK
#define SYMBOL_DIALOGSETFILTERMASK_SIZE wxSize(400, 300)
#define SYMBOL_DIALOGSETFILTERMASK_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * DialogSetfiltermask class declaration
 */

class DialogSetfiltermask: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( DialogSetfiltermask )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  DialogSetfiltermask();
  DialogSetfiltermask( wxWindow* parent, 
                            wxWindowID id = SYMBOL_DIALOGSETFILTERMASK_IDNAME, 
                            const wxString& caption = SYMBOL_DIALOGSETFILTERMASK_TITLE, 
                            const wxPoint& pos = SYMBOL_DIALOGSETFILTERMASK_POSITION, 
                            const wxSize& size = SYMBOL_DIALOGSETFILTERMASK_SIZE, 
                            long style = SYMBOL_DIALOGSETFILTERMASK_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DIALOGSETFILTERMASK_IDNAME, 
                    const wxString& caption = SYMBOL_DIALOGSETFILTERMASK_TITLE, 
                    const wxPoint& pos = SYMBOL_DIALOGSETFILTERMASK_POSITION, 
                    const wxSize& size = SYMBOL_DIALOGSETFILTERMASK_SIZE, 
                    long style = SYMBOL_DIALOGSETFILTERMASK_STYLE );

  /// Destructor
  ~DialogSetfiltermask();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

  /*!
    Calculate filter/masks from selected events
  */
  void calculateFilterMask( void );

  /// Class mask
  uint16_t m_classmask;

  /// Class filter
  uint16_t m_classfilter;

  /// Type mask
  uint16_t m_typemask;

  /// Type filter
  uint16_t m_typefilter;

////@begin DialogSetfiltermask event handler declarations

  /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX
  void OnComboboxSelected( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD_EVENT
  void OnButtonAddEventClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_REMOVE_EVENT
  void OnButtonRemoveEventClick( wxCommandEvent& event );

////@end DialogSetfiltermask event handler declarations

////@begin DialogSetfiltermask member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end DialogSetfiltermask member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin DialogSetfiltermask member variables
  wxComboBox* m_wxComboClass;
  wxComboBox* m_wxComboType;
  wxStaticText* m_statusText;
  wxListBox* m_listboxEvents;
  /// Control identifiers
  enum {
    ID_DIALOGSETFILTERMASK = 10027,
    ID_COMBOBOX = 10028,
    ID_COMBOBOX1 = 10000,
    ID_BUTTON_ADD_EVENT = 10029,
    ID_BUTTON_REMOVE_EVENT = 10001,
    ID_LISTBOX = 10030
  };
////@end DialogSetfiltermask member variables
};

#endif
  // _DIALOGSETFILTERMASK_H_
