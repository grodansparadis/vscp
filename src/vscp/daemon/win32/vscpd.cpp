// vscpd.cpp : Defines the class behaviors for the application.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (http://www.vscp.org) 
//
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#include <signal.h>

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include "wx/cmdline.h"
#include "wx/tokenzr.h"
#include <wx/stdpaths.h>
 
#include "../../common/canal_macro.h"
#include "../../common/controlobject.h"
#include "../../common/version.h"

static const wxCmdLineEntryDesc cmdLineDesc[] = { 
  { 
    wxCMD_LINE_OPTION, 
    _T("c"), 
    _T("configpath"), 
    _T("Path to configuration file"), 
    wxCMD_LINE_VAL_STRING, 
    wxCMD_LINE_PARAM_OPTIONAL 
  },
 { 
    wxCMD_LINE_OPTION, 
    _T("d"), 
    _T("debuglevel"), 
    _T("Debug Level."), 
    wxCMD_LINE_VAL_NUMBER, 
    wxCMD_LINE_PARAM_OPTIONAL 
  },
  { wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("Shows this message"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_OPTION_HELP  },
  { wxCMD_LINE_SWITCH, _T("v"), _T("verbose"), _T("Vebose mode"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
  { wxCMD_LINE_SWITCH, _T("w"), _T("hide"), _T("Hide debug window"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
  { wxCMD_LINE_SWITCH, _T("g"), _T("gnu"), _T("Copyleft"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
  { wxCMD_LINE_NONE } 
};


void doabort(int sig) {
 printf("Wake up call ... !!! - Catched signal: %d ... !!\n", sig);
 (void) signal(SIGINT, SIG_DFL);
}

int main(int argc, char **argv)
{
    (void) signal( SIGABRT, doabort ); 
    (void) signal( SIGTERM, doabort );  
    (void) signal( SIGINT, doabort );

#if wxUSE_UNICODE
    wxChar **wxArgv = new wxChar *[argc + 1];

    {
        int n;

        for (n = 0; n < argc; n++ )
        {
            wxMB2WXbuf warg = wxConvertMB2WX( argv[n] );
            wxArgv[ n ] = wxStrdup( warg );
        }

        wxArgv[n] = NULL;
    }
#else // !wxUSE_UNICODE
    #define wxArgv argv
#endif // wxUSE_UNICODE/!wxUSE_UNICODE

    if ( !::wxInitialize() ) {
        fprintf(stderr, "Failed to initialize the wxWindows library, aborting.");
        return -1;      
    }

    int arg = 0;
    unsigned char nDebugLevel = 0;
    bool bVerbose = false;
    bool bCopyLeft = false;
    bool bHideWindow = false;
    wxStandardPaths stdPath;
    wxString strCfgFile;
    CControlObject ctrlobj;

   
    strCfgFile = stdPath.GetConfigDir();
    strCfgFile += _("/vscp/vscpd.conf");

    wxCmdLineParser *pparser = new wxCmdLineParser( cmdLineDesc, argc, argv );
    
    if ( NULL != pparser ) {
		
		wxString wxstr;
	
		if ( pparser->Parse(false) > 0 ) {
			printf("\n\nUsage for vscpd.exe\n");
			printf("-------------------------\n");
			char idx = 0;
			while( cmdLineDesc[idx].kind != wxCMD_LINE_NONE ) {
				wxPrintf( _("%s or %s \t %s \n"),
							cmdLineDesc[idx].shortName, 
							cmdLineDesc[idx].longName,
							cmdLineDesc[idx].description );
				idx++;
			}
		}
		

        // * * * Verbose * * *
        if ( pparser->Found( wxT("verbose") ) ) {
            bVerbose = true;
            {
	            wxString wxstr;
	            wxstr.Printf(wxT("Verbose mode set\n"));
            }
        }

        // * * * Hide debug window * * *
        if ( pparser->Found( wxT("hide") ) ) {
            bHideWindow = true;
            {
                wxString wxstr;
                wxstr.Printf(wxT("Debug window hidden\n"));
            }
        }

        // * * * Copyleft * * *
        if ( pparser->Found( wxT("gnu") ) ) {
            bCopyLeft = true;
            {
                wxString wxstr;
                wxstr.Printf(wxT("Verbose mode set\n"));
            }
        }

        // * * * Path to configuration file * * *
        if ( pparser->Found( _T("configpath"), &wxstr ) ) {
            strCfgFile = wxstr;
        }

        delete pparser;
    
    }

	if ( bHideWindow ) {
    
		TCHAR savetitle[1024];
        
        // Save the title
        GetConsoleTitle( savetitle, 1024 );
        
        // Set dummy title
        SetConsoleTitle( _T("_____VSCP Daemon console window____") );
        
        // Find the window
        HWND hwnd = FindWindow( NULL, _T("_____VSCP Daemon console window____") );
         
        if ( NULL != hwnd ) {
          ShowWindow( hwnd, SW_HIDE );
        }

        // Restore old title
        SetConsoleTitle( savetitle );
    }


    if ( !ctrlobj.init( strCfgFile ) ) {
		ctrlobj.logMsg( _("Unable to initialize the vscpd application."), 
                          DAEMON_LOGMSG_CRITICAL );
		::wxUninitialize();
		return FALSE;
	}

	if ( !ctrlobj.run() ) {
		ctrlobj.logMsg( _("Unable to start the vscpd application."), 
                          DAEMON_LOGMSG_CRITICAL );
	}


    if ( !ctrlobj.cleanup() ) {
        ctrlobj.logMsg( _("Unable to clean up the vscpd application."), 
                        DAEMON_LOGMSG_CRITICAL );
    }

#if wxUSE_UNICODE
    {
        for ( int n = 0; n < argc; n++ ) {
            free(wxArgv[n]);
		}

        delete [] wxArgv;
    }
#endif // wxUSE_UNICODE

    ::wxUninitialize();
    return TRUE;
   
}





 



