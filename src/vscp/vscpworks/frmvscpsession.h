/////////////////////////////////////////////////////////////////////////////
// Name:        frmvscpsession.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Sat 30 Jun 2007 14:08:14 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
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

#ifndef _FRMVSCPSESSION_H_
#define _FRMVSCPSESSION_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "frmvscpsession.h"
#endif

#include "wx/frame.h"
#include "wx/toolbar.h"
#include "wx/tglbtn.h"
#include "wx/grid.h"
#include "wx/html/htmlwin.h"
#include "wx/statusbr.h"

#include <wx/list.h>
#include <wx/ffile.h>
#include <wx/dynlib.h>
#include <wx/progdlg.h>

#include "vscptxobj.h"
#include "gridtable.h"

#include "dlgeventfilter.h"

#include <canaldlldef.h>
#include <canalsuperwrapper.h>

// Statusbar id's
#define STATUSBAR_STATUS_LEFT       0
#define STATUSBAR_STATUS_RIGHT      1

// List with transmission objects
WX_DECLARE_LIST ( VscpTXObj, TXLIST );


enum {
    Menu_Help_About = wxID_ABOUT,
    Menu_Popup_Submenu = 2000,
    Menu_Popup_TX_Transmit,
    Menu_Popup_TX_Add,
    Menu_Popup_TX_Edit,
    Menu_Popup_TX_Delete,
    Menu_Popup_TX_Periodic,
    Menu_Popup_TX_Clone
};

// Forward declarations
class wxToggleButton;
class wxGrid;
class wxHtmlWindow;

// Control identifiers
#define SYMBOL_FRMVSCPSESSION_STYLE wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX
#define SYMBOL_FRMVSCPSESSION_TITLE _("VSCP Client Window Session")
#define SYMBOL_FRMVSCPSESSION_IDNAME ID_FRMVSCPSESSION
#ifdef WIN32
#define VSCPWORKS_SESSION_SIZE          wxSize(880, 600)
#define VSCPWORKS_SESSION_POSITION      wxDefaultPosition
#else
#define VSCPWORKS_SESSION_SIZE          wxSize(920, 600)
#define VSCPWORKS_SESSION_POSITION      wxDefaultPosition
#endif

DECLARE_EVENT_TYPE(wxVSCP_IN_EVENT, wxID_ANY)                   // Received event
DECLARE_EVENT_TYPE(wxVSCP_OUT_EVENT, wxID_ANY)                  // Transmitted event
DECLARE_EVENT_TYPE(wxVSCP_CTRL_LOST_EVENT, wxID_ANY)            // Control thread connection lost
DECLARE_EVENT_TYPE(wxVSCP_RCV_LOST_EVENT, wxID_ANY)             // Receive thread connection lost
DECLARE_EVENT_TYPE(wxVSCP_RCV_PREP_CONNECT_EVENT, wxID_ANY)     // Prepare to connect
DECLARE_EVENT_TYPE(wxVSCP_RCV_CONNECTED_EVENT, wxID_ANY)        // Connected
//DECLARE_EVENT_TYPE(wxVSCP_STATUS_CHANGE_EVENT, wxID_ANY )       // status change

// Error constants for worker threads
#define VSCP_SESSION_ERROR_UNABLE_TO_CONNECT        -1
#define VSCP_SESSION_ERROR_TERMINATED               -99

//WX_DECLARE_LIST ( vscpEvent, eventInQueue );
WX_DECLARE_LIST ( vscpEvent, eventOutQueue );


class frmVSCPSession;
class deviceThread;

  
// Shared object among threads.
class ctrlObj
{

public:
  
    ctrlObj();
    ~ctrlObj();
  
    // Thread run control
    bool m_bQuit;

    // Pointer to VSCP session window
    frmVSCPSession *m_pVSCPSessionWnd;
  
    // Thread error return code for control thread
    long m_errorControl;

    // Thread error return code for receive thread
    long m_errorReceive;
  
    // Chennel ID for tx channel 
    unsigned long m_txChannelID;
  
    // Chennel ID for rx channel 
    unsigned long m_rxChannelID;

    // Copy of the receive gridtable
    BigGridTable *m_pgridTable;
  
    // Event output queue
    eventOutQueue m_outQueue;
  
    // Protection for output queue
    wxMutex m_mutexOutQueue;
    wxSemaphore m_semOutQue;
  
    // Table for transmission objects
    TXLIST m_txList;
  
    // Interface type
    int m_interfaceType;

	// CANAL driver level
    unsigned char m_driverLevel;
  
    // Interface information CANAL interface type
    canal_interface m_ifCANAL;
  
    // Interface information VSCP interface type
    vscp_interface m_ifVSCP;

	// Mutex handle that is used for sharing of the device.
	wxMutex m_deviceMutex;

    // GUID for CANAL device
    unsigned char m_GUID[16];

	// Handle for dll/dl driver interface
	long m_openHandle;

	// CANAL methods
	LPFNDLL_CANALOPEN                   m_proc_CanalOpen;
	LPFNDLL_CANALCLOSE                  m_proc_CanalClose;
	LPFNDLL_CANALGETLEVEL               m_proc_CanalGetLevel;
	LPFNDLL_CANALSEND                   m_proc_CanalSend;
	LPFNDLL_CANALRECEIVE                m_proc_CanalReceive;
	LPFNDLL_CANALDATAAVAILABLE          m_proc_CanalDataAvailable;
	LPFNDLL_CANALGETSTATUS              m_proc_CanalGetStatus;
	LPFNDLL_CANALGETSTATISTICS          m_proc_CanalGetStatistics;
	LPFNDLL_CANALSETFILTER              m_proc_CanalSetFilter;
	LPFNDLL_CANALSETMASK                m_proc_CanalSetMask;
	LPFNDLL_CANALSETBAUDRATE            m_proc_CanalSetBaudrate;
	LPFNDLL_CANALGETVERSION             m_proc_CanalGetVersion;
	LPFNDLL_CANALGETDLLVERSION          m_proc_CanalGetDllVersion;
	LPFNDLL_CANALGETVENDORSTRING        m_proc_CanalGetVendorString;
	// Generation 2
	LPFNDLL_CANALBLOCKINGSEND           m_proc_CanalBlockingSend;
	LPFNDLL_CANALBLOCKINGRECEIVE        m_proc_CanalBlockingReceive;
	LPFNDLL_CANALGETDRIVERINFO          m_proc_CanalGetdriverInfo;
  
};


///////////////////////////////////////////////////////////////////////////////
// TXWorkerThread
//
// This class implement a thread that handles
// transmit events
//

class TXWorkerThread : public wxThread
{

public:
	
	/// Constructor
	TXWorkerThread(frmVSCPSession *pForm );

	/// Destructor
	virtual ~TXWorkerThread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();


	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
    virtual void OnExit();

	/*!
		Pointer to control object.
	*/
	ctrlObj *m_pCtrlObject;

    /// TCP/IP Control
    VscpRemoteTcpIf m_tcpifControl;

    // Session form
    frmVSCPSession *m_frmSession;
};

///////////////////////////////////////////////////////////////////////////////
// RXWorkerThread
//
//	This class implement a thread that handles
//	client receive events
//

class RXWorkerThread : public wxThread
{

public:
	
	/// Constructor
	RXWorkerThread(frmVSCPSession *pForm );

	/// Destructor
	virtual ~RXWorkerThread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();


	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
    virtual void OnExit();

	/*!
		Pointer to control object.
	*/
	ctrlObj *m_pCtrlObject;

    /// TCP/IP Control
    VscpRemoteTcpIf m_tcpifReceive;

    // Session form
    frmVSCPSession *m_frmSession;
};






/////////////////////////////////////////////////////////////////////
//                          D E V I C E
/////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////
// deviceWriteThread
//
//	This class implement a thread that write data
//	to a blocking driver
//

class deviceWriteThread : public wxThread
{

public:
	
	/// Constructor
	deviceWriteThread();

	/// Destructor
	virtual ~deviceWriteThread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();

	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
	virtual void OnExit();

	/*!
		Pointer to master thread.
	*/
	deviceThread *m_pMainThreadObj;
  

  /*!
    Termination control
  */
  bool m_bQuit;


};


///////////////////////////////////////////////////////////////////////////////
// deviceReceiveThread
//
//	This class implement a thread that read data
//	from a blocking driver
//

class deviceReceiveThread : public wxThread
{

public:
	
	/// Constructor
	deviceReceiveThread();

	/// Destructor
	virtual ~deviceReceiveThread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();


	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
    virtual void OnExit();

	/*!
		Pointer to master thread.
	*/
	deviceThread *m_pMainThreadObj;
  

  /*!
    Termination control
  */
  bool m_bQuit;

};


///////////////////////////////////////////////////////////////////////////////
// deviceThread
//
//	This class implement a one of thread that look
//	for specific events and react on them appropriatly.
//


class deviceThread : public wxThread
{

public:
	
	/// Constructor
	deviceThread();

	/// Destructor
	virtual ~deviceThread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();


	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
  virtual void OnExit();


	/// dl/dll handler
	wxDynamicLibrary m_wxdll;


	/*!
		Pointer to control object.
	*/
	ctrlObj *m_pCtrlObject;
  

	/*!
		Holder for receive thread
	*/
	deviceReceiveThread *m_preceiveThread;

	/*!
		Holder for write thread
	*/
	deviceWriteThread *m_pwriteThread;

		/*!
			Check filter/mask to see if filter should be delivered

			The filter have the following matrix

			mask bit n	|	filter bit n	| msg id bit	|	result
			===========================================================
				0				X					X			Accept
				1				0					0			Accept
				1				0					1			Reject
				1				1					0			Reject
				1				1					1			Accept

				Formula is !( ( filter �d ) & mask )

			@param pclientItem Pointer to client item
			@param pcanalMsg Pointer to can message
			@return True if message is accepted false if rejected
			TODO
		*/
		
};


///////////////////////////////////////////////////////////////////////////////
// RXGridCellAttrProvider
//
// ----------------------------------------------------------------------------
// RX custom attr. provider: this one makes all odd rows appear grey
// ----------------------------------------------------------------------------
//

class RXGridCellAttrProvider : public wxGridCellAttrProvider
{

public:
    RXGridCellAttrProvider();
    virtual ~RXGridCellAttrProvider();

    virtual wxGridCellAttr *GetAttr( int row, int col,
                                    wxGridCellAttr::wxAttrKind  kind) const;

private:
    wxGridCellAttr *m_attrForEvenRows;
    wxGridCellAttr *m_attrForOddRows;

    wxGridCellAttr *m_attrEvenColDirection;
    wxGridCellAttr *m_attrEvenColTime;
    wxGridCellAttr *m_attrEvenColGuid;
    wxGridCellAttr *m_attrEvenColClass;
    wxGridCellAttr *m_attrEvenColType;
    wxGridCellAttr *m_attrEvenColHead;
    wxGridCellAttr *m_attrEvenColCount;
    wxGridCellAttr *m_attrEvenColData;
    wxGridCellAttr *m_attrEvenColTimestamp;
    wxGridCellAttr *m_attrEvenColNote;

    wxGridCellAttr *m_attrOddColDirection;
    wxGridCellAttr *m_attrOddColTime;
    wxGridCellAttr *m_attrOddColGuid;
    wxGridCellAttr *m_attrOddColClass;
    wxGridCellAttr *m_attrOddColType;
    wxGridCellAttr *m_attrOddColHead;
    wxGridCellAttr *m_attrOddColCount;
    wxGridCellAttr *m_attrOddColData;
    wxGridCellAttr *m_attrOddColTimestamp;
    wxGridCellAttr *m_attrOddColNote;

};


///////////////////////////////////////////////////////////////////////////////
// frmVSCPSession
//
// frmVSCPSession class declaration
//

class frmVSCPSession: public wxFrame
{    

    DECLARE_CLASS( frmVSCPSession )
    DECLARE_EVENT_TABLE()

public:

    /// Constructors
    frmVSCPSession();
    frmVSCPSession( wxWindow* parent, 
                        wxWindowID id = SYMBOL_FRMVSCPSESSION_IDNAME, 
                        const wxString& caption = SYMBOL_FRMVSCPSESSION_TITLE, 
                        const wxPoint& pos = VSCPWORKS_SESSION_POSITION,
                        const wxSize& size = VSCPWORKS_SESSION_SIZE,
                        long style = SYMBOL_FRMVSCPSESSION_STYLE );

    bool Create( wxWindow* parent, 
                        wxWindowID id = SYMBOL_FRMVSCPSESSION_IDNAME, 
                        const wxString& caption = SYMBOL_FRMVSCPSESSION_TITLE, 
                        const wxPoint& pos = VSCPWORKS_SESSION_POSITION,
                        const wxSize& size = VSCPWORKS_SESSION_SIZE,
                        long style = SYMBOL_FRMVSCPSESSION_STYLE );

    // Destructor
    ~frmVSCPSession();

    // Initialises member variables
    void Init();

    // Creates the controls and sizers
    void CreateControls();
  
    // Start the worker threads
    void startWorkerThreads( frmVSCPSession *pFrm  );

    // Stop the worker threads
    void stopWorkerThreads( void );
  
    // WxEvent handler for received VSCP event
    void eventReceive( wxCommandEvent &event );
  
    // WxEvent handler for transmit VSCP event
    void eventTransmit( wxCommandEvent &event );

	// Lost Control channel
	void eventLostCtrlIf( wxCommandEvent &event );

	// Lost receive channel
	void eventLostRcvIf( wxCommandEvent &event );

	// Preparing connection to remote host
	void eventPrepareConnect( wxCommandEvent &event );
  
	// Connected to remote host
	void eventConnected( wxCommandEvent &event );

    // Status change for statusbar
    void eventStatusChange( wxCommandEvent &event );

    /*!
        Add transmission object to the transmission list
        @param pObj Pointer to a transmission object.
        @param selrow row to write information to. If -1 a new
                line is added and information is added to that
                line.
        @return True on success.
    */
    bool addToTxGrid( VscpTXObj *pObj, int selrow = -1 );

    /*!
        Write info to the HTML receive window
        @param pRecord Receive object to write info. from.
    */
    void fillRxHtmlInfo( VscpRXObj *pRecord );

    /// Clone a TX row
    void OnTxCloneRow( wxCommandEvent& event );

    /// wxEVT_CLOSE_WINDOW event handler for ID_FRMVSCPSESSION
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_TOGGLEBUTTON_ACTIVATE
    void OnInterfaceActivate( wxCommandEvent& event );

    void OnFilterActivate(wxCommandEvent& event);

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_LOAD_MSG_LIST
    void LoadRXEventList( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_SAVE_MSG_LIST
    void SaveRXEventList( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_LOAD_TRANSMISSION_SET
    void OnTxLoadClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_SAVE_TRANSMISSION_SET
    void OnTxSaveClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_SESSION_EXIT
    void OnMenuitemVscpSessionExitClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM4
    void ClearRxList( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM5
    void ClearTxList( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_LOG
    void OnMenuitemVscpViewLogClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_COUNT
    void OnMenuitemVscpViewCountClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_READ_REGISTER
    void OnMenuitemSetAutoreplyClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_WRITE_REGISTER
    void OnMenuitemSetBurstCountClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_READ_ALL_REGISTERS
    void OnMenuitemSetFilterClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_RX_GRID_WIDTH
    void OnMenuitemSaveRxCellWidth( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_SAVE_TX_GRID_WIDTH
    void OnMenuitemSaveTxCellWidth( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM
    void OnMenuitemConfigurationClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_HELP
    void OnMenuitemVscpHelpClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_FAQ
    void OnMenuitemVscpFaqClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_SHORTCUTS
    void OnMenuitemVscpShortcutsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_THANKS
    void OnMenuitemVscpThanksClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_CREDITS
    void OnMenuitemVscpCreditsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_VSCP_SITE
    void OnMenuitemVscpVscpSiteClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENUITEM_VSCP_ABOUT
    void OnMenuitemVscpAboutClick( wxCommandEvent& event );

    /// wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_VSCP_GRID_RECEIVE
    void OnCellReceiveLeftClick( wxGridEvent& event );

    /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_VSCP_GRID_RECEIVE
    void OnGridCellReceiveRightClick( wxGridEvent& event );

    /// wxEVT_GRID_LABEL_LEFT_DCLICK event handler for ID_VSCP_GRID_RECEIVE
    void OnGridLabelLeftDClick( wxGridEvent& event );

    /// wxEVT_GRID_SELECT_CELL event handler for ID_VSCP_GRID_RECEIVE
    void OnSelectCell( wxGridEvent& event );

    /// wxEVT_COMMAND_HTML_LINK_CLICKED event handler for ID_HTMLWINDOW_RCVINFO
    void OnHtmlwindowRcvinfoLinkClicked( wxHtmlLinkEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON_TX_ADD
    void OnTxAddClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON_TX_EDIT
    void OnTxEditClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTONID_MENUITEM_TX_DELETE
    void OnTxDeleteClick( wxCommandEvent& event );

    /// wxEVT_GRID_CELL_LEFT_CLICK event handler for ID_VSCP_GRID_TRANSMISSION
    void OnCellTxLeftClick( wxGridEvent& event );

    /// wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_VSCP_GRID_TRANSMISSION
    void OnCellTxRightClick( wxGridEvent& event );

    /// wxEVT_GRID_CELL_LEFT_DCLICK event handler for ID_VSCP_GRID_TRANSMISSION
    void OnGridLeftDClick( wxGridEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTONID_MENUITEM_CANAL_SEND
    void OnTxSendClick( wxCommandEvent& event );

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();
  
    /// Last loaded path for TX Set 
	wxString m_wxStrTxSetPath;
  
    /// TRUE as long as worker thread should run
    bool m_bRun;
  
    /// Mutex to control the event grid
    wxMutex *m_pmutexEventGrid;
  
    /// Mutex to control the transmission grid
    wxMutex *m_pmutexTransmissionGrid;
  
    /// Receive event grid Table
    BigGridTable *m_pgridTable;
  
    /// Common control object
    ctrlObj m_CtrlObject;

    /// Mutex that control the stopThreads method
    wxMutex m_mutexStopThread; 

    // Filters
    bool m_bfilterActive;
    int m_nfilterMode;
    wxArrayLong m_filterArrayDisplay;
    wxArrayLong m_filterArrayFilter;
  
    // * * *   Threads  * * *
  
    /// Worker thread for control and transmit
    TXWorkerThread * m_pTXWorkerThread;
  
    /// Worker thread for receive
    RXWorkerThread * m_pRXWorkerThread;

	// Device Workerthread
	deviceThread * m_pDeviceWorkerThread;
  
    wxStatusBar* m_pitemStatusBar;
    wxToggleButton* m_BtnActivateInterface;
    wxToggleButton* m_BtnActivateFilter;
    wxPanel* m_pPanel;
    wxGrid* m_ctrlGridReceive;
    wxHtmlWindow* m_ctrlRcvHtmlInfo;
    wxBitmapButton* m_btnAdd;
    wxBitmapButton* m_btnEdit;
    wxBitmapButton* m_btnDelete;
    wxBitmapButton* m_btnLoadSet;
    wxBitmapButton* m_btnSaveSet;
    wxGrid* m_ctrlGridTransmission;
    wxBitmapButton* m_btnSend;
    wxBitmapButton* m_btnActivate;
    wxBitmapButton* m_btnClear;


    /// Control identifiers
    enum {
        ID_FRMVSCPSESSION = 13000,
        ID_TOOLBAR_VSCP_SESSION = 13011,
        ID_MENUITEM_VSCP_LOAD_MSG_LIST = 13012,
        ID_MENUITEM_VSCP_SAVE_MSG_LIST = 13013,
        ID_TOOL_VSCP_CUT = 13014,
        ID_TOOL_VSCP_COPY = 13015,
        ID_TOOL_VSCP_PASTE = 13016,
        ID_TOOL_VSCP_PRINT = 13017,
        ID_TOGGLEBUTTON_CONNECTION_ACTIVATE = 13019,
        ID_TOGGLEBUTTON_FILTER_ACTIVATE = 13020,
        ID_MENUITEM_VSCP_LOAD_TRANSMISSION_SET = 13023,
        ID_MENUITEM_VSCP_SAVE_TRANSMISSION_SET = 13024,
        ID_MENUITEM_VSCP_SESSION_EXIT = 13025,
        ID_MENUITEM_VSCP_CUT = 13026,
        ID_MENUITEM_VSCP_COPY = 13027,
        ID_MENUITEM_VSCP_PASTE = 13028,
        ID_MENUITEM4 = 13051,
        ID_MENUITEM5 = 13031,
        ID_MENUITEM_VSCP_LOG = 13029,
        ID_MENUITEM_VSCP_COUNT = 13030,
        ID_MENUITEM_READ_REGISTER = 13044,
        ID_MENUITEM_WRITE_REGISTER = 13045,
        ID_MENUITEM_READ_ALL_REGISTERS = 13046,
        ID_MENUITEM_GET_GUID = 13048,
        ID_MENUITEM_GET_MDF_URL = 13049,
        ID_MENUITEM_GET_MDF = 13047,
        ID_MENUITEM_RX_GRID_WIDTH = 13020,
        ID_MENUITEM_SAVE_TX_GRID_WIDTH = 13050,
        ID_MENUITEM1 = 13033,
        ID_MENUITEM2 = 13034,
        ID_MENUITEM3 = 13035,
        ID_MENUITEM = 13043,
        ID_MENUITEM_VSCP_HELP = 13036,
        ID_MENUITEM_VSCP_FAQ = 13037,
        ID_MENUITEM_VSCP_SHORTCUTS = 13038,
        ID_MENUITEM_VSCP_THANKS = 13039,
        ID_MENUITEM_VSCP_CREDITS = 13040,
        ID_MENUITEM_VSCP_VSCP_SITE = 13041,
        ID_MENUITEM_VSCP_ABOUT = 13042,
        ID_PANEL_VSCP_SESSION = 13032,
        ID_VSCP_GRID_RECEIVE = 13001,
        ID_HTMLWINDOW_RCVINFO = 13052,
        ID_BITMAPBUTTON_TX_ADD = 13002,
        ID_BITMAPBUTTON_TX_EDIT = 13003,
        ID_BITMAPBUTTONID_MENUITEM_TX_DELETE = 13004,
        ID_BITMAPBUTTONID_MENUITEM_TX_LOAD = 13005,
        ID_BITMAPBUTTONID_MENUITEM_TX_SAVE = 13006,
        ID_VSCP_GRID_TRANSMISSION = 13007,
        ID_BITMAPBUTTONID_MENUITEM_CANAL_SEND = 13008,
        ID_BITMAPBUTTON12 = 13009,
        ID_BITMAPBUTTON13 = 13010,
        ID_STATUSBAR = 10000
    };

};

#endif
  // _FRMVSCPSESSION_H_
