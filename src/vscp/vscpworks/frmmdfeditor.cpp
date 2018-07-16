/////////////////////////////////////////////////////////////////////////////
// Name:        frmmdfeditor.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     09/04/2009 11:05:11
// RCS-ID:      
// Copyright:   (C) 2009-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://can.sourceforge.net) 
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

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmmdfeditor.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/imaglist.h"

#include <wx/stdpaths.h>
#include "frmmdfheader_images.h"
#include "frmmdfeditor.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmMDFEditor type definition
//

IMPLEMENT_CLASS( frmMDFEditor, wxFrame )


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmMDFEditor event table definition
//

BEGIN_EVENT_TABLE( frmMDFEditor, wxFrame )

EVT_MENU( ID_MENUITEM_EXIT, frmMDFEditor::OnMenuitemExitClick )
EVT_MENU( ID_TOOL_NEW, frmMDFEditor::OnToolNewClick )
EVT_MENU( ID_TOOL_LOAD, frmMDFEditor::OnToolLoadClick )
EVT_MENU( ID_TOOL_SAVE, frmMDFEditor::OnToolSaveClick )
EVT_MENU( ID_TOOL_DOWNLOAD, frmMDFEditor::OnToolDownloadClick )
EVT_MENU( ID_TOOL_REMOVE_ITEM, frmMDFEditor::OnToolRemoveItemClick )
EVT_TREE_SEL_CHANGED( ID_TREECTRL, frmMDFEditor::OnTreectrlSelChanged )

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmMDFEditor constructors
//

frmMDFEditor::frmMDFEditor()
{
    Init();
}

frmMDFEditor::frmMDFEditor( wxWindow* parent, 
                                wxWindowID id, 
                                const wxString& caption, 
                                const wxPoint& pos, 
                                const wxSize& size, 
                                long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmMDFEditor creator
//

bool frmMDFEditor::Create( wxWindow* parent, 
                                wxWindowID id, 
                                const wxString& caption, 
                                const wxPoint& pos, 
                                const wxSize& size, 
                                long style )
{
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// frmMDFEditor destructor
//

frmMDFEditor::~frmMDFEditor()
{

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisation
//

void frmMDFEditor::Init()
{
    m_mdfTree = NULL;
    m_htmlInfoWnd = NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control creation for frmMDFEditor
//

void frmMDFEditor::CreateControls()
{
    frmMDFEditor* itemFrame = this;

    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* itemMenu3 = new wxMenu;
    itemMenu3->Append( ID_MENUITEM_NEW, _( "Create New MDF File" ), wxEmptyString, wxITEM_NORMAL );
    itemMenu3->AppendSeparator();
    itemMenu3->Append( ID_MENUITEM_OPEN, _( "Open MDF file..." ), wxEmptyString, wxITEM_NORMAL );
    itemMenu3->Append( ID_MENUITEM_SAVE, _( "Save MDF File..." ), wxEmptyString, wxITEM_NORMAL );
    itemMenu3->AppendSeparator();
    itemMenu3->Append( ID_MENUITEM_DOWNLOAD, _( "Download MDF file..." ), wxEmptyString, wxITEM_NORMAL );
    itemMenu3->Append( ID_MENUITEM_UPLOAD, _( "Upload MDF file..." ), wxEmptyString, wxITEM_NORMAL );
    itemMenu3->AppendSeparator();
    itemMenu3->Append( ID_MENUITEM_EXIT, _( "Exit" ), wxEmptyString, wxITEM_NORMAL );
    menuBar->Append( itemMenu3, _( "File" ) );
    wxMenu* itemMenu13 = new wxMenu;
    itemMenu13->Append( ID_MENUITEM_ADD, _( "Add MDF Item..." ), wxEmptyString, wxITEM_NORMAL );
    itemMenu13->Append( ID_MENUITEM_REMOVE, _( "Remove MDF Item..." ), wxEmptyString, wxITEM_NORMAL );
    menuBar->Append( itemMenu13, _( "Edit" ) );
    itemFrame->SetMenuBar( menuBar );

    wxPanel* itemPanel = new wxPanel;
    itemPanel->Create( itemFrame, 
                            ID_PANEL, 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxSUNKEN_BORDER | wxTAB_TRAVERSAL );

    wxBoxSizer* itemBoxSizer = new wxBoxSizer( wxVERTICAL );
    itemPanel->SetSizer( itemBoxSizer );

    wxToolBar* itemToolBar = new wxToolBar;
    itemToolBar->Create( itemPanel, 
                            ID_TOOLBAR3, 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxTB_FLAT | wxTB_HORIZONTAL );
    wxBitmap itemtool19Bitmap( itemFrame->GetBitmapResource( wxT( "deffile.xpm" ) ) );
    wxBitmap itemtool19BitmapDisabled;
    itemToolBar->AddTool( ID_TOOL_NEW, 
                                wxEmptyString, 
                                itemtool19Bitmap, 
                                itemtool19BitmapDisabled, 
                                wxITEM_NORMAL, 
                                _( "Create new MDF file" ), 
                                wxEmptyString );
    wxBitmap itemtool20Bitmap( itemFrame->GetBitmapResource( wxT( "open.xpm" ) ) );
    wxBitmap itemtool20BitmapDisabled;
    itemToolBar->AddTool( ID_TOOL_LOAD, 
                                wxEmptyString, 
                                itemtool20Bitmap, 
                                itemtool20BitmapDisabled, 
                                wxITEM_NORMAL, 
                                _( "Load MDF file" ), 
                                wxEmptyString );
    wxBitmap itemtool21Bitmap( itemFrame->GetBitmapResource( wxT( "save.xpm" ) ) );
    wxBitmap itemtool21BitmapDisabled;
    itemToolBar->AddTool( ID_TOOL_SAVE, 
                                wxEmptyString, 
                                itemtool21Bitmap, 
                                itemtool21BitmapDisabled, 
                                wxITEM_NORMAL, 
                                _( "Save MDF file" ), 
                                wxEmptyString );
    wxBitmap itemtool22Bitmap( itemFrame->GetBitmapResource( wxT( "filesaveas.xpm" ) ) );
    wxBitmap itemtool22BitmapDisabled;
    itemToolBar->AddTool( ID_TOOL_UPLOAD, 
                                wxEmptyString, 
                                itemtool22Bitmap, 
                                itemtool22BitmapDisabled, 
                                wxITEM_NORMAL, 
                                _( "Upload MDF file" ), 
                                wxEmptyString );
    wxBitmap itemtool23Bitmap( itemFrame->GetBitmapResource( wxT( "redo.xpm" ) ) );
    wxBitmap itemtool23BitmapDisabled;
    itemToolBar->AddTool( ID_TOOL_DOWNLOAD, 
                                wxEmptyString, 
                                itemtool23Bitmap, 
                                itemtool23BitmapDisabled, 
                                wxITEM_NORMAL, 
                                _( "Upload MDF file" ), 
                                wxEmptyString );
    itemToolBar->AddSeparator();
    wxBitmap itemtool25Bitmap( itemFrame->GetBitmapResource( wxT( "New1.xpm" ) ) );
    wxBitmap itemtool25BitmapDisabled;
    itemToolBar->AddTool( ID_TOOL_ADD_ITEM, 
                                wxEmptyString, 
                                itemtool25Bitmap, 
                                itemtool25BitmapDisabled, 
                                wxITEM_NORMAL, 
                                _( "Add MDF item" ), 
                                wxEmptyString );
    wxBitmap itemtool26Bitmap( itemFrame->GetBitmapResource( wxT( "delete.xpm" ) ) );
    wxBitmap itemtool26BitmapDisabled;
    itemToolBar->AddTool( ID_TOOL_REMOVE_ITEM, 
                                wxEmptyString, 
                                itemtool26Bitmap, 
                                itemtool26BitmapDisabled, 
                                wxITEM_NORMAL, 
                                _( "Remove MDF item" ), 
                                wxEmptyString );
    itemToolBar->Realize();
    itemBoxSizer->Add( itemToolBar, 0, wxGROW | wxALL, 5 );

    m_mdfTree = new wxTreeCtrl;
    m_mdfTree->Create( itemPanel, 
                            ID_TREECTRL, 
                            wxDefaultPosition, 
                            wxSize( 400, 300 ), 
                            wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT | wxTR_LINES_AT_ROOT | wxTR_ROW_LINES | wxTR_SINGLE );
    itemBoxSizer->Add( m_mdfTree, 0, wxGROW | wxALL, 2 );

    m_htmlInfoWnd = new wxHtmlWindow;
    m_htmlInfoWnd->Create( itemPanel, 
                                ID_HTMLWINDOW2, 
                                wxDefaultPosition, 
                                wxSize( 200, 150 ), 
                                wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL );
    m_htmlInfoWnd->SetPage( _( "<html><h4>No data</h4></html>" ) );
    itemBoxSizer->Add( m_htmlInfoWnd, 0, wxGROW | wxALL, 5 );

    // Connect events and objects
    m_mdfTree->Connect( ID_TREECTRL, 
                            wxEVT_LEFT_DOWN, 
                            wxMouseEventHandler( frmMDFEditor::OnLeftDown ), 
                            NULL, 
                            this );
    m_mdfTree->Connect( ID_TREECTRL, 
                            wxEVT_LEFT_DCLICK, 
                            wxMouseEventHandler( frmMDFEditor::OnLeftDClick ), 
                            NULL, 
                            this );

    wxImageList* itemImageList = new wxImageList( 16, 16, true, 5 );
    {
        wxImage icon0( Home_xpm );
        icon0.Rescale( 16, 16 );
        itemImageList->Add( icon0 );
        
        wxImage icon1( Folder_Add_xpm );
        icon1.Rescale( 16, 16 );
        itemImageList->Add( icon1 );
        
        wxImage icon2( Info_xpm );
        icon2.Rescale( 16, 16 );
        itemImageList->Add( icon2 );
        
        wxImage icon3( copy_xpm );
        icon3.Rescale( 16, 16 );
        itemImageList->Add( icon3 );
        
        wxImage icon4( copy_xpm );
        icon4.Rescale( 16, 16 );
        itemImageList->Add( icon4 );
    }

    m_mdfTree->AssignImageList( itemImageList );
    addDefaultContent();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Should we show tooltips?
//

bool frmMDFEditor::ShowToolTips()
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get bitmap resources
//

wxBitmap frmMDFEditor::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar( name );
    if ( name == _T( "deffile.xpm" ) )
    {
        wxBitmap bitmap( deffile_xpm );
        return bitmap;
    }
    else if ( name == _T( "open.xpm" ) )
    {
        wxBitmap bitmap( open_xpm );
        return bitmap;
    }
    else if ( name == _T( "save.xpm" ) )
    {
        wxBitmap bitmap( save_xpm );
        return bitmap;
    }
    else if ( name == _T( "filesaveas.xpm" ) )
    {
        wxBitmap bitmap( filesaveas_xpm );
        return bitmap;
    }
    else if ( name == _T( "redo.xpm" ) )
    {
        wxBitmap bitmap( redo_xpm );
        return bitmap;
    }
    else if ( name == _T( "New1.xpm" ) )
    {
        wxBitmap bitmap( new_xpm );
        return bitmap;
    }
    else if ( name == _T( "delete.xpm" ) )
    {
        wxBitmap bitmap( delete_xpm );
        return bitmap;
    }
    return wxNullBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get icon resources
//

wxIcon frmMDFEditor::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar( name );
    return wxNullIcon;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_NEW
//

void frmMDFEditor::OnToolNewClick( wxCommandEvent& event )
{
    // Before editing this code, remove the block markers.
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_LOAD
//

void frmMDFEditor::OnToolLoadClick( wxCommandEvent& event )
{
    wxString localMDF;
    //wxStandardPaths stdpaths;

    // Load MDF from local file
    wxFileDialog dlg( this,
                      _( "Choose file to load MDF from " ),
                      wxStandardPaths::Get().GetUserDataDir(),
                      _( "" ),
                      _( "MSF Files (*.mdf)|*.mdf|XML Files (*.xml)|*.xml|All files (*.*)|*.*" ) );
    if ( wxID_OK == dlg.ShowModal() ) {

        localMDF = dlg.GetPath();

        if ( m_mdfTree->GetCount() ) {

            if ( wxYES != wxMessageBox( _( "Should the old MDF data be removed and lost" ),
                                        _( "Delete MDF data?" ),
                                        wxYES_NO | wxCANCEL ) ) {
                event.Skip( false );
                return;
            }

            m_mdfTree->DeleteAllItems();

        }

        if ( !m_mdf.parseMDF( localMDF ) ) {
            wxMessageBox( _( "Failed to parse MDF." ) );
            event.Skip( false );
            return;
        }



        // Fill up the TreeControl with data from the MDF

    }
    else {
        event.Skip( false );
        return;
    }


    event.Skip( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_SAVE
//

void frmMDFEditor::OnToolSaveClick( wxCommandEvent& event )
{

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_DOWNLOAD
//

void frmMDFEditor::OnToolDownloadClick( wxCommandEvent& event )
{

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_TOOL_REMOVE_ITEM
//

void frmMDFEditor::OnToolRemoveItemClick( wxCommandEvent& event )
{

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_TREE_SEL_CHANGED event handler for ID_TREECTRL
//

void frmMDFEditor::OnTreectrlSelChanged( wxTreeEvent& event )
{
    wxString strPage;
    strPage = _( "<html><body><h3>" );
    wxTreeItemId itemID = event.GetItem();
    //MyTreeItemData *item = itemId.IsOk() ? (MyTreeItemData *)GetItemData(itemId)
    //                                     : NULL;
    if ( itemID.IsOk() ) {
        //strPage += m_mdfTree->GetItemText( itemID );
        //strPage += event.GetLabel();
    }
    else {
        strPage += _( "????" );
    }
    //strPage += _( "</h3></body></html>" );
    //m_htmlInfoWnd->SetPage( strPage );
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_LEFT_DOWN event handler for ID_TREECTRL
//

void frmMDFEditor::OnLeftDown( wxMouseEvent& event )
{

    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_LEFT_DCLICK event handler for ID_TREECTRL
//

void frmMDFEditor::OnLeftDClick( wxMouseEvent& event )
{

    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// addDefaultContent
//

void frmMDFEditor::addDefaultContent( void )
{
    m_rootItem = m_mdfTree->AddRoot( _( "mdf" ), MDF_EDITOR_TOP_ITEM );
    m_moduleItem = m_mdfTree->AppendItem( m_rootItem, _( "module" ), MDF_EDITOR_MAIN_ITEM );

    m_mdfTree->AppendItem( m_moduleItem, _( "Name" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Model" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Version" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Description" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "URL for full module information" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Last date changed" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Buffersize" ), MDF_EDITOR_SUB_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Location of MDF file" ), MDF_EDITOR_SUB_ITEM );

    m_mdfTree->AppendItem( m_moduleItem, _( "Manufacturer" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Driver" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Abstractions" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Registers" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Decision Matrix" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Events" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Alarm" ), MDF_EDITOR_MAIN_ITEM );
    m_mdfTree->AppendItem( m_moduleItem, _( "Boot" ), MDF_EDITOR_MAIN_ITEM );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_EXIT
//

void frmMDFEditor::OnMenuitemExitClick( wxCommandEvent& event )
{
    Close();
    event.Skip();
}

