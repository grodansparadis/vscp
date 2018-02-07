/////////////////////////////////////////////////////////////////////////////
// Name:        dlgvscptrmit.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     22/11/2007 17:46:01
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

#ifndef _DLGVSCPTRMIT_H_
#define _DLGVSCPTRMIT_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dlgvscptrmit.h"
#endif


#include <vscp.h>
#include "vscptxobj.h"


#define SYMBOL_DLGVSCPTRMITELEMENT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_DLGVSCPTRMITELEMENT_TITLE _("VSCP Transmission Event")
#define SYMBOL_DLGVSCPTRMITELEMENT_IDNAME ID_DIALOG_VSCP_TRANSMITT_ELEMENT
#define SYMBOL_DLGVSCPTRMITELEMENT_SIZE wxSize(400, 340)
#define SYMBOL_DLGVSCPTRMITELEMENT_POSITION wxDefaultPosition


///////////////////////////////////////////////////////////////////////////////
// dlgVSCPTrmitElement class declaration
//

class dlgVSCPTrmitElement: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( dlgVSCPTrmitElement )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    dlgVSCPTrmitElement();
    dlgVSCPTrmitElement( wxWindow* parent, 
                            wxWindowID id = SYMBOL_DLGVSCPTRMITELEMENT_IDNAME, 
                            const wxString& caption = SYMBOL_DLGVSCPTRMITELEMENT_TITLE, 
                            const wxPoint& pos = SYMBOL_DLGVSCPTRMITELEMENT_POSITION, 
                            const wxSize& size = SYMBOL_DLGVSCPTRMITELEMENT_SIZE, 
                            long style = SYMBOL_DLGVSCPTRMITELEMENT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                    wxWindowID id = SYMBOL_DLGVSCPTRMITELEMENT_IDNAME, 
                    const wxString& caption = SYMBOL_DLGVSCPTRMITELEMENT_TITLE, 
                    const wxPoint& pos = SYMBOL_DLGVSCPTRMITELEMENT_POSITION, 
                    const wxSize& size = SYMBOL_DLGVSCPTRMITELEMENT_SIZE, 
                    long style = SYMBOL_DLGVSCPTRMITELEMENT_STYLE );

    /// Destructor
    ~dlgVSCPTrmitElement();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
  
    /*!
        Get event data from dialog
        @param pEvent Event to fill with data.
    */
    bool getEventData( VscpTXObj *pObj );
  
    /*!
        Write event data to dialog
        @param pEvent Event to fill with data.
    */
    bool writeEventData( VscpTXObj *pObj );
  
    /*!
        Fill Class combo with data
    */
    void fillClassCombo( void );
  
    /*!
        Fill Type combo with data
    */
    void fillTypeCombo( unsigned int vscp_class );

    /// wxEVT_INIT_DIALOG event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT
    void OnInitDialog( wxInitDialogEvent& event );

    /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_VSCP_TRANSMITT_ELEMENT
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_StrVscpClass
    void OnStrVscpClassSelected( wxCommandEvent& event );


    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxCheckBox* m_wxChkActive;
    wxTextCtrl* m_wxStrName;
    wxComboBox* m_wxComboClass;
    wxComboBox* m_wxComboType;
    wxChoice* m_wxChoicePriority;
    wxCheckBox* m_wxChkDefaultGUID;
    wxTextCtrl* m_wxStrGUID15;
    wxTextCtrl* m_wxStrGUID14;
    wxTextCtrl* m_wxStrGUID13;
    wxTextCtrl* m_wxStrGUID12;
    wxTextCtrl* m_wxStrGUID11;
    wxTextCtrl* m_wxStrGUID10;
    wxTextCtrl* m_wxStrGUID9;
    wxTextCtrl* m_wxStrGUID8;
    wxTextCtrl* m_wxStrGUID7;
    wxTextCtrl* m_wxStrGUID6;
    wxTextCtrl* m_wxStrGUID5;
    wxTextCtrl* m_wxStrGUID4;
    wxTextCtrl* m_wxStrGUID3;
    wxTextCtrl* m_wxStrGUID2;
    wxTextCtrl* m_wxStrGUID1;
    wxTextCtrl* m_wxStrGUID0;
    wxTextCtrl* m_wxStrData;
    wxTextCtrl* m_wxStrCount;
    wxTextCtrl* m_wxStrPeriod;
    wxComboBox* m_wxComboTrigger;
  
    /// Control identifiers
    enum {
        ID_DIALOG_VSCP_TRANSMITT_ELEMENT = 19000,
        ID_CHECKBOX1 = 19001,
        ID_StrName = 19002,
        ID_StrVscpClass = 19003,
        ID_StrVscpType = 19004,
        ID_WxChoicePriority = 19043,
        ID_CHKBOX_USE_DEFAULT = 19044,
        ID_StrGUID15 = 19045,
        ID_WxStrGUID14 = 19046,
        ID_WxStrGUID13 = 19047,
        ID_WxStrGUID12 = 19048,
        ID_WxStrGUID11 = 19049,
        ID_WxStrGUID10 = 19050,
        ID_WxStrGUID9 = 19051,
        ID_WxStrGUID8 = 19052,
        ID_WxStrGUID7 = 19053,
        ID_WxStrGUID6 = 19054,
        ID_WxStrGUID5 = 19055,
        ID_WxStrGUID4 = 19056,
        ID_WxStrGUID3 = 19057,
        ID_WxStrGUID2 = 19058,
        ID_WxStrGUID1 = 19059,
        ID_WxStrGUID0 = 19060,
        ID_StrVscpData = 19061,
        ID_StrCount = 19062,
        ID_StrPeriod = 19063,
        ID_ComboTrigger = 19064
  };

};

#endif // _DLGVSCPTRMIT_H_



