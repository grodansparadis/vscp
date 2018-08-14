/////////////////////////////////////////////////////////////////////////////
// Name:        vscpworks.cpp
// Purpose:
// Author:      Ake Hedman
// Modified by:
// Created:     Thu 28 Jun 2007 10:05:16 CEST
// RCS-ID:
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// 
//	This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//	You should have received a copy of the GNU General Public License
//	along with this file see the file COPYING.  If not, write to
//	the Free Software Foundation, 59 Temple Place - Suite 330,
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
#pragma implementation "vscpworks.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
//#include "wx/wxprec.h"

//#include <vld.h>

// https://msdn.microsoft.com/en-us/library/x98tx3cf.aspx
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Configuration stuff
#include <wx/config.h> 
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/debug.h> 

#ifdef __WXMSW__
#include <wx/msw/regconf.h>
#endif

#ifndef __WXMSW__
#include <syslog.h>
#endif

#include <wx/tokenzr.h>
#include <wx/socket.h>
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/progdlg.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/imaglist.h>
#include <wx/url.h>

// This file links platform dependent stuff
#include "platform.h"
#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <vscpmulticast.h>
#include <mdf.h>
#include <frmdeviceconfig.h>
#include <frmvscpsession.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include "vscpworks.h"

#include <sockettcp.h>

// Global nodes info
canal_nodeinfo g_nodesCANAL[ MAX_NUMBER_OF_NODES ];
vscp_nodeinfo g_nodesVSCP[ MAX_NUMBER_OF_NODES ];
int g_cntCanalNodes;
int g_cntVscpNodes;

appConfiguration g_Config;
VSCPInformation g_vscpinfo;

// Lists for interfaces
WX_DEFINE_LIST(LIST_CANAL_IF);
WX_DEFINE_LIST(LIST_VSCP_IF);

/*!
* Application instance implementation
*/

IMPLEMENT_APP( VscpworksApp ) 


/*!
* VscpworksApp type definition
*/

IMPLEMENT_CLASS( VscpworksApp, wxApp )


/*!
* VscpworksApp event table definition
*/

BEGIN_EVENT_TABLE( VscpworksApp, wxApp )


END_EVENT_TABLE()

/*!
* Constructor for VscpworksApp
*/

VscpworksApp::VscpworksApp()
{
    // Fix for menu problems in UBUNTU
    // http://trac.wxwidgets.org/ticket/14302
    wxSetEnv(wxT("UBUNTU_MENUPROXY"), wxT("0"));

    // Set assert handler
#if (wxMAJOR_VERSION >= 3)
    wxSetAssertHandler( VscpworksApp::AssertHandler );
#endif
    
    // net_skeleton generates on no receiving socket at other end
    // we can safely dismiss.
#ifdef WIN32	
    WSADATA wsaData;                    // Windows socket DLL structure 

    // Load Winsock 2.0 DLL
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) != 0 ) {
        fprintf( stderr, "Multicast WSAStartup() failed" );
    }
#else
    //int set = 1;
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
    signal(SIGPIPE, SIG_IGN);
#endif	
    
    Init();
    wxSocketBase::Initialize();
}

char *reverse(const char * const s)
{
  if ( NULL == s ) return NULL;
  
  size_t i, len = strlen(s);
  char *r = (char *)malloc(len + 1);

  for(i = 0; i < len; ++i) {
    r[i] = s[len - i - 1];
  }
  
  r[len] = 0;
  return r;
}

char *rstrstr( const char *s1, const char *s2)
{
  size_t  s1len = strlen(s1);
  size_t  s2len = strlen(s2);
  char *s;

  if (s2len > s1len) return NULL;
  
  for (s = (char *)s1 + s1len - s2len; s >= s1; --s) {
    if ( 0 == strncmp(s, s2, s2len) ) {
      return s;
    }
  }
  
  return NULL;
}


/*!
* Member initialisation
*/

void VscpworksApp::Init()
{

    /*
    // Test filter
    vscpEvent Event;
    vscpEventFilter Filter;


    Event.vscp_type = 0x0001;
    Event.pdata = NULL;

    ClearVSCPFilter( &Filter );

    Event.vscp_class = 0x9598;

    Filter.filter_class = 0x9598;
    Filter.mask_class = 0xffff;

    uint16_t or_func = Filter.filter_class ^ Event.vscp_class;
    uint16_t not_or_func = ~or_func;
    uint16_t result = not_or_func | ( ~Filter.mask_class );

    uint16_t result2 = ( ~( Filter.filter_class ^ Event.vscp_class ) | ~Filter.mask_class );

    if ( 0xffff != (uint16_t)( ~( Filter.filter_class ^ Event.vscp_class ) | ~Filter.mask_class ) ) {
    result=1;
    }
    else {
    result=2; // Deliver
    }

    if ( doLevel2Filter( &Event, &Filter ) ) {
    result=3;	// Deliver
    }
    else {
    result=3;
    }
    */

    m_pmulticastWorkerThread = NULL;

    m_bUseGlobalConfig = false; // Store config in users folder

    g_Config.m_strPathLogFile = wxStandardPaths::Get().GetTempDir();
    g_Config.m_strPathLogFile += _("\\vscpworks.log");
    g_Config.m_strPathTemp = wxStandardPaths::Get().GetTempDir();



    int i,j;
    for ( i=0; i<MAX_NUMBER_OF_NODES; i++ ) {

        // Nill GUID
        for ( j=0; j<15; j++ ) {
            g_nodesCANAL[ i ].m_GUID[j] = 0;
            g_nodesVSCP[ i ].m_GUID[j] = 0;
        }

        // No IP Address / hostname
        g_nodesCANAL[ i ].m_strHostname = _T("");
        g_nodesVSCP[ i ].m_strHostname = _T("");

        // No name
        g_nodesCANAL[ i ].m_strRealname = _T("");
        g_nodesVSCP[ i ].m_strRealname = _T("");

        // No zone
        g_nodesCANAL[ i ].m_zone = 0;
        g_nodesVSCP[ i ].m_zone = 0;

    }

    g_cntCanalNodes = 0;  // No nodes yet
    g_cntVscpNodes = 0;   // No nodes yet

    g_Config.m_sizeMainFrameWidth = VSCPWORKS_MAIN_WINDOW_DEFAULT_WIDTH;
    g_Config.m_sizeMainFrameHeight = VSCPWORKS_MAIN_WINDOW_DEFAULT_HEGHT;

    g_Config.m_sizeConfigurationFrameWidth = VSCPWORKS_DEVICECONFIG_SIZE.GetWidth();
    g_Config.m_sizeConfigurationFrameHeight = VSCPWORKS_DEVICECONFIG_SIZE.GetHeight();

    g_Config.m_sizeSessionFrameWidth = VSCPWORKS_SESSION_SIZE.GetWidth();
    g_Config.m_sizeSessionFrameHeight = VSCPWORKS_SESSION_SIZE.GetHeight();

    g_Config.m_VscpTrmitShowField[0] = true;
    g_Config.m_VscpTrmitShowField[1] = true;
    g_Config.m_VscpTrmitShowField[2] = true;
    g_Config.m_VscpTrmitShowField[3] = true;
    g_Config.m_VscpTrmitShowField[4] = true;
    g_Config.m_VscpTrmitShowField[5] = true;

    g_Config.m_VscpTrmitFieldOrder[0] = 0;
    g_Config.m_VscpTrmitFieldOrder[1] = 1;
    g_Config.m_VscpTrmitFieldOrder[2] = 2;
    g_Config.m_VscpTrmitFieldOrder[3] = 3;
    g_Config.m_VscpTrmitFieldOrder[4] = 4;
    g_Config.m_VscpTrmitFieldOrder[5] = 5;

    g_Config.m_VscpTrmitFieldText[0] = VCSP_TRMIT_FIELD_TEXT_0;
    g_Config.m_VscpTrmitFieldText[1] = VCSP_TRMIT_FIELD_TEXT_1;
    g_Config.m_VscpTrmitFieldText[2] = VCSP_TRMIT_FIELD_TEXT_2;
    g_Config.m_VscpTrmitFieldText[3] = VCSP_TRMIT_FIELD_TEXT_3;
    g_Config.m_VscpTrmitFieldText[4] = VCSP_TRMIT_FIELD_TEXT_4;
    g_Config.m_VscpTrmitFieldText[5] = VCSP_TRMIT_FIELD_TEXT_5;

    g_Config.m_VscpTrmitFieldWidth[0] = VCSP_TRMIT_FIELD_WIDTH_0;
    g_Config.m_VscpTrmitFieldWidth[1] = VCSP_TRMIT_FIELD_WIDTH_1;
    g_Config.m_VscpTrmitFieldWidth[2] = VCSP_TRMIT_FIELD_WIDTH_2;
    g_Config.m_VscpTrmitFieldWidth[3] = VCSP_TRMIT_FIELD_WIDTH_3;
    g_Config.m_VscpTrmitFieldWidth[4] = VCSP_TRMIT_FIELD_WIDTH_4;
    g_Config.m_VscpTrmitFieldWidth[5] = VCSP_TRMIT_FIELD_WIDTH_5;

    g_Config.m_VscpRcvShowField[0] = true; 
    g_Config.m_VscpRcvShowField[1] = true;
    g_Config.m_VscpRcvShowField[2] = true;
    g_Config.m_VscpRcvShowField[3] = true;

    g_Config.m_VscpRcvFieldOrder[0] = 0;
    g_Config.m_VscpRcvFieldOrder[1] = 1;
    g_Config.m_VscpRcvFieldOrder[2] = 2;
    g_Config.m_VscpRcvFieldOrder[3] = 3;

    g_Config.m_VscpRcvFieldWidth[0] = VCSP_RSCV_FIELD_WIDTH_0; 
    g_Config.m_VscpRcvFieldWidth[1] = VCSP_RSCV_FIELD_WIDTH_1;
    g_Config.m_VscpRcvFieldWidth[2] = VCSP_RSCV_FIELD_WIDTH_2;
    g_Config.m_VscpRcvFieldWidth[3] = VCSP_RSCV_FIELD_WIDTH_3;

    g_Config.m_VscpRcvFieldText[ 0 ] = VCSP_RSCV_FIELD_TEXT_0;
    g_Config.m_VscpRcvFieldText[ 1 ] = VCSP_RSCV_FIELD_TEXT_1;
    g_Config.m_VscpRcvFieldText[ 2 ] = VCSP_RSCV_FIELD_TEXT_2;
    g_Config.m_VscpRcvFieldText[ 3 ] = VCSP_RSCV_FIELD_TEXT_3;

    g_Config.m_VscpRcvFrameRxTextColour = wxColour( 0x00, 0x00, 0x80 );
    g_Config.m_VscpRcvFrameRxBgColour = wxColour( 0xff, 0xff, 0xff );
    g_Config.m_VscpRcvFrameTxTextColour = wxColour( 0x80, 0x80, 0x80 );
    g_Config.m_VscpRcvFrameTxBgColour = wxColour( 0xff, 0xff, 0xff );
    g_Config.m_VscpRcvFrameLineColour = wxColour( 0x00, 0x00, 0x00 );

    g_Config.m_VscpRcvFrameRowLineHeight = VCSP_RSCV_FIELD_DEFAULT_HEIGHT;

    g_Config.m_bAutoscollRcv = true;          // Yes autoscroll
    g_Config.m_VscpRcvFrameRxbPyamas = true;  // Yes use pyamas look
    g_Config.m_UseSymbolicNames = true;       // Yes use symbolic names

    g_Config.m_CANALRegResendTimeout = VSCP_CANAL_RESEND_TIMEOUT;
    g_Config.m_CANALRegErrorTimeout = VSCP_CANAL_ERROR_TIMEOUT;
    g_Config.m_CANALRegMaxRetries = VSCP_CANAL_MAX_TRIES;

    g_Config.m_TCPIP_ResponseTimeout = VSCPWORKS_TCPIP_DEFAULT_RESPONSE_TIMEOUT;
    g_Config.m_TCPIPRegMaxRetries = VSCPWORKS_TCPIP_REGISTER_READ_MAX_TRIES;
    g_Config.m_TCPIPRegResendTimeout = VSCPWORKS_TCPIP_REGISTER_READ_RESEND_TIMEOUT;
    g_Config.m_TCPIPRegErrorTimeout = VSCPWORKS_TCPIP_REGISTER_READ_ERROR_TIMEOUT;

    g_Config.m_Numberbase = VSCP_DEVCONFIG_NUMBERBASE_HEX;

    g_Config.m_bConfirmDelete = true;

    g_Config.bGuidWritable = false;
    g_Config.m_manufacturerId = 0;
    g_Config.m_manufacturerSubId = 0;
    
    /*
     
     Testblock for sockettcp lib
     
    char errbuf[200];
    char buf[8192];
    stcp_connection *conn;
    conn = stcp_connect_client( "185.144.156.45", 9598, 0, errbuf, sizeof( errbuf ), 5 );
    if ( NULL != conn ) {
        
        stcp_read( conn, buf, sizeof( buf ), 200 );
        wxPrintf( buf );
        
        stcp_write( conn, "user admin\r\n", 12 );
        stcp_read( conn, buf, sizeof( buf ), 200 );
        wxPrintf( buf );
        
        stcp_write( conn, "pass secret\r\n", 13 );
        stcp_read( conn, buf, sizeof( buf ), 300 );
        wxPrintf( buf );
        
        stcp_write( conn, "interface list\r\n", 16 );
        stcp_read( conn, buf, sizeof( buf ), 200 );
        wxPrintf( buf );
        
        stcp_close_connection( conn );
        
        conn = NULL;
    }
    */ 
    
}

/*!
* Initialisation for VscpworksApp
*/

bool VscpworksApp::OnInit()
{    
    // Get the configuration
    readConfiguration();

    /////////////////////////////////////////////////////////////////////////////
    //                  Start the mulicast worker thread
    /////////////////////////////////////////////////////////////////////////////
    m_pmulticastWorkerThread = new worksMulticastThread;

    if ( NULL != m_pmulticastWorkerThread ) {
        //m_pmulticastWorkerThread->m_pApp = this;
        wxThreadError err;
        if ( wxTHREAD_NO_ERROR == ( err = m_pmulticastWorkerThread->Create() ) ) {
            m_pmulticastWorkerThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            if ( wxTHREAD_NO_ERROR != ( err = m_pmulticastWorkerThread->Run() ) ) {
                delete m_pmulticastWorkerThread;
                m_pmulticastWorkerThread = NULL;
                wxMessageBox( _( "Unable to run multicast discovery thread. Functionality is disabled." ) );
            }
        }
        else {
            delete m_pmulticastWorkerThread;
            m_pmulticastWorkerThread = NULL;
            wxMessageBox( _( "Unable to create multicast discovery thread. Functionality is disabled." ) );
        }
    }
    else {
        wxMessageBox( _( "Unable to create multicast discovery thread. Functionality is disabled." ) );
    }

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
    mainWindow->Create( NULL );
    mainWindow->Show(true);

    //Move( g_Config.m_xpos, g_Config.m_ypos );
    mainWindow->SetSize(  g_Config.m_xposMainFrame, 
                            g_Config.m_yposMainFrame, 
                            g_Config.m_sizeMainFrameWidth, 
                            g_Config.m_sizeMainFrameHeight );
    return true;
}


/*!
* Cleanup for VscpworksApp
*/

int VscpworksApp::OnExit()
{ 
    // Save the configuration
    writeConfiguration();

    // Clean up CANAL i/f list
    LIST_CANAL_IF::iterator iterCanal;
    for (iterCanal = g_Config.m_canalIfList.begin(); 
        iterCanal != g_Config.m_canalIfList.end(); 
        ++iterCanal) {
            canal_interface *current = *iterCanal;
            if ( NULL != current ) delete current;
    }
    g_Config.m_canalIfList.clear();

    // Clean up VSCP i/f list
    LIST_VSCP_IF::iterator iterVSCP;
    for (iterVSCP = g_Config.m_vscpIfList.begin(); 
        iterVSCP != g_Config.m_vscpIfList.end(); 
        ++iterVSCP) {
            vscp_interface *current = *iterVSCP;
            if ( NULL != current ) delete current;
    }
    g_Config.m_vscpIfList.clear();

    // Terminate multicast discovery system
    if ( NULL != m_pmulticastWorkerThread ) {
        m_mutexmulticastWorkerThread.Lock();
        m_pmulticastWorkerThread->m_bQuit = true;
        m_pmulticastWorkerThread->Wait();
        delete m_pmulticastWorkerThread;
        m_pmulticastWorkerThread = NULL;
        m_mutexmulticastWorkerThread.Unlock();
    }

#ifdef WIN32
    // Cleanup Winsock
    WSACleanup();
#endif

    //_CrtDumpMemoryLeaks();

    return wxApp::OnExit();
}


/////////////////////////////////////////////////////////////////////////////
// logMsg
//

void VscpworksApp::logMsg( const wxString& wxstr, unsigned char level )
{
#ifndef WIN32
    (void)level;    // Suppress warning
#endif

    wxString wxdebugmsg;
    wxdebugmsg = _("vscpd :") + wxstr;

    wxLogDebug( wxdebugmsg );

#ifndef WIN32

    switch ( level ) {
        case VSCPWORKS_LOGMSG_DEBUG:
            syslog( LOG_DEBUG, "%s", (const char *)wxdebugmsg.ToAscii()  );
            break;

        case VSCPWORKS_LOGMSG_INFO:
            syslog( LOG_INFO, "%s", (const char *)wxdebugmsg.ToAscii() );
            break;

        case VSCPWORKS_LOGMSG_NOTICE:
            syslog( LOG_NOTICE, "%s", (const char *)wxdebugmsg.ToAscii() );
            break;

        case VSCPWORKS_LOGMSG_WARNING:
            syslog( LOG_WARNING, "%s", (const char *)wxdebugmsg.ToAscii() );
            break;

        case VSCPWORKS_LOGMSG_ERROR:
            syslog( LOG_ERR, "%s", (const char *)wxdebugmsg.ToAscii() );
            break;

        case VSCPWORKS_LOGMSG_CRITICAL:
            syslog( LOG_CRIT, "%s", (const char *)wxdebugmsg.ToAscii() );
            break;

        case VSCPWORKS_LOGMSG_ALERT:
            syslog( LOG_ALERT, "%s", (const char *)wxdebugmsg.ToAscii() );
            break;

        case VSCPWORKS_LOGMSG_EMERGENCY:
            syslog( LOG_EMERG, "%s", (const char *)wxdebugmsg.ToAscii() );
            break;

    };
#endif
}


/////////////////////////////////////////////////////////////////////////////
// AssertHandler
//
#if (wxMAJOR_VERSION >= 3)
void VscpworksApp::AssertHandler( const wxString &file, 
                                        int line, 
                                        const wxString &func, 
                                        const wxString &cond, 
                                        const wxString &msg )
{
    wxString logmsg = _("ASSERT - ");
    logmsg += _( " file: " ) + file;
    logmsg += wxString::Format( _( " line:%d" ), line );
    logmsg += _( " func: " ) + func;
    logmsg += _( " cond: " ) + cond;
    logmsg += _( " msg: " ) + msg;

    ::wxGetApp().logMsg( logmsg, 
                        VSCPWORKS_LOGMSG_ALERT );
}
#endif

/////////////////////////////////////////////////////////////////////////////
// loadConfigData
//
// Read persistant data from configuration file
// 
//
/*
bool VscpworksApp::loadConfigData( void )
{
wxString wxstr;
long val;

SetVendorName(_T("VSCP"));
SetAppName(_T("vscpworks"));  


#ifdef __WXMSW__
wxChar sz[ MAX_PATH ];
GetSystemDirectory( sz, sizeof( sz ) );
wxString strPathSystemDir( sz );
strPathSystemDir += VSCPWORKS_CONFIG_FILE_NAME;

// Check if file exists
if ( ::wxFileExists( strPathSystemDir ) ) {
;
}
else {
// Try local file
strPathSystemDir = VSCPWORKS_CONFIG_FILE_NAME;
if ( !::wxFileExists( strPathSystemDir ) ) {
wxString strError;
strError.Printf( _("Configuration file %s does not exists! A new file will be created when the program ends."), strPathSystemDir );
wxMessageBox( strError );
return false;
}
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

//wxConfigBase *pconfig = wxConfigBase::Get();

// uncomment this to force writing back of the defaults for all values
// if they're not present in the config - this can give the user an idea
// of all possible settings for this program
//pconfig->SetRecordDefaults();

pconfig->SetPath(_("/GENERAL"));


// bDisplayDeleteWarning
m_bDisplayDeleteWarning = true;
if ( pconfig->Read( "bDisplayDeleteWarning", &val, 0 ) ) {
m_bDisplayDeleteWarning = ( val ? true : false);
}
wxLogDebug("Read Configuration: bDisplayDeleteWarning=%d", m_bDisplayDeleteWarning );


// sizeWidth
pconfig->Read( _("sizeWidth"), &val, 640 );
g_Config.m_sizeWidth = val;

wxLogDebug( _("Read Configuration: sizeWidth=%d"), g_Config.m_sizeWidth );

// sizeHeight
pconfig->Read( _("sizeHeight"), &val, 480 );
g_Config.m_sizeHeight = val;
wxLogDebug( _("Read Configuration: sizeHeight=%d"), g_Config.m_sizeHeight );

// xpos
pconfig->Read( _("xpos"), &val, 0 );
g_Config.m_xpos = val;

// ypos
pconfig->Read( _("ypos"), &val, 0 );
g_Config.m_ypos = val;


// Path for temporary files
#ifdef WIN32    
pconfig->Read( _("PathToTempFiles"), &wxstr, _("c:\\") );
#else
pconfig->Read( _("PathToTempFiles"), &wxstr, _("/tmp/") );
#endif    
g_Config.m_strPathTemp = wxstr;




pconfig->SetPath(_("/TCPIP"));




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
pconfig->Read( _("password"), &wxstr, _("secret") );
g_Config.m_strPassword = wxstr;
wxLogDebug( wxstr );



pconfig->SetPath(_("/LOGFILE"));



// Enable Logfile
pconfig->Read( _("bEnableLogFile"), &g_Config.m_bLogFile, true );

// Path to Logfile
#ifdef WIN32    
pconfig->Read( _("path2logfile"), &wxstr, _("c:\\vscpworks.log") );
#else
pconfig->Read( _("path2logfile"), &wxstr, _("/tmp/vscpworks.log") );
#endif    
g_Config.m_strPathLogFile = wxstr;
wxLogDebug( wxstr );	





pconfig->SetPath(_("/CANAL"));



pconfig->Read( _("CanalRcvFrameTextColour"), &wxstr, _("255,0,0") );
{
long r=0,g=0,b=0;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_CanalRcvFrameTextColour = wxColour( r,g,b );
} 


pconfig->Read( _("CanalRcvFrameBgColour"), &wxstr, _("240, 255, 255") );
{
long r=240,g=255,b=255;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_CanalRcvFrameBgColour = wxColour( r,g,b );
} 


pconfig->Read( _("CanalRcvFrameLineColour"), &wxstr, _("192, 192, 192") );
{
long r=192,g=192,b=192;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_CanalRcvFrameLineColour = wxColour( r,g,b );
} 

g_Config.m_CanalRcvFrameFont = wxFont( 8, wxDEFAULT, wxNORMAL, wxBOLD ); 
if ( pconfig->Read( _("CanalRcvFrameFont"), &wxstr ) ) {

}


pconfig->Read( _("CanalRcvShowField0"), &val, TRUE );
g_Config.m_CanalRcvShowField[0] =  ( val ? true : false);

pconfig->Read( _("CanalRcvShowField1"), &val, TRUE );
g_Config.m_CanalRcvShowField[1] =  ( val ? true : false);

pconfig->Read( _("CanalRcvShowField2"), &val, TRUE );
g_Config.m_CanalRcvShowField[2] =  ( val ? true : false);

pconfig->Read( _("CanalRcvShowField3"), &val, TRUE );
g_Config.m_CanalRcvShowField[3] =  ( val ? true : false);

pconfig->Read( _("CanalRcvShowField4"), &val, TRUE );
g_Config.m_CanalRcvShowField[4] =  ( val ? true : false);

pconfig->Read( _("CanalRcvShowField5"), &val, TRUE );
g_Config.m_CanalRcvShowField[5] =  ( val ? true : false);

pconfig->Read( _("CanalRcvShowField6"), &val, TRUE );
g_Config.m_CanalRcvShowField[6] =  ( val ? true : false);

pconfig->Read( _("CanalRcvShowField7"), &val, TRUE );
g_Config.m_CanalRcvShowField[7] =  ( val ? true : false);


pconfig->Read( _("CanalRcvFieldText0"), &g_Config.m_CanalRcvFieldText[0], _("Dir") );
pconfig->Read( _("CanalRcvFieldText1"), &g_Config.m_CanalRcvFieldText[1], _("Time") );
pconfig->Read( _("CanalRcvFieldText2"), &g_Config.m_CanalRcvFieldText[2], _("Timestamp") );
pconfig->Read( _("CanalRcvFieldText3"), &g_Config.m_CanalRcvFieldText[3], _("Flags") );
pconfig->Read( _("CanalRcvFieldText4"), &g_Config.m_CanalRcvFieldText[4], _("Id") );
pconfig->Read( _("CanalRcvFieldText5"), &g_Config.m_CanalRcvFieldText[5], _("Len") );
pconfig->Read( _("CanalRcvFieldText6"), &g_Config.m_CanalRcvFieldText[6], _("Data") );
pconfig->Read( _("CanalRcvFieldText7"), &g_Config.m_CanalRcvFieldText[7], _("Note") );

pconfig->Read( _("CanalRcvFieldOrder0"), &val, 0 );
g_Config.m_CanalRcvFieldOrder[0] = val;
pconfig->Read( _("CanalRcvFieldOrder1"), &val, 1 );
g_Config.m_CanalRcvFieldOrder[1] = val;
pconfig->Read( _("CanalRcvFieldOrder2"), &val, 2 );
g_Config.m_CanalRcvFieldOrder[2] = val;
pconfig->Read( _("CanalRcvFieldOrder3"), &val, 3 );
g_Config.m_CanalRcvFieldOrder[3] = val;
pconfig->Read( _("CanalRcvFieldOrder4"), &val, 4 );
g_Config.m_CanalRcvFieldOrder[4] = val;
pconfig->Read( _("CanalRcvFieldOrder5"), &val, 5 );
g_Config.m_CanalRcvFieldOrder[5] = val;
pconfig->Read( _("CanalRcvFieldOrder6"), &val, 6 );
g_Config.m_CanalRcvFieldOrder[6] = val;
pconfig->Read( _("CanalRcvFieldOrder7"), &val, 7 );
g_Config.m_CanalRcvFieldOrder[7] = val;

pconfig->Read( _("CanalRcvFieldWidth0"), &val, 30 );
g_Config.m_CanalRcvFieldWidth[0] = val;
pconfig->Read( _("CanalRcvFieldWidth1"), &val, 125 );
g_Config.m_CanalRcvFieldWidth[1] = val;
pconfig->Read( _("CanalRcvFieldWidth2"), &val, 90 );
g_Config.m_CanalRcvFieldWidth[2] = val;
pconfig->Read( _("CanalRcvFieldWidth3"), &val, 80 );
g_Config.m_CanalRcvFieldWidth[3] = val;
pconfig->Read( _("CanalRcvFieldWidth4"), &val, 80 );
g_Config.m_CanalRcvFieldWidth[4] = val;
pconfig->Read( _("CanalRcvFieldWidth5"), &val, 30 );
g_Config.m_CanalRcvFieldWidth[5] = val;
pconfig->Read( _("CanalRcvFieldWidth6"), &val, 150 );
g_Config.m_CanalRcvFieldWidth[6] = val;
pconfig->Read( _("CanalRcvFieldWidth7"), &val, 500 );
g_Config.m_CanalRcvFieldWidth[7] = val;

pconfig->Read( _("CanalTrmitFrameTextColour"), &wxstr, _("0,0,0") );
{
long r=0,g=0,b=0;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_CanalTrmitFrameTextColour = wxColour( r,g,b );
}

pconfig->Read( _("CanalTrmitFrameBgColour"), &wxstr, _("255,255,255") );
{
long r=255,g=255,b=255;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_CanalTrmitFrameBgColour = wxColour( r,g,b );
}

pconfig->Read( _("CanalTrmitFrameLineColour"), &wxstr, _("192,192,192") );
{
long r=192,g=192,b=192;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_CanalTrmitFrameLineColour = wxColour( r,g,b );
}

g_Config.m_CanalTrmitFrameFont = wxFont( 8, wxDEFAULT, wxNORMAL, wxBOLD );
if ( pconfig->Read( _("CanalTrmitFrameFont"), &wxstr ) ) {

}

pconfig->Read( _("CanalTrmitShowField0"), &val, TRUE );
g_Config.m_CanalTrmitShowField[0] =  ( val ? true : false);
pconfig->Read( _("CanalTrmitShowField0"), &val, TRUE );
g_Config.m_CanalTrmitShowField[1] =  ( val ? true : false);
pconfig->Read( _("CanalTrmitShowField0"), &val, TRUE );
g_Config.m_CanalTrmitShowField[2] =  ( val ? true : false);
pconfig->Read( _("CanalTrmitShowField0"), &val, TRUE );
g_Config.m_CanalTrmitShowField[3] =  ( val ? true : false);
pconfig->Read( _("CanalTrmitShowField0"), &val, TRUE );
g_Config.m_CanalTrmitShowField[4] =  ( val ? true : false);


pconfig->Read( _("CanalTrmitFieldText0"), &g_Config.m_CanalTrmitFieldText[0], _("Name") );
pconfig->Read( _("CanalTrmitFieldText1"), &g_Config.m_CanalTrmitFieldText[1], _("Period") );
pconfig->Read( _("CanalTrmitFieldText2"), &g_Config.m_CanalTrmitFieldText[2], _("Count") );
pconfig->Read( _("CanalTrmitFieldText3"), &g_Config.m_CanalTrmitFieldText[3], _("Trigger") );
pconfig->Read( _("CanalTrmitFieldText4"), &g_Config.m_CanalTrmitFieldText[4], _("Msg") );


pconfig->Read( _("CanalTrmitFieldOrder0"), &val, 0 );
g_Config.m_CanalTrmitFieldOrder[0] = val;
pconfig->Read( _("CanalTrmitFieldOrder1"), &val, 1 );
g_Config.m_CanalTrmitFieldOrder[1] = val;
pconfig->Read( _("CanalTrmitFildOrder2"), &val, 2 );
g_Config.m_CanalTrmitFieldOrder[2] = val;
pconfig->Read( _("CanalTrmitFieldOrder3"), &val, 3 );
g_Config.m_CanalTrmitFieldOrder[3] = val;
pconfig->Read( _("CanalTrmitFieldOrder4"), &val, 4 );
g_Config.m_CanalTrmitFieldOrder[4] = val;

pconfig->Read( _("CanalTrmitFieldWidth0"), &val, 300 );
g_Config.m_CanalTrmitFieldWidth[0] = val;
pconfig->Read( _("CanalTrmitFieldWidth1"), &val, 60 );
g_Config.m_CanalTrmitFieldWidth[1] = val;
pconfig->Read( _("CanalTrmitFieldWidth2"), &val, 60 );
g_Config.m_CanalTrmitFieldWidth[2] = val;
pconfig->Read( _("CanalTrmitFieldWidth3"), &val, 60 );
g_Config.m_CanalTrmitFieldWidth[3] = val;
pconfig->Read( _("CanalTrmitFieldWidth4"), &val, 500 );
g_Config.m_CanalTrmitFieldWidth[4] = val;





pconfig->SetPath(_("/VSCP"));





pconfig->Read( _("VscpRcvFrameTextColour"), &wxstr, _("0,0,0") );
{
long r=0,g=0,b=0;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_VscpRcvFrameTextColour = wxColour( r,g,b );
} 


pconfig->Read( _("VscpRcvFrameBgColour"), &wxstr, _("255, 255, 230") );
{
long r=240,g=255,b=255;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_VscpRcvFrameBgColour = wxColour( r,g,b );
} 


pconfig->Read( _("VscpRcvFrameLineColour"), &wxstr, _("0xc0, 0xc0, 0xc0 ") );
{
long r=192,g=192,b=192;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_VscpRcvFrameLineColour = wxColour( r,g,b );
} 

g_Config.m_VscpRcvFrameFont = wxFont( 8, wxDEFAULT, wxNORMAL, wxBOLD );
if ( pconfig->Read( _("VscpRcvFrameFont"), &wxstr ) ) {

}

pconfig->Read( _("VscpRcvShowField0"), &val, TRUE );
g_Config.m_VscpRcvShowField[0] =  ( val ? true : false);

pconfig->Read( _("VscpRcvShowField1"), &val, TRUE );
g_Config.m_VscpRcvShowField[1] =  ( val ? true : false);

pconfig->Read( _("VscpRcvShowField2"), &val, TRUE );
g_Config.m_VscpRcvShowField[2] =  ( val ? true : false);

pconfig->Read( _("VscpRcvShowField3"), &val, TRUE );
g_Config.m_VscpRcvShowField[3] =  ( val ? true : false);

pconfig->Read( _("VscpRcvShowField4"), &val, TRUE );
g_Config.m_VscpRcvShowField[4] =  ( val ? true : false);

pconfig->Read( _("VscpRcvShowField5"), &val, TRUE );
g_Config.m_VscpRcvShowField[5] =  ( val ? true : false);

pconfig->Read( _("VscpRcvShowField6"), &val, TRUE );
g_Config.m_VscpRcvShowField[6] =  ( val ? true : false);

pconfig->Read( _("VscpRcvShowField7"), &val, TRUE );
g_Config.m_VscpRcvShowField[7] =  ( val ? true : false);


pconfig->Read( _("VscpRcvFieldText0"), &g_Config.m_VscpRcvFieldText[0], _("Dir") );
pconfig->Read( _("VscpRcvFieldText1"), &g_Config.m_VscpRcvFieldText[1], _("Time") );
pconfig->Read( _("VscpRcvFieldText2"), &g_Config.m_VscpRcvFieldText[2], _("GUID") );
pconfig->Read( _("VscpRcvFieldText3"), &g_Config.m_VscpRcvFieldText[3], _("Class") );
pconfig->Read( _("VscpRcvFieldText4"), &g_Config.m_VscpRcvFieldText[4], _("Type") );
pconfig->Read( _("VscpRcvFieldText5"), &g_Config.m_VscpRcvFieldText[5], _("Head") );
pconfig->Read( _("VscpRcvFieldText6"), &g_Config.m_VscpRcvFieldText[6], _("Cnt") );
pconfig->Read( _("VscpRcvFieldText7"), &g_Config.m_VscpRcvFieldText[7], _("Data") );
pconfig->Read( _("VscpRcvFieldText8"), &g_Config.m_VscpRcvFieldText[8], _("Timestamp") );
pconfig->Read( _("VscpRcvFieldText9"), &g_Config.m_VscpRcvFieldText[9], _("Note") );

pconfig->Read( _("VscpRcvFieldOrder0"), &val, 0 );
g_Config.m_VscpRcvFieldOrder[0] = val;
pconfig->Read( _("VscpRcvFieldOrder1"), &val, 1 );
g_Config.m_VscpRcvFieldOrder[1] = val;
pconfig->Read( _("VscpRcvFieldOrder2"), &val, 2 );
g_Config.m_VscpRcvFieldOrder[2] = val;
pconfig->Read( _("VscpRcvFieldOrder3"), &val, 3 );
g_Config.m_VscpRcvFieldOrder[3] = val;
pconfig->Read( _("VscpRcvFieldOrder4"), &val, 4 );
g_Config.m_VscpRcvFieldOrder[4] = val;
pconfig->Read( _("VscpRcvFieldOrder5"), &val, 5 );
g_Config.m_VscpRcvFieldOrder[5] = val;
pconfig->Read( _("VscpRcvFieldOrder6"), &val, 6 );
g_Config.m_VscpRcvFieldOrder[6] = val;
pconfig->Read( _("VscpRcvFieldOrder7"), &val, 7 );
g_Config.m_VscpRcvFieldOrder[7] = val;
pconfig->Read( _("VscpRcvFieldOrder8"), &val, 8 );
g_Config.m_VscpRcvFieldOrder[9] = val;
pconfig->Read( _("VscpRcvFieldOrder9"), &val, 9 );
g_Config.m_VscpRcvFieldOrder[9] = val;

pconfig->Read( _("VscpRcvFieldWidth0"), &val, 30 );
g_Config.m_VscpRcvFieldWidth[0] = val;
pconfig->Read( _("VscpRcvFieldWidth1"), &val, 110 );
g_Config.m_VscpRcvFieldWidth[1] = val;
pconfig->Read( _("VscpRcvFieldWidth2"), &val, 250 );
g_Config.m_VscpRcvFieldWidth[2] = val;
pconfig->Read( _("VscpRcvFieldWidth3"), &val, 70 );
g_Config.m_VscpRcvFieldWidth[3] = val;
pconfig->Read( _("VscpRcvFieldWidth4"), &val, 70 );
g_Config.m_VscpRcvFieldWidth[4] = val;
pconfig->Read( _("VscpRcvFieldWidth5"), &val, 60 );
g_Config.m_VscpRcvFieldWidth[5] = val;
pconfig->Read( _("VscpRcvFieldWidth6"), &val, 60 );
g_Config.m_VscpRcvFieldWidth[6] = val;
pconfig->Read( _("VscpRcvFieldWidth7"), &val, 300 );
g_Config.m_VscpRcvFieldWidth[7] = val;
pconfig->Read( _("VscpRcvFieldWidth8"), &val, 90 );
g_Config.m_VscpRcvFieldWidth[8] = val;
pconfig->Read( _("VscpRcvFieldWidth9"), &val, 200 );
g_Config.m_VscpRcvFieldWidth[9] = val;


pconfig->Read( _("VscpTrmitFrameTextColour"), &wxstr, _("0,0,0") );
{
long r=0,g=0,b=0;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_VscpTrmitFrameTextColour = wxColour( r,g,b );
}

pconfig->Read( _("VscpTrmitFrameBgColour"), &wxstr, _("255,255,255") );
{
long r=255,g=255,b=255;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_VscpTrmitFrameBgColour = wxColour( r,g,b );
}

pconfig->Read( _("VscpTrmitFrameLineColour"), &wxstr, _("0,0,0") );
{
long r=0,g=0,b=0;
wxStringTokenizer tkz( wxstr, _(",") );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &r );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &g );
if ( tkz.HasMoreTokens() )  ( tkz.GetNextToken() ).ToLong( &b );
g_Config.m_VscpTrmitFrameLineColour = wxColour( r,g,b );
}

g_Config.m_VscpTrmitFrameFont = wxFont( 8, wxDEFAULT, wxNORMAL, wxBOLD );
if ( pconfig->Read( _("VscpTrmitFrameFont"), &wxstr ) ) {

}

pconfig->Read( _("VscpTrmitShowField0"), &val, TRUE );
g_Config.m_VscpTrmitShowField[0] =  ( val ? true : false);
pconfig->Read( _("VscpTrmitShowField1"), &val, TRUE );
g_Config.m_VscpTrmitShowField[1] =  ( val ? true : false);
pconfig->Read( _("VscpTrmitShowField2"), &val, TRUE );
g_Config.m_VscpTrmitShowField[2] =  ( val ? true : false);
pconfig->Read( _("VscpTrmitShowField3"), &val, TRUE );
g_Config.m_VscpTrmitShowField[3] =  ( val ? true : false);
pconfig->Read( _("VscpTrmitShowField4"), &val, TRUE );
g_Config.m_VscpTrmitShowField[4] =  ( val ? true : false);


pconfig->Read( _("VscpTrmitFieldText0"), &g_Config.m_VscpTrmitFieldText[0], _("Name") );
pconfig->Read( _("VscpTrmitFieldText1"), &g_Config.m_VscpTrmitFieldText[1], _("Period") );
pconfig->Read( _("VscpTrmitFieldText2"), &g_Config.m_VscpTrmitFieldText[2], _("Count") );
pconfig->Read( _("VscpTrmitFieldText3"), &g_Config.m_VscpTrmitFieldText[3], _("Trigger") );
pconfig->Read( _("VscpTrmitFieldText4"), &g_Config.m_VscpTrmitFieldText[4], _("Msg") );


pconfig->Read( _("VscpTrmitFieldOrder0"), &val, 0 );
g_Config.m_VscpTrmitFieldOrder[0] = val;
pconfig->Read( _("VscpTrmitFieldOrder1"), &val, 1 );
g_Config.m_VscpTrmitFieldOrder[1] = val;
pconfig->Read( _("VscpTrmitFieldOrder2"), &val, 2 );
g_Config.m_VscpTrmitFieldOrder[2] = val;
pconfig->Read( _("VscpTrmitFieldOrder3"), &val, 3 );
g_Config.m_VscpTrmitFieldOrder[3] = val;
pconfig->Read( _("VscpTrmitFieldOrder4"), &val, 4 );
g_Config.m_VscpTrmitFieldOrder[4] = val;

pconfig->Read( _("VscpTrmitFieldWidth0"), &val, 300 );
g_Config.m_VscpTrmitFieldWidth[0] = val;
pconfig->Read( _("VscpTrmitFieldWidth1"), &val, 60 );
g_Config.m_VscpTrmitFieldWidth[1] = val;
pconfig->Read( _("VscpTrmitFieldWidth2"), &val, 60 );
g_Config.m_VscpTrmitFieldWidth[2] = val;
pconfig->Read( _("VscpTrmitFieldWidth3"), &val, 60 );
g_Config.m_VscpTrmitFieldWidth[3] = val;
pconfig->Read( _("VscpTrmitFieldWidth4"), &val, 500 );
g_Config.m_VscpTrmitFieldWidth[4] = val;





pconfig->SetPath(_("/VSCP_KNOWN_NODES"));






int i;
wxString wxbuf;

for ( i=0; i<MAX_NUMBER_OF_NODES; i++ ) {

wxbuf.Printf(_("node%d"),i);

if ( pconfig->Read( wxbuf, &wxstr ) ) {

wxStringTokenizer tok1, tok2;
tok1.SetString( wxstr,_(",") );

// Real name
if ( tok1.HasMoreTokens() ) {
g_nodesVSCP[ g_cntVscpNodes ].m_strRealname = tok1.GetNextToken();
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
g_nodesVSCP[ g_cntVscpNodes ].m_GUID[15 - i] = (unsigned char)val;
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
g_nodesVSCP[ g_cntVscpNodes ].m_strHostname = tok1.GetNextToken();
}
else {
continue;
}

// Zone
if ( tok1.HasMoreTokens() ) {
long val;
(tok1.GetNextToken()).ToLong( &val );
g_nodesVSCP[ g_cntVscpNodes ].m_zone = (unsigned char)val;
g_cntVscpNodes++;
}
else {
continue;
}

}
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

bool VscpworksApp::saveConfigData( void )
{
long val;
wxString wxstr;


#ifdef __WXMSW__
wxChar sz[MAX_PATH];
GetSystemDirectory( sz, sizeof( sz ) );
wxString strPathSystemDir( sz );
strPathSystemDir += _("\\");
strPathSystemDir += VSCPWORKS_CONFIG_FILE_NAME;
#else
wxString strPathSystemDir;
strPathSystemDir = _("/etc/");
strPathSystemDir += VSCPWORKS_CONFIG_FILE_NAME;
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


//wxConfigBase *pconfig = wxConfigBase::Get();

if ( pconfig == NULL ) return false;


// uncomment this to force writing back of the defaults for all values
// if they're not present in the config - this can give the user an idea
// of all possible settings for this program
//pconfig->SetRecordDefaults();


pconfig->SetPath(_("/GENERAL"));


// sizeWidth
pconfig->Write( _("/GENERAL/sizeWidth"), (long)g_Config.m_sizeWidth );



// sizeHeight
pconfig->Write( _("/GENERAL/sizeHeight"), (long)g_Config.m_sizeHeight );

// xpos
pconfig->Write( _("/GENERAL/xpos"), (long)g_Config.m_xpos );

// ypos
pconfig->Write( _("/GENERAL/ypos"), (long)g_Config.m_ypos );



// PathToTempFiles
pconfig->Write( _("/GENERAL/PathToTempFiles"), g_Config.m_strPathTemp );




pconfig->SetPath(_("/TCPIP"));



// Server
pconfig->Write( _("server"), g_Config.m_strServer );

// Port
pconfig->Write( _("port"), g_Config.m_strPort );


// Username
pconfig->Write( _("username"), g_Config.m_strUsername );

// Password
pconfig->Write( _("password"), g_Config.m_strPassword );


pconfig->SetPath(_("/LOGFILE"));



// Enable logfile writes
pconfig->Write( _("bEnableLogFile"), (long)g_Config.m_bLogFile );

// Path to configuration file
pconfig->Write( _("path2logfile"), g_Config.m_strPathLogFile );





pconfig->SetPath(_("/CANAL"));




wxstr.Printf( _("%d,%d,%d"), g_Config.m_CanalRcvFrameTextColour.Red(),
g_Config.m_CanalRcvFrameTextColour.Green(),
g_Config.m_CanalRcvFrameTextColour.Blue() );
pconfig->Write( _("CanalRcvFrameTextColour"), wxstr );


wxstr.Printf( _("%d,%d,%d"), g_Config.m_CanalRcvFrameBgColour.Red(),
g_Config.m_CanalRcvFrameBgColour.Green(),
g_Config.m_CanalRcvFrameBgColour.Blue() );
pconfig->Write( _("CanalRcvFrameBgColour"), wxstr );


wxstr.Printf( _("%d,%d,%d"), g_Config.m_CanalRcvFrameLineColour.Red(),
g_Config.m_CanalRcvFrameLineColour.Green(),
g_Config.m_CanalRcvFrameLineColour.Blue() );
pconfig->Write( _("CanalRcvFrameLineColour"), wxstr );


//pconfig->Write( _("CanalRcvFrameFont"), &wxstr ) ) {

val = ( g_Config.m_CanalRcvShowField[0] ? 1 : 0 );
pconfig->Write( _("CanalRcvShowField0"), &val );

val = ( g_Config.m_CanalRcvShowField[1] ? 1 : 0 );
pconfig->Write( _("CanalRcvShowField1"), &val );

val = ( g_Config.m_CanalRcvShowField[2] ? 1 : 0 );
pconfig->Write( _("CanalRcvShowField2"), &val );

val = ( g_Config.m_CanalRcvShowField[3] ? 1 : 0 );
pconfig->Write( _("CanalRcvShowField3"), &val );

val = ( g_Config.m_CanalRcvShowField[4] ? 1 : 0 );
pconfig->Write( _("CanalRcvShowField4"), &val );

val = ( g_Config.m_CanalRcvShowField[5] ? 1 : 0 );
pconfig->Write( _("CanalRcvShowField5"), &val );

val = ( g_Config.m_CanalRcvShowField[6] ? 1 : 0 );
pconfig->Write( _("CanalRcvShowField6"), &val );

val = ( g_Config.m_CanalRcvShowField[7] ? 1 : 0 );
pconfig->Write( _("CanalRcvShowField7"), &val );


pconfig->Write( _("CanalRcvFieldText0"), g_Config.m_CanalRcvFieldText[0] );
pconfig->Write( _("CanalRcvFieldText1"), g_Config.m_CanalRcvFieldText[1] );
pconfig->Write( _("CanalRcvFieldText2"), g_Config.m_CanalRcvFieldText[2] );
pconfig->Write( _("CanalRcvFieldText3"), g_Config.m_CanalRcvFieldText[3] );
pconfig->Write( _("CanalRcvFieldText4"), g_Config.m_CanalRcvFieldText[4] );
pconfig->Write( _("CanalRcvFieldText5"), g_Config.m_CanalRcvFieldText[5] );
pconfig->Write( _("CanalRcvFieldText6"), g_Config.m_CanalRcvFieldText[6] );
pconfig->Write( _("CanalRcvFieldText0"), g_Config.m_CanalRcvFieldText[0] );( _("CanalRcvFieldText7"), g_Config.m_CanalRcvFieldText[7] );


pconfig->Write( _("CanalRcvFieldOrder0"), g_Config.m_CanalRcvFieldOrder[0] );
pconfig->Write( _("CanalRcvFieldOrder1"), g_Config.m_CanalRcvFieldOrder[1] );
pconfig->Write( _("CanalRcvFieldOrder2"), g_Config.m_CanalRcvFieldOrder[2] );
pconfig->Write( _("CanalRcvFieldOrder3"), g_Config.m_CanalRcvFieldOrder[3] );
pconfig->Write( _("CanalRcvFieldOrder4"), g_Config.m_CanalRcvFieldOrder[4] );
pconfig->Write( _("CanalRcvFieldOrder5"), g_Config.m_CanalRcvFieldOrder[5] );
pconfig->Write( _("CanalRcvFieldOrder6"), g_Config.m_CanalRcvFieldOrder[6] );
pconfig->Write( _("CanalRcvFieldOrder7"), g_Config.m_CanalRcvFieldOrder[7] );

pconfig->Write( _("CanalRcvFieldWidth0"), g_Config.m_CanalRcvFieldWidth[0] );
pconfig->Write( _("CanalRcvFieldWidth1"), g_Config.m_CanalRcvFieldWidth[1] );
pconfig->Write( _("CanalRcvFieldWidth2"), g_Config.m_CanalRcvFieldWidth[2] );
pconfig->Write( _("CanalRcvFieldWidth3"), g_Config.m_CanalRcvFieldWidth[3] );
pconfig->Write( _("CanalRcvFieldWidth4"), g_Config.m_CanalRcvFieldWidth[4] );
pconfig->Write( _("CanalRcvFieldWidth5"), g_Config.m_CanalRcvFieldWidth[5] );
pconfig->Write( _("CanalRcvFieldWidth6"), g_Config.m_CanalRcvFieldWidth[6] );
pconfig->Write( _("CanalRcvFieldWidth7"), g_Config.m_CanalRcvFieldWidth[7] );

wxstr.Printf( _("%d,%d,%d"), g_Config.m_CanalTrmitFrameTextColour.Red(),
g_Config.m_CanalTrmitFrameTextColour.Green(),
g_Config.m_CanalTrmitFrameTextColour.Blue() );
pconfig->Write( _("CanalTrmitFrameTextColour"), wxstr );

wxstr.Printf( _("%d,%d,%d"), g_Config.m_CanalTrmitFrameBgColour.Red(),
g_Config.m_CanalTrmitFrameBgColour.Green(),
g_Config.m_CanalTrmitFrameBgColour.Blue() );
pconfig->Write( _("CanalTrmitFrameBgColour"), wxstr );

wxstr.Printf( _("%d,%d,%d"), g_Config.m_CanalTrmitFrameLineColour.Red(),
g_Config.m_CanalTrmitFrameLineColour.Green(),
g_Config.m_CanalTrmitFrameLineColour.Blue() );
pconfig->Write( _("CanalTrmitFrameLineColour"), wxstr );



//pconfig->Write( _("CanalTrmitFrameFont"), &wxstr ) );

val = ( g_Config.m_CanalTrmitShowField[0] ? 1 : 0 );
pconfig->Write( _("CanalTrmitShowField0"), val );
val = ( g_Config.m_CanalTrmitShowField[1] ? 1 : 0 );
pconfig->Write( _("CanalTrmitShowField1"), val );
val = ( g_Config.m_CanalTrmitShowField[2] ? 1 : 0 );
pconfig->Write( _("CanalTrmitShowField2"), val );
val = ( g_Config.m_CanalTrmitShowField[3] ? 1 : 0 );
pconfig->Write( _("CanalTrmitShowField3"), val );
val = ( g_Config.m_CanalTrmitShowField[4] ? 1 : 0 );
pconfig->Write( _("CanalTrmitShowField4"), val );


pconfig->Write( _("CanalTrmitFieldText0"), g_Config.m_CanalTrmitFieldText[0] );  
pconfig->Write( _("CanalTrmitFieldText1"), g_Config.m_CanalTrmitFieldText[1] );
pconfig->Write( _("CanalTrmitFieldText2"), g_Config.m_CanalTrmitFieldText[2] );
pconfig->Write( _("CanalTrmitFieldText3"), g_Config.m_CanalTrmitFieldText[3] );
pconfig->Write( _("CanalTrmitFieldText4"), g_Config.m_CanalTrmitFieldText[4] );


pconfig->Write( _("CanalTrmitFieldOrder0"), g_Config.m_CanalTrmitFieldOrder[0] );
pconfig->Write( _("CanalTrmitFieldOrder1"), g_Config.m_CanalTrmitFieldOrder[1] );
pconfig->Write( _("CanalTrmitFieldOrder2"), g_Config.m_CanalTrmitFieldOrder[2] );
pconfig->Write( _("CanalTrmitFieldOrder3"), g_Config.m_CanalTrmitFieldOrder[3] );
pconfig->Write( _("CanalTrmitFieldOrder4"), g_Config.m_CanalTrmitFieldOrder[4] );


pconfig->Write( _("CanalTrmitFieldWidth0"), g_Config.m_CanalTrmitFieldWidth[0] );
pconfig->Write( _("CanalTrmitFieldWidth1"), g_Config.m_CanalTrmitFieldWidth[1] );
pconfig->Write( _("CanalTrmitFieldWidth2"), g_Config.m_CanalTrmitFieldWidth[2] );
pconfig->Write( _("CanalTrmitFieldWidth3"), g_Config.m_CanalTrmitFieldWidth[3] );
pconfig->Write( _("CanalTrmitFieldWidth4"), g_Config.m_CanalTrmitFieldWidth[4] );





pconfig->SetPath(_("/VSCP"));





wxstr.Printf( _("%d,%d,%d"), g_Config.m_VscpRcvFrameTextColour.Red(),
g_Config.m_VscpRcvFrameTextColour.Green(),
g_Config.m_VscpRcvFrameTextColour.Blue() );
pconfig->Write( _("VscpRcvFrameTextColour"), wxstr );


wxstr.Printf( _("%d,%d,%d"), g_Config.m_VscpRcvFrameBgColour.Red(),
g_Config.m_VscpRcvFrameBgColour.Green(),
g_Config.m_VscpRcvFrameBgColour.Blue() );
pconfig->Write( _("VscpRcvFrameBgColour"), wxstr );


wxstr.Printf( _("%d,%d,%d"), g_Config.m_VscpRcvFrameLineColour.Red(),
g_Config.m_VscpRcvFrameLineColour.Green(),
g_Config.m_VscpRcvFrameLineColour.Blue() );
pconfig->Write( _("VscpRcvFrameLineColour"), wxstr );





wxstr.Printf( _("%d,%d,%d"), g_Config.m_VscpTrmitFrameTextColour.Red(),
g_Config.m_VscpTrmitFrameTextColour.Green(),
g_Config.m_VscpTrmitFrameTextColour.Blue() );
pconfig->Write( _("VscpTrmitFrameTextColour"), wxstr );

wxstr.Printf( _("%d,%d,%d"), g_Config.m_VscpTrmitFrameBgColour.Red(),
g_Config.m_VscpTrmitFrameBgColour.Green(),
g_Config.m_VscpTrmitFrameBgColour.Blue() );
pconfig->Write( _("VscpTrmitFrameBgColour"), wxstr );

wxstr.Printf( _("%d,%d,%d"), g_Config.m_VscpTrmitFrameLineColour.Red(),
g_Config.m_VscpTrmitFrameLineColour.Green(),
g_Config.m_VscpTrmitFrameLineColour.Blue() );
pconfig->Write( _("VscpTrmitFrameLineColour"), wxstr );  


//pconfig->Write( _("VscpRcvFrameFont"), wxstr );

val = ( g_Config.m_VscpRcvShowField[0] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField0"), val );
val = ( g_Config.m_VscpRcvShowField[1] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField1"), val );
val = ( g_Config.m_VscpRcvShowField[2] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField2"), val );
val = ( g_Config.m_VscpRcvShowField[3] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField3"), val );
val = ( g_Config.m_VscpRcvShowField[4] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField4"), val );
val = ( g_Config.m_VscpRcvShowField[5] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField5"), val );
val = ( g_Config.m_VscpRcvShowField[6] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField6"), val );
val = ( g_Config.m_VscpRcvShowField[7] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField7"), val );
val = ( g_Config.m_VscpRcvShowField[8] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField7"), val );
val = ( g_Config.m_VscpRcvShowField[9] ? 1 : 0 );
pconfig->Write( _("VscpRcvShowField7"), val );

pconfig->Write( _("VscpRcvFieldText0"), g_Config.m_VscpRcvFieldText[0] );
pconfig->Write( _("VscpRcvFieldText1"), g_Config.m_VscpRcvFieldText[1] );
pconfig->Write( _("VscpRcvFieldText2"), g_Config.m_VscpRcvFieldText[2] );
pconfig->Write( _("VscpRcvFieldText3"), g_Config.m_VscpRcvFieldText[3] );
pconfig->Write( _("VscpRcvFieldText4"), g_Config.m_VscpRcvFieldText[4] );
pconfig->Write( _("VscpRcvFieldText5"), g_Config.m_VscpRcvFieldText[5] );
pconfig->Write( _("VscpRcvFieldText6"), g_Config.m_VscpRcvFieldText[6] );
pconfig->Write( _("VscpRcvFieldText7"), g_Config.m_VscpRcvFieldText[7] );
pconfig->Write( _("VscpRcvFieldText7"), g_Config.m_VscpRcvFieldText[8] );
pconfig->Write( _("VscpRcvFieldText7"), g_Config.m_VscpRcvFieldText[9] );

pconfig->Write( _("VscpRcvFieldOrder0"), g_Config.m_VscpRcvFieldOrder[0] );
pconfig->Write( _("VscpRcvFieldOrder1"), g_Config.m_VscpRcvFieldOrder[1] );
pconfig->Write( _("VscpRcvFieldOrder2"), g_Config.m_VscpRcvFieldOrder[2] );
pconfig->Write( _("VscpRcvFieldOrder3"), g_Config.m_VscpRcvFieldOrder[3] );
pconfig->Write( _("VscpRcvFieldOrder4"), g_Config.m_VscpRcvFieldOrder[4] );
pconfig->Write( _("VscpRcvFieldOrder5"), g_Config.m_VscpRcvFieldOrder[5] );
pconfig->Write( _("VscpRcvFieldOrder6"), g_Config.m_VscpRcvFieldOrder[6] );
pconfig->Write( _("VscpRcvFieldOrder7"), g_Config.m_VscpRcvFieldOrder[7] );
pconfig->Write( _("VscpRcvFieldOrder8"), g_Config.m_VscpRcvFieldOrder[8] );
pconfig->Write( _("VscpRcvFieldOrder9"), g_Config.m_VscpRcvFieldOrder[9] );

pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[0] );
pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[1] );
pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[2] );
pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[3] );
pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[4] );
pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[5] );
pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[6] );
pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[7] );
pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[8] );
pconfig->Write( _("VscpRcvFieldWidth0"), g_Config.m_VscpRcvFieldWidth[9] );

//pconfig->Write( _("VscpTrmitFrameFont"), wxstr ) ) {

val = (g_Config.m_VscpTrmitShowField[0] ? 1 : 0 );
pconfig->Write( _("VscpTrmitShowField0"), val );
val = (g_Config.m_VscpTrmitShowField[1] ? 1 : 0 );
pconfig->Write( _("VscpTrmitShowField1"), val );
val = (g_Config.m_VscpTrmitShowField[2] ? 1 : 0 );
pconfig->Write( _("VscpTrmitShowField2"), val );
val = (g_Config.m_VscpTrmitShowField[3] ? 1 : 0 );
pconfig->Write( _("VscpTrmitShowField3"), val );
val = (g_Config.m_VscpTrmitShowField[4] ? 1 : 0 );
pconfig->Write( _("VscpTrmitShowField4"), val );

pconfig->Write( _("VscpTrmitFieldText0"), g_Config.m_VscpTrmitFieldText[0] );
pconfig->Write( _("VscpTrmitFieldText1"), g_Config.m_VscpTrmitFieldText[1] );
pconfig->Write( _("VscpTrmitFieldText2"), g_Config.m_VscpTrmitFieldText[2] );
pconfig->Write( _("VscpTrmitFieldText3"), g_Config.m_VscpTrmitFieldText[3] );
pconfig->Write( _("VscpTrmitFieldText4"), g_Config.m_VscpTrmitFieldText[4] );

pconfig->Write( _("VscpTrmitFieldOrder0"), g_Config.m_VscpTrmitFieldOrder[0] );  
pconfig->Write( _("VscpTrmitFieldOrder1"), g_Config.m_VscpTrmitFieldOrder[1] );
pconfig->Write( _("VscpTrmitFieldOrder2"), g_Config.m_VscpTrmitFieldOrder[2] );
pconfig->Write( _("VscpTrmitFieldOrder3"), g_Config.m_VscpTrmitFieldOrder[3] );
pconfig->Write( _("VscpTrmitFieldOrder4"), g_Config.m_VscpTrmitFieldOrder[4] );


pconfig->Write( _("VscpTrmitFieldWidth0"), g_Config.m_VscpTrmitFieldWidth[0] );
pconfig->Write( _("VscpTrmitFieldWidth1"), g_Config.m_VscpTrmitFieldWidth[1] );
pconfig->Write( _("VscpTrmitFieldWidth2"), g_Config.m_VscpTrmitFieldWidth[2] );
pconfig->Write( _("VscpTrmitFieldWidth3"), g_Config.m_VscpTrmitFieldWidth[3] );
pconfig->Write( _("VscpTrmitFieldWidth4"), g_Config.m_VscpTrmitFieldWidth[4] );




pconfig->SetPath(_("/VSCP_KNOWN_NODES"));
int i,j;
wxString wxbuf;
wxString wxName;
wxString wxData;
wxString wxNum;

for ( i=0; i<g_cntVscpNodes; i++ ) {

if ( ( g_nodesVSCP[ i ].m_strRealname).length() ) {

wxName.Printf(_("node%d"), i );

wxData = g_nodesVSCP[ i ].m_strRealname;
wxData += _(",");
for ( j=0; j<16; j++ ) {
if ( 15 != j ) {
wxNum.Printf( _("%d:"), g_nodesVSCP[ i ].m_GUID[15 - j] );
}
else {
wxNum.Printf( _("%d"), g_nodesVSCP[ i ].m_GUID[15 - j] );
}

wxData += wxNum;

}

wxData += _(",");
wxData += g_nodesVSCP[ i ].m_strHostname;

wxData += _(",");  
wxNum.Printf( _("%d"), g_nodesVSCP[ i ].m_zone);
wxData += wxNum;

if ( wxData.length() ) pconfig->Write( wxName, wxData );

}

}


// Save changes
pconfig->Save( cfgOutstream );
//pconfig->Flush();

if ( NULL!= pconfig ) delete pconfig;

return true;

}
*/

///////////////////////////////////////////////////////////////////////////////
// readConfiguration
//
// Read the configuration file
//

bool VscpworksApp::readConfiguration( void )
{
    unsigned long val;
    wxString strcfgfile;
    wxXmlDocument doc;

    strcfgfile = wxStandardPaths::Get().GetUserDataDir();
    strcfgfile += _("/vscpworks.conf");

    if ( !wxFileName::FileExists( strcfgfile ) ) {
        strcfgfile = wxStandardPaths::Get().GetConfigDir() + _( "/vscpworks.conf" );
        if ( wxFileName::FileExists( strcfgfile ) ) {
            m_bUseGlobalConfig = true;
        }
        else {
            return false;
        }
    }

    if (!doc.Load( strcfgfile ) ) {
        // test global location
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != _("vscpworksconfig") ) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if (child->GetName() == _("general")) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                g_Config.m_strPathTemp = _("/tmp");
                if (subchild->GetName() == _("MainFrameWidth")) {

                    unsigned long val;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        if ( val ) {
                            g_Config.m_sizeMainFrameWidth = val;
                        }
                        else {
                            g_Config.m_sizeMainFrameWidth = 400;
                        }
                    }

                }
                else if (subchild->GetName() == _("MainFrameHeight")) {

                    unsigned long val;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        if ( val ) {
                            g_Config.m_sizeMainFrameHeight = val;
                        }
                        else {
                            g_Config.m_sizeMainFrameHeight = 300;
                        }
                    }

                }
                else if (subchild->GetName() == _("MainFramexpos")) {

                    unsigned long val;
                    g_Config.m_xposMainFrame = 0;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        g_Config.m_xposMainFrame = val;
                    }

                }
                else if (subchild->GetName() == _("MainFrameypos")) {

                    unsigned long val;
                    g_Config.m_yposMainFrame = 0;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        g_Config.m_yposMainFrame = val;
                    }

                }
                else if (subchild->GetName() == _("SessionFrameWidth")) {

                    unsigned long val;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        if ( val ) {
                            g_Config.m_sizeSessionFrameWidth = val;
                        }
                        else {
                            g_Config.m_sizeSessionFrameWidth = 400;
                        }
                    }

                }
                else if (subchild->GetName() == _("SessionFrameHeight")) {

                    unsigned long val;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        if ( val ) {
                            g_Config.m_sizeSessionFrameHeight = val;
                        }
                        else {
                            g_Config.m_sizeSessionFrameHeight = 300;
                        }
                    }

                }
                else if (subchild->GetName() == _("ConfigurationFrameWidth")) {

                    unsigned long val;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        if ( val ) {
                            g_Config.m_sizeConfigurationFrameWidth = val;
                        }
                        else {
                            g_Config.m_sizeConfigurationFrameWidth = 
                                VSCPWORKS_DEVICECONFIG_SIZE.GetWidth();
                        }
                    }

                }
                else if (subchild->GetName() == _("ConfigurationFrameHeight")) {

                    unsigned long val;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        if ( val ) {
                            g_Config.m_sizeConfigurationFrameHeight = val;
                        }
                        else {
                            g_Config.m_sizeConfigurationFrameHeight = 
                                VSCPWORKS_DEVICECONFIG_SIZE.GetHeight();
                        }
                    }

                }
                else if (subchild->GetName() == _("path2tempfiles")) {

                    g_Config.m_strPathTemp = subchild->GetNodeContent();
                    if ( 0 == g_Config.m_strPathTemp.Length() ) {
                        //wxStandardPaths stdPath;
                        g_Config.m_strPathTemp = wxStandardPaths::Get().GetTempDir();
                    }

                }
                else if (subchild->GetName() == _("path2logfile")) {

                    g_Config.m_strPathLogFile = subchild->GetNodeContent();
                    if ( 0 == g_Config.m_strPathLogFile.Length() ) {
                        g_Config.m_strPathTemp = wxStandardPaths::Get().GetTempDir();
                    }

                    // enable/disable                     
                    wxString enable = subchild->GetAttribute(_("enable"), _("false"));
                 
                    if ( enable.IsSameAs(_("true"), false ) ) {
                        g_Config.m_bEnableLog = true;
                    }

                    // level
                    g_Config.m_logLevel = VSCPWORKS_LOGMSG_EMERGENCY;                   
                    wxString level = subchild->GetAttribute(_("level"), _("0"));
                      
                    level = level.Upper();
                    if ( wxNOT_FOUND  != level.Find(_("DEBUG")) ) {
                        g_Config.m_logLevel = VSCPWORKS_LOGMSG_DEBUG;
                    }
                    else if ( wxNOT_FOUND  != level.Find(_("INFO")) ) {
                        g_Config.m_logLevel = VSCPWORKS_LOGMSG_INFO;
                    }
                    else if ( wxNOT_FOUND  != level.Find(_("WARNING")) ) {
                        g_Config.m_logLevel = VSCPWORKS_LOGMSG_WARNING;
                    }
                    else if ( wxNOT_FOUND  != level.Find(_("ERROR")) ) {
                        g_Config.m_logLevel = VSCPWORKS_LOGMSG_ERROR;
                    }
                    else if ( wxNOT_FOUND  != level.Find(_("CRITICAL")) ) {
                        g_Config.m_logLevel = VSCPWORKS_LOGMSG_CRITICAL;
                    }
                    else if ( wxNOT_FOUND  != level.Find(_("ALERT")) ) {
                        g_Config.m_logLevel = VSCPWORKS_LOGMSG_ALERT;
                    }
                    else if ( wxNOT_FOUND  != level.Find(_("EMERGENCY")) ) {
                        g_Config.m_logLevel = VSCPWORKS_LOGMSG_EMERGENCY;
                    }
                    else {
                        g_Config.m_logLevel = VSCPWORKS_LOGMSG_EMERGENCY;
                    }

                }

                // CANAL communication parameters

                else if (subchild->GetName() == _("CANALReadResendTimeout")) {

                    unsigned long val;
                    g_Config.m_CANALRegResendTimeout = VSCP_CANAL_RESEND_TIMEOUT;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        g_Config.m_CANALRegResendTimeout = val;
                    }

                }
                else if (subchild->GetName() == _("CANALReadMaxRetries")) {

                    unsigned long val;
                    g_Config.m_CANALRegMaxRetries = VSCP_CANAL_MAX_TRIES;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        g_Config.m_CANALRegMaxRetries = val;
                    }

                }
                else if (subchild->GetName() == _("CANALReadErrorTimeout")) {

                    unsigned long val;
                    g_Config.m_CANALRegErrorTimeout = VSCP_CANAL_ERROR_TIMEOUT;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        g_Config.m_CANALRegErrorTimeout = val;
                    }

                }
                

                // TCP/IP communication parameters

                else if (subchild->GetName() == _("TCPIPResponseTimeout")) {

                    unsigned long val;
                    g_Config.m_TCPIP_ResponseTimeout = VSCPWORKS_TCPIP_DEFAULT_RESPONSE_TIMEOUT;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        if ( val < 1000 ) val = VSCPWORKS_TCPIP_DEFAULT_RESPONSE_TIMEOUT;
                        g_Config.m_TCPIP_ResponseTimeout = val;
                    }

                }
                else if (subchild->GetName() == _("TCPIPReadMaxRetries")) {

                    unsigned long val;
                    g_Config.m_TCPIPRegMaxRetries = VSCPWORKS_TCPIP_REGISTER_READ_MAX_TRIES;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        g_Config.m_TCPIPRegMaxRetries = val;
                    }

                }
                else if (subchild->GetName() == _("TCPIPReadResendTimeout")) {

                    unsigned long val;
                    g_Config.m_TCPIPRegResendTimeout = VSCPWORKS_TCPIP_REGISTER_READ_ERROR_TIMEOUT;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        g_Config.m_TCPIPRegResendTimeout = val;
                    }

                }
                else if (subchild->GetName() == _("TCPIPReadErrorTimeout")) {

                    unsigned long val;
                    g_Config.m_TCPIPRegErrorTimeout = VSCPWORKS_TCPIP_REGISTER_READ_ERROR_TIMEOUT;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        g_Config.m_TCPIPRegErrorTimeout = val;
                    }

                }

                else if (subchild->GetName() == _("NumberBase")) {

                    g_Config.m_Numberbase = VSCP_DEVCONFIG_NUMBERBASE_HEX;
                    wxString str = subchild->GetNodeContent();
                    str = str.Upper();
                    if ( wxNOT_FOUND  != str.Find(_("HEX")) ) {
                        g_Config.m_Numberbase = VSCP_DEVCONFIG_NUMBERBASE_HEX;
                    }
                    if ( wxNOT_FOUND  != str.Find(_("HEXADECIMAL")) ) {
                        g_Config.m_Numberbase = VSCP_DEVCONFIG_NUMBERBASE_HEX;
                    }
                    else if ( wxNOT_FOUND  != str.Find(_("DECIMAL")) ) {
                        g_Config.m_Numberbase = VSCP_DEVCONFIG_NUMBERBASE_DECIMAL;
                    }

                }
                else if (subchild->GetName() == _("ConfirmDelete")) {

                    g_Config.m_bConfirmDelete = true;
                    wxString str = subchild->GetNodeContent();
                    str = str.Upper();
                    if ( wxNOT_FOUND  != str.Find(_("FALSE")) ) {
                        g_Config.m_bConfirmDelete = false;
                    }
                    else {
                        g_Config.m_bConfirmDelete = true;
                    }

                }
                else if ( subchild->GetName() == _( "GuidWriteEnable" ) ) {

                    wxString wxstr = subchild->GetNodeContent();
                    if ( wxNOT_FOUND != wxstr.Find( _( "enable" ) ) ) {
                        g_Config.bGuidWritable = true;
                    }

                }
                else if ( subchild->GetName() == _( "ManufacturerGuid" ) ) {

                    wxString wxstr = subchild->GetNodeContent();
                    if ( 0 != wxstr.Length() ) {
                        g_Config.m_manufacturerGuid.getFromString( wxstr );
                    }

                }
                else if ( subchild->GetName() == _( "ManufacturerId" ) ) {

                    wxString wxstr = subchild->GetNodeContent();
                    if ( 0 != wxstr.Length() ) {
                        g_Config.m_manufacturerId = vscp_readStringValue( wxstr );
                    }

                }
                else if ( subchild->GetName() == _( "ManufacturerSubId" ) ) {

                    wxString wxstr = subchild->GetNodeContent();
                    if ( 0 != wxstr.Length() ) {
                        g_Config.m_manufacturerSubId = vscp_readStringValue( wxstr );
                    }

                }

                subchild = subchild->GetNext();
            }

        } 
        else if (child->GetName() == _("vscpclient")) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {


                if (subchild->GetName() == _("VscpRcvFrameRxTextColour")) {

                    unsigned long rval = 0;
                    unsigned long gval = 0;
                    unsigned long bval = 0;

                  
                    wxString r = subchild->GetAttribute(_("r"), _("0"));                  
                    rval = vscp_readStringValue( r );		
                    
                    wxString g = subchild->GetAttribute(_("g"), _("0"));                    
                    gval = vscp_readStringValue( g );	
                   
                    wxString b = subchild->GetAttribute(_("b"), _("0"));
                      
                    bval = vscp_readStringValue( b );	

                    // Assign the colours
                    g_Config.m_VscpRcvFrameRxTextColour = wxColour(rval, gval, bval );
                }
                else if (subchild->GetName() == _("VscpRcvFrameRxBgColour")) {

                    unsigned long rval = 0;
                    unsigned long gval = 0;
                    unsigned long bval = 0;
                     
                    wxString r = subchild->GetAttribute(_("r"), _("0"));                   
                    rval = vscp_readStringValue( r );		
                    
                    wxString g = subchild->GetAttribute(_("g"), _("0"));
                      
                    gval = vscp_readStringValue( g );	
                  
                    wxString b = subchild->GetAttribute(_("b"), _("0"));                    
                    bval = vscp_readStringValue( b );

                    // Assign the colours
                    g_Config.m_VscpRcvFrameRxBgColour = wxColour(rval, gval, bval );          
                }

                if (subchild->GetName() == _("VscpRcvFrameTxTextColour")) {

                    unsigned long rval = 0;
                    unsigned long gval = 0;
                    unsigned long bval = 0;
                    
                    wxString r = subchild->GetAttribute(_("r"), _("0"));
                       
                    rval = vscp_readStringValue( r );		                    
                    wxString g = subchild->GetAttribute(_("g"), _("0"));                      
                    gval = vscp_readStringValue( g );	
                     
                    wxString b = subchild->GetAttribute(_("b"), _("0"));                      
                    bval = vscp_readStringValue( b );

                    // Assign the colours
                    g_Config.m_VscpRcvFrameTxTextColour = wxColour(rval, gval, bval );
                }
                else if (subchild->GetName() == _("VscpRcvFrameTxBgColour")) {

                    unsigned long rval = 0;
                    unsigned long gval = 0;
                    unsigned long bval = 0;
                    
                    wxString r = subchild->GetAttribute(_("r"), _("0"));               
                    rval = vscp_readStringValue( r );		
                    
                    wxString g = subchild->GetAttribute(_("g"), _("0"));                       
                    gval = vscp_readStringValue( g );	
                  
                    wxString b = subchild->GetAttribute(_("b"), _("0"));                      
                    bval = vscp_readStringValue( b );

                    // Assign the colours
                    g_Config.m_VscpRcvFrameTxBgColour = wxColour(rval, gval, bval );          
                }					
                else if (subchild->GetName() == _("VscpRcvFrameLineColour")) {

                    unsigned long rval = 0;
                    unsigned long gval = 0;
                    unsigned long bval = 0;
                   
                    wxString r = subchild->GetAttribute(_("r"), _("0"));                     
                    rval = vscp_readStringValue( r );		
                    
                    wxString g = subchild->GetAttribute(_("g"), _("0"));                       
                    gval = vscp_readStringValue( g );	
                    
                    wxString b = subchild->GetAttribute(_("b"), _("0"));                      
                    bval = vscp_readStringValue( b );

                    // Assign the colours
                    g_Config.m_VscpRcvFrameLineColour = wxColour(rval, gval, bval );          
                }
                else if (subchild->GetName() == _("VscpRcvFrameFont")) {
                    // 8, wxDEFAULT, wxNORMAL, wxBOLD
                    // size, family, style, weight
                    wxString str = subchild->GetNodeContent();
                }
                else if (subchild->GetName() == _("Autoscroll")) {
                    wxString str = subchild->GetNodeContent();
                    if ( wxNOT_FOUND != str.Find(_("true")) ) {
                        g_Config.m_bAutoscollRcv = true;
                    }
                    else {
                        g_Config.m_bAutoscollRcv = false;
                    }
                }
                else if (subchild->GetName() == _("VscpRcvPyjamasStyle")) {
                    wxString str = subchild->GetNodeContent();
                    if ( wxNOT_FOUND != str.Find(_("true")) ) {
                        g_Config.m_VscpRcvFrameRxbPyamas = true;
                    }
                    else {
                        g_Config.m_VscpRcvFrameRxbPyamas = false;
                    }
                }
                else if (subchild->GetName() == _("VscpUseSymbolicNames")) {
                    wxString str = subchild->GetNodeContent();
                    if ( wxNOT_FOUND != str.Find(_("true")) ) {
                        g_Config.m_UseSymbolicNames = true;
                    }
                    else {
                        g_Config.m_UseSymbolicNames = false;
                    }
                }          
                else if (subchild->GetName() == _("VscpRcvRowLineHeight")) {

                    unsigned long val;
                    g_Config.m_VscpRcvFrameRowLineHeight = VCSP_RSCV_FIELD_DEFAULT_HEIGHT;
                    if ( subchild->GetNodeContent().ToULong( &val, 10 ) ) {
                        g_Config.m_VscpRcvFrameRowLineHeight = val;
                    }

                }
                else if (subchild->GetName() == _("VscpRcvShowField0")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpRcvShowField[0] =  ( val ? true : false);
                }
                else if (subchild->GetName() == _("VscpRcvShowField1")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpRcvShowField[1] =  ( val ? true : false);
                }
                else if (subchild->GetName() == _("VscpRcvShowField2")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpRcvShowField[2] =  ( val ? true : false);
                }
                else if (subchild->GetName() == _("VscpRcvShowField3")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpRcvShowField[3] =  ( val ? true : false);
                }
                else if (subchild->GetName() == _("VscpRcvFieldText0")) {
                    g_Config.m_VscpRcvFieldText[ 0 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpRcvFieldText[ 0 ].Length() ) {
                        g_Config.m_VscpRcvFieldText[ 0 ] = VCSP_RSCV_FIELD_TEXT_0;
                    }
                }
                else if (subchild->GetName() == _("VscpRcvFieldText1")) {
                    g_Config.m_VscpRcvFieldText[ 1 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpRcvFieldText[ 1].Length() ) {
                        g_Config.m_VscpRcvFieldText[ 1 ] = VCSP_RSCV_FIELD_TEXT_1;
                    }
                }
                else if (subchild->GetName() == _("VscpRcvFieldText2")) {
                    g_Config.m_VscpRcvFieldText[ 2 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpRcvFieldText[ 2 ].Length() ) {
                        g_Config.m_VscpRcvFieldText[ 2 ] = VCSP_RSCV_FIELD_TEXT_2;
                    }
                }
                else if (subchild->GetName() == _("VscpRcvFieldText3")) {
                    g_Config.m_VscpRcvFieldText[ 3 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpRcvFieldText[ 3 ].Length() ) {
                        g_Config.m_VscpRcvFieldText[ 3 ] = VCSP_RSCV_FIELD_TEXT_3;
                    }
                }
                else if (subchild->GetName() == _("VscpRcvFieldOrder0")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 0;
                    g_Config.m_VscpRcvFieldOrder[0] = val;
                }
                else if (subchild->GetName() == _("VscpRcvFieldOrder1")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 1;
                    g_Config.m_VscpRcvFieldOrder[1] = val;
                }
                else if (subchild->GetName() == _("VscpRcvFieldOrder2")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 2;
                    g_Config.m_VscpRcvFieldOrder[2] = val;
                }
                else if (subchild->GetName() == _("VscpRcvFieldOrder3")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 3;
                    g_Config.m_VscpRcvFieldOrder[3] = val;
                }
                else if (subchild->GetName() == _("VscpRcvFieldWidth0")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = VCSP_RSCV_FIELD_WIDTH_0;
                    g_Config.m_VscpRcvFieldWidth[0] = val; 
                }
                else if (subchild->GetName() == _("VscpRcvFieldWidth1")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = VCSP_RSCV_FIELD_WIDTH_1;
                    g_Config.m_VscpRcvFieldWidth[1] = val;
                }
                else if (subchild->GetName() == _("VscpRcvFieldWidth2")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = VCSP_RSCV_FIELD_WIDTH_2;
                    g_Config.m_VscpRcvFieldWidth[2] = val;
                }
                else if (subchild->GetName() == _("VscpRcvFieldWidth3")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = VCSP_RSCV_FIELD_WIDTH_3;
                    g_Config.m_VscpRcvFieldWidth[3] = val;
                }
                else if (subchild->GetName() == _("VscpTrmitShowField0")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpTrmitShowField[0] =  ( val ? true : false);
                }
                else if (subchild->GetName() == _("VscpTrmitShowField1")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpTrmitShowField[1] =  ( val ? true : false);
                }
                else if (subchild->GetName() == _("VscpTrmitShowField2")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpTrmitShowField[2] =  ( val ? true : false);
                }
                else if (subchild->GetName() == _("VscpTrmitShowField3")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpTrmitShowField[3] =  ( val ? true : false);
                }
                else if (subchild->GetName() == _("VscpTrmitShowField4")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpTrmitShowField[4] =  ( val ? true : false);
                }
                else if (subchild->GetName() == _("VscpTrmitShowField5")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = true;
                    g_Config.m_VscpTrmitShowField[5] =  ( val ? true : false);
                }          
                else if (subchild->GetName() == _("VscpTrmitFieldText0")) {
                    g_Config.m_VscpTrmitFieldText[ 0 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpTrmitFieldText[ 0 ].Length() ) {
                        g_Config.m_VscpTrmitFieldText[ 0 ] = VCSP_TRMIT_FIELD_TEXT_0;
                    }
                }
                else if (subchild->GetName() == _("VscpTrmitFieldText1")) {
                    g_Config.m_VscpTrmitFieldText[ 1 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpTrmitFieldText[ 1 ].Length() ) {
                        g_Config.m_VscpTrmitFieldText[ 1 ] = VCSP_TRMIT_FIELD_TEXT_1;
                    }
                }
                else if (subchild->GetName() == _("VscpTrmitFieldText2")) {
                    g_Config.m_VscpTrmitFieldText[ 2 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpTrmitFieldText[ 2 ].Length() ) {
                        g_Config.m_VscpTrmitFieldText[ 2 ] = VCSP_TRMIT_FIELD_TEXT_2;
                    }
                }
                else if (subchild->GetName() == _("VscpTrmitFieldText3")) {
                    g_Config.m_VscpTrmitFieldText[ 3 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpTrmitFieldText[ 3 ].Length() ) {
                        g_Config.m_VscpTrmitFieldText[ 3 ] = VCSP_TRMIT_FIELD_TEXT_3;
                    }
                }
                else if (subchild->GetName() == _("VscpTrmitFieldText4")) {
                    g_Config.m_VscpTrmitFieldText[ 4 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpTrmitFieldText[ 4 ].Length() ) {
                        g_Config.m_VscpTrmitFieldText[ 4 ] = VCSP_TRMIT_FIELD_TEXT_4;
                    }
                }
                else if (subchild->GetName() == _("VscpTrmitFieldText5")) {
                    g_Config.m_VscpTrmitFieldText[ 5 ] = subchild->GetNodeContent();
                    if ( !g_Config.m_VscpTrmitFieldText[ 5 ].Length() ) {
                        g_Config.m_VscpTrmitFieldText[ 5 ] = VCSP_TRMIT_FIELD_TEXT_5;
                    }
                }          
                else if (subchild->GetName() == _("VscpTrmitFieldOrder0")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 0;
                    g_Config.m_VscpTrmitFieldOrder[0] = val;  
                }
                else if (subchild->GetName() == _("VscpTrmitFieldOrder1")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 1;
                    g_Config.m_VscpTrmitFieldOrder[1] = val;  
                }
                else if (subchild->GetName() == _("VscpTrmitFieldOrder2")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 2;
                    g_Config.m_VscpTrmitFieldOrder[2] = val;  
                }
                else if (subchild->GetName() == _("VscpTrmitFieldOrder3")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 3;
                    g_Config.m_VscpTrmitFieldOrder[3] = val;  
                }
                else if (subchild->GetName() == _("VscpTrmitFieldOrder4")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 4;
                    g_Config.m_VscpTrmitFieldOrder[4] = val;  
                }
                else if (subchild->GetName() == _("VscpTrmitFieldOrder5")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 5;
                    g_Config.m_VscpTrmitFieldOrder[5] = val;  
                }          
                else if (subchild->GetName() == _("VscpTrmitFieldWidth0")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = 5;
                    g_Config.m_VscpTrmitFieldOrder[5] = val;  
                }
                else if (subchild->GetName() == _("VscpTrmitFieldWidth1")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = VCSP_TRMIT_FIELD_WIDTH_1;
                    g_Config.m_VscpTrmitFieldWidth[1] = val; 
                }
                else if (subchild->GetName() == _("VscpTrmitFieldWidth2")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = VCSP_TRMIT_FIELD_WIDTH_2;
                    g_Config.m_VscpTrmitFieldWidth[2] = val; 
                }
                else if (subchild->GetName() == _("VscpTrmitFieldWidth3")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = VCSP_TRMIT_FIELD_WIDTH_3;
                    g_Config.m_VscpTrmitFieldWidth[3] = val; 
                }
                else if (subchild->GetName() == _("VscpTrmitFieldWidth4")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = VCSP_TRMIT_FIELD_WIDTH_4;
                    g_Config.m_VscpTrmitFieldWidth[4] = val; 
                }
                else if (subchild->GetName() == _("VscpTrmitFieldWidth5")) {
                    wxString str = subchild->GetNodeContent();
                    if ( !str.ToULong( &val ) ) val = VCSP_TRMIT_FIELD_WIDTH_5;
                    g_Config.m_VscpTrmitFieldWidth[5] = val; 
                }

                subchild = subchild->GetNext();

            } // while

        } 
        else if (child->GetName() == _("vscpinterface")) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == _("canaldriver")) {

                    canal_interface *pCanalif = new canal_interface;
                    if ( NULL == pCanalif ) return false;

                    wxXmlNode *subsubchild = subchild->GetChildren();
                    while (subsubchild) {
                        if (subsubchild->GetName() == _("description")) {
                            pCanalif->m_strDescription = subsubchild->GetNodeContent();

                        }
                        else if (subsubchild->GetName() == _("path")) {
                            pCanalif->m_strPath = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName() == _("config")) {

                            pCanalif->m_strConfig = subsubchild->GetNodeContent();

                        }
                        else if (subsubchild->GetName() == _("flags")) {

                            unsigned long val;
                            pCanalif->m_flags = 0;
                            if (  subsubchild->GetNodeContent().ToULong( &val, 10 ) ) {
                                pCanalif->m_flags = val;
                            }
                            else if (subsubchild->GetNodeContent().ToULong( &val, 16 ) ) {
                                pCanalif->m_flags = val;
                            }

                        }


                        subsubchild = subsubchild->GetNext();

                    } // while

                    // Add interface
                    if ( NULL != pCanalif ) {
                        g_Config.m_canalIfList.Append( pCanalif );
                    }

                }
                else if (subchild->GetName() == _("vscpdaemon")) {

                    vscp_interface *pVSCPif = new vscp_interface;
                    if ( NULL == pVSCPif ) return false;
                    vscp_clearVSCPFilter( &pVSCPif->m_vscpfilter );

                    wxXmlNode *subsubchild = subchild->GetChildren();
                    while (subsubchild) {

                        if (subsubchild->GetName() == _("description")) {

                            pVSCPif->m_strDescription = subsubchild->GetNodeContent();

                        }
                        else if (subsubchild->GetName() == _("host")) {
                            pVSCPif->m_strHost = subsubchild->GetNodeContent();
                            pVSCPif->m_strHost.Trim(false);
                        }
                        else if (subsubchild->GetName() == _("username")) {

                            pVSCPif->m_strUser = subsubchild->GetNodeContent().Trim();
                            pVSCPif->m_strUser.Trim(false);
                            pVSCPif->m_strUser.Trim();

                        }
                        else if (subsubchild->GetName() == _("password")) {

                            pVSCPif->m_strPassword = subsubchild->GetNodeContent().Trim();
                            pVSCPif->m_strPassword.Trim(false);
                            pVSCPif->m_strPassword.Trim();

                        }
                        else if (subsubchild->GetName() == _("interfacename")) {

                            pVSCPif->m_strInterfaceName = subsubchild->GetNodeContent().Trim();
                            pVSCPif->m_strInterfaceName.Trim(false);
                            pVSCPif->m_strInterfaceName.Trim();

                        }
                        else if (subsubchild->GetName() == _("level2")) {

                            wxString str = subsubchild->GetNodeContent().Trim();
                            str = str.Upper();
                            if ( wxNOT_FOUND != str.Find( _("TRUE") )  ) {
                                pVSCPif->m_bLevel2 = true;		
                            }
                            else {
                                pVSCPif->m_bLevel2 = false;	
                            }

                        }
                        else if (subsubchild->GetName() == _("guid")) {
                            wxString str = subsubchild->GetNodeContent();
                            vscp_getGuidFromStringToArray( pVSCPif->m_GUID, str );
                        }
                        else if (subsubchild->GetName() == _("filter")) {                             
                            pVSCPif->m_vscpfilter.filter_priority = vscp_readStringValue( subsubchild->GetAttribute( _( "priority" ), _("0") ) );

                             
                            pVSCPif->m_vscpfilter.filter_class = vscp_readStringValue( subsubchild->GetAttribute( _( "class" ), _("0") ) );
                              
                            
                            pVSCPif->m_vscpfilter.filter_type = vscp_readStringValue( subsubchild->GetAttribute( _( "type" ), _("0") ) );
                                                                                    
                            wxString strGUID = subsubchild->GetAttribute( _( "GUID" ), 
                                      
                                _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00") );
                            vscp_getGuidFromStringToArray( pVSCPif->m_vscpfilter.filter_GUID, strGUID );
                        }
                        else if (subsubchild->GetName() == _("mask")) {
                            
                            pVSCPif->m_vscpfilter.mask_priority = vscp_readStringValue( subsubchild->GetAttribute( _( "priority" ), _("0") ) );
                                                            
                            pVSCPif->m_vscpfilter.mask_class = vscp_readStringValue( subsubchild->GetAttribute( _( "class" ), _("0") ) );
                               
                            pVSCPif->m_vscpfilter.mask_type = vscp_readStringValue( subsubchild->GetAttribute( _( "type" ), _("0") ) );
   
                            wxString strGUID = subsubchild->GetAttribute( _( "GUID" ), 
   
                                    _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00") );
                            vscp_getGuidFromStringToArray( pVSCPif->m_vscpfilter.mask_GUID, strGUID );
                        }

                        subsubchild = subsubchild->GetNext();

                    }  // While

                    // Add interface
                    if ( NULL != pVSCPif ) {
                        g_Config.m_vscpIfList.Append( pVSCPif );
                    }

                } // vscpdaemon

                subchild = subchild->GetNext();

            } // while interface

        } // vscpinterface
        else if ( child->GetName() == _( "mdfurl" ) ) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName() == _( "translate" ) ) {

                    wxString strFrom = subchild->GetAttribute( _( "from" ), _( "" ) );
                    wxString strTo = subchild->GetAttribute( _( "to" ), _( "" ) );
                    
                    // Both string needs to have a value
                    if ( strFrom.Length() && strTo.Length() ) {
                        g_Config.m_mfProxyHashTable[ strFrom ] = strTo;
                    }
                }

                subchild = subchild->GetNext();

            }
        }

        child = child->GetNext();

    } // while child

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// writeConfiguration
//
// Write the configuration file
//

bool VscpworksApp::writeConfiguration( void )
{
    int i;
    wxString strcfgfile;
    wxString buf;
    //wxStandardPaths strpath;

    if ( !m_bUseGlobalConfig ) {
        strcfgfile = wxStandardPaths::Get().GetUserDataDir();
        strcfgfile += _( "/vscpworks.conf" );
    }
    else {
        strcfgfile = wxStandardPaths::Get().GetConfigDir() + _( "/vscpworks.conf" );
    }

    // Backup
    if ( wxFileName::FileExists( strcfgfile ) ) {
        ::wxCopyFile( strcfgfile, strcfgfile + _( ".bak" ) );
    }

    wxFFileOutputStream *pFileStream = new wxFFileOutputStream( strcfgfile );
    if ( NULL == pFileStream ) return false;

    pFileStream->Write( "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                        strlen( "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n" ) );

    // VSCP Works configuration start
    pFileStream->Write( "<vscpworksconfig>\n\n", strlen( "<vscpworksconfig>\n\n" ) );




    // **************************************************************************



    // General
    pFileStream->Write( "<general>\n", strlen( "<general>\n" ) );

    // Width MainFrame
    pFileStream->Write( "<MainFrameWidth>", strlen( "<MainFrameWidth>" ) );
    buf.Printf( _( "%d" ), g_Config.m_sizeMainFrameWidth );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</MainFrameWidth>\n", strlen( "</MainFrameWidth>\n" ) );

    // Height MainFrame
    pFileStream->Write( "<MainFrameHeight>", strlen( "<MainFrameHeight>" ) );
    buf.Printf( _( "%d" ), g_Config.m_sizeMainFrameHeight );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</MainFrameHeight>\n", strlen( "</MainFrameHeight>\n" ) );

    // Xpos MainFrame
    pFileStream->Write( "<MainFramexpos>", strlen( "<MainFramexpos>" ) );
    buf.Printf( _( "%d" ), g_Config.m_xposMainFrame );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</MainFramexpos>\n", strlen( "</MainFramexpos>\n" ) );

    // Ypos MainFrame
    pFileStream->Write( "<MainFrameypos>", strlen( "<MainFrameypos>" ) );
    buf.Printf( _( "%d" ), g_Config.m_yposMainFrame );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</MainFrameypos>\n", strlen( "</MainFrameypos>\n" ) );
    
    // Width Session Frame
    pFileStream->Write( "<SessionFrameWidth>", strlen( "<SessionFrameWidth>" ) );
    buf.Printf( _( "%d" ), g_Config.m_sizeSessionFrameWidth );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</SessionFrameWidth>\n", strlen( "</SessionFrameWidth>\n" ) );

    // Height Session Frame
    pFileStream->Write( "<SessionFrameHeight>", strlen( "<SessionFrameHeight>" ) );
    buf.Printf( _( "%d" ), g_Config.m_sizeSessionFrameHeight );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</SessionFrameHeight>\n", strlen( "</SessionFrameHeight>\n" ) );
    
    // Width Configuration Frame
    pFileStream->Write( "<ConfigurationFrameWidth>", strlen( "<ConfigurationFrameWidth>" ) );
    buf.Printf( _( "%d" ), g_Config.m_sizeConfigurationFrameWidth );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</ConfigurationFrameWidth>\n", strlen( "</ConfigurationFrameWidth>\n" ) );

    // Height Configuration Frame
    pFileStream->Write( "<ConfigurationFrameHeight>", strlen( "<ConfigurationFrameHeight>" ) );
    buf.Printf( _( "%d" ), g_Config.m_sizeConfigurationFrameHeight );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</ConfigurationFrameHeight>\n", strlen( "</ConfigurationFrameHeight>\n" ) );

    // Path2TempFile
    pFileStream->Write( "<path2tempfile>", strlen( "<path2tempfile>" ) );
    pFileStream->Write( g_Config.m_strPathTemp.mb_str(), strlen( g_Config.m_strPathTemp.mb_str() ) );
    pFileStream->Write( "</path2tempfile>\n", strlen( "</path2tempfile>\n" ) );

    // Path2LogFile  
    pFileStream->Write( "<path2logfile enable=\"", strlen( "<path2logfile enable=\"" ) );
    if ( g_Config.m_bEnableLog ) {
        pFileStream->Write( "true", strlen( "true" ) );
    }
    else {
        pFileStream->Write( "false", strlen( "false" ) );
    }

    pFileStream->Write( "\" level=\"", strlen( "\" level=\"" ) );

    switch ( g_Config.m_logLevel ) {

        case VSCPWORKS_LOGMSG_DEBUG:
            pFileStream->Write( "debug", strlen( "debug" ) );
            break;

        case VSCPWORKS_LOGMSG_INFO:
            pFileStream->Write( "info", strlen( "info" ) );
            break;

        case VSCPWORKS_LOGMSG_NOTICE:
            pFileStream->Write( "notice", strlen( "notice" ) );
            break;

        case VSCPWORKS_LOGMSG_WARNING:
            pFileStream->Write( "warning", strlen( "warning" ) );
            break;

        case VSCPWORKS_LOGMSG_ERROR:
            pFileStream->Write( "error", strlen( "error" ) );
            break;

        case VSCPWORKS_LOGMSG_CRITICAL:
            pFileStream->Write( "critical", strlen( "critical" ) );
            break;

        case VSCPWORKS_LOGMSG_ALERT:
            pFileStream->Write( "alert", strlen( "alert" ) );
            break;

        default:
            pFileStream->Write( "emergency", strlen( "emergency" ) );
            break;
    }

    pFileStream->Write( "\" >", strlen( "\" >" ) );
    pFileStream->Write( g_Config.m_strPathLogFile.mb_str(), strlen( g_Config.m_strPathLogFile.mb_str() ) );
    pFileStream->Write( "</path2logfile>\n", strlen( "</path2logfile>\n" ) );

    // CANALReadMaxRetries
    pFileStream->Write( "<CANALReadMaxRetries>", strlen( "<CANALReadMaxRetries>" ) );
    buf.Printf( _( "%d" ), g_Config.m_CANALRegMaxRetries );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</CANALReadMaxRetries>\n", strlen( "</CANALReadMaxRetries>\n" ) );

    // CANALReadResendTimeout
    pFileStream->Write( "<CANALReadResendTimeout>", strlen( "<CANALReadResendTimeout>" ) );
    buf.Printf( _( "%d" ), g_Config.m_CANALRegResendTimeout );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</CANALReadResendTimeout>\n", strlen( "</CANALReadResendTimeout>\n" ) );

    // CANALReadErrorTimeout
    pFileStream->Write( "<CANALReadErrorTimeout>", strlen( "<CANALReadErrorTimeout>" ) );
    buf.Printf( _( "%d" ), g_Config.m_CANALRegErrorTimeout );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</CANALReadErrorTimeout>\n", strlen( "</CANALReadErrorTimeout>\n" ) );


    // TCPIPResponseTimeout
    pFileStream->Write( "<TCPIPResponseTimeout>", strlen( "<TCPIPResponseTimeout>" ) );
    buf.Printf( _( "%d" ), g_Config.m_TCPIP_ResponseTimeout );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</TCPIPResponseTimeout>\n", strlen( "</TCPIPResponseTimeout>\n" ) );

    // TCPIPResponseTimeout
    pFileStream->Write( "<TCPIPSleepAfterCommand>", strlen( "<TCPIPSleepAfterCommand>" ) );
    buf.Printf( _( "%d" ), g_Config.m_TCPIP_SleepAfterCommand );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</TCPIPSleepAfterCommand>\n", strlen( "</TCPIPSleepAfterCommand>\n" ) );

    // TCPIPReadMaxRetries
    pFileStream->Write( "<TCPIPReadMaxRetries>", strlen( "<TCPIPReadMaxRetries>" ) );
    buf.Printf( _( "%d" ), g_Config.m_TCPIPRegMaxRetries );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</TCPIPReadMaxRetries>\n", strlen( "</TCPIPReadMaxRetries>\n" ) );

    // TCPIPReadResendTimeout
    pFileStream->Write( "<TCPIPReadResendTimeout>", strlen( "<TCPIPReadResendTimeout>" ) );
    buf.Printf( _( "%d" ), g_Config.m_TCPIPRegResendTimeout );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</TCPIPReadResendTimeout>\n", strlen( "</TCPIPReadResendTimeout>\n" ) );

    // TCPIPReadErrorTimeout
    pFileStream->Write( "<TCPIPReadErrorTimeout>", strlen( "<TCPIPReadErrorTimeout>" ) );
    buf.Printf( _( "%d" ), g_Config.m_TCPIPRegErrorTimeout );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
    pFileStream->Write( "</TCPIPReadErrorTimeout>\n", strlen( "</TCPIPReadErrorTimeout>\n" ) );

    // ConfigNumberBase
    pFileStream->Write( "<NumberBase>", strlen( "<NumberBase>" ) );
    if ( VSCP_DEVCONFIG_NUMBERBASE_DECIMAL == g_Config.m_Numberbase ) {
        pFileStream->Write( "Decimal", strlen( "Decimal" ) );
    }
    else {
        pFileStream->Write( "Hex", strlen( "Hex" ) );
    }
    pFileStream->Write( "</NumberBase>\n", strlen( "</NumberBase>\n" ) );

    // ConfirmDelete
    pFileStream->Write( "<ConfirmDelete>", strlen( "<ConfirmDelete>" ) );
    if ( g_Config.m_bConfirmDelete ) {
        pFileStream->Write( "true", strlen( "true" ) );
    }
    else {
        pFileStream->Write( "false", strlen( "false" ) );
    }
    pFileStream->Write( "</ConfirmDelete>\n", strlen( "</ConfirmDelete>\n" ) );


    
    pFileStream->Write( "<GuidWriteEnable>", strlen( "<GuidWriteEnable>" ) );
    if ( g_Config.bGuidWritable ) {    
        pFileStream->Write( "enable", strlen( "enable" ) );
    }
    else {
        pFileStream->Write( "do-not-use", strlen( "do-not-use" ) );
    }
    pFileStream->Write( "</GuidWriteEnable>\n", strlen( "</GuidWriteEnable>\n" ) );

    pFileStream->Write( "<ManufacturerGuid>", strlen( "<ManufacturerGuid>" ) );
    {
        wxString wxstr;
        g_Config.m_manufacturerGuid.toString( wxstr );
        pFileStream->Write( wxstr.mb_str(), strlen( wxstr.mb_str() ) );
    }
    pFileStream->Write( "</ManufacturerGuid>\n", strlen( "</ManufacturerGuid>\n" ) );


    pFileStream->Write( "<ManufacturerId>", strlen( "<ManufacturerId>" ) );
    {
        wxString wxstr;
        wxstr = wxString::Format( _( "0x%08X" ), g_Config.m_manufacturerId );
        pFileStream->Write( wxstr.mb_str(), strlen( wxstr.mb_str() ) );
    }
    pFileStream->Write( "</ManufacturerId>\n", strlen( "</ManufacturerId>\n" ) );


    pFileStream->Write( "<ManufacturerSubId>", strlen( "<ManufacturerSubId>" ) );
    {
        wxString wxstr;
        wxstr = wxString::Format( _( "0x%08X" ), g_Config.m_manufacturerSubId );
        pFileStream->Write( wxstr.mb_str(), strlen( wxstr.mb_str() ) );
    }
    pFileStream->Write( "</ManufacturerSubId>\n", strlen( "</ManufacturerSubId>\n" ) );

    // /General
    pFileStream->Write("</general>\n\n",strlen("</general>\n\n"));


    // **************************************************************************


    // mdfurl
    pFileStream->Write( "<mdfurl>\n", strlen( "<mdfurl>\n" ) );

    MdfProxyHash::iterator it;
    for ( it = g_Config.m_mfProxyHashTable.begin(); it != g_Config.m_mfProxyHashTable.end(); ++it )
    {
        wxString key = it->first, value = it->second;
        pFileStream->Write( "<translate ", strlen( "<translate " ) );

        pFileStream->Write( "from=\"", strlen( "from=\"" ) );
        pFileStream->Write( key.mbc_str(), strlen( key.mbc_str() ) );
        pFileStream->Write( "\" ", strlen( "\" " ) );

        pFileStream->Write( "to=\"", strlen( "to=\"" ) );
        pFileStream->Write( value.mbc_str(), strlen( value.mbc_str() ) );
        pFileStream->Write( "\" ", strlen( "\" " ) );

        pFileStream->Write( "/>\n", strlen( "/>\n" ) );
    }

    pFileStream->Write( "</mdfurl>\n\n", strlen( "</mdfurl>\n\n" ) );

    // **************************************************************************



    // vscpclient
    pFileStream->Write("<vscpclient>\n",strlen("<vscpclient>\n"));

    pFileStream->Write("<Autoscroll>",strlen("<Autoscroll>"));
    if ( g_Config.m_bAutoscollRcv ) {
        pFileStream->Write("true",strlen("true"));
    }
    else {
        pFileStream->Write("false",strlen("false"));
    }
    pFileStream->Write("</Autoscroll>\n",strlen("</Autoscroll>\n"));  


    pFileStream->Write("<VscpRcvPyjamasStyle>",strlen("<VscpRcvPyjamasStyle>"));
    if ( g_Config.m_VscpRcvFrameRxbPyamas ) {
        pFileStream->Write("true",strlen("true"));
    }
    else {
        pFileStream->Write("false",strlen("false"));
    }
    pFileStream->Write("</VscpRcvPyjamasStyle>\n",strlen("</VscpRcvPyjamasStyle>\n")); 


    pFileStream->Write("<VscpUseSymbolicNames>",strlen("<VscpUseSymbolicNames>"));
    if ( g_Config.m_UseSymbolicNames ) {
        pFileStream->Write("true",strlen("true"));
    }
    else {
        pFileStream->Write("false",strlen("false"));
    }
    pFileStream->Write("</VscpUseSymbolicNames>\n",strlen("</VscpUseSymbolicNames>\n"));


    pFileStream->Write("<VscpRcvRowLineHeight>",strlen("<VscpRcvRowLineHeight>"));
    buf.Printf(_("%d"), g_Config.m_VscpRcvFrameRowLineHeight );
    pFileStream->Write( buf.mb_str(), strlen(buf.mb_str() ) );
    pFileStream->Write("</VscpRcvRowLineHeight>\n", strlen("</VscpRcvRowLineHeight>\n"));


    for ( i=0; i<4; i++ ) {
        wxString str;
        str.Printf( _("<VscpRcvShowField%d  enable=\""), i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));

        if ( g_Config.m_VscpRcvShowField[i] ) {
            str = _("true\" >");
        }
        else {
            str = _("false\" >");
        }
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));

        str.Printf( _("</VscpRcvShowField%d>\n"), i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));
    }

    for ( i=0; i<4; i++ ) {
        wxString str;
        str.Printf( _("<VscpRcvFieldOrder%d>%d</VscpRcvFieldOrder%d>\n"), 
            i, 
            g_Config.m_VscpRcvFieldOrder[i], i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));
    }

    for ( i=0; i<4; i++ ) {
        wxString str;
        str.Printf( _("<VscpRcvFieldText%d>"), i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));

        pFileStream->Write( g_Config.m_VscpRcvFieldText[i].mb_str(), 
            strlen( g_Config.m_VscpRcvFieldText[i].mb_str() ));

        str.Printf( _("</VscpRcvFieldText%d>\n"), i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));
    }

    for ( i=0; i<4; i++ ) {
        wxString str;
        str.Printf( _("<VscpRcvFieldWidth%d>%d</VscpRcvFieldWidth%d>\n"), 
            i, 
            g_Config.m_VscpRcvFieldWidth[i], i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));
    }

    // ----------------------

    for ( i=0; i<6; i++ ) {
        wxString str;
        str.Printf( _("<VscpTrmitShowField%d  enable=\""), i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));

        if ( g_Config.m_VscpTrmitShowField[i] ) {
            str = _("true\" >");
        }
        else {
            str = _("false\" >");
        }
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));

        str.Printf( _("</VscpTrmitShowField%d>\n"), i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));
    }

    for ( i=0; i<6; i++ ) {
        wxString str;
        str.Printf( _("<VscpTrmitFieldOrder%d>%d</VscpTrmitFieldOrder%d>\n"), 
            i, 
            g_Config.m_VscpTrmitFieldOrder[i], i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));
    }

    for ( i=0; i<6; i++ ) {
        wxString str;
        str.Printf( _("<VscpTrmitFieldText%d>"), i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));

        pFileStream->Write( g_Config.m_VscpTrmitFieldText[i].mb_str(), 
            strlen( g_Config.m_VscpTrmitFieldText[i].mb_str() ));

        str.Printf( _("</VscpTrmitFieldText%d>\n"), i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));
    }

    for ( i=0; i<6; i++ ) {
        wxString str;
        str.Printf( _("<VscpTrmitFieldWidth%d>%d</VscpTrmitFieldWidth%d>\n"), 
            i, 
            g_Config.m_VscpTrmitFieldWidth[i], i );
        pFileStream->Write( str.mb_str(), strlen( str.mb_str() ));
    }


    pFileStream->Write("<VscpRcvFrameRxTextColour " ,strlen("<VscpRcvFrameRxTextColour "));
    buf.Printf( _(" r=\"%d\" g=\"%d\" b=\"%d\" >"), 
        g_Config.m_VscpRcvFrameRxTextColour.Red(),
        g_Config.m_VscpRcvFrameRxTextColour.Green(),
        g_Config.m_VscpRcvFrameRxTextColour.Blue() );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ));															
    pFileStream->Write("</VscpRcvFrameRxTextColour>\n",strlen("</VscpRcvFrameRxTextColour>\n"));

    pFileStream->Write("<VscpRcvFrameRxBgColour " ,strlen("<VscpRcvFrameRxBgColour "));
    buf.Printf( _(" r=\"%d\" g=\"%d\" b=\"%d\" >"), 
        g_Config.m_VscpRcvFrameRxBgColour.Red(),
        g_Config.m_VscpRcvFrameRxBgColour.Green(),
        g_Config.m_VscpRcvFrameRxBgColour.Blue() );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ));															
    pFileStream->Write("</VscpRcvFrameRxBgColour>\n",strlen("</VscpRcvFrameRxBgColour>\n"));

    pFileStream->Write("<VscpRcvFrameTxTextColour " ,strlen("<VscpRcvFrameTxTextColour "));
    buf.Printf( _(" r=\"%d\" g=\"%d\" b=\"%d\" >"), 
        g_Config.m_VscpRcvFrameTxTextColour.Red(),
        g_Config.m_VscpRcvFrameTxTextColour.Green(),
        g_Config.m_VscpRcvFrameTxTextColour.Blue() );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );															
    pFileStream->Write("</VscpRcvFrameTxTextColour>\n",strlen("</VscpRcvFrameTxTextColour>\n"));

    pFileStream->Write("<VscpRcvFrameTxBgColour " ,strlen("<VscpRcvFrameTxBgColour "));
    buf.Printf( _(" r=\"%d\" g=\"%d\" b=\"%d\" >"), 
        g_Config.m_VscpRcvFrameTxBgColour.Red(),
        g_Config.m_VscpRcvFrameTxBgColour.Green(),
        g_Config.m_VscpRcvFrameTxBgColour.Blue() );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ));															
    pFileStream->Write("</VscpRcvFrameTxBgColour>\n",strlen("</VscpRcvFrameTxBgColour>\n"));

    pFileStream->Write("<VscpRcvFrameLineColour " ,strlen("<VscpRcvFrameLineColour "));
    buf.Printf( _(" r=\"%d\" g=\"%d\" b=\"%d\" >"), 
        g_Config.m_VscpRcvFrameLineColour.Red(),
        g_Config.m_VscpRcvFrameLineColour.Green(),
        g_Config.m_VscpRcvFrameLineColour.Blue() );
    pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ));															
    pFileStream->Write("</VscpRcvFrameLineColour>\n",strlen("</VscpRcvFrameLineColour>\n"));

    // /vscpclient
    pFileStream->Write("</vscpclient>\n\n",strlen("</vscpclient>\n\n"));



    // **************************************************************************


    // Interfaces start
    pFileStream->Write("<vscpinterface>\n",strlen("<vscpinterface>\n"));

    // Canal drivers
    if ( g_Config.m_canalIfList.GetCount() ) {

        LIST_CANAL_IF::iterator iter;
        for (iter = g_Config.m_canalIfList.begin(); iter != g_Config.m_canalIfList.end(); ++iter) {

            canal_interface *pIf = *iter;

            // CANAL Driver interface start
            pFileStream->Write("<canaldriver>\n",strlen("<canaldriver>\n"));

            // description
            pFileStream->Write("<description>",strlen("<description>"));
            pFileStream->Write( pIf->m_strDescription.mb_str(), strlen( pIf->m_strDescription.mb_str() ) );
            pFileStream->Write("</description>\n",strlen("</description>\n"));

            // path
            pFileStream->Write("<path>",strlen("<path>"));
            pFileStream->Write( pIf->m_strPath.mb_str(), strlen( pIf->m_strPath.mb_str() ) );
            pFileStream->Write("</path>\n",strlen("</path>\n"));

            // config
            pFileStream->Write("<config>",strlen("<config>"));
            pFileStream->Write( pIf->m_strConfig.mb_str(), strlen( pIf->m_strConfig.mb_str() ) );
            pFileStream->Write("</config>\n",strlen("</confign"));

            // flags
            pFileStream->Write("<flags>",strlen("<flags>"));
            buf.Printf(_("%lu"), pIf->m_flags );
            pFileStream->Write( buf.mb_str(),strlen(buf.mb_str()));
            pFileStream->Write("</flags>\n",strlen("</flags>\n"));	

            // CANAL Driver interface stop
            pFileStream->Write("</canaldriver>\n\n",strlen("</canaldriver>\n\n"));

        }

    }



    // **************************************************************************



    // VSCP TCP/IP interface
    if ( g_Config.m_vscpIfList.GetCount() ) {

        LIST_VSCP_IF::iterator iter;
        for (iter = g_Config.m_vscpIfList.begin(); iter != g_Config.m_vscpIfList.end(); ++iter) {

            wxString strGUID;
            vscp_interface *pIf = *iter;

            // VSCP Daemon interface start
            pFileStream->Write("<vscpdaemon>\n",strlen("<vscpdaemon>\n"));

            // description
            pFileStream->Write("<description>",strlen("<description>"));
            pFileStream->Write( pIf->m_strDescription.mb_str(), strlen( pIf->m_strDescription.mb_str() ) );
            pFileStream->Write("</description>\n",strlen("</description>\n"));

            // host
            pFileStream->Write("<host>",strlen("<host>"));
            pFileStream->Write( pIf->m_strHost.mb_str(), strlen( pIf->m_strHost.mb_str() ) );
            pFileStream->Write("</host>\n",strlen("</host>\n"));

            // username
            pFileStream->Write("<username>",strlen("<username>"));
            pFileStream->Write( pIf->m_strUser.mb_str(), strlen( pIf->m_strUser.mb_str() ) );
            pFileStream->Write("</username>\n",strlen("</username>\n"));

            // password
            pFileStream->Write("<password>",strlen("<password>"));
            pFileStream->Write( pIf->m_strPassword.mb_str(), strlen( pIf->m_strPassword.mb_str() ) );
            pFileStream->Write("</password>\n",strlen("</password>\n"));

            // Daemon interface
            pFileStream->Write("<interfacename>",strlen("<interfacename>"));
            pFileStream->Write( pIf->m_strInterfaceName.mb_str(), strlen( pIf->m_strInterfaceName.mb_str() ) );
            pFileStream->Write("</interfacename>\n",strlen("</interfacename>\n"));

            // Fill Level II support
            pFileStream->Write("<level2>",strlen("<level2>"));
            if ( pIf->m_bLevel2 ) {
                pFileStream->Write( "true", 4 );
            }
            else {
                pFileStream->Write( "false", 5 );
            }
            pFileStream->Write("</level2>\n",strlen("</level2>\n"));

            // interface GUID
            pFileStream->Write("<guid>",strlen("<guid>"));
            vscp_writeGuidArrayToString( pIf->m_GUID, buf );	
            pFileStream->Write( buf.mb_str(),strlen(buf.mb_str()));	
            pFileStream->Write("</guid>\n",strlen("</guid>\n"));

            // Filter
            pFileStream->Write ( "<filter ",strlen ( "<filter " ) );
            buf.Printf( _( " priority=\"%d\" " ), pIf->m_vscpfilter.filter_priority );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
            buf.Printf( _( " class=\"%d\" " ), pIf->m_vscpfilter.filter_class );
            pFileStream->Write ( buf.mb_str(), strlen( buf.mb_str() ) );
            buf.Printf( _( " type=\"%d\" " ), pIf->m_vscpfilter.filter_type );
            pFileStream->Write ( buf.mb_str(), strlen( buf.mb_str() ) );
            buf.Printf( _( " GUID=\"" ) );
            pFileStream->Write ( buf.mb_str(), strlen( buf.mb_str() ) );
            vscp_writeGuidArrayToString( pIf->m_vscpfilter.filter_GUID, strGUID );
            pFileStream->Write( strGUID.mb_str(), strlen( strGUID.mb_str() ) );
            pFileStream->Write( "\">",strlen ( "\">" ) );
            pFileStream->Write( "</filter>\n",strlen ( "</filter>\n" ) );

            // Mask
            pFileStream->Write ( "<mask ",strlen ( "<mask " ) );
            buf.Printf( _( " priority=\"%d\" " ), pIf->m_vscpfilter.mask_priority );
            pFileStream->Write ( buf.mb_str(), strlen( buf.mb_str() ) );
            buf.Printf( _( " class=\"%d\" " ), pIf->m_vscpfilter.mask_class );
            pFileStream->Write ( buf.mb_str(), strlen( buf.mb_str() ) );
            buf.Printf( _( " type=\"%d\" " ), pIf->m_vscpfilter.mask_type );
            pFileStream->Write ( buf.mb_str(), strlen( buf.mb_str() ) );
            buf.Printf( _( " GUID=\"" ) );
            pFileStream->Write ( buf.mb_str(), strlen( buf.mb_str() ) );
            vscp_writeGuidArrayToString( pIf->m_vscpfilter.mask_GUID, strGUID );
            pFileStream->Write ( strGUID.mb_str(), strlen( strGUID.mb_str() ) );
            pFileStream->Write ( "\">",strlen ( "\">" ) );
            pFileStream->Write ( "</mask>\n",strlen ( "</mask>\n" ) );

            // VSCP Daemon interface stop
            pFileStream->Write("</vscpdaemon>\n\n",strlen("</vscpdaemon>\n\n"));

        }

    }

    // Interfaces stop
    pFileStream->Write("</vscpinterface>\n\n",strlen("</vscpinterface>\n\n"));




    // **************************************************************************



    // VSCP Works configuration end
    pFileStream->Write("</vscpworksconfig>\n",strlen("</vscpworksconfig>\n"));

    // Close the file
    pFileStream->Close();

    // Clean up
    delete pFileStream;

    return true;
}

/*
//////////////////////////////////////////////////////////////////////////////
// readLevel1Register
//

bool VscpworksApp::readLevel1Register( CCanalSuperWrapper *pcsw,
                                            unsigned char nodeid, 
                                            unsigned char reg, 
                                            unsigned char *pcontent )
{
    bool rv = true;
    int errors = 0;
    bool bResend;
    wxString strBuf;
    canalMsg canalEvent;

    // Check pointer
    if ( NULL == pcsw ) return false;

    canalEvent.flags = CANAL_IDFLAG_EXTENDED;
    canalEvent.obid = 0;
    canalEvent.id = 0x0900;             // CLASS1.PROTOCOL Read register
    canalEvent.sizeData = 2;
    canalEvent.data[ 0 ] = nodeid;      // Node to read from
    canalEvent.data[ 1 ] = reg;         // Register to read

    bResend = false;
    pcsw->doCmdSend( &canalEvent );

    //wxDateTime start = wxDateTime::Now();
    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( true ) {

        if ( pcsw->doCmdDataAvailable() ) {	                                    // Message available
            if ( CANAL_ERROR_SUCCESS == pcsw->doCmdReceive( &canalEvent ) ) {   // Valid event
                if ( (unsigned short)( canalEvent.id & 0xffff ) ==
                    ( 0x0a00 + nodeid ) ) {                 // Read reply?
                        if ( canalEvent.data[ 0 ] == reg ) {// Requested register?

                            if ( NULL != pcontent ) {
                                *pcontent = canalEvent.data[ 1 ];
                            }
                            break;

                        } // Check for correct node
                } // Check for correct reply event 
            }
        }
        else {
            wxMilliSleep( 1 );
        }

        // Check for read error timeout
        if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
            g_Config.m_VscpRegisterReadErrorTimeout ) {
                errors++;
        }
        // Should we resend?
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
            g_Config.m_VscpRegisterReadResendTimeout ) {
                // Send again
                if ( !bResend) {
                    pcsw->doCmdSend( &canalEvent );
                }
                bResend = true;
        }

        if ( errors > g_Config.m_VscpRegisterReadMaxRetries ) {
            rv = false;
            break;
        }

    } // while

    return rv;
}
*/

//////////////////////////////////////////////////////////////////////////////
// writeLevel1Register
//

bool VscpworksApp::writeLevel1Register( CCanalSuperWrapper *pcsw,
                                       unsigned char nodeid, 
                                       unsigned char reg, 
                                       unsigned char *pcontent )
{
    bool rv = true;
    int errors = 0;
    bool bResend;
    wxString strBuf;
    canalMsg canalEvent;

    // Check pointer
    if ( NULL == pcsw ) return false;

    // Check pointer
    if ( NULL == pcontent ) return false;

    canalEvent.flags = CANAL_IDFLAG_EXTENDED;
    canalEvent.obid = 0;
    canalEvent.id = 0x0B00;             // CLASS1.PROTOCOL Write register
    canalEvent.sizeData = 3;
    canalEvent.data[ 0 ] = nodeid;      // Node to read from
    canalEvent.data[ 1 ] = reg;         // Register to write
    canalEvent.data[ 2 ] = *pcontent;   // value to write

    bResend = false;
    pcsw->doCmdSend( &canalEvent );

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( true ) {

        if ( pcsw->doCmdDataAvailable() ) {                                     // Message available
            if ( CANAL_ERROR_SUCCESS == pcsw->doCmdReceive( &canalEvent ) ) {   // Valid event
                if ( (unsigned short)( canalEvent.id & 0xffff ) ==
                    ( 0x0a00 + nodeid ) ) {         // Read reply?
                        if ( canalEvent.data[ 0 ] == reg ) {                    // Requested register?

                            if ( *pcontent != canalEvent.data[ 1 ] ) rv = false;
                            // Save read value
                            *pcontent = canalEvent.data[ 1 ];
                            break;
                        } // Check for correct node

                        // Save read value
                        *pcontent = canalEvent.data[ 1 ];

                } // Check for correct reply event 
            }
        }
        else {
            wxMilliSleep( 1 );
        }

        if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
            g_Config.m_CANALRegErrorTimeout ) {
                errors++;
        }
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
            g_Config.m_CANALRegResendTimeout ) {
                // Send again
                if ( !bResend) {
                    pcsw->doCmdSend( &canalEvent );
                }
                bResend = true;
        }

        if ( errors > g_Config.m_CANALRegMaxRetries ) {
            rv = false;
            break;
        }

    } // while

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// readLevel2Register
//

bool VscpworksApp::readLevel2Register( CCanalSuperWrapper *pcsw,
                                        uint8_t *interfaceGUID, 
                                        uint32_t reg, 
                                        uint8_t *pcontent,
                                        uint8_t *pdestGUID,
                                        bool bLevel2 )
{
    int i;
    bool rv = true;
    bool bInterface = false;  // No specific interface set
    int errors = 0;
    bool bResend;
    wxString strBuf;
    vscpEventEx event;

    // Check pointer
    if ( NULL == pcsw ) return false;

    // Check if a specific interface is used
    for ( i=0; i<16; i++ ) {
        if ( interfaceGUID[ i ] ) {
            bInterface= true;
            break;
        }
    }

    if ( bInterface ) {

        // Event should be sent to a specific interface

        event.head = VSCP_PRIORITY_NORMAL;
        event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        event.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;
        
        memset( event.GUID, 0, 16 );                // We use GUID for interface 
        
        event.sizeData = 16 + 2;                    // Interface GUID + nodeid + register to read
        
        for ( i=0; i<16; i++ ) {
            event.data[ i ] = interfaceGUID[ 15 - i ];
        }
            
        event.data[16] = interfaceGUID[0];          // nodeid
        event.data[17] = reg;                       // Register to read
    
    }

    else {

        // Event should be sent to all interfaces
    
        // Must have a destination GUID
        if ( NULL == pdestGUID ) return false;

        if ( bLevel2 ) {

            // True real Level II event

            event.head = VSCP_PRIORITY_NORMAL;
            event.vscp_class = VSCP_CLASS2_PROTOCOL;
            event.vscp_type = VSCP2_TYPE_PROTOCOL_READ_REGISTER;
        
            memset( event.GUID, 0, 16 );        // We use GUID for interface 

            event.sizeData = 22;                // nodeid + register to read
        
            for ( i=0; i<16; i++ ) {            // Destination GUID
                event.data[ i ] = pdestGUID[ 15 - i ];	
            }	
            event.data[ 16 ] = 0x00;            // Register to read
            event.data[ 17 ] = 0x00;
            event.data[ 18 ] = 0x00;
            event.data[ 19 ] = reg;
            event.data[ 20 ] = 0x00;            // Read one register
            event.data[ 21 ] = 0x01;
        
        }
        else {

            // Level I over CLASS2 to all interfaces 

            event.head = VSCP_PRIORITY_NORMAL;
            event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            event.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;
        
            memset( event.GUID, 0, 16 );                // We use GUID for interface 
            event.sizeData = 16 + 2;                    // nodeid + register to read

            for ( i=0; i<16; i++ ) {
                event.data[ i ] = pdestGUID[ 15 - i ];
            }
            
            event.data[16] = 0x00;                      // nodeid
            event.data[17] = reg;                       // Register to read
            
        }
    }

    bResend = false;

    // Send the event
    pcsw->doCmdClear();
    event.timestamp = 0;
    pcsw->doCmdSend( &event );
    

    //wxDateTime start = wxDateTime::Now();
    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( true ) {

        if ( pcsw->doCmdDataAvailable() ) {                                     // Message available
            if ( CANAL_ERROR_SUCCESS == pcsw->doCmdReceive( &event ) ) {        // Valid event
                
                // Check for correct reply event
                
                // Level I Read reply?
                if ( bInterface && ( VSCP_CLASS1_PROTOCOL == event.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_RW_RESPONSE == event.vscp_type ) ) {   
                        if ( event.data[ 0 ] == reg ) {                         // Requested register?
                            if ( event.GUID[0] == interfaceGUID[0] ) {          // Correct node?
                                if ( NULL != pcontent ) {
                                    *pcontent = event.data[ 1 ];
                                    break;
                                }
                                break;
                            }
                        } // Check for correct node
                }
                // Level II 512 Read reply?
                else if ( !bInterface && !bLevel2 && 
                    ( VSCP_CLASS2_LEVEL1_PROTOCOL == event.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_RW_RESPONSE == event.vscp_type ) ) { 

                    if ( vscp_isSameGUID( pdestGUID, event.GUID ) ) {
                        // Reg we requested?
                        if ( event.data[ 0 ] == reg ) {
                            // OK get the data
                            if ( NULL != pcontent ) {
                                *pcontent = event.data[ 18 ];
                                break;
                            }
                        }
                    }

                }
                // Level II Read reply?
                else if ( !bInterface && bLevel2 && 
                    ( VSCP_CLASS2_PROTOCOL == event.vscp_class ) && 
                    ( VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == event.vscp_type ) ) { 
                    
                    // from us
                    if ( vscp_isSameGUID( pdestGUID, event.GUID ) ) {	
                        
                        uint32_t retreg = ( event.data[ 0 ]  << 24 ) +
                                          (	event.data[ 1 ]  << 16 ) +
                                          (	event.data[ 2 ]  << 8 ) +
                                            event.data[ 3 ];
                        
                        // Reg we requested?
                        if ( retreg == reg ) {
                            // OK get the data
                            if ( NULL != pcontent ) {
                                *pcontent = event.data[ 18 ];
                                break;
                            }
                        }
                    }

                }
            } // valid event
        }
        else {
            //wxMilliSleep( 1 );
        }

        if ( ( ::wxGetLocalTimeMillis() - startTime ) >   
                    g_Config.m_CANALRegErrorTimeout ) {
            errors++;
        }
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
            g_Config.m_CANALRegResendTimeout ) {
                // Send again
                if ( !bResend) {
                    pcsw->doCmdClear();
                    event.timestamp = 0;
                    pcsw->doCmdSend( &event );
                }
                bResend = true;
        }   

        if ( errors > g_Config.m_CANALRegMaxRetries ) {
            rv = false;
            break;
        }

    } // while

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// writeLevel2Register
//

bool VscpworksApp::writeLevel2Register( CCanalSuperWrapper *pcsw,
                                            uint8_t *interfaceGUID, 
                                            uint32_t reg, 
                                            uint8_t *pcontent,
                                            uint8_t *pdestGUID,
                                            bool bLevel2 )
{
    int i;
    bool rv = true;
    bool bInterface = false;  // No specific interface set
    int errors = 0;
    bool bResend;
    wxString strBuf;
    vscpEventEx event;

    // Check pointers
    if ( NULL == pcsw ) return false;
    if ( NULL == pcontent ) return false;

    // Check if a specific interface is used
    for ( i=0; i<16; i++ ) {
        if ( interfaceGUID[ i ] ) {
            bInterface= true;
            break;
        }
    }

    if ( bInterface ) {

        event.head = VSCP_PRIORITY_NORMAL;
        event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        event.vscp_type = VSCP_TYPE_PROTOCOL_WRITE_REGISTER;
        
        memset( event.GUID, 0, 16 );              // We use interface GUID
        
        event.sizeData = 16 + 3;                  // Interface GUID + nodeid + register to read + valied
        
        for ( i=0; i<16; i++ ) {
            event.data[ i ] = interfaceGUID[ 15 - i ];
        }
        event.data[16] = interfaceGUID[ 0 ];      // nodeid
        event.data[17] = reg;                     // Register to write
        event.data[18] = *pcontent;	              // value to write
    
    }
    else {

        if ( bLevel2 ) {

            // Must have a destination GUID
            if ( NULL == pdestGUID ) return false;

            event.head = VSCP_PRIORITY_NORMAL;
            event.vscp_class = VSCP_CLASS2_PROTOCOL;
            event.vscp_type = VSCP2_TYPE_PROTOCOL_WRITE_REGISTER;

            memset( event.GUID, 0, 16 );        // We use interface GUID
        
            event.sizeData = 21;                // nodeid + register to read
        
            for ( i=0; i<16; i++ ) {            // Destination GUID
                event.data[ i ] = pdestGUID[ 15 - i ];
            }
                
            event.data[ 16 ] = 0x00;            // Register to write
            event.data[ 17 ] = 0x00;
            event.data[ 18 ] = 0x00;
            event.data[ 19 ] = reg;
            event.data[ 20 ] = *pcontent;       // Data to write
        
        }
        else {

            // Level I over CLASS2 to all interfaces 

            event.head = VSCP_PRIORITY_NORMAL;
            event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            event.vscp_type = VSCP_TYPE_PROTOCOL_WRITE_REGISTER;

            memset( event.GUID, 0, 16 );            // We use interface GUID
        
            event.sizeData = 16 + 3;

            for ( i=0; i<16; i++ ) {
                event.data[ i ] = interfaceGUID[ 15 - i ];
            }

            event.data[16] = interfaceGUID[0];        // nodeid
            event.data[17] = reg;                     // Register to write
            event.data[18] = *pcontent;	              // value to write
            
        }

    }

    bResend = false;
    pcsw->doCmdSend( &event );

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( true ) {

        if ( pcsw->doCmdDataAvailable() ) {                                 // Message available
            if ( CANAL_ERROR_SUCCESS == pcsw->doCmdReceive( &event ) ) {    // Valid event
                if ( ( VSCP_CLASS1_PROTOCOL == event.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_RW_RESPONSE == event.vscp_type ) ) {   // Read reply?
                        if ( event.data[ 0 ] == reg ) {                         // Requested register?
                            if ( event.GUID[0] == interfaceGUID[0] ) {          // Correct node?

                                // We go a rw reply from the correct node is
                                // the written data same as we expect.
                                if ( *pcontent != event.data[ 1 ] ) rv = false;
                                break;

                            }
                        }   // Check for correct node
                }       // Check for correct reply event 
            }
        }
        else {
            wxMilliSleep( 1 );
        }

        if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
            g_Config.m_CANALRegErrorTimeout ) {
                errors++;
        }
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
            g_Config.m_CANALRegResendTimeout ) {
                // Send again
                if ( !bResend) {
                    pcsw->doCmdSend( &event );
                }
                bResend = true;
        }

        if ( errors > g_Config.m_CANALRegMaxRetries ) {
            rv = false;
            break;
        }

    } // while

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// getMDFfromDevice
//

wxString VscpworksApp::getMDFfromDevice( CCanalSuperWrapper *pcsw, 
                                            uint8_t *pid, 
                                            bool bLevel2,
                                            bool bSilent )
{
    wxString strWrk;
    char url[ 33 ];

    memset( url, 0, sizeof( url ) );

    if ( bLevel2 ) {
        
        // Level 2 device
        uint8_t *p = (uint8_t *)url;
        for ( int i=0; i<32; i++ ) {
            if ( !pcsw->getDllInterface()->readLevel1Register( 0,
                                                                *pid, 
                                                                0xE0 + i, 
                                                                p++ ) ) {
                if ( !bSilent ) {												
                    ::wxMessageBox( _("Unable to read register."), _("VSCP Works"), wxICON_ERROR );
                }
                break;
             }
            
        }

    }
    else {
    
        // Level 1 device
        uint8_t *p = (uint8_t *)url;
        for ( int i=0; i<32; i++ ) {
             if ( !readLevel2Register( pcsw,
                                        pid, 
                                        0xFFFFFFE0 + i, 
                                        p++ ) ) {
                if ( !bSilent ) {
                    ::wxMessageBox( _("Unable to read register."), _("VSCP Works"), wxICON_ERROR );
                }
                break;
             }
            
        }

    }

    return strWrk;
}


//////////////////////////////////////////////////////////////////////////////
// formatString
//

wxString VscpworksApp::formatString( const wxString& str, const unsigned int width )
{
    const int stateNone = 0;
    const int stateEscape = 1;
    wxString strWrk;
    wxString strLine;
    unsigned char state = stateNone;
    bool bBreakAtNextSpace = false;

    for ( unsigned int i=0; i<str.Length(); i++ ) {
        switch ( state ) {

            case stateNone:
                // Check for escape
                if ( '\\' == str[i] ) {
                    state = stateEscape;
                }
                else if ( bBreakAtNextSpace && ' ' == str[i] ) {
                    bBreakAtNextSpace = false;
                    strWrk += strLine + _("\n");
                    strLine.Empty();
                }
                else {
                    strLine += str[i];
                }
                break;

            case stateEscape:

                // Restore default state
                state = stateNone;

                // Ceck for backslash
                if ( '\\' == str[i] ) {
                    strLine += _("\\");	
                }
                // Check for new line
                else if ( 'n' == str[i] ) {
                    // New line
                    strWrk += strLine + _("\n");
                    strLine.Empty();
                    bBreakAtNextSpace = false;
                }
                break;

        } // case

        if ( strLine.Length() > width ) {
            bBreakAtNextSpace = true;	
        }

    } // for

    strWrk += strLine;

    return strWrk;
}





//////////////////////////////////////////////////////////////////////////////
// getLevel1DmInfo
//
/*
bool VscpworksApp::getLevel1DmInfo( CCanalSuperWrapper *pcsw, 
                                   const unsigned char nodeid, 
                                   unsigned char *pdata )
{
    bool rv = true;
    bool bResend;
    wxString strBuf;
    canalMsg canalEvent;

    // Check pointer
    if ( NULL == pdata ) return false;

    canalEvent.flags = CANAL_IDFLAG_EXTENDED;
    canalEvent.obid = 0;
    canalEvent.id = 0x2000;             // CLASS1.PROTOCOL Get decision matrix info
    canalEvent.sizeData = 1;
    canalEvent.data[ 0 ] = nodeid;      // Node to read from

    bResend = false;
    pcsw->doCmdSend( &canalEvent );

    wxDateTime start = wxDateTime::Now();

    while ( true ) {

        if ( pcsw->doCmdDataAvailable() ) {                         // Message available
            if ( pcsw->doCmdReceive( &canalEvent ) ) {              // Valid message
                if ( (unsigned short)( canalEvent.id & 0xffff ) ==
                    ( 0x2100 + nodeid ) ) {                     // DM info reply?
                        // Copy in response data
                        memcpy( pdata, canalEvent.data, 8 );
                        break;
                }
            }
        }

        if ( (wxDateTime::Now() - start).GetSeconds() > 2 ) {
            rv = false;
            break;
        }
        else if ( (wxDateTime::Now() - start).GetSeconds() > 1 ) {
            // Send again
            if ( !bResend) {
                pcsw->doCmdSend( &canalEvent );
            }
            bResend = true;
        }
    } // while

    return rv;
}
*/

//////////////////////////////////////////////////////////////////////////////
// getLevel2DmInfo
//
/*
bool VscpworksApp::getLevel2DmInfo( CCanalSuperWrapper *pcsw,
                                   unsigned char *interfaceGUID, 
                                   unsigned char *pdata,
                                   bool bLevel2 )
{
    bool rv = true;
    bool bResend;
    wxString strBuf;
    vscpEventEx event;

    // Check pointer
    if ( NULL == pdata ) return false;

    event.head = 0;
    event.vscp_class = 512;                   // CLASS2.PROTOCOL1
    event.vscp_type = 9;                      // Get decision matrix info
    memset( event.GUID, 0, 16 );              // We use interface GUID
    event.sizeData = 16 + 1;                  // Interface GUID + nodeid 
    memcpy( event.data, interfaceGUID, 16 );  // Address node
    event.data[16] = interfaceGUID[0];        // nodeid


    bResend = false;
    pcsw->doCmdSend( &event );

    wxDateTime start = wxDateTime::Now();

    while ( true ) {

        if ( pcsw->doCmdDataAvailable() ) {									// Message available
            if ( CANAL_ERROR_SUCCESS == pcsw->doCmdReceive( &event ) ) {    // Valid event
                if ( ( 0 == event.vscp_class ) && 
                    ( 0x21 == event.vscp_type ) ) {							// DM reply?
                        memcpy( pdata, event.data, 8 );
                        break;
                }
            }
        }

        if ( (wxDateTime::Now() - start).GetSeconds() > 2 ) {
            rv = false;
            break;
        }
        else if ( (wxDateTime::Now() - start).GetSeconds() > 1 ) {
            // Send again
            if ( !bResend) {
                pcsw->doCmdSend( &event );
            }
            bResend = true;
        }
    } // while

    return rv;
}
*/
//////////////////////////////////////////////////////////////////////////////
// getHtmlStatusInfo
//
/*
wxString VscpworksApp::getHtmlStatusInfo( CCanalSuperWrapper *pcsw,
                                            const uint8_t *registers )
{
    wxString strHTML;
    wxString str;

    strHTML = _("<html><body>");
    strHTML += _("<h4>Clear text node data</h4>");
    strHTML += _("<font color=\"#009900\">");

    strHTML += _("nodeid = ");
    str = wxString::Format(_("%d"), registers[0x91] );
    strHTML += str;
    strHTML += _("<br>");

    
    //if ( USE_TCPIP_INTERFACE == m_csw.getDeviceType() ) {
    //strHTML += _("Daemon Interface = ");
    //strHTML += m_comboNodeID->GetValue();
    //strHTML += _("<br>");
    //}
      
    strHTML += _("GUID = ");
    writeGuidArrayToString( registers + 0xd0, str );
    strHTML += str;
    strHTML += _("<br>");

    strHTML += _("MDF URL = ");
    char url[33];
    memset( url, 0, sizeof( url ) );
    memcpy( url, registers + 0xe0, 32 );
    str = wxString::From8BitData( url );
    strHTML += str;
    strHTML += _("<br>");

    strHTML += _("Alarm: ");
    if ( registers[0x80] ) {
        strHTML += _("Yes");
    }
    else {
        strHTML += _("No");
    }
    strHTML += _("<br>");


    strHTML += _("Node Control Flags: ");
    if ( registers[0x83] & 0x10 ) {
        strHTML += _("[Register Write Protect] ");
    }
    else {
        strHTML += _("[Register Read/Write] ");
    }
    switch ( (registers[0x83] & 0xC0) >> 6 ) {
    case 1:
        strHTML += _(" [Initialized] ");
        break;
    default:
        strHTML += _(" [Uninitialized] ");
        break;
    }
    strHTML += _("<br>");

    strHTML += _("Firmware VSCP confirmance: ");
    strHTML += wxString::Format(_("%d.%d"), registers[0x81], registers[0x82] );
    strHTML += _("<br>");

    strHTML += _("User ID: ");
    strHTML += wxString::Format(_("%d.%d.%d.%d.%d"), 
        registers[0x84], 
        registers[0x85],
        registers[0x86],
        registers[0x87],
        registers[0x88] );
    strHTML += _("<br>");

    strHTML += _("Manufacturer device ID: ");
    strHTML += wxString::Format(_("%d.%d.%d.%d"), 
        registers[0x89], 
        registers[0x8A],
        registers[0x8B],
        registers[0x8C] );
    strHTML += _("<br>");

    strHTML += _("Manufacturer sub device ID: ");
    strHTML += wxString::Format(_("%d.%d.%d.%d"), 
        registers[0x8d], 
        registers[0x8e],
        registers[0x8f],
        registers[0x90] );
    strHTML += _("<br>");

    strHTML += _("Page select: ");
    strHTML += wxString::Format(_("%d (MSB=%d LSB=%d)"), 
        registers[0x92] * 256 + registers[0x93], 
        registers[0x92],
        registers[0x93] );
    strHTML += _("<br>");

    strHTML += _("Firmware version: ");
    strHTML += wxString::Format(_("%d.%d.%d"), 
        registers[0x94], 
        registers[0x95],
        registers[0x96] );
    strHTML += _("<br>");

    strHTML += _("Boot loader algorithm: ");
    strHTML += wxString::Format(_("%d - "), 
        registers[0x97] );
    switch( registers[0x97] ) {

    case 0x00:
        strHTML += _("VSCP universal algorithm 0");
        break;

    case 0x01:
        strHTML += _("Microchip PIC algorithm 0");
        break;

    case 0x10:
        strHTML += _("Atmel AVR algorithm 0");
        break;

    case 0x20:
        strHTML += _("NXP ARM algorithm 0");
        break;

    case 0x30:
        strHTML += _("ST ARM algorithm 0");
        break;

    default:
        strHTML += _("Unknown algorithm.");
        break;
    }

    strHTML += _("<br>");

    strHTML += _("Buffer size: ");
    strHTML += wxString::Format(_("%d bytes. "), 
        registers[0x98] );
    if ( !registers[0x98] ) strHTML += _(" ( == default size (8 or 487 bytes) )");
    strHTML += _("<br>");

    strHTML += _("Number of register pages: ");
    strHTML += wxString::Format(_("%d"), 
        registers[0x99] );
    if ( registers[0x99] > 22 ) {
        strHTML += _(" (Note: VSCP Works display max 22 pages.) ");
    }
    strHTML += _("<br>");

    if ( USE_DLL_INTERFACE == pcsw->getDeviceType() ) {

        unsigned char data[8];
        memset( data, 0, 8 );
        if ( getLevel1DmInfo( pcsw, registers[0x91], data ) ) {
            strHTML += _("Decison Matrix: Rows=");
            strHTML += wxString::Format(_("%d "), data[0] );
            strHTML += _(" Offset=");
            strHTML += wxString::Format(_("%d "), data[1] );
            strHTML += _(" Page start=");
            strHTML += wxString::Format(_("%d "), data[2] * 256 + data[3] );
            strHTML += _(" Page end=");
            strHTML += wxString::Format(_("%d "), data[4] * 256 + data[5] );
            strHTML += _("<br>");
        }
        else {
            strHTML += _("No Decision Matrix is available on this device.");
            strHTML += _("<br>");
        }

    }
    else if ( USE_TCPIP_INTERFACE == pcsw->getDeviceType() ) {

        unsigned char data[8];
        memset( data, 0, 8 );
        // Get the interface GUID
        unsigned char interfaceGUID[16];
        //getGuidFromStringToArray( interfaceGUID, m_comboNodeID->GetValue() );
        if ( getLevel2DmInfo( pcsw, interfaceGUID, data ) ) {
            strHTML += _("Decison Matrix: Rows=");
            strHTML += wxString::Format(_("%d "), data[0] );
            strHTML += _(" Offset=");
            strHTML += wxString::Format(_("%d "), data[1] );
            strHTML += _(" Page start=");
            strHTML += wxString::Format(_("%d "), data[2] * 256 + data[3] );
            strHTML += _(" Page end=");
            strHTML += wxString::Format(_("%d "), data[4] * 256 + data[5] );
            strHTML += _("<br>");
        }
        else {
            strHTML += _("No Decision Matrix is available on this device.");
            strHTML += _("<br>");
        }

    }

    strHTML += _("</font>");
    strHTML += _("</body></html>");

    return strHTML;
}
*/
//////////////////////////////////////////////////////////////////////////////
// loadMDF
//

bool VscpworksApp::loadMDF( wxWindow *pwnd,
                                CCanalSuperWrapper *pcsw, 
                                CMDF *pmdf, 
                                wxString& url, 
                                uint8_t *pid )
{
    bool rv = true;
    //wxStandardPaths stdpaths;
    wxString remoteFile;
    uint8_t mdf_url[33];

    // Check pointers
    if ( NULL == pcsw ) return false;
    if ( NULL == pmdf ) return false;
    if ( NULL == pid ) return false;

    // If length of url is zero registers should be read
    // from device to get mdf url
    if ( 0 == url.Length() ) {

        memset( mdf_url, 0, sizeof( mdf_url ) );

        for ( int i=0; i<32; i++ ) {

            if ( USE_DLL_INTERFACE == pcsw->getDeviceType() ) {
                uint8_t reg;
                if ( !pcsw->getDllInterface()->readLevel1Register( 0,
                                                                    *pid,
                                                                    0xe0 + i, 
                                                                    &reg ) ) return false;
                mdf_url[ i ] = reg;
            }
            else {
                if ( !readLevel2Register( pcsw,
                                            pid, 
                                            0xe0 + i, 
                                            ( mdf_url + i ) ) ) 
                {
                    return false;
                }
            }

        } // for

        remoteFile = _("http://") + wxString::From8BitData( (const char *)mdf_url );
        //wxString remoteFile = _("http://www.grodansparadis.com/smart2_001.mdf");

    }
    else {
        remoteFile = url;
    }

    wxString localFile;

    wxProgressDialog progressDlg( _("VSCP Works"),
                                    _("Load and parse MDF"),
                                    100, 
                                    pwnd,
                                    wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL );

    wxDateTime now = wxDateTime::Now();

    if ( wxNOT_FOUND == remoteFile.Find( _("://") ) ) {

        localFile = remoteFile;

        // Load MDF from local file
        wxFileDialog dlg( pwnd,
                            _("Choose file to load MDF from "),
                            wxStandardPaths::Get().GetUserDataDir(),
                            _(""),
                            _("MSF Files (*.mdf)|*.mdf|XML Files (*.xml)|*.xml|All files (*.*)|*.*") );
        if ( wxID_OK == dlg.ShowModal() ) {
            localFile = dlg.GetPath();
        }
        else {
            return false;
        }
    }
    else {
        // Download the MDF
        progressDlg.Update( 30, _("Download MDF.") );

        if ( !pmdf->downLoadMDF( remoteFile, localFile ) ) {
            wxMessageBox( _("Failed to download MDF.") );
            progressDlg.Update( 100 );
            return false;
        }
    }

    progressDlg.Update( 60, _("Parsing MDF.") );

    pmdf->clearStorage();

    if ( !pmdf->parseMDF( localFile ) ) {
        wxMessageBox( _("Failed to parse MDF.") );
        progressDlg.Update( 100 );
        return false;
    }

    return rv;
}




//////////////////////////////////////////////////////////////////////////////
// readAllLevel1Registers
//
/*
bool VscpworksApp::readAllLevel1Registers( wxWindow *pwnd,
                                            CCanalSuperWrapper *pcsw,
                                            uint8_t *pregisters,
                                            unsigned char nodeid  )
{
    int i;
    unsigned char val;
    bool rv = true;
    int errors = 0;
    wxString strBuf;

    wxProgressDialog progressDlg( _("VSCP Works"),
                                    _("Reading Registers"),
                                    256, 
                                    pwnd,
                                    wxPD_ELAPSED_TIME | 
                                        wxPD_AUTO_HIDE | 
                                        wxPD_APP_MODAL | 
                                        wxPD_CAN_ABORT );

    progressDlg.Pulse( _("Reading registers!") );


    // *********************
    // Read register content
    // *********************
    for ( i = 0; i < 256; i++ ) {

        if ( !progressDlg.Update( i ) ) {
            rv = false;
            break;   // User aborted
        }
    
        progressDlg.Pulse( wxString::Format(_("Reading register %d"), i) );
    
        if ( pcsw->readLevel1Register( nodeid, i, &val ) ) {
    
            pregisters[ i ] = val;

        }
        else {
            errors++;
        }

        if ( errors > 2 ) {
            wxMessageBox( _("No node present or problems when communicating with node. Aborting!") );
            rv = false;
            break;
        }
            
    } // for

    return rv;
}
*/
/*
//////////////////////////////////////////////////////////////////////////////
// readAllLevel2Registers
//

bool VscpworksApp::readAllLevel2Registers( wxWindow *pwnd,
                                            CCanalSuperWrapper *pcsw,
                                            uint8_t *pregisters,
                                            uint8_t *pinterfaceGUID )
{
    int i;
    unsigned char val;
    bool rv = true;
    int errors = 0;
    wxString strBuf;


    wxProgressDialog progressDlg( _("VSCP Works"),
                                    _("Reading Registers"),
                                    256, 
                                    pwnd,
                                    wxPD_ELAPSED_TIME | 
                                    wxPD_AUTO_HIDE | 
                                    wxPD_APP_MODAL |
                                    wxPD_CAN_ABORT );

    progressDlg.Pulse( _("Reading registers!") );



    // *********************
    // Read register content
    // *********************
    for ( i = 0; i < 256; i++ ) {

        if ( !progressDlg.Update( i ) ) {
            rv = false;
            break;
        }

        progressDlg.Pulse( wxString::Format(_("Reading register %d"), i) );
    
        if ( wxGetApp().readLevel2Register( pcsw, pinterfaceGUID, i, &val ) ) {
      
            pregisters[ i ] = val;
      
        }
        else {
            errors++;
        }


        if ( errors > 2 ) {
            wxMessageBox( _("No node present or problems when communicating with node. Aborting!") );
            rv = false;
            break;
        }
            
    } // for


    return rv;
}
*/
//////////////////////////////////////////////////////////////////////////////
// addMDFInfo
//
/*
wxString VscpworksApp::addMDFInfo( CMDF *pmdf )
{
    wxString strHTML;

    // MDF Info
    strHTML = _("<h1>MDF Information</h1>");

    strHTML += _("<font color=\"#009900\">");

    // Manufacturer data
    strHTML += _("<b>Module name :</b> ");
    strHTML += pmdf->m_strModule_Name;
    strHTML += _("<br>");

    strHTML += _("<b>Module model:</b> ");
    strHTML += pmdf->m_strModule_Model;
    strHTML += _("<br>");

    strHTML += _("<b>Module version:</b> ");
    strHTML += pmdf->m_strModule_Version;
    strHTML += _("<br>");

    strHTML += _("<b>Module last change:</b> ");
    strHTML += pmdf->m_changeDate;
    strHTML += _("<br>");

    strHTML += _("<b>Module description:</b> ");
    strHTML += pmdf->m_strModule_Description;
    strHTML += _("<br>");

    strHTML += _("<b>Module URL</b> : ");
    strHTML += _("<a href=\"");
    strHTML += pmdf->m_strModule_InfoURL;
    strHTML += _("\">");
    strHTML += pmdf->m_strModule_InfoURL;
    strHTML += _("</a>");
    strHTML += _("<br>");
    

    MDF_MANUFACTURER_LIST::iterator iter;
    for ( iter = pmdf->m_list_manufacturer.begin(); 
            iter != pmdf->m_list_manufacturer.end(); ++iter ) {

        strHTML += _("<hr><br>");
        
        CMDF_Manufacturer *manufacturer = *iter;
        strHTML += _("<b>Manufacturer:</b> ");
        strHTML += manufacturer->m_strName;
        strHTML += _("<br>");

        MDF_ADDRESS_LIST::iterator iterAddr;
        for ( iterAddr = manufacturer->m_list_Address.begin(); 
                iterAddr != manufacturer->m_list_Address.end(); ++iterAddr ) {

            CMDF_Address *address = *iterAddr;
            strHTML += _("<h4>Address</h4>");
            strHTML += _("<b>Street:</b> ");
            strHTML += address->m_strStreet;
            strHTML += _("<br>");
            strHTML += _("<b>Town:</b> ");
            strHTML += address->m_strTown;
            strHTML += _("<br>");
            strHTML += _("<b>City:</b> ");
            strHTML += address->m_strCity;
            strHTML += _("<br>");
            strHTML += _("<b>Post Code:</b> ");
            strHTML += address->m_strPostCode;
            strHTML += _("<br>");
            strHTML += _("<b>State:</b> ");
            strHTML += address->m_strState;
            strHTML += _("<br>");
            strHTML += _("<b>Region:</b> ");
            strHTML += address->m_strRegion;
            strHTML += _("<br>");
            strHTML += _("<b>Country:</b> ");
            strHTML += address->m_strCountry;
            strHTML += _("<br><br>");
       }

        MDF_ITEM_LIST::iterator iterPhone;
        for ( iterPhone = manufacturer->m_list_Phone.begin(); 
                iterPhone != manufacturer->m_list_Phone.end(); ++iterPhone ) {

            CMDF_Item *phone = *iterPhone;
            strHTML += _("<b>Phone:</b> ");
            strHTML += phone->m_strItem;
            strHTML += _(" ");
            strHTML += phone->m_strDescription;
            strHTML += _("<br>");
        }

        MDF_ITEM_LIST::iterator iterFax;
        for ( iterFax = manufacturer->m_list_Fax.begin(); 
                iterFax != manufacturer->m_list_Fax.end(); ++iterFax ) {

            CMDF_Item *fax = *iterFax;
            strHTML += _("<b>Fax:</b> ");
            strHTML += fax->m_strItem;
            strHTML += _(" ");
            strHTML += fax->m_strDescription;
            strHTML += _("<br>");
        }

        MDF_ITEM_LIST::iterator iterEmail;
        for ( iterEmail = manufacturer->m_list_Email.begin(); 
                iterEmail != manufacturer->m_list_Email.end(); ++iterEmail ) {

            CMDF_Item *email = *iterEmail;
            strHTML += _("<b>Email:</b> <a href=\"");
            strHTML += email->m_strItem;
            strHTML += _("\" >");
            strHTML += email->m_strItem;
            strHTML += _("</a> ");
            strHTML += email->m_strDescription;
            strHTML += _("<br>");
        }

        MDF_ITEM_LIST::iterator iterWeb;
        for ( iterWeb = manufacturer->m_list_Web.begin(); 
                iterWeb != manufacturer->m_list_Web.end(); ++iterWeb ) {

            CMDF_Item *web = *iterWeb;
            strHTML += _("<b>Web:</b> <a href=\"");
            strHTML += web->m_strItem;
            strHTML += _("\">");
            strHTML += web->m_strItem;
            strHTML += _("</a> ");
            strHTML += web->m_strDescription;
            strHTML += _("<br>");
        }

    } // manufacturer

    return strHTML;
}

*/




