/////////////////////////////////////////////////////////////////////////////
// Name:        dlgsimulatedvscpnode.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 10 Nov 2007 17:27:17 CET
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/init.h>  // For wxGetApp

////@begin includes
////@end includes

#include "dlgsimulatedvscpnode.h"
#include "../../common/vscp.h"
#include "../../common/canal.h"
#include "../../common/vscphelper.h"

// Externals
extern unsigned char *g_registers;
extern vscpEventEx g_eventPeriodic;
extern vscpEventEx g_eventOne;
extern vscpEventEx g_eventTwo;
extern vscpEventEx g_eventThree;

////@begin XPM images
////@end XPM images


/*!
 * DlgSimulatedVSCPNode type definition
 */

IMPLEMENT_DYNAMIC_CLASS( DlgSimulatedVSCPNode, wxDialog )


/*!
 * DlgSimulatedVSCPNode event table definition
 */

BEGIN_EVENT_TABLE( DlgSimulatedVSCPNode, wxDialog )

////@begin DlgSimulatedVSCPNode event table entries
////@end DlgSimulatedVSCPNode event table entries

END_EVENT_TABLE()


/*!
 * DlgSimulatedVSCPNode constructors
 */

DlgSimulatedVSCPNode::DlgSimulatedVSCPNode()
{
    Init();
}

DlgSimulatedVSCPNode::DlgSimulatedVSCPNode( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * DlgSimulatedVSCPNode creator
 */

bool DlgSimulatedVSCPNode::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin DlgSimulatedVSCPNode creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end DlgSimulatedVSCPNode creation
    return true;
}


/*!
 * DlgSimulatedVSCPNode destructor
 */

DlgSimulatedVSCPNode::~DlgSimulatedVSCPNode()
{
////@begin DlgSimulatedVSCPNode destruction
////@end DlgSimulatedVSCPNode destruction
}


/*!
 * Member initialisation
 */

void DlgSimulatedVSCPNode::Init()
{
////@begin DlgSimulatedVSCPNode member initialisation
    m_gridRegisters = NULL;
////@end DlgSimulatedVSCPNode member initialisation
}


/*!
 * Control creation for DlgSimulatedVSCPNode
 */

void DlgSimulatedVSCPNode::CreateControls()
{    
////@begin DlgSimulatedVSCPNode content construction
    DlgSimulatedVSCPNode* itemDialog1 = this;

    wxPanel* itemPanel2 = new wxPanel;
    itemPanel2->Create( itemDialog1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer3);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText;
    itemStaticText6->Create( itemPanel2, wxID_STATIC, _("Registers"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText6->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Sans")));
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_gridRegisters = new wxGrid;
    m_gridRegisters->Create( itemPanel2, ID_REG_GRID, wxDefaultPosition, wxSize(300, 330), wxHSCROLL|wxVSCROLL );
    m_gridRegisters->SetForegroundColour(wxColour(37, 208, 25));
    m_gridRegisters->SetBackgroundColour(wxColour(255, 255, 0));
    m_gridRegisters->SetDefaultColSize(50);
    m_gridRegisters->SetDefaultRowSize(25);
    m_gridRegisters->SetColLabelSize(25);
    m_gridRegisters->SetRowLabelSize(50);
    m_gridRegisters->CreateGrid(256, 1, wxGrid::wxGridSelectRows);
    itemBoxSizer5->Add(m_gridRegisters, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer8, 0, wxALIGN_TOP|wxALL, 5);

    wxToggleButton* itemToggleButton9 = new wxToggleButton;
    itemToggleButton9->Create( itemPanel2, ID_TOGGLEBUTTON_CONNECT, _("Connect"), wxDefaultPosition, wxSize(120, 50), 0 );
    itemToggleButton9->SetValue(false);
    itemToggleButton9->SetBackgroundColour(wxColour(144, 238, 144));
    itemBoxSizer8->Add(itemToggleButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer8->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton;
    itemButton11->Create( itemPanel2, ID_BUTTON_RESTORE_DEFAULTS, _("Restore defaults"), wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer8->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer8->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton;
    itemButton13->Create( itemPanel2, ID_BUTTON_SAVE_EVENTS, _("Save..."), wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer8->Add(itemButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxButton* itemButton14 = new wxButton;
    itemButton14->Create( itemPanel2, ID_BUTTON_LOAD_EVENS, _("Load..."), wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer8->Add(itemButton14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    itemBoxSizer8->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton;
    itemButton16->Create( itemPanel2, ID_BUTTON_SEND_EVENT1, _("Send Event 1"), wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer8->Add(itemButton16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxButton* itemButton17 = new wxButton;
    itemButton17->Create( itemPanel2, ID_BUTTON_SEND_EVENT2, _("Send Event 2"), wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer8->Add(itemButton17, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxButton* itemButton18 = new wxButton;
    itemButton18->Create( itemPanel2, ID_BUTTON_SEND_EVENT3, _("Send Event 3"), wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer8->Add(itemButton18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    itemBoxSizer8->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton;
    itemButton20->Create( itemPanel2, ID_BUTTON_SETTINGS, _("Settings"), wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer8->Add(itemButton20, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end DlgSimulatedVSCPNode content construction

    m_gridRegisters->SetRowLabelSize( 110 );
    m_gridRegisters->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_CENTRE );
    
    m_gridRegisters->SetColSize( 0, 150 );
    m_gridRegisters->SetColLabelValue( 0, _("Register Content") );
    
    wxString strBuf;
    for ( int i=0; i<256; i++ ) {
    
      strBuf.Printf( _("Register %d"), i );
      m_gridRegisters->SetRowLabelValue( i, strBuf );
      
      strBuf.Printf( _("0x%02lx"), i );
      m_gridRegisters->SetCellValue( i, 0,  strBuf );
      m_gridRegisters->SetCellTextColour( i, 0, *wxBLUE );
      m_gridRegisters->SetCellAlignment( wxALIGN_CENTRE, i, 0 );
      //m_gridRegisters->SetReadOnly( m_gridRegisters0->GetNumberRows()-1, 2 );
      m_gridRegisters->SelectRow( i );
      m_gridRegisters->MakeCellVisible( i, 0 );
      m_gridRegisters->Update();
    }
    
    setDefaultRegisters();
    
}

/////////////////////////////////////////////////////////////////////
// setDefaultRegisters
//

void DlgSimulatedVSCPNode::setDefaultRegisters()
{
  g_registers[ 0 ] = 0;    // Zone
  g_registers[ 1 ] = 0;    // Subzone
  g_registers[ 2 ] = 30;   // Interval for event-4 (seconds)
  
  // Alarm
  g_registers[ 128 ] = 0;
  
  // VSCP Version
  g_registers[ 129 ] = 1;
  g_registers[ 130 ] = 5;
  
  // Node Control Flags
  g_registers[ 131 ] = 0;
  
  // Client settable node id
  g_registers[ 132 ] = 1;
  g_registers[ 133 ] = 10;
  g_registers[ 134 ] = 1;
  g_registers[ 135 ] = 22;
  g_registers[ 136 ] = 8;
  
  // Manufacturer device ID
  g_registers[ 137 ] = 8;
  g_registers[ 138 ] = 16;
  g_registers[ 139 ] = 24;
  g_registers[ 140 ] = 32;
  
  // Manufacturer sub device ID
  g_registers[ 141 ] = 1;
  g_registers[ 142 ] = 2;
  g_registers[ 143 ] = 4;
  g_registers[ 144 ] = 8;
  
  //Nickname id for node if assigned or 0xff if no nickname id assigned.
  g_registers[ 145 ] = 0xff;
  
  // Page select registers
  g_registers[ 146 ] = 0xff; // MSB
  g_registers[ 147 ] = 0xff; // LSB
  
  // Firmware major version number.
  g_registers[ 148 ] = 0;
  g_registers[ 149 ] = 0;
  g_registers[ 150 ] = 3;
  
  // Boot loader algorithm used.
  g_registers[ 151 ] = 0xff; // No bootloader
  
  // Buffer size
  g_registers[ 152 ] = 0;
  
  // Number of register pages used
  g_registers[ 153 ] = 1;
  
  // GUID
  g_registers[ 208 ] = 0x00;
  g_registers[ 209 ] = 0x00;
  g_registers[ 210 ] = 0x00;
  g_registers[ 211 ] = 0x00;
  g_registers[ 212 ] = 0x00;
  g_registers[ 213 ] = 0x00;
  g_registers[ 214 ] = 0x00;
  g_registers[ 215 ] = 0x00;
  g_registers[ 216 ] = 0x00;
  g_registers[ 217 ] = 0x00;
  g_registers[ 218 ] = 0x00;
  g_registers[ 219 ] = 0x00;
  g_registers[ 220 ] = 0x00;
  g_registers[ 221 ] = 0x00;
  g_registers[ 222 ] = 0x00;
  g_registers[ 223 ] = 0x02;
  
  // MDF
  g_registers[ 224 ] = 'w';
  g_registers[ 225 ] = 'w';
  g_registers[ 226 ] = 'w';
  g_registers[ 227 ] = '.'; 
  g_registers[ 228 ] = 'd';
  g_registers[ 229 ] = 'o';
  g_registers[ 230 ] = 'f';
  g_registers[ 231 ] = 's';
  g_registers[ 232 ] = 'c';
  g_registers[ 233 ] = 'a';
  g_registers[ 234 ] = 'n';
  g_registers[ 235 ] = 'd';
  g_registers[ 236 ] = 'i';
  g_registers[ 237 ] = 'n';
  g_registers[ 238 ] = 'a';
  g_registers[ 239 ] = 'v';
  g_registers[ 240 ] = 'i';
  g_registers[ 241 ] = 'a';
  g_registers[ 242 ] = '.';
  g_registers[ 243 ] = 'c';
  g_registers[ 244 ] = 'o';
  g_registers[ 245 ] = 'm';
  g_registers[ 246 ] = '/';
  g_registers[ 247 ] = 's';
  g_registers[ 248 ] = 'i';
  g_registers[ 249 ] = 'm';
  g_registers[ 250 ] = 'n';
  g_registers[ 251 ] = 'o';
  g_registers[ 252 ] = 'd';
  g_registers[ 253 ] = 'e';
  g_registers[ 254 ] = '0';
  g_registers[ 255 ] = '1';
  
}


/////////////////////////////////////////////////////////////////////
// loadNodeData
//

bool DlgSimulatedVSCPNode::loadNodeData( wxString path )
{

  return true;
}

/////////////////////////////////////////////////////////////////////
// saveNodeData
//

bool DlgSimulatedVSCPNode::saveNodeData( wxString path )
{

  return true;
}

/*!
 * Should we show tooltips?
 */

bool DlgSimulatedVSCPNode::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap DlgSimulatedVSCPNode::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin DlgSimulatedVSCPNode bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end DlgSimulatedVSCPNode bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon DlgSimulatedVSCPNode::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin DlgSimulatedVSCPNode icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end DlgSimulatedVSCPNode icon retrieval
}
