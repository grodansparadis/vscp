/////////////////////////////////////////////////////////////////////////////
// Name:        merlin.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     22/03/2012 21:03:29
// RCS-ID:      
// Copyright:   (C) 2012-2018 
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

#ifndef _MERLIN_H_
#define _MERLIN_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "merlin.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/toolbar.h"
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
#define SYMBOL_MERLIN_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MERLIN_TITLE _("Merlin")
#define SYMBOL_MERLIN_IDNAME ID_MERLIN
#define SYMBOL_MERLIN_SIZE wxSize(800, 600)
#define SYMBOL_MERLIN_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * Merlin class declaration
 */

class Merlin: public wxFrame
{    
  DECLARE_CLASS( Merlin )
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  Merlin();
  Merlin( wxWindow* parent, wxWindowID id = SYMBOL_MERLIN_IDNAME, const wxString& caption = SYMBOL_MERLIN_TITLE, const wxPoint& pos = SYMBOL_MERLIN_POSITION, const wxSize& size = SYMBOL_MERLIN_SIZE, long style = SYMBOL_MERLIN_STYLE );

  bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MERLIN_IDNAME, const wxString& caption = SYMBOL_MERLIN_TITLE, const wxPoint& pos = SYMBOL_MERLIN_POSITION, const wxSize& size = SYMBOL_MERLIN_SIZE, long style = SYMBOL_MERLIN_STYLE );

  /// Destructor
  ~Merlin();

  /// Initialises member variables
  void Init();

  /// Creates the controls and sizers
  void CreateControls();

////@begin Merlin event handler declarations

  /// wxEVT_PAINT event handler for ID_DRAWING_AREA
  void OnPaint( wxPaintEvent& event );

////@end Merlin event handler declarations

////@begin Merlin member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end Merlin member function declarations

  /// Should we show tooltips?
  static bool ShowToolTips();

////@begin Merlin member variables
  /// Control identifiers
  enum {
    ID_MERLIN = 10044,
    ID_MENUITEM6 = 10140,
    ID_MENUITEM = 10141,
    ID_MENUITEM1 = 10142,
    ID_TOOLBAR1 = 10138,
    ID_SCROLLEDWINDOW = 10045,
    ID_BITMAPBUTTON1 = 10001,
    ID_BITMAPBUTTON = 10046,
    ID_BITMAPBUTTON2 = 10002,
    ID_BITMAPBUTTON3 = 10003,
    ID_BITMAPBUTTON4 = 10004,
    ID_BITMAPBUTTON6 = 10006,
    ID_BITMAPBUTTON7 = 10007,
    ID_BITMAPBUTTON8 = 10008,
    ID_BITMAPBUTTON9 = 10009,
    ID_BITMAPBUTTON10 = 10010,
    ID_BITMAPBUTTON11 = 10011,
    ID_BITMAPBUTTON12 = 10012,
    ID_BITMAPBUTTON13 = 10013,
    ID_BITMAPBUTTON14 = 10014,
    ID_BITMAPBUTTON15 = 10015,
    ID_BITMAPBUTTON16 = 10016,
    ID_BITMAPBUTTON17 = 10017,
    ID_BITMAPBUTTON18 = 10018,
    ID_BITMAPBUTTON19 = 10019,
    ID_BITMAPBUTTON20 = 10020,
    ID_BITMAPBUTTON21 = 10021,
    ID_BITMAPBUTTON22 = 10022,
    ID_BITMAPBUTTON23 = 10023,
    ID_BITMAPBUTTON24 = 10024,
    ID_BITMAPBUTTON25 = 10025,
    ID_BITMAPBUTTON26 = 10026,
    ID_BITMAPBUTTON27 = 10027,
    ID_BITMAPBUTTON28 = 10028,
    ID_BITMAPBUTTON29 = 10029,
    ID_BITMAPBUTTON30 = 10030,
    ID_BITMAPBUTTON31 = 10031,
    ID_BITMAPBUTTON32 = 10032,
    ID_BITMAPBUTTON33 = 10033,
    ID_BITMAPBUTTON34 = 10034,
    ID_BITMAPBUTTON35 = 10035,
    ID_BITMAPBUTTON36 = 10036,
    ID_BITMAPBUTTON37 = 10037,
    ID_BITMAPBUTTON38 = 10038,
    ID_BITMAPBUTTON39 = 10039,
    ID_BITMAPBUTTON40 = 10040,
    ID_BITMAPBUTTON41 = 10041,
    ID_BITMAPBUTTON42 = 10042,
    ID_BITMAPBUTTON43 = 10043,
    ID_BITMAPBUTTON44 = 10047,
    ID_BITMAPBUTTON45 = 10048,
    ID_BITMAPBUTTON46 = 10049,
    ID_BITMAPBUTTON47 = 10050,
    ID_BITMAPBUTTON48 = 10051,
    ID_BITMAPBUTTON49 = 10052,
    ID_BITMAPBUTTON50 = 10053,
    ID_BITMAPBUTTON51 = 10054,
    ID_BITMAPBUTTON52 = 10055,
    ID_BITMAPBUTTON53 = 10056,
    ID_BITMAPBUTTON54 = 10057,
    ID_BITMAPBUTTON55 = 10058,
    ID_BITMAPBUTTON56 = 10059,
    ID_BITMAPBUTTON57 = 10060,
    ID_BITMAPBUTTON58 = 10061,
    ID_BITMAPBUTTON59 = 10062,
    ID_BITMAPBUTTON60 = 10063,
    ID_BITMAPBUTTON61 = 10064,
    ID_BITMAPBUTTON62 = 10065,
    ID_BITMAPBUTTON63 = 10066,
    ID_BITMAPBUTTON64 = 10067,
    ID_BITMAPBUTTON65 = 10068,
    ID_BITMAPBUTTON66 = 10069,
    ID_BITMAPBUTTON67 = 10070,
    ID_BITMAPBUTTON68 = 10071,
    ID_BITMAPBUTTON69 = 10072,
    ID_BITMAPBUTTON70 = 10073,
    ID_BITMAPBUTTON71 = 10074,
    ID_BITMAPBUTTON72 = 10075,
    ID_BITMAPBUTTON73 = 10076,
    ID_BITMAPBUTTON74 = 10077,
    ID_BITMAPBUTTON75 = 10078,
    ID_BITMAPBUTTON76 = 10079,
    ID_BITMAPBUTTON77 = 10080,
    ID_BITMAPBUTTON78 = 10081,
    ID_BITMAPBUTTON79 = 10082,
    ID_BITMAPBUTTON80 = 10083,
    ID_BITMAPBUTTON81 = 10084,
    ID_BITMAPBUTTON82 = 10085,
    ID_BITMAPBUTTON83 = 10086,
    ID_BITMAPBUTTON84 = 10087,
    ID_BITMAPBUTTON85 = 10088,
    ID_BITMAPBUTTON86 = 10089,
    ID_BITMAPBUTTON87 = 10090,
    ID_BITMAPBUTTON88 = 10091,
    ID_BITMAPBUTTON89 = 10092,
    ID_BITMAPBUTTON90 = 10093,
    ID_BITMAPBUTTON91 = 10094,
    ID_BITMAPBUTTON92 = 10095,
    ID_BITMAPBUTTON93 = 10096,
    ID_BITMAPBUTTON94 = 10097,
    ID_BITMAPBUTTON95 = 10098,
    ID_BITMAPBUTTON96 = 10099,
    ID_BITMAPBUTTON97 = 10100,
    ID_BITMAPBUTTON98 = 10101,
    ID_BITMAPBUTTON99 = 10102,
    ID_BITMAPBUTTON100 = 10103,
    ID_BITMAPBUTTON101 = 10104,
    ID_BITMAPBUTTON102 = 10105,
    ID_BITMAPBUTTON103 = 10106,
    ID_BITMAPBUTTON104 = 10107,
    ID_BITMAPBUTTON105 = 10108,
    ID_BITMAPBUTTON106 = 10109,
    ID_BITMAPBUTTON107 = 10110,
    ID_BITMAPBUTTON108 = 10111,
    ID_BITMAPBUTTON109 = 10112,
    ID_BITMAPBUTTON110 = 10113,
    ID_BITMAPBUTTON111 = 10114,
    ID_BITMAPBUTTON112 = 10115,
    ID_BITMAPBUTTON113 = 10116,
    ID_BITMAPBUTTON114 = 10117,
    ID_BITMAPBUTTON115 = 10118,
    ID_BITMAPBUTTON116 = 10119,
    ID_BITMAPBUTTON117 = 10120,
    ID_BITMAPBUTTON118 = 10121,
    ID_BITMAPBUTTON119 = 10122,
    ID_BITMAPBUTTON120 = 10123,
    ID_BITMAPBUTTON121 = 10124,
    ID_BITMAPBUTTON122 = 10125,
    ID_BITMAPBUTTON123 = 10126,
    ID_BITMAPBUTTON124 = 10127,
    ID_BITMAPBUTTON125 = 10128,
    ID_BITMAPBUTTON126 = 10129,
    ID_BITMAPBUTTON127 = 10130,
    ID_BITMAPBUTTON128 = 10131,
    ID_BITMAPBUTTON129 = 10132,
    ID_BITMAPBUTTON130 = 10133,
    ID_BITMAPBUTTON131 = 10134,
    ID_BITMAPBUTTON132 = 10135,
    ID_BITMAPBUTTON133 = 10136,
    ID_BITMAPBUTTON134 = 10137,
    ID_BITMAPBUTTON5 = 10005,
    ID_DRAWING_AREA = 10000
  };
////@end Merlin member variables
};

#endif
  // _MERLIN_H_
