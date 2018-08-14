/////////////////////////////////////////////////////////////////////////////
//  Name:        frmvscpsession.cpp
//  Purpose:     
//  Author:      Ake Hedman, eurosource
//  Modified by: 
//  Created:     Sat 30 Jun 2007 14:08:14 CEST
//  RCS-ID:      
//  Copyright:   (C) 2007-2018                       
//  Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//  Licence:     
//  This program is free software; you can redistribute it and/or  
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version
//  2 of the License, or (at your option) any later version.
// 
//  This file is part of the VSCP (http://www.vscp.org) 
// 
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this file see the file COPYING.  If not, write to
//  the Free Software Foundation, 59 Temple Place - Suite 330,
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

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmvscpsession.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h". 
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/app.h>
#include <wx/listimpl.cpp> 
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/dynlib.h>
#include <wx/progdlg.h>
#include "dlgconfiguration.h"

#include "vscpworks.h"
#include "vscptxobj.h"
#include "dlgvscptrmit.h"
#include "dlgabout.h"
#include "dlgeventfilter.h"
#include "frmvscpsession.h"
#include <canal.h>
#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include "frmvscpsession_images.h"

extern appConfiguration g_Config;   // Configuration data
extern VSCPInformation g_vscpinfo;  // VSCP class type information

WX_DEFINE_LIST( eventOutQueue );
WX_DEFINE_LIST(TXLIST);

DEFINE_EVENT_TYPE(wxVSCP_IN_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_OUT_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_CTRL_LOST_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_RCV_PREP_CONNECT_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_RCV_CONNECTED_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_RCV_LOST_EVENT)
DEFINE_EVENT_TYPE(wxVSCP_STATUS_CHANGE_EVENT)

extern appConfiguration g_Config;   // Global configuration for VSCP


////////////////////////////////////////////////////////////////////////////////
// frmVSCPSession type definition
//

IMPLEMENT_CLASS(frmVSCPSession, wxFrame)


////////////////////////////////////////////////////////////////////////////////
// frmVSCPSession event table definition
//

BEGIN_EVENT_TABLE(frmVSCPSession, wxFrame)

    EVT_CLOSE( frmVSCPSession::OnCloseWindow )
    EVT_TOGGLEBUTTON( ID_TOGGLEBUTTON_CONNECTION_ACTIVATE, frmVSCPSession::OnInterfaceActivate )
    EVT_TOGGLEBUTTON( ID_TOGGLEBUTTON_FILTER_ACTIVATE, frmVSCPSession::OnFilterActivate )
    EVT_MENU( ID_MENUITEM_VSCP_LOAD_MSG_LIST, frmVSCPSession::LoadRXEventList )
    EVT_MENU( ID_MENUITEM_VSCP_SAVE_MSG_LIST, frmVSCPSession::SaveRXEventList )
    EVT_MENU( ID_MENUITEM_VSCP_LOAD_TRANSMISSION_SET, frmVSCPSession::OnTxLoadClick )
    EVT_MENU( ID_MENUITEM_VSCP_SAVE_TRANSMISSION_SET, frmVSCPSession::OnTxSaveClick )
    EVT_MENU( ID_MENUITEM_VSCP_SESSION_EXIT, frmVSCPSession::OnMenuitemVscpSessionExitClick )
    EVT_MENU( ID_MENUITEM4, frmVSCPSession::ClearRxList )
    EVT_MENU( ID_MENUITEM5, frmVSCPSession::ClearTxList )
    EVT_MENU( ID_MENUITEM_VSCP_LOG, frmVSCPSession::OnMenuitemVscpViewLogClick )
    EVT_MENU( ID_MENUITEM_VSCP_COUNT, frmVSCPSession::OnMenuitemVscpViewCountClick )
    EVT_MENU( ID_MENUITEM_READ_REGISTER, frmVSCPSession::OnMenuitemSetAutoreplyClick )
    EVT_MENU( ID_MENUITEM_WRITE_REGISTER, frmVSCPSession::OnMenuitemSetBurstCountClick )
    EVT_MENU( ID_MENUITEM_READ_ALL_REGISTERS, frmVSCPSession::OnMenuitemSetFilterClick )
    EVT_MENU( ID_MENUITEM_GET_GUID, frmVSCPSession::OnMenuitemSetAutoreplyClick )
    EVT_MENU( ID_MENUITEM_GET_MDF_URL, frmVSCPSession::OnMenuitemSetAutoreplyClick )
    EVT_MENU( ID_MENUITEM_GET_MDF, frmVSCPSession::OnMenuitemSetAutoreplyClick )
    EVT_MENU( ID_MENUITEM_RX_GRID_WIDTH, frmVSCPSession::OnMenuitemSaveRxCellWidth )
    EVT_MENU( ID_MENUITEM_SAVE_TX_GRID_WIDTH, frmVSCPSession::OnMenuitemSaveTxCellWidth )
    EVT_MENU( ID_MENUITEM1, frmVSCPSession::OnMenuitemSetAutoreplyClick )
    EVT_MENU( ID_MENUITEM2, frmVSCPSession::OnMenuitemSetBurstCountClick )
    EVT_MENU( ID_MENUITEM3, frmVSCPSession::OnMenuitemSetFilterClick )
    EVT_MENU( ID_MENUITEM, frmVSCPSession::OnMenuitemConfigurationClick )
    EVT_MENU( ID_MENUITEM_VSCP_HELP, frmVSCPSession::OnMenuitemVscpHelpClick )
    EVT_MENU( ID_MENUITEM_VSCP_FAQ, frmVSCPSession::OnMenuitemVscpFaqClick )
    EVT_MENU( ID_MENUITEM_VSCP_SHORTCUTS, frmVSCPSession::OnMenuitemVscpShortcutsClick )
    EVT_MENU( ID_MENUITEM_VSCP_THANKS, frmVSCPSession::OnMenuitemVscpThanksClick )
    EVT_MENU( ID_MENUITEM_VSCP_CREDITS, frmVSCPSession::OnMenuitemVscpCreditsClick )
    EVT_MENU( ID_MENUITEM_VSCP_VSCP_SITE, frmVSCPSession::OnMenuitemVscpVscpSiteClick )
    EVT_MENU( ID_MENUITEM_VSCP_ABOUT, frmVSCPSession::OnMenuitemVscpAboutClick )
    EVT_GRID_CELL_LEFT_CLICK( frmVSCPSession::OnCellReceiveLeftClick )
    EVT_GRID_CELL_RIGHT_CLICK( frmVSCPSession::OnGridCellReceiveRightClick )
    EVT_GRID_LABEL_LEFT_DCLICK( frmVSCPSession::OnGridLabelLeftDClick )
    EVT_GRID_SELECT_CELL( frmVSCPSession::OnSelectCell )
    EVT_HTML_LINK_CLICKED( ID_HTMLWINDOW_RCVINFO, frmVSCPSession::OnHtmlwindowRcvinfoLinkClicked )
    EVT_BUTTON( ID_BITMAPBUTTON_TX_ADD, frmVSCPSession::OnTxAddClick )
    EVT_BUTTON( ID_BITMAPBUTTON_TX_EDIT, frmVSCPSession::OnTxEditClick )
    EVT_BUTTON( ID_BITMAPBUTTONID_MENUITEM_TX_DELETE, frmVSCPSession::OnTxDeleteClick )
    EVT_BUTTON( ID_BITMAPBUTTONID_MENUITEM_TX_LOAD, frmVSCPSession::OnTxLoadClick )
    EVT_BUTTON( ID_BITMAPBUTTONID_MENUITEM_TX_SAVE, frmVSCPSession::OnTxSaveClick )
    EVT_GRID_CELL_LEFT_CLICK( frmVSCPSession::OnCellTxLeftClick )
    EVT_GRID_CELL_RIGHT_CLICK( frmVSCPSession::OnCellTxRightClick )
    EVT_GRID_CELL_LEFT_DCLICK( frmVSCPSession::OnGridLeftDClick )
    EVT_BUTTON( ID_BITMAPBUTTONID_MENUITEM_CANAL_SEND, frmVSCPSession::OnTxSendClick )

    EVT_MENU(Menu_Popup_TX_Transmit, frmVSCPSession::OnTxSendClick)
    EVT_MENU(Menu_Popup_TX_Add, frmVSCPSession::OnTxAddClick)
    EVT_MENU(Menu_Popup_TX_Edit, frmVSCPSession::OnTxEditClick)
    EVT_MENU(Menu_Popup_TX_Delete, frmVSCPSession::OnTxDeleteClick)
    EVT_MENU(Menu_Popup_TX_Clone, frmVSCPSession::OnTxCloneRow)

    EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_IN_EVENT, frmVSCPSession::eventReceive)
    EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_OUT_EVENT, frmVSCPSession::eventTransmit)
    EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_CTRL_LOST_EVENT, frmVSCPSession::eventLostCtrlIf)
    EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_RCV_PREP_CONNECT_EVENT, frmVSCPSession::eventPrepareConnect)
    EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_RCV_CONNECTED_EVENT, frmVSCPSession::eventConnected)
    EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_RCV_LOST_EVENT, frmVSCPSession::eventLostRcvIf)
    EVT_COMMAND(ID_FRMVSCPSESSION, wxVSCP_STATUS_CHANGE_EVENT, frmVSCPSession::eventStatusChange )

END_EVENT_TABLE()



// ----------------------------------------------------------------------------
// RXGridCellAttrProvider
// ----------------------------------------------------------------------------

RXGridCellAttrProvider::RXGridCellAttrProvider()
{
    // * * * Even rows * * *
    m_attrForEvenRows = new wxGridCellAttr;
    m_attrForEvenRows->SetBackgroundColour(*wxWHITE);

    m_attrEvenColDirection = new wxGridCellAttr;
    m_attrEvenColDirection->SetAlignment(wxALIGN_CENTRE, wxALIGN_TOP);
    m_attrEvenColDirection->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrEvenColDirection->SetBackgroundColour(g_Config.m_VscpRcvFrameRxBgColour);
    //m_attrEvenColDirection->SetFont();

    m_attrEvenColClass = new wxGridCellAttr;
    m_attrEvenColClass->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrEvenColClass->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrEvenColClass->SetBackgroundColour(g_Config.m_VscpRcvFrameRxBgColour);
    //m_attrEvenColClass->SetFont();

    m_attrEvenColType = new wxGridCellAttr;
    m_attrEvenColType->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrEvenColType->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrEvenColType->SetBackgroundColour(g_Config.m_VscpRcvFrameRxBgColour);
    //m_attrEvenColType->SetFont();

    m_attrEvenColNote = new wxGridCellAttr;
    m_attrEvenColNote->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrEvenColNote->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrEvenColNote->SetBackgroundColour(g_Config.m_VscpRcvFrameRxBgColour);
    //m_attrEvenColNote->SetFont();  


    // * * * Odd rows * * * 

    m_attrForOddRows = new wxGridCellAttr;
    m_attrForOddRows->SetBackgroundColour(*wxLIGHT_GREY);

    m_attrOddColDirection = new wxGridCellAttr;
    m_attrOddColDirection->SetAlignment(wxALIGN_CENTRE, wxALIGN_TOP);
    m_attrOddColDirection->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrOddColDirection->SetBackgroundColour(*wxLIGHT_GREY);
    //m_attrOddColDirection->SetFont();

    m_attrOddColClass = new wxGridCellAttr;
    m_attrOddColClass->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrOddColClass->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrOddColClass->SetBackgroundColour(*wxLIGHT_GREY);
    //m_attrOddColClass->SetFont();

    m_attrOddColType = new wxGridCellAttr;
    m_attrOddColType->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrOddColType->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrOddColType->SetBackgroundColour(*wxLIGHT_GREY);
    //m_attrOddColType->SetFont();

    m_attrOddColNote = new wxGridCellAttr;
    m_attrOddColNote->SetAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_attrOddColNote->SetTextColour(g_Config.m_VscpRcvFrameRxTextColour);
    m_attrOddColNote->SetBackgroundColour(*wxLIGHT_GREY);
    //m_attrOddColNote->SetFont();
}

RXGridCellAttrProvider::~RXGridCellAttrProvider()
{
    m_attrForOddRows->DecRef();

    m_attrEvenColDirection->DecRef();
    m_attrEvenColClass->DecRef();
    m_attrEvenColType->DecRef();
    m_attrEvenColNote->DecRef();

    m_attrOddColDirection->DecRef();
    m_attrOddColClass->DecRef();
    m_attrOddColType->DecRef();
    m_attrOddColNote->DecRef();
}

wxGridCellAttr *RXGridCellAttrProvider::GetAttr( int row, 
                                                    int col,
            wxGridCellAttr::wxAttrKind kind /* = wxGridCellAttr::Any */) const
{
    wxGridCellAttr *attr = wxGridCellAttrProvider::GetAttr(row, col, kind);

    if ( g_Config.m_VscpRcvFrameRxbPyamas && (row % 2) ) {

        // Odd rows    
        if (!attr) {

            switch (col) {

            case 0:
                attr = m_attrOddColDirection;
                attr->IncRef();
                break;

            case 1:
                attr = m_attrOddColClass;
                attr->IncRef();
                break;

            case 2:
                attr = m_attrOddColType;
                attr->IncRef();
                break;

            case 3:
                attr = m_attrOddColNote;
                attr->IncRef();
                break;

            default:
                attr = m_attrForOddRows;
                attr->IncRef();
                break;

            }

        } 
        else {
            if (!attr->HasBackgroundColour()) {
                wxGridCellAttr *attrNew = attr->Clone();
                attr->DecRef();
                attr = attrNew;
                attr->SetBackgroundColour(*wxLIGHT_GREY);
            }
        }
    } 
    else {
        if (!attr) {

            switch (col) {

            case 0:
                attr = m_attrEvenColDirection;
                attr->IncRef();
                break;

            case 1:
                attr = m_attrEvenColClass;
                attr->IncRef();
                break;

            case 2:
                attr = m_attrEvenColType;
                attr->IncRef();
                break;

            case 3:
                attr = m_attrEvenColNote;
                attr->IncRef();
                break;

            default:
                attr = m_attrForEvenRows;
                attr->IncRef();
                break;

            }
        }
    }

    return attr;
}




//////////////////////////////////////////////////////////////////////////////// 
// Constructor
//

ctrlObj::ctrlObj()
{
    m_bQuit = false;        // Dont even think of quiting yet...
    m_errorControl = 0;     // No error
    m_errorReceive = 0;
    m_pgridTable = NULL;    // No valid grid yet
}

//////////////////////////////////////////////////////////////////////////////// 
// Destructor
//

ctrlObj::~ctrlObj()
{

}


//////////////////////////////////////////////////////////////////////////////// 
// frmVSCPSession constructors
//

frmVSCPSession::frmVSCPSession()
{
    Init();
}

frmVSCPSession::frmVSCPSession( wxWindow* parent, 
                                    wxWindowID id, 
                                    const wxString& caption, 
                                    const wxPoint& pos, 
                                    const wxSize& size, 
                                    long style)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


////////////////////////////////////////////////////////////////////////////////
// frmVSCPSession creator
//

bool frmVSCPSession::Create( wxWindow* parent, 
                                wxWindowID id, 
                                const wxString& caption, 
                                const wxPoint& pos, 
                                const wxSize& size, 
                                long style)
{
     wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    SetIcon(GetIconResource(wxT("../../../docs/vscp/logo/fatbee_v2.ico")));

    return true;
}


////////////////////////////////////////////////////////////////////////////////
// Destructor
//

frmVSCPSession::~frmVSCPSession()
{
    // Remove possible items in TXLIST
    TXLIST::iterator iter;
    for ( iter = m_CtrlObject.m_txList.begin(); 
            iter != m_CtrlObject.m_txList.end(); 
            ++iter) {
        VscpTXObj *obj = *iter;
        if (NULL != obj) {
            if (NULL != obj->m_Event.pdata) {
                delete obj->m_Event.pdata;
            }
            delete obj;
        }
    }

    m_CtrlObject.m_txList.Clear();

    // Stop the worker threads
    stopWorkerThreads();

    m_pgridTable->clearEventList();

}


////////////////////////////////////////////////////////////////////////////////
// Init
//

void frmVSCPSession::Init()
{      
    m_BtnActivateInterface = NULL;
    m_pPanel = NULL;
    m_ctrlGridReceive = NULL;
    m_ctrlRcvHtmlInfo = NULL;
    m_btnAdd = NULL;
    m_btnEdit = NULL;
    m_btnDelete = NULL;
    m_btnLoadSet = NULL;
    m_btnSaveSet = NULL;
    m_ctrlGridTransmission = NULL;
    m_btnSend = NULL;
    m_btnActivate = NULL;
    m_btnClear = NULL;
    
    m_CtrlObject.m_bQuit = false;
    m_CtrlObject.m_interfaceType = INTERFACE_VSCP;
    memset(m_CtrlObject.m_GUID, 0, 16);

    m_pTXWorkerThread = NULL;
    m_pRXWorkerThread = NULL;
    m_pDeviceWorkerThread = NULL;

    m_bfilterActive = false;
    m_nfilterMode = FILTER_MODE_DISPLAY;
}

////////////////////////////////////////////////////////////////////////////////
// OnCloseWindow
//

void frmVSCPSession::OnCloseWindow(wxCloseEvent& event)
{
    wxBusyCursor wait;
    
    m_BtnActivateInterface->SetValue(false);
    
    // Save frame size and position
    wxRect rc = GetRect();
    g_Config.m_sizeSessionFrameWidth = rc.width;
    g_Config.m_sizeSessionFrameHeight = rc.height;

    wxProgressDialog progressDlg(_("VSCP Works"),
            _("Clearing VSCP events..."),
            100,
            this,
            wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);

    // Cleanup
    progressDlg.Update(30);
    stopWorkerThreads();

    progressDlg.Update(85);
    m_pgridTable->clearEventList();

    progressDlg.Update(100);
    progressDlg.Pulse(_("Done!"));

    // Hide window
    Show(false);

    // Destroy the window
    Destroy();

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// startWorkerThreads
//

void frmVSCPSession::startWorkerThreads(frmVSCPSession *pFrm)
{
    wxBusyCursor wait;

    if (INTERFACE_VSCP == m_CtrlObject.m_interfaceType) {

        ////////////////////////////////////////////////////////////////////////
        // Load controlobject control handler
        ////////////////////////////////////////////////////////////////////////
        
        m_pTXWorkerThread = new TXWorkerThread( this );

        if (NULL != m_pTXWorkerThread) {
            m_CtrlObject.m_pVSCPSessionWnd = (frmVSCPSession *)pFrm;
            m_pTXWorkerThread->m_pCtrlObject = &m_CtrlObject;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pTXWorkerThread->Create())) {
                m_pTXWorkerThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_pTXWorkerThread->Run())) {
                    ::wxGetApp().logMsg(_("Unable to run controlobject device thread."),
                                         VSCPWORKS_LOGMSG_CRITICAL );
                }
            } 
            else {
                ::wxGetApp().logMsg(_("Unable to create controlobject device thread."),
                                     VSCPWORKS_LOGMSG_CRITICAL );
            }
        } 
        else {
            ::wxGetApp().logMsg( _( "Unable to allocate memory for controlobject device thread." ), 
                                    VSCPWORKS_LOGMSG_CRITICAL );
        }
        

        /////////////////////////////////////////////////////////////////////////////
        // Load controlobject client message handler
        /////////////////////////////////////////////////////////////////////////////

        m_pRXWorkerThread = new RXWorkerThread( this );

        if (NULL != m_pRXWorkerThread) {
            m_CtrlObject.m_pVSCPSessionWnd = (frmVSCPSession *)pFrm;
            m_pRXWorkerThread->m_pCtrlObject = &m_CtrlObject;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pRXWorkerThread->Create())) {
                m_pRXWorkerThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_pRXWorkerThread->Run())) {
                    ::wxGetApp().logMsg(_("Unable to run controlobject client thread."),
                                         VSCPWORKS_LOGMSG_CRITICAL );
                }
            } 
            else {
                ::wxGetApp().logMsg(_("Unable to create controlobject client thread."),
                                     VSCPWORKS_LOGMSG_CRITICAL );
            }
        } 
        else {
            ::wxGetApp().logMsg( _( "Unable to allocate memory for controlobject client thread." ), 
                                    VSCPWORKS_LOGMSG_CRITICAL );
        }
    } 
    else {

        /////////////////////////////////////////////////////////////////////////////
        // Load device handler
        /////////////////////////////////////////////////////////////////////////////
        m_pDeviceWorkerThread = new deviceThread;

        if (NULL != m_pDeviceWorkerThread) {
            m_CtrlObject.m_pVSCPSessionWnd = (frmVSCPSession *) pFrm;
            m_pDeviceWorkerThread->m_pCtrlObject = &m_CtrlObject;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pDeviceWorkerThread->Create())) {
                m_pDeviceWorkerThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
                if (wxTHREAD_NO_ERROR != (err = m_pDeviceWorkerThread->Run())) {
                    ::wxGetApp().logMsg( _( "Unable to run controlobject device thread." ), 
                                            VSCPWORKS_LOGMSG_CRITICAL );
                }
            } 
            else {
                ::wxGetApp().logMsg( _( "Unable to create controlobject device thread." ), 
                                        VSCPWORKS_LOGMSG_CRITICAL );
            }
        } 
        else {
            ::wxGetApp().logMsg( _( "Unable to allocate memory for controlobject device thread." ), 
                                    VSCPWORKS_LOGMSG_CRITICAL );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// stopWorkerThreads
//

void frmVSCPSession::stopWorkerThreads(void)
{
    wxBusyCursor wait;

    m_BtnActivateInterface->SetValue(false);
    m_BtnActivateInterface->SetLabel(_("Disconnected"));
    m_BtnActivateInterface->Update();

    m_CtrlObject.m_bQuit = true;
    wxSleep( 1 );
    //wxLogNull noError;   

    if (INTERFACE_VSCP == m_CtrlObject.m_interfaceType) {

        if (NULL != m_pTXWorkerThread) {
            m_pTXWorkerThread->Delete();
            // Thread pointer set to NULL in thread destructor
        }

        if (NULL != m_pRXWorkerThread) {
            m_pRXWorkerThread->Delete();
            // Thread pointer set to NULL in thread destructor
        }

    } 
    else {
        if (NULL != m_pDeviceWorkerThread) {
            m_pDeviceWorkerThread->Wait();
            // Thread pointer set to NULL in thread destructor
        }
    }

}

////////////////////////////////////////////////////////////////////////////////
// OnInterfaceActivate
// 

void frmVSCPSession::OnInterfaceActivate(wxCommandEvent& event)
{
    wxBusyCursor wait;

    if (!m_BtnActivateInterface->GetValue()) {
        m_CtrlObject.m_bQuit = true;
        m_BtnActivateInterface->SetValue(false);
        m_BtnActivateInterface->SetLabel(_("Disconnected"));
    } 
    else {
        m_CtrlObject.m_bQuit = false;
        startWorkerThreads(this); // Start worker threads
        m_BtnActivateInterface->SetValue(true);
        m_BtnActivateInterface->SetLabel(_("Connected"));
    }

    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// OnFilterActivate
// 

void frmVSCPSession::OnFilterActivate(wxCommandEvent& event)
{
    wxBusyCursor wait;

    if ( !m_BtnActivateFilter->GetValue() ) {
        m_BtnActivateFilter->SetValue(false);
        m_bfilterActive = false;
        m_BtnActivateFilter->SetLabel(_("Filter"));
    } 
    else {
        if ( ( FILTER_MODE_DISPLAY == m_nfilterMode ) && 
                ( 0 == m_filterArrayDisplay.GetCount() ) ) {
            wxMessageBox(_("There is no class/type pairs defined to display."));
            m_BtnActivateFilter->SetValue(false);
            m_bfilterActive = false;
        }
        else if ( FILTER_MODE_FILTER == m_nfilterMode && 
                ( 0 == m_filterArrayFilter.GetCount() ) ) {
            wxMessageBox(_("There is no class/type pairs defined to filter."));
            m_BtnActivateFilter->SetValue(false);
            m_bfilterActive  = false;
        }
        else {
            m_BtnActivateFilter->SetValue(true);
            m_bfilterActive = true;
        }
    }

    event.Skip();

}


////////////////////////////////////////////////////////////////////////////////
// CreateControls
//

void frmVSCPSession::CreateControls()
{   
    frmVSCPSession* itemFrame1 = this;

    // Statusbar
    m_pitemStatusBar = new wxStatusBar;
    m_pitemStatusBar->Create( itemFrame1, 
                                ID_STATUSBAR, 
                                wxST_SIZEGRIP | wxNO_BORDER );
    m_pitemStatusBar->SetFieldsCount( 2 );
    itemFrame1->SetStatusBar( m_pitemStatusBar );

    // Menu
    wxMenuBar* menuBar = new wxMenuBar;

    wxMenu* itemMenu16 = new wxMenu;
    itemMenu16->Append( ID_MENUITEM_VSCP_LOAD_MSG_LIST, 
                        _("Load VSCP events from file..."), 
                        wxEmptyString, wxITEM_NORMAL);
    itemMenu16->Append( ID_MENUITEM_VSCP_SAVE_MSG_LIST, 
                        _("Save VSCP events to file..."), 
                        wxEmptyString, wxITEM_NORMAL);
    itemMenu16->AppendSeparator();
    itemMenu16->Append( ID_MENUITEM_VSCP_LOAD_TRANSMISSION_SET, 
                        _("Load transmission set from file..."), 
                        wxEmptyString, 
                        wxITEM_NORMAL);
    itemMenu16->Append( ID_MENUITEM_VSCP_SAVE_TRANSMISSION_SET, 
                            _("Save transmission set to file..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu16->AppendSeparator();
    itemMenu16->Append( ID_MENUITEM_VSCP_SESSION_EXIT, 
                        _("Exit"), 
                        wxEmptyString, 
                        wxITEM_NORMAL);
    menuBar->Append( itemMenu16, 
                        _("File"));
    
    wxMenu* itemMenu24 = new wxMenu;
    itemMenu24->Append( ID_MENUITEM_VSCP_CUT, 
                            _("Cut"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu24->Append( ID_MENUITEM_VSCP_COPY, 
                            _("Copy"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu24->Append( ID_MENUITEM_VSCP_PASTE, 
                            _("Paste"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu24->AppendSeparator();
    itemMenu24->Append( ID_MENUITEM4, 
                            _("Clear receive list"), wxEmptyString, wxITEM_NORMAL);
    itemMenu24->Append( ID_MENUITEM5, 
                            _("Clear transmission list"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    menuBar->Append( itemMenu24, 
                        _("Edit"));
    
    wxMenu* itemMenu31 = new wxMenu;
    itemMenu31->Append( ID_MENUITEM_VSCP_LOG, 
                            _("Message Log"), 
                            wxEmptyString, 
                            wxITEM_RADIO);
    itemMenu31->Check( ID_MENUITEM_VSCP_LOG, 
                        true);
    itemMenu31->Append( ID_MENUITEM_VSCP_COUNT, 
                            _("Message Count"), 
                            wxEmptyString, 
                            wxITEM_RADIO);
    itemMenu31->AppendSeparator();
    menuBar->Append( itemMenu31, 
                        _("View"));
    
    wxMenu* itemMenu35 = new wxMenu;
    itemMenu35->Append( ID_MENUITEM_READ_REGISTER, 
                            _("Read Register..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu35->Append( ID_MENUITEM_WRITE_REGISTER, 
                            _("Write Register..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu35->Append( ID_MENUITEM_READ_ALL_REGISTERS, 
                            _("Read all registers..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu35->AppendSeparator();
    itemMenu35->Append( ID_MENUITEM_GET_GUID, 
                            _("Get GUID for node..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu35->Append( ID_MENUITEM_GET_MDF_URL, 
                            _("Get MDF URL..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu35->Append( ID_MENUITEM_GET_MDF, 
                            _("Get MDF..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    menuBar->Append( itemMenu35, _("VSCP") );
    
    wxMenu* itemMenu43 = new wxMenu;
    itemMenu43->Append( ID_MENUITEM_RX_GRID_WIDTH, 
                            _("Save RX Grid widths as standard"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu43->Append( ID_MENUITEM_SAVE_TX_GRID_WIDTH, 
                            _("Save TX Grid widths as standard"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu43->AppendSeparator();
    itemMenu43->Append( ID_MENUITEM1, 
                            _("Auto Reply Settings..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu43->Append( ID_MENUITEM2, 
                            _("Set Burst Count..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu43->Append( ID_MENUITEM3, 
                            _("Set Filter..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu43->AppendSeparator();
    itemMenu43->Append( ID_MENUITEM, 
                            _("Settings..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    menuBar->Append(itemMenu43, _("Settings"));
    
    wxMenu* itemMenu52 = new wxMenu;
    menuBar->Append(itemMenu52, _("Tools"));
    wxMenu* itemMenu53 = new wxMenu;
    itemMenu53->Append( ID_MENUITEM_VSCP_HELP, 
                            _("VSCP Works Help"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu53->Append( ID_MENUITEM_VSCP_FAQ, 
                            _("Frequently Asked Questions"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu53->Append( ID_MENUITEM_VSCP_SHORTCUTS, 
                            _("Keyboard shortcuts"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu53->AppendSeparator();
    itemMenu53->Append( ID_MENUITEM_VSCP_THANKS, 
                            _("Thanks..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu53->Append( ID_MENUITEM_VSCP_CREDITS, 
                            _("Credits..."), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu53->AppendSeparator();
    itemMenu53->Append( ID_MENUITEM_VSCP_VSCP_SITE, 
                            _("Go to VSCP site"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    itemMenu53->AppendSeparator();
    itemMenu53->Append( ID_MENUITEM_VSCP_ABOUT, 
                            _("About"), 
                            wxEmptyString, 
                            wxITEM_NORMAL);
    menuBar->Append(itemMenu53, _("Help"));
    
    itemFrame1->SetMenuBar(menuBar);

    wxToolBar* itemToolBar2 = CreateToolBar( wxTB_FLAT | wxTB_HORIZONTAL, 
                                                ID_TOOLBAR_VSCP_SESSION );
    wxBitmap itemtool3Bitmap(itemFrame1->GetBitmapResource(wxT("open.xpm")));
    wxBitmap itemtool3BitmapDisabled;
    itemToolBar2->AddTool( ID_MENUITEM_VSCP_LOAD_MSG_LIST, 
                            wxEmptyString, 
                            itemtool3Bitmap, 
                            itemtool3BitmapDisabled, 
                            wxITEM_NORMAL, 
                            _("Fetch data from file"), 
                            wxEmptyString);
    wxBitmap itemtool4Bitmap(itemFrame1->GetBitmapResource(wxT("save.xpm")));
    wxBitmap itemtool4BitmapDisabled;
    itemToolBar2->AddTool( ID_MENUITEM_VSCP_SAVE_MSG_LIST, 
                            wxEmptyString, 
                            itemtool4Bitmap, 
                            itemtool4BitmapDisabled, 
                            wxITEM_NORMAL, 
                            _("Save data to file"), 
                            wxEmptyString);
    itemToolBar2->AddSeparator();
    wxBitmap itemtool6Bitmap(itemFrame1->GetBitmapResource(wxT("cut.xpm")));
    wxBitmap itemtool6BitmapDisabled;
    itemToolBar2->AddTool( ID_TOOL_VSCP_CUT, 
                            wxEmptyString, 
                            itemtool6Bitmap, 
                            itemtool6BitmapDisabled, 
                            wxITEM_NORMAL, 
                            _("Remove selected row(s)"), 
                            wxEmptyString );
    wxBitmap itemtool7Bitmap(itemFrame1->GetBitmapResource(wxT("copy.xpm")));
    wxBitmap itemtool7BitmapDisabled;
    itemToolBar2->AddTool( ID_TOOL_VSCP_COPY, 
                            wxEmptyString, 
                            itemtool7Bitmap, 
                            itemtool7BitmapDisabled, 
                            wxITEM_NORMAL, 
                            _("Copy selected row(s) \nto the clipboard"), 
                            wxEmptyString );
    wxBitmap itemtool8Bitmap(itemFrame1->GetBitmapResource(wxT("paste.xpm")));
    wxBitmap itemtool8BitmapDisabled;
    itemToolBar2->AddTool( ID_TOOL_VSCP_PASTE, 
                            wxEmptyString, 
                            itemtool8Bitmap, 
                            itemtool8BitmapDisabled, 
                            wxITEM_NORMAL, 
                            _("Paste row(s) from clipboard"), 
                            wxEmptyString);
    itemToolBar2->AddSeparator();
    wxBitmap itemtool10Bitmap(itemFrame1->GetBitmapResource(wxT("Print.xpm")));
    wxBitmap itemtool10BitmapDisabled;
    itemToolBar2->AddTool( ID_TOOL_VSCP_PRINT, 
                            wxEmptyString, 
                            itemtool10Bitmap, 
                            itemtool10BitmapDisabled, 
                            wxITEM_NORMAL, 
                            _("Print selected or all row(s)"), 
                            wxEmptyString);
    itemToolBar2->AddSeparator();
    itemToolBar2->AddSeparator();

    m_BtnActivateFilter = new wxToggleButton;
    m_BtnActivateFilter->Create( itemToolBar2, 
                                    ID_TOGGLEBUTTON_FILTER_ACTIVATE, 
                                    _("Filter"), 
                                    wxDefaultPosition, 
                                    wxSize(-1, -1), 
                                    0 );
    m_BtnActivateFilter->SetValue(false);
    if (frmVSCPSession::ShowToolTips())
        m_BtnActivateFilter->SetToolTip(_("Activate/Deactivate the receive filter"));
    itemToolBar2->AddControl( m_BtnActivateFilter );

    m_BtnActivateInterface = new wxToggleButton;
    m_BtnActivateInterface->Create( itemToolBar2, 
                                        ID_TOGGLEBUTTON_CONNECTION_ACTIVATE, 
                                        _("Connected     "), 
                                        wxDefaultPosition, 
                                        wxSize(-1, -1), 
                                        0 );
    m_BtnActivateInterface->SetValue(true);
    if (frmVSCPSession::ShowToolTips())
        m_BtnActivateInterface->SetToolTip(_("Activate/Deactivate the interface"));
    //m_BtnActivateInterface->SetForegroundColour(wxColour(255, 255, 255));
    //m_BtnActivateInterface->SetBackgroundColour(wxColour(165, 42, 42));
    //m_BtnActivateInterface->SetFont( wxFont(10, 
    //                                  wxFONTFAMILY_SWISS, 
    //                                  wxFONTSTYLE_NORMAL, 
    //                                  wxFONTWEIGHT_BOLD, 
    //                                  false, 
    //                                  wxT("Sans")));
    itemToolBar2->AddControl(m_BtnActivateInterface);

    


    itemToolBar2->Realize();
    itemFrame1->SetToolBar(itemToolBar2);

    m_pPanel = new wxPanel;
    m_pPanel->Create( itemFrame1, 
                        ID_PANEL_VSCP_SESSION, 
                        wxDefaultPosition, 
                        wxDefaultSize, 
                        wxSUNKEN_BORDER | wxTAB_TRAVERSAL );

    wxBoxSizer* itemBoxSizer65 = new wxBoxSizer(wxVERTICAL);
    m_pPanel->SetSizer(itemBoxSizer65);

    wxBoxSizer* itemBoxSizer66 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer65->Add(itemBoxSizer66, 1, wxGROW|wxALL, 2);

    wxBoxSizer* itemBoxSizer67 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer66->Add(itemBoxSizer67, 1, wxGROW|wxALL, 5);

    m_ctrlGridReceive = new wxGrid;
    m_ctrlGridReceive->Create( m_pPanel, 
                                ID_VSCP_GRID_RECEIVE, 
                                wxDefaultPosition, 
                                wxSize(550, 300), 
                                wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL );
    m_ctrlGridReceive->SetName(wxT("vscprcveventgrid"));
    m_ctrlGridReceive->SetBackgroundColour(wxColour(204, 244, 244));
    m_ctrlGridReceive->SetFont( wxFont( 8, 
                                            wxFONTFAMILY_DEFAULT, 
                                            wxFONTSTYLE_NORMAL, 
                                            wxFONTWEIGHT_BOLD, 
                                            false, 
                                            wxT("")));
    m_ctrlGridReceive->SetDefaultColSize(100);
    m_ctrlGridReceive->SetDefaultRowSize(20);
    m_ctrlGridReceive->SetColLabelSize(18);
    m_ctrlGridReceive->SetRowLabelSize(30);
    itemBoxSizer67->Add(m_ctrlGridReceive, 2, wxGROW|wxALL, 2);

    m_ctrlRcvHtmlInfo = new wxHtmlWindow;
    m_ctrlRcvHtmlInfo->Create( m_pPanel, 
                                ID_HTMLWINDOW_RCVINFO, 
                                wxDefaultPosition, 
                                wxSize(300, 300), 
                                wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL );
    itemBoxSizer67->Add(m_ctrlRcvHtmlInfo, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer70 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer65->Add(itemBoxSizer70, 0, wxGROW|wxALL, 2);

    wxBoxSizer* itemBoxSizer71 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer70->Add(itemBoxSizer71, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer72 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer71->Add(itemBoxSizer72, 0, wxALIGN_TOP|wxALL, 5);

    m_btnAdd = new wxBitmapButton;
    m_btnAdd->Create( m_pPanel, 
                        ID_BITMAPBUTTON_TX_ADD, 
                        itemFrame1->GetBitmapResource(wxT("New1.xpm")), 
                        wxDefaultPosition, 
                        wxDefaultSize, 
                        wxBU_AUTODRAW );
    if (frmVSCPSession::ShowToolTips())
        m_btnAdd->SetToolTip(_("Add transmission line"));
    itemBoxSizer72->Add(m_btnAdd, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_btnEdit = new wxBitmapButton;
    m_btnEdit->Create( m_pPanel, 
                        ID_BITMAPBUTTON_TX_EDIT, 
                        itemFrame1->GetBitmapResource(wxT("copy.xpm")),
                        wxDefaultPosition, 
                        wxDefaultSize, 
                        wxBU_AUTODRAW );
    if (frmVSCPSession::ShowToolTips())
        m_btnEdit->SetToolTip(_("Edit selected line"));
    itemBoxSizer72->Add(m_btnEdit, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_btnDelete = new wxBitmapButton;
    m_btnDelete->Create( m_pPanel, 
                            ID_BITMAPBUTTONID_MENUITEM_TX_DELETE, 
                            itemFrame1->GetBitmapResource(wxT("delete.xpm")), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxBU_AUTODRAW );
    if (frmVSCPSession::ShowToolTips())
        m_btnDelete->SetToolTip(_("Delete selected line"));
    itemBoxSizer72->Add(m_btnDelete, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    itemBoxSizer72->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_btnLoadSet = new wxBitmapButton;
    m_btnLoadSet->Create( m_pPanel, 
                            ID_BITMAPBUTTONID_MENUITEM_TX_LOAD, 
                            itemFrame1->GetBitmapResource(wxT("open.xpm")), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxBU_AUTODRAW );
    if (frmVSCPSession::ShowToolTips())
        m_btnLoadSet->SetToolTip(_("Load transmission set from file..."));
    itemBoxSizer72->Add(m_btnLoadSet, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_btnSaveSet = new wxBitmapButton;
    m_btnSaveSet->Create( m_pPanel, 
                            ID_BITMAPBUTTONID_MENUITEM_TX_SAVE, 
                            itemFrame1->GetBitmapResource(wxT("filesaveas.xpm")), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxBU_AUTODRAW );
    if (frmVSCPSession::ShowToolTips())
        m_btnSaveSet->SetToolTip(_("Save transmission set to file..."));
    itemBoxSizer72->Add(m_btnSaveSet, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer79 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer71->Add(itemBoxSizer79, 5, wxALIGN_TOP|wxALL, 0);

    m_ctrlGridTransmission = new wxGrid;
    m_ctrlGridTransmission->Create( m_pPanel, 
                                        ID_VSCP_GRID_TRANSMISSION, 
                                        wxDefaultPosition, 
                                        wxSize(700, 190), 
                                        wxHSCROLL|wxVSCROLL );
    if (frmVSCPSession::ShowToolTips())
        m_ctrlGridTransmission->SetToolTip(_("Right click for menu / double click to transmit"));
    m_ctrlGridTransmission->SetFont(wxFont( 8, 
                                                wxFONTFAMILY_DEFAULT, 
                                                wxFONTSTYLE_NORMAL, 
                                                wxFONTWEIGHT_BOLD, 
                                                false, 
                                                wxT("")));
    m_ctrlGridTransmission->SetDefaultColSize(100);
    m_ctrlGridTransmission->SetDefaultRowSize(18);
    m_ctrlGridTransmission->SetColLabelSize(18);
    m_ctrlGridTransmission->SetRowLabelSize(30);
    m_ctrlGridTransmission->CreateGrid(1, 6, wxGrid::wxGridSelectRows);
    itemBoxSizer79->Add(m_ctrlGridTransmission, 5, wxGROW|wxALL, 2);

    // Sizer for transmitt button space
    wxBoxSizer* itemBoxSizerTx = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer71->Add( itemBoxSizerTx, 0, wxALIGN_TOP|wxALL, 5 );

    m_btnSend = new wxBitmapButton;
    m_btnSend->Create( m_pPanel, 
                        ID_BITMAPBUTTONID_MENUITEM_CANAL_SEND, 
                        itemFrame1->GetBitmapResource(wxT("redo.xpm")), 
                        wxDefaultPosition, 
                        wxDefaultSize, 
                        wxBU_AUTODRAW );
    if (frmVSCPSession::ShowToolTips())
        m_btnSend->SetToolTip(_("Transmit event from selected row(s)"));
    itemBoxSizerTx->Add(m_btnSend, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    itemBoxSizerTx->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    m_btnActivate = new wxBitmapButton;
    m_btnActivate->Create( m_pPanel, 
                            ID_BITMAPBUTTON12, 
                            itemFrame1->GetBitmapResource(wxT("Print.xpm")), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxBU_AUTODRAW );
    m_btnActivate->Show(false);
    itemBoxSizerTx->Add(m_btnActivate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_btnClear = new wxBitmapButton;
    m_btnClear->Create( m_pPanel, 
                            ID_BITMAPBUTTON13, 
                            itemFrame1->GetBitmapResource(wxT("open.xpm")), 
                            wxDefaultPosition, 
                            wxDefaultSize, 
                            wxBU_AUTODRAW );
    m_btnClear->Show(false);
    itemBoxSizerTx->Add(m_btnClear, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_ctrlGridReceive->EnableEditing(false);

    // Create the grid table
    m_pgridTable = new BigGridTable();

    // Set gridtable
    m_ctrlGridReceive->SetTable(m_pgridTable, true, wxGrid::wxGridSelectRows);

    m_pgridTable->SetAttrProvider(new RXGridCellAttrProvider);

    m_ctrlGridReceive->SetCellHighlightPenWidth(0); // No ugly cell selection

    /*
      m_ctrlGridReceive->SetCellBackgroundColour( g_Config.m_VscpRcvFrameBgColour );
      m_ctrlGridReceive->SetGridLineColour( g_Config.m_VscpRcvFrameLineColour );
      m_ctrlGridReceive->SetCellTextColour( g_Config.m_VscpRcvFrameTextColour );
     */
    m_ctrlGridReceive->SetRowLabelSize(50);

    m_ctrlGridReceive->SetDefaultRowSize(g_Config.m_VscpRcvFrameRowLineHeight);

    // Dir
    m_ctrlGridReceive->SetColSize(0, g_Config.m_VscpRcvFieldWidth[0]);
    m_ctrlGridReceive->SetColLabelValue(0, g_Config.m_VscpRcvFieldText[0]);

    // Class
    m_ctrlGridReceive->SetColSize(1, g_Config.m_VscpRcvFieldWidth[1]);
    m_ctrlGridReceive->SetColLabelValue(1, g_Config.m_VscpRcvFieldText[1]);

    // Type
    m_ctrlGridReceive->SetColSize(2, g_Config.m_VscpRcvFieldWidth[2]);
    m_ctrlGridReceive->SetColLabelValue(2, g_Config.m_VscpRcvFieldText[2]);


    // Notes
    m_ctrlGridReceive->SetColSize(3, g_Config.m_VscpRcvFieldWidth[3]);
    m_ctrlGridReceive->SetColLabelValue(3, g_Config.m_VscpRcvFieldText[3]);

    // Make all content visible
    m_ctrlGridReceive->AutoSizeRows();

    // *** 


    m_ctrlGridTransmission->EnableEditing(false); // No uncontrolled editing
    m_ctrlGridTransmission->SetSelectionMode(wxGrid::wxGridSelectRows);
    m_ctrlGridTransmission->SetCellHighlightPenWidth(0); // No ugly cell selection

    m_ctrlGridTransmission->SetColSize(0, g_Config.m_VscpTrmitFieldWidth[0]);
    m_ctrlGridTransmission->SetColLabelValue(0, g_Config.m_VscpTrmitFieldText[0]);
    m_ctrlGridTransmission->SetColSize(1, g_Config.m_VscpTrmitFieldWidth[1]);
    m_ctrlGridTransmission->SetColLabelValue(1, g_Config.m_VscpTrmitFieldText[1]);
    m_ctrlGridTransmission->SetColSize(2, g_Config.m_VscpTrmitFieldWidth[2]);
    m_ctrlGridTransmission->SetColLabelValue(2, g_Config.m_VscpTrmitFieldText[2]);
    m_ctrlGridTransmission->SetColSize(3, g_Config.m_VscpTrmitFieldWidth[3]);
    m_ctrlGridTransmission->SetColLabelValue(3, g_Config.m_VscpTrmitFieldText[3]);
    m_ctrlGridTransmission->SetColSize(4, g_Config.m_VscpTrmitFieldWidth[4]);
    m_ctrlGridTransmission->SetColLabelValue(4, g_Config.m_VscpTrmitFieldText[4]);
    m_ctrlGridTransmission->SetColSize(5, g_Config.m_VscpTrmitFieldWidth[5]);
    m_ctrlGridTransmission->SetColLabelValue(5, g_Config.m_VscpTrmitFieldText[5]);
    m_ctrlGridTransmission->DeleteRows(0);

    // Make all content visible
    m_ctrlGridTransmission->AutoSizeRows();

    // Asssign a vadlid copy of the gridtable to the shared object.
    m_CtrlObject.m_pgridTable = m_pgridTable;

}


//m_ctrlRcvHtmlInfo = new HtmlWindow();
//m_ctrlRcvHtmlInfo->Create( m_pPanel, ID_HTMLWINDOW_RCVINFO, wxDefaultPosition, wxSize(300, 300), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
//itemBoxSizer67->Add(m_ctrlRcvHtmlInfo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnGridCellReceiveRightClick
//

void frmVSCPSession::OnGridCellReceiveRightClick(wxGridEvent& event)
{
    event.Skip();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnGridLabelLeftDClick
//

void frmVSCPSession::OnGridLabelLeftDClick(wxGridEvent& event)
{
    event.Skip();
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpAboutClick
//

void frmVSCPSession::OnMenuitemVscpAboutClick(wxCommandEvent& event)
{
    dlgAbout dlg(this);
    if (wxID_OK == dlg.ShowModal()) {

    }
    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// ShowToolTips
//

bool frmVSCPSession::ShowToolTips()
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// GetBitmapResource
//

wxBitmap frmVSCPSession::GetBitmapResource(const wxString& name)
{
    // Bitmap retrieval
    wxUnusedVar(name);
    if (name == wxT("open.xpm")) {
        wxBitmap bitmap(open_xpm);
        return bitmap;
    }
    else if (name == wxT("save.xpm")) {
        wxBitmap bitmap(save_xpm);
        return bitmap;
    }
    else if (name == wxT("cut.xpm")) {
        wxBitmap bitmap(cut_xpm);
        return bitmap;
    }
    else if (name == wxT("copy.xpm")) {
        wxBitmap bitmap(copy_xpm);
        return bitmap;
    }
    else if (name == wxT("paste.xpm")) {
        wxBitmap bitmap(paste_xpm);
        return bitmap;
    }
    else if (name == wxT("Print.xpm")) {
        wxBitmap bitmap(print_xpm);
        return bitmap;
    }
    else if (name == wxT("New1.xpm")) {
        wxBitmap bitmap(new_xpm);
        return bitmap;
    }
    else if (name == wxT("delete.xpm")) {
        wxBitmap bitmap(delete_xpm);
        return bitmap;
    }
    else if (name == wxT("filesaveas.xpm")) {
        wxBitmap bitmap(filesaveas_xpm);
        return bitmap;
    }
    else if (name == wxT("redo.xpm")) {
        wxBitmap bitmap(redo_xpm);
        return bitmap;
    }

    return wxNullBitmap;
}

////////////////////////////////////////////////////////////////////////////////
// GetIconResource
//

wxIcon frmVSCPSession::GetIconResource(const wxString& name)
{
    // Icon retrieval
    wxUnusedVar(name);
    if ( name == wxT("../../../docs/vscp/logo/fatbee_v2.ico") ) {
        wxIcon icon(fatbee_v2_xpm);
        return icon;
    }

    return wxNullIcon;
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpSessionExitClick
//

void frmVSCPSession::OnMenuitemVscpSessionExitClick(wxCommandEvent& event)
{
    Close();
    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// OnSelectCell
//

void frmVSCPSession::OnSelectCell(wxGridEvent& event)
{
    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// LoadRXEventList
//

void frmVSCPSession::LoadRXEventList(wxCommandEvent& event)
{
    wxString str;
    wxXmlDocument doc;


    // If the grid contains row ask if we should clear it before
    // loading new data
    if (m_pgridTable->m_rxList.GetCount()) {
        if (wxYES == wxMessageBox(_("Should current rows be removed before loading new set from file?"),
                _("Remove rows?"),
                wxYES_NO | wxICON_QUESTION)) {
            m_ctrlGridReceive->DeleteRows(0, m_ctrlGridReceive->GetNumberRows());

            RXLIST::iterator iter;
            for ( iter = m_pgridTable->m_rxList.begin(); 
                        iter != m_pgridTable->m_rxList.end(); 
                        ++iter ) {
                VscpRXObj *obj = *iter;
                if (NULL != obj) delete obj;
            }

            m_pgridTable->m_rxList.Clear();

        }
    }

    // First find a path to read RX data from
    wxFileDialog dlg(this,
            _("Choose file to load events from "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("rxevents"),
            _("RX Data Files (*.txd)|*.rxd|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));
    if (wxID_OK == dlg.ShowModal()) {

        wxBusyCursor wait;
        wxProgressDialog progressDlg(_("VSCP Works"),
                _("Prepare to loading VSCP events..."),
                100,
                this,
                wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);


        if (!doc.Load(dlg.GetPath())) {
            return;
        }

        progressDlg.Pulse(_("Checking VSCP events file..."));

        // start processing the XML file xxxx
        if (doc.GetRoot()->GetName() != wxT("vscprxdata")) {
            wxMessageBox(_("File with invalid format."));
            return;
        }

        progressDlg.Pulse(_("Loading VSCP events file..."));

        //progressDlg.Show();
        m_ctrlGridReceive->BeginBatch();


        wxXmlNode *child = doc.GetRoot()->GetChildren();
        while (child) {

            if (child->GetName() == wxT("event")) {

                // Allocate a new transmission set
                VscpRXObj *pObj = new VscpRXObj;
                if (NULL == pObj) {
                    wxMessageBox(_("Unable to create event object."));
                    return;
                }

                pObj->m_pEvent = NULL;

                pObj->m_pEvent = new vscpEvent;
                if (NULL == pObj->m_pEvent) {
                    delete pObj;
                    wxMessageBox(_("Unable to create VSCP event storage."));
                    return;
                }

                pObj->m_pEvent->pdata = NULL;

                wxXmlNode *subchild = child->GetChildren();
                while (subchild) {

                    if (subchild->GetName() == wxT("dir")) {
                        str = subchild->GetNodeContent();
                        if (wxNOT_FOUND != str.Find(_("rx"))) {
                            pObj->m_nDir = VSCP_EVENT_DIRECTION_RX;
                        } else {
                            pObj->m_nDir = VSCP_EVENT_DIRECTION_TX;
                        }
                    } 
                    else if (subchild->GetName() == wxT("time")) {
                        str = subchild->GetNodeContent();
                        pObj->m_time.ParseDateTime(str);
                    } 
                    else if (subchild->GetName() == wxT("class")) {
                        str = subchild->GetNodeContent();
                        pObj->m_pEvent->vscp_class = vscp_readStringValue(str);
                    } 
                    else if (subchild->GetName() == wxT("type")) {
                        str = subchild->GetNodeContent();
                        pObj->m_pEvent->vscp_type = vscp_readStringValue(str);
                    } 
                    else if (subchild->GetName() == wxT("head")) {
                        str = subchild->GetNodeContent();
                        pObj->m_pEvent->head = (uint8_t) vscp_readStringValue(str);
                    } 
                    else if (subchild->GetName() == wxT("guid")) {
                        str = subchild->GetNodeContent();
                        vscp_getGuidFromString(pObj->m_pEvent, str);
                    } 
                    else if (subchild->GetName() == wxT("data")) {
                        str = subchild->GetNodeContent();
                        vscp_setVscpEventDataFromString(pObj->m_pEvent, str);
                    } 
                    else if (subchild->GetName() == wxT("timestamp")) {
                        str = subchild->GetNodeContent();
                        pObj->m_pEvent->timestamp = vscp_readStringValue(str);
                    } 
                    else if (subchild->GetName() == wxT("period")) {
                        pObj->m_wxStrNote = subchild->GetNodeContent();
                    }


                    subchild = subchild->GetNext();

                } // while subchild

                // Update progress dialog
                progressDlg.Pulse(_("Loading VSCP events..."));

                // Append to the RX list
                m_pgridTable->writeEvent(pObj, true);

            }

            child = child->GetNext();

        } // while child

        progressDlg.Update(100);

        m_ctrlGridReceive->EndBatch();

        // If selected mow the added row into seight
        if (g_Config.m_bAutoscollRcv) {
            m_ctrlGridReceive->MakeCellVisible(m_pgridTable->m_rxList.GetCount() - 1, 0);
        }

    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// SaveRXEventList
//

void frmVSCPSession::SaveRXEventList(wxCommandEvent& event)
{
    wxString str;

    // First find a path to save RX data to
    wxFileDialog dlg(this,
            _("Please choose a file to save received events to "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("rxevents"),
            _("RX Data Files (*.txd)|*.rxd|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));
    if (wxID_OK == dlg.ShowModal()) {

        wxBusyCursor wait;

        wxProgressDialog progressDlg(_("VSCP Works"),
                _("Saving VSCP events..."),
                100,
                this,
                wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);

        wxFFileOutputStream *pFileStream = new wxFFileOutputStream(dlg.GetPath());
        if (NULL == pFileStream) {
            wxMessageBox(_("Failed to create file."));
            return;
        }

        pFileStream->Write("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"));

        // RX data start
        pFileStream->Write("<vscprxdata>\n", strlen("<vscprxdata>\n"));

        RXLIST::iterator iter;
        for ( iter = m_pgridTable->m_rxList.begin(); 
                iter != m_pgridTable->m_rxList.end(); 
                ++iter ) {

            VscpRXObj *pobj = *iter;
            pFileStream->Write("<event>\n", strlen("<event>\n"));


            pFileStream->Write("<dir>", strlen("<dir>"));
            if (VSCP_EVENT_DIRECTION_RX == pobj->m_nDir) {
                pFileStream->Write("rx", strlen("rx"));
            } else {
                pFileStream->Write("tx", strlen("tx"));
            }
            pFileStream->Write("</dir>\n", strlen("</dir>\n"));

            pFileStream->Write("<time>", strlen("<time>"));
            str = pobj->m_time.FormatISODate() + _(" ") + 
                    pobj->m_time.FormatISOTime();
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</time>\n", strlen("</time>\n"));

            pFileStream->Write("<head>", strlen("<head>"));
            str.Printf(_("%d"), pobj->m_pEvent->head);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</head>\n", strlen("</head>\n"));

            pFileStream->Write("<class>", strlen("<class>"));
            str.Printf(_("%d"), pobj->m_pEvent->vscp_class);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</class>\n", strlen("</class>\n"));

            pFileStream->Write("<type>", strlen("<type>"));
            str.Printf(_("%d"), pobj->m_pEvent->vscp_type);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</type>\n", strlen("</type>\n"));

            pFileStream->Write("<guid>", strlen("<guid>"));
            vscp_writeGuidToString(pobj->m_pEvent, str);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</guid>\n", strlen("</guid>\n"));

            pFileStream->Write("<sizedata>", strlen("<sizedata>")); // Not used by read routine	
            str.Printf(_("%d"), pobj->m_pEvent->sizeData);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</sizedata>\n", strlen("</sizedata>\n"));

            if (0 != pobj->m_pEvent->sizeData) {
                pFileStream->Write("<data>", strlen("<data>"));
                vscp_writeVscpDataToString(pobj->m_pEvent, str);
                pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
                pFileStream->Write("</data>\n", strlen("</data>\n"));
            }

            pFileStream->Write("<timestamp>", strlen("<timestamp>"));
            str.Printf(_("%lu"), pobj->m_pEvent->timestamp);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</timestamp>\n", strlen("</timestamp>\n"));

            pFileStream->Write("<note>", strlen("<note>"));
            pFileStream->Write( pobj->m_wxStrNote.mb_str(), 
                                    strlen(pobj->m_wxStrNote.mb_str()));
            pFileStream->Write("</note>\n", strlen("</note>\n"));

            pFileStream->Write("</event>\n", strlen("</event>\n"));

        }

        // Transmission set end
        pFileStream->Write("</vscprxdata>\n", strlen("</vscprxdata>\n"));

        // Close the file
        pFileStream->Close();

        // Clean up
        delete pFileStream;

    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// OnCellReceiveLeftClick
//

void frmVSCPSession::OnCellReceiveLeftClick(wxGridEvent& event)
{
    if (ID_VSCP_GRID_TRANSMISSION == event.GetId()) {
        m_ctrlGridTransmission->SelectRow(event.GetRow());
    } 
    else if (ID_VSCP_GRID_RECEIVE == event.GetId()) {
        m_ctrlGridReceive->SelectRow(event.GetRow());
        VscpRXObj *pRecord = m_pgridTable->readEvent(event.GetRow());
        if (NULL != pRecord) {
            fillRxHtmlInfo(pRecord);
        }
    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
//  fillRxHtmlInfo
//

void frmVSCPSession::fillRxHtmlInfo(VscpRXObj *pRecord)
{
    wxString str, wrkstr;

    // Check pointers
    if (NULL == pRecord) return;
    if (NULL == pRecord->m_pEvent) return;

    str = _("<html><body><h3>VSCP Event</h3>");
    str += _("<small><font color=\"#993399\">");
    if (VSCP_EVENT_DIRECTION_RX == pRecord->m_nDir) {
        str += _("Received event");
    } 
    else {
        str += _("Transmitted event");
    }
    str += _("</font></small><br>");
    str += _("<b>Time: </b>");
    str += _("<font color=\"rgb(0, 0, 153);\"><tt>");
    str += pRecord->m_time.FormatISODate() + _(" ") + 
            pRecord->m_time.FormatISOTime();
    str += _("</tt></font><br>");
    str += _("<br>");
    str += _("<b>Class: </b>");
    str += _("<font color=\"rgb(0, 102, 0);\">");

    // Class
    wrkstr = _("");
    if ( g_Config.m_UseSymbolicNames ) {
        wxString strClass =
                g_vscpinfo.getClassDescription(pRecord->m_pEvent->vscp_class);
        if ( 0 == strClass.Length() ) strClass = _("Unknown class");
        wrkstr = wxString::Format(_("%s <br><tt>0x%04X, %d</tt>"),
                strClass.c_str(),
                pRecord->m_pEvent->vscp_class,
                pRecord->m_pEvent->vscp_class);
    } 
    else {
        wrkstr = wxString::Format(_("<tt>0x%04X, %d</tt>"),
                pRecord->m_pEvent->vscp_class,
                pRecord->m_pEvent->vscp_class);
    }
    str += wrkstr;
    str += _("</font>");
    str += _("<br><b>");
    str += _("Type:");
    str += _("</b> ");
    str += _("<font color=\"rgb(0, 102, 0);\">");

    // Type
    wrkstr = _("");
    if (g_Config.m_UseSymbolicNames) {
        wxString strType =
                g_vscpinfo.getTypeDescription(pRecord->m_pEvent->vscp_class,
                pRecord->m_pEvent->vscp_type);
        if (0 == strType.Length()) strType = _("Unknown type");
        wrkstr = wxString::Format(_("%s <br><tt>0x%04X, %d </tt>"),
                strType.c_str(),
                pRecord->m_pEvent->vscp_type,
                pRecord->m_pEvent->vscp_type);
    } 
    else {
        wrkstr = wxString::Format(_("<tt>0x%04X, %d</tt>"),
                pRecord->m_pEvent->vscp_type,
                pRecord->m_pEvent->vscp_type);
    }
    str += wrkstr;
    str += _("</font>");
    str += _("<br><br>");

    // Data
    str += _("<b>Data count:</b> ");
    str += _("<color color=\"rgb(0, 0, 153);\"><tt>");
    str += wxString::Format(_("%d"), pRecord->m_pEvent->sizeData);
    
    str += _("</tt></font><br>");
    str += _("<font color=\"rgb(0, 0, 153);\"><tt>");
    vscp_writeVscpDataToString(pRecord->m_pEvent, wrkstr, true);
    str += wrkstr;
    str += _("</tt></font><br>");
    str += _("<font color=\"rgb(0, 0, 153);\">");
    wrkstr = vscp_getRealTextData(pRecord->m_pEvent);
    vscp_makeHtml(wrkstr);
    str += wrkstr;
    str += _("</font><br>");

    str += _("<b>From GUID:</b><br>");
    str += _("<font color=\"rgb(0, 0, 153);\"><tt>");
    vscp_writeGuidToString4Rows(pRecord->m_pEvent, wrkstr);
    str += wrkstr;
    str += _("</tt></font><br><br>");
    str += _("<b>Head:</b> ");
    str += _("<font color=\"rgb(0, 0, 153);\"><tt>");
    str += wxString::Format(_("%02X "), pRecord->m_pEvent->head);

    if (pRecord->m_pEvent->head & VSCP_HEADER_HARD_CODED) {
        str += _(" [Hard coded] ");
    }

    if (pRecord->m_pEvent->head & VSCP_HEADER_NO_CRC) {
        str += _(" [No CRC Calculation] ");
    }

    str += _("</font></tt><br>");
    str += _("<b>Timestamp:</b> ");
    str += _("<font color=\"rgb(0, 0, 153);\">");
    str += wxString::Format(_("%08X"), pRecord->m_pEvent->timestamp);
    str += _("</font>");
    str += _("<br>");
    str += _("<br></body></html>");

    m_ctrlRcvHtmlInfo->SetPage(str);
}

////////////////////////////////////////////////////////////////////////////////
//  addToTxGrid
//

bool frmVSCPSession::addToTxGrid(VscpTXObj *pObj, int selrow)
{
    int row;
    wxString str;

    // Check pointer
    if (NULL == pObj) return false;

    if (-1 == selrow) {
        m_ctrlGridTransmission->AppendRows(1);
        row = m_ctrlGridTransmission->GetNumberRows();
    } 
    else {
        row = selrow + 1;
    }

    // Active checkbox
    m_ctrlGridTransmission->SetCellRenderer( row - 1, 
                                                0, 
                                                new wxGridCellBoolRenderer);
    m_ctrlGridTransmission->SetCellEditor( row - 1, 
                                            0, 
                                            new wxGridCellBoolEditor);
    m_ctrlGridTransmission->SetCellAlignment( row - 1, 
                                                0, 
                                                wxALIGN_CENTRE, 
                                                wxALIGN_CENTRE );

    // Name
    m_ctrlGridTransmission->SetCellValue( row - 1, 
                                            1, 
                                            pObj->m_wxStrName );
    m_ctrlGridTransmission->SetCellRenderer( row - 1, 
                                                2, 
                                                new wxGridCellStringRenderer );

    // Period
    m_ctrlGridTransmission->SetCellValue( row - 1, 
                                            2, 
                                            wxString::Format(_("%lu"), 
                                            pObj->m_period ) );
    m_ctrlGridTransmission->SetCellRenderer( row - 1, 
                                                2, 
                                                new wxGridCellStringRenderer );
    m_ctrlGridTransmission->SetCellAlignment( row - 1, 
                                                2,
                                                wxALIGN_CENTRE, 
                                                wxALIGN_CENTRE );

    // Count
    m_ctrlGridTransmission->SetCellValue( row - 1, 
                                            3, 
                                            wxString::Format(_("%lu"), 
                                            pObj->m_count));
    m_ctrlGridTransmission->SetCellRenderer( row - 1, 
                                                3, 
                                                new wxGridCellStringRenderer);
    m_ctrlGridTransmission->SetCellAlignment( row - 1, 
                                                3,
                                                wxALIGN_CENTRE,
                                                wxALIGN_CENTRE );

    // Trigger
    m_ctrlGridTransmission->SetCellValue( row - 1, 
                                            4, 
                                            wxString::Format(_("%lu"), 
                                            pObj->m_trigger ) );
    m_ctrlGridTransmission->SetCellRenderer( row - 1, 
                                                4, 
                                                new wxGridCellStringRenderer );
    m_ctrlGridTransmission->SetCellAlignment( row - 1, 
                                                4,
                                                wxALIGN_CENTRE,
                                                wxALIGN_CENTRE );

    // Event
    str.Printf(_("class=%d type=%d dsize=%d \ndata="),
                    pObj->m_Event.vscp_class,
                    pObj->m_Event.vscp_type,
                    pObj->m_Event.sizeData);

    for (int i = 0; i < pObj->m_Event.sizeData; i++) {
        str += wxString::Format(_("%02X "), pObj->m_Event.pdata[i]);
        if (i && !(i % 8)) {
            str += _("\n\t\t");
        }
    }

    m_ctrlGridTransmission->SetCellValue(row - 1, 5, str);
    m_ctrlGridTransmission->AutoSizeRow(row - 1);

    return true;
}


////////////////////////////////////////////////////////////////////////////////
// OnCellTxRightClick
//

void frmVSCPSession::OnCellTxRightClick(wxGridEvent& event)
{
    wxMenu menu;

    wxPoint pos = ClientToScreen(event.GetPosition());

    if (ID_VSCP_GRID_TRANSMISSION == event.GetId()) {

        menu.Append( Menu_Popup_TX_Transmit, _T("Transmit event from selected row(s)") );
        menu.AppendSeparator();
        menu.Append( Menu_Popup_TX_Add, _T("Add transmission row...") );
        menu.Append( Menu_Popup_TX_Edit, _T("Edit transmission row...") );
        menu.Append( Menu_Popup_TX_Delete, _T("Delete transmission row...") );
        menu.AppendSeparator();
        menu.Append( Menu_Popup_TX_Clone, _T("Clone transmission row...") );
        menu.AppendSeparator();
        menu.AppendCheckItem(Menu_Popup_TX_Periodic, _T("Enable periodic transmission") );
        menu.Check( Menu_Popup_TX_Periodic, true);
        //menu.Append(Menu_Popup_Submenu, _T("&Submenu"), CreateDummyMenu(NULL));
        //menu.Append(Menu_Popup_ToBeDeleted, _T("To be &deleted"));
        //menu.AppendCheckItem(Menu_Popup_ToBeChecked, _T("To be &checked"));
        //menu.Append(Menu_Popup_ToBeGreyed, _T("To be &greyed"),
        //              _T("This menu item should be initially greyed out"));
        //menu.AppendSeparator();
        //menu.Append(Menu_File_Quit, _T("E&xit"));

        //menu.Delete(Menu_Popup_ToBeDeleted);
        //menu.Check(Menu_Popup_ToBeChecked, true);
        //menu.Enable(Menu_Popup_ToBeGreyed, false);
    } 
    else if (ID_VSCP_GRID_RECEIVE == event.GetId()) {
        menu.Append( ID_MENUITEM4, _T("Clear all receive events") );
        /*menu.AppendSeparator();
        menu.Append( Menu_Popup_TX_Transmit, _T("Edit row..."));
        menu.Append( Menu_Popup_TX_Transmit, _T("Add note..."));
        menu.AppendSeparator();
        menu.Append( Menu_Popup_TX_Transmit, _T("Copy row"));
        menu.Append( Menu_Popup_TX_Transmit, _T("Cut row"));
        menu.Append( Menu_Popup_TX_Transmit, _T("Paste row"));
        menu.AppendSeparator();
        menu.Append( Menu_Popup_TX_Transmit, _T("Transmit selected row(s)"));
        menu.Append( Menu_Popup_TX_Transmit, _T("Edit and transmit selected row(s)"));
        menu.Append( Menu_Popup_TX_Transmit, _T("Save row(s) as transmission object(s)"));*/
        menu.AppendSeparator();
        menu.Append( ID_MENUITEM_VSCP_LOAD_MSG_LIST, _T("Load VSCP events from file...") );
        menu.Append( ID_MENUITEM_VSCP_SAVE_MSG_LIST, _T("Save VSCP events to file...") );
    }

    PopupMenu(&menu/*, pos.x, pos.y*/);

    event.Skip(false);

}


////////////////////////////////////////////////////////////////////////////////
// OnCellTxLeftClick
//

void frmVSCPSession::OnCellTxLeftClick(wxGridEvent& event)
{
    if (ID_VSCP_GRID_TRANSMISSION == event.GetId()) {
        m_ctrlGridTransmission->SelectRow(event.GetRow());
    } 
    else if (ID_VSCP_GRID_RECEIVE == event.GetId()) {
        m_ctrlGridReceive->SelectRow(event.GetRow());
    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// OnGridLeftDClick
//

void frmVSCPSession::OnGridLeftDClick(wxGridEvent& event)
{
    if (ID_VSCP_GRID_TRANSMISSION == event.GetId()) {
        wxCommandEvent event; // Not uses so we dont care about actual data
        OnTxSendClick(event);
    } 
    else if (ID_VSCP_GRID_RECEIVE == event.GetId()) {
        ;
    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpViewLogClick
//

void frmVSCPSession::OnMenuitemVscpViewLogClick(wxCommandEvent& event)
{
    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpViewCountClick
//

void frmVSCPSession::OnMenuitemVscpViewCountClick(wxCommandEvent& event)
{
    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// ClearRxList
//

void frmVSCPSession::ClearRxList(wxCommandEvent& event)
{
    if (wxYES == wxMessageBox(_("Are you sure that all receive events should be removed?"),
            _("Remove receive rows?"),
            wxYES_NO | wxICON_QUESTION)) {

        wxBusyCursor wait;

        // Remove elements from the receive list
        m_pgridTable->clearEventList();

        // Clear the grid
        if (m_ctrlGridReceive->GetNumberRows()) {
            m_ctrlGridReceive->DeleteRows(0, m_ctrlGridReceive->GetNumberRows());
        }
        //m_ctrlGridReceive->ForceRefresh();
        //m_ctrlGridReceive->MakeCellVisible( 0, 0 );
    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// ClearTxList
//

void frmVSCPSession::ClearTxList(wxCommandEvent& event)
{
    if (wxYES == wxMessageBox(_("Are you sure that all transmission events should be removed?"),
            _("Remove transmission rows?"),
            wxYES_NO | wxICON_QUESTION)) {
        TXLIST::iterator iter;
        for (iter = m_CtrlObject.m_txList.begin();
                iter != m_CtrlObject.m_txList.end();
                ++iter) {
            VscpTXObj *pobj = *iter;
            if (NULL != pobj) {
                if (NULL != pobj->m_Event.pdata) delete pobj->m_Event.pdata;
                delete pobj;
            }
        }

        // Clear the list
        m_CtrlObject.m_txList.Clear();

        // Clear the grid
        m_ctrlGridTransmission->DeleteRows(0, m_ctrlGridTransmission->GetNumberRows());

    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// OnTxSendClick
//

void frmVSCPSession::OnTxSendClick(wxCommandEvent& event)
{
    if (m_ctrlGridTransmission->GetNumberRows()) {
        wxArrayInt selrows = m_ctrlGridTransmission->GetSelectedRows();
        if (selrows.GetCount()) {
            for (int i = selrows.GetCount() - 1; i >= 0; i--) {
                TXLIST::compatibility_iterator node = 
                            m_CtrlObject.m_txList.Item(selrows[i]);
                VscpTXObj *obj = node->GetData();

                m_CtrlObject.m_mutexOutQueue.Lock();
                vscpEvent *pEvent = new vscpEvent;
                if (NULL != pEvent) {
                    vscp_copyVSCPEvent(pEvent, &obj->m_Event);
                    pEvent->head = 0x00;
                    pEvent->timestamp = 0x00L;
                    m_CtrlObject.m_outQueue.Append(pEvent);
                    m_CtrlObject.m_semOutQue.Post();
                }
                m_CtrlObject.m_mutexOutQueue.Unlock();

            }
        } 
        else {
            wxMessageBox(_("Must select one or more rows to transmit event(s)."));
        }
    } 
    else {
        wxMessageBox(_("Needs selected transmission row(s) to transmit event(s)."));
    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// OnTxAddClick
//

void frmVSCPSession::OnTxAddClick(wxCommandEvent& event)
{
    wxBusyCursor wait;
    dlgVSCPTrmitElement dlg(this);
    if (wxID_OK == dlg.ShowModal()) {

        VscpTXObj *pObj = new VscpTXObj;

        if (NULL != pObj) {
            if (dlg.getEventData(pObj)) {

                // Append to the TX list
                m_CtrlObject.m_txList.Append(pObj);

                // Add to TX grid
                addToTxGrid(pObj);

            } 
            else {
                wxMessageBox(_("Failed to get data from dialog."));
            }
        } 
        else {
            wxMessageBox(_("Failed to allocate data."));
        }
    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// OnTxEditClick
//

void frmVSCPSession::OnTxEditClick(wxCommandEvent& event)
{
    wxBusyCursor wait;
    dlgVSCPTrmitElement dlg(this);

    wxArrayInt selrows = m_ctrlGridTransmission->GetSelectedRows();
    if (m_ctrlGridTransmission->GetNumberRows() && selrows.GetCount()) {


        for (unsigned short i = 0; i < selrows.GetCount(); i++) {

            TXLIST::compatibility_iterator node = 
                    m_CtrlObject.m_txList.Item(selrows.Item(i));
            VscpTXObj *pObj = node->GetData();

            dlg.writeEventData(pObj);

            if (wxID_OK == dlg.ShowModal()) {

                dlg.getEventData(pObj);

                // Replace info on TX grid
                addToTxGrid(pObj, selrows.Item(i));

            }
        }
    } 
    else {
        wxMessageBox(_("No row selected so unable to edit."));
    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// OnTxDeleteClick
//

void frmVSCPSession::OnTxDeleteClick(wxCommandEvent& event)
{
    wxBusyCursor wait;
    if (m_ctrlGridTransmission->GetNumberRows()) {
        wxArrayInt selrows = m_ctrlGridTransmission->GetSelectedRows();
        if (selrows.GetCount()) {
            if (wxYES == wxMessageBox(_("Selected rows will be removed. Are you sure that is what is intended?"),
                    _("Remove rows?"),
                    wxYES_NO | wxICON_QUESTION)) {
                for (int i = selrows.GetCount() - 1; i >= 0; i--) {
                    m_ctrlGridTransmission->DeleteRows(selrows.Item(i), 1);
                    TXLIST::compatibility_iterator node = 
                            m_CtrlObject.m_txList.Item(i);
                    VscpTXObj *obj = node->GetData();
                    if (NULL != obj) delete obj;
                    m_CtrlObject.m_txList.DeleteNode(node);
                }
            }
        } 
        else {
            wxMessageBox(_("At least one row must be selected."));
        }
    } 
    else {
        wxMessageBox(_("There must be rows to delete to use this function."));
    }

    event.Skip(false);

}

////////////////////////////////////////////////////////////////////////////////
// OnTxLoadClick
//

void frmVSCPSession::OnTxLoadClick(wxCommandEvent& event)
{
    wxString str;
    //wxStandardPaths stdpaths;
    wxXmlDocument doc;

    wxBusyCursor wait;

    // If the grid contains row ask if we should clear it before
    // loading new data
    if (m_ctrlGridTransmission->GetNumberRows()) {
        if (wxYES == wxMessageBox(_("Should current transmission rows be removed before loading new set from file?"),
                _("Remove rows?"),
                wxYES_NO | wxICON_QUESTION)) {
            m_ctrlGridTransmission->DeleteRows( 0, 
                                    m_ctrlGridTransmission->GetNumberRows());

            TXLIST::iterator iter;
            for ( iter = m_CtrlObject.m_txList.begin(); 
                    iter != m_CtrlObject.m_txList.end(); ++iter) {
                VscpTXObj *obj = *iter;
                if (NULL != obj->m_Event.pdata) {
                    delete obj->m_Event.pdata;
                }
                delete obj;
            }

            m_CtrlObject.m_txList.Clear();

        }
    }

    // First find a path to save TX data to
    wxFileDialog dlg(this,
            _("Choose file to load transmission set from "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("txset"),
            _("TX Data Files (*.txd)|*.txd|XML Files (*.xml)|*.xml|All files (*.*)|*.*"));
    if (wxID_OK == dlg.ShowModal()) {


        if (!doc.Load(dlg.GetPath())) {
            return;
        }

        // start processing the XML file
        if (doc.GetRoot()->GetName() != wxT("vscptxset")) {
            return;
        }

        wxXmlNode *child = doc.GetRoot()->GetChildren();
        while (child) {

            if (child->GetName() == wxT("set")) {

                // Allocate a new transmission set
                VscpTXObj *pObj = new VscpTXObj;
                if (NULL == pObj) {
                    wxMessageBox(_("Unable to create transmission object."));
                    return;
                }

                wxXmlNode *subchild = child->GetChildren();
                while (subchild) {

                    if (subchild->GetName() == wxT("active")) {
                        str = subchild->GetNodeContent();
                        if (wxNOT_FOUND != str.Find(_("true"))) {
                            pObj->m_bActive = true;
                        } else {
                            pObj->m_bActive = false;
                        }
                    } 
                    else if (subchild->GetName() == wxT("name")) {
                        pObj->m_wxStrName = subchild->GetNodeContent();
                    } 
                    else if (subchild->GetName() == wxT("class")) {
                        str = subchild->GetNodeContent();
                        pObj->m_Event.vscp_class = vscp_readStringValue(str);
                    } 
                    else if (subchild->GetName() == wxT("type")) {
                        str = subchild->GetNodeContent();
                        pObj->m_Event.vscp_type = vscp_readStringValue(str);
                    } 
                    else if (subchild->GetName() == wxT("priority")) {
                        str = subchild->GetNodeContent();
                        vscp_setVscpPriority(&pObj->m_Event, vscp_readStringValue(str));
                    } 
                    else if (subchild->GetName() == wxT("guid")) {                        
                        wxString attribute = subchild->GetAttribute(wxT("default"), wxT("false"));
                   
                        if (attribute.IsSameAs(_("true"), false)) {
                            pObj->m_bUseDefaultGUID = true;
                        } 
                        else {
                            pObj->m_bUseDefaultGUID = false;
                        }
                        str = subchild->GetNodeContent();
                        vscp_getGuidFromString(&pObj->m_Event, str);
                    } 
                    else if (subchild->GetName() == wxT("data")) {
                        str = subchild->GetNodeContent();
                        vscp_setVscpEventDataFromString(&pObj->m_Event, str);
                    } 
                    else if (subchild->GetName() == wxT("count")) {
                        str = subchild->GetNodeContent();
                        pObj->m_count = vscp_readStringValue(str);
                    } 
                    else if (subchild->GetName() == wxT("period")) {
                        str = subchild->GetNodeContent();
                        pObj->m_period = vscp_readStringValue(str);
                    } 
                    else if (subchild->GetName() == wxT("trigger")) {
                        str = subchild->GetNodeContent();
                        pObj->m_trigger = vscp_readStringValue(str);
                    }

                    subchild = subchild->GetNext();

                } // while subchild

                // Append to the TX list
                m_CtrlObject.m_txList.Append(pObj);

                // Add to TX grid
                addToTxGrid(pObj);

            }

            child = child->GetNext();

        } // while child

    }

    event.Skip(false);
}


////////////////////////////////////////////////////////////////////////////////
// OnTxSaveClick
//

void frmVSCPSession::OnTxSaveClick(wxCommandEvent& event)
{
    wxString str;
    //wxStandardPaths stdpaths;

    wxBusyCursor wait;

    // First find a path to save TX data to
    wxFileDialog dlg(this,
            _("Choose file to save transmission set to "),
            wxStandardPaths::Get().GetUserDataDir(),
            _("txset"),
            _("TX Data Files (*.txd)|*.txd|XML Files (*.xml)|*.xml|All files (*.*)|*.*"),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (wxID_OK == dlg.ShowModal()) {

        wxFFileOutputStream *pFileStream = new wxFFileOutputStream(dlg.GetPath());
        if (NULL == pFileStream) {
            wxMessageBox(_("Failed to create transmission set file."));
            return;
        }

        pFileStream->Write("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"));

        // Transmission set start
        pFileStream->Write("<vscptxset>\n", strlen("<vscptxset>\n"));

        TXLIST::iterator iter;
        for (iter = m_CtrlObject.m_txList.begin(); iter != m_CtrlObject.m_txList.end(); ++iter) {

            VscpTXObj *obj = *iter;
            pFileStream->Write("<set>\n", strlen("<set>\n"));

            pFileStream->Write("<active>", strlen("<active>"));
            if (obj->m_bActive) {
                pFileStream->Write("true", strlen("true"));
            } 
            else {
                pFileStream->Write("false", strlen("false"));
            }

            pFileStream->Write("</active>\n", strlen("</active>\n"));

            pFileStream->Write("<name>", strlen("<name>"));
            pFileStream->Write(obj->m_wxStrName.mb_str(), strlen(obj->m_wxStrName.mb_str()));
            pFileStream->Write("</name>\n", strlen("</name>\n"));

            pFileStream->Write("<class>", strlen("<class>"));
            str.Printf(_("%d"), obj->m_Event.vscp_class);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</class>\n", strlen("</class>\n"));

            pFileStream->Write("<type>", strlen("<type>"));
            str.Printf(_("%d"), obj->m_Event.vscp_type);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</type>\n", strlen("</type>\n"));

            pFileStream->Write("<priority>", strlen("<priority>"));
            str.Printf(_("%d"), vscp_getVscpPriority(&obj->m_Event));
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</priority>\n", strlen("</priority>\n"));

            pFileStream->Write("<guid default=\"", strlen("<guid default=\""));
            if (obj->m_bUseDefaultGUID) {
                pFileStream->Write("true", strlen("true"));
            } 
            else {
                pFileStream->Write("false", strlen("false"));
            }

            pFileStream->Write("\" >", strlen("\" >"));
            vscp_writeGuidToString(&obj->m_Event, str);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</guid>\n", strlen("</guid>\n"));

            pFileStream->Write("<sizedata>", strlen("<sizedata>")); // Not used by read routine	
            str.Printf(_("%d"), obj->m_Event.sizeData);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</sizedata>\n", strlen("</sizedata>\n"));

            if (0 != obj->m_Event.sizeData) {
                pFileStream->Write("<data>", strlen("<data>"));
                vscp_writeVscpDataToString(&obj->m_Event, str);
                pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
                pFileStream->Write("</data>\n", strlen("</data>\n"));
            }

            pFileStream->Write("<count>", strlen("<count>"));
            str.Printf(_("%lu"), obj->m_count);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</count>\n", strlen("</count>\n"));

            pFileStream->Write("<period>", strlen("<period>"));
            str.Printf(_("%lu"), obj->m_period);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</period>\n", strlen("</period>\n"));

            pFileStream->Write("<trigger>", strlen("<trigger>"));
            str.Printf(_("%lu"), obj->m_trigger);
            pFileStream->Write(str.mb_str(), strlen(str.mb_str()));
            pFileStream->Write("</trigger>\n", strlen("</trigger>\n"));

            pFileStream->Write("</set>\n", strlen("</set>\n"));
        }

        // Transmission set end
        pFileStream->Write("</vscptxset>\n", strlen("</vscptxset>\n"));

        // Close the file
        pFileStream->Close();

        // Clean up
        delete pFileStream;

    }

    event.Skip(false);
}

////////////////////////////////////////////////////////////////////////////////
// OnTxCloneRow
//

void frmVSCPSession::OnTxCloneRow(wxCommandEvent& event)
{
    dlgVSCPTrmitElement dlg(this);

    wxArrayInt selrows = m_ctrlGridTransmission->GetSelectedRows();
    if (m_ctrlGridTransmission->GetNumberRows() && selrows.GetCount()) {

        for (unsigned short i = 0; i < selrows.GetCount(); i++) {

            TXLIST::compatibility_iterator node = 
                                m_CtrlObject.m_txList.Item(selrows.Item(i));
            VscpTXObj *pObj = node->GetData();

            VscpTXObj *pNewObj = new VscpTXObj;
            pNewObj->m_Event.pdata = NULL;
            pNewObj->m_pworkerThread = NULL; // Worker thread is note cloded
            pNewObj->m_bActive = false; // Object is inactive
            pNewObj->m_bUseDefaultGUID = pObj->m_bUseDefaultGUID;
            pNewObj->m_count = pObj->m_count;
            pNewObj->m_period = pObj->m_period;
            pNewObj->m_trigger = pObj->m_trigger;
            pNewObj->m_wxStrName = pObj->m_wxStrName;
            memcpy(&pNewObj->m_Event, &pObj->m_Event, sizeof( vscpEvent));
            if (pObj->m_Event.sizeData) {
                pNewObj->m_Event.pdata = new unsigned char[ pObj->m_Event.sizeData ];
                memcpy(pNewObj->m_Event.pdata, pObj->m_Event.pdata, pObj->m_Event.sizeData);
            } 
            else {
                pNewObj->m_Event.pdata = NULL;
            }

            // Append to the TX list
            m_CtrlObject.m_txList.Append(pNewObj);

            // Add to TX grid
            addToTxGrid(pNewObj);
        }
    } 
    else {
        wxMessageBox(_("No row selected so unable to clone any rows."));
    }

    event.Skip(false);
}

////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpVscpSiteClick
//

void frmVSCPSession::OnMenuitemVscpVscpSiteClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpHelpClick
//

void frmVSCPSession::OnMenuitemVscpHelpClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(_("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_docu"), wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpFaqClick
//

void frmVSCPSession::OnMenuitemVscpFaqClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/vscp_faq"), 
                                wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpShortcutsClick
//

void frmVSCPSession::OnMenuitemVscpShortcutsClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/vscpworks/vscpw_shortcuts"), 
                                wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpThanksClick
//

void frmVSCPSession::OnMenuitemVscpThanksClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks"), 
                                wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemVscpCreditsClick
//

void frmVSCPSession::OnMenuitemVscpCreditsClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser( _("http://www.vscp.org/wiki/doku.php/who_why_where/vscp_thanks#credits"), 
                                wxBROWSER_NEW_WINDOW);
    event.Skip(); //[wiki-url]
}


////////////////////////////////////////////////////////////////////////////////
// eventReceive
//

void frmVSCPSession::eventReceive(wxCommandEvent &event)
{
    VscpRXObj *pRecord = (VscpRXObj *) event.GetClientData();

    if ( ( NULL != pRecord ) && ( NULL != pRecord->m_pEvent ) ) {

        // Check if record should be filtered
        if ( m_bfilterActive ) {
            
            uint32_t searchValue;

            if ( ( FILTER_MODE_DISPLAY == m_nfilterMode ) && 
                    m_filterArrayDisplay.GetCount() ) {

                // * * * If event is in list it should be displayed * * *
                
                // First search for class wildcard
                searchValue = pRecord->m_pEvent->vscp_class<<16; 
                if ( wxNOT_FOUND != m_filterArrayDisplay.Index( searchValue ) ) {
                    // All events in this class should be displayed
                    m_pgridTable->writeEvent(pRecord);
                    return;
                }

                // Check if the event should be displays
                searchValue = ( pRecord->m_pEvent->vscp_class<<16 ) +  
                                                pRecord->m_pEvent->vscp_type; 
                if ( wxNOT_FOUND != m_filterArrayDisplay.Index( searchValue ) ) {
                    // This event should be displayed
                    m_pgridTable->writeEvent(pRecord);
                    return;
                }
            }
            else if ( ( FILTER_MODE_FILTER == m_nfilterMode ) && 
                                m_filterArrayFilter.GetCount() ) {
                
                // * * * If filter is in list it should not be displayed * * *

                // First search for class wildcard
                searchValue = pRecord->m_pEvent->vscp_class<<16; 
                if ( wxNOT_FOUND != m_filterArrayFilter.Index( searchValue ) ) {
                    // All events in this class should be filtered out
                    return;
                }

                // Check if the event should be displays
                searchValue = ( pRecord->m_pEvent->vscp_class<<16 ) +  
                                                pRecord->m_pEvent->vscp_type; 
                if ( wxNOT_FOUND != m_filterArrayFilter.Index( searchValue ) ) {
                    // This event is filtered out
                    return;
                }

                // Not filtered our - Display
                m_pgridTable->writeEvent(pRecord);

            }
            else {
                m_pgridTable->writeEvent(pRecord);
            }
        }
        else {        
            m_pgridTable->writeEvent(pRecord);
        }

    }
}

////////////////////////////////////////////////////////////////////////////////
// eventTransmit
//

void frmVSCPSession::eventTransmit(wxCommandEvent &event)
{
    VscpRXObj *pRecord = (VscpRXObj *) event.GetClientData();
    if (NULL != pRecord) {
        m_pgridTable->writeEvent(pRecord);
    }
}

////////////////////////////////////////////////////////////////////////////////
// eventPrepareConnect
//

void frmVSCPSession::eventPrepareConnect(wxCommandEvent &event)
{
    event.SetInt(1); // Avoid warning
    if (m_BtnActivateInterface->GetValue()) {
        // Show status
    }
}

////////////////////////////////////////////////////////////////////////////////
// eventConnected
//

void frmVSCPSession::eventConnected(wxCommandEvent &event)
{
    event.SetInt(1); // Avoid warning
    if ( m_BtnActivateInterface->GetValue() ) {
        // show progress
    }
}

////////////////////////////////////////////////////////////////////////////////
// eventLostCtrlIf
//

void frmVSCPSession::eventLostCtrlIf(wxCommandEvent &event)
{
    event.SetInt(1); // Avoid warning
    //wxMessageBox(_("Uanble to connect to server!"));

    if ( m_BtnActivateInterface->GetValue() ) {
        stopWorkerThreads();
    }
}

////////////////////////////////////////////////////////////////////////////////
// eventReceive
//

void frmVSCPSession::eventLostRcvIf(wxCommandEvent &event)
{
    event.SetInt(1); // Avoid warning
    if (m_BtnActivateInterface->GetValue()) {
        stopWorkerThreads();
    }
}


////////////////////////////////////////////////////////////////////////////////
// eventStatusChange
//

void frmVSCPSession::eventStatusChange( wxCommandEvent &evt )
{
    switch (evt.GetInt() ) { 

        case STATUSBAR_STATUS_RIGHT:
            m_pitemStatusBar->SetStatusText( evt.GetString(), 
                                                STATUSBAR_STATUS_RIGHT );   
            break;

        case STATUSBAR_STATUS_LEFT:
        default:
            m_pitemStatusBar->SetStatusText( evt.GetString(), 
                                                STATUSBAR_STATUS_LEFT );   
            break;
    }
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemSetFilterClick
//

void frmVSCPSession::OnMenuitemSetFilterClick( wxCommandEvent& event )
{
    uint16_t vscp_class;
    uint16_t vscp_type;

    dlgEventFilter dlg(this);

    // Filter active checkbox
    dlg.m_bfilterActive = m_bfilterActive;
    if ( m_bfilterActive ) {
        dlg.m_ctrlCheckBoxEnable->SetValue( true );
    }
    else {
        dlg.m_ctrlCheckBoxEnable->SetValue( false );
    }

    dlg.m_nfilterMode = m_nfilterMode;
    if ( FILTER_MODE_DISPLAY == m_nfilterMode ) {
        dlg.m_ctrlRadioDisplay->SetValue( true );
    }
    else if ( FILTER_MODE_FILTER == m_nfilterMode ) {
        dlg.m_ctrlRadioFilter->SetValue( true );
    }

    // Fill in values into display combo
    for ( size_t i=0; i<m_filterArrayDisplay.GetCount(); i++ ) {
        
        vscp_class = (m_filterArrayDisplay.Item(i)>>16) & 0xffff;
        vscp_type = m_filterArrayDisplay.Item(i) & 0xffff;

        wxString str = wxString::Format( _("%04X:%04X"), vscp_class, vscp_type);
        if ( 0 != vscp_type ) {
            str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             g_vscpinfo.getTypeDescription( vscp_class, vscp_type ) + 
                             _(" - ") +
                             str;
        }
        else {
            str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             _("[All events of class] - ") +
                             str;
        }
        
        dlg.m_ctrlListDisplay->Append( str, 
                        (void *)m_filterArrayDisplay.Item(i) ); // Yes I know

    }

    // Fill in values into filter combo
    for ( size_t i=0; i<m_filterArrayFilter.GetCount(); i++ ) {
        
        vscp_class = (m_filterArrayFilter.Item(i)>>16) & 0xffff;
        vscp_type = m_filterArrayFilter.Item(i) & 0xffff;

        wxString str = wxString::Format( _("%04X:%04X"), vscp_class, vscp_type);
        if ( 0 != vscp_type ) {
            str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             g_vscpinfo.getTypeDescription( vscp_class, 
                                                                vscp_type ) + 
                                                                _(" - ") +
                                                                str;
        }
        else {
            str = g_vscpinfo.getClassDescription( vscp_class ) + _(":") + 
                             _("[All events of class] - ") +
                             str;
        }
        
        dlg.m_ctrlListFilter->Append( str, 
                            (void *)m_filterArrayFilter.Item(i) ); // Yes I know 

    }


    if ( wxID_OK == dlg.ShowModal() ) {

        m_bfilterActive = dlg.m_bfilterActive;
        m_nfilterMode = dlg.m_nfilterMode;

        // Get filter values for display
        m_filterArrayDisplay.Clear();
        for ( uint16_t i=0; i<dlg.m_ctrlListDisplay->GetCount(); i++ ) {
            m_filterArrayDisplay.Add( (unsigned long)dlg.m_ctrlListDisplay->GetClientData( i ) );
        }

        // Get filter values for filter
        m_filterArrayFilter.Clear();
        for ( uint16_t i=0; i<dlg.m_ctrlListFilter->GetCount(); i++ ) {
            m_filterArrayFilter.Add( (unsigned long)dlg.m_ctrlListFilter->GetClientData( i ) );
        }

        if ( m_bfilterActive ) {
       
            if ( ( FILTER_MODE_DISPLAY == m_nfilterMode ) && 
                                ( 0 == m_filterArrayDisplay.GetCount() ) ) {
                m_BtnActivateFilter->SetValue(false);
            }
            else if ( FILTER_MODE_FILTER == m_nfilterMode && 
                                ( 0 == m_filterArrayFilter.GetCount() ) ) {
                m_BtnActivateFilter->SetValue(false);
            }
            else {
                m_BtnActivateFilter->SetValue(true);
            }
        }

        // Set button
        if ( m_bfilterActive ) {
            if ( ( FILTER_MODE_DISPLAY == m_nfilterMode ) && 
                                ( 0 == m_filterArrayDisplay.GetCount() ) ) {
                wxMessageBox(_("There is no class/type pairs defined to display. Filter will not be activated."));
                m_BtnActivateFilter->SetValue(false);
                m_bfilterActive = false;
            }
            else if ( FILTER_MODE_FILTER == m_nfilterMode && 
                                ( 0 == m_filterArrayFilter.GetCount() ) ) {
                wxMessageBox(_("There is no class/type pairs defined to filter.Filter will not be activated."));
                m_BtnActivateFilter->SetValue(false);
                m_bfilterActive  = false;
            }
            else {
                m_BtnActivateFilter->SetValue(true);
                m_bfilterActive = true;
            }
        }
        else {
            m_BtnActivateFilter->SetValue(false);
        }

    }

    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemSetBurstCountClick
//

void frmVSCPSession::OnMenuitemSetBurstCountClick( wxCommandEvent& event )
{
    wxMessageBox(_("Sorry this functionality is not available yet."));
    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemConfigurationClick
//

void frmVSCPSession::OnMenuitemConfigurationClick(wxCommandEvent& event)
{
    dlgConfiguration dlg(this);

    if (wxID_OK == dlg.ShowModal()) {
        dlg.getDialogData();
        m_ctrlGridReceive->ForceRefresh();
        m_ctrlGridTransmission->ForceRefresh();
    }
    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemSetAutoreplyClick
//

void frmVSCPSession::OnMenuitemSetAutoreplyClick(wxCommandEvent& event)
{

    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
// OnMenuitemSaveRxCellWidth
//

void frmVSCPSession::OnMenuitemSaveRxCellWidth(wxCommandEvent& event)
{
    int index   = 0;
    
    for(index = 0; index < VCSP_RSCV_FIELD_COUNT; ++index)
    {
        g_Config.m_VscpRcvFieldWidth[index] = 
                    m_ctrlGridReceive->GetColSize(index);
    }
    
    wxGetApp().writeConfiguration();
    event.Skip();
}

////////////////////////////////////////////////////////////////////////////////
// OnMenuitemSaveTxCellWidth
//

void frmVSCPSession::OnMenuitemSaveTxCellWidth(wxCommandEvent& event)
{
    int index   = 0;
    
    for(index = 0; index < VCSP_TRMIT_FIELD_COUNT; ++index)
    {
        g_Config.m_VscpTrmitFieldWidth[index] = 
                    m_ctrlGridTransmission->GetColSize(index);
    }
    
    wxGetApp().writeConfiguration();
    event.Skip();
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// TXWorkerThread
//

TXWorkerThread::TXWorkerThread(frmVSCPSession *pForm)
: wxThread(wxTHREAD_DETACHED)
{
    m_pCtrlObject = NULL;
    m_frmSession = pForm;
}

///////////////////////////////////////////////////////////////////////////////
// transmitWorkerThread
//

TXWorkerThread::~TXWorkerThread()
{
    m_frmSession->m_pTXWorkerThread = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to read from devices. Read it/them and send
// it/them to all other devices clients.
//

void *TXWorkerThread::Entry()
{
    eventOutQueue::compatibility_iterator node;
    vscpEvent *pEvent;

    wxCommandEvent eventConnectionLost( wxVSCP_CTRL_LOST_EVENT, 
                                            frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventPrepConnection( wxVSCP_RCV_PREP_CONNECT_EVENT, 
                                            frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventConnected( wxVSCP_RCV_CONNECTED_EVENT, 
                                            frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventStatus( wxVSCP_STATUS_CHANGE_EVENT, 
                                            frmVSCPSession::ID_FRMVSCPSESSION);
    eventStatus.SetInt( STATUSBAR_STATUS_LEFT );

    // Set timing
    m_tcpifControl.setResponseTimeout( g_Config.m_TCPIP_ResponseTimeout );
    m_tcpifControl.setAfterCommandSleep( g_Config.m_TCPIP_SleepAfterCommand );

    // Must be a valid control object pointer
    if (NULL == m_pCtrlObject) return NULL;

    eventStatus.SetString(_("TCP/IP: (TX) Open connection in progress."));
    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventPrepConnection);

    // Connect to the server with the control interface
    if ( VSCP_ERROR_SUCCESS != 
        m_tcpifControl.doCmdOpen( m_pCtrlObject->m_ifVSCP.m_strHost,
                                        m_pCtrlObject->m_ifVSCP.m_strUser,
                                        m_pCtrlObject->m_ifVSCP.m_strPassword)) {
        ::wxGetApp().logMsg( _( "VSCP TX thread - Unable to connect to server." ), 
                                VSCPWORKS_LOGMSG_CRITICAL );
        m_pCtrlObject->m_errorControl = VSCP_SESSION_ERROR_UNABLE_TO_CONNECT;
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        //wxQueueEvent( m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost );
        eventStatus.SetString(_("TCP/IP: Failed to open connection."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        eventStatus.SetString(_("TCP/IP: (TX) Failed to open connection."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnected);

    eventStatus.SetString(_("TCP/IP: (TX) Connection open."));
    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    // Get channel ID
    if (CANAL_ERROR_SUCCESS !=
        m_tcpifControl.doCmdGetChannelID( (uint32_t *)&m_pCtrlObject->m_txChannelID)) {
        ::wxGetApp().logMsg( _( "VSCP TX thread - Unable to get channel ID." ), VSCPWORKS_LOGMSG_INFO );
    }

    while ( !TestDestroy() && !m_pCtrlObject->m_bQuit && m_tcpifControl.isConnected() ) {

        if (wxSEMA_TIMEOUT == m_pCtrlObject->m_semOutQue.WaitTimeout(500)) continue;
        m_pCtrlObject->m_mutexOutQueue.Lock();
        node = m_pCtrlObject->m_outQueue.GetFirst();
        pEvent = node->GetData();
        m_tcpifControl.doCmdSend(pEvent);
        m_pCtrlObject->m_outQueue.DeleteNode(node);
        vscp_deleteVSCPevent(pEvent);
        m_pCtrlObject->m_mutexOutQueue.Unlock();

    } // while

    // Close the interface
    m_tcpifControl.doCmdClose();

    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);

    return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void TXWorkerThread::OnExit()
{
    // Nothing to do
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// receiveWorkerThread
//

RXWorkerThread::RXWorkerThread(frmVSCPSession *pForm )
: wxThread(wxTHREAD_DETACHED)
{
    m_pCtrlObject = NULL;
    m_frmSession = pForm;
}

///////////////////////////////////////////////////////////////////////////////
// receiveWorkerThread
//

RXWorkerThread::~RXWorkerThread()
{
    m_frmSession->m_pRXWorkerThread = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void *RXWorkerThread::Entry()
{
    int rv;
    //VscpRemoteTcpIf tcpifReceive;

    // Set timing
    m_tcpifReceive.setResponseTimeout( g_Config.m_TCPIP_ResponseTimeout );
    m_tcpifReceive.setAfterCommandSleep( g_Config.m_TCPIP_SleepAfterCommand );

    wxCommandEvent eventReceive(wxVSCP_IN_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventConnectionLost(wxVSCP_RCV_LOST_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventStatus(wxVSCP_STATUS_CHANGE_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    eventStatus.SetInt( STATUSBAR_STATUS_RIGHT );

    // Must be a valid control object pointer
    if (NULL == m_pCtrlObject) return NULL;

    eventStatus.SetString(_("TCP/IP: (RX) Open connection in progress."));
    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    // Connect to the server with the control interface
    if (VSCP_ERROR_SUCCESS != m_tcpifReceive.doCmdOpen(m_pCtrlObject->m_ifVSCP.m_strHost,
                                                        m_pCtrlObject->m_ifVSCP.m_strUser,
                                                        m_pCtrlObject->m_ifVSCP.m_strPassword)) {
        ::wxGetApp().logMsg( _( "TCP/IP Receive thread - Unable to connect to server." ), VSCPWORKS_LOGMSG_CRITICAL );
        m_pCtrlObject->m_errorReceive = VSCP_SESSION_ERROR_UNABLE_TO_CONNECT;
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("TCP/IP: (RX) Failed to open connection."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    eventStatus.SetString(_("TCP/IP: (RX) Connection open."));
    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    // Find the channel id
    m_tcpifReceive.doCmdGetChannelID( (uint32_t *) &m_pCtrlObject->m_rxChannelID );

    // Set filter
    if ( CANAL_ERROR_SUCCESS !=
        m_tcpifReceive.doCmdFilter(&m_pCtrlObject->m_ifVSCP.m_vscpfilter)) {
        ::wxGetApp().logMsg( _( "TCP/IP: (RX) Receive thread - Failed to set filter." ), VSCPWORKS_LOGMSG_INFO );
        eventStatus.SetString(_("TCP/IP: (RX) Receive thread - Failed to set filter."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
    }

    // Start Receive Loop
    m_tcpifReceive.doCmdEnterReceiveLoop();

    vscpEvent event;
    while ( !TestDestroy() && !m_pCtrlObject->m_bQuit && m_tcpifReceive.isConnected() ) {

        if (CANAL_ERROR_SUCCESS ==
                ( rv = m_tcpifReceive.doCmdBlockingReceive( &event, 10 ) ) ) {

            if ( NULL != m_pCtrlObject->m_pVSCPSessionWnd ) {

                VscpRXObj *pRecord = new VscpRXObj;

                if ( NULL != pRecord ) {

                    vscpEvent *pEvent = new vscpEvent;
                    if (NULL != pEvent) {

                        vscp_copyVSCPEvent( pEvent, &event );

                        pRecord->m_pEvent = pEvent;
                        pRecord->m_wxStrNote.Empty();
                        
                        wxString dt;
                        if ( vscp_getDateStringFromEvent( pEvent, dt ) ) {
                            if (!pRecord->m_time.ParseISOCombined( dt ) )  {
                                // Set to UTC
                                pRecord->m_time = wxDateTime::UNow();
                            }
                        }
                        else {
                            // Set to UTC
                            pRecord->m_time = wxDateTime::UNow();
                        }

                        if (pEvent->obid == m_pCtrlObject->m_txChannelID) {
                            pRecord->m_nDir = VSCP_EVENT_DIRECTION_TX;
                        } else {
                            pRecord->m_nDir = VSCP_EVENT_DIRECTION_RX;
                        }

                        eventReceive.SetClientData( pRecord );
                        wxPostEvent( m_pCtrlObject->m_pVSCPSessionWnd, eventReceive );

                        // Remove data if any
                        if (NULL != event.pdata) {
                            delete [] event.pdata;
                        }

                    } 
                    else {
                        delete pRecord;
                    }

                } // record exists

            } // Session window exist

        } else {
            if (CANAL_ERROR_COMMUNICATION == rv) m_pCtrlObject->m_bQuit = true;
        }

    } // while

    //wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);

    return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void RXWorkerThread::OnExit()
{
    // Close the interface
    m_tcpifReceive.doCmdClose();
}





///////////////////////////////////////////////////////////////////////////////
//                               D E V I C E
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// deviceThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

deviceThread::deviceThread()
: wxThread(wxTHREAD_DETACHED)
{
    m_pCtrlObject = NULL;
    m_preceiveThread = NULL;
    m_pwriteThread = NULL;
}

deviceThread::~deviceThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceThread::Entry()
{
    wxCommandEvent eventReceive(wxVSCP_IN_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventConnectionLost(wxVSCP_RCV_LOST_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventStatus(wxVSCP_STATUS_CHANGE_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    eventStatus.SetInt( STATUSBAR_STATUS_LEFT );

    // Must have a valid pointer to the control object
    if (NULL == m_pCtrlObject) return NULL;

    eventStatus.SetString(_("CANAL: Loading driver."));
    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    // Load dynamic library
    if (!m_wxdll.Load(m_pCtrlObject->m_ifCANAL.m_strPath, wxDL_LAZY)) {
        ::wxGetApp().logMsg( _T( "CANAL: Unable to load dynamic library." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to load dynamic library."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // Now find methods in library

    // * * * * CANAL OPEN * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalOpen =
            (LPFNDLL_CANALOPEN) m_wxdll.GetSymbol(_T("CanalOpen")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalOpen." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalOpen."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL CLOSE * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalClose =
            (LPFNDLL_CANALCLOSE) m_wxdll.GetSymbol(_T("CanalClose")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalClose." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalClose."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL GETLEVEL * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetLevel =
            (LPFNDLL_CANALGETLEVEL) m_wxdll.GetSymbol(_T("CanalGetLevel")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalGetLevel." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalGetLevel."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL SEND * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalSend =
            (LPFNDLL_CANALSEND) m_wxdll.GetSymbol(_T("CanalSend")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalSend." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalSend."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL DATA AVAILABLE * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalDataAvailable =
            (LPFNDLL_CANALDATAAVAILABLE) m_wxdll.GetSymbol(_T("CanalDataAvailable")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalDataAvailable." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalDataAvailable."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }


    // * * * * CANAL RECEIVE * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalReceive =
            (LPFNDLL_CANALRECEIVE) m_wxdll.GetSymbol(_T("CanalReceive")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalReceive." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalReceive."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL GET STATUS * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetStatus =
            (LPFNDLL_CANALGETSTATUS) m_wxdll.GetSymbol(_T("CanalGetStatus")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalGetStatus." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalGetStatus."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL GET STATISTICS * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetStatistics =
            (LPFNDLL_CANALGETSTATISTICS) m_wxdll.GetSymbol(_T("CanalGetStatistics")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalGetStatistics." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalGetStatistics."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL SET FILTER * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalSetFilter =
            (LPFNDLL_CANALSETFILTER) m_wxdll.GetSymbol(_T("CanalSetFilter")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalSetFilter." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalSetFilter."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL SET MASK * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalSetMask =
            (LPFNDLL_CANALSETMASK) m_wxdll.GetSymbol(_T("CanalSetMask")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalSetMask." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalSetMask."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL GET VERSION * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetVersion =
            (LPFNDLL_CANALGETVERSION) m_wxdll.GetSymbol(_T("CanalGetVersion")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalGetVersion." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalGetVersion."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL GET DLL VERSION * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetDllVersion =
            (LPFNDLL_CANALGETDLLVERSION) m_wxdll.GetSymbol(_T("CanalGetDllVersion")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalGetDllVersion." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalGetDllVersion."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    // * * * * CANAL GET VENDOR STRING * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetVendorString =
            (LPFNDLL_CANALGETVENDORSTRING) m_wxdll.GetSymbol(_T("CanalGetVendorString")))) {
        // Free the library
        ::wxGetApp().logMsg( _T( "CANAL: Unable to get dl entry for CanalGetVendorString." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalGetVendorString."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }


    wxLogNull logNo;


    // ******************************
    //     Generation 2 Methods
    // ******************************


    // * * * * CANAL BLOCKING SEND * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalBlockingSend =
            (LPFNDLL_CANALBLOCKINGSEND) m_wxdll.GetSymbol(_T("CanalBlockingSend")))) {
        ::wxGetApp().logMsg(_T("CANAL: Unable to get dl entry for CanalBlockingSend. Probably Generation 1 driver."),
                             VSCPWORKS_LOGMSG_CRITICAL );
        m_pCtrlObject->m_proc_CanalBlockingSend = NULL;
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalBlockingSend. Probably Generation 1 driver."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
    }

    // * * * * CANAL BLOCKING RECEIVE * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalBlockingReceive =
            (LPFNDLL_CANALBLOCKINGRECEIVE) m_wxdll.GetSymbol(_T("CanalBlockingReceive")))) {
        ::wxGetApp().logMsg(_T("CANAL: Unable to get dl entry for CanalBlockingReceive. Probably Generation 1 driver."),
                             VSCPWORKS_LOGMSG_CRITICAL );
        m_pCtrlObject->m_proc_CanalBlockingReceive = NULL;
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalBlockingReceive. Probably Generation 1 driver."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
    }

    // * * * * CANAL GET DRIVER INFO * * * *
    if (NULL == (m_pCtrlObject->m_proc_CanalGetdriverInfo =
            (LPFNDLL_CANALGETDRIVERINFO) m_wxdll.GetSymbol(_T("CanalGetDriverInfo")))) {
        ::wxGetApp().logMsg(_T("CANAL: Unable to get dl entry for CanalGetDriverInfo. Probably Generation 1 driver."),
                             VSCPWORKS_LOGMSG_CRITICAL );
        m_pCtrlObject->m_proc_CanalGetdriverInfo = NULL;
        eventStatus.SetString(_("CANAL: Unable to get dl entry for CanalGetDriverInfo. Probably Generation 1 driver."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
    }

    //
    // =====================================================================================
    //

    eventStatus.SetString(_("CANAL: Driver open in progress."));
    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    // Open the device
    m_pCtrlObject->m_openHandle =
            m_pCtrlObject->m_proc_CanalOpen((const char *) m_pCtrlObject->m_ifCANAL.m_strConfig.mb_str(wxConvUTF8),
            m_pCtrlObject->m_ifCANAL.m_flags);

    // Check if the driver opened properly
    if (0 == m_pCtrlObject->m_openHandle) {
        ::wxGetApp().logMsg( _T( "CANAL: Failed to open driver." ), VSCPWORKS_LOGMSG_CRITICAL );
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);
        eventStatus.SetString(_("CANAL: Failed to open driver."));
        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);
        return NULL;
    }

    eventStatus.SetString(_("CANAL: Driver open OK."));
    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    // Get Driver Level
    m_pCtrlObject->m_driverLevel = m_pCtrlObject->m_proc_CanalGetLevel(m_pCtrlObject->m_openHandle);


    // If this is a TCPIP level driver then nothing more then open and close should be done without
    // the driver. No messages should be put on any queues
    if (CANAL_LEVEL_USES_TCPIP == m_pCtrlObject->m_driverLevel) {
        // Just sit and wait until the end
        while (!TestDestroy() && !m_pCtrlObject->m_bQuit) {
            wxSleep(1);
        }
    } 
    else {

        if (NULL != m_pCtrlObject->m_proc_CanalBlockingReceive) {

            // * * * * Blocking version * * * *

            /////////////////////////////////////////////////////////////////////////////
            // Device write worker thread
            /////////////////////////////////////////////////////////////////////////////
            m_pwriteThread = new deviceWriteThread;

            if (m_pwriteThread) {
                m_pwriteThread->m_pMainThreadObj = this;
                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = m_pwriteThread->Create())) {
                    m_pwriteThread->SetPriority(WXTHREAD_MAX_PRIORITY);
                    if (wxTHREAD_NO_ERROR != (err = m_pwriteThread->Run())) {
                        ::wxGetApp().logMsg( _( "Unable to run device write worker thread." ), VSCPWORKS_LOGMSG_CRITICAL );
                    }
                } 
                else {
                    ::wxGetApp().logMsg( _( "Unable to create device write worker thread." ), VSCPWORKS_LOGMSG_CRITICAL );
                }
            } 
            else {
                ::wxGetApp().logMsg( _( "Unable to allocate memory for device write worker thread." ), VSCPWORKS_LOGMSG_CRITICAL );
            }

            /////////////////////////////////////////////////////////////////////////////
            // Device read worker thread
            /////////////////////////////////////////////////////////////////////////////
            m_preceiveThread = new deviceReceiveThread;

            if (m_preceiveThread) {
                m_preceiveThread->m_pMainThreadObj = this;
                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = m_preceiveThread->Create())) {
                    m_preceiveThread->SetPriority(WXTHREAD_MAX_PRIORITY);
                    if (wxTHREAD_NO_ERROR != (err = m_preceiveThread->Run())) {
                        ::wxGetApp().logMsg( _( "Unable to run device receive worker thread." ), VSCPWORKS_LOGMSG_CRITICAL );
                    }
                } 
                else {
                    ::wxGetApp().logMsg( _( "Unable to create device receive worker thread." ), VSCPWORKS_LOGMSG_CRITICAL );
                }
            } 
            else {
                ::wxGetApp().logMsg( _( "Unable to allocate memory for device receive worker thread." ), VSCPWORKS_LOGMSG_CRITICAL );
            }

            // Just sit and wait until the end
            while (!m_pCtrlObject->m_bQuit) {
                wxSleep(1);
            }

            m_preceiveThread->m_bQuit = true;
            m_pwriteThread->m_bQuit = true;
            m_preceiveThread->Wait();
            m_pwriteThread->Wait();
        } 
        else {

            // * * * * Non blocking version * * * *

            eventStatus.SetInt( STATUSBAR_STATUS_RIGHT );
            eventStatus.SetString(_("CANAL: Non blocking driver used."));
            wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

            bool bActivity;
            while (!TestDestroy() && !m_pCtrlObject->m_bQuit) {

                bActivity = false;
                /////////////////////////////////////////////////////////////////////////////
                //                           Receive from device								   				 //
                /////////////////////////////////////////////////////////////////////////////
                canalMsg msg;
                if (m_pCtrlObject->m_proc_CanalDataAvailable(m_pCtrlObject->m_openHandle)) {

                    m_pCtrlObject->m_proc_CanalReceive(m_pCtrlObject->m_openHandle, &msg);

                    bActivity = true;

                    vscpEvent *pEvent = new vscpEvent;
                    if (NULL != pEvent) {

                        // Convert CANAL message to VSCP event
                        vscp_convertCanalToEvent( pEvent,
                                                    &msg,
                                                    m_pCtrlObject->m_GUID);

                        VscpRXObj *pRecord = new VscpRXObj;
                        wxASSERT(NULL != pRecord);

                        pRecord->m_pEvent = pEvent;
                        pRecord->m_wxStrNote.Empty();
                        pRecord->m_time = wxDateTime::Now();

                        pRecord->m_nDir = VSCP_EVENT_DIRECTION_RX;

                        eventReceive.SetClientData(pRecord);
                        wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventReceive);

                    } // record exists

                }


                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                //             Send messages (if any) in the out queue
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                if  ( m_pCtrlObject->m_outQueue.GetCount() ) {

                    m_pCtrlObject->m_mutexOutQueue.Lock();

                    eventOutQueue::compatibility_iterator node =
                            m_pCtrlObject->m_outQueue.GetFirst();
                    vscpEvent *pEvent = node->GetData();

                    canalMsg canalMsg;
                    vscp_convertEventToCanal(&canalMsg, pEvent);
                    if (CANAL_ERROR_SUCCESS ==
                            m_pCtrlObject->m_proc_CanalSend(m_pCtrlObject->m_openHandle, &canalMsg)) {

                        VscpRXObj *pRecord = new VscpRXObj;
                        if (NULL != pRecord) {

                            pRecord->m_pEvent = pEvent;
                            pRecord->m_wxStrNote.Empty();
                            pRecord->m_time = wxDateTime::Now();

                            pRecord->m_nDir = VSCP_EVENT_DIRECTION_TX;

                            eventReceive.SetClientData(pRecord);
                            wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventReceive);
                        } // record 

                        // Remove the node
                        m_pCtrlObject->m_outQueue.DeleteNode(node);
                    } 
                    else {
                        // Give it another try
                        m_pCtrlObject->m_semOutQue.Post();
                    }

                    m_pCtrlObject->m_mutexOutQueue.Unlock();

                }

                if (!bActivity) {
                    ::wxMilliSleep(100);
                }

                bActivity = false;

            } // while working - non blocking

        } // if blocking/non blocking

    } // not TCPIP Driver

    // Close CANAL channel
    m_pCtrlObject->m_proc_CanalClose(m_pCtrlObject->m_openHandle);

    // Library is unloaded in destructor

    if (NULL != m_preceiveThread) {
        m_preceiveThread->Wait();
        delete m_preceiveThread;
    }

    if (NULL != m_pwriteThread) {
        m_pwriteThread->Wait();
        delete m_pwriteThread;
    }

    wxPostEvent(m_pCtrlObject->m_pVSCPSessionWnd, eventConnectionLost);

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceThread::OnExit()
{
    ;
}




// ****************************************************************************





///////////////////////////////////////////////////////////////////////////////
// deviceReceiveThread
//

deviceReceiveThread::deviceReceiveThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_pMainThreadObj = NULL;
    m_bQuit = false;
}

deviceReceiveThread::~deviceReceiveThread()
{
    m_pMainThreadObj->m_preceiveThread = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceReceiveThread::Entry()
{
    wxCommandEvent eventReceive(wxVSCP_IN_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventStatus(wxVSCP_STATUS_CHANGE_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    eventStatus.SetInt( STATUSBAR_STATUS_LEFT );
    canalMsg msg;

    // Must be a valid main object pointer
    if (NULL == m_pMainThreadObj) return NULL;

    // Blocking receive method must have been found
    if (NULL == m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingReceive) return NULL;

    eventStatus.SetString(_("CANAL: (RX) Open and working."));
    wxPostEvent(m_pMainThreadObj->m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    int rv;
    while (!TestDestroy() && !m_bQuit) {

        if ( CANAL_ERROR_SUCCESS ==
                ( rv = m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingReceive(
                            m_pMainThreadObj->m_pCtrlObject->m_openHandle,
                            &msg, 
                            500 ) ) ) {

            vscpEvent *pEvent = new vscpEvent;
            if (NULL != pEvent) {

                // Convert CANAL message to VSCP event
                vscp_convertCanalToEvent( pEvent,
                                            &msg,
                                            m_pMainThreadObj->m_pCtrlObject->m_GUID );

                VscpRXObj *pRecord = new VscpRXObj;
                if (NULL != pRecord) {

                    pRecord->m_pEvent = pEvent;
                    pRecord->m_wxStrNote.Empty();
                    pRecord->m_time = wxDateTime::Now();

                    pRecord->m_nDir = VSCP_EVENT_DIRECTION_RX;

                    eventReceive.SetClientData(pRecord);
                    wxPostEvent( m_pMainThreadObj->m_pCtrlObject->m_pVSCPSessionWnd,
                                    eventReceive);

                } 
                // record exists
                else {
                    delete pEvent;
                }

            }
        }        
    }

    eventStatus.SetString(_("CANAL: (RX) Terminated."));
    wxPostEvent(m_pMainThreadObj->m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceReceiveThread::OnExit()
{
    ;
}





// ****************************************************************************





///////////////////////////////////////////////////////////////////////////////
// deviceWriteThread
//

deviceWriteThread::deviceWriteThread()
: wxThread(wxTHREAD_DETACHED)
{
    m_pMainThreadObj = NULL;
    m_bQuit = false;
}

deviceWriteThread::~deviceWriteThread()
{
    m_pMainThreadObj->m_pwriteThread = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *deviceWriteThread::Entry()
{
    wxCommandEvent eventReceive(wxVSCP_IN_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    wxCommandEvent eventStatus(wxVSCP_STATUS_CHANGE_EVENT, frmVSCPSession::ID_FRMVSCPSESSION);
    eventStatus.SetInt( STATUSBAR_STATUS_RIGHT );
    eventOutQueue::compatibility_iterator node;
    vscpEvent *pEvent;

    // Must be a valid main object pointer
    if (NULL == m_pMainThreadObj) return NULL;

    // Blocking send method must have been found
    if (NULL == m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingSend) return NULL;

    eventStatus.SetString(_("CANAL: (TX) Open and working."));
    wxPostEvent(m_pMainThreadObj->m_pCtrlObject->m_pVSCPSessionWnd, eventStatus);

    while (!TestDestroy() && !m_bQuit) {

        if (wxSEMA_TIMEOUT == m_pMainThreadObj->m_pCtrlObject->m_semOutQue.WaitTimeout(500)) continue;
        m_pMainThreadObj->m_pCtrlObject->m_mutexOutQueue.Lock();

        node = m_pMainThreadObj->m_pCtrlObject->m_outQueue.GetFirst();
        pEvent = node->GetData();

        canalMsg canalMsg;
        vscp_convertEventToCanal(&canalMsg, pEvent);
        if (CANAL_ERROR_SUCCESS ==
                m_pMainThreadObj->m_pCtrlObject->m_proc_CanalBlockingSend( 
                                                        m_pMainThreadObj->m_pCtrlObject->m_openHandle, 
                                                        &canalMsg, 
                                                        300 ) ) {

            VscpRXObj *pRecord = new VscpRXObj;
            if (NULL != pRecord) {

                pRecord->m_pEvent = pEvent;
                pRecord->m_wxStrNote.Empty();
                pRecord->m_time = wxDateTime::Now();

                pRecord->m_nDir = VSCP_EVENT_DIRECTION_TX;

                eventReceive.SetClientData(pRecord);
                wxPostEvent( m_pMainThreadObj->m_pCtrlObject->m_pVSCPSessionWnd,
                                eventReceive);
            } // record 

            // Remove the node
            m_pMainThreadObj->m_pCtrlObject->m_outQueue.DeleteNode( node );

        } 
        else {
            // Give it another try
            m_pMainThreadObj->m_pCtrlObject->m_semOutQue.Post();
        }

        m_pMainThreadObj->m_pCtrlObject->m_mutexOutQueue.Unlock();

    } // while

    eventStatus.SetString(_("CANAL: (TX) Terminated."));
    wxPostEvent( m_pMainThreadObj->m_pCtrlObject->m_pVSCPSessionWnd, eventStatus );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void deviceWriteThread::OnExit()
{
    ;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OnHtmlwindowRcvinfoLinkClicked
//

void frmVSCPSession::OnHtmlwindowRcvinfoLinkClicked(wxHtmlLinkEvent& event)
{
    if ( event.GetLinkInfo().GetHref().StartsWith(_("http://") ) ) {
        wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
        event.Skip(false);
        return;
    }

    event.Skip();
}

