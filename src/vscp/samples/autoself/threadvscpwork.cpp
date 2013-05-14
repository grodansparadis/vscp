/////////////////////////////////////////////////////////////////////////////
// Name:        treadvscpwork.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 23 Apr 2007 21:41:07 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
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

#include "autoself.h"
#include "dlgselectunit.h"
#include "frmmain.h"
#include "threadvscpwork.h"
#include "frmNode.h"
#include "../../common/vscp_classes.h"
#include "../../common/vscp_types.h"

extern _nodeinfo g_nodes[];
extern int g_cntNodes;
extern _appConfiguration g_Config;



//extern WXTYPE wxUPDATE_EVENT;

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
  wxString wxstr;
	
	// events  
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
	wxCommandEvent eventTerminate( wxTERMINATE_EVENT, frmNode::ID_FRMNODE );

	//m_pfrmNode->GetEventHandler()->ProcessEvent( eventUpdate );
	wxPostEvent(m_pfrmNode, eventUpdate);
	
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
	
	eventProgress.SetInt( 100 );
	wxPostEvent(m_pfrmNode, eventProgress );
	
	wxString strParam;
	strParam = g_Config.m_strUsername + _(";");
	strParam += g_Config.m_strPassword + _(";");
	strParam += g_Config.m_strServer + _(";");
	strParam += g_Config.m_strPort;

  logmsg( _("Connecting to server...") );
  wxstr.Printf( _("---- GUID=%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d"),  
                g_nodes[ m_FrameId ].m_GUID[15],
                g_nodes[ m_FrameId ].m_GUID[14],
                g_nodes[ m_FrameId ].m_GUID[13],
                g_nodes[ m_FrameId ].m_GUID[12],
                g_nodes[ m_FrameId ].m_GUID[11],
                g_nodes[ m_FrameId ].m_GUID[10],
                g_nodes[ m_FrameId ].m_GUID[9],
                g_nodes[ m_FrameId ].m_GUID[8],
                g_nodes[ m_FrameId ].m_GUID[7],
                g_nodes[ m_FrameId ].m_GUID[6],
                g_nodes[ m_FrameId ].m_GUID[5],
                g_nodes[ m_FrameId ].m_GUID[4],
                g_nodes[ m_FrameId ].m_GUID[3],
                g_nodes[ m_FrameId ].m_GUID[2],
                g_nodes[ m_FrameId ].m_GUID[1],
                g_nodes[ m_FrameId ].m_GUID[0] );
  logmsg( wxstr );
  wxstr.Printf( _("---- Hostname=%s"), g_nodes[ m_FrameId ].m_strHostname.c_str() );
  logmsg( wxstr );
  wxstr.Printf( _("---- Zone=%d"), g_nodes[ m_FrameId ].m_zone );
  logmsg( wxstr );
  
	// Connect to the server
	if ( m_srvif.doCmdOpen( strParam ) ) {
  
    logmsg( _("Connected.") );
	
		// Initialize UI
		eventTemperature.SetString(_("----") );
		wxPostEvent(m_pfrmNode, eventTemperature);
		
		eventVoltage.SetString(_("----") );
		wxPostEvent(m_pfrmNode, eventVoltage);
	
		eventProgress.SetInt( 50 );
		wxPostEvent(m_pfrmNode, eventProgress );
	
		vscpEventEx vscpEvent;
		while ( m_pfrmNode->m_bRun ) {
		
			if ( !m_srvif.doCmdDataAvailable() ) {
				wxMilliSleep( 100 );
				continue;
			}
      
			while ( m_srvif.doCmdReceiveEx(  &vscpEvent  ) ) {
			
				// Event received
				eventProgress.SetInt( progress++ );
				wxPostEvent(m_pfrmNode, eventProgress );
				if ( progress > 100 ) {
					progress = 0;
				}	
 
 
        wxstr.Printf( _("Event received %d"), progress );
        logmsg( wxstr );
        
        // Log all events
        if ( 1 ) {
          
          wxstr.Printf( _("---- CLASS=%d TYPE=%d"), vscpEvent.vscp_class, vscpEvent.vscp_type );
          logmsg( wxstr );
          wxstr.Printf( _("---- GUID=%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X"),  
                vscpEvent.GUID[15],
                vscpEvent.GUID[14],
                vscpEvent.GUID[13],
                vscpEvent.GUID[12],
                vscpEvent.GUID[11],
                vscpEvent.GUID[10],
                vscpEvent.GUID[9],
                vscpEvent.GUID[8],
                vscpEvent.GUID[7],
                vscpEvent.GUID[6],
                vscpEvent.GUID[5],
                vscpEvent.GUID[4],
                vscpEvent.GUID[3],
                vscpEvent.GUID[2],
                vscpEvent.GUID[1],
                vscpEvent.GUID[0] );
          
          logmsg( wxstr );
          wxstr.Printf( _("---- DATACNT=%d DATA="), vscpEvent.sizeData );
          wxString wxstrbuf;
          for ( int i=0; i<vscpEvent.sizeData; i++ ) {
            wxstrbuf.Printf(_("%X "), vscpEvent.data[i] );
            wxstr += wxstrbuf;
          }
          
          logmsg( wxstr );
          
        }
        
        // CLASS1_INFOMATION TYPE=ON (0x03)
        if ( ( VSCP_CLASS_INFORMATION_EVENT == vscpEvent.vscp_class ) && 
            ( VSCP_TYPE_INFORMATION_EVENT_ON == vscpEvent.vscp_type ) &&
            ( g_nodes[ m_FrameId ].m_zone == vscpEvent.data[1] ) ) {
        
          switch ( vscpEvent.data[2] ) {
          
            case SUBZONE_S1:
              eventS1.SetInt( LAMP_GREEN );	
              wxPostEvent(m_pfrmNode, eventS1 );
              break;
              
            case SUBZONE_S2:
              eventS2.SetInt( LAMP_GREEN );	
              wxPostEvent(m_pfrmNode, eventS2 );
              break;
              
            case SUBZONE_SB:
              eventSB.SetInt( LAMP_RED );	
              wxPostEvent(m_pfrmNode, eventSB );
              break;
              
            case SUBZONE_BLOCK:
              eventBlock.SetInt( LAMP_RED );	
              wxPostEvent(m_pfrmNode, eventBlock );
              eventDrift.SetInt( LAMP_BLUE );	
              wxPostEvent(m_pfrmNode, eventDrift );
              break;
              
          }
        
        }

        // CLASS1_INFOMATION TYPE=OFF (0x04)
        if ( ( VSCP_CLASS_INFORMATION_EVENT == vscpEvent.vscp_class ) && 
            ( VSCP_TYPE_INFORMATION_EVENT_OFF == vscpEvent.vscp_type ) &&
            ( g_nodes[ m_FrameId ].m_zone == vscpEvent.data[1] ) ) {
        
          switch ( vscpEvent.data[2] ) {
          
            case SUBZONE_S1:
              eventS1.SetInt( LAMP_BLUE );	
              wxPostEvent(m_pfrmNode, eventS1 );
              break;
              
            case SUBZONE_S2:
              eventS2.SetInt( LAMP_BLUE );	
              wxPostEvent(m_pfrmNode, eventS2 );
              break;
              
            case SUBZONE_SB:
              eventSB.SetInt( LAMP_BLUE );	
              wxPostEvent(m_pfrmNode, eventSB );
              break;
              
            case SUBZONE_BLOCK:
              eventBlock.SetInt( LAMP_BLUE );	
              wxPostEvent(m_pfrmNode, eventBlock );
              eventDrift.SetInt( LAMP_GREEN );	
              wxPostEvent(m_pfrmNode, eventDrift );
              break;
              
          }
        
        }
				
			}
			
		}	
	
		// Close the interface
		m_srvif.doCmdClose();
    
    logmsg(_("Connection closed.") );
	
	}
	else {
  
    logmsg(_("Unable to connect to server.") );
    logmsg( strParam );
    
		eventMessage.SetString(_("Unable to connect to server. Application will terminate.") );
		wxPostEvent(m_pfrmNode, eventMessage );
		wxPostEvent(m_pfrmNode, eventTerminate );
    
	}

  return NULL;
}





///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VscpWorkThread::OnExit()
{
	// Close the interface
	m_srvif.doCmdClose();
}


///////////////////////////////////////////////////////////////////////////////
// logmsg
//

void VscpWorkThread::logmsg( wxString str )
{
  wxCommandEvent eventLog( wxMAINLOG_EVENT, frmMain::ID_FRMMAIN );
  
  eventLog.SetInt( m_FrameId );
  eventLog.SetString( _("[") + m_strFrameName + _("] ")+ str );
  wxPostEvent( m_pfrmNode, eventLog );
}


