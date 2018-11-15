// relayd.cpp
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
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>

#include <canaldlldef.h>    /* Level I driver functions */  

#define MAX_NUMBER_OF_THREADS 2

typedef struct canalif {
    /* Level I (CANAL) driver methods */
    LPFNDLL_CANALOPEN                   m_proc_CanalOpen;
    LPFNDLL_CANALCLOSE                  m_proc_CanalClose;
    LPFNDLL_CANALGETLEVEL               m_proc_CanalGetLevel;
    LPFNDLL_CANALSEND                   m_proc_CanalSend;
    LPFNDLL_CANALRECEIVE                m_proc_CanalReceive;
    LPFNDLL_CANALDATAAVAILABLE          m_proc_CanalDataAvailable;
    LPFNDLL_CANALGETSTATUS              m_proc_CanalGetStatus;
    LPFNDLL_CANALGETSTATISTICS          m_proc_CanalGetStatistics;
    LPFNDLL_CANALSETFILTER              m_proc_CanalSetFilter;
    LPFNDLL_CANALSETMASK                m_proc_CanalSetMask;
    LPFNDLL_CANALSETBAUDRATE            m_proc_CanalSetBaudrate;
    LPFNDLL_CANALGETVERSION             m_proc_CanalGetVersion;
    LPFNDLL_CANALGETDLLVERSION          m_proc_CanalGetDllVersion;
    LPFNDLL_CANALGETVENDORSTRING        m_proc_CanalGetVendorString;
    /* Generation 2 */
    LPFNDLL_CANALBLOCKINGSEND           m_proc_CanalBlockingSend;
    LPFNDLL_CANALBLOCKINGRECEIVE        m_proc_CanalBlockingReceive;
    LPFNDLL_CANALGETDRIVERINFO          m_proc_CanalGetdriverInfo;

} m_canalif;

/* Prototypes */
void *workThread( void *id );

int main( void ) 
{
    /* Our process ID and Session ID */
    pid_t pid, sid;
    int i;
    m_canalif drvobj;
    pthread_t workthreads[ MAX_NUMBER_OF_THREADS ];
	pthread_attr_t thread_attr;
        
    /* Fork off the parent process */
    pid = fork();
    if ( pid < 0 ) {
        exit( EXIT_FAILURE );
    }

    /* If we got a good PID, then
        we can exit the parent process. */
    if ( pid > 0 ) {
        exit( EXIT_SUCCESS );
    }

    /* Change the file mode mask */
    umask(0);
                
    /* Open any logs here */        
                
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
        
    openlog("vscp-relayd", LOG_CONS | LOG_PID, LOG_DAEMON );

    /* Daemon-specific initialization goes here */
	pthread_attr_init( &thread_attr );

    for (i=0; i<MAX_NUMBER_OF_THREADS; i++ ) {
        workthreads[ i ] = 0;   
    }

    /* No methods found in the driver yet */
    drvobj.m_proc_CanalOpen = NULL;
    drvobj.m_proc_CanalClose = NULL;
    drvobj.m_proc_CanalGetLevel = NULL;
    drvobj.m_proc_CanalSend = NULL;
    drvobj.m_proc_CanalDataAvailable = NULL;
    drvobj.m_proc_CanalReceive = NULL;
    drvobj.m_proc_CanalGetStatus = NULL;
    drvobj.m_proc_CanalGetStatistics = NULL;
    drvobj.m_proc_CanalSetFilter = NULL;
    drvobj.m_proc_CanalSetMask = NULL;
    drvobj.m_proc_CanalSetBaudrate = NULL;
    drvobj.m_proc_CanalGetVersion = NULL;
    drvobj.m_proc_CanalGetDllVersion = NULL;
    drvobj.m_proc_CanalGetVendorString = NULL;

    /* Generation 2 */
    drvobj.m_proc_CanalBlockingSend = NULL;
    drvobj.m_proc_CanalBlockingReceive = NULL;
    drvobj.m_proc_CanalGetdriverInfo = NULL;

    /* Open the driver */
    void *handle =  dlopen("/srv/vscp/drivers/level1/vscpl1drv-can4vscp.so", RTLD_NOW );
    if ( NULL == handle ) {
        syslog( LOG_CRIT, "Unable to load library" );
        /* Log the failure */
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL OPEN * * * * */
    if ( NULL == ( drvobj.m_proc_CanalOpen =
                    (LPFNDLL_CANALOPEN)dlsym( handle, "CanalOpen" ) ) ) {
        // Free the library
        syslog( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalOpen.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL CLOSE * * * * */
    if ( NULL == ( drvobj.m_proc_CanalClose = 
                    (LPFNDLL_CANALCLOSE)dlsym( handle, "CanalClose" ) ) ) {
        // Free the library
        //m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalClose.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GETLEVEL * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetLevel =
        (LPFNDLL_CANALGETLEVEL) m_wxdll.GetSymbol(_("CanalGetLevel")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetLevel.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL SEND * * * * */
    if (NULL == ( drvobj.m_proc_CanalSend =
        (LPFNDLL_CANALSEND) m_wxdll.GetSymbol(_("CanalSend")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalSend.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL DATA AVAILABLE * * * * */
    if (NULL == ( drvobj.m_proc_CanalDataAvailable =
        (LPFNDLL_CANALDATAAVAILABLE) m_wxdll.GetSymbol(_("CanalDataAvailable")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalDataAvailable.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }


    /* * * * * CANAL RECEIVE * * * * */
    if (NULL == ( drvobj.m_proc_CanalReceive =
        (LPFNDLL_CANALRECEIVE) m_wxdll.GetSymbol(_("CanalReceive")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalReceive.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET STATUS * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetStatus =
        (LPFNDLL_CANALGETSTATUS) m_wxdll.GetSymbol(_("CanalGetStatus")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetStatus.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET STATISTICS * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetStatistics =
        (LPFNDLL_CANALGETSTATISTICS) m_wxdll.GetSymbol(_("CanalGetStatistics")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetStatistics.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL SET FILTER * * * * */
    if (NULL == ( drvobj.m_proc_CanalSetFilter =
        (LPFNDLL_CANALSETFILTER) m_wxdll.GetSymbol(_("CanalSetFilter")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalSetFilter.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL SET MASK * * * * */
    if (NULL == ( drvobj.m_proc_CanalSetMask =
        (LPFNDLL_CANALSETMASK) m_wxdll.GetSymbol(_("CanalSetMask")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalSetMask.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET VERSION * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetVersion =
        (LPFNDLL_CANALGETVERSION) m_wxdll.GetSymbol(_("CanalGetVersion")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetVersion.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET DLL VERSION * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetDllVersion =
        (LPFNDLL_CANALGETDLLVERSION) m_wxdll.GetSymbol(_("CanalGetDllVersion")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetDllVersion.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }

    /* * * * * CANAL GET VENDOR STRING * * * * */
    if (NULL == ( drvobj.m_proc_CanalGetVendorString =
        (LPFNDLL_CANALGETVENDORSTRING) m_wxdll.GetSymbol(_("CanalGetVendorString")))) {
        // Free the library
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalGetVendorString.\n") );
        dlclose(handle);
        exit( EXIT_FAILURE );
    }


    /* 
        ******************************
              Generation 2 Methods
        ******************************
    */

    /* * * * * CANAL BLOCKING SEND * * * * */
    drvobj.m_proc_CanalBlockingSend = NULL;
    if (m_wxdll.HasSymbol(_("CanalBlockingSend"))) {
        if (NULL == ( drvobj.m_proc_CanalBlockingSend =
            (LPFNDLL_CANALBLOCKINGSEND) m_wxdll.GetSymbol(_("CanalBlockingSend")))) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalBlockingSend. Probably Generation 1 driver.\n") );
            m_wxdll.m_proc_CanalBlockingSend = NULL;
        }
    }
    else {
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": CanalBlockingSend not available. \n\tNon blocking operations set.\n") );
    }

    /* * * * * CANAL BLOCKING RECEIVE * * * * */
    drvobj.m_proc_CanalBlockingReceive = NULL;
    if (m_wxdll.HasSymbol(_("CanalBlockingReceive"))) {
        if (NULL == ( drvobj.m_proc_CanalBlockingReceive =
            (LPFNDLL_CANALBLOCKINGRECEIVE) m_wxdll.GetSymbol(_("CanalBlockingReceive")))) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": Unable to get dl entry for CanalBlockingReceive. Probably Generation 1 driver.\n") );
            m_wxdll.m_proc_CanalBlockingReceive = NULL;
        }
    }
    else {
        m_pCtrlObject->logMsg( m_pDeviceItem->m_strName +  _(": CanalBlockingReceive not available. \n\tNon blocking operations set.\n"));
    }

    /* * * * * CANAL GET DRIVER INFO * * * * */
    drvobj.m_proc_CanalGetdriverInfo = NULL;
    if (m_wxdll.HasSymbol(_("CanalGetDriverInfo"))) {
        if (NULL == ( drvobj.m_proc_CanalGetdriverInfo =
            (LPFNDLL_CANALGETDRIVERINFO) m_wxdll.GetSymbol(_("CanalGetDriverInfo")))) {
            m_pCtrlObject->logMsg( m_pDeviceItem->m_strName + _(": Unable to get dl entry for CanalGetDriverInfo. Probably Generation 1 driver.\n") );
            m_wxdll.m_proc_CanalGetdriverInfo = NULL;
        }
    }

    /* Open the device */
    m_pDeviceItem->m_openHandle =
        drvobj.m_proc_CanalOpen((const char *) m_pDeviceItem->m_strParameter.mb_str(wxConvUTF8),
                                            m_pDeviceItem->m_DeviceFlags);

    /* Check if the driver opened properly */
    if (m_pDeviceItem->m_openHandle <= 0) {
        wxString errMsg = _("Failed to open driver. Will not use it! \n\t[ ")
            + m_pDeviceItem->m_strName + _(" ]\n");
        m_pCtrlObject->logMsg( errMsg );
        /* Log the failure */
        exit( EXIT_FAILURE );
    }
    else {
        wxString wxstr =
            wxString::Format(_("Driver %s opended.\n"),
                                (const char *)m_pDeviceItem->m_strName.mbc_str() );
        m_pCtrlObject->logMsg( wxstr );
    }
        
    /* The Big Loop */
    while ( 1 ) {
        /* Do some task here ... */
           
        sleep(30); /* wait 30 seconds */
    }

    for ( i=0; i<MAX_NUMBER_OF_THREADS; i++ ) {
		int *rv;
        if ( 0 != workthreads[ i ] ) {
		    pthread_join( workthreads[ i ], (void **)&rv );
        }
	}
   
    closelog();             /* Close syslog session*/
    exit( EXIT_SUCCESS );
}