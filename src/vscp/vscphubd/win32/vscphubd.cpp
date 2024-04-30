// vscpd.cpp : Defines the class behaviors for the application.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (https://www.vscp.org) 
//
// Copyright (C) 2000-2017 Ake Hedman, the VSCP project, <info@vscp.org>
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

#ifdef WIN32
#include <pch.h>
#endif

#include <iostream>
#include <string>

#define XSTR(x) STR(x)
//#define STR(x) #x

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>    
#endif

#include <signal.h>
 
#include <canal_macro.h>
#include <controlobject.h>
#include <version.h>

// The global control object
CControlObject *gpobj;

// static const wxCmdLineEntryDesc cmdLineDesc[] = { 
//   { 
//     wxCMD_LINE_OPTION, 
//     _("c"), 
//     _("configpath"), 
//     _("Path to configuration file"), 
//     wxCMD_LINE_VAL_STRING, 
//     wxCMD_LINE_PARAM_OPTIONAL 
//   },
//  { 
//     wxCMD_LINE_OPTION, 
//     _("d"), 
//     _("debuglevel"), 
//     _("Debug Level."), 
//     wxCMD_LINE_VAL_NUMBER, 
//     wxCMD_LINE_PARAM_OPTIONAL 
//   },
//   { wxCMD_LINE_SWITCH, _("h"), _("help"), _("Shows this message"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_OPTION_HELP  },
//   { wxCMD_LINE_SWITCH, _("v"), _("verbose"), _("Vebose mode"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
//   { wxCMD_LINE_SWITCH, _("w"), _("hide"), _("Hide debug window"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
//   { wxCMD_LINE_SWITCH, _("g"), _("gnu"), _("Copyleft"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
//   { wxCMD_LINE_NONE } 
// };


void doabort(int sig) {
 printf("Wake up call ... !!! - Catched signal: %d ... !!\n", sig);
 (void) signal(SIGINT, SIG_DFL);
}

int main(int argc, char **argv)
{
    (void) signal( SIGABRT, doabort ); 
    (void) signal( SIGTERM, doabort );  
    (void) signal( SIGINT, doabort );

#if defined( _MSC_VER ) && _MSC_VER < 1600
#pragma message ( "Visual studio version = "  STR(_MSC_VER) )
#endif

    // wxChar **wxArgv = new wxChar *[argc + 1];

    // {
    //     int n;

    //     for (n = 0; n < argc; n++ )
    //     {
    //         wxMB2WXbuf warg = wxConvertMB2WX( argv[n] );
    //         wxArgv[ n ] = wxStrdup( warg );
    //     }

    //     wxArgv[n] = NULL;
    // }
        

        // /* Clean up */
        // {
        //     for ( int n = 0; n < argc; n++ ) {
        //         free(wxArgv[n]);
        //     }

        //     delete [] wxArgv;
        // }

        
    //     return -1;      
    // }

    bool bHideWindow = false;
    std::string strCfgFile;
    std::string rootFolder;
    CControlObject ctrlobj;

    rootFolder = ""; // wxStandardPaths::Get().GetUserDataDir();
    strCfgFile = ""; // wxStandardPaths::Get().GetConfigDir();

#ifdef WIN32
    // The following needed because wx 3.1 add "vscpd" to standardpath
    size_t pos;
    if (std::string::npos != (pos = strCfgFile.find("vscpd"))) {
        strCfgFile = strCfgFile.substr( strCfgFile.length() - 5 );
        strCfgFile += "vscp";
    }
    else if (std::string::npos != (pos = strCfgFile.find("vscpd32"))) {
        strCfgFile = strCfgFile.substr( strCfgFile.length() - 7 );
        strCfgFile += "vscp";
    }
    strCfgFile += "\\vscpd.json";
#else
    strCfgFile += "/vscp/vscpd.json";
#endif

    // wxCmdLineParser *pparser = new wxCmdLineParser( cmdLineDesc, argc, argv );
    
    // if ( NULL != pparser ) {
		
	// 	std::string wxstr;
	// 	if ( pparser->Parse(false) > 0 ) {
	// 		printf("\n\nUsage for vscpd.exe\n");
	// 		printf("-------------------------\n");
	// 		char idx = 0;
	// 		while( cmdLineDesc[idx].kind != wxCMD_LINE_NONE ) {
	// 			wxPrintf( _("%s or %s \t %s \n"),
	// 						cmdLineDesc[idx].shortName, 
	// 						cmdLineDesc[idx].longName,
	// 						cmdLineDesc[idx].description );
	// 			idx++;
	// 		}
	// 	}
		

    //     // * * * Verbose * * *
    //     if ( pparser->Found( wxT("verbose") ) ) {
    //         std::string wxstr;
    //         wxstr.Printf(wxT("Verbose mode set\n"));
    //     }

    //     // * * * Hide debug window * * *
    //     if ( pparser->Found( wxT("hide") ) ) {
    //         bHideWindow = true;
    //         {
    //             std::string wxstr;
    //             wxstr.Printf(wxT("Debug window hidden\n"));
    //         }
    //     }

    //     // * * * Copyleft * * *
    //     if ( pparser->Found( wxT("gnu") ) ) {
    //         std::string wxstr;
    //         wxstr.Printf(wxT("Verbose mode set\n"));
    //     }

    //     // * * * Path to configuration file * * *
    //     if ( pparser->Found( _T("configpath"), &wxstr ) ) {
    //         strCfgFile = wxstr;
    //     }

    //     delete pparser;
    
    // }

	if ( bHideWindow ) {
    
		TCHAR savetitle[1024];
        
        // Save the title
        GetConsoleTitle( savetitle, 1024 );
        
        // Set dummy title
        SetConsoleTitle("_____VSCP Daemon console window____" );
        
        // Find the window
        HWND hwnd = FindWindow( NULL, "_____VSCP Daemon console window____");
         
        if ( NULL != hwnd ) {
          ShowWindow( hwnd, SW_HIDE );
        }

        // Restore old title
        SetConsoleTitle( savetitle );
    }

    if ( !ctrlobj.init( strCfgFile, rootFolder ) ) {
		// ctrlobj.logMsg("Unable to initialize the vscpd application.", 
        //                   DAEMON_LOGMSG_NORMAL );
                          
#if wxUSE_UNICODE
        /* Clean up */
        {
            for ( int n = 0; n < argc; n++ ) {
                free(wxArgv[n]);
            }

            delete [] wxArgv;
        }
#endif // wxUSE_UNICODE

		//::wxUninitialize();
		return FALSE;
	}

	if ( !ctrlobj.run() ) {
		// ctrlobj.logMsg("Unable to start the vscpd application.", 
        //                 DAEMON_LOGMSG_NORMAL );
	}


    if ( !ctrlobj.cleanup() ) {
        // ctrlobj.logMsg( _("Unable to clean up the vscpd application."), 
        //                 DAEMON_LOGMSG_NORMAL );
    }

#if wxUSE_UNICODE
    {
        for ( int n = 0; n < argc; n++ ) {
            free(wxArgv[n]);
		}

        delete [] wxArgv;
    }
#endif // wxUSE_UNICODE

    //::wxUninitialize();
    return TRUE;
   
}





 



