// vscpd.cpp : Defines the class behaviour for the application.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2024 Ake Hedman, the VSCP project
// <info@vscp.org>
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

#ifdef WIN32
// For getopt
#define __GNU_LIBRARY__
#include <pch.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef WIN32
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#else

#endif

#include "canal-macro.h"
#include "vscpd.h"
#include "controlobject.h"
#include <crc.h>
#include <version.h>
#include <vscphelper.h>

#include <deque>
#include <string>

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

//#define DEBUG

// Globals for the daemon
int gbStopDaemon;
uint64_t gDebugLevel   = 0;
bool gbDontRunAsDaemon = false;

// The default random encryption key
uint8_t __vscp_key[32] = { 0x2d, 0xbb, 0x07, 0x9a, 0x38, 0x98, 0x5a, 0xf0, 0x0e, 0xbe, 0xef,
                           0xe2, 0x2f, 0x9f, 0xfa, 0x0e, 0x7f, 0x72, 0xdf, 0x06, 0xeb, 0xe4,
                           0x45, 0x63, 0xed, 0xf4, 0xa1, 0x07, 0x3c, 0xab, 0xc7, 0xd4 };

// Control object
CControlObject *gpobj;

// Forward declarations
void
copyleft(void);
void
help(char *szPrgname);

void
_sighandlerStop(int sig)
{
  fprintf(stderr, "vscpd: signal received, forced to stop.\n");
  gpobj->m_bQuit = true;
  gbStopDaemon   = true;
}

void
_sighandlerRestart(int sig)
{
  fprintf(stderr, "vscpd: signal received, restart. %s\n", strerror(errno));
  gpobj->m_bQuit = true;
  gbStopDaemon   = false;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only app. object
//

int
main(int argc, char **argv)
{
  int opt = 0;
  std::string rootFolder; // Folder where VSCP files & folders will be located
  std::string strcfgfile; // Points to XML configuration file

  // Init pool
  spdlog::init_thread_pool(8192, 1);

  // Flush log every five seconds
  spdlog::flush_every(std::chrono::seconds(5));

  auto console = spdlog::stdout_color_mt("console");
  // Start out with level=info. Config may change this
  console->set_level(spdlog::level::trace);
  console->set_pattern("[vscp: %c] [%^%l%$] %v");
  spdlog::set_default_logger(console);

  // Ignore return value from defunct processes id
#ifndef WIN32
  signal(SIGCHLD, SIG_IGN);
#endif
  crcInit();

  rootFolder   = "/var/lib/vscp/";
  strcfgfile   = "/etc/vscp/vscpd.json";
  gbStopDaemon = false;

  while ((opt = getopt(argc, argv, "d:c:r:k:hgsv")) != -1) {

    switch (opt) {

      case 's':
        gbDontRunAsDaemon = true;
        console->info("I will ***NOT*** run as a daemon! (use ctrl+c to terminate)");
        break;

      case 'c':
        strcfgfile = optarg;
        break;

      case 'r':
        rootFolder = optarg;
        console->info("Will use rootfolder = %s", rootFolder.c_str());
        break;

      case 'k':
        // Set system key
        vscp_hexStr2ByteArray(__vscp_key, 32, optarg);
        break;

      case 'd':
        gDebugLevel = std::stoull(optarg);
        console->info("Debug flags=%s\n", optarg);
        break;

      case 'g':
        copyleft();
        exit(0);
        break;
        
      case 'v':
        fprintf(stderr, "%s\n", VSCPD_DISPLAY_VERSION);
        exit(0);
        break;

      default:
      case 'h':
        help(argv[0]);
        exit(-1);
    }
  }

  console->info("Starting the VSCP daemon...");

  // * * * init * * *

  console->info("Configfile = {}", strcfgfile);
#ifndef WIN32
  pid_t pid, sid;
#endif // WIN32

#ifndef WIN32
  if (!gbDontRunAsDaemon) {

    // Fork child
    if (0 > (pid = fork())) {
      // Failure
      console->error("Failed to fork.");
      return -1;
    }
    else if (0 != pid) {
      exit(0); // Parent goes by by.
    }

    sid = setsid(); // Become session leader
    if (sid < 0) {
      // Failure
      console->error("Failed to become session leader.");
      return -1;
    }

    umask(0); // Clear out file mode creation mask

    // Close out the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    if (open("/", 0)) {
      console->warn("Open / not 0: %m.");
    }

    dup2(0, 1);
    dup2(0, 2);

  }    // !gbDontRunAsDaemon
#endif // WIN32

#ifndef WIN32
  // Write pid to file
  FILE *pFile;
  pFile = fopen("/var/run/vscpd.pid", "w");
  if (NULL == pFile) {
    console->warn("Writing pid file failed (access rights?).");
  }
  else {
    console->debug("Writing pid file [/var/run/vscpd.pid] sid=%u\n", sid);
    fprintf(pFile, "%u\n", sid);
    fclose(pFile);
  }
#endif // WIN32

#ifndef WIN32
  // Change working directory to VSCP root folder
  if (chdir((const char *) rootFolder.c_str())) {
    console->warn("Failed to change dir to rootdir.");
    if (-1 == chdir("/var/lib/vscp/vscpd")) {
      console->warn("Unable to chdir to home folder [/var/lib/vscp/vscpd] errno=%d", errno);
    }

    unlink("/var/run/vscpd.pid");
    // spdlog::drop_all();
    // spdlog::shutdown();
    // exit(EXIT_FAILURE);
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
#endif // WIN32

  // Create the control object
  gpobj = new CControlObject();

  // Tansfer read debug parameters if set
  console->info("Debugflags (cmdline): 0x{0:x}", gDebugLevel);

  if (!gpobj->init(strcfgfile, rootFolder)) {
    console->critical("Can't initialize daemon. Exiting.\n");
#ifndef WIN32
    unlink("/var/run/vscpd.pid");
#endif
    spdlog::drop_all();
    spdlog::shutdown();
    exit(EXIT_FAILURE);
  }

  // spdlog::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");

  // Console log
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  if (gpobj->m_bEnableConsoleLog) {
    console_sink->set_level(gpobj->m_consoleLogLevel);
    console_sink->set_pattern(gpobj->m_consoleLogPattern);
  }
  else {
    // If disabled set to off
    console_sink->set_level(spdlog::level::off);
  }

  try {
    auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(gpobj->m_path_to_log_file.c_str(),
                                                                                     gpobj->m_max_log_size,
                                                                                     gpobj->m_max_log_files);
    if (gpobj->m_bEnableFileLog) {
      rotating_file_sink->set_level(gpobj->m_fileLogLevel);
      rotating_file_sink->set_pattern(gpobj->m_fileLogPattern);
    }
    else {
      // If disabled set to off
      rotating_file_sink->set_level(spdlog::level::off);
    }

    std::vector<spdlog::sink_ptr> sinks{ console_sink, rotating_file_sink };
    auto logger = std::make_shared<spdlog::async_logger>("logger",
                                                         sinks.begin(),
                                                         sinks.end(),
                                                         spdlog::thread_pool(),
                                                         spdlog::async_overflow_policy::block);
    // The separate sub loggers will handle trace levels
    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
  }
  catch (...) {
    console->critical("vscpd: Unable to start the vscpd application. Logs Exiting.");
    spdlog::drop_all();
    spdlog::shutdown();
    exit(EXIT_FAILURE);
  }

  // *******************************
  //    Main loop is entered here
  // *******************************

  //         * * * RUN * * *

  console->debug("vscpd: run.");

  if (!gpobj->run()) {
    console->critical("vscpd: Unable to start the vscpd application. Exiting.");
#ifndef WIN32
    unlink("/var/run/vscpd.pid");
#endif
    spdlog::drop_all();
    spdlog::shutdown();
    exit(EXIT_FAILURE);
  }

  // * * * CLEAN UP * * *

  console->debug("vscpd: cleanup.");

  if (!gpobj->cleanup()) {
    console->critical("vscpd: Unable to clean up the vscpd application.");
    spdlog::drop_all();
    spdlog::shutdown();
    exit(EXIT_FAILURE);
  }

  console->debug("vscpd: Deleting the control object.");
  delete gpobj;

  // Remove the pid file
#ifndef WIN32
  unlink("/var/run/vscp/vscpd.pid");
#endif
  gpobj = NULL;

  console->info("vscpd: Bye, bye.");

  spdlog::drop_all();
  spdlog::shutdown();

  exit(EXIT_SUCCESS);
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
  fprintf(stderr,
          "The MIT License (MIT)"
          "\n"
          "Copyright (C) 2000-2024 Ake Hedman, the VSCP project\n"
          "<info@vscp.org>\n"
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
help(char *szPrgname)
{
  fprintf(stderr,
          "Usage: %s [-hg] [-r rootfolder] [-c config-file] [-k key] "
          "-dd0,d1,d2...\n",
          szPrgname);
  fprintf(stderr, "\t-h\tThis help message.\n");
  fprintf(stderr, "\t-v\tPrint version. \n");
  fprintf(stderr, "\t-s\tStandalone (don't run as daemon). \n");
  fprintf(stderr, "\t-r\tSpecify VSCP root folder. \n");
  fprintf(stderr, "\t-c\tSpecify a configuration file. \n");
  fprintf(stderr, "\t-k\t32 byte encryption key string in hex format. \n");
  fprintf(stderr, "\t-d\tDebug flags.");
  fprintf(stderr, "that should be used (default: /etc/vscpd.json).\n");
  fprintf(stderr, "\t-g\tPrint MIT license info.\n");
}
