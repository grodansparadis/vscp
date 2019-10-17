// vscpd.cpp : Defines the class behaviour for the application.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#include <string>

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
#include <getopt.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include <crc.h>
#include "vscpd.h"
#include <controlobject.h>
#include <version.h>

//#define DEBUG

// Globals for the daemon
int gbStopDaemon;
int gnDebugLevel = 0;
bool gbDontRunAsDaemon = false;
bool gbRestart = false;
std::string systemKey;

// Control object
CControlObject *gpobj;

// Forward declarations
int init( std::string& strcfgfile, std::string& rootFolder );
void copyleft( void );
void help( char *szPrgname );
void createFolderStuct( std::string& rootFolder );

void _sighandler( int sig )
{
    fprintf(stderr, "VSCPD: signal received, forced to stop.\n");
    syslog( LOG_ERR, "VSCPD: signal received, forced to stop.: %m");
    gpobj->m_bQuit = true;
    gbStopDaemon = true;
    gbRestart = false;
}


/////////////////////////////////////////////////////////////////////////////
// The one and only app. object
//

int main( int argc, char **argv )
{
    int opt = 0;
    std::string rootFolder;    // Folder where VSCP files & folders will be located
    std::string strcfgfile;    // Points to XML configuration file

    openlog( "vscpd", LOG_PERROR | LOG_PID | LOG_CONS, LOG_DAEMON );

    fprintf( stderr, "Prepare to start vscpd...\n" );

    // Ignore return value from defunct processes d
    signal( SIGCHLD, SIG_IGN );

    crcInit();

    rootFolder = "/srv/vscp/";
    strcfgfile = "/etc/vscp/vscpd.conf";
    gbStopDaemon = false;

    while ( ( opt = getopt(argc, argv, "d:c:f:k:hgs") ) != -1 ) {

        switch (opt) {

        case 's':
            fprintf( stderr, "Stay Hungry. Stay Foolish.\n" );
            fprintf( stderr, "I will ***NOT*** run as daemon! "
                             "(ctrl+c to terminate)\n\n");
            gbDontRunAsDaemon = true;
            break;

        case 'c':
            strcfgfile = optarg;
            break;

        case 'r':
            rootFolder = optarg;
            break;

        case 'k':
            systemKey = optarg;
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
            help( argv[0] );
            exit( -1 );
        }

    }

    fprintf( stderr, "[vscpd] Configfile = %s\n",
                (const char *)strcfgfile.c_str() );

    if ( !init( strcfgfile, rootFolder ) ) {
        syslog( LOG_ERR, "[vscpd] Failed to configure. Terminating.\n");
        fprintf( stderr, "VSCPD: Failed to configure. Terminating.\n");
        exit( -1 );
    }

    closelog(); // Close syslog

    fprintf( stderr, "VSCPD: Bye, bye.\n");
    exit(EXIT_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////
// initialisation

int init(std::string& strcfgfile, std::string& rootFolder )
{
    pid_t pid, sid;

    if ( !gbDontRunAsDaemon ) {

        // Fork child
        if (0 > (pid = fork())) {
            // Failure
            syslog( LOG_ERR, "Failed to fork.\n" );
            return -1;
        }
        else if (0 != pid) {
            exit(0); // Parent goes by by.
        }

        sid = setsid(); // Become session leader
        if ( sid < 0 ) {
            // Failure
            syslog( LOG_ERR, "Failed to become session leader.\n" );
            return -1;
        }

        umask(0); // Clear out file mode creation mask

        // Close out the standard file descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        if ( open("/", 0 ) ) {
            syslog( LOG_ERR, "VSCPD: open / not 0: %m" );
        }

        dup2(0, 1);
        dup2(0, 2);

    }

    signal(SIGHUP, _sighandler );

    // Write pid to file
    FILE *pFile;
    pFile = fopen("/var/run/vscpd/vscpd.pid", "w");
    if ( NULL != pFile ) {
        syslog( LOG_ERR, "%d\n", sid );
        fprintf( pFile, "%d\n", sid );
        fclose( pFile );
    }

    // Create folder structure
    createFolderStuct( rootFolder );

    // Change working directory to root folder
    if ( chdir( (const char *)rootFolder.c_str() ) ) {
        syslog( LOG_ERR, "VSCPD: Failed to change dir to rootdir" );
        fprintf( stderr, "VSCPD: Failed to change dir to rootdir" );
        int n = chdir("/tmp"); // security measure
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
                                (const char *)systemKey.c_str() );

        fprintf( stderr, "VSCPD: init.\n");
        if ( !gpobj->init( strcfgfile, rootFolder ) ) {
            fprintf(stderr, "Can't initialize daemon. Exiting.\n");
            syslog(LOG_ERR, "Can't initialize daemon. Exiting.");
            return FALSE;
        }

        // *******************************
        //    Main loop is entered here
        // *******************************

        fprintf( stderr, "VSCPD: run.\n");
        if ( !gpobj->run() ) {
            fprintf(stderr, "Unable to start the VSCPD application. Exiting.\n");
            syslog(LOG_ERR, "Unable to start the VSCPD application. Exiting.");
            return FALSE;
        }

        fprintf( stderr, "VSCPD: cleanup.\n");
        //gbRestart = 0;
        /*if ( !gpobj->cleanup() ) {
            fprintf(stderr, "Unable to clean up the VSCPD application.\n");
            syslog( LOG_ERR, "Unable to clean up the VSCPD application.");
            return FALSE;
        }*/

        fprintf( stderr, "VSCPD: cleanup done.\n");

        if ( gbRestart ) {
            syslog( LOG_ERR, "VSCPD: Will try to restart.\n" );
            fprintf( stderr, "VSCPD: Will try to restart.\n" );
        }
        else {
            syslog( LOG_ERR, "VSCPD: Will end things.\n" );
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
    fprintf( stderr, "\n\n");
    fprintf( stderr, "vscpd - ");
    fprintf( stderr, VSCPD_DISPLAY_VERSION);
    fprintf( stderr, "\n");
    fprintf( stderr, VSCPD_COPYRIGHT);
    fprintf( stderr, "\n");
    fprintf( stderr, "\n");
    fprintf( stderr, "This program is free software; you can redistribute it and/or \n");
    fprintf( stderr, "modify it under the terms of the GNU General Public License as \n");
    fprintf( stderr, "published by the Free Software Foundation; either version 2 of \n");
    fprintf( stderr, "the License, or ( at your option ) any later version.\n");
    fprintf( stderr, "\n");
    fprintf( stderr, "This program is distributed in the hope that it will be useful,\n");
    fprintf( stderr, "but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    fprintf( stderr, "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    fprintf( stderr, "\n");
    fprintf( stderr, "Check the MIT license for more details.\n");
    fprintf( stderr, "\n");

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
    fprintf(stderr, "that should be used (default: /etc/vscpd.conf).\n");
    fprintf(stderr, "\t-g\tPrint MIT license info.\n");
}


///////////////////////////////////////////////////////////////////////////////
// createFolderStuct
//

void createFolderStuct( std::string& rootFolder )
{
    std::string path;

    path = rootFolder + "/certs";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/web";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/web/images";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/web/js";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/web/service";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/drivers";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/drivers/level1";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/drivers/level2";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/drivers/level3";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/actions";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/scripts";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/tables";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/logs";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/ux";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/upload";
    if ( 0 == vscp_dirExists( path.c_str() ) ){
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    path = rootFolder + "/mdf";
    if ( 0 == vscp_dirExists( path.c_str() ) ) {
        mkdir( path.c_str(),
                S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

}
