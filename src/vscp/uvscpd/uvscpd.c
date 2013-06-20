// uvscpd.c : Mainfile for the application.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#include "uvscpd.h"
#include "../../common/crc.h"
#include "../common/version.h"

#define DEBUG
#define PORT 8088
#define BUF_LEN 0xFFFF
#define PACKET_DUMP

// Globals for the daemon
int gbStopDaemon;
int gnDebugLevel = 0;
int gbDontRunAsDaemon = FALSE;

void copyleft(void);
void help(char *szPrgname);

void sighandler(int sig)
{
    gbStopDaemon = TRUE;
    syslog(LOG_CRIT, "uvscpd: signal received, forced to stop.: %m");
    //sleep( 3 );
    //exit(-1);
}


/////////////////////////////////////////////////////////////////////////////
// main

int main(int argc, char **argv)
{
    int listenSocket;
    
    crcInit();

    if (!gbDontRunAsDaemon) {

        pid_t pid, sid;

        // Fork child	
        if (0 > (pid = fork())) {
            // Failure
            printf("Failed to fork.");
            return -1;
        } else if (0 != pid) {
            exit(0); // Parent goes by by.
        }

        sid = setsid(); // Become session leader
        if (sid < 0) {
            // Failure 
            printf("Failed to become session leader.");
            return -1;
        }

        // Write pid to file
        FILE *pFile;
        pFile = fopen("/var/run/vscpd.pid", "w");
        if (NULL != pFile) {
            fprintf(pFile, "%d\n", sid);
            fclose(pFile);
        }

        int rv = chdir("/"); // Change working directory
        umask(0); // Clear out file mode creation mask

        // Close out the standard file descriptors 
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        if (open("/", 0)) {
            syslog(LOG_CRIT, "VSCPD: open / not 0: %m");
        }

        dup2(0, 1);
        dup2(0, 2);

        struct sigaction my_action;

        // Ignore SIGPIPE
        my_action.sa_handler = SIG_IGN;
        my_action.sa_flags = SA_RESTART;
        sigaction(SIGPIPE, &my_action, NULL);

        // Redirect SIGQUIT
        my_action.sa_handler = sighandler;
        my_action.sa_flags = SA_RESTART;
        sigaction(SIGQUIT, &my_action, NULL);

        // Redirect SIGABRT
        my_action.sa_handler = sighandler;
        my_action.sa_flags = SA_RESTART;
        sigaction(SIGABRT, &my_action, NULL);

        // Redirect SIGINT
        my_action.sa_handler = sighandler;
        my_action.sa_flags = SA_RESTART;
        sigaction(SIGINT, &my_action, NULL);

        // Redirect SIGTERM
        my_action.sa_handler = sighandler;
        my_action.sa_flags = SA_RESTART;
        sigaction(SIGTERM, &my_action, NULL);

    }
    
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        error("create socket failed");
    }
    
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(PORT);
    if (bind(listenSocket, (struct sockaddr *) &local, sizeof(local)) == -1) {
        error("bind failed");
    }
    
    if (listen(listenSocket, 1) == -1) {
        error("listen failed");
    }
    printf("opened %s:%d\n", inet_ntoa(local.sin_addr), ntohs(local.sin_port));
    
    while (TRUE) {
        struct sockaddr_in remote;
        socklen_t sockaddrLen = sizeof(remote);
        int clientSocket = accept(listenSocket, (struct sockaddr*)&remote, &sockaddrLen);
        if (clientSocket == -1) {
            error("accept failed");
        }
        
        printf("connected %s:%d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
        //clientWorker(clientSocket);
        printf("disconnected\n");
    }
    
    close(listenSocket);
    return EXIT_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// copyleft

void copyleft(void)
{
    printf("\n\n");
    printf("uvscpd - ");
    printf(VSCPD_DISPLAY_VERSION);
    printf("\n");
    printf(VSCPD_COPYRIGHT);
    printf("\n");
    printf("\n");
    printf("This program is free software; you can redistribute it and/or \n");
    printf("modify it under the terms of the GNU General Public License as \n");
    printf("published by the Free Software Foundation; either version 2 of \n");
    printf("the License, or ( at your option ) any later version.\n");
    printf("\n");
    printf("This program is distributed in the hope that it will be useful,\n");
    printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    printf("\n");
    printf("GNU General Public License for more details.\n");
    printf("\n");
    printf("You should have received a copy of the GNU General Public License\n");
    printf("along with this program; if not, write to the Free Software\n");
    printf("Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n");
}


///////////////////////////////////////////////////////////////////////////////
// help

void help(char *szPrgname)
{
    fprintf(stderr, "Usage: %s [-ahg] [-c command-file] -dn\n", szPrgname);
    fprintf(stderr, "\t-h\tThis help message.\n");
    fprintf(stderr, "\t-s\tStandalone (don't run as daemon)..\n");
    fprintf(stderr, "\t-c\tSpecify a configuration file \n");
    fprintf(stderr, "\t-d\tDebug level. 0=None, 99=Don't run as daemon. ");
    fprintf(stderr, "that should be used (default: /etc/canalworks.conf).\n");
    fprintf(stderr, "\t-g\tPrint the GNU copyleft info.\n");
}




