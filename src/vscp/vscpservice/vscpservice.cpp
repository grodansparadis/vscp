// vscpservice (C) 2001-2011 Grodans Paradis AB
// changes and adoptions Ake Hedman, akhe@grodansparadis.com
// 
// From Microsoft example built by Dave McPherson (davemm)   11-March-98
// and MSDN code.
//
#ifdef __GNUG__
    //#pragma implementation
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include <wx/ownerdrw.h>
	#include <wx/stdpaths.h>
#endif
 

#include "../common/controlobject.h"
#include "../common/version.h"


#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include "wx/cmdline.h"

#include "VSCPServApp.h"
#include "vscpservice.h"
#include "../common/controlobject.h"

// External variables
extern HANDLE  ghStopEvent;

// Global variables
static bool gbRun = true;	// Thread are running as long as value is true...


///////////////////////////////////////////////////////////////////////////////
// CVSCPService
//

CVSCPService::CVSCPService() : CNTService(_("vscpservice"))
{
	m_iStartParam = 0;
	m_iState = m_iStartParam;
}

///////////////////////////////////////////////////////////////////////////////
//	OnInit
//

BOOL CVSCPService::OnInit()
{
	bool rv = true;

	if ( !::wxInitialize() ) {
        return false;      
    }

	//wxStandardPaths stdPath;
    wxString strCfgFile;
   
    strCfgFile = wxStandardPaths::Get().GetConfigDir();
    strCfgFile += _("/vscp/vscpd.conf");

	rv = m_ctrlObj.init( strCfgFile );
	 
	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Run
//
// 
//

void CVSCPService::Run()
{	
	m_bIsRunning = true;

	while ( m_bIsRunning ) {
		if ( m_ctrlObj.run() ) {
			m_bIsRunning = false;
		}
	}

	m_ctrlObj.m_bQuit = true;
	Sleep( 3000 );

	gbRun = false;	// End all threads

}	

///////////////////////////////////////////////////////////////////////////////
//	OnUserControl
//

BOOL CVSCPService::OnUserControl( DWORD dwOpcode )
{
    switch ( dwOpcode ) {

    case SERVICE_CONTROL_USER + 0:

        // Save the current status in the registry
        SaveStatus();
        return TRUE;

    default:
        break;

    }

    return FALSE; // say not handled
}


///////////////////////////////////////////////////////////////////////////////
// OnStop

void CVSCPService::OnStop( void )
{
	m_ctrlObj.m_bQuit = true;
	Sleep( 3000 );

	// Stop all threads
	m_bIsRunning = false;
}


///////////////////////////////////////////////////////////////////////////////
// OnInterrogate

void CVSCPService::OnInterrogate( void )
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// OnPause

void CVSCPService::OnPause( void )
{
	// Stop all threads
	m_bIsRunning = false;
}

///////////////////////////////////////////////////////////////////////////////
// OnContinue

void CVSCPService::OnContinue( void )
{

}

///////////////////////////////////////////////////////////////////////////////
// OnShutdown

void CVSCPService::OnShutdown( void )
{
	// Stop all threads
	m_bIsRunning = false;
}


///////////////////////////////////////////////////////////////////////////////
// SaveStatus
//

void CVSCPService::SaveStatus()
{
    DebugMsg(_("Saving current status"));
    // Try opening the registry key:
    // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\<AppName>\...
    HKEY hkey = NULL;
	WCHAR szKey[1024];
	wcscpy(szKey, _("SYSTEM\\CurrentControlSet\\Services\\"));
	wcscat(szKey, m_szServiceName);
	wcscat(szKey, _("\\Status"));
    DWORD dwDisp;
	DWORD dwErr;
    DebugMsg(_("Creating key: %s"), szKey);
    dwErr = RegCreateKeyEx(	HKEY_LOCAL_MACHINE,
                           	szKey,
                   			0,
                   			NULL,
                   			REG_OPTION_NON_VOLATILE,
                   			KEY_WRITE,
                   			NULL,
                   			&hkey,
                   			&dwDisp);
	
	if (dwErr != ERROR_SUCCESS) 
	{
		DebugMsg(_("Failed to create Status key (%lu)"), dwErr);
		return;
	}	

    // Set the registry values
	DebugMsg(_("Saving 'Current' as %ld"), m_iState); 
    RegSetValueEx(hkey,
                  _("Current"),
                  0,
                  REG_DWORD,
                  (BYTE*)&m_iState,
                  sizeof(m_iState));


    // Finished with key
    RegCloseKey(hkey);

}


///////////////////////////////////////////////////////////////////////////////
//							Global routines
///////////////////////////////////////////////////////////////////////////////


