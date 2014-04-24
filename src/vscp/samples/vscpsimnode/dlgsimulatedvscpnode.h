/////////////////////////////////////////////////////////////////////////////
// Name:        dlgsimulatedvscpnode.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 10 Nov 2007 17:27:17 CET
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _DLGSIMULATEDVSCPNODE_H_
#define _DLGSIMULATEDVSCPNODE_H_


/*!
 * Includes
 */

////@begin includes
#include "dlgsimulatedvscpnode_symbols.h"
#include "wx/grid.h"
#include "wx/tglbtn.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DLGSIMULATEDVSCPNODE 10000
#define ID_PANEL 10002
#define ID_REG_GRID 10001
#define ID_TOGGLEBUTTON_CONNECT 10005
#define ID_BUTTON_RESTORE_DEFAULTS 10003
#define ID_BUTTON_SAVE_EVENTS 10004
#define ID_BUTTON_LOAD_EVENS 10007
#define ID_BUTTON_SEND_EVENT1 10006
#define ID_BUTTON_SEND_EVENT2 10008
#define ID_BUTTON_SEND_EVENT3 10009
#define ID_BUTTON_SETTINGS 10010
#define SYMBOL_DLGSIMULATEDVSCPNODE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_DLGSIMULATEDVSCPNODE_TITLE _("Simulated VSCP Node")
#define SYMBOL_DLGSIMULATEDVSCPNODE_IDNAME ID_DLGSIMULATEDVSCPNODE
#define SYMBOL_DLGSIMULATEDVSCPNODE_SIZE wxSize(480, 390)
#define SYMBOL_DLGSIMULATEDVSCPNODE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * DlgSimulatedVSCPNode class declaration
 */

class DlgSimulatedVSCPNode: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( DlgSimulatedVSCPNode )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    DlgSimulatedVSCPNode();
    DlgSimulatedVSCPNode( wxWindow* parent, wxWindowID id = SYMBOL_DLGSIMULATEDVSCPNODE_IDNAME, const wxString& caption = SYMBOL_DLGSIMULATEDVSCPNODE_TITLE, const wxPoint& pos = SYMBOL_DLGSIMULATEDVSCPNODE_POSITION, const wxSize& size = SYMBOL_DLGSIMULATEDVSCPNODE_SIZE, long style = SYMBOL_DLGSIMULATEDVSCPNODE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_DLGSIMULATEDVSCPNODE_IDNAME, const wxString& caption = SYMBOL_DLGSIMULATEDVSCPNODE_TITLE, const wxPoint& pos = SYMBOL_DLGSIMULATEDVSCPNODE_POSITION, const wxSize& size = SYMBOL_DLGSIMULATEDVSCPNODE_SIZE, long style = SYMBOL_DLGSIMULATEDVSCPNODE_STYLE );

    /// Destructor
    ~DlgSimulatedVSCPNode();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    
    /// Set register default values
    void setDefaultRegisters();
    
    /// Save node data
    bool saveNodeData( wxString path );
    
    /// Load external data
    bool loadNodeData( wxString path );

////@begin DlgSimulatedVSCPNode event handler declarations

////@end DlgSimulatedVSCPNode event handler declarations

////@begin DlgSimulatedVSCPNode member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end DlgSimulatedVSCPNode member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin DlgSimulatedVSCPNode member variables
    wxGrid* m_gridRegisters;
////@end DlgSimulatedVSCPNode member variables
};

#endif
    // _DLGSIMULATEDVSCPNODE_H_
