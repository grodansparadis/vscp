// vscpd.cpp : Defines the class behaviour for the application.
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <syslog.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/app.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>

#include "vscpd.h"
#include <controlobject.h>
#include <version.h>

//#define DEBUG

// Globals for the daemon
int gbStopDaemon;
int gnDebugLevel = 0;
bool gbDontRunAsDaemon = false;
bool gbRestart = false;
wxString systemKey;

CControlObject *gpobj;

void copyleft(void);
void help(char *szPrgname);
void createFolderStuct(wxString& rootFolder);

void _sighandler( int sig )
{
    fprintf(stderr, "VSCPD: signal received, forced to stop.\n");
    syslog( LOG_CRIT, "VSCPD: signal received, forced to stop.: %m");    
    gpobj->logMsg(_("VSCPD: signal received, forced to stop."));
    gpobj->m_bQuit = true;
    gbStopDaemon = true;
    gbRestart = false;
    wxSleep( 1 );
    fprintf(stderr, "VSCPD: Shutdown in progress 1.\n");
    wxSleep( 1 );
    fprintf(stderr, "VSCPD: Shutdown in progress 2.\n");
    wxSleep( 1 );
    fprintf(stderr, "VSCPD: Shutdown in progress 3.\n");
}

/////////////////////////////////////////////////////////////////////////////
// VSCPApp


/////////////////////////////////////////////////////////////////////////////
// VSCPApp construction

VSCPApp::VSCPApp()
{
    ;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only app. object
//

int main( int argc, char **argv )
{
    int arg = 0;
    wxString rootFolder;    // Folder where VSCP files & folders will be located
    wxString strcfgfile;    // Points to XML configuration file
    
    fprintf( stderr, "Prepare to start vscpd...\n" );
    
    // Ignore return value from defunct processes d
    signal( SIGCHLD, SIG_IGN );

    crcInit(); 

    wxInitializer initializer;
    if ( !::wxInitialize() ) {
        fprintf( stderr, 
                    "[vscpd] Failed to initialize the wxWindows library, "
                    "aborting.\n");
        syslog( LOG_CRIT, 
                    "[vscpd] Failed to initialize the wxWindows library, "
                    "aborting.\n");
        return -1;
    }

#ifdef WIN32
    rootFolder = _("/programdata/vscp");
#else
    rootFolder = _("/srv/vscp/");
#endif

    strcfgfile = wxStandardPaths::Get().GetConfigDir() + _("/vscp/vscpd.conf");
    gbStopDaemon = false;

    VSCPApp theApp;
    wxSocketBase::Initialize();

    while ( ( arg = getopt( argc, argv, "d:c:f:k:hgs" ) ) != EOF ) {

        switch (arg) {
 
        case 's':
            wxPrintf( _("Stay Hungry. Stay Foolish.\n") );
            wxPrintf( _("I will ***NOT*** run as daemon! "
                        "(ctrl+c to terminate)\n\n"));
            gbDontRunAsDaemon = true;
            break;

        case 'c':
            strcfgfile = wxString( optarg, wxConvUTF8 );
            break;

        case 'r':
            rootFolder = wxString( optarg, wxConvUTF8 );
            break;

        case 'k':
            systemKey = wxString( optarg, wxConvUTF8 );
            break;

        case 'd':
            gnDebugLevel = atoi( optarg );
            break;

        case 'g':
            copyleft();
            exit(0);
            break;

        default:
        case 'h':
            help(argv[0]);
            exit(-1);
        }

    }

    fprintf( stderr, "[vscpd] Configfile = %s\n",
                (const char *)strcfgfile.mbc_str() );
    
    if ( !theApp.init( strcfgfile, rootFolder ) ) {
        syslog( LOG_CRIT, "[vscpd] Failed to configure. Terminating.\n");
        fprintf( stderr, "VSCPD: Failed to configure. Terminating.\n");
        exit( -1 );
    }

    fprintf( stderr, "VSCPD: Bye, bye.\n");
    exit( 0 );
}

/////////////////////////////////////////////////////////////////////////////
// initialisation

BOOL VSCPApp::init(wxString& strcfgfile, wxString& rootFolder )
{
    pid_t pid, sid;

    if ( !gbDontRunAsDaemon ) {

        // Fork child
        if (0 > (pid = fork())) {
            // Failure
            syslog( LOG_CRIT, "Failed to fork.\n" );
            return -1;
        }
        else if (0 != pid) {
            exit(0); // Parent goes by by.
        }

        sid = setsid(); // Become session leader
        if ( sid < 0 ) {
            // Failure
            syslog( LOG_CRIT, "Failed to become session leader.\n" );
            return -1;
        }

        umask(0); // Clear out file mode creation mask

        // Close out the standard file descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        if ( open("/", 0 ) ) {
            syslog( LOG_CRIT, "VSCPD: open / not 0: %m" );
        }

        dup2(0, 1);
        dup2(0, 2);

    }
    
    signal(SIGHUP, _sighandler );

    // Write pid to file
    FILE *pFile;
    pFile = fopen("/var/run/vscpd/vscpd.pid", "w");
    if ( NULL != pFile ) {
        syslog( LOG_CRIT, "%d\n", sid );
        fprintf( pFile, "%d\n", sid );
        fclose( pFile );
    }
    
    // Create folder structure
    createFolderStuct( rootFolder );

    // Change working directory to root folder
    if ( chdir( (const char *)rootFolder.mbc_str() ) ) {
        syslog( LOG_CRIT, "VSCPD: Failed to change dir to rootdir" );
        fprintf( stderr, "VSCPD: Failed to change dir to rootdir" );
        chdir("/tmp"); // security measure
    }

    struct sigaction my_action;

    // Ignore SIGPIPE
    my_action.sa_handler = SIG_IGN;
    my_action.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &my_action, NULL);

    // Redirect SIGQUIT
    my_action.sa_handler = _sighandler;
    my_action.sa_flags = SA_RESTART;
    sigaction(SIGQUIT, &my_action, NULL);

    // Redirect SIGABRT
    my_action.sa_handler = _sighandler;
    my_action.sa_flags = SA_RESTART;
    sigaction(SIGABRT, &my_action, NULL);

    // Redirect SIGINT
    my_action.sa_handler = _sighandler;
    my_action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &my_action, NULL);

    // Redirect SIGTERM
    my_action.sa_handler = _sighandler;
    my_action.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &my_action, NULL);
    
    // Redirect SIGHUP
    my_action.sa_handler = _sighandler;
    my_action.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &my_action, NULL);


    do {

        gbRestart = false;
        
        // Create the control object
        gpobj = new CControlObject();
        
        // Set system key
        vscp_hexStr2ByteArray( gpobj->m_systemKey,
                                32,
                                (const char *)systemKey.mbc_str() );

        fprintf( stderr, "VSCPD: init.\n");
        if ( !gpobj->init( strcfgfile, rootFolder ) ) {
            fprintf(stderr, "Can't initialise daemon. Exiting.\n");
            syslog(LOG_CRIT, "Can't initialise daemon. Exiting.");
            return FALSE;
        }

        fprintf( stderr, "VSCPD: run.\n");
        if ( !gpobj->run() ) {
            fprintf(stderr, "Unable to start the VSCPD application. Exiting.\n");
            syslog(LOG_CRIT, "Unable to start the VSCPD application. Exiting.");
            return FALSE;
        }

        fprintf( stderr, "VSCPD: cleanup.\n");
        if ( !gpobj->cleanup() ) {
            fprintf(stderr, "Unable to clean up the VSCPD application.\n");
            syslog( LOG_CRIT, "Unable to clean up the VSCPD application.");
            return FALSE;
        }
        
        fprintf( stderr, "VSCPD: cleanup done.\n");

        if ( gbRestart ) {
            syslog( LOG_CRIT, "VSCPD: Will try to restart.\n" );
            fprintf( stderr, "VSCPD: Will try to restart.\n" );
        }
        else {
            syslog( LOG_CRIT, "VSCPD: Will end things.\n" );
            fprintf( stderr, "VSCPD: Will end things.\n" );
        }
                
        fprintf( stderr, "VSCPD: Deleting the control object.\n");
        delete gpobj;

    } while ( gbRestart );

    // Remove the pid file
    unlink("/var/run/vscp/vscpd/vscpd.pid");
    
    fprintf( stderr, "VSCPD: Yes we are leaving this world...\n");
 
    gpobj = NULL;

    return TRUE;
}




///////////////////////////////////////////////////////////////////////////////
// copyleft

void copyleft(void)
{
    wxPrintf(_("\n\n"));
    wxPrintf(_("vscpd - "));
    wxPrintf(_(VSCPD_DISPLAY_VERSION));
    wxPrintf(_("\n"));
    wxPrintf(_(VSCPD_COPYRIGHT));
    wxPrintf(_("\n"));
    wxPrintf(_("\n"));
    wxPrintf(_("This program is free software; you can redistribute it and/or \n"));
    wxPrintf(_("modify it under the terms of the GNU General Public License as \n"));
    wxPrintf(_("published by the Free Software Foundation; either version 2 of \n"));
    wxPrintf(_("the License, or ( at your option ) any later version.\n"));
    wxPrintf(_("\n"));
    wxPrintf(_("This program is distributed in the hope that it will be useful,\n"));
    wxPrintf(_("but WITHOUT ANY WARRANTY; without even the implied warranty of\n"));
    wxPrintf(_("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"));
    wxPrintf(_("\n"));
    wxPrintf(_("Check the MIT license for more details.\n"));
    wxPrintf(_("\n"));

}


///////////////////////////////////////////////////////////////////////////////
// help

void help(char *szPrgname)
{
    fprintf(stderr, "Usage: %s [-ahg] [-c command-file] [-c key] -dn\n", szPrgname);
    fprintf(stderr, "\t-h\tThis help message.\n");
    fprintf(stderr, "\t-s\tStandalone (don't run as daemon). \n");
    fprintf(stderr, "\t-r\tSpecify VSCP root folder. \n");
    fprintf(stderr, "\t-c\tSpecify a configuration file. \n");
    fprintf(stderr, "\t-k\t32 byte encryption key string in hex format. \n");
    fprintf(stderr, "\t-d\tDebug level (0-2). 0=Default. ");
    fprintf(stderr, "that should be used (default: /etc/canalworks.conf).\n");
    fprintf(stderr, "\t-g\tPrint the GNU copyleft info.\n");
}


///////////////////////////////////////////////////////////////////////////////
// createFolderStuct
//

void createFolderStuct( wxString& rootFolder )
{
    if ( !wxDir::Exists( rootFolder + _("/certs") ) ) {
        wxDir::Make( rootFolder + _("/certs"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/web/images") ) ) {
        wxDir::Make( rootFolder + _("/web/images"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/web/js") ) ) {
        wxDir::Make( rootFolder + _("/web/js"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/web/service") ) ) {
        wxDir::Make( rootFolder + _("/web/service"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/drivers/level1") ) ) {
        wxDir::Make( rootFolder + _("/drivers/level1"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/drivers/level2") ) ) {
        wxDir::Make( rootFolder + _("/drivers/level2"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/drivers/level3") ) ) {
        wxDir::Make( rootFolder + _("/drivers/level3"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/actions") ) ) {
        wxDir::Make( rootFolder + _("/actions"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/scripts") ) ) {
        wxDir::Make( rootFolder + _("/scripts"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/tables") ) ) {
        wxDir::Make( rootFolder + _("/tables"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/logs") ) ) {
        wxDir::Make( rootFolder + _("/logs"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/ux") ) ) {
        wxDir::Make( rootFolder + _("/ux"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/upload") ) ){
        wxDir::Make( rootFolder + _("/upload"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
    if ( !wxDir::Exists( rootFolder + _("/mdf") ) ) {
        wxDir::Make( rootFolder + _("/mdf"), 
                wxS_DIR_DEFAULT,
                wxPATH_MKDIR_FULL );
    }
    
}   
