/////////////////////////////////////////////////////////////////////////////
// Name:        dlgnewvscpsession.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 09:36:35 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2011 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://m2m.sourceforge.net) 
//
// Copyright (C) 2000-2011 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
//  Grodans Paradis AB at info@Grodans Paradis AB.se, http://www.grodansparadis.com
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dlgnewvscpsession.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/notebook.h>
#include <wx/list.h>

////@begin includes
////@end includes

#include "../common/vscp.h"
#include "../common/vscphelper.h"
#include "vscpworks.h"
#include "dlgnewvscpsession.h"
#include "dlgvscpinterfacesettings.h"

extern appConfiguration g_Config;


////@begin XPM images
/* XPM */
static const char *fatbee_v2_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 253 2",
"c. c #75790B",
"c  c Black",
"dX c #A8AE0F",
"3. c #3C3C3C",
"l  c #73E4E4",
"d  c #595B12",
"~  c #4A4C08",
"y. c #292A04",
"8X c #6E4F0B",
".  c #828817",
"I  c #B0B60C",
"(. c #E8EF0F",
"H  c #5C5F0A",
"x  c #5C5F0B",
"L. c #C2C90E",
"k. c #5C5F12",
"K  c #2A463A",
"xX c #211C13",
"w. c #7E8062",
"s. c #D9E00D",
"8. c #D9E00E",
"r  c #566060",
"(  c #B8BE0D",
"HX c #533B09",
"{. c #0B0C04",
"C. c #EBF30E",
"*  c #6ED1D1",
"}. c #454213",
"T  c #3E4107",
"p  c #427F7F",
"t  c #6CD7D7",
"W  c #010100",
"mX c #996D0C",
"4. c #767A0C",
"D  c #010101",
"M  c #3D7878",
"lX c #101010",
"OX c #767A0F",
"+  c #7CFCFC",
"4X c #4C4021",
">. c #343607",
"$  c #52A1A1",
"9  c #7CF9F9",
"V. c #BBC20F",
"<X c #CBA455",
"a  c #458686",
"1  c #6FDEDD",
"1. c #5C7171",
"_  c #D2D90E",
"6  c #4D9797",
"$. c #7E5C10",
"r. c #E4EC0D",
"O. c #131410",
"N. c #90950B",
"KX c #8B910C",
"^. c #90950D",
"J  c #53580C",
"0. c #6F7309",
"f. c #707252",
"7X c #1D1C1B",
"FX c #363636",
"=X c #6F730D",
"T. c #DAE10D",
"M. c #4E5108",
"-. c #4E5109",
"bX c #312306",
"*X c #393413",
"G. c #81860D",
"B  c #ECF40C",
";X c #656809",
"E. c #ECF40D",
"e. c #0C0D04",
"nX c #72510A",
"zX c #77550C",
"w  c #75ECEC",
"j. c #777B0B",
"n. c #3F420C",
"Q  c #020202",
"[. c #070604",
"P  c #3F4212",
"I. c #353707",
"b. c #C1C70D",
"#  c #598787",
"9X c #BD994F",
":. c #686C09",
"JX c #BCC30F",
"5  c #8E9090",
"LX c #898E1B",
" X c #0A0A04",
"v  c #E5ED0D",
"0  c #5D6F6F",
"N  c #68C8C8",
"). c #7F693E",
",  c #498E8D",
"!. c #DBE20D",
"x. c #2B2C17",
"u. c #DBE20E",
"<. c #D6DE0D",
"ZX c #D6DE0E",
"a. c #A3A90F",
",X c #322A1B",
"g  c #7BF7F7",
"#. c #2E3008",
"z  c #59B0AF",
"k  c #274444",
"^  c #CCD30E",
"2X c #030301",
"&. c #030302",
"5. c #ABB10D",
"S. c #E3EA0E",
"j  c #151C12",
"-  c #4C9292",
"%. c #302409",
"_. c #BE9A51",
"u  c #4F9C9B",
"F. c #4D4F08",
">X c #D4DB0D",
"VX c #7D5D2A",
"jX c #D4DB0F",
"=  c #949595",
"|. c #B3B90C",
"[  c #A37714",
";. c #C5CC0F",
"   c None",
"<  c #52A3A3",
"Y. c #71750C",
"R  c #A9AE0D",
"3  c #7CFBFB",
"h. c #D7DF0D",
" . c #DCE30E",
"g. c #0E0F02",
"vX c #D0920D",
"6. c #BBC10E",
"iX c #513A09",
"t. c #9A9F0C",
"sX c #464808",
"tX c #DFE70D",
".X c #DFE70E",
"B. c #373908",
"h  c #488C8C",
"o  c #686969",
"oX c #494C0A",
"i  c #7AF5F5",
"~. c #282A07",
"eX c #0C0C04",
"G  c #E7EF0D",
"/. c #60630B",
"n  c #020404",
"BX c #815C0C",
"1X c #725C31",
"!  c #C6CD0D",
"kX c #0C0C0C",
"7  c #68D0D0",
"qX c #515408",
"+X c #9A7C41",
"C  c #393939",
"4  c #6DCEC0",
"`  c #696A4F",
"DX c #141403",
".. c #84890D",
"$X c #7A6434",
"m  c #4E9796",
"&X c #262706",
"AX c #050501",
",. c #42450C",
"F  c #686B16",
"MX c #595C0A",
"wX c #987C42",
"X. c #595C0D",
"q. c #605A10",
"Z  c #595C0F",
"z. c #C4CA0F",
";  c #607878",
"e  c #63BDBD",
"SX c #9EA410",
"uX c #D4940C",
"9. c #7D820B",
"2. c #55514C",
"cX c #D4940D",
"L  c #D6DD12",
"/  c #080902",
"A  c #B5BB0D",
"q  c #6EDBDB",
"hX c #94990D",
"'  c #A57914",
"&  c #585E5E",
"GX c #553F1A",
"}  c #73770A",
"o. c #0D0D0D",
"+. c #A6AC0C",
"b  c #DEE50D",
"J. c #6E7312",
"Q. c #101102",
"-X c #BDC30E",
"7. c #101103",
"@X c #4E4D4D",
"v. c #676767",
"{  c #64680A",
"E  c #696C0C",
"%  c #6ED2D2",
"Y  c #484A0B",
"P. c #767B0A",
"0X c #7B7F0C",
"fX c #5A5D08",
"m. c #393B07",
"`. c #C0C70D",
"U. c #343708",
"y  c #67CECE",
"6X c #6C700C",
"D. c #5A5D14",
"]. c #C6A155",
"H. c #6C700F",
"5X c #6C7010",
"d. c #4B4E0A",
">  c #838585",
"=. c #2F3006",
"Z. c #7E7E7D",
"V  c #2A2C07",
"#X c #B1B80D",
"l. c #E9F10D",
"@. c #E9F110",
"8  c #55A9A8",
"rX c #3C3F07",
"R. c #2F300F",
"O  c #458180",
"NX c #3E3F1E",
"S  c #202106",
"@  c #7AF7F7",
"X  c #434C4C",
"pX c #4F390A",
":X c #65690D",
"K. c #D0D70F",
"*. c #D0D710",
"2  c #A4A4A4",
")  c #AFB50D",
"gX c #161613",
"p. c #070707",
"CX c #1B1404",
"]  c #CC9414",
"aX c #1B1406",
"f  c #4B8F8E",
"'. c #A0A60B",
"A. c #191A07",
"%X c #342B18",
":  c #468888",
"|  c #303105",
"W. c #646550",
"i. c #EAF20D",
"yX c #63660A",
"3X c #614F2A",
"XX c #EAF20E",
"U  c #EAF20F",
"s  c #9D9D9D",
/* pixels */
"                            . .             . X o               ",
"                          O + @ #         $ % + % .             ",
"                        & * + + % .     . % + + + =             ",
"                        - + + + + ;     : + + + + >             ",
"                        , + + + + <   . 1 + + + + 2             ",
"                    .   . 3 + 4 + + 5 6 + 4 + + 7 .   .         ",
"                          8 + 9 4 + 0 q w + e @ r   .           ",
"                          . + t y + u i p a + s     .           ",
"                        s d : + f g h j k l z                   ",
"              . x c c c v b c n m M N B B V c C .               ",
"            Z A S c D F G H c c c J K B L c c c P I             ",
"          c B U c c Y B T c c c R B B E W c Q ! B B I .         ",
"      . c W B ~ c c ^ B c c c / ( B B ) c c c _ B B B `         ",
". ' ] [ c { } c c | B  .c c c ..B B B X.c o.O.B B B +.@.#.      ",
"  . $.%.&.*.=.c c -.B ;.c c c :.B B B c c c >.B B B ,.B <.1.2.. ",
"      3.4.B c c c 5.B 6.c c 7.8.B B B c c c 9.B I 0.B B q.w..   ",
"      c _ B c c e.r.B t.c c y.u.B B i.c c p.a.B s.d.B B B f.    ",
"    . c B B c c g.h.B j.c c k.l.B B z.c c x.+.B B I I B +.c.v.  ",
"    . c B b.c c n.B B m.c c M.B B B N.c c B.V.B B B I I I C.Z.  ",
"    . A.B S.c c D.B B F.c c G.B B B H.c c J.K.B B B B B B L..   ",
"    . c B P.c c I.B B U.c c Y.B B T.4.c c R.E.B B B B B B W.    ",
"      Q.B !.c c ~.r.B ^.c c H B B C./.c c ,.(.B B B B B +..     ",
"    )._.`.'.].[.{.E.}.|.c c  X.XB XXoXc c OXB B B B B B /.      ",
"  ,.+X@X#X$X%Xc &XY.*X=Xc c ,.B B -X;Xc c :X>XB B B B K..       ",
". ,X,.,.<X1X2Xc 3X4XB B c c c 5XB B `.c D c 6XB B B B /.        ",
"7X8X.   9X0XqXc wXeXB B c c c rXtXB B c c c yXB B +./.          ",
",.uXiXpXaXsXdXc c c B B fXc c gXhXB jXkXc c lXl.+./.            ",
". zXxXcXvXbXnXmXc c B B MXc c c NXB B c c c c /..               ",
"      BXuXVXuXuXvXCXB B ZXAXc c c B B SXDXFX.                   ",
"      . GX. ,.uXuXHXB B B JXc c c KXB LX.                       ",
"            . ,.,..   . . . . . . .                             ",
"                                                                "
};

////@end XPM images



/*!
 * dlgNewVSCPSession type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dlgNewVSCPSession, wxDialog )


/*!
 * dlgNewVSCPSession event table definition
 */

BEGIN_EVENT_TABLE( dlgNewVSCPSession, wxDialog )

////@begin dlgNewVSCPSession event table entries
  EVT_INIT_DIALOG( dlgNewVSCPSession::OnInitDialog )
  EVT_WINDOW_DESTROY( dlgNewVSCPSession::OnDestroy )

  EVT_LISTBOX( ID_LISTBOX_INTERFACES, dlgNewVSCPSession::OnListboxInterfacesSelected )
  EVT_LISTBOX_DCLICK( ID_LISTBOX_INTERFACES, dlgNewVSCPSession::OnListboxInterfacesDoubleClicked )

  EVT_BUTTON( wxID_OK, dlgNewVSCPSession::OnOKClick )

  EVT_BUTTON( wxID_CANCEL, dlgNewVSCPSession::OnCANCELClick )

  EVT_BUTTON( ID_BUTTON_ADD, dlgNewVSCPSession::OnButtonAddClick )

  EVT_BUTTON( ID_BUTTON_EDIT, dlgNewVSCPSession::OnButtonEditClick )

  EVT_BUTTON( ID_BUTTON_REMOVE, dlgNewVSCPSession::OnButtonRemoveClick )

////@end dlgNewVSCPSession event table entries

END_EVENT_TABLE()


/*!
 * dlgNewVSCPSession constructors
 */

dlgNewVSCPSession::dlgNewVSCPSession()
{
  Init();
}

dlgNewVSCPSession::dlgNewVSCPSession( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
  Init();
  Create(parent, id, caption, pos, size, style);
}


/*!
 * DlgNewVSCPSession creator
 */

bool dlgNewVSCPSession::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dlgNewVSCPSession creation
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create( parent, id, caption, pos, size, style );

  CreateControls();
  SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));
  if (GetSizer())
  {
    GetSizer()->SetSizeHints(this);
  }
////@end dlgNewVSCPSession creation
  return true;
}


/*!
 * dlgNewVSCPSession destructor
 */

dlgNewVSCPSession::~dlgNewVSCPSession()
{
////@begin dlgNewVSCPSession destruction
////@end dlgNewVSCPSession destruction
}


/*!
 * Member initialisation
 */

void dlgNewVSCPSession::Init()
{
////@begin dlgNewVSCPSession member initialisation
  m_ctrlListInterfaces = NULL;
////@end dlgNewVSCPSession member initialisation

	m_bShowUnconnectedMode = true;
}


/*!
 * Control creation for DlgNewVSCPSession
 */

void dlgNewVSCPSession::CreateControls()
{    
////@begin dlgNewVSCPSession content construction
  dlgNewVSCPSession* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxArrayString m_ctrlListInterfacesStrings;
  m_ctrlListInterfaces = new wxListBox;
  m_ctrlListInterfaces->Create( itemDialog1, ID_LISTBOX_INTERFACES, wxDefaultPosition, wxSize(300, -1), m_ctrlListInterfacesStrings, wxLB_SINGLE );
  itemBoxSizer2->Add(m_ctrlListInterfaces, 0, wxGROW|wxALL, 5);

  wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  wxButton* itemButton5 = new wxButton;
  itemButton5->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton5->SetDefault();
  if (dlgNewVSCPSession::ShowToolTips())
    itemButton5->SetToolTip(_("Use selected interface"));
  itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton6 = new wxButton;
  itemButton6->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  if (dlgNewVSCPSession::ShowToolTips())
    itemButton6->SetToolTip(_("Go back without selecting any interface"));
  itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  itemBoxSizer4->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  wxButton* itemButton8 = new wxButton;
  itemButton8->Create( itemDialog1, ID_BUTTON_ADD, _("Add..."), wxDefaultPosition, wxDefaultSize, 0 );
  if (dlgNewVSCPSession::ShowToolTips())
    itemButton8->SetToolTip(_("Add new interface"));
  itemBoxSizer4->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton9 = new wxButton;
  itemButton9->Create( itemDialog1, ID_BUTTON_EDIT, _("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
  itemButton9->SetDefault();
  if (dlgNewVSCPSession::ShowToolTips())
    itemButton9->SetToolTip(_("Edit selected interface"));
  itemBoxSizer4->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  wxButton* itemButton10 = new wxButton;
  itemButton10->Create( itemDialog1, ID_BUTTON_REMOVE, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
  if (dlgNewVSCPSession::ShowToolTips())
    itemButton10->SetToolTip(_("Remove selected interface"));
  itemBoxSizer4->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

  // Connect events and objects
  itemDialog1->Connect(ID_DIALOG_NEW_VSCP_SESSION, wxEVT_DESTROY, wxWindowDestroyEventHandler(dlgNewVSCPSession::OnDestroy), NULL, this);
////@end dlgNewVSCPSession content construction
}


/*!
 * wxEVT_INIT_DIALOG event handler for ID_DIALOG_NEW_VSCP_SESSION
 */

void dlgNewVSCPSession::OnInitDialog( wxInitDialogEvent& event )
{
////@begin wxEVT_INIT_DIALOG event handler for ID_DIALOG_NEW_VSCP_SESSION in DlgNewVSCPSession.
  // Before editing this code, remove the block markers.
  event.Skip();
////@end wxEVT_INIT_DIALOG event handler for ID_DIALOG_NEW_VSCP_SESSION in DlgNewVSCPSession. 

  fillListBox( wxString(_("")) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_DESTROY event handler for ID_DIALOG_NEW_VSCP_SESSION
//

void dlgNewVSCPSession::OnDestroy( wxWindowDestroyEvent& event )
{
  event.Skip();
}



/*!
 * Should we show tooltips?
 */

bool dlgNewVSCPSession::ShowToolTips()
{
  return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dlgNewVSCPSession::GetBitmapResource( const wxString& name )
{
  // Bitmap retrieval
////@begin dlgNewVSCPSession bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
////@end dlgNewVSCPSession bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dlgNewVSCPSession::GetIconResource( const wxString& name )
{
  // Icon retrieval
////@begin dlgNewVSCPSession icon retrieval
  wxUnusedVar(name);
  if (name == _T("../../../docs/vscp/logo/fatbee_v2.ico"))
  {
    wxIcon icon(fatbee_v2_xpm);
    return icon;
  }
  return wxNullIcon;
////@end dlgNewVSCPSession icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void dlgNewVSCPSession::OnOKClick( wxCommandEvent& event )
{
  event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void dlgNewVSCPSession::OnCANCELClick( wxCommandEvent& event )
{
  cleanupListbox();
  event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ADD
 */

void dlgNewVSCPSession::OnButtonAddClick( wxCommandEvent& event )
{
  wxString strDecription;
  dlgVscpInterfaceSettings dlg( this );
  dlg.SetTitle(_("Add new VSCP/CANAL interface"));

  if ( wxID_OK == dlg.ShowModal() ) {

    // We add the driver to the structure
    if ( dlg.m_DriverDescription->GetValue().Length() ) {

      // A new CANAL driver
      canal_interface *pInfo = new canal_interface;
      if ( NULL != pInfo ) {
        strDecription = pInfo->m_strDescription = dlg.m_DriverDescription->GetValue();
        pInfo->m_strPath = dlg.m_PathToDriver->GetValue();
        pInfo->m_strConfig = dlg.m_DriverConfigurationString->GetValue();
        dlg.m_DriverFlags->GetValue().ToULong( &pInfo->m_flags ); 				
        g_Config.m_canalIfList.Append( pInfo );
	
				// Write the configuration
        ::wxGetApp().writeConfiguration();
				
      }
    }
    else if ( dlg.m_RemoteServerDescription->GetValue().Length() ) {

      // A new remote host
      vscp_interface *pInfo = new vscp_interface;
      if ( NULL != pInfo ) {
        strDecription = pInfo->m_strDescription = dlg.m_RemoteServerDescription->GetValue();
        pInfo->m_strHost = dlg.m_RemoteServerURL->GetValue();
				pInfo->m_strHost.Trim(false);
				pInfo->m_strHost.Trim();
        pInfo->m_strUser = dlg.m_RemoteServerUsername->GetValue();
				pInfo->m_strUser.Trim(false);
				pInfo->m_strUser.Trim();
        pInfo->m_strPassword = dlg.m_RemoteServerPassword->GetValue();
				pInfo->m_strPassword.Trim(false);
				pInfo->m_strPassword.Trim();
        dlg.m_RemoteServerPort->GetValue().ToULong( &pInfo->m_port );
        wxString str;
        str = dlg.m_GUID0->GetValue();	
				pInfo->m_GUID[ 0 ] = readStringValue( str ); 
				str = dlg.m_GUID1->GetValue();
				pInfo->m_GUID[ 1 ] = readStringValue( str ); 
				str = dlg.m_GUID2->GetValue();	
				pInfo->m_GUID[ 2 ] = readStringValue( str ); 
				str = dlg.m_GUID3->GetValue();	
				pInfo->m_GUID[ 3 ] = readStringValue( str ); 
				str = dlg.m_GUID4->GetValue();	
				pInfo->m_GUID[ 4 ] = readStringValue( str ); 
				str = dlg.m_GUID5->GetValue();	
				pInfo->m_GUID[ 5 ] = readStringValue( str ); 
				str = dlg.m_GUID6->GetValue();	
				pInfo->m_GUID[ 6 ] = readStringValue( str ); 
				str = dlg.m_GUID7->GetValue();	
				pInfo->m_GUID[ 7 ] = readStringValue( str ); 
				str = dlg.m_GUID8->GetValue();
				pInfo->m_GUID[ 8 ] = readStringValue( str ); 
				str = dlg.m_GUID9->GetValue();	
				pInfo->m_GUID[ 9 ] = readStringValue( str ); 
				str = dlg.m_GUID10->GetValue();
				pInfo->m_GUID[ 10 ] = readStringValue( str ); 
				str = dlg.m_GUID11->GetValue();	
				pInfo->m_GUID[ 11 ] = readStringValue( str ); 
				str = dlg.m_GUID12->GetValue();	
				pInfo->m_GUID[ 12 ] = readStringValue( str ); 
				str = dlg.m_GUID13->GetValue();
				pInfo->m_GUID[ 13 ] = readStringValue( str ); 
				str = dlg.m_GUID14->GetValue();
				pInfo->m_GUID[ 14 ] = readStringValue( str ); 
				str = dlg.m_GUID15->GetValue();
				pInfo->m_GUID[ 15 ] = readStringValue( str );

				memcpy( &pInfo->m_vscpfilter, &dlg.m_vscpfilter, sizeof( vscpEventFilter ) );

        g_Config.m_vscpIfList.Append( pInfo );
	
				// Write the configuration
        ::wxGetApp().writeConfiguration();
				
      }
    }
    else {
      wxMessageBox(_("No driver added as a needed description is not found."));
    }
    
    fillListBox( strDecription );
    
    
  }
  event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_EDIT
 */

void dlgNewVSCPSession::OnButtonEditClick( wxCommandEvent& event )
{
  int selidx = -1;
  
  if ( wxNOT_FOUND != ( selidx = m_ctrlListInterfaces->GetSelection() ) ) {

		if ( ( 0 == selidx ) && m_bShowUnconnectedMode ) {
			wxMessageBox(_("Can't edit this line."));
		}
		else {

    	both_interface *pBoth = (both_interface *)m_ctrlListInterfaces->GetClientData( selidx );
    	if ( NULL != pBoth ) {
      
      	dlgVscpInterfaceSettings dlg( this );
      
      	if ( INTERFACE_CANAL == pBoth->m_type ) {
        	dlg.SetTitle(_("Edit VSCP interface"));
        	dlg.m_DriverDescription->SetValue( pBoth->m_pcanalif->m_strDescription );
        	dlg.m_PathToDriver->SetValue( pBoth->m_pcanalif->m_strPath );
        	dlg.m_DriverConfigurationString->SetValue( pBoth->m_pcanalif->m_strConfig );
        	dlg.m_DriverFlags->SetValue( wxString::Format( _("%lu"), pBoth->m_pcanalif->m_flags ) );
          dlg.GetBookCtrl()->SetSelection( 0 );
        	dlg.GetBookCtrl()->RemovePage( 1 );
        	
      	}
      	else if ( INTERFACE_VSCP == pBoth->m_type ) {
        	dlg.m_RemoteServerDescription->SetValue( pBoth->m_pvscpif->m_strDescription );
        	dlg.m_RemoteServerURL->SetValue( pBoth->m_pvscpif->m_strHost );
        	dlg.m_RemoteServerUsername->SetValue( pBoth->m_pvscpif->m_strUser );
        	dlg.m_RemoteServerPassword->SetValue( pBoth->m_pvscpif->m_strPassword );
        	dlg.m_RemoteServerPort->SetValue( wxString::Format( _("%lu"), pBoth->m_pvscpif->m_port ) );
          dlg.m_GUID0->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 0 ] ) );
					dlg.m_GUID1->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 1 ] ) );
					dlg.m_GUID2->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 2 ] ) );
					dlg.m_GUID3->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 3 ] ) );
					dlg.m_GUID4->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 4 ] ) );
					dlg.m_GUID5->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 5 ] ) );
					dlg.m_GUID6->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 6 ] ) );
					dlg.m_GUID7->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 7 ] ) );
					dlg.m_GUID8->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 8 ] ) );
					dlg.m_GUID9->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 9 ] ) );
					dlg.m_GUID10->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 10 ] ) );
					dlg.m_GUID11->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 11 ] ) );
					dlg.m_GUID12->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 12 ] ) );
					dlg.m_GUID13->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 13 ] ) );
					dlg.m_GUID14->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 14 ] ) );
					dlg.m_GUID15->SetValue( wxString::Format( _("0x%02X"), pBoth->m_pvscpif->m_GUID[ 15 ] ) );
					memcpy( &dlg.m_vscpfilter, &pBoth->m_pvscpif->m_vscpfilter, sizeof( vscpEventFilter ) );
          dlg.GetBookCtrl()->SetSelection( 1 );
        	dlg.GetBookCtrl()->RemovePage( 0 );
      	}
      	else {
        	wxMessageBox(_("Unknown interface type!"), _("Edit Interface"), wxICON_STOP);
        	return;
      	}
      
      	if ( wxID_OK == dlg.ShowModal() ) {
      
        	if ( INTERFACE_CANAL == pBoth->m_type ) {
          	pBoth->m_pcanalif->m_strDescription = dlg.m_DriverDescription->GetValue();
          	pBoth->m_pcanalif->m_strPath = dlg.m_PathToDriver->GetValue();
          	pBoth->m_pcanalif->m_strConfig = dlg.m_DriverConfigurationString->GetValue();
          	dlg.m_DriverFlags->GetValue().ToULong( &pBoth->m_pcanalif->m_flags );
        	}
        	else if ( INTERFACE_VSCP == pBoth->m_type ) {
          	pBoth->m_pvscpif->m_strDescription = dlg.m_RemoteServerDescription->GetValue();
          	pBoth->m_pvscpif->m_strHost = dlg.m_RemoteServerURL->GetValue();
          	pBoth->m_pvscpif->m_strUser = dlg.m_RemoteServerUsername->GetValue();
          	pBoth->m_pvscpif->m_strPassword = dlg.m_RemoteServerPassword->GetValue();
          	dlg.m_RemoteServerPort->GetValue().ToULong( &pBoth->m_pvscpif->m_port );
            pBoth->m_pvscpif->m_GUID[0] = readStringValue( dlg.m_GUID0->GetValue() );
            pBoth->m_pvscpif->m_GUID[1] = readStringValue( dlg.m_GUID1->GetValue() );
            pBoth->m_pvscpif->m_GUID[2] = readStringValue( dlg.m_GUID2->GetValue() );
            pBoth->m_pvscpif->m_GUID[3] = readStringValue( dlg.m_GUID3->GetValue() );
            pBoth->m_pvscpif->m_GUID[4] = readStringValue( dlg.m_GUID4->GetValue() );
            pBoth->m_pvscpif->m_GUID[5] = readStringValue( dlg.m_GUID5->GetValue() );
            pBoth->m_pvscpif->m_GUID[6] = readStringValue( dlg.m_GUID6->GetValue() );
            pBoth->m_pvscpif->m_GUID[7] = readStringValue( dlg.m_GUID7->GetValue() );
            pBoth->m_pvscpif->m_GUID[8] = readStringValue( dlg.m_GUID8->GetValue() );
            pBoth->m_pvscpif->m_GUID[9] = readStringValue( dlg.m_GUID9->GetValue() );
            pBoth->m_pvscpif->m_GUID[10] = readStringValue( dlg.m_GUID10->GetValue() );
            pBoth->m_pvscpif->m_GUID[11] = readStringValue( dlg.m_GUID11->GetValue() );
            pBoth->m_pvscpif->m_GUID[12] = readStringValue( dlg.m_GUID12->GetValue() );
            pBoth->m_pvscpif->m_GUID[13] = readStringValue( dlg.m_GUID13->GetValue() );
            pBoth->m_pvscpif->m_GUID[14] = readStringValue( dlg.m_GUID14->GetValue() );
            pBoth->m_pvscpif->m_GUID[15] = readStringValue( dlg.m_GUID15->GetValue() );
						memcpy( &pBoth->m_pvscpif->m_vscpfilter, &dlg.m_vscpfilter, sizeof( vscpEventFilter ) );
        	}
  
					// Write the configuration
        	::wxGetApp().writeConfiguration();
					
        	fillListBox( wxString(_("")) );
        
					m_ctrlListInterfaces->SetSelection( selidx );
      	}
    	}
    	else {
      	wxMessageBox(_("No data associated with listbox line"), _("Edit Interface"), wxICON_STOP );
    	}
		} // 0 == selidx
  } 
  else {
    wxMessageBox(_("You need to select one interface to edit before this operation can be performed."),
                  _("Edit interface"),
                  wxICON_INFORMATION );
  }
  
  event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_REMOVE
 */

void dlgNewVSCPSession::OnButtonRemoveClick( wxCommandEvent& event )
{
  int selidx = -1;
  if ( wxNOT_FOUND != (selidx = m_ctrlListInterfaces->GetSelection() ) ) {

		if ( ( 0 == selidx ) && m_bShowUnconnectedMode ) {
			wxMessageBox(_("Can't remove this line."));
		}
		else {
    	if ( wxYES == wxMessageBox( _("Do you really want to remove interface?"),
      	                            _("Confirm"),
        	                          wxYES_NO | wxCANCEL ) ) {
      	both_interface *pBoth = (both_interface *)m_ctrlListInterfaces->GetClientData( selidx );
      	if ( NULL != pBoth ) {
        	if ( ( INTERFACE_CANAL == pBoth->m_type ) && ( NULL != pBoth->m_pcanalif ) ) {
          	if ( g_Config.m_canalIfList.DeleteObject( pBoth->m_pcanalif ) ) {
            	delete pBoth->m_pcanalif;
            	fillListBox( wxString(_("")) );
            	::wxGetApp().writeConfiguration();
          	}
        	}
        	else if ( ( INTERFACE_VSCP == pBoth->m_type )  && ( NULL != pBoth->m_pvscpif ) ) {
          	if ( g_Config.m_vscpIfList.DeleteObject( pBoth->m_pvscpif ) ) {
            	delete pBoth->m_pvscpif;
            	fillListBox( wxString(_("")));
            	::wxGetApp().writeConfiguration();
          	}
        	}
      	}
			}
    }
  } 
  else {
    wxMessageBox(_("You need to select one interface to remove before this operation can be performed."),
                  _("Remove interface"),
                  wxICON_INFORMATION );
  }
  
  event.Skip();
}

//////////////////////////////////////////////////////////////////////////////
// fillListBox
//

void dlgNewVSCPSession::fillListBox( const wxString& strDecription )
{
  cleanupListbox();
  
	if ( m_bShowUnconnectedMode ) {
		both_interface *pBoth = new both_interface; // Dummy for unconnected mode
		wxASSERT( NULL != pBoth );
		pBoth->m_pcanalif = NULL;
		pBoth->m_pvscpif = NULL;
		m_ctrlListInterfaces->Append( _("Unconnected Mode"), pBoth );
	}
  
  // Start with CANAL 
  LIST_CANAL_IF::iterator iterCanal;
  for (iterCanal = g_Config.m_canalIfList.begin(); iterCanal != g_Config.m_canalIfList.end(); ++iterCanal) {
    canal_interface *pIf = *iterCanal;
    both_interface *pBoth = new both_interface;
    if ( NULL != pBoth ) {
      pBoth->m_type = INTERFACE_CANAL;
      pBoth->m_pcanalif = pIf;
      m_ctrlListInterfaces->Append( _("CANAL: ") + pIf->m_strDescription, pBoth );
    }
  }
  
  // VSCP TCP/IP interface
  LIST_VSCP_IF::iterator iterTcpip;
  for (iterTcpip = g_Config.m_vscpIfList.begin(); iterTcpip != g_Config.m_vscpIfList.end(); ++iterTcpip) {
    vscp_interface *pIf = *iterTcpip;
    both_interface *pBoth = new both_interface;
    if ( NULL != pBoth ) {
      pBoth->m_type = INTERFACE_VSCP;
      pBoth->m_pvscpif = pIf;
      m_ctrlListInterfaces->Append( _("TCP/IP: ") + pIf->m_strDescription, pBoth );
    }
  }
  
  if ( !strDecription.IsEmpty() ) {
    if ( !m_ctrlListInterfaces->SetStringSelection( _("CANAL: ") + strDecription ) ) {
      m_ctrlListInterfaces->SetStringSelection( _("TCP/IP: ") + strDecription );
    }
  }
  else {
    // Select first item in list if items are available
    if ( m_ctrlListInterfaces->GetCount() ) {
      m_ctrlListInterfaces->Select( 0 );
    }
  }
  
}


//////////////////////////////////////////////////////////////////////////////
// cleanupListbox
//

void dlgNewVSCPSession::cleanupListbox( void )
{
	int n;

	// Must be something in the lixtbic
	if ( 0 == ( n = m_ctrlListInterfaces->GetCount() ) ) return;

  for ( unsigned int i=0; i<m_ctrlListInterfaces->GetCount(); i++ ) {
    both_interface *pBoth = (both_interface *)m_ctrlListInterfaces->GetClientData( i );
    if ( NULL != pBoth ) delete pBoth;
  }

  // Clear the list
  m_ctrlListInterfaces->Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX_INTERFACES
//

void dlgNewVSCPSession::OnListboxInterfacesSelected( wxCommandEvent& event )
{

  event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX_INTERFACES
//

void dlgNewVSCPSession::OnListboxInterfacesDoubleClicked( wxCommandEvent& WXUNUSED( event ) )
{ 
  EndModal( wxID_OK );
  return;
}




