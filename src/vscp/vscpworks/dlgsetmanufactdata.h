/////////////////////////////////////////////////////////////////////////////
// Name:        dlgsetmanufactdata.h
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

#ifndef _DLGSETMANUFACTDATA_H_
#define _DLGSETMANUFACTDATA_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgsetmanufactdata.h"
#endif


/*!
 * Control identifiers
 */

#define ID_CSETMANUFACTDATA 10000
#define ID_TEXTCTRL_GUID 10001
#define ID_TEXTCTRL_MANDEVID 10002
#define ID_TEXTCTRL_MANDEVSUBID 10003
#define ID_BUTTON_LOAD 10006
#define ID_BUTTON_WRITE 10004
#define SYMBOL_CSETMANUFACTDATA_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSETMANUFACTDATA_TITLE _("Set GUID")
#define SYMBOL_CSETMANUFACTDATA_IDNAME ID_CSETMANUFACTDATA
#define SYMBOL_CSETMANUFACTDATA_SIZE wxSize(400, 300)
#define SYMBOL_CSETMANUFACTDATA_POSITION wxDefaultPosition

/*!
 * CsetManufactData class declaration
 */

class CsetManufactData: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CsetManufactData )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CsetManufactData();
    CsetManufactData( wxWindow* parent, 
                        wxWindowID id = SYMBOL_CSETMANUFACTDATA_IDNAME, 
                        const wxString& caption = SYMBOL_CSETMANUFACTDATA_TITLE, 
                        const wxPoint& pos = SYMBOL_CSETMANUFACTDATA_POSITION, 
                        const wxSize& size = SYMBOL_CSETMANUFACTDATA_SIZE, 
                        long style = SYMBOL_CSETMANUFACTDATA_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_CSETMANUFACTDATA_IDNAME, 
                    const wxString& caption = SYMBOL_CSETMANUFACTDATA_TITLE, 
                    const wxPoint& pos = SYMBOL_CSETMANUFACTDATA_POSITION, 
                    const wxSize& size = SYMBOL_CSETMANUFACTDATA_SIZE, 
                    long style = SYMBOL_CSETMANUFACTDATA_STYLE );

    /// Destructor
    ~CsetManufactData();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxTextCtrl* m_pctrlGUID;
    wxTextCtrl* m_pctrlManDevId;
    wxTextCtrl* m_pctrlManDevSubId;
};

#endif
    // _DLGSETMANUFACTDATA_H_
