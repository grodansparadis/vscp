///////////////////////////////////////////////////////////////////////////////
// mkpasswd.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB,<akhe@grodansparadis.com>
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


#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/stdpaths.h>

#include "wx/cmdline.h"
#include "wx/tokenzr.h"
#include "wx/config.h"
#include "wx/wfstream.h"
#include "wx/fileconf.h"
#include <math.h>

#ifdef WIN32
 
#else

#include <unistd.h>
#include <sys/types.h>

#endif

#include "../common/canal_macro.h"
#include "../../common/md5.h"
#include "mkpasswd.h"

// Prototypes
unsigned long getDataValue( const char *szData );

static const wxCmdLineEntryDesc cmdLineDesc[] = { 
  { 
	wxCMD_LINE_OPTION, 
    _("p"), 
    _("password"), 
    _("Password to convert."), 
    wxCMD_LINE_VAL_STRING, 
    wxCMD_LINE_OPTION_MANDATORY
  },
  { 
    wxCMD_LINE_OPTION, 
    _("d"), 
    _("dest"), 
    _("Path (desitination for) to config file (if not given OS default will be used)."), 
    wxCMD_LINE_VAL_STRING, 
    wxCMD_LINE_PARAM_OPTIONAL 
  },
  /*
  { 
    wxCMD_LINE_OPTION, 
    _("u"), 
    _("user"), 
    _("Username to write to config file."), 
    wxCMD_LINE_VAL_STRING, 
    wxCMD_LINE_PARAM_OPTIONAL 
  },

	{ 
	  wxCMD_LINE_SWITCH, 
		_("h"), 
		_("help"), 
		_("Shows this message"), 
		wxCMD_LINE_VAL_NONE, 
		wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_OPTION_HELP  
	},
*/
	{ 
		wxCMD_LINE_SWITCH, 
		_("v"), 
		_("verbose"), 
		_("Vebose mode"), 
		wxCMD_LINE_VAL_NONE, 
		wxCMD_LINE_PARAM_OPTIONAL 
	},
	{ 
		wxCMD_LINE_NONE 
	} 
};


#define WIN32_USE_FILE


int main(int argc, char **argv)
{

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

	wxApp::CheckBuildOptions( WX_BUILD_OPTIONS_SIGNATURE, "program" );

	if ( !::wxInitialize() ) {
		fprintf(stderr, "Failed to initialize the wxWindows library, aborting.");
		return -1;      
 	}

	//wxCmdLineParser *pparser = new wxCmdLineParser( argc, wxArgv );

	bool bWrite = false;
	bool bVerbose = false;
	
	bool bUsername = false;
	bool bPassword = false;
	bool bPath = false;

	wxString wxstrUsername;
	wxString wxstrPassword;
	wxString wxstrPath;
	Cmd5 md5;
	
	//appname = wxFileName(wxTheApp->argv[0]).GetFullName();
	//wxString ttt = wxTheApp->argv[0];

	wxCmdLineParser parser(cmdLineDesc, argc, wxArgv);
  	
	wxString wxstr;
	if ( parser.Parse(false) > 0 ) {
		printf("\n\nUsage for mkpassws.exe\n");
		printf("----------------------\n");
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
	if ( parser.Found( _("verbose") ) ) {
		bVerbose = true;	
		printf("Verbose mode set\n");
	}

		// * * * Write to config file * * *
		//if ( parser.Found( _("write") ) ) {
		//	bWrite = true;	
		//}

		// * * * Password * * * 
	if ( parser.Found( _("password"), &wxstrPassword ) ) {
		bPassword = true;
		if ( bVerbose ) {
			printf(" Password = '%s'\n", (const char *)wxstrPassword.mb_str(wxConvUTF8) );	
		}
	}

		// * * * Username * * * 
		//if ( parser.Found( _("user"), &wxstrUsername ) ) {
		//	bUsername = true;
		//	if ( bVerbose ) {
		//		printf(" user = '%s'\n", (const char *)wxstrUsername.mb_str(wxConvUTF8 ) );	
		//	}
		//}

		// * * * Path * * * 
	if ( !parser.Found( _("dest"), &wxstrPath ) ) {
		bPath = true;			
	}
#ifdef WIN32
	//GetSystemDirectory( wxstrPath.GetWriteBuf( MAX_PATH ), MAX_PATH );
	//wxstrPath.UngetWriteBuf()
	wxstrPath += _("c:\\programdata\\vscp\\canalworks.conf");
#else
	wxstrPath = _("/etc/vscp/vscpworks.conf");
#endif

	if ( bVerbose ) {
			//printf(" path = '%s'\n", (const char *)wxstrPath.mb_str(wxConvUTF8 ) );	
	}
		
	unsigned char buf[2148];
	strncpy( (char *)buf, wxstrPassword.mb_str(), sizeof(buf) );

	if (  wxstrPassword.Length() ) {
		md5.digest( buf );

		if ( bVerbose ) {
			printf(" Encrypted password = '%s'\n", md5.getDigest() );	
		}
	}

	// * * * Write to Config file * * *

	if ( bWrite ) {
	
//#ifdef WIN32
//#ifdef WIN32_USE_FILE
		
		if ( !wxFile::Exists( wxstrPath ) ) {
			wxFile f;
			f.Create( wxstrPath );
			f.Close();
		}

		wxFFileInputStream cfgstream( wxstrPath );

		if ( !cfgstream.Ok() ) {
			printf("Unable to open config file1!\n");
			::wxUninitialize();
			return -1;
		}

		wxFileConfig *pconfig = new wxFileConfig( cfgstream );

//#else
//		wxConfig *pconfig = new wxConfig("canald", "VSCP" );
//#endif
//#else
//        wxConfig *pconfig = new wxConfig("canald", 
//					 "VSCP", 
//					 "localcanalworks.conf", 
//					 "canalworks.conf", 
//					 wxCONFIG_USE_GLOBAL_FILE );
//#endif 

		pconfig->SetPath(_("/DAEMON"));

		// Write username if it should be written
		if ( bUsername ) {
			if ( pconfig->Write(_("tcpuser"), wxstrUsername ) ) {
				if ( bVerbose ) {
					printf("Username written to config file '%s'\n", 
							(const char *)wxstrUsername.mb_str(wxConvUTF8 ));
				}
			}
		}

		// Write password if it should be written
		if ( bPassword ) {
      wxString str( md5.getDigest(), wxConvUTF8 );
			if ( pconfig->Write(_("tcppassword"), str ) ) {
				if ( bVerbose ) {
					printf("Encrypted password written to config file '%s'\n", 
							md5.getDigest() );
				}
			}
		}

		wxFFileOutputStream cfgOutstream( wxstrPath );
		printf("Write to file %s\n", (const char *)wxstrPath.mb_str(wxConvUTF8 ) ); 		

		if ( !cfgOutstream.Ok() ) {
			printf("Unable to open config file!\n");
			::wxUninitialize();
			return -1;
		}

		// Save changes
		if ( pconfig->Save( cfgOutstream ) ) {
			printf("Saved\n");
		}

		if ( NULL!= pconfig ) delete pconfig;
	
	} // bWrite

	printf("Encrypted password = %s\n", md5.getDigest() );

#if wxUSE_UNICODE
    {
        for ( int n = 0; n < argc; n++ )
            free(wxArgv[n]);

        delete [] wxArgv;
    }
#endif // wxUSE_UNICODE

	::wxUninitialize();

	printf("Done\n");
	return 0;
}





