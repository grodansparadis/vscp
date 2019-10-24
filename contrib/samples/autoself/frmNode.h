/////////////////////////////////////////////////////////////////////////////
// Name:        frmNode.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Wed 25 Apr 2007 11:47:53 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _FRMNODE_H_
#define _FRMNODE_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmNode.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "frmNode_symbols.h"
#include "wx/mdi.h"
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
#define SYMBOL_FRMNODE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FRMNODE_TITLE _T("")
#define SYMBOL_FRMNODE_IDNAME ID_FRMNODE
#define SYMBOL_FRMNODE_SIZE wxSize(300, 520)
#define SYMBOL_FRMNODE_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

DECLARE_EVENT_TYPE(wxUPDATE_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxS1_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxS2_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxSB_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxBLOCK_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxDRIFT_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxPROGRESS_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxTEMPERATURE_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxVOLTAGE_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxMESSAGE_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxLOG_EVENT, wxID_ANY)
DECLARE_EVENT_TYPE(wxTERMINATE_EVENT, wxID_ANY)

// Lamp picture enums
enum {
	LAMP_BLACK = 0,
	LAMP_BLUE,
	LAMP_GREEN,
	LAMP_PURPLE,
	LAMP_RED,
	LAMP_SEAGREEN,
	LAMP_YELLOW
};
 
/*!
 * frmNode class declaration
 */

class frmNode: public wxMDIChildFrame
{    
    DECLARE_CLASS( frmNode )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    frmNode();
    frmNode( wxMDIParentFrame* parent, wxWindowID id = SYMBOL_FRMNODE_IDNAME, const wxString& caption = SYMBOL_FRMNODE_TITLE, const wxPoint& pos = SYMBOL_FRMNODE_POSITION, const wxSize& size = SYMBOL_FRMNODE_SIZE, long style = SYMBOL_FRMNODE_STYLE );

    bool Create( wxMDIParentFrame* parent, wxWindowID id = SYMBOL_FRMNODE_IDNAME, const wxString& caption = SYMBOL_FRMNODE_TITLE, const wxPoint& pos = SYMBOL_FRMNODE_POSITION, const wxSize& size = SYMBOL_FRMNODE_SIZE, long style = SYMBOL_FRMNODE_STYLE );

    /// Destructor
    ~frmNode();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
	
		/// Windows refresh
		void OnUpdate( wxCommandEvent &event );
	
		/// Set picture bitmap
		void setBitmap( wxStaticBitmap *pBitmap, int nBitmap );
    
    /// S1 status update
    void OnS1Update( wxCommandEvent &event );
		
		/// S2 status update	
		void OnS2Update( wxCommandEvent &event );
		
		/// SB status update	
		void OnSBUpdate( wxCommandEvent &event );
		
	  /// Block status update	
		void OnBlockUpdate( wxCommandEvent &event );
		
		/// Drift status update
		void OnDriftUpdate( wxCommandEvent &event );
	
		/// Progress status update
		void OnProgressUpdate( wxCommandEvent &event );
	
		/// Temperature update
		void OnTemperatureUpdate( wxCommandEvent &event );
	
		/// Voltage update
		void OnVoltageUpdate( wxCommandEvent &event );
	
		/// Worker thread messages
		void OnWrkThreadMsg( wxCommandEvent &event );
	
		/// log messages
		void OnLogMsg( wxCommandEvent &event );
	
		/// Terminate messages
		void OnTerminateMsg( wxCommandEvent &event );
    
	

////@begin frmNode event handler declarations

  /// wxEVT_CLOSE_WINDOW event handler for ID_FRMNODE
  void OnCloseWindow( wxCloseEvent& event );

  /// wxEVT_MENU_CLOSE event handler for ID_FRMNODE
  void OnMenuClose( wxMenuEvent& event );

  /// wxEVT_DESTROY event handler for ID_FRMNODE
  void OnDestroy( wxWindowDestroyEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_ON
  void OnButtonOnClick( wxCommandEvent& event );

  /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OFF
  void OnButtonOffClick( wxCommandEvent& event );

////@end frmNode event handler declarations

////@begin frmNode member function declarations

  /// Retrieves bitmap resources
  wxBitmap GetBitmapResource( const wxString& name );

  /// Retrieves icon resources
  wxIcon GetIconResource( const wxString& name );
////@end frmNode member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin frmNode member variables
  wxStaticBitmap* m_Bitmap_S1;
  wxStaticBitmap* m_Bitmap_S2;
  wxStaticBitmap* m_Bitmap_SB;
  wxStaticBitmap* m_Bitmap_Block;
  wxStaticBitmap* m_Bitmap_Drift;
  wxStaticText* m_labelTemperature;
  wxGauge* m_Progressbar;
  wxStaticText* m_labelVoltage;
  /// Control identifiers
  enum {
    ID_FRMNODE = 10007,
    ID_ANIMATIONCTRL1 = 12000,
    ID_ANIMATIONCTRL = 12001,
    ID_ANIMATIONCTRL2 = 12002,
    ID_ANIMATIONCTRL3 = 10003,
    ID_STATICBITMAP = 12004,
    ID_BUTTON_ON = 10011,
    ID_BUTTON_OFF = 10103,
    ID_GAUGE1 = 10012
  };
////@end frmNode member variables

  /*! 
    Worker thread control variable which is true a long the
    worker thread should run
  */
  bool m_bRun;
	
	/// Index for node this thread work against
	int m_nodeIndex;
};

#endif
    // _FRMNODE_H_
