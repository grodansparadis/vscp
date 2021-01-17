// vscpd.cpp : Defines the class behaviour for the application.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <deque>
#include <string>

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include "canal_macro.h"
#include "vscpd.h"
#include <controlobject.h>
#include <crc.h>
#include <version.h>
#include <vscphelper.h>

//#define DEBUG

// Globals for the daemon
int gbStopDaemon;
uint64_t gDebugLevel  = 0;
bool gbDontRunAsDaemon = false;
bool gbRestart         = false;
std::string systemKey;

// Control object
CControlObject* gpobj;

// Forward declarations
int
init(std::string& strcfgfile, std::string& rootFolder);
void
copyleft(void);
void
help(char* szPrgname);
bool
createFolderStuct(std::string& rootFolder);

void
_sighandlerStop(int sig)
{
    fprintf(stderr, "vscpd: signal received, forced to stop.\n");
    syslog(LOG_ERR, "vscpd: signal received, forced to stop.: %m");
    gpobj->m_bQuit = true;
    gbStopDaemon   = true;
    gbRestart      = false;
}

void
_sighandlerRestart(int sig)
{
    fprintf(stderr, "vscpd: signal received, restart. %m\n");
    syslog(LOG_ERR, "vscpd: signal received, restart.: %m");
    gpobj->m_bQuit = true;
    gbStopDaemon   = false;
    gbRestart      = true;
}


/////////////////////////////////////////////////////////////////////////////
// The one and only app. object
//

int
main(int argc, char** argv)
{
    int opt = 0;
    std::string rootFolder; // Folder where VSCP files & folders will be located
    std::string strcfgfile; // Points to XML configuration file

    fprintf(stderr, "Prepare to start vscpd...\n");

    openlog("vscpd", LOG_PERROR | LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Starting the VSCP daemon...");

    // Ignore return value from defunct processes d
    signal(SIGCHLD, SIG_IGN);

    crcInit();

    rootFolder   = "/var/lib/vscp/";
    strcfgfile   = "/etc/vscp/vscpd.json";
    gbStopDaemon = false;

    while ((opt = getopt(argc, argv, "d:c:r:k:hgs")) != -1) {

        switch (opt) {

            case 's':
                fprintf(stderr,
                        "I will ***NOT*** run as daemon! "
                        "(ctrl+c to terminate)\n");
                gbDontRunAsDaemon = true;
                break;

            case 'c':
                strcfgfile = optarg;
                break;

            case 'r':
                rootFolder = optarg;
                fprintf(stderr, "Will use rootfolder = %s", rootFolder.c_str());
                syslog(LOG_INFO,
                       "Will use rootfolder = %s",
                       rootFolder.c_str());
                break;

            case 'k':
                systemKey = optarg;
                break;

            case 'd':
                gDebugLevel =  std::stoull(optarg);     //atoi(optarg);
                fprintf(stderr, "Debug flags=%s\n", optarg);
                syslog(LOG_INFO, "Debug flags=%s\n", optarg);
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

    fprintf(stderr,
            "[vscpd] Configfile = %s\n",
            (const char*)strcfgfile.c_str());

    if (!init(strcfgfile, rootFolder)) {
        syslog(LOG_ERR, "[vscpd] Failed to configure. Terminating.\n");
        fprintf(stderr, "vscpd: Failed to configure. Terminating.\n");
        exit(-1);
    }

    closelog();     // Close syslog

    fprintf(stderr, "vscpd: Bye, bye.\n");
    exit(EXIT_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////
// initialisation

int
init(std::string& strcfgfile, std::string& rootFolder)
{
    pid_t pid, sid;

    if (!gbDontRunAsDaemon) {

        // Fork child
        if (0 > (pid = fork())) {
            // Failure
            syslog(LOG_ERR, "Failed to fork.\n");
            return -1;
        } else if (0 != pid) {
            exit(0);    // Parent goes by by.
        }

        sid = setsid();     // Become session leader
        if (sid < 0) {
            // Failure
            syslog(LOG_ERR, "Failed to become session leader.\n");
            return -1;
        }

        umask(0);   // Clear out file mode creation mask

        // Close out the standard file descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        if (open("/", 0)) {
            syslog(LOG_ERR, "vscpd: open / not 0: %m");
        }

        dup2(0, 1);
        dup2(0, 2);
    }

    // signal(SIGHUP, _sighandlerStop);
    // signal(SIGUSR1, _sighandlerStop);
    // signal(SIGUSR2, _sighandlerRestart);

    // Write pid to file
    FILE* pFile;
    pFile = fopen("/var/run/vscpd.pid", "w");
    if (NULL == pFile) {
        syslog(LOG_ERR, "Writing pid file failed.\n");
        fprintf(stderr, "Writing pid file failed.\n");
    } else {
        syslog(LOG_ERR, "Writing pid file [/var/run/vscpd.pid] sid=%u\n", sid);
        fprintf(pFile, "%u\n", sid);
        fclose(pFile);
    }

    // Create folder structure
    if (!createFolderStuct(rootFolder)) {
        syslog(LOG_ERR,
               "vscpd: Folder structure is not in place (You may need to run "
               "as root).");
        fprintf(stderr,
                "vscpd: Folder structure is not in place (You may need to run "
                "as root).");
        unlink("/var/run/vscpd.pid");
        return -1;
    }

    // Change working directory to root folder
    if (chdir((const char*)rootFolder.c_str())) {
        syslog(LOG_ERR, "vscpd: Failed to change dir to rootdir");
        fprintf(stderr, "vscpd: Failed to change dir to rootdir");
        unlink("/var/run/vscpd.pid");
        if (-1 == chdir("/var/lib/vscp/vscpd")) {
            syslog(
              LOG_ERR,
              "Unable to chdir to home folder [/var/lib/vscp/vscpd] errno=%d",
              errno);
        }

        return -1;
    }

    struct sigaction my_action;

    // Ignore SIGPIPE
    my_action.sa_handler = SIG_IGN;
    my_action.sa_flags   = SA_RESTART;
    sigaction(SIGPIPE, &my_action, NULL);

    // Redirect SIGQUIT
    my_action.sa_handler = _sighandlerStop;
    my_action.sa_flags   = SA_RESTART;
    sigaction(SIGQUIT, &my_action, NULL);

    // Redirect SIGABRT
    my_action.sa_handler = _sighandlerStop;
    my_action.sa_flags   = SA_RESTART;
    sigaction(SIGABRT, &my_action, NULL);

    // Redirect SIGINT
    my_action.sa_handler = _sighandlerStop;
    my_action.sa_flags   = SA_RESTART;
    sigaction(SIGINT, &my_action, NULL);

    // Redirect SIGTERM
    my_action.sa_handler = _sighandlerStop;
    my_action.sa_flags   = SA_RESTART;
    sigaction(SIGTERM, &my_action, NULL);

    // Redirect SIGHUP
    my_action.sa_handler = _sighandlerStop;
    my_action.sa_flags   = SA_RESTART;
    sigaction(SIGHUP, &my_action, NULL);

    do {

        gbRestart = false;

        // Create the control object
        gpobj = new CControlObject();

        // Set system key
        vscp_hexStr2ByteArray(gpobj->m_systemKey,
                              32,
                              (const char*)systemKey.c_str());

        // Tansfer read debug parameters if set
        gpobj->m_debugFlags = gDebugLevel;

        fprintf(stderr, "vscpd: init.\n");
        if (!gpobj->init(strcfgfile, rootFolder)) {
            fprintf(stderr, "Can't initialize daemon. Exiting.\n");
            syslog(LOG_ERR, "Can't initialize daemon. Exiting.");
            unlink("/var/run/vscpd.pid");
            return FALSE;
        }

        // *******************************
        //    Main loop is entered here
        // *******************************

        fprintf(stderr, "vscpd: run.\n");
        if (!gpobj->run()) {
            fprintf(stderr,
                    "Unable to start the vscpd application. Exiting.\n");
            syslog(LOG_ERR, "Unable to start the vscpd application. Exiting.");
            unlink("/var/run/vscpd.pid");
            return FALSE;
        }

        fprintf(stderr, "vscpd: cleanup.\n");

        if (!gpobj->cleanup()) {
            fprintf(stderr, "Unable to clean up the vscpd application.\n");
            syslog(LOG_ERR, "Unable to clean up the vscpd application.");
            return FALSE;
        }

        fprintf(stderr, "vscpd: cleanup done.\n");

        if (gbRestart) {
            syslog(LOG_ERR, "vscpd: Will try to restart.\n");
            fprintf(stderr, "vscpd: Will try to restart.\n");
        } else {
            syslog(LOG_ERR, "vscpd: Will end things.\n");
            fprintf(stderr, "vscpd: Will end things.\n");
        }

        fprintf(stderr, "vscpd: Deleting the control object.\n");
        delete gpobj;

    } while (gbRestart);

    // Remove the pid file
    unlink("/var/run/vscp/vscpd.pid");

    fprintf(stderr, "vscpd: ending...\n");

    gpobj = NULL;

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// copyleft

void
copyleft(void)
{
    fprintf(stderr, "\n\n");
    fprintf(stderr, "vscpd - ");
    fprintf(stderr, VSCPD_DISPLAY_VERSION);
    fprintf(stderr, "\n");
    fprintf(stderr, VSCPD_COPYRIGHT);
    fprintf(stderr, "\n");
    fprintf(stderr, "\n");
    fprintf(
      stderr,
      "The MIT License (MIT)"
      "\n"
      "Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB\n"
      "<info@grodansparadis.com>\n"
      "\n"
      "Permission is hereby granted, free of charge, to any person obtaining a "
      "copy\n"
      "of this software and associated documentation files (the 'Software'), "
      "to deal\n"
      "in the Software without restriction, including without limitation the "
      "rights\n"
      "to use, copy, modify, merge, publish, distribute, sublicense, and/or "
      "sell\n"
      "copies of the Software, and to permit persons to whom the Software is\n"
      "furnished to do so, subject to the following conditions:\n"
      "\n"
      "The above copyright notice and this permission notice shall be included "
      "in\n"
      "all copies or substantial portions of the Software.\n"
      "\n"
      "THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS "
      "OR\n"
      "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
      "MERCHANTABILITY,\n"
      "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL "
      "THE\n"
      "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
      "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING "
      "FROM,\n"
      "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS "
      "IN THE\n"
      "SOFTWARE.\n");
    fprintf(stderr, "\n");
}

///////////////////////////////////////////////////////////////////////////////
// help

void
help(char* szPrgname)
{
    fprintf(stderr,
            "Usage: %s [-hg] [-r rootfolder] [-c config-file] [-k key] "
            "-dd0,d1,d2...\n",
            szPrgname);
    fprintf(stderr, "\t-h\tThis help message.\n");
    fprintf(stderr, "\t-s\tStandalone (don't run as daemon). \n");
    fprintf(stderr, "\t-r\tSpecify VSCP root folder. \n");
    fprintf(stderr, "\t-c\tSpecify a configuration file. \n");
    fprintf(stderr, "\t-k\t32 byte encryption key string in hex format. \n");
    fprintf(stderr, "\t-d\tDebug flags.");
    fprintf(stderr, "that should be used (default: /etc/vscpd.json).\n");
    fprintf(stderr, "\t-g\tPrint MIT license info.\n");
}

///////////////////////////////////////////////////////////////////////////////
// createFolder
//

bool
createFolder(const char* folder)
{
    if (0 == vscp_dirExists(folder)) {
        if (-1 == mkdir(folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
            fprintf(stderr, "Failed to create folder %s\n", folder);
            syslog(LOG_ERR, "Failed to create folder %s\n", folder);
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// createFolderStuct
//

bool
createFolderStuct(std::string& rootFolder)
{
    std::string path;

    if (!createFolder(rootFolder.c_str())) {
        return false;
    }

    if (!createFolder("/etc/vscp/certs")) {
        return false;
    }

    if (!createFolder("/etc/vscp/ca_certificates")) {
        return false;
    }

    path = rootFolder + "/web";
    if (!createFolder(path.c_str())) {
        return false;
    }

    path = rootFolder + "/web/html";
    if (!createFolder(path.c_str())) {
        return false;
    }

    path = rootFolder + "/web/html/images";
    if (!createFolder(path.c_str())) {
        return false;
    }

    path = rootFolder + "/web/html/js";
    if (!createFolder(path.c_str())) {
        return false;
    }

    path = rootFolder + "/web/html/css";
    if (!createFolder(path.c_str())) {
        return false;
    }

    return true;
}
