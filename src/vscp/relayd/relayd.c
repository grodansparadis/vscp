// relayd.c
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
// Daemon howto - http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>

#define _GNU_SOURCE
#include <crypt.h>

#include <canalif.h>        /* CANAL DLL interfaces functionality   */ 

#define MAX_NUMBER_OF_THREADS   20
#define MAX_CLIENTS             200
#define POLL_TIMEOUT            1000
#define TRUE                    1
#define FALSE                   0

/* Prototypes */
void *workThread( void *id );

/*
    Authenticate system user

    Must be root user to execute this method.

    @param username - Username for user
    @param password - Password for user
    @return 0 == OK, 1 == User not valid, 2 == Password wrong
*/   
int auth_sys_user( const char*username, const char*password ) 
{
    struct passwd *pw;
    struct spwd *sp;
    char *encrypted, *correct;

    // Get use entry */
    pw = getpwnam( username );
    endpwent();

    if (!pw) return 1; /* user doesn't really exist */

    /* get shadow password file entry */
    sp = getspnam( pw->pw_name );
    endspent();

    /* Check validity */
    correct = sp ? sp->sp_pwdp : pw->pw_passwd;
    encrypted = crypt( password, correct );
    return strcmp( encrypted, correct ) ? 2 : 0;  /* bad pw=2, success=0 */
}

/******************************************************************************
 *                               Main routine
 * 
 * 
 * 
 * 
 * ****************************************************************************/
int main( int argc, char **argv ) 
{
    /* Our process ID and Session ID */
    pid_t pid, sid;
    int c, i, j;
    opterr = 0;
    
    pthread_t workthreads[ MAX_NUMBER_OF_THREADS ];
	pthread_attr_t thread_attr;

    static canalif_t drvobj;

    static char *pDriverPath = NULL;    /* -d --driver      */
    static char *pConfigStr = NULL;     /* -c --config      */
    unsigned long flags = 0;            /* -f --flags       */
    static int bRunStandalone = 0;      /* -s --standalone  */
    static int port = 9598;             /* -p --port        */
    static char *pInterface = NULL;     /* -i --interface   */
    static int bVerbose = 0;            /* -v --verbose     */
    static int filter = 0;              /* -f --filter      */
    static int mask = 0;                /* -m --mask        */

    canalif_init( &drvobj );  /* Init the CANAL if structure */

    uid_t uid = getuid();
    uid_t euid = geteuid();
    if ( !( ( 0 == uid ) || ( 0 == euid ) ) ) {
       printf("You must be root to run this program. uid=%d euid=%d\n", uid, euid );
       exit( EXIT_FAILURE );
    }

    for ( i=0; i<argc; i++ ) {
        printf("Argument=%s\n", argv[i] );
    }

    // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
    while ( 1 ) {

        static struct option long_options[] =
        {
            /* These options set a flag. */
            {"brief",   no_argument,       &bVerbose, 0},
          
            /* 
                These options don’t set a flag.
                We distinguish them by their indices. 
            */
            {"help",          no_argument, 0, '?'},   
            {"config",        required_argument, 0, 'c'},
            {"driver",        required_argument, 0, 'd'},
            {"filter",        required_argument, 0, 'f'},
            {"interface",     required_argument, 0, 'i'},
            {"mask",          required_argument, 0, 'm'},
            {"port",          required_argument, 0, 'p'},
            {"standalone",    no_argument,       0, 's'},
            {"verbose",       no_argument,       0, 'v'},
            {"flags",         required_argument, 0, 'z'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long( argc, 
                            argv, 
                            "?c:d:f:i:m:p:svz:",
                            long_options, 
                            &option_index );    

         /* Detect the end of the options. */
        if ( c == -1 ) break;

        switch ( c ) {
                        
            case 0:
                /* If this option set a flag, do nothing else now. */
                if ( long_options[option_index].flag != 0 ) break;
            
                printf("option %s", long_options[option_index].name);
                if ( optarg ) printf( " with arg %s", optarg );
                printf("\n");

                break;

            case 'c':
                pConfigStr = optarg;
                break;

            case 'd':
                if ( NULL == canalif_setPath( &drvobj, optarg ) ) {
                    syslog( LOG_CRIT, "Unable to set driver path. path=%s", pDriverPath );
                    exit(EXIT_FAILURE);
                }
                break;

            case 'f':
                filter = atoi( optarg );
                break;

            case 'i':
                pInterface = optarg;
                break;

            case 'm':
                mask = atoi( optarg );
                break; 

            case 'p':
                port = atoi( optarg );
                break;       

            case 's':
                bRunStandalone = 1;
                break;   

            case 'v':
                bVerbose = 1;
                break;    

            case 'z':
                flags = atoi( optarg );
                break;     

            case '?':
                puts( "option -?\n" );
                break;  

            default:
                printf("Unknown switch c='%c'\n", c);

        }



    } // while

    if ( bVerbose ) {
        puts ("verbose flag is set.");
        if ( bRunStandalone ) puts ("NOT run a as a daemon.");
    }

    if ( NULL == drvobj.m_drvPath ) {
        printf("You must specify a path to a valid CANAL driver (-d/--driver)\n");
        exit( EXIT_FAILURE );
    }
    
    if ( !bRunStandalone ) {

        /* Fork off the parent process */
        pid = fork();
        if ( pid < 0 ) {
            exit( EXIT_FAILURE );
        }

        /* 
            If we got a good PID, then
            we can exit the parent process. 
        */
        if ( pid > 0 ) {
            exit( EXIT_SUCCESS );
        }

    }

    /* Change the file mode mask */
    umask(0);
                
    /* Open any logs here */ 
    openlog("vscp-relayd", LOG_CONS | LOG_PID, LOG_DAEMON );       

    if ( !bRunStandalone ) {            
        /* Create a new SID for the child process */
        sid = setsid();
        if ( sid < 0 ) {
            /* Log the failure */
            exit( EXIT_FAILURE );
        }
    
        /* Change the current working directory */
        if ( ( chdir("/") ) < 0 ) {
            /* Log the failure */
            exit( EXIT_FAILURE );
        }   
        
        /* Close out the standard file descriptors */
    
        close( STDIN_FILENO );
        close( STDOUT_FILENO );
        close( STDERR_FILENO );
    }
        
    /* Daemon-specific initialization goes here */
	pthread_attr_init( &thread_attr );

    for (i=0; i<MAX_NUMBER_OF_THREADS; i++ ) {
        workthreads[ i ] = 0;   
    }

    // "/srv/vscp/drivers/level1/vscpl1drv-can4vscp.so"
    if ( bVerbose ) {
        printf("Driver path: %s\n", drvobj.m_drvPath );
        printf("Driver Config str: %s\n", drvobj.m_drvConfig );
        printf("Driver flags: 0x%lX\n", drvobj.m_drvFlags );      
    }

    // https://dwheeler.com/program-library/Program-Library-HOWTO/t1.html

    /* Open the driver */
    drvobj.m_hdll = dlopen( drvobj.m_drvPath, RTLD_LAZY | RTLD_GLOBAL );
    if ( NULL == drvobj.m_hdll ) {
        syslog( LOG_CRIT, "Unable to load library." );
        if ( bVerbose) fprintf( stderr, "dlopen failed: %s\n", dlerror() );
        exit(EXIT_FAILURE);
    }

    /* * * * * CANAL OPEN * * * * */
    if ( NULL == ( drvobj.m_proc_CanalOpen =
                    (LPFNDLL_CANALOPEN)dlsym( drvobj.m_hdll, "CanalOpen" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalOpen'.");
        dlclose(drvobj.m_hdll);
        exit(EXIT_FAILURE);
    }

    /* * * * * CANAL CLOSE * * * * */
    if ( NULL == ( drvobj.m_proc_CanalClose = 
                    (LPFNDLL_CANALCLOSE)dlsym( drvobj.m_hdll, "CanalClose" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalClose'.");
        dlclose(drvobj.m_hdll);
        exit(EXIT_FAILURE);
    }

    /* * * * * CANAL GETLEVEL * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetLevel =
                    (LPFNDLL_CANALGETLEVEL)dlsym( drvobj.m_hdll, "CanalGetLevel") ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalGetLevel'.");
        dlclose(drvobj.m_hdll);
        exit(EXIT_FAILURE);
    }

    /* * * * * CANAL SEND * * * * */
    if (NULL == ( drvobj.m_proc_CanalSend =
                    (LPFNDLL_CANALSEND)dlsym( drvobj.m_hdll, "CanalSend") ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalSend'.");
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL DATA AVAILABLE * * * * */
    if (NULL == ( drvobj.m_proc_CanalDataAvailable =
                    (LPFNDLL_CANALDATAAVAILABLE)dlsym( drvobj.m_hdll, "CanalDataAvailable" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalDataAvailable'." );
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL RECEIVE * * * * */
    if (NULL == ( drvobj.m_proc_CanalReceive =
                    (LPFNDLL_CANALRECEIVE)dlsym( drvobj.m_hdll, "CanalReceive" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalReceive'.");
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET STATUS * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetStatus =
                    (LPFNDLL_CANALGETSTATUS)dlsym( drvobj.m_hdll, "CanalGetStatus" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalGetStatus'.");
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET STATISTICS * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetStatistics =
                    (LPFNDLL_CANALGETSTATISTICS)dlsym( drvobj.m_hdll, "CanalGetStatistics" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalGetStatistics'.");
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL SET FILTER * * * * */
    if (NULL == ( drvobj.m_proc_CanalSetFilter =
                    (LPFNDLL_CANALSETFILTER)dlsym( drvobj.m_hdll, "CanalSetFilter" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalSetFilter'.");
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL SET MASK * * * * */
    if (NULL == ( drvobj.m_proc_CanalSetMask =
                    (LPFNDLL_CANALSETMASK)dlsym( drvobj.m_hdll, "CanalSetMask" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalSetMask'.");
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET VERSION * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetVersion =
                    (LPFNDLL_CANALGETVERSION)dlsym( drvobj.m_hdll, "CanalGetVersion" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalGetVersion'.");
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET DLL VERSION * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetDllVersion =
                    (LPFNDLL_CANALGETDLLVERSION)dlsym( drvobj.m_hdll, "CanalGetDllVersion" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalGetDllVersion'.");
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET VENDOR STRING * * * * */
    if ( NULL == ( drvobj.m_proc_CanalGetVendorString =
                    (LPFNDLL_CANALGETVENDORSTRING)dlsym( drvobj.m_hdll, "CanalGetVendorString" ) ) ) {
        // Free the library
        syslog( LOG_CRIT, "Unable to get dl entry for 'CanalGetVendorString'.");
        dlclose(drvobj.m_hdll);
        exit( EXIT_FAILURE );
    }


    /* 
        ******************************
              Generation 2 Methods
        ******************************
    */

    /* * * * * CANAL BLOCKING SEND * * * * */
    if ( NULL == ( drvobj.m_proc_CanalBlockingSend =
                    (LPFNDLL_CANALBLOCKINGSEND)dlsym( drvobj.m_hdll, "CanalBlockingSend" ) ) ) {
            syslog( LOG_INFO, "CanalBlockingSend not available. Non blocking operations set.");
    }

    /* * * * * CANAL BLOCKING RECEIVE * * * * */
    if ( NULL == ( drvobj.m_proc_CanalBlockingReceive =
                    (LPFNDLL_CANALBLOCKINGRECEIVE)dlsym( drvobj.m_hdll, "CanalBlockingReceive" ) ) ) {
            syslog( LOG_INFO, "CanalBlockingReceive not available. Non blocking operations set.");
    }

    /* * * * * CANAL GET DRIVER INFO * * * * */
    if ( NULL == ( drvobj.m_proc_CanalGetdriverInfo =
                    (LPFNDLL_CANALGETDRIVERINFO)dlsym( drvobj.m_hdll, "CanalGetDriverInfo" ) ) ) {
        syslog( LOG_INFO, "No dl entry for 'CanalGetDriverInfo'. Assume generation 1 driver.");
    }

    /* Open the driver */
    if ( ( drvobj.m_hCANAL =
            drvobj.m_proc_CanalOpen( drvobj.m_drvConfig, drvobj.m_drvFlags ) ) <= 0 ) {
        /* Failed to open */
        syslog( LOG_CRIT, 
                    "Failed to open CANAL driver. path=%s config=%s flags=%lu",
                    drvobj.m_drvPath,
                    drvobj.m_drvConfig,
                    drvobj.m_drvFlags );
        dlclose(drvobj.m_hdll);                   
        exit(EXIT_FAILURE);
    }
    else {
        /* Driver opened properly */
        syslog( LOG_INFO, 
                    "Driver opended. path=%s config=%s flags=%lu", 
                    drvobj.m_drvPath,
                    drvobj.m_drvConfig,
                    drvobj.m_drvFlags );
    }

    int len, rc, on = 1;
    int timeout;
    int listen_sd = -1;
    int new_sd = -1;
    struct sockaddr_in6 addr;
    struct pollfd fds[MAX_CLIENTS+1];   /* Listen + clients         */
    int nfds = 1;                       /* Current # connections    */
    int current_size = 0;               
    int bQuit = FALSE;
    int bCompressArray = FALSE;
    char buffer[80];

    /* Create listen socket */    
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if ( listen_sd < 0) {
        syslog( LOG_CRIT, "socket() failed" );
        /* Close CANAL driver */
        drvobj.m_proc_CanalClose( drvobj.m_hCANAL );
        /* Close DLL */
        dlclose( drvobj.m_hdll );
        exit(EXIT_FAILURE);
    }

    /* Allow socket descriptor to be reuseable */
    rc = setsockopt( listen_sd, 
                        SOL_SOCKET,  
                        SO_REUSEADDR,
                        (char *)&on, 
                        sizeof( on ) );
    if ( rc < 0 ) {
        syslog( LOG_CRIT, "setsockopt() failed");
        /* Close listen socket */
        close(listen_sd);  
        /* Close CANAL driver */
        drvobj.m_proc_CanalClose( drvobj.m_hCANAL );
        /* Close DLL */
        dlclose( drvobj.m_hdll ); 
        exit(EXIT_FAILURE);
    }

    /*************************************************************/
    /* Set socket to be nonblocking. All of the sockets for      */
    /* the incoming connections will also be nonblocking since   */
    /* they will inherit that state from the listening socket.   */
    /*************************************************************/
    rc = ioctl(listen_sd, FIONBIO, (char *)&on);
    if ( rc < 0 ) {
        syslog( LOG_CRIT, "ioctl() failed");
        /* Close listen socket */
        close(listen_sd);  
        /* Close CANAL driver */
        drvobj.m_proc_CanalClose( drvobj.m_hCANAL );
        /* Close DLL */
        dlclose( drvobj.m_hdll );
        exit(EXIT_FAILURE);
    }

    /* Bind the socket */
    memset( &addr, 0, sizeof( addr ) );
    addr.sin6_family = AF_INET6;
    memcpy( &addr.sin6_addr, &in6addr_any, sizeof( in6addr_any ) );
    addr.sin6_port = htons(port );
    rc = bind( listen_sd,
                (struct sockaddr *)&addr, 
                sizeof( addr ) );
    if (rc < 0) {
        syslog( LOG_CRIT, "bind() failed");
        /* Close listen socket */
        close(listen_sd);  
        /* Close CANAL driver */
        drvobj.m_proc_CanalClose( drvobj.m_hCANAL );
        /* Close DLL */
        dlclose( drvobj.m_hdll );
        exit(EXIT_FAILURE);
    }

    /* Set the listen back log */
    rc = listen( listen_sd, 32 );
    if ( rc < 0 ) {
        syslog( LOG_CRIT, "listen() failed");
        /* Close listen socket */
        close(listen_sd);  
        /* Close CANAL driver */
        drvobj.m_proc_CanalClose( drvobj.m_hCANAL );
        /* Close DLL */
        dlclose( drvobj.m_hdll );
        exit(EXIT_FAILURE);
    }

    /* Initialize the pollfd structure */
    memset( fds, 0 , sizeof( fds ) );

    /* Set up the initial listening socket */
    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;
            
    /* Main Loop */
    do {
        /* Call poll() and wait for it to complete. */
        rc = poll( fds, nfds, POLL_TIMEOUT );

        /* Check to see if the poll call failed.*/
        if ( rc < 0 ) {
            syslog( LOG_CRIT, "  poll() failed");
            break;
        }
           
        /* Check to see if the  time out expired. */
        if ( 0 == rc ) {
            printf("  poll() timed out.  End program.\n");

            // Send "+OK" on all rcvloop connections
            continue;
        } 

        /***********************************************************/
        /* One or more descriptors are readable.  Need to          */
        /* determine which ones they are.                          */
        /***********************************************************/
        current_size = nfds;
        for ( i = 0; i < current_size; i++ ) {

            /*********************************************************/
            /* Loop through to find the descriptors that returned    */
            /* POLLIN and determine whether it's the listening       */
            /* or the active connection.                             */
            /*********************************************************/
            if ( 0 == fds[i].revents ) {
                continue;
            }

            /*********************************************************/
            /* If revents is not POLLIN, it's an unexpected result,  */
            /* log and end the server.                               */
            /*********************************************************/
            if ( fds[i].revents != POLLIN ) {
                printf("  Error! revents = %d\n", fds[i].revents);
                bQuit = TRUE;
                break;
            }

            if ( fds[i].fd == listen_sd ) {

                /*******************************************************/
                /* Listening descriptor is readable.                   */
                /*******************************************************/
                printf("  Listening socket is readable\n");

                /*******************************************************/
                /* Accept all incoming connections that are            */
                /* queued up on the listening socket before we         */
                /* loop back and call poll again.                      */
                /*******************************************************/
                do
                {
                    /*****************************************************/
                    /* Accept each incoming connection. If               */
                    /* accept fails with EWOULDBLOCK, then we            */
                    /* have accepted all of them. Any other              */
                    /* failure on accept will cause us to end the        */
                    /* server.                                           */
                    /*****************************************************/
                    new_sd = accept( listen_sd, NULL, NULL );
                    if ( new_sd < 0 ) {
                        if (errno != EWOULDBLOCK) {
                            perror("  accept() failed");
                            bQuit = TRUE;
                        }
                        break;
                    }

                    /*****************************************************/
                    /* Add the new incoming connection to the            */
                    /* pollfd structure                                  */
                    /*****************************************************/
                    printf("  New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;

                    /*****************************************************/
                    /* Loop back up and accept another incoming          */
                    /* connection                                        */
                    /*****************************************************/
                } while ( new_sd != -1 );

            } /* listen */

            /*********************************************************/
            /* This is not the listening socket, therefore an        */
            /* existing connection must be readable                  */
            /*********************************************************/

            else {

                printf("  Descriptor %d is readable\n", fds[i].fd);
                bQuit = FALSE;

                /*******************************************************/
                /* Receive all incoming data on this socket            */
                /* before we loop back and call poll again.            */
                /*******************************************************/

                do
                {
                    /*****************************************************/
                    /* Receive data on this connection until the         */
                    /* recv fails with EWOULDBLOCK. If any other         */
                    /* failure occurs, we will close the                 */
                    /* connection.                                       */
                    /*****************************************************/
                    rc = recv( fds[i].fd, buffer, sizeof(buffer), 0 );
                    if ( rc < 0 ) {
                        if ( errno != EWOULDBLOCK ) {
                            syslog( LOG_CRIT, "  recv() failed");
                            bQuit = TRUE;
                        }
                        break;
                    }

                    /*****************************************************/
                    /* Check to see if the connection has been           */
                    /* closed by the client                              */
                    /*****************************************************/
                    if ( 0 == rc ) {
                        printf("  Connection closed\n");
                        bQuit = TRUE;
                        break;
                    }

                    /* Data was received */
                    len = rc;
                    printf("  %d bytes received\n", len);


                    /* Echo the data back to the client */
                    rc = send( fds[i].fd, buffer, len, 0 );
                    if ( rc < 0 ) {
                        syslog( LOG_CRIT, "  send() failed");
                        bQuit = TRUE;
                        break;
                    }

                } while( TRUE );

                /*******************************************************/
                /* If the close_conn flag was turned on, we need       */
                /* to clean up this active connection. This            */
                /* clean up process includes removing the              */
                /* descriptor.                                         */
                /*******************************************************/
                if ( bQuit ) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    bCompressArray = TRUE;
                }

            }

        } /* for each descriptor */

        

        /***********************************************************/
        /* If the compress_array flag was turned on, we need       */
        /* to squeeze together the array and decrement the number  */
        /* of file descriptors. We do not need to move back the    */
        /* events and revents fields because the events will always*/
        /* be POLLIN in this case, and revents is output.          */
        /***********************************************************/
        if ( bCompressArray ) {

            bCompressArray = FALSE;
            
            for (i = 0; i < nfds; i++) {

                if ( fds[i].fd == -1 ) {
                    
                    for( j = i; j < nfds; j++ ) {
                        fds[j].fd = fds[j+1].fd;
                    }

                    i--;
                    nfds--;
                }
            }
        }


    } while ( FALSE == bQuit );     /* Main loop */

    /* Clean up all of the sockets that are open */
    for ( i = 0; i < nfds; i++ ) {
        if ( fds[i].fd >= 0 ) {
            close( fds[i].fd );
        }
    }

    for ( i=0; i<MAX_NUMBER_OF_THREADS; i++ ) {
		int *rv;
        if ( 0 != workthreads[ i ] ) {
		    pthread_join( workthreads[ i ], (void **)&rv );
        }
	}

    /* Close the driver */
    if ( CANAL_ERROR_SUCCESS != drvobj.m_proc_CanalClose( drvobj.m_hCANAL ) ) {
        /* Failed to open */
        syslog( LOG_CRIT, 
                    "Failed to close CANAL driver. path=%s config=%s flags=%lu",
                    drvobj.m_drvPath,
                    drvobj.m_drvConfig,
                    drvobj.m_drvFlags );
    }
    else {
        /* Driver closed properly */
        syslog( LOG_INFO, 
                    "Driver closed. path=%s", 
                    drvobj.m_drvPath );
    }

    /* Close DLL */
    dlclose( drvobj.m_hdll );

    canalif_cleanup( &drvobj );  /* Cleanup the CANAL if structure */
   
    closelog();             /* Close syslog session*/
    exit( EXIT_SUCCESS );
}