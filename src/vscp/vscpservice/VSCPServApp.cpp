// NTService.cpp
// 
// This is the main program file containing the entry point.

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
    #include  "wx/ownerdrw.h"
#endif

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/init.h"

#include "VSCPServApp.h"
#include "vscpservice.h"

// the service object
CVSCPService gVSCPService;

int main(int argc, char* argv[])
{   
	if ( !::wxInitialize() ) {
	    fprintf(stderr, "Failed to initialize the wxWindows library, aborting.");
		return -1;      
    }

	// Parse for standard arguments (install, uninstall, version etc.)
	if ( !gVSCPService.ParseStandardArgs( argc, argv ) ) {
		// Didn't find any standard args so start the service
		// Uncomment the DebugBreak line below to enter the debugger
		// when the service is started.
		//DebugBreak();
		//gVSCPService.DebugMsg( "Start Service" );
		gVSCPService.StartService();
	
	}

	::wxUninitialize();

  // When we get here, the service has been stopped
  return gVSCPService.m_Status.dwWin32ExitCode;
}
