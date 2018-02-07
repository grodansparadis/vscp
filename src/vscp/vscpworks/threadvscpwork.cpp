/////////////////////////////////////////////////////////////////////////////
// Name:        treadvscpwork.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 23 Apr 2007 21:41:07 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vscpworks.h"
#include "threadvscpwork.h"



///////////////////////////////////////////////////////////////////////////////
// TcpClientThread
//
// This thread communicates with a specific client.
//

VscpWorkThread::VscpWorkThread( wxThreadKind kind )
  : wxThread( kind )
{ 
    m_pfrmNode = NULL;	// No window to work for yet
}	


VscpWorkThread::~VscpWorkThread()
{ 
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *VscpWorkThread::Entry()
{
    unsigned char progress = 0;
    
/*
  wxCommandEvent eventUpdate( wxUPDATE_EVENT, frmNode::ID_FRMNODE );
  wxCommandEvent eventS1( wxS1_EVENT, frmNode::ID_FRMNODE );
  wxCommandEvent eventS2( wxS2_EVENT, frmNode::ID_FRMNODE );
    wxCommandEvent eventSB( wxSB_EVENT, frmNode::ID_FRMNODE );
    wxCommandEvent eventDrift( wxDRIFT_EVENT, frmNode::ID_FRMNODE );
    wxCommandEvent eventBlock( wxBLOCK_EVENT, frmNode::ID_FRMNODE );
    wxCommandEvent eventProgress( wxPROGRESS_EVENT, frmNode::ID_FRMNODE );
    wxCommandEvent eventTemperature( wxTEMPERATURE_EVENT, frmNode::ID_FRMNODE );
    wxCommandEvent eventVoltage( wxVOLTAGE_EVENT, frmNode::ID_FRMNODE );
    wxCommandEvent eventMessage( wxMESSAGE_EVENT, frmNode::ID_FRMNODE );
    wxCommandEvent eventLog( wxLOG_EVENT, frmNode::ID_FRMNODE );
    wxCommandEvent eventTerminate( wxTERMINATE_EVENT, frmNode::ID_FRMNODE );
*/

    //m_pfrmNode->GetEventHandler()->ProcessEvent( eventUpdate );
    //wxPostEvent(m_pfrmNode, eventUpdate);
    
// Debug section
#if 0
    
    int i=0;
  int a=0;
    
    eventTemperature.SetString(_("-38C") );
    wxPostEvent(m_pfrmNode, eventTemperature);
    eventVoltage.SetString(_("24V") );
    wxPostEvent(m_pfrmNode, eventVoltage);
    
    while ( m_pfrmNode->m_bRun ) {

        wxMilliSleep( 100 );
    
        eventProgress.SetInt( progress++ );
        wxPostEvent(m_pfrmNode, eventProgress );
        if ( progress > 100 ) {
            progress = 0;
        }		

    i++;
    if ( i>10 ) {
      i = 0;
            if ( a ) {
        a=0;
                eventS1.SetInt( LAMP_SEAGREEN );	
                wxPostEvent(m_pfrmNode, eventS1 );
      } 
      else {
        a=1;  
                eventS1.SetInt( LAMP_RED );
                wxPostEvent(m_pfrmNode, eventS1 );			
      }
    }
  }

#endif
    
    //eventProgress.SetInt( 100 );
    //wxPostEvent(m_pfrmNode, eventProgress );
    
    wxString strParam;
    //strParam = g_Config.strUsername + _(";");
    //strParam += g_Config.strPassword + _(";");
    //strParam += g_Config.strServer + _(";");
    //strParam += g_Config.strPort;

    // Connect to the server
    if ( m_vscptcpif.doCmdOpen( strParam ) ) {
    
        // Initialize UI
        //eventTemperature.SetString(_("----") );
        //wxPostEvent(m_pfrmNode, eventTemperature);
        
        //eventVoltage.SetString(_("----") );
        //wxPostEvent(m_pfrmNode, eventVoltage);
    
        //eventProgress.SetInt( 50 );
        //wxPostEvent(m_pfrmNode, eventProgress );
    
        vscpEventEx vscpEvent;
        while ( m_pfrmNode->m_bRun ) {
        
            if ( !m_vscptcpif.doCmdDataAvailable() ) {
                wxMilliSleep( 100 );
                continue;
            }
    
            if ( m_vscptcpif.doCmdReceiveEx(  &vscpEvent  ) ) {
            
                // Event received
                //eventProgress.SetInt( progress++ );
                //ŗwxPostEvent(m_pfrmNode, eventProgress );
                if ( progress > 100 ) {
                    progress = 0;
                }	
                
            }
            
        }	
    
        // Close the interface
        m_vscptcpif.doCmdClose();
    
    }
    else {
        //eventMessage.SetString(_("Unable to connect to server. Application will terminate.") );
        //wxPostEvent(m_pfrmNode, eventMessage );
        //wxPostEvent(m_pfrmNode, eventTerminate );
    }

  return NULL;
}





///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VscpWorkThread::OnExit()
{
  // Close the interface
    m_vscptcpif.doCmdClose();
}




