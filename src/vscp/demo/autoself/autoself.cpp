/////////////////////////////////////////////////////////////////////////////
// Name:        autoself.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 23 Apr 2007 21:41:07 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "autoself.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


// Configuration stuff
#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include "wx/wfstream.h"
#ifdef __WXMSW__
#include <wx/msw/regconf.h>
#endif

#include <wx/tokenzr.h>
#include <wx/socket.h>

#include "platform.h"

////@begin includes
////@end includes

#include "autoself.h"

////@begin XPM images
////@end XPM images

#include "frmmain.h"
#include "dlgselectunit.h"

// Application globals

_nodeinfo g_nodes[ MAX_NUMBER_OF_NODES ];
int g_cntNodes;
_appConfiguration g_Config;


/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( AutoselfApp )
////@end implement app

/*!
 * AutoselfApp type definition
 */

IMPLEMENT_CLASS( AutoselfApp, wxApp )

/*!
 * AutoselfApp event table definition
 */

BEGIN_EVENT_TABLE( AutoselfApp, wxApp )

////@begin AutoselfApp event table entries
////@end AutoselfApp event table entries

END_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////
// AutoselfApp
//
// Constructor for AutoselfApp
//

AutoselfApp::AutoselfApp()
{
	Init();
	wxSocketBase::Initialize();
}

/////////////////////////////////////////////////////////////////////////////
// Init
//
// Member initialisation
//

void AutoselfApp::Init()
{
////@begin AutoselfApp member initialisation
////@end AutoselfApp member initialisation

    int i,j;
    for ( i=0; i<MAX_NUMBER_OF_NODES; i++ ) {
        
        // Nill GUID
        for ( j=0; j<15; j++ ) {
            g_nodes[ i ].m_GUID[j] = 0;
        }
        
        // No IP Address / hostname
        g_nodes[ i ].m_strHostname = _T("");
        
        // No name
        g_nodes[ i ].m_strRealname = _T("");
        
        // No zone
        g_nodes[ i ].m_zone = 0;
        
    }
    
    g_cntNodes = 0; // No nodes yet
    

}

/////////////////////////////////////////////////////////////////////////////
// OnInit
//
// Initialisation for AutoselfA
//

bool AutoselfApp::OnInit()
{    
    // Get configuration from file
    loadConfigData();
    
////@begin AutoselfApp initialisation
	// Remove the comment markers above and below this block
	// to make permanent changes to the code.

#if wxUSE_XPM
	wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
	wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
	wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
	wxImage::AddHandler(new wxGIFHandler);
#endif
	frmMain* mainWindow = new frmMain;
	mainWindow->Create( NULL, frmMain::ID_FRMMAIN );
	mainWindow->Show(true);
////@end AutoselfApp initialisation

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// OnExit
//
// Cleanup for AutoselfApp
//

int AutoselfApp::OnExit()
{
    // Save configuration data
    saveConfigData();
        
////@begin AutoselfApp cleanup
	return wxApp::OnExit();
////@end AutoselfApp cleanup

}


/////////////////////////////////////////////////////////////////////////////
// loadConfigData
//
// Read persistant data from configuration file
// 
//

bool AutoselfApp::loadConfigData( void )
{
    wxString wxstr;
    long val;
        
#ifdef __WXMSW__
    wxChar sz[ MAX_PATH ];
    GetSystemDirectory( sz, sizeof( sz ) );
    wxString strPathSystemDir( sz );
    strPathSystemDir += _("\\autoself.conf");

    // Check if file exists
    if ( !::wxFileExists( strPathSystemDir ) ) {
        wxString strError;
        strError.Printf( _("Configuration file %s does not exists! A new file will be created when the program ends."), strPathSystemDir );
        wxMessageBox( strError );
        return FALSE;
    }

    wxFFileInputStream cfgstream( strPathSystemDir );

    if ( !cfgstream.Ok() ) {
        return false;
    }

    wxFileConfig *pconfig = new wxFileConfig( cfgstream );

#else
    wxConfig *pconfig = new wxConfig( _("autoself"),
                                         _("AUTOSELF"),
                                         _("autoself.conf"),
                                         _("autoself.conf"),
                                         wxCONFIG_USE_GLOBAL_FILE );
#endif

    pconfig->SetPath(_("/OPTIONS"));

/*!
    // bDisplayDeleteWarning
    m_bDisplayDeleteWarning = true;
    if ( pconfig->Read( "bDisplayDeleteWarning", &val, 0 ) ) {
        m_bDisplayDeleteWarning = ( val ? true : false);
    }
    wxLogDebug("Read Configuration: bDisplayDeleteWarning=%d", m_bDisplayDeleteWarning );
*/

    // sizeWidth
    pconfig->Read( _("sizeWidth"), &val, 640 );
		g_Config.m_sizeWidth = val;
    
    //wxLogDebug( _("Read Configuration: sizeWidth=%d"), g_Config.m_sizeWidth );

    // sizeHeight
    pconfig->Read( _("sizeHeight"), &val, 480 );
		g_Config.m_sizeHeight = val;
    //wxLogDebug( _("Read Configuration: sizeHeight=%d"), g_Config.m_sizeHeight );
    
    // xpos
    pconfig->Read( _("xpos"), &val, 0 );
		g_Config.m_xpos = val;
    
    // ypos
    pconfig->Read( _("ypos"), &val, 0 );
		g_Config.m_ypos = val;
    
    // Server
		pconfig->Read( _("server"), &wxstr, _("localhost") );
		g_Config.m_strServer = wxstr;
		wxLogDebug( wxstr );
	
		// Port
		pconfig->Read( _("port"), &wxstr, _("9598") );
		g_Config.m_strPort = wxstr;
		wxLogDebug( wxstr );
    
    // Username
		pconfig->Read( _("username"), &wxstr, _("admin") );
		g_Config.m_strUsername = wxstr;
	  wxLogDebug( wxstr );	
	
		// Password
		pconfig->Read( _("password"), &wxstr, _("password") );
		g_Config.m_strPassword = wxstr;
	  wxLogDebug( wxstr );	
    
    // Enable Logfile
    pconfig->Read( _("bEnableLogFile"), &g_Config.m_bLogFile, true );
    
    // Path to Logfile
#ifdef WIN32    
		pconfig->Read( _("path2logfile"), &wxstr, _("c:\\autoself.log") );
#else
    pconfig->Read( _("path2logfile"), &wxstr, _("/tmp/autoself.log") );
#endif    
		g_Config.m_strPathLogFile = wxstr;
	  wxLogDebug( wxstr );	
		
    pconfig->SetPath(_("/KNOWN_NODES"));
    int i;
    wxString wxbuf;
  
    for ( i=0; i<MAX_NUMBER_OF_NODES; i++ ) {
    
        wxbuf.Printf(_("node%d"),i);
        
        if ( pconfig->Read( wxbuf, &wxstr, _("") ) ) {
        
          wxStringTokenizer tok1, tok2;
          tok1.SetString( wxstr,_(",") );
          
          // Real name
          if ( tok1.HasMoreTokens() ) {
            g_nodes[ g_cntNodes ].m_strRealname = tok1.GetNextToken();
          }
          else {
            continue;
          }
          
          // GUID
          if ( tok1.HasMoreTokens() ) {
            wxstr = tok1.GetNextToken();
            
            tok2.SetString( wxstr, _(":") );
            long val;
            for ( int i=0; i<16; i++ ) {
              if ( tok2.HasMoreTokens() ) {
                wxstr = tok2.GetNextToken();
                wxstr.ToLong( &val, 16 );
                g_nodes[ g_cntNodes ].m_GUID[15 - i] = (unsigned char)val;
              }
              else {
                continue;
              }
            }
          }
          else {
            continue;
          }
          
          // IP Address / Hostname
          if ( tok1.HasMoreTokens() ) {
              g_nodes[ g_cntNodes ].m_strHostname = tok1.GetNextToken();
          }
          else {
            continue;
          }
          
          // Zone
          if ( tok1.HasMoreTokens() ) {
              long val;
              (tok1.GetNextToken()).ToLong( &val );
              g_nodes[ g_cntNodes ].m_zone = (unsigned char)val;
              g_cntNodes++;
          }
          else {
            continue;
          }
          
        }
        //wxLogDebug(_("Read Configuration: %s=%d"), wxbuf, g_Config.m_ypos );
    }
    
    if ( NULL!= pconfig ) delete pconfig;

    return true;

}


/////////////////////////////////////////////////////////////////////////////
// saveConfigData
// 
// Save persistant data to configuration file
// 
//

bool AutoselfApp::saveConfigData( void )
{
    wxString str;
        
#ifdef __WXMSW__
    wxChar sz[MAX_PATH];
    GetSystemDirectory( sz, sizeof( sz ) );
	wxString strPathSystemDir( sz );
	strPathSystemDir +=  _("\\autoself.conf");
#else
	wxString strPathSystemDir;
	strPathSystemDir = _("/etc/autoself.conf");
#endif

    wxFFileOutputStream cfgOutstream( strPathSystemDir );

	if ( !cfgOutstream.Ok() ) {
        wxMessageBox( _("Failed to create output stream to save config information."));
		return false;
	}


	wxFFileInputStream cfgstream( strPathSystemDir );

	if ( !cfgstream.Ok() ) {
        wxMessageBox( _("Failed to create stream to save config information."));
		return false;
	}

	wxFileConfig *pconfig = new wxFileConfig( cfgstream );

	pconfig->SetPath(_("/OPTIONS"));

/*
	// bDisplayDeleteWarning
	pconfig->Write( "bDisplayDeleteWarning", ( m_bDisplayDeleteWarning ? 1 : 0 ) );
*/ 
	// sizeWidth
	pconfig->Write( _("sizeWidth"), (long)g_Config.m_sizeWidth );
	
	// sizeHeight
	pconfig->Write( _("sizeHeight"), (long)g_Config.m_sizeHeight );
    
    // xpos
	pconfig->Write( _("xpos"), (long)g_Config.m_xpos );
    
    // ypos
	pconfig->Write( _("ypos"), (long)g_Config.m_ypos );
    
	// Server
	pconfig->Write( _("server"), g_Config.m_strServer );
	
	// Port
	pconfig->Write( _("port"), g_Config.m_strPort );
    
	// Username
	pconfig->Write( _("username"), g_Config.m_strUsername );
	
	// Password
	pconfig->Write( _("password"), g_Config.m_strPassword );
  
  // Enable logfile writes
  pconfig->Write( _("bEnableLogFile"), (long)g_Config.m_bLogFile );
  
  // Path to configuration file
	pconfig->Write( _("path2logfile"), g_Config.m_strPathLogFile );
	
	
  pconfig->SetPath(_("/KNOWN_NODES"));
  int i,j;
  wxString wxbuf;
  wxString wxName;
  wxString wxData;
  wxString wxNum;
    
  for ( i=0; i<g_cntNodes; i++ ) {
    
    wxName.Printf(_("node%d"), i );
        
    wxData = g_nodes[ i ].m_strRealname;
    wxData += _(",");
    for ( j=0; j<16; j++ ) {
      if ( 15 != j ) {
        wxNum.Printf( _("%d:"), g_nodes[ i ].m_GUID[15 - j] );
      }
      else {
        wxNum.Printf( _("%d"), g_nodes[ i ].m_GUID[15 - j] );
      }

      wxData += wxNum;

    }
        
    wxData += _(",");
    wxData += g_nodes[ i ].m_strHostname;

    wxData += _(",");  
    wxNum.Printf( _("%d"), g_nodes[ i ].m_zone);
    wxData += wxNum;

    pconfig->Write( wxName, wxData );

  }
      

	// Save changes
	pconfig->Save( cfgOutstream );

	if ( NULL!= pconfig ) delete pconfig;
     
  return true;

}

