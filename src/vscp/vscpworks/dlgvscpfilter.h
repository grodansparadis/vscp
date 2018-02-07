/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscpfilter.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 25 Oct 2007 22:17:45 CEST
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

#ifndef _DLGVSCPFILTER_H_
#define _DLGVSCPFILTER_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgvscpfilter.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "dlgvscpfilter_symbols.h"
////@end includes

#include <vscp.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_DLGVSCPFILTER_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_DLGVSCPFILTER_TITLE _("VSCP Filter")
#define SYMBOL_DLGVSCPFILTER_IDNAME ID_DLGVSCPFILTER
#define SYMBOL_DLGVSCPFILTER_SIZE wxSize(400, 300)
#define SYMBOL_DLGVSCPFILTER_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * dlgVSCPFilter class declaration
 */

class dlgVSCPFilter: public wxDialog
{    
  DECLARE_DYNAMIC_CLASS( dlgVSCPFilter )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  dlgVSCPFilter();
  dlgVSCPFilter( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGVSCPFILTER_IDNAME, 
                    const wxString& caption = SYMBOL_DLGVSCPFILTER_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGVSCPFILTER_POSITION, 
                    const wxSize& size = SYMBOL_DLGVSCPFILTER_SIZE, 
                    long style = SYMBOL_DLGVSCPFILTER_STYLE );

  /// Creation
  bool Create( wxWindow* parent, 
                wxWindowID id = SYMBOL_DLGVSCPFILTER_IDNAME, 
                const wxString& caption = SYMBOL_DLGVSCPFILTER_TITLE, 
                const wxPoint& pos = SYMBOL_DLGVSCPFILTER_POSITION, 
                const wxSize& size = SYMBOL_DLGVSCPFILTER_SIZE, 
                long style = SYMBOL_DLGVSCPFILTER_STYLE );

  /// Destructor
  ~dlgVSCPFilter();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin dlgVSCPFilter event handler declarations

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_CLASS
  void OnButtonSetClassClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_TYPE
  void OnButtonSetTypeClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_PRIORITY
  void OnButtonSetPriorityClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_INDEX
  void OnButtonSetIndexClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_ZONE
  void OnButtonSetZoneClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SET_SUBZONE
  void OnButtonSetSubzoneClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_LOAD_FILTER
  void OnButtonLoadFilterClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SAVE_FILTER
  void OnButtonSaveFilterClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_GUID_FILTER_WIZARD
  void OnButtonGuidFilterWizardClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_VSCP_FILTER_SETUP_WIZARD
  void OnButtonVscpFilterSetupWizardClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CLEAR
  void OnButtonClearClick( wxCommandEvent& event );

////@end dlgVSCPFilter event handler declarations

////@begin dlgVSCPFilter member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end dlgVSCPFilter member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();
	
	/*!
		Set VSCP filter values
		@param pFilter Pointer to VSCP filter stucture	
	*/
	void setFilter( vscpEventFilter *pFilter );
	
		/*!
		Get VSCP filter values
		@param pFilter Pointer to VSCP filter stucture	
	*/
	void getFilter( vscpEventFilter *pFilter );

////@begin dlgVSCPFilter member variables
  wxTextCtrl* m_CLassFilter;
  wxTextCtrl* m_ClassMask;
  wxTextCtrl* m_TypeFilter;
  wxTextCtrl* m_TypeMask;
  wxTextCtrl* m_PriorityFilter;
  wxTextCtrl* m_PriorityMask;
  wxTextCtrl* m_IndexFilter;
  wxTextCtrl* m_IndexMask;
  wxTextCtrl* m_ZoneFilter;
  wxTextCtrl* m_ZoneMask;
  wxTextCtrl* m_SubZoneFilter;
  wxTextCtrl* m_SubZoneMask;
  wxTextCtrl* m_filterGUID7;
  wxTextCtrl* m_filterGUID6;
  wxTextCtrl* m_filterGUID5;
  wxTextCtrl* m_filterGUID4;
  wxTextCtrl* m_filterGUID3;
  wxTextCtrl* m_filterGUID2;
  wxTextCtrl* m_filterGUID1;
  wxTextCtrl* m_filterGUID0;
  wxTextCtrl* m_filterGUID15;
  wxTextCtrl* m_filterGUID14;
  wxTextCtrl* m_filterGUID13;
  wxTextCtrl* m_filterGUID12;
  wxTextCtrl* m_filterGUID11;
  wxTextCtrl* m_filterGUID10;
  wxTextCtrl* m_filterGUID9;
  wxTextCtrl* m_filterGUID8;
  wxTextCtrl* m_maskGUID7;
  wxTextCtrl* m_maskGUID6;
  wxTextCtrl* m_maskGUID5;
  wxTextCtrl* m_maskGUID4;
  wxTextCtrl* m_maskGUID3;
  wxTextCtrl* m_maskGUID2;
  wxTextCtrl* m_maskGUID1;
  wxTextCtrl* m_maskGUID0;
  wxTextCtrl* m_maskGUID15;
  wxTextCtrl* m_maskGUID14;
  wxTextCtrl* m_maskGUID13;
  wxTextCtrl* m_maskGUID12;
  wxTextCtrl* m_maskGUID11;
  wxTextCtrl* m_maskGUID10;
  wxTextCtrl* m_maskGUID9;
  wxTextCtrl* m_maskGUID8;
  /// Control identifiers
  enum {
    ID_DLGVSCPFILTER = 17000,
    ID_CLassFilter = 17001,
    ID_ClassMask = 17002,
    ID_BUTTON_SET_CLASS = 17003,
    ID_TEXTCTRL_TYPEFILTER = 17038,
    ID_TEXTCTRL_TYPEMASK = 17039,
    ID_BUTTON_SET_TYPE = 17040,
    ID_TEXTCTRL_PRIORITY_FILTER = 17053,
    ID_TEXTCTRL_PRIORITY_MASK = 17054,
    ID_BUTTON_SET_PRIORITY = 17055,
    ID_TEXTCTRL16 = 17041,
    ID_TEXTCTRL17 = 17042,
    ID_BUTTON_SET_INDEX = 17043,
    ID_TEXTCTRL18 = 17044,
    ID_TEXTCTRL19 = 17045,
    ID_BUTTON_SET_ZONE = 17046,
    ID_TEXTCTRL20 = 17047,
    ID_TEXTCTRL21 = 17048,
    ID_BUTTON_SET_SUBZONE = 17049,
    ID_FilterGUID7 = 17004,
    ID_FilterGUID6 = 17005,
    ID_FilterGUID5 = 17006,
    ID_FilterGUID4 = 17007,
    ID_FilterGUID3 = 17008,
    ID_FilterGUID2 = 17009,
    ID_FilterGUID1 = 17010,
    ID_FilterGUID0 = 17011,
    ID_TEXTCTRL = 17012,
    ID_TEXTCTRL1 = 17013,
    ID_TEXTCTRL2 = 17014,
    ID_TEXTCTRL3 = 17015,
    ID_TEXTCTRL4 = 17016,
    ID_TEXTCTRL5 = 17017,
    ID_TEXTCTRL6 = 17018,
    ID_TEXTCTRL7 = 17019,
    ID_TEXTCTRL8 = 17020,
    ID_TEXTCTRL9 = 17021,
    ID_TEXTCTRL10 = 17022,
    ID_TEXTCTRL11 = 17023,
    ID_TEXTCTRL12 = 17024,
    ID_TEXTCTRL13 = 17025,
    ID_TEXTCTRL14 = 17026,
    ID_TEXTCTRL15 = 17027,
    ID_MaskGUID15 = 17028,
    ID_MaskGUID14 = 17029,
    ID_MaskGUID13 = 17030,
    ID_MaskGUID12 = 17031,
    ID_MaskGUID11 = 17032,
    ID_MaskGUID10 = 17033,
    ID_MaskGUID9 = 17034,
    ID_MaskGUID8 = 17035,
    ID_BUTTON_LOAD_FILTER = 17051,
    ID_BUTTON_SAVE_FILTER = 17050,
    ID_BUTTON_GUID_FILTER_WIZARD = 17036,
    ID_BUTTON_VSCP_FILTER_SETUP_WIZARD = 17037,
    ID_BUTTON_CLEAR = 17052
  };
////@end dlgVSCPFilter member variables
};

#endif
  // _DLGVSCPFILTER_H_
